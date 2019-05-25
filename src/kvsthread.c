//
// Created by yoshinori on 15/12/29.
//
#include <stdlib.h>
#include <sys/syslog.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <unistd.h>

#include "kvsthread.h"
#include "myclib/myclib.h"



void *KVSThread_Main(void *_param) {
    KVSThread *self = (KVSThread*)_param;

    EventLoop_Run(self->eventloop, -1);
    return NULL;
}



int KVSThread_Ctrl(void *arg) {
    syslog(LOG_INFO, "Worker thread received a control command");
    KVSThread *self = (KVSThread*)arg;
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
        syslog(LOG_ERR, "Control socket has been closed by the main thread for unknown reason.[%d]", errno);
        close(self->ctrlSock);
        return HANDLER_SHUTDOWN;
    } else if (strncmp(buf, "SHUTDOWN", 8) == 0) {
        syslog(LOG_INFO, "SHUTDOWN command");
        close(self->ctrlSock);
        return HANDLER_SHUTDOWN;
    } else {
        syslog(LOG_ERR, "Unknown control command");
    }
    return HANDLER_OK;
}


KVSThread *KVSThread_New(void) {
    KVSThread *self = my_calloc(1, sizeof(KVSThread));

    self->eventloop = EventLoop_New();
    if (!self->eventloop) {
        syslog(LOG_ERR, "Failed to create event loop instance");
        KVSThread_Delete(self);
        return NULL;
    }

    if (pthread_create(&self->thread, NULL, KVSThread_Main, (void *) self) < 0) {
        syslog(LOG_ERR, "Failed to create threads");
        KVSThread_Delete(self);
        return NULL;
    }

    return self;
}


void KVSThread_Delete(KVSThread *self) {
    if (self->eventloop) {
        EventLoop_Delete(self->eventloop);
    }

    if (self->thread) {
        pthread_join(self->thread, NULL);
        // TODO terminate thread
    }

    close(self->ctrlSock);
    my_free(self);
}

int KVSThread_AddConnection(KVSThread *self, Connection *conn) {

    if (EventLoop_RegisterEvent(self->eventloop, conn->sock, &(conn->eh)) < 0) {
        return -1;
    }
    return 0;
}

int KVSThread_Observe(KVSThread *self, int ctrlSock) {

    self->ctrlSock = ctrlSock;
    self->eh.func = KVSThread_Ctrl;
    self->eh.arg = self;

    if (EventLoop_RegisterEvent(self->eventloop, ctrlSock, &(self->eh)) < 0) {
        syslog(LOG_ERR, "Failed to register control socket to the kvs thread event loop");
        return -1;
    }
    return 0;
}