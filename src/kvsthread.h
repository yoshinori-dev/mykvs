//
// Created by yoshinori on 15/12/29.
//

#ifndef MYKVS_KVSTHREAD_H
#define MYKVS_KVSTHREAD_H

#include "connection.h"

typedef struct KVSThread_ {
    EventLoop *eventloop;
    pthread_t thread;
    EventHandler eh;
    int ctrlSock;
} KVSThread;

KVSThread *KVSThread_New(void);

void KVSThread_Delete(KVSThread *self);

int KVSThread_AddConnection(KVSThread *self, Connection *conn);

int KVSThread_Observe(KVSThread *self, int ctrlSock);

#endif //MYKVS_KVSTHREAD_H
