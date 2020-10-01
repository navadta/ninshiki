#include <stdio.h>

#include "error.h"

int main() {
    char buffer[100];
    format_last_error(SUCCESS, buffer);
    printf("%s", buffer);
    return 0;
}