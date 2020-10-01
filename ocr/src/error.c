#include "error.h"

#include <stdio.h>
#include <string.h>

const char *get_error_string(int error) {
    switch (error) {
        case SUCCESS:
            return "Success";
        case NOT_FOUND:
            return "Not found";
        case ALLOCATION_FAILED:
            return "Allocation failed";
        default:
            return "Undefined Error";
    }
}

static const char *last_error_message[ERRORS_NUMBER];

void set_last_error_message(int error, const char *message) {
    last_error_message[error] = message;
}

const char *get_last_error_message(int error) {
    return last_error_message[error];
}

void format_last_error(int error, char *buffer) {
    snprintf(buffer, 100, "%s: %s", get_error_string(error),
             get_last_error_message(error));
}