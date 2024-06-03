#ifndef RDK_OS_H
#define RDK_OS_H
#include <stdint.h>
#include <stddef.h>
#include "status.h"


#define MAX_PATH 108
#define MAX_ARGS     128
#define MAX_ARG_SIZE 108


typedef enum {
    OK,
    LEADING_SPACES,
    ARGUEMNT_SIZE_EXCEEDED
} status_t;

typedef struct {
    int  argc;
    char **argv;
} arguments_t;

void print(const char *buffer);


#endif
