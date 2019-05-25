//
// Created by yoshinori on 15/11/22.
//
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/epoll.h>
#include <syslog.h>
#include <unistd.h>
#include <errno.h>


#include "myclib/myclib.h"
#include "pal.h"

// EventLoops


#define MAX_EVENTS  128

typedef struct EventLoop_ {
    int epfd;
    struct epoll_event events[MAX_EVENTS];
} EventLoop;


EventLoop *EventLoop_New(void) {
    EventLoop* self = my_malloc(sizeof(EventLoop));
    if (!self) {
        return NULL;
    }
    self->epfd = epoll_create(4096);
    if (self->epfd < 0) {
        syslog(LOG_ERR, "Failed to create epoll");
        my_free(self);
        return NULL;
    }
    return self;
}


int EventLoop_RegisterEvent(EventLoop *self, int fd, EventHandler *handler) {
    struct epoll_event ev;
    ev.data.ptr = handler;
    ev.events = EPOLLIN;
    if (epoll_ctl(self->epfd, EPOLL_CTL_ADD, fd, &ev) < 0) {
        syslog(LOG_ERR,  "Failed to register event. %d", errno);
        return -1;
    }
    return 0;
}


/**
 * TODO: shut down
 */
int EventLoop_Run(EventLoop* self, int timeout) {
    while (1) {
        int nfd = epoll_wait(self->epfd, self->events, MAX_EVENTS, timeout);
        if (nfd < 0) {
            if (errno == EINTR) {
                continue;
            }
            syslog(LOG_ERR, "epoll_wait error %d", errno);
            return EVENTLOOP_SHUTDOWN;
        }
        for (int i = 0; i < nfd; i++) {
            EventHandler *handler = self->events[i].data.ptr;
            switch (handler->func(handler->arg)) {
                case HANDLER_SHUTDOWN:
                    return EVENTLOOP_SHUTDOWN;
                default:
                    continue;
            }
        }
    };
}


void EventLoop_Delete(EventLoop* self) {
    close(self->epfd);
    my_free(self);
}

