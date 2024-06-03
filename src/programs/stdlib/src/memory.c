#include "memory.h"
#include "string.h"

void* memset(void* ptr, char c, size_t size) 
{
    char* ch_ptr = (char*)ptr;
    for (size_t i = 0; i < size; i++)
        ch_ptr[i] = c;
    return ptr;
}

int memcmp(const void* ptr1, const void* ptr2, size_t n)
{
    char* s1 = (char*) ptr1,
        * s2 = (char*) ptr2;

    for (size_t i = 0; i < n; i++)
    {
        if (s1[i] != s2[i])
            return (s1[i] < s2[i]) ? -1 : 1;
    }

    return 0;
}

void* memcpy(void* dest, const void* src, size_t n)
{
    char* d = (char *)dest,
        * s = (char *)src;
    
    for (size_t i = 0; i < n; i++) 
    {
        d[i] = s[i];
    }

    return dest;
}
