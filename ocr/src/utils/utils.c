#include "utils/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils/matrix.h"

static char buffer[512];

const char *concat2(const char *s1, const char *s2) {
    const size_t len1 = strlen(s1);
    const size_t len2 = strlen(s2);
    memcpy(buffer, s1, len1);
    memcpy(buffer + len1, s2, len2 + 1);
    return buffer;
}

const char *concat3(const char *s1, const char *s2, const char *s3) {
    const size_t len1 = strlen(s1);
    const size_t len2 = strlen(s2);
    const size_t len3 = strlen(s3);
    memcpy(buffer, s1, len1);
    memcpy(buffer + len1, s2, len2);
    memcpy(buffer + len1 + len2, s3, len3 + 1);
    return buffer;
}

unsigned char char_index(char c) {
    char *charset =
        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    unsigned char position = 0;
    while (charset[position] != c && charset[position] != '\0') position++;
    return position;
}

ERROR char_to_matrix(char c, MATRIX *matrix) {
    ERROR err = SUCCESS;
    unsigned char position = char_index(c);
    if (position >= 62) return NOT_HANDLED;
    err_throw(err, matrix_init(matrix, 62, 1, NULL, NULL));
    matrix_set(matrix, position, 0, 1.0d);
    return err;
}
