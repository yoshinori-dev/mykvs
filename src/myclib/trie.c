#include <stdio.h>
#include <stdbool.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "trie.h"
#include "chlist.h"
#include "myclib.h"



typedef struct Trie_ {
    long long* base;
    long long* check;
    CharList** children;
    void** record;
    long long size;
} Trie;


static void Trie_AddChild(Trie* self, long long idx, unsigned char key);
static void* Trie_SearchKey(Trie* self, unsigned char* key, int keylen, long long* errIdx, int *errKeyIdx);

const size_t DA_SIZE = 10240;
Trie* Trie_New() {
    Trie* self = (Trie*)my_malloc(sizeof(Trie));
    if (self == NULL) {
        return NULL;
    }

    self->base = my_calloc(DA_SIZE,  sizeof(long long));
    if (self->base == NULL) {
        goto error;
    }
    self->check = my_calloc(DA_SIZE, sizeof(long long));
    if (self->check == NULL) {
        goto error;
    }
    self->record = my_calloc(DA_SIZE, sizeof(void*));
    if (self->record == NULL) {
        goto error;
    }
    self->children = my_calloc(DA_SIZE, sizeof(CharList*));
    if (self->children == NULL) {
        goto error;
    }

    self->size = DA_SIZE;
    self->base[1] = 1;
    return self;
error:
    Trie_Delete(self);
    return NULL;
}

/**
 */
void Trie_Delete(Trie* self) {
    if (self) {
        if (self->base)
            my_free(self->base);
        if (self->check)
            my_free(self->check);
        if (self->record)
            my_free(self->record);
        if (self->children) {
            for(long long i = 0; i < self->size; i++) {
                if (self->children[i]) {
                    CharList_Delete(self->children[i]);
                }
            }
            my_free(self->children);
        }

        my_free(self);
    }
}


/**
 * @brief expand trie.user
 */
static bool Trie_Expand(Trie* self) {
    long long newSize = self->size << 1;
    void* tmp;
    if ((tmp = my_realloc(self->base, newSize * sizeof(long long))) == NULL) {
        return false;
    }
    self->base = tmp;
    my_memset(self->base + self->size, 0, self->size * sizeof(long long));
    if ((tmp = my_realloc(self->check, newSize * sizeof(long long))) == NULL) {
        return false;
    }
    self->check = tmp;
    my_memset(self->check + self->size, 0, self->size * sizeof(long long));
    if ((tmp = my_realloc(self->record, newSize * sizeof(void*))) == NULL) {
        return false;
    }
    self->record = tmp;
    my_memset(self->record + self->size, 0, self->size * sizeof(void*));
    if ((tmp = my_realloc(self->children, newSize * sizeof(CharList*))) == NULL) {
        return false;
    }
    self->children = tmp;
    my_memset(self->children + self->size, 0, self->size * sizeof(CharList*));
    self->size = newSize;
    return true;
}

/**
 * @param self self
 * @param idx  index of the node to which we want to add a new edge.
 * @param ch   character that we want to add.
 * @return     base value that accommodates all the children of idx. -1 if no slots are available.
 */
static long long Trie_SearchAvailableBaseValue(Trie* self, long long idx, unsigned char ch) {

    long long lastIdx = self->size - UCHAR_MAX - 1;
    if (idx >= lastIdx) {
        return -1;
    }

    CharList* children = self->children[idx];
    for (long long testBaseVal = self->base[idx] == 0 ? 1 : self->base[idx]; testBaseVal < lastIdx; testBaseVal++) {
        bool conflict = false;
        if (self->check[testBaseVal + ch] != 0) {
            continue;
        }
        if (children != NULL) {
            CharList_Head(children);
            while (CharList_HasNext(children)) {
                if (self->check[testBaseVal + CharList_Next(children)] != 0) {
                    conflict = true;
                    break;
                }
            }
        }
        if (!conflict) {
            return testBaseVal;
        }
    }
    return -1;
}


/**
 * @brief move node.
 */
static void Trie_Move(Trie* self, long long idx, long long newBaseValue) {
    long long* base = self->base;
    long long* check = self->check;
    void** record = self->record;
    long long orgBaseValue = base[idx];
    base[idx] = newBaseValue;
    CharList* children = self->children[idx];
    if (children != NULL) {
        CharList_Head(children);
        while (CharList_HasNext(children)) {
            char child = CharList_Next(children);
            long long newChildIdx = newBaseValue + child;
            long long orgChildIdx = orgBaseValue + child;
            check[newChildIdx] = idx;
            check[orgChildIdx] = 0;
            base[newChildIdx] = base[orgChildIdx];
            base[orgChildIdx] = 0;
            record[newChildIdx] = record[orgChildIdx];
            record[orgChildIdx] = NULL;
            CharList *grandChildren = self->children[newChildIdx] = self->children[orgChildIdx];
            self->children[orgChildIdx] = NULL;
            if (grandChildren != NULL) {
                CharList_Head(grandChildren);
                while (CharList_HasNext(grandChildren)) {
                    char grandchild = CharList_Next(grandChildren);
                    check[base[newChildIdx] + grandchild] = newChildIdx;
                }
            }
        }
    }
}

static void Trie_AddChild(Trie* self, long long idx, unsigned char key) {
    if (self->children[idx] == NULL) {
        self->children[idx] = CharList_New();
    }
    CharList_Add(self->children[idx], key);
}

/**
 * @brief add a key to the trie. thread unsafe
 * return false when it failed to add.
 */
bool Trie_Set(Trie* self, unsigned char* key, int keylen, void* record, void** old) {
    long long idx = 0;
    int keyIdx = 0;
    Trie_SearchKey(self, key, keylen, &idx, &keyIdx);
    for (int i = keyIdx; i < keylen;) {
        long long baseValue = Trie_SearchAvailableBaseValue(self, idx, key[i]);
        if (baseValue < 0) {
            // no available slot. try to expand.
            if (!Trie_Expand(self)) {
                return false;
            }

            baseValue = Trie_SearchAvailableBaseValue(self, idx, key[i]);
            if (baseValue < 0) {
                return false;
            }
        }
        if (self->base[idx] != 0 && baseValue != self->base[idx]) {
            // conflict. need to move node.
            Trie_Move(self, idx, baseValue);
        } else {
            // add node.
            self->base[idx] = baseValue;
        }
        self->check[baseValue + key[i]] = idx;
        Trie_AddChild(self, idx, key[i]);

        idx = baseValue + key[i];
        i++;
    }

    *old = self->record[idx];
    self->record[idx] = record;
    return true;
}

/**
 * @brief search key and return the record.
 *
 */
static void* Trie_SearchKey(Trie* self, unsigned char* key, int keylen, long long* errIdx, int *errKeyIdx) {
    long long* base = self->base;
    long long* check = self->check;
    long long idx = 1;
    long long next;
    for (int i = 0; i < keylen; i++) {
        next = base[idx] + key[i];
        if (check[next] != idx) {
            *errIdx = idx;
            *errKeyIdx = i;
            return NULL;
        }
        idx = next;
    }
    void* record = self->record[idx];
    *errIdx = idx;
    *errKeyIdx = keylen;
    return record;

}

/**
 * @param self self
 * @param key  key to add
 * @param keylen length of the key
 * @return     pointer to the entry. NULL if not found.
 */

void* Trie_Find(Trie* self, unsigned char* key, int keylen) {
    long long dummy;
    int dummy2;
    return Trie_SearchKey(self, key, keylen, &dummy, &dummy2);
}
