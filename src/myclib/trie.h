//
// Created by yoshinori on 15/09/12.
//

#ifndef MYPROJ_TRIE_H
#define MYPROJ_TRIE_H

#include <stdbool.h>

typedef struct Trie_ Trie;

Trie* Trie_New(void);

void Trie_Delete(Trie* self);

bool Trie_Set(Trie* self, unsigned char* key, int kenlen, void* entry, void** old);

void* Trie_Find(Trie* self, unsigned char* key, int keylen);

#endif //MYPROJ_TRIE_H
