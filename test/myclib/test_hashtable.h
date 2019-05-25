//
// Created by yoshinori on 16/02/03.
//

#ifndef MYKVS_TEST_HASHTABLE_H
#define MYKVS_TEST_HASHTABLE_H

#include <CUnit/CUnit.h>
#include <CUnit/Console.h>
#include <myclib/myclib.h>
#include <pthread.h>
#include <unistd.h>
#include <pal/pal.h>

#include "myclib/hashtable.h"


void hashtable_set_get(void) {

    int num1 = 12345;
    int num2 = 23456;
    int num3 = 34567;

    HashTable *ht = HashTable_New(1);
    HashTable_Set(ht, (char*)"abc", 3, "ABC", 3, 0);
    HashTable_Set(ht, (char*)"abb", 3, "ABB", 3, 0);
    HashTable_Set(ht, (char*)"abcd", 4, "ABCD", 4, 0);
    HashTable_Set(ht, (char*)"abcde", 5, "ABCDE", 5, 0);
    HashTable_Set(ht, (char*)"ac", 2, "AC", 2, 0);
    HashTable_Set(ht, (char*)"azazazazazaz", 12, "AZAZAZAZAZAZ", 12, 0);
    HashTable_Set(ht, (char*)"bbbbb", 5, "BBBBB", 5, 0);
    HashTable_Set(ht, (char*)"num1", 4, &num1, 1, 0);
    HashTable_Set(ht, (char*) "num2", 4, &num2, 1, 0);
    HashTable_Set(ht, (char*)"num3", 4, &num3, 1, 0);
    HashTable_Set(ht, (char*)"a", 1, "A", 1, 0);
    HashTable_Set(ht, (char*)"あいうえお", my_strlen("あいうえお"), "アイウエオ", my_strlen("アイウエオ"), 0);
    DictionaryItem *item = HashTable_Get(ht, (char*)"abc", 3);
    CU_ASSERT(my_memcmp(item->value, "ABC", item->valuelen) == 0);
    HashTable_Return(ht, item);
    item = HashTable_Get(ht, (char*)"abb", 3);
    CU_ASSERT(my_memcmp(item->value, "ABB", item->valuelen) == 0);
    HashTable_Return(ht, item);
    item = HashTable_Get(ht, (char*)"abcd", 4);
    CU_ASSERT(my_memcmp(item->value, "ABCD", item->valuelen) == 0);
    HashTable_Return(ht, item);
    item = HashTable_Get(ht, (char*)"abcde", 5);
    CU_ASSERT(my_memcmp(item->value, "ABCDE", item->valuelen) == 0);
    HashTable_Return(ht, item);
    item = HashTable_Get(ht, (char*)"ac", 2);
    CU_ASSERT(my_memcmp(item->value, "AC", item->valuelen) == 0);
    HashTable_Return(ht, item);
    item = HashTable_Get(ht, (char*)"azazazazazaz", 12);
    CU_ASSERT(my_memcmp(item->value, "AZAZAZAZAZAZ", item->valuelen) == 0);
    HashTable_Return(ht, item);
    item = HashTable_Get(ht, (char*)"bbbbb", 5);
    CU_ASSERT(my_memcmp(item->value, "BBBBB", item->valuelen) == 0);
    HashTable_Return(ht, item);
    item = HashTable_Get(ht, (char*)"num1", 4);
    CU_ASSERT(my_memcmp(item->value, &num1, item->valuelen) == 0);
    HashTable_Return(ht, item);
    item = HashTable_Get(ht, (char*)"num2", 4);
    CU_ASSERT(my_memcmp(item->value, &num2, item->valuelen) == 0);
    HashTable_Return(ht, item);
    item = HashTable_Get(ht, (char*)"num3", 4);
    CU_ASSERT(my_memcmp(item->value, &num3, item->valuelen) == 0);
    HashTable_Return(ht, item);
    item = HashTable_Get(ht, (char*)"a", 1);
    CU_ASSERT(my_memcmp(item->value, "A", item->valuelen) == 0);
    HashTable_Return(ht, item);
    item = HashTable_Get(ht, (char*)"あいうえお", my_strlen("あいうえお"));
    CU_ASSERT(my_memcmp(item->value, "アイウエオ", item->valuelen) == 0);
    HashTable_Return(ht, item);
    CU_ASSERT(HashTable_Remove(ht, (char*)"あいうえお", my_strlen("あいうえお")));
    CU_ASSERT(!HashTable_Remove(ht, (char*)"あいうえお", my_strlen("あいうえお")));

    HashTable_Delete(ht);
}


