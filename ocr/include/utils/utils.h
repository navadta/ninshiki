#ifndef H_UTILS
#define H_UTILS

#define FREE(resource, func)              \
    if (resource != NULL) func(resource); \
    resource = NULL;

const char *concat2(const char *s1, const char *s2);
const char *concat3(const char *s1, const char *s2, const char *s3);

#endif