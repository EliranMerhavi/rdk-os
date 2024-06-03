#pragma once
#include "memory.h"
#include <stddef.h>

size_t strlen(const char* str);
size_t strnlen(const char* str, size_t maxlen);

char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, size_t n);

int strncmp(const char* s1, const char* s2, size_t n);

int isdigit(char ch);
int isupper(char ch);
int islower(char ch);
int isalpha(char ch);

char toupper(char ch);
char tolower(char ch);