void hashtable_set_get_binary(void) {

    HashTable *ht = HashTable_New(1);
    unsigned char data[256];
    for (int i = 0; i < 256; i++) {
        data[i] = i;
    }
    HashTable_Set(ht, "abc", 3, data, 256, 0);
    DictionaryItem *item = HashTable_Get(ht, (char*)"abc", 3);
    CU_ASSERT(item->valuelen == 256);
    bool allok = true;
    for (int i = 0; i < item->valuelen; i++) {
        if (item->value[i] != (char)data[i]) {
            allok = false;
            break;
        }
    }
    CU_ASSERT(allok);
    HashTable_Return(ht, item);
    HashTable_Delete(ht);
}



void hashtable_overwrite(void) {

    HashTable *ht = HashTable_New(2);
    HashTable_Set(ht, (char*)"abc", 3, "ABC", 3, 0);
    HashTable_Set(ht, (char*)"abb", 3, "ABB", 3, 0);
    HashTable_Set(ht, (char*)"abcd", 4, "ABCD", 4, 0);
    HashTable_Set(ht, (char*)"abcde", 5, "ABCDE", 5, 0);
    DictionaryItem *item = HashTable_Get(ht, (char*)"abc", 3);
    CU_ASSERT(my_memcmp(item->value, "ABC", item->valuelen) == 0);
    HashTable_Return(ht, item);
    item = HashTable_Get(ht, (char*)"abb", 3);
    CU_ASSERT(my_memcmp(item->value, "ABB", item->valuelen) == 0);
    HashTable_Return(ht, item);
    item = HashTable_Get(ht, (char*)"abcd", 4);
    CU_ASSERT(my_memcmp(item->value, "ABCD", item->valuelen) == 0);
    HashTable_Return(ht, item);
    item = HashTable_Get(ht, (char*)"abcde", 5);
    CU_ASSERT(my_memcmp(item->value, "ABCDE", item->valuelen) == 0);
    HashTable_Return(ht, item);

    HashTable_Set(ht, (char*)"abc", 3, "abc", 3, 0);
    HashTable_Set(ht, (char*)"abb", 3, "abb", 3, 0);
    HashTable_Set(ht, (char*)"abcd", 4, "abcd", 4, 0);
    HashTable_Set(ht, (char*)"abcde", 5, "abcde", 5, 0);

    item = HashTable_Get(ht, (char*)"abc", 3);
    CU_ASSERT(my_memcmp(item->value, "abc", item->valuelen) == 0);
    HashTable_Return(ht, item);
    item = HashTable_Get(ht, (char*)"abb", 3);
    CU_ASSERT(my_memcmp(item->value, "abb", item->valuelen) == 0);
    HashTable_Return(ht, item);
    item = HashTable_Get(ht, (char*)"abcd", 4);
    CU_ASSERT(my_memcmp(item->value, "abcd", item->valuelen) == 0);
    HashTable_Return(ht, item);
    item = HashTable_Get(ht, (char*)"abcde", 5);
    CU_ASSERT(my_memcmp(item->value, "abcde", item->valuelen) == 0);
    HashTable_Return(ht, item);

    HashTable_Delete(ht);
}


