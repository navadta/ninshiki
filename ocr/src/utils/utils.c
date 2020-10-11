#include "utils/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *concat2(const char *s1, const char *s2) {
    const size_t len1 = strlen(s1);
    const size_t len2 = strlen(s2);
    char *result = malloc(len1 + len2 + 1);
    if (!result) {
        return s1;
    }
    memcpy(result, s1, len1);
    memcpy(result + len1, s2, len2 + 1);
    return result;
}

const char *concat3(const char *s1, const char *s2, const char *s3) {
    const size_t len1 = strlen(s1);
    const size_t len2 = strlen(s2);
    const size_t len3 = strlen(s3);
    char *result = malloc(len1 + len2 + len3 + 1);
    if (!result) {
        return s1;
    }
    memcpy(result, s1, len1);
    memcpy(result + len1, s2, len2);
    memcpy(result + len1 + len2, s3, len3 + 1);
    return result;
}