//
// Created by yoshinori on 16/01/20.
//

#ifndef MYKVS_HASHTABLE_H
#define MYKVS_HASHTABLE_H


#include <stdbool.h>
#include "dictionaryitem.h"

typedef struct HashTable_ HashTable;

HashTable* HashTable_New(unsigned int capacity);

void HashTable_Delete(HashTable* self);

bool HashTable_RemoveExpired(HashTable* self, long long int now);

unsigned long long int HashTable_GetMemoryUsage(HashTable* self);

bool HashTable_Evict(HashTable* self, bool any);

bool HashTable_Remove(HashTable* self, char* key, unsigned int keylen);

bool HashTable_Set(HashTable* self, char* key, unsigned int keylen, void* value, unsigned long valuelen, unsigned long expires);

DictionaryItem* HashTable_Get(HashTable* self, char* key, unsigned int keylen);

void HashTable_Return(HashTable* self, DictionaryItem* item);


#endif //MYKVS_HASHTABLE_H
