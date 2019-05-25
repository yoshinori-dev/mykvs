//
// Created by yoshinori on 15/12/05.
//

#include <stdlib.h>
#include "stringutil.h"


#define PRIME_NUMBER 31
unsigned int StringUtil_Hash(char *str, int strlen) {
    unsigned int hash = 0;
    for (int i = 0; i < strlen; i++) {
        hash += hash * PRIME_NUMBER + str[i];
    }
    return hash;
}

/**
 *
 */
int StringUtil_Tokenize(char *str, int strlen, char delimiter, Token *tokens, int maxtokenlen) {
    char *ptr = str;
    char *last = str + strlen;
    int num = 0;
    for (int tokenidx = 0; tokenidx < maxtokenlen; tokenidx++) {
        while (*ptr == delimiter && ptr < last) {
            // skip what spaces
            ptr++;
        }
        if (ptr == last) {
            break;
        }
        tokens[tokenidx].token = ptr;
        while (*ptr != delimiter && ptr < last) {
            ptr++;
        }
        tokens[tokenidx].len = ptr - tokens[tokenidx].token;
        num++;
    }
    return num;
}