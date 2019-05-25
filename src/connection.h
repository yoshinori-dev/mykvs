//
// Created by yoshinori on 15/11/28.
//

#ifndef MYPROJ_CONNECTION_H
#define MYPROJ_CONNECTION_H

#include "kvs.h"
#include "myclib/stringutil.h"




typedef struct Connection_ {
    KVS* kvs;
    int sock;
    char* cmdbuf;
    unsigned long cmdbufsize;
    unsigned long cmdlen;
    char* key;
    unsigned int keylen;
    unsigned long expires;
    unsigned long bytes;
    char* data;
    volatile int state;
    char* sendcmdbuf;
    struct iovec sendiov[3];
    EventHandler eh;
} Connection;

Connection *Connection_New(KVS *kvs, int connectionsocket);

void Connection_Delete(Connection *self);

int Connection_Handler(void *arg);

bool Connection_IsClosed(Connection *self);

#endif //MYPROJ_CONNECTION_H
