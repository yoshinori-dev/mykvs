//
// Created by yoshinori on 15/09/12.
//
#include <syslog.h>
#include <signal.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <myclib/myclib.h>

#include "kvs.h"
#include "utils.h"


const char* LOG_IDENT = "mykvs";
const int PORT_NUM = 5555;
const int MAX_CONNECTIONS = 100;
const int THREADS_NUM = 2;
const long long int MAX_HEAP = 100 * 1024 * 1024;

typedef struct HandlerParam_ {
    KVS *kvs;
    int listeningSock;
} HandlerParam;


void termsignal_handler(int sig) {
    if (sig == SIGTERM) {
        syslog(LOG_INFO, "Received TERM signal.");
        SendShutdownCmd(DOMAIN_SOCKET_PATH);
    }
}


int accept_handler(void *arg) {
    HandlerParam *param = (HandlerParam*)arg;
    struct sockaddr_in addr;
    socklen_t l = sizeof(addr);
    int connection_socket = accept(param->listeningSock, (struct sockaddr *) &addr, &l);
    if (connection_socket < 0) {
        syslog(LOG_ERR, "Failed to accept");
        return HANDLER_SHUTDOWN;
    }
    if (KVS_AddConnection(param->kvs, connection_socket) < 0) {
        syslog(LOG_WARNING, "Failed to accept the new connection.");
        close(connection_socket);
    }
    return HANDLER_OK;
}


int mainloop(KVS *kvs, int listening_socket) {
    EventLoop *loop = EventLoop_New();
    if (!loop) {
        syslog(LOG_ERR, "Failed to create main loop");
        goto error;
    }
    if (signal(SIGTERM, termsignal_handler) == SIG_ERR) {
        syslog(LOG_ERR, "Failed to register signal handler");
        goto error;
    }

    if (KVS_Observe(kvs, loop) < 0) {
        syslog(LOG_ERR, "Failed to observe the main loop to the KVS.");
        goto error;
    }

    HandlerParam param;
    param.kvs = kvs;
    param.listeningSock = listening_socket;
    EventHandler ehAccept;
    ehAccept.func = accept_handler;
    ehAccept.arg = &param;
    if (EventLoop_RegisterEvent(loop, listening_socket, &ehAccept) != 0) {
        syslog(LOG_ERR, "Failed to start the main loop.. %d", errno);
        goto error;
    }

    EventLoop_Run(loop, -1);
    EventLoop_Delete(loop);
    return 0;
error:
    if (loop) {
        EventLoop_Delete(loop);
    }
    return -1;
}


int main(int argc, char* argv[]) {
    openlog(LOG_IDENT, 0, LOG_LOCAL0);
    syslog(LOG_INFO, "Starting up..");
    KVS *kvs = NULL;
    if (signal(SIGINT, SIG_IGN) == SIG_ERR) {
        syslog(LOG_ERR, "Failed to initialize the signal handler.");
        goto error;
    }

    int listening_socket = socket(AF_INET, SOCK_STREAM, 0);
    int yes = 1;
    if (listening_socket < 0) {
        syslog(LOG_ERR, "Failed to create listening socket %d", errno);
        goto error;
    }
    setsockopt(listening_socket, SOL_SOCKET, SO_REUSEADDR, (const char *) &yes, sizeof(yes));

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT_NUM);
    addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(listening_socket, (struct sockaddr *) &addr, sizeof(addr)) != 0) {
        syslog(LOG_ERR, "Failed to bind listening socket %d", errno);
        goto error;
    }

    kvs = KVS_New(MAX_CONNECTIONS, THREADS_NUM, MAX_HEAP);
    if (!kvs) {
        syslog(LOG_ERR, "Failed to create the KVS instance");
        goto error;
    }


    if (listen(listening_socket, SOMAXCONN) != 0) {
        syslog(LOG_ERR, "Failed to start listening.. %d", errno);
        goto error;
    }

    mainloop(kvs, listening_socket);

    syslog(LOG_INFO, "Shutting down..");
    KVS_Delete(kvs);
    syslog(LOG_INFO, "Exit");
    return 0;
error:
    if (kvs) {
        KVS_Delete(kvs);
    }

    syslog(LOG_ERR, "Aborting..");

    return -1;
}