#include "utils/error.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils/utils.h"

const char *get_error_string(ERROR error) {
    switch (error) {
        case SUCCESS:
            return "Success";
        case NOT_FOUND:
            return "Not found";
        case IO_ERROR:
            return "IO Error";
        case ALLOCATION_FAILED:
            return "Allocation failed";
        case INDEX_OUT_OF_BOUNDS:
            return "Index out of bounds";
        case NOT_HANDLED:
            return "Not handled";
        default:
            return "Undefined Error";
    }
}

static const char *last_error_message[ERRORS_COUNT];

void set_last_error_message(ERROR error, const char *message) {
    last_error_message[error] = message;
}

const char *get_last_error_message(ERROR error) {
    return last_error_message[error];
}

const char *format_last_error(ERROR error) {
    const char *error_string = get_error_string(error);
    const char *error_message = get_last_error_message(error);
    if (!error_message) error_message = "(no message)";
    return concat3(error_string, ": ", error_message);
}