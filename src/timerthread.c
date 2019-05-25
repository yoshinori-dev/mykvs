//
// Created by yoshinori on 16/04/25.
//
#include <pthread.h>
#include <syslog.h>
#include <errno.h>
#include "myclib/myclib.h"
#include "timerthread.h"


typedef struct TimerThread_ {
    pthread_t thread;
    long long intervalInNanoSec;
    void (*func)(void* arg);
    void* arg;
    pthread_mutex_t mutex;
    pthread_cond_t runningCondition;
} TimerThread;

#define NANOSEC_PER_SEC 1000000000

static int TimerThread_GetTime(struct timespec *next, long long int interval) {
    int ret = my_clock_gettime(CLOCK_REALTIME, next);
    if (ret < 0) {
        syslog(LOG_ERR, "Failed to get the OS time.");
        return ret;
    }
    next->tv_nsec += interval;
    next->tv_sec += next->tv_nsec / NANOSEC_PER_SEC;
    next->tv_nsec %= NANOSEC_PER_SEC;
    return ret;
}


static void* TimerThread_Run(void* arg) {
    TimerThread* self = (TimerThread*)arg;
    struct timespec next;
    if (TimerThread_GetTime(&next, self->intervalInNanoSec) < 0) {
        return NULL;
    }

    while (pthread_cond_timedwait(&self->runningCondition, &self->mutex,  &next) == ETIMEDOUT) {
        self->func(self->arg);
        if (TimerThread_GetTime(&next, self->intervalInNanoSec) < 0) {
            return NULL;
        }
    }
    return NULL;
}


void TimerThread_Start(TimerThread* self) {
    if (pthread_create(&self->thread, NULL, TimerThread_Run, (void *) self) < 0) {
        syslog(LOG_ERR, "Failed to create timer thread");
    }
}


/**
 * @param intervalInMilliSec  interval that func gets called. it has to be 1000 or less.
 * @param func  callback function.
 * @param arg  argument that gets passed to the func.
 */
TimerThread* TimerThread_New(long long int intervalInMilliSec, void (*func)(void*), void* arg) {
    TimerThread* self = my_malloc(sizeof(TimerThread));
    if (self == NULL) {
        return NULL;
    }
    self->func = func;
    self->arg = arg;
    self->intervalInNanoSec = intervalInMilliSec * 1000 * 1000;
    pthread_mutex_init(&self->mutex, NULL);
    pthread_cond_init(&self->runningCondition, NULL);

    return self;
}


void TimerThread_Delete(TimerThread* self) {
    pthread_cond_signal(&self->runningCondition);
    pthread_join(self->thread, NULL);
    pthread_cond_destroy(&self->runningCondition);
    pthread_mutex_destroy(&self->mutex);
    my_free(self);
}