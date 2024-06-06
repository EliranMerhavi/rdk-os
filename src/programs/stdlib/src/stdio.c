#include "stdio.h"
#include <stdarg.h>

int count;

void put_addr(const void* addr);
void put_string(const char* string);
void put_char(char ch);
void put_uint32(uint32_t n, uint8_t base);
void put_int32(int32_t n, uint8_t base);

int printf(const char* format, ...)
{
    count = 0;

    char ch;
    va_list args;

    va_start(args, format);

    while ((ch = *(format++)))
    {
        if (ch != '%')
        {
            put_char(ch);
            continue;
        }

        ch = *(format++);
        
        switch (ch)
        {
            case 'c':
            {
                char arg = va_arg(args, int32_t);
                put_char(arg);
                break;
            }
            case 'u':
            {
                uint32_t arg = va_arg(args, uint32_t);
                put_uint32(arg, 10);
                break;
            }
            case 'd':
            {
                int32_t arg = va_arg(args, int32_t);
                put_int32(arg, 10);
                break;
            }
            case 'x':
            {
                uint32_t arg = va_arg(args, uint32_t);
                put_uint32(arg, 16);
                break;
            }
            case 'b':
            {
                uint32_t arg = va_arg(args, uint32_t);
                put_uint32(arg, 2);
                break;
            }
            case 'p':
            {
                const void* arg = va_arg(args,  const void *);
                put_addr(arg);
                break;
            }
            case 's':
            {   
                const char* arg = va_arg(args, const char *);
                put_string(arg);
                break;
            }
            case 'f':
            {   
                break;
            } 
            default:
                // TODO: error or something
                break;
        }
    }   

    return count;
}

void put_addr(const void* addr)
{
    put_uint32((uint32_t)addr, 16);
}

void put_string(const char* string)
{
    while (*string) {
        put_char(*string);
        string++;
    }
}

void put_int32(int32_t n, uint8_t base)
{
    if (n < 0) {
        put_char('-');
        n = -n;
    }
    put_uint32((uint32_t)n, base);
}

void put_char(char ch) {
    putchar(ch);
    count++;
}

void put_uint32(uint32_t n, uint8_t base)
{
    static char digits[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

    uint32_t div = 1, temp = n, _base = base;

    while (temp >= _base) {
        div *= _base;
        temp /= _base;
    }

    while (div) {
        put_char(digits[(n / div) % base]);
        div /= _base;
    }
}