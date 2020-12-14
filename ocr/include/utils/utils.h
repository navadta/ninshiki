#include "utils/matrix.h"

#ifndef H_UTILS
#define H_UTILS

#define FREE(resource, func)              \
    if (resource != NULL) func(resource); \
    resource = NULL;

#define CHARSET                  \
    "0123456789"                 \
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ" \
    "abcdefghijklmnopqrstuvwxyz" \
    "-.,?"
#define CHARSET_LENGTH 66

const char *concat2(const char *s1, const char *s2);
const char *concat3(const char *s1, const char *s2, const char *s3);

unsigned char char_index(char c);
ERROR char_to_matrix(char c, MATRIX *matrix);

#endif