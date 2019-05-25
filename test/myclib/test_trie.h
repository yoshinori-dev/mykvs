#include <CUnit/CUnit.h>
#include <CUnit/Console.h>


#include "myclib/trie.h"
#include "myclib/myclib.h"



void Trie_Set_mixed_items(void) {

    int num1 = 12345;
    int num2 = 23456;
    int num3 = 34567;

    Trie* trie = Trie_New();
    void* old;
    Trie_Set(trie, (unsigned char*)"abc", 3, "ABC", &old);
    CU_ASSERT(old == NULL);
    Trie_Set(trie, (unsigned char*)"abb", 3, "ABB", &old);
    CU_ASSERT(old == NULL);
    Trie_Set(trie, (unsigned char*)"abcd", 4, "ABCD", &old);
    CU_ASSERT(old == NULL);
    Trie_Set(trie, (unsigned char*)"abcde", 5, "ABCDE", &old);
    CU_ASSERT(old == NULL);
    Trie_Set(trie, (unsigned char*)"ac", 2, "AC", &old);
    CU_ASSERT(old == NULL);
    Trie_Set(trie, (unsigned char*)"azazazazazaz", 12, "AZAZAZAZAZAZ", &old);
    CU_ASSERT(old == NULL);
    Trie_Set(trie, (unsigned char*)"bbbbb", 5, "BBBBB", &old);
    CU_ASSERT(old == NULL);
    Trie_Set(trie, (unsigned char*)"num1", 4, &num1, &old);
    CU_ASSERT(old == NULL);
    Trie_Set(trie, (unsigned char*) "num2", 4, &num2, &old);
    CU_ASSERT(old == NULL);
    Trie_Set(trie, (unsigned char*)"num3", 4, &num3, &old);
    CU_ASSERT(old == NULL);
    Trie_Set(trie, (unsigned char*)"a", 1, "A", &old);
    CU_ASSERT(old == NULL);
    Trie_Set(trie, (unsigned char*)"あいうえお", my_strlen("あいうえお"), "アイウエオ", &old);
    CU_ASSERT(old == NULL);

    CU_ASSERT(Trie_Find(trie, (unsigned char*)"abc", 3) == "ABC");
    CU_ASSERT(Trie_Find(trie, (unsigned char*)"abb", 3) == "ABB");
    CU_ASSERT(Trie_Find(trie, (unsigned char*)"abcd", 4) == "ABCD");
    CU_ASSERT(Trie_Find(trie, (unsigned char*)"abcde", 5) == "ABCDE");
    CU_ASSERT(Trie_Find(trie, (unsigned char*)"ac", 2) == "AC");
    CU_ASSERT(Trie_Find(trie, (unsigned char*)"azazazazazaz", 12) == "AZAZAZAZAZAZ");
    CU_ASSERT(Trie_Find(trie, (unsigned char*)"bbbbb", 5) == "BBBBB");
    CU_ASSERT(Trie_Find(trie, (unsigned char*)"num1", 4) == &num1);
    CU_ASSERT(Trie_Find(trie, (unsigned char*)"num2", 4) == &num2);
    CU_ASSERT(Trie_Find(trie, (unsigned char*)"num3", 4) == &num3);
    CU_ASSERT(Trie_Find(trie, (unsigned char*)"a", 1) == "A");
    CU_ASSERT(Trie_Find(trie, (unsigned char*)"あいうえお", my_strlen("あいうえお")) == "アイウエオ");
    Trie_Delete(trie);

}

void Trie_Set_overwrite(void) {

    void* old;
    Trie* trie = Trie_New();
    Trie_Set(trie, (unsigned char*)"abc", 3, "ABC", &old);
    CU_ASSERT(old == NULL);
    CU_ASSERT(Trie_Find(trie, (unsigned char*)"abc", 3) == "ABC");

    Trie_Set(trie, (unsigned char*)"abc", 3, "ABB", &old);
    CU_ASSERT(old == "ABC");
    CU_ASSERT(Trie_Find(trie, (unsigned char*)"abc", 3) == "ABB");

    Trie_Set(trie, (unsigned char*)"abc", 3, "ZZZ", &old);
    CU_ASSERT(old == "ABB");
    CU_ASSERT(Trie_Find(trie, (unsigned char*)"abc", 3) == "ZZZ");

    Trie_Set(trie, (unsigned char*)"abc", 3, "Z", &old);
    CU_ASSERT(old == "ZZZ");
    CU_ASSERT(Trie_Find(trie, (unsigned char*)"abc", 3) == "Z");
    Trie_Delete(trie);

}
int KEY_SIZE = 16;
void trie_set_many_items(void) {
    void* old;
    Trie* trie = Trie_New();
    unsigned char firstch = '!';
    unsigned char lastch = '~';
    int key_size = KEY_SIZE;
    unsigned char key[key_size + 1];
    my_memset(key, lastch, key_size);
    key[key_size] = 0;
    for (int i = 0; i < key_size; i++) {
        for (unsigned char ch = firstch; ch < lastch; ch++) {
            key[i] = ch;
            int keylen = my_strlen(key);
            char* data  = my_malloc(keylen + 1);
            data[keylen] = 0;
            my_strncpy(data, key, keylen);
            Trie_Set(trie, key, keylen, data, &old);
        }
    }
    my_memset(key, lastch, key_size);
    key[key_size] = 0;
    for (int i = 0; i < key_size; i++) {
        for (unsigned char ch = firstch; ch < lastch; ch++) {
            key[i] = ch;
            char *data = Trie_Find(trie, key, my_strlen(key));
            CU_ASSERT(strncmp(data, key, my_strlen(key)) == 0);
            my_free(data);
        }
    }
    Trie_Delete(trie);
}



void trie_add_test() {
    CU_pSuite trie = CU_add_suite("Trie", NULL, NULL);
    CU_add_test(trie, "Trie_Set_mixed_items", Trie_Set_mixed_items);
    CU_add_test(trie, "Trie_Set_overwrite", Trie_Set_overwrite);
    CU_add_test(trie, "trie_set_many_items", trie_set_many_items);

}