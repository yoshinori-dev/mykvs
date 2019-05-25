//
// Created by yoshinori on 15/09/12.
//
#include <CUnit/CUnit.h>
#include <CUnit/Console.h>

#include "testmain.h"
#include "myclib/test_linkedlist.h"
#include "myclib/test_chlist.h"
#include "myclib/test_trie.h"
#include "myclib/test_stringutil.h"
#include "myclib/test_hashtable.h"
#include "test_timerthread.h"

int main() {
    CU_initialize_registry();
    linkedlist_add_test();
    chlist_add_test();
    trie_add_test();
    stringutil_add_test();
    hashtable_add_test();
    timerthread_add_test();

    CU_console_run_tests();
    CU_cleanup_registry();
}
