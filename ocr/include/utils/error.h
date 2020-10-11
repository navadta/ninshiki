#ifndef H_ERROR
#define H_ERROR

typedef enum error {
    SUCCESS,
    NOT_FOUND,
    IO_ERROR,
    ALLOCATION_FAILED,
    INDEX_OUT_OF_BOUNDS,
    NOT_HANDLED,
    ERRORS_COUNT
} ERROR;

const char *get_error_string(ERROR error);

void set_last_error_message(ERROR error, const char *message);

const char *get_last_error_message(ERROR error);

const char *format_last_error(ERROR error);

#endif