//
// Created by yoshinori on 15/09/29.
//

#include <CUnit/CUnit.h>
#include <CUnit/Console.h>

#include "myclib/chlist.h"

#ifndef MYPROJ_TEST_CHLIST_H
#define MYPROJ_TEST_CHLIST_H


void chlist_add_6items(void) {
    CharList* chlist = CharList_New();
    CharList_Add(chlist, 'a');
    CharList_Add(chlist, 'b');
    CharList_Add(chlist, 'c');
    CharList_Add(chlist, 'd');
    CharList_Add(chlist, 'e');
    CharList_Add(chlist, 'f');

    CU_ASSERT(CharList_HasNext(chlist));
    CU_ASSERT(CharList_Next(chlist) == 'a');
    CU_ASSERT(CharList_HasNext(chlist));
    CU_ASSERT(CharList_Next(chlist) == 'b');
    CU_ASSERT(CharList_HasNext(chlist));
    CU_ASSERT(CharList_Next(chlist) == 'c');
    CU_ASSERT(CharList_HasNext(chlist));
    CU_ASSERT(CharList_Next(chlist) == 'd');
    CU_ASSERT(CharList_HasNext(chlist));
    CU_ASSERT(CharList_Next(chlist) == 'e');
    CU_ASSERT(CharList_HasNext(chlist));
    CU_ASSERT(CharList_Next(chlist) == 'f');
    CU_ASSERT(!CharList_HasNext(chlist));
    CharList_Delete(chlist);
}


void chlist_add_and_remove() {
    CharList* chlist = CharList_New();
    CharList_Add(chlist, 'a');
    CharList_Add(chlist, 'b');
    CharList_Add(chlist, 'c');
    CharList_Remove(chlist, 'a');
    CU_ASSERT(CharList_Length(chlist) == 2);
    CharList_Add(chlist, 'd');
    CharList_Add(chlist, 'e');
    CharList_Add(chlist, 'f');
    CU_ASSERT(CharList_Length(chlist) == 5);
    CharList_Add(chlist, 'e');
    CU_ASSERT(CharList_Length(chlist) == 6);
    CU_ASSERT(CharList_Remove(chlist, 'e'));
    CU_ASSERT(CharList_Length(chlist) == 5);
    CU_ASSERT(CharList_Remove(chlist, 'e'));
    CU_ASSERT(CharList_Length(chlist) == 4);
    CU_ASSERT(CharList_Next(chlist) == 'b');
    CU_ASSERT(!CharList_Remove(chlist, 'e'));
    CU_ASSERT(CharList_Next(chlist) == 'c');
    CU_ASSERT(CharList_Next(chlist) == 'd');
    CU_ASSERT(CharList_Next(chlist) == 'f');
    CU_ASSERT(!CharList_HasNext(chlist));
    CharList_Add(chlist, 'e');
    CU_ASSERT(CharList_HasNext(chlist));
    CU_ASSERT(CharList_Next(chlist) == 'e');
    CharList_Delete(chlist);
}

void chlist_add_test() {
    CU_pSuite chlist = CU_add_suite("ChList", NULL, NULL);
    CU_add_test(chlist, "chlist_add_6items", chlist_add_6items);
    CU_add_test(chlist, "chlist_add_and_remove", chlist_add_and_remove);
}
#endif //MYPROJ_TEST_CHLIST_H
