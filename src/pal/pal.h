//
// Created by yoshinori on 15/11/22.
//

#ifndef MYPROJ_PAL_H
#define MYPROJ_PAL_H


typedef struct EventLoop_ EventLoop;


typedef struct EventHandler_ {
    int (*func)(void *arg);
    void *arg;
} EventHandler;

enum HANDLER_RETURNS {
    HANDLER_OK = 0,
    HANDLER_SHUTDOWN
};

enum EVENTLOOP_STATUS {
    EVENTLOOP_OK = 0,
    EVENTLOOP_SHUTDOWN
};

EventLoop* EventLoop_New(void);
void EventLoop_Delete(EventLoop* self);
int EventLoop_RegisterEvent(EventLoop *self, int fd, EventHandler *handler);
int EventLoop_Run(EventLoop* self, int timeout);


// Counter
#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#if GCC_VERSION >= 40400
#define ATOMIC_DECREMENT(counter) __sync_sub_and_fetch(counter,1)
#define ATOMIC_INCREMENT(counter) __sync_add_and_fetch(counter,1)
#define ATOMIC_ADD(counter, plus) __sync_add_and_fetch((counter), (plus))
#define ATOMIC_SUB(counter, minus) __sync_sub_and_fetch((counter), (minus))
#else // GCC_VERSION
#error "please implement atomic operators."
#endif // GCC_VERSION


#define DOMAIN_SOCKET_PATH "/tmp/mykvs.sock"

#endif //MYPROJ_PAL_H
