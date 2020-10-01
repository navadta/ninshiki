#ifndef H_ERROR
#define H_ERROR

#define SUCCESS           0
#define NOT_FOUND         1
#define ALLOCATION_FAILED 2

#define ERRORS_NUMBER 3

const char *get_error_string(int error);

void set_last_error_message(int error, const char *message);

const char *get_last_error_message(int error);

void format_last_error(int error, char *buffer);

#endif