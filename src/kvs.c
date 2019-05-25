//
// Created by yoshinori on 15/11/28.
//

#include <pthread.h>
#include <syslog.h>
#include <errno.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/un.h>

#include "kvs.h"
#include "kvsthread.h"
#include "timerthread.h"
#include "utils.h"
#include "myclib/myclib.h"
#include "myclib/linkedlist.h"
#include "myclib/hashtable.h"


typedef struct KVS_ {
    HashTable *db;
    unsigned int threadsNum;
    KVSThread **kvsThreads;
    TimerThread *timerThread;
    LinkedList *connectionList;
    int maxConnections;
    int ctrlSock;
    EventHandler eh;
    int nextThread;
    long long maxHeapSize;
} KVS;


void KVS_CreateSocketPath(int threadNo, char *buf, int buflen) {
    my_snprintf(buf, buflen, "%s.%d", DOMAIN_SOCKET_PATH, threadNo);
}


int KVS_Ctrl(void *arg) {
    KVS *self = (KVS*)arg;
    char buf[32];
    my_memset(buf, 0, 32);
    struct sockaddr_un addr;
    socklen_t l = sizeof(addr);
    int connSock = accept(self->ctrlSock, (struct sockaddr *) &addr, &l);
    if (connSock < 0) {
        syslog(LOG_ERR, "Failed to accept on the control socket.");
        return HANDLER_OK;
    }

    int nrd = recv(connSock, buf, 32, 0);
    if (nrd <= 0) {
        syslog(LOG_ERR, "Control socket has been closed by the main thread for some unknown reason.[%d]", errno);
        close(connSock);
        return HANDLER_SHUTDOWN;
    } else if (strncmp(buf, "SHUTDOWN", 8) == 0) {
        syslog(LOG_INFO, "Terminating the worker threads..");
        for (int i = 0; i < self->threadsNum; i++) {
            char buf[128];
            KVS_CreateSocketPath(i, buf, 128);
            SendShutdownCmd(buf);
        }
        close(connSock);
        return HANDLER_SHUTDOWN;
    } else {
        syslog(LOG_ERR, "Unknown control command");
    }
    close(connSock);
    return HANDLER_OK;
}


bool KVS_RemoveCmd(KVS *self, char *key, unsigned int keylen) {
    return HashTable_Remove(self->db, key, keylen);
}


DictionaryItem *KVS_GetCmd(KVS *self, char *key, unsigned int keylen) {
    return HashTable_Get(self->db, key, keylen);
}


void KVS_ReturnItem(KVS *self, DictionaryItem *item) {
    HashTable_Return(self->db, item);
}


bool KVS_SetCmd(KVS *self, char *key, unsigned int keylen, void *value, unsigned long valuelen, unsigned long expires) {
    return HashTable_Set(self->db, key, keylen, value, valuelen, expires);
}


bool KVS_EvictIfNeeeded(KVS *self) {
    long long int toBeFreed = HashTable_GetMemoryUsage(self->db) - self->maxHeapSize;
    while (toBeFreed > 0) {
        if (!HashTable_Evict(self->db, false)) {
            return false;
        }
        toBeFreed = HashTable_GetMemoryUsage(self->db) - self->maxHeapSize;
    }
}


static void KVS_CleanupClosedConnections(KVS *self) {
    Connection *conn;
    LinkedList *tobeClosed = LinkedList_New();
    while ((conn = LinkedList_Next(self->connectionList)) != NULL) {
        if (Connection_IsClosed(conn)) {
            LinkedList_Add(tobeClosed, conn);
        }
    }
    while ((conn = LinkedList_Next(tobeClosed)) != NULL) {
        LinkedList_Remove(self->connectionList, conn);
        Connection_Delete(conn);
    }
    LinkedList_Delete(tobeClosed);
}


/**
 * @return 0 if successful -1 otherwise
 *
 */
int KVS_AddConnection(KVS *self, int connsock) {
    if (LinkedList_Size(self->connectionList) >= self->maxConnections) {
        // make room for the new connection.
        KVS_CleanupClosedConnections(self);
        if (LinkedList_Size(self->connectionList) >= self->maxConnections) {
            syslog(LOG_ERR, "Failed to create a new connection. Max has been reached.");
            return -1;
        }
    }
    Connection *conn = Connection_New(self, connsock);
    if (!conn) {
        syslog(LOG_ERR, "Failed to create connection");
        return -1;
    }
    if (LinkedList_Add(self->connectionList, conn) < 0) {
        syslog(LOG_ERR, "Failed to add connection");
        return -1;
    }
    int ret = KVSThread_AddConnection(self->kvsThreads[self->nextThread], conn);
    if (++self->nextThread >= self->threadsNum) {
        self->nextThread = 0;
    }
    return ret;
}


/**
 * register the kvs to watch the loop.
 */
