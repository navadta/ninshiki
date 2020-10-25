#ifndef H_ERROR
#define H_ERROR

#define err_throw_msg(e, err, message)      \
    e = err;                                \
    if (e) {                                \
        set_last_error_message(e, message); \
        return e;                           \
    }

#define err_throw(e, err) \
    e = err;              \
    if (e) return e;

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