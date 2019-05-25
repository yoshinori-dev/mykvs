#include <CUnit/CUnit.h>
#include <CUnit/Console.h>

#include "myclib/linkedlist.h"
#include "myclib/myclib.h"



void linkedlist_add_and_next(void) {

    int data = 4649;
    int data2 = 5963;
    LinkedList* testee = LinkedList_New();
    LinkedList_Add(testee, &data);
    LinkedList_Add(testee, &data2);
    int* taken = LinkedList_Next(testee);
    CU_ASSERT(*taken == 4649);
    int* taken2 = LinkedList_Next(testee);
    CU_ASSERT(*taken2 == 5963);
    CU_ASSERT(LinkedList_Next(testee) == NULL);
    LinkedList_Delete(testee);
    CU_ASSERT(*taken == 4649);
    CU_ASSERT(*taken2 == 5963);
    // takenValue_does_not_get_affected_after_Delete
    CU_ASSERT(taken == &data);
    CU_ASSERT(taken2 == &data2);
}

void linkedlist_add_and_remove(void) {
    const int BUF_SIZE = 128;
    const int ARRAY_SIZE = 16;
    char *bufferArray[ARRAY_SIZE];
    LinkedList *testee = LinkedList_New();
    for (int i = 0; i < ARRAY_SIZE; i++) {
        bufferArray[i] = my_malloc(BUF_SIZE);
        my_memset(bufferArray[i], i, BUF_SIZE);
        CU_ASSERT(LinkedList_Size(testee) == i);
        LinkedList_Add(testee, bufferArray[i]);
    }
    for (int k = 0; k < ARRAY_SIZE; k++) {
        char *data = LinkedList_Next(testee);
        char expected[BUF_SIZE];
        my_memset(expected, k, BUF_SIZE);
        CU_ASSERT(strncmp(data, expected, BUF_SIZE) == 0);
    }
    CU_ASSERT(LinkedList_Next(testee) == NULL);
    for (int j = 0; j < ARRAY_SIZE; j++) {
        CU_ASSERT(LinkedList_Size(testee) == ARRAY_SIZE - j);
        LinkedList_Remove(testee, bufferArray[j]);
        my_free(bufferArray[j]);
    }
    CU_ASSERT(LinkedList_Size(testee) == 0);

    for (int i = 0; i < ARRAY_SIZE; i++) {
        bufferArray[i] = my_malloc(BUF_SIZE);
        my_memset(bufferArray[i], i, BUF_SIZE);
        CU_ASSERT(LinkedList_Size(testee) == i);
        LinkedList_Add(testee, bufferArray[i]);
    }
    for (int j = ARRAY_SIZE - 1; j >= 0; j--) {
        CU_ASSERT(LinkedList_Size(testee) == j + 1);
        LinkedList_Remove(testee, bufferArray[j]);
        my_free(bufferArray[j]);
    }

    for (int i = 0; i < ARRAY_SIZE; i++) {
        bufferArray[i] = my_malloc(BUF_SIZE);
        my_memset(bufferArray[i], i, BUF_SIZE);
        CU_ASSERT(LinkedList_Size(testee) == i);
        LinkedList_Add(testee, bufferArray[i]);
    }
    for (int j = 0; j < ARRAY_SIZE; j++) {
        CU_ASSERT(LinkedList_Size(testee) == ARRAY_SIZE - j);
        char *target = j == ARRAY_SIZE - 1 ? bufferArray[0] : bufferArray[j+1];
        LinkedList_Remove(testee, target);
        my_free(target);
    }

    CU_ASSERT(LinkedList_Size(testee) == 0);

    LinkedList_Delete(testee);

}

void linkedlist_add_test() {
    CU_pSuite linkedList = CU_add_suite("LinkedList", NULL, NULL);
    CU_add_test(linkedList, "add_and_next", linkedlist_add_and_next);
    CU_add_test(linkedList, "linkedlist_add_and_remove", linkedlist_add_and_remove);

}