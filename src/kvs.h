//
// Created by yoshinori on 15/11/28.
//

#ifndef MYPROJ_DB_MAIN_H
#define MYPROJ_DB_MAIN_H

#include <stdbool.h>
#include <myclib/dictionaryitem.h>

#include "pal/pal.h"

typedef struct KVS_ KVS;


KVS *KVS_New(int maxconnections, int threadsNum, unsigned long long int maxHeapSize);

void KVS_Delete(KVS *self);

int KVS_AddConnection(KVS *self, int connsock);

bool KVS_RemoveCmd(KVS *self, char *key, unsigned int keylen);

DictionaryItem *KVS_GetCmd(KVS *self, char* key, unsigned int keylen);

void KVS_ReturnItem(KVS *self, DictionaryItem *item);

bool KVS_SetCmd(KVS *self, char* key, unsigned int keylen, void* value, unsigned long valuelen, unsigned long expires);

int KVS_Observe(KVS* self, EventLoop* loop);

bool KVS_EvictIfNeeeded(KVS *self);

#endif //MYPROJ_DB_MAIN_H
