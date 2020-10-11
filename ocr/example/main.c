#include <stdio.h>

#include "utils/error.h"

int main() {
    printf("%s\n", format_last_error(SUCCESS));
    return 0;
}