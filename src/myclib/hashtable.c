//
// Created by yoshinori on 16/01/20.
//
#include <pthread.h>
#include <pal/pal.h>
#include "dictionaryitem.h"
#include "myclib.h"
#include "stringutil.h"
#include "hashtable.h"


typedef struct HashTable_ {
    DictionaryItem** table;
    pthread_mutex_t* locks;
    long long int usedMemory;
    unsigned int size;
    unsigned int lockSize;
    unsigned int evictionPosition;
} HashTable;


typedef enum EVICT_ {
    ONE_EXPIRED = 0,
    ALL_EXPIRED,
    WITH_EXPIRES,
    ANY
} EVICT;

HashTable* HashTable_New(unsigned int capacity) {
    HashTable *self = my_calloc(1, sizeof(HashTable));
    if (!self) {
        return NULL;
    }
    self->size = capacity;
    self->table = my_calloc(self->size, sizeof(DictionaryItem*));
    if (!self->table) {
        my_free(self);
        return NULL;
    }
    self->lockSize = capacity >> 5;
    if (self->lockSize < 1) {
        self->lockSize = 1;
    }
    self->locks = my_calloc(self->lockSize, sizeof(pthread_mutex_t));
    if (!self->locks) {
        my_free(self->table);
        my_free(self);
        return NULL;
    }
    for (int i = 0; i < self->lockSize; i++) {
        self->locks[i] = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    }
    self->usedMemory = 0l;
    return self;
}

void HashTable_Delete(HashTable* self) {
    if  (self->table) {
        for (int i = 0; i < self->size; i++) {
            if (self->table[i]) {
                DictionaryItem *currentItem = self->table[i];
                while (currentItem != NULL) {
                    DictionaryItem *next = DictionaryItem_Next(currentItem);
                    DictionaryItem_Deref(currentItem);
                    currentItem = next;
                }
            }
        }
        my_free(self->table);
    }
    for (int i = 0; i < self->lockSize; i++) {
        pthread_mutex_destroy(&self->locks[i]);
    }
    my_free(self->locks);
    my_free(self);
}

static void HashTable_DoRemove(HashTable* self, unsigned int hash, DictionaryItem* prevItem, DictionaryItem* toBeRemoved) {
    if (prevItem) {
        DictionaryItem_Unchain(prevItem, toBeRemoved);
    } else {
        self->table[hash] = toBeRemoved->next;
    }
    ATOMIC_SUB(&self->usedMemory, toBeRemoved->usedMemory);
    DictionaryItem_Deref(toBeRemoved);
}


// Please note mutation lock has to be done outside this function.
// This function does not increment the reference counter.
static bool HashTable_Search(HashTable *self, char* key, unsigned int keylen, unsigned int hash, DictionaryItem** prev, DictionaryItem** found) {

    DictionaryItem *currentItem = self->table[hash];
    *prev = NULL;
    while (currentItem != NULL) {
        int l = currentItem->keylen;
        if (l == keylen && my_memcmp(currentItem->key, key, l) == 0) {
            *found = currentItem;
            return true;
        }
        *prev = currentItem;
        currentItem = DictionaryItem_Next(currentItem);
    }
    return false;
}


static bool HashTable_EvictFromSlot(HashTable *self, unsigned int pos, EVICT evictMode, long long int now) {
    pthread_mutex_t* lock = &self->locks[pos % self->lockSize];
    pthread_mutex_lock(lock);
    DictionaryItem* currentItem = self->table[pos];
    DictionaryItem *candidate = NULL;
    DictionaryItem *prevItem = NULL;
    DictionaryItem *candidatePrev = NULL;
    unsigned long cnt = 0;
    while (currentItem != NULL) {
        switch(evictMode) {
            case ANY:
                HashTable_DoRemove(self, pos, NULL, currentItem);
                cnt++;
                break;
            case ALL_EXPIRED:
            case ONE_EXPIRED:
                if (currentItem->expires > 0 && currentItem->expires < now) {
                    candidate = currentItem;
                    candidatePrev = prevItem;
                    prevItem = currentItem;
                    currentItem = DictionaryItem_Next(currentItem);
                    HashTable_DoRemove(self, pos, candidatePrev, candidate);
                    cnt++;
                } else {
                    prevItem = currentItem;
                    currentItem = DictionaryItem_Next(currentItem);
                }
                break;
            case WITH_EXPIRES:
                if (currentItem->expires > 0 && (candidate == NULL || candidate->expires >= currentItem->expires)) {
                    candidate = currentItem;
                    candidatePrev = prevItem;
                }
                prevItem = currentItem;
                currentItem = DictionaryItem_Next(currentItem);
                break;
        }
        if (cnt > 0 && evictMode != ALL_EXPIRED) {
            break;
        }
    }
    if (cnt == 0 && candidate != NULL) {
        HashTable_DoRemove(self, pos, candidatePrev, candidate);
        cnt++;
    }

    pthread_mutex_unlock(lock);
    return cnt > 0;
}


