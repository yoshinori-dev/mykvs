//
// Created by yoshinori on 15/09/26.
//

#ifndef MYPROJ_CHLIST_H
#define MYPROJ_CHLIST_H

#include <stdbool.h>

typedef struct CharList_ CharList;

CharList* CharList_New(void);

bool CharList_HasNext(CharList* self);

char CharList_Next(CharList* self);

bool CharList_Add(CharList* self, unsigned char entry);

bool CharList_Remove(CharList* self, unsigned char toBeRemoved);

void CharList_Head(CharList* self);

unsigned int CharList_Length(CharList* self);

void CharList_Delete(CharList* self);


#endif //MYPROJ_CHLIST_H
