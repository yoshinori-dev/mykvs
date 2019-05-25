//
// Created by yoshinori on 16/05/04.
//

#ifndef MYKVS_TEST_TIMERTHREAD_H
#define MYKVS_TEST_TIMERTHREAD_H

#include <stdbool.h>
#include <CUnit/CUnit.h>
#include <CUnit/Console.h>
#include <malloc.h>
#include <pal/pal.h>
#include <unistd.h>

#include "timerthread.h"

volatile int callcnt;
void mycallback(void* arg) {
    int* param = (int*)arg;
    ATOMIC_INCREMENT(&callcnt);
    *param = 4649;
}
void timerthread_run(void) {
    callcnt = 0;
    void* context = malloc(1);
    TimerThread* testee = TimerThread_New(100, mycallback, context);
    TimerThread_Start(testee);
    CU_ASSERT_EQUAL(callcnt, 0);
    usleep(150000);
    CU_ASSERT_EQUAL(callcnt, 1);
    usleep(100000);
    CU_ASSERT_EQUAL(callcnt, 2);
    TimerThread_Delete(testee);
    usleep(100000);
    CU_ASSERT_EQUAL(callcnt, 2);
}

void timerthread_add_test() {
    CU_pSuite timerthreadsuite = CU_add_suite("TimerThread", NULL, NULL);
    CU_add_test(timerthreadsuite, "timerthread_run", timerthread_run);
}


#endif //MYKVS_TEST_TIMERTHREAD_H
