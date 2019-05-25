//
// Created by yoshinori on 16/01/22.
//

#include <pthread.h>
#include <pal/pal.h>
#include "myclib.h"

#include "dictionaryitem.h"


DictionaryItem *DictionaryItem_New(char *key, unsigned int keylen, char *value, unsigned long valuelen, unsigned long long expires) {
    DictionaryItem *self = my_malloc(sizeof(DictionaryItem));
    if (!self) {
        return NULL;
    }
    self->usedMemory = sizeof(DictionaryItem);
    self->usedMemory += keylen + 1;
    self->key = my_malloc(keylen + 1);
    if (!self->key) {
        my_free(self);
        return NULL;
    }
    my_memcpy(self->key, key, keylen);
    self->key[keylen] = 0;
    self->keylen = keylen;

    self->usedMemory += valuelen;
    self->value = my_malloc(valuelen);
    if (!self->value) {
        my_free(self->key);
        my_free(self);
        return NULL;
    }
    my_memcpy(self->value, value, valuelen);
    self->valuelen = valuelen;
    self->expires = my_time(NULL) + expires;
    self->refCnt = 1;
    self->next = NULL;
    return self;
}

static void DictionaryItem_Delete(DictionaryItem *self) {
    my_free(self->key);
    my_free(self->value);
    my_free(self);
}


DictionaryItem *DictionaryItem_Next(DictionaryItem *self) {
    return self->next;
}


void DictionaryItem_Unchain(DictionaryItem *self, DictionaryItem *next) {
    self->next = next->next;
    next->next = NULL;
}


void DictionaryItem_Ref(DictionaryItem *self) {
    self->refCnt++;
}

void DictionaryItem_Deref(DictionaryItem *self) {
    ATOMIC_DECREMENT(&self->refCnt);
    if (self->refCnt <= 0) {
        DictionaryItem_Delete(self);
    }
}