void hashtable_expire(void) {
    long long now = my_time(NULL);
    long long tomorrow = now + 86400;
    HashTable *ht = HashTable_New(1);
    HashTable_Set(ht, (char*)"abc", 3, "ABC", 3, 0);
    HashTable_Set(ht, (char*)"abb", 3, "ABB", 3, now);
    HashTable_Set(ht, (char*)"abcd", 4, "ABCD", 4, tomorrow);
    HashTable_Set(ht, (char*)"abcde", 5, "ABCDE", 5, 0);
    sleep(1);
    DictionaryItem *item = HashTable_Get(ht, (char*)"abc", 3);
    CU_ASSERT(my_memcmp(item->value, "ABC", item->valuelen) == 0);
    HashTable_Return(ht, item);
    item = HashTable_Get(ht, (char*)"abb", 3);
    CU_ASSERT(item == NULL);
    item = HashTable_Get(ht, (char*)"abcd", 4);
    CU_ASSERT(my_memcmp(item->value, "ABCD", item->valuelen) == 0);
    HashTable_Return(ht, item);
    item = HashTable_Get(ht, (char*)"abcde", 5);
    CU_ASSERT(my_memcmp(item->value, "ABCDE", item->valuelen) == 0);
    HashTable_Return(ht, item);

    HashTable_Delete(ht);
}


void hashtable_multiple_get(void) {
    HashTable *ht = HashTable_New(1);
    CU_ASSERT(HashTable_Set(ht, (char*)"abc", 3, "ABC", 3, 0));
    DictionaryItem *item = HashTable_Get(ht, "abc", 3);
    CU_ASSERT(my_memcmp(item->value, "ABC", item->valuelen) == 0);
    DictionaryItem *item2 = HashTable_Get(ht, "abc", 3);
    CU_ASSERT(item == item2);
    CU_ASSERT(item->refCnt == 3);
    CU_ASSERT(HashTable_Set(ht, (char*)"abc", 3, "NEW", 3, 0));
    CU_ASSERT(item->refCnt == 2);
    HashTable_Return(ht, item);
    CU_ASSERT(item->refCnt == 1);
    HashTable_Return(ht, item);
    CU_ASSERT(HashTable_Set(ht, (char*)"abc", 3, NULL, 0, 0));
    HashTable_Delete(ht);
}

#define THREAD_COMMON_KEY "0123456789"
#define VALUELEN 100
#define WRITE_COUNT 1000000
#define READ_COUNT 1000000
void *hashtable_multi_thread_1(void *_param) {
    int* status = my_malloc(sizeof(int));
    *status = 0;
    HashTable* ht = (HashTable*)_param;
    unsigned int keylen = my_strlen(THREAD_COMMON_KEY);
    char* value = my_malloc(VALUELEN);
    my_memset(value, 'A', VALUELEN);
    for (int i = 0; i < WRITE_COUNT; i++) {
        if (!HashTable_Set(ht, THREAD_COMMON_KEY, keylen, value, VALUELEN, 0)) {
            *status = -1;
            break;
        }
    }
    my_free(value);
    return status;
}


void *hashtable_multi_thread_2(void *_param) {
    int* status = my_malloc(sizeof(int));
    *status = 0;
    HashTable* ht = (HashTable*)_param;
    unsigned int keylen = my_strlen(THREAD_COMMON_KEY);
    char* value = my_malloc(VALUELEN);
    my_memset(value, 'B', VALUELEN);
    for (int i = 0; i < WRITE_COUNT; i++) {
        if (!HashTable_Set(ht, THREAD_COMMON_KEY, keylen, value, VALUELEN, 0)) {
            *status = -1;
            break;
        }
    }
    my_free(value);
    return status;
}

