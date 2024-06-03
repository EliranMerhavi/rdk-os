#include "string.h"

size_t strlen(const char* str)
{
    size_t res = 0;
    while (str[res]) {
        res++;
    }
    return res;
}

size_t strnlen(const char* str, size_t maxlen)
{
    size_t res = 0;
    while (res < maxlen && str[res]) {
        res++;
    }
    return res;
}

char* strcpy(char* dest, const char* src)
{
    int i = 0;
    while (src[i]) {
        dest[i] = src[i];
        i++;
    } 
    return dest;
}

char* strncpy(char* dest, const char* src, size_t n)
{
    size_t i = 0;
    while (src[i] && i < n) {
        dest[i] = src[i];
        i++;
    } 
    return dest;
}

int strncmp(const char* s1, const char* s2, size_t n)
{ 
    while ( n && *s1 && ( *s1 == *s2 ) )
    {
        ++s1;
        ++s2;
        --n;
    }

    if (!n)
        return 0;
 
    return ( *(unsigned char *)s1 - *(unsigned char *)s2 );
}

int isdigit(char ch)
{
    return '0' <= ch && ch <= '9';
}

int isalpha(char ch) 
{
    return isupper(ch) || islower(ch);
}

int isupper(char ch)
{
    return 'A' <= ch && ch <= 'Z';
}

int islower(char ch)
{
    return 'a' <= ch && ch <= 'z';
}

char toupper(char ch)
{
    return ch ^ (islower(ch) * ('a' - 'A'));
}

char tolower(char ch)
{
    return ch ^ (isupper(ch) * ('a' - 'A'));
}