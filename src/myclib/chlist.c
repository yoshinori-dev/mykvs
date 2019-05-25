//
// Created by yoshinori on 15/09/26.
//

#include "myclib.h"
#include "chlist.h"
#include <stdlib.h>
#include <string.h>

struct CharList_ {
    unsigned char* charArray;
    unsigned int position;
    unsigned int length;
    unsigned int capacity;
};


int INITIAL_CAPACITY = 8;

CharList* CharList_New(void) {
    CharList* self = (CharList*)my_malloc(sizeof(CharList));
    self->length = 0;
    self->capacity = INITIAL_CAPACITY;
    self->charArray = (char*)my_malloc(INITIAL_CAPACITY);
    self->position = 0;
    return self;
}

bool CharList_HasNext(CharList* self) {
    return self->length > 0 && self->position < self->length;
}

char CharList_Next(CharList* self) {
    return self->charArray[self->position++];
}

void CharList_Head(CharList* self) {
    self->position = 0;
}

/**
 * @TODO
 * thread unsafe
 */
bool CharList_Add(CharList* self, unsigned char entry) {
    if (self->length == self->capacity) {
        int newSize = self->capacity << 1;
        unsigned char* tmp = realloc(self->charArray, newSize);
        if (tmp == NULL) {
            return false;
        }
        my_memset(tmp + self->capacity, 0, newSize - self->capacity);
        self->capacity = newSize;
        self->charArray = tmp;
    }
    self->charArray[self->length++] = entry;
    return true;
}

/**
 * @TODO
 * thread unsafe
 * remove the first char found in the list.
 */
bool CharList_Remove(CharList* self, unsigned char toBeRemoved) {
    for (int pos = 0; pos < self->length; pos++) {
        if (self->charArray[pos] == toBeRemoved) {
            self->length--;
            memmove(self->charArray + pos, self->charArray + pos + 1, self->length - pos);
            return true;
        }
    }
    return false;
}

unsigned int CharList_Length(CharList* self) {
    return self->length;
}

void CharList_Delete(CharList* self) {
    free(self->charArray);
    free(self);
}
