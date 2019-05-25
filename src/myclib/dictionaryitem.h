//
// Created by yoshinori on 16/01/22.
//

#ifndef DICTIONARYITEM_H
#define DICTIONARYITEM_H

typedef struct DictionaryItem_ {
    char *key;
    unsigned int keylen;
    char *value;
    unsigned long long valuelen;
    unsigned long long expires;
    int refCnt;
    unsigned long long usedMemory;
    struct DictionaryItem_ *next;
} DictionaryItem;


DictionaryItem *DictionaryItem_New(char *key, unsigned int keylen, char *value, unsigned long valuelen, unsigned long long expires);

DictionaryItem *DictionaryItem_Next(DictionaryItem *self);

void DictionaryItem_Unchain(DictionaryItem *self, DictionaryItem *next);

void DictionaryItem_Ref(DictionaryItem *self);

void DictionaryItem_Deref(DictionaryItem *self);


#endif //DICTIONARYITEM_H
