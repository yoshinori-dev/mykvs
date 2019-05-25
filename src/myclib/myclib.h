//
// Created by yoshinori on 15/10/03.
//

#ifndef MYPROJ_MYCLIB_H
#define MYPROJ_MYCLIB_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>

#define my_malloc(a) malloc((size_t)(a))
#define my_free(b) free((b))
#define my_realloc(a, b) realloc((a), (size_t)(b))
#define my_calloc(a, b)  calloc((a), (size_t)(b))
#define my_strlen(a)     (int)strlen((const char*)(a))
#define my_memset(a, b, c) memset((a), (b), (size_t)(c))
#define my_memcpy(a, b, c) memcpy((a), (b), (size_t)(c))
#define my_memmove(a, b, c) memmove((a), (b), (size_t)(c))
#define my_memcmp(a, b, c) memcmp((a), (b), (c))
#define my_strncpy(a, b, c) strncpy((a), (b), (size_t)(c))
#define my_memchr(a, b, c) memchr((a), (b), (size_t)(c))
#define my_strtol(a,b,c) strtol((a),(b),(c))
#define my_snprintf(a, b, ...) snprintf((a), (b), __VA_ARGS__)
#define my_time(a) (long long)time((a))
#define my_clock_gettime(a, b) clock_gettime((a), (b))

#endif //MYPROJ_MYCLIB_H