int KVS_Observe(KVS *self, EventLoop *loop) {
    syslog(LOG_INFO, "Prepare for control connection at %s", DOMAIN_SOCKET_PATH);
    struct sockaddr_un addr;
    my_memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    my_memcpy(addr.sun_path, DOMAIN_SOCKET_PATH, my_strlen(DOMAIN_SOCKET_PATH));
    unlink(DOMAIN_SOCKET_PATH);
    self->ctrlSock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (self->ctrlSock < 0) {
        syslog(LOG_ERR, "Failed to create control socket. errno[%d]\n", errno);
        return -1;
    }
    if (bind(self->ctrlSock, (struct sockaddr*)&addr, sizeof(struct sockaddr_un)) < 0) {
        syslog(LOG_ERR, "Failed to bind control socket.");
        return -1;
    }
    if (listen(self->ctrlSock, SOMAXCONN) != 0) {
        syslog(LOG_ERR, "Failed to start listening on the control socket.. %d", errno);
        return -1;
    }
    self->eh.func = KVS_Ctrl;
    self->eh.arg = self;
    if (EventLoop_RegisterEvent(loop, self->ctrlSock, &(self->eh)) < 0) {
        syslog(LOG_ERR, "Failed to register control socket to the event loop");
        return -1;
    }

    for (int i = 0; i < self->threadsNum; i++) {
        struct sockaddr_un addr;
        my_memset(&addr, 0, sizeof(struct sockaddr_un));
        addr.sun_family = AF_UNIX;
        char path[128];
        KVS_CreateSocketPath(i, path, 128);
        my_memcpy(addr.sun_path, path, my_strlen(path));
        unlink(path);
        int threadCtrlSock = socket(AF_UNIX, SOCK_STREAM, 0);
        if (threadCtrlSock < 0) {
            syslog(LOG_ERR, "Failed to create thread control socket. errno[%d]\n", errno);
            return -1;
        }
        if (bind(threadCtrlSock, (struct sockaddr*)&addr, sizeof(struct sockaddr_un)) < 0) {
            syslog(LOG_ERR, "Failed to bind thread control socket.");
            return -1;
        }
        if (listen(threadCtrlSock, SOMAXCONN) != 0) {
            syslog(LOG_ERR, "Failed to start listening on the thread control socket.. %d", errno);
            return -1;
        }
        KVSThread_Observe(self->kvsThreads[i], threadCtrlSock);
    }
    return 0;
}


void KVS_Vacuum(void* arg) {
    KVS* self = (KVS*)arg;
    HashTable_RemoveExpired(self->db, my_time(NULL));
}


const unsigned int INIT_CAPACITY = 4096;
const long long int VACUUM_INTERVAL = 1000;
KVS *KVS_New(int maxconnections, int threadsNum, unsigned long long int maxHeapSize) {
    KVS *self = my_calloc(1, sizeof(KVS));
    self->db = HashTable_New(INIT_CAPACITY);
    if (!self->db) {
        syslog(LOG_ERR, "Failed to create trie instance");
        KVS_Delete(self);
        return NULL;
    }

    self->connectionList = LinkedList_New();
    self->maxConnections = maxconnections;
    self->threadsNum = threadsNum;
    self->maxHeapSize = maxHeapSize;
    self->kvsThreads = my_calloc(self->threadsNum, sizeof(KVSThread));
    if (!self->kvsThreads) {
        syslog(LOG_ERR, "Failed to create kvs thread array");
        KVS_Delete(self);
        return NULL;
    }
    self->timerThread = TimerThread_New(VACUUM_INTERVAL, KVS_Vacuum, self);
    if (!self->timerThread) {
        syslog(LOG_ERR, "Failed to create timer thread");
        KVS_Delete(self);
        return NULL;
    }
    for (int i = 0; i < self->threadsNum; i++) {
        self->kvsThreads[i] = KVSThread_New();
        if (!self->kvsThreads[i]) {
            syslog(LOG_ERR, "Failed to create kvs thread");
            KVS_Delete(self);
            return NULL;
        }
    }
    TimerThread_Start(self->timerThread);
    return self;
}

void KVS_Delete(KVS *self) {
    if (self->db) {
        HashTable_Delete(self->db);
    }
    if (self->connectionList) {
        Connection *conn;
        while ((conn = LinkedList_Next(self->connectionList)) != NULL) {
            LinkedList_Remove(self->connectionList, conn);
            Connection_Delete(conn);
        }
        LinkedList_Delete(self->connectionList);
    }
    if (self->ctrlSock > 0) {
        close(self->ctrlSock);
    }
    if (self->timerThread) {
        TimerThread_Delete(self->timerThread);
    }
    if (self->kvsThreads) {
        for (int i = 0; i < self->threadsNum; i++) {
            if (self->kvsThreads[i]) {
                KVSThread_Delete(self->kvsThreads[i]);
            }
        }
        my_free(self->kvsThreads);
    }

    my_free(self);
}