void *hashtable_multi_thread_3(void *_param) {
    int* status = my_malloc(sizeof(int));
    *status = 0;
    HashTable* ht = (HashTable*)_param;
    unsigned int keylen = my_strlen(THREAD_COMMON_KEY);
    char* expectedValue1 = my_malloc(VALUELEN);
    my_memset(expectedValue1, 'A', VALUELEN);
    char* expectedValue2 = my_malloc(VALUELEN);
    my_memset(expectedValue2, 'B', VALUELEN);

    bool written = false;
    for (int i = 0; i < READ_COUNT; i++) {
        DictionaryItem* item = HashTable_Get(ht, THREAD_COMMON_KEY, keylen);
        if (written && !item) {
            // 書き込まれた後なのにNULLが返った場合はFail
            *status = -1;
        }
        if (item) {
            written = true;
            if(my_memcmp(item->value, expectedValue1, VALUELEN) != 0 && my_memcmp(item->value, expectedValue2, VALUELEN) != 0) {
                *status = -1;
            };
            HashTable_Return(ht, item);
        }
        if (*status != 0) {
            break;
        }
    }
    my_free(expectedValue1);
    my_free(expectedValue2);
    return status;
}

void hashtable_multi_thread(void) {

    unsigned int keylen = my_strlen(THREAD_COMMON_KEY);
    HashTable *ht = HashTable_New(1);

    char* expectedValue1 = my_malloc(VALUELEN);
    my_memset(expectedValue1, 'A', VALUELEN);
    char* expectedValue2 = my_malloc(VALUELEN);
    my_memset(expectedValue2, 'B', VALUELEN);

    pthread_t thread1;
    pthread_t thread2;
    pthread_t thread3;
    pthread_create(&thread1, NULL, hashtable_multi_thread_1, (void *) ht);
    pthread_create(&thread2, NULL, hashtable_multi_thread_2, (void *) ht);
    pthread_create(&thread3, NULL, hashtable_multi_thread_3, (void *) ht);
    bool written = false;
    for (int i = 0; i < READ_COUNT; i++) {
        DictionaryItem* item = HashTable_Get(ht, THREAD_COMMON_KEY, keylen);
        if (written && !item) {
            // 書き込まれた後なのにNULLが返った場合はFail
            CU_ASSERT(false);
            break;
        }
        if (item) {
            written = true;
            CU_ASSERT(my_memcmp(item->value, expectedValue1, VALUELEN) == 0 || my_memcmp(item->value, expectedValue2, VALUELEN) == 0);
            HashTable_Return(ht, item);
        }
    }
    int* status;
    pthread_join(thread1, (void**)&status);
    CU_ASSERT(*status == 0);
    my_free(status);
    pthread_join(thread2, (void**)&status);
    CU_ASSERT(*status == 0);
    my_free(status);
    pthread_join(thread3, (void**)&status);
    CU_ASSERT(*status == 0);
    my_free(status);
    CU_ASSERT(HashTable_Set(ht, THREAD_COMMON_KEY, my_strlen(THREAD_COMMON_KEY), NULL, 0, 0));

    HashTable_Delete(ht);
    my_free(expectedValue1);
    my_free(expectedValue2);
}

