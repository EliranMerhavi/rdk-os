#ifndef STDLIB_H
#define STDLIB_H
#include <stdint.h>
#include <stddef.h>

char* itoa(int i);
void* malloc(size_t size);
void free(void* ptr);

int system(const char* command);
void exit();
#endif
