//
// Created by yoshinori on 15/12/05.
//

#ifndef MYKVS_STRINGUTIL_H
#define MYKVS_STRINGUTIL_H

unsigned int StringUtil_Hash(char *str, int strlen);

typedef struct Token_ {
    char *token;
    int len;
} Token;


int StringUtil_Tokenize(char *str, int strlen, char delimiter, Token *tokens, int maxtokenlen);

#endif //MYKVS_STRINGUTIL_H