void hashtable_init_cap_2(void) {

    int num1 = 12345;
    int num2 = 23456;
    int num3 = 34567;

    HashTable *ht = HashTable_New(2);
    HashTable_Set(ht, (char*)"abc", 3, "ABC", 3, 0);
    HashTable_Set(ht, (char*)"abb", 3, "ABB", 3, 0);
    HashTable_Set(ht, (char*)"abcd", 4, "ABCD", 4, 0);
    HashTable_Set(ht, (char*)"abcde", 5, "ABCDE", 5, 0);
    HashTable_Set(ht, (char*)"ac", 2, "AC", 2, 0);
    HashTable_Set(ht, (char*)"azazazazazaz", 12, "AZAZAZAZAZAZ", 12, 0);
    HashTable_Set(ht, (char*)"bbbbb", 5, "BBBBB", 5, 0);
    HashTable_Set(ht, (char*)"num1", 4, &num1, 1, 0);
    HashTable_Set(ht, (char*) "num2", 4, &num2, 1, 0);
    HashTable_Set(ht, (char*)"num3", 4, &num3, 1, 0);
    HashTable_Set(ht, (char*)"a", 1, "A", 1, 0);
    HashTable_Set(ht, (char*)"あいうえお", my_strlen("あいうえお"), "アイウエオ", my_strlen("アイウエオ"), 0);
    DictionaryItem *item = HashTable_Get(ht, (char*)"abc", 3);
    CU_ASSERT(my_memcmp(item->value, "ABC", item->valuelen) == 0);
    HashTable_Return(ht, item);
    item = HashTable_Get(ht, (char*)"abb", 3);
    CU_ASSERT(my_memcmp(item->value, "ABB", item->valuelen) == 0);
    HashTable_Return(ht, item);
    item = HashTable_Get(ht, (char*)"abcd", 4);
    CU_ASSERT(my_memcmp(item->value, "ABCD", item->valuelen) == 0);
    HashTable_Return(ht, item);
    item = HashTable_Get(ht, (char*)"abcde", 5);
    CU_ASSERT(my_memcmp(item->value, "ABCDE", item->valuelen) == 0);
    HashTable_Return(ht, item);
    item = HashTable_Get(ht, (char*)"ac", 2);
    CU_ASSERT(my_memcmp(item->value, "AC", item->valuelen) == 0);
    HashTable_Return(ht, item);
    item = HashTable_Get(ht, (char*)"azazazazazaz", 12);
    CU_ASSERT(my_memcmp(item->value, "AZAZAZAZAZAZ", item->valuelen) == 0);
    HashTable_Return(ht, item);
    item = HashTable_Get(ht, (char*)"bbbbb", 5);
    CU_ASSERT(my_memcmp(item->value, "BBBBB", item->valuelen) == 0);
    HashTable_Return(ht, item);
    item = HashTable_Get(ht, (char*)"num1", 4);
    CU_ASSERT(my_memcmp(item->value, &num1, item->valuelen) == 0);
    HashTable_Return(ht, item);
    item = HashTable_Get(ht, (char*)"num2", 4);
    CU_ASSERT(my_memcmp(item->value, &num2, item->valuelen) == 0);
    HashTable_Return(ht, item);
    item = HashTable_Get(ht, (char*)"num3", 4);
    CU_ASSERT(my_memcmp(item->value, &num3, item->valuelen) == 0);
    HashTable_Return(ht, item);
    item = HashTable_Get(ht, (char*)"a", 1);
    CU_ASSERT(my_memcmp(item->value, "A", item->valuelen) == 0);
    HashTable_Return(ht, item);
    item = HashTable_Get(ht, (char*)"あいうえお", my_strlen("あいうえお"));
    CU_ASSERT(my_memcmp(item->value, "アイウエオ", item->valuelen) == 0);
    HashTable_Return(ht, item);
    HashTable_Delete(ht);
}

char* getRandomString(unsigned int size) {
    char* str = my_malloc(size + 1);
    for (int i = 0; i < size; i++) {
        str[i] = 'a' + rand() % 26;
    }
    str[size] = 0;
    return str;
}


void hashtable_remove_expired(void) {

    long long now = my_time(NULL);
    long long oneSecond = now + 1;
    long long twoSecond = oneSecond + 1;

    HashTable *ht = HashTable_New(2);
    unsigned long long int start = HashTable_GetMemoryUsage(ht);
    CU_ASSERT(start == 0);
    char* str1 = getRandomString(4096);
    char* str2 = getRandomString(4096);
    char* str3 = getRandomString(4096);
    HashTable_Set(ht, (char*)"abc", 3, str1, my_strlen(str1), oneSecond);
    HashTable_Set(ht, (char*)"abb", 3, str2, my_strlen(str2), now);
    HashTable_Set(ht, (char*)"abcd", 4, str3, my_strlen(str3), 0);

    unsigned long long int max = HashTable_GetMemoryUsage(ht);
    CU_ASSERT(!HashTable_RemoveExpired(ht, now));
    CU_ASSERT(HashTable_GetMemoryUsage(ht) == max);
    CU_ASSERT(HashTable_RemoveExpired(ht, oneSecond));
    CU_ASSERT(HashTable_GetMemoryUsage(ht) < max);
    CU_ASSERT(!HashTable_RemoveExpired(ht, now));
    CU_ASSERT(!HashTable_RemoveExpired(ht, oneSecond));
    CU_ASSERT(HashTable_RemoveExpired(ht, twoSecond));
    CU_ASSERT(!HashTable_RemoveExpired(ht, twoSecond));
    CU_ASSERT(HashTable_GetMemoryUsage(ht) > start);

    CU_ASSERT(HashTable_Get(ht, "abc", 3) == NULL);
    CU_ASSERT(HashTable_Get(ht, "abb", 3) == NULL);
    DictionaryItem* item = HashTable_Get(ht, "abcd", 4);
    CU_ASSERT(strncmp(item->value, str3, my_strlen(str3)) == 0);
    HashTable_Return(ht, item);

    HashTable_Delete(ht);

}

void hashtable_evict(void) {

    long long now = my_time(NULL);
    long long tomorrow = now + 86400;
    HashTable *ht = HashTable_New(2);
    unsigned long long int start = HashTable_GetMemoryUsage(ht);
    CU_ASSERT(start == 0);
    char* str1 = getRandomString(4096);
    char* str2 = getRandomString(4096);
    char* str3 = getRandomString(4096);
    HashTable_Set(ht, (char*)"abc", 3, str1, my_strlen(str1), tomorrow);
    HashTable_Set(ht, (char*)"abb", 3, str2, my_strlen(str2), tomorrow);
    HashTable_Set(ht, (char*)"abcd", 4, str3, my_strlen(str3), 0);
    unsigned long long int  max = HashTable_GetMemoryUsage(ht);
    CU_ASSERT(HashTable_Evict(ht, false));
    unsigned long long int  evicted = HashTable_GetMemoryUsage(ht);
    CU_ASSERT(HashTable_Evict(ht, false));
    unsigned long long int  evictedTwice = HashTable_GetMemoryUsage(ht);
    CU_ASSERT(!HashTable_Evict(ht, false));
    CU_ASSERT(start < max);
    CU_ASSERT(evicted < max);
    CU_ASSERT(evictedTwice < evicted);
    CU_ASSERT(HashTable_Get(ht, "abc", 3) == NULL);
    CU_ASSERT(HashTable_Get(ht, "abb", 3) == NULL);
    DictionaryItem* item = HashTable_Get(ht, "abcd", 4);
    CU_ASSERT(strncmp(item->value, str3, my_strlen(str3)) == 0);
    HashTable_Return(ht, item);
    CU_ASSERT(HashTable_Evict(ht, true));
    CU_ASSERT(!HashTable_Get(ht, "abcd", 4));
    unsigned long evictedAll = HashTable_GetMemoryUsage(ht);
    CU_ASSERT(evictedAll == 0);

    HashTable_Delete(ht);
}


void hashtable_add_test() {
    CU_pSuite hashtablesuite = CU_add_suite("HashTable", NULL, NULL);
    CU_add_test(hashtablesuite, "hashtable_set_get", hashtable_set_get);
    CU_add_test(hashtablesuite, "hashtable_set_get_binary", hashtable_set_get_binary);
    CU_add_test(hashtablesuite, "hashtable_overwrite", hashtable_overwrite);
    CU_add_test(hashtablesuite, "hashtable_expire", hashtable_expire);
    CU_add_test(hashtablesuite, "hashtable_multiple_get", hashtable_multiple_get);
    CU_add_test(hashtablesuite, "hashtable_multi_thread", hashtable_multi_thread);
    CU_add_test(hashtablesuite, "hashtable_init_cap_2", hashtable_init_cap_2);
    CU_add_test(hashtablesuite, "hashtable_evict", hashtable_evict);
    CU_add_test(hashtablesuite, "hashtable_remove_expired", hashtable_remove_expired);

}

#endif //MYKVS_TEST_HASHTABLE_H
