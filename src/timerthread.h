//
// Created by yoshinori on 16/04/25.
//

#ifndef MYKVS_TIMERTHREAD_H
#define MYKVS_TIMERTHREAD_H

typedef struct TimerThread_ TimerThread;

TimerThread* TimerThread_New(long long int intervalInMicroSec, void (*func)(void* arg), void* arg);
void TimerThread_Delete(TimerThread* self);
void TimerThread_Start(TimerThread* self);

#endif //MYKVS_TIMERTHREAD_H