/**
 * @return true if successfully removed. false otherwise
 */
bool HashTable_Remove(HashTable* self, char* key, unsigned int keylen) {
    return HashTable_Set(self, key, keylen, NULL, 0, 0);
}

/**
 * @param now unixtime in sec
 * @return true when removed some items, false when no items were removed
 */
bool HashTable_RemoveExpired(HashTable* self, long long int now) {
    unsigned int pos = self->evictionPosition;
    unsigned int startPos = pos;
    bool evicted;
    while (true) {
        evicted = HashTable_EvictFromSlot(self, pos, ONE_EXPIRED, now);
        if (++pos >= self->size) {
            pos -= self->size;
        }
        if (evicted || pos == startPos) {
            break;
        }
    }
    return evicted;
}



unsigned long long int HashTable_GetMemoryUsage(HashTable* self) {
    return self->usedMemory;
}


/**
 * @param any if true it might remove item that expires is zero
 * @return true if it removed an item successfuly. false otherwise
 *
 */
bool HashTable_Evict(HashTable* self, bool any) {

    unsigned int pos = self->evictionPosition;
    unsigned int startPos = pos;

    bool evicted = HashTable_EvictFromSlot(self, pos, ALL_EXPIRED, my_time(NULL));
    while (!evicted) {
        evicted = HashTable_EvictFromSlot(self, pos, WITH_EXPIRES, 0);
        if (++pos >= self->size) {
            pos -= self->size;
        }
        if (pos == startPos) {
            break;
        }
    }
    while (any && !evicted) {
        evicted = HashTable_EvictFromSlot(self, pos, ANY, 0);
        if (++pos >= self->size) {
            pos -= self->size;
        }
        if (pos == startPos) {
            break;
        }
    }
    self->evictionPosition = pos;
    return evicted;
}


/**
 * set. if valuelen == 0, then remove the item.
 */
bool HashTable_Set(HashTable* self, char* key, unsigned int keylen, void* value, unsigned long valuelen, unsigned long expires) {
    unsigned int inthash = StringUtil_Hash(key, keylen);
    unsigned int hash = inthash % self->size;
    unsigned int lock = hash % self->lockSize;

    DictionaryItem *newItem;
    if (valuelen > 0) {
        newItem = DictionaryItem_New(key, keylen, value, valuelen, expires);
        if (!newItem) {
            return false;
        }
    }

    bool ret = false;
    DictionaryItem *prevItem;
    DictionaryItem *old;
    pthread_mutex_lock(&self->locks[lock]);
    if (HashTable_Search(self, key, keylen, hash, &prevItem, &old)) {
        // found the same key item. remove.
        HashTable_DoRemove(self, hash, prevItem, old);
        ret = true;
    }
    if (valuelen > 0) {
        // Should we think abount 'real' size of allocated by using  'malloc_usable_size' etc..?
        ATOMIC_ADD(&self->usedMemory, newItem->usedMemory);
        newItem->next = self->table[hash];
        self->table[hash] = newItem;
        ret = true;
    }
    pthread_mutex_unlock(&self->locks[lock]);
    return ret;
}

DictionaryItem* HashTable_Get(HashTable* self, char* key, unsigned int keylen) {
    unsigned int inthash = StringUtil_Hash(key, keylen);
    unsigned int hash = inthash % self->size;
    unsigned int lock = hash % self->lockSize;
    DictionaryItem* prev;
    DictionaryItem* item;
    pthread_mutex_lock(&self->locks[lock]);
    if (!HashTable_Search(self, key, keylen, hash, &prev, &item)) {
        pthread_mutex_unlock(&self->locks[lock]);
        return NULL;
    }
    if (item->expires > 0 && item->expires < my_time(NULL)) {
        pthread_mutex_unlock(&self->locks[lock]);
        HashTable_Remove(self, key, keylen);
        return NULL;
    }
    DictionaryItem_Ref(item);
    pthread_mutex_unlock(&self->locks[lock]);
    return item;
}

void HashTable_Return(HashTable* self, DictionaryItem* item) {
    unsigned int inthash = StringUtil_Hash(item->key, item->keylen);
    unsigned int hash = inthash % self->size;
    unsigned int lock = hash % self->lockSize;
    pthread_mutex_lock(&self->locks[lock]);
    DictionaryItem_Deref(item);
    pthread_mutex_unlock(&self->locks[lock]);
}