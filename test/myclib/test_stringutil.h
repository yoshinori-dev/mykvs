//
// Created by yoshinori on 15/12/05.
//

#ifndef MYKVS_TEST_STRINGUTIL_H
#define MYKVS_TEST_STRINGUTIL_H

#include <CUnit/CUnit.h>
#include <CUnit/Console.h>

#include "myclib/stringutil.h"
#include "myclib/myclib.h"





void stringutil_tokenize_some_strs(void) {
    char *str1 = "AAA 0 BBB 1 CCC 12345678901234567890 DDD 333\r\n";
    Token tokens[8];
    int ntoken = StringUtil_Tokenize(str1, my_strlen(str1), ' ', tokens, 8);
    CU_ASSERT(ntoken == 8);
    CU_ASSERT(strncmp(tokens[0].token, "AAA", 3) == 0);
    CU_ASSERT(tokens[0].len == 3);
    CU_ASSERT(strncmp(tokens[1].token, "0", 1) == 0);
    CU_ASSERT(tokens[1].len == 1);
    CU_ASSERT(strncmp(tokens[2].token, "BBB", 3) == 0);
    CU_ASSERT(tokens[2].len == 3);
    CU_ASSERT(strncmp(tokens[3].token, "1", 1) == 0);
    CU_ASSERT(tokens[3].len == 1);
    CU_ASSERT(strncmp(tokens[4].token, "CCC", 3) == 0);
    CU_ASSERT(tokens[4].len == 3);
    CU_ASSERT(strncmp(tokens[5].token, "12345678901234567890", 20) == 0);
    CU_ASSERT(tokens[5].len == 20);
    CU_ASSERT(strncmp(tokens[6].token, "DDD", 3) == 0);
    CU_ASSERT(tokens[6].len == 3);
    CU_ASSERT(strncmp(tokens[7].token, "333\r\n", 3) == 0);
    CU_ASSERT(tokens[7].len == 5);
    char *str2 = "   AAA       BBB\r\n1 0 ";
    ntoken = StringUtil_Tokenize(str2, strlen(str2), ' ', tokens, 5);
    CU_ASSERT(ntoken == 3);
    CU_ASSERT(strncmp(tokens[0].token, "AAA", 3) == 0);
    CU_ASSERT(tokens[0].len == 3);
    CU_ASSERT(strncmp(tokens[1].token, "BBB\r\n1", 6) == 0);
    CU_ASSERT(tokens[1].len == 6);
    CU_ASSERT(strncmp(tokens[2].token, "0", 1) == 0);
    CU_ASSERT(tokens[2].len == 1);
    char *str3 = "    ";
    ntoken = StringUtil_Tokenize(str3, strlen(str3), ' ', tokens, 5);
    CU_ASSERT(ntoken == 0);

}


void stringutil_add_test() {
    CU_pSuite stringutil = CU_add_suite("StringUtil", NULL, NULL);
    CU_add_test(stringutil, "stringutil_tokenize_some_strs", stringutil_tokenize_some_strs);
}
#endif //MYKVS_TEST_STRINGUTIL_H
