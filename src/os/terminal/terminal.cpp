#include "terminal.h"
#include "stdint.h"
#include "assert.h"
#include <stdarg.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

namespace terminal
{
    uint16_t* addr; 
    uint16_t x, y;

    void update_cursor();
    void backspace();
}

void terminal::init()
{
    addr = (uint16_t*)0xb8000;
    x = y = 0;

    for (uint16_t i = 0; i < VGA_HEIGHT; i++) {
        for (uint16_t j = 0; j < VGA_WIDTH; j++) {
            *(addr + i * VGA_WIDTH + j) = 0;
        }
    }
}

void terminal::printf(const char* format, ...)
{
    char ch;
    va_list args;

    va_start(args, format);

    while ((ch = *(format++)))
    {
        if (ch != '%')
        {
            print(ch);
            continue;
        }

        ch = *(format++);
        
        switch (ch)
        {
            case 'c':
            {
                char arg = va_arg(args, int32_t);
                print(arg);
                break;
            }
            case 'u':
            {
                uint32_t arg = va_arg(args, uint32_t);
                print(arg);
                break;
            }
            case 'd':
            {
                int32_t arg = va_arg(args, int32_t);
                print(arg);
                break;
            }
            case 'x':
            {
                uint32_t arg = va_arg(args, uint32_t);
                print(arg, 16);
                break;
            }
            case 'b':
            {
                uint32_t arg = va_arg(args, uint32_t);
                print(arg, 2);
                break;
            }
            case 'p':
            {
                const void* arg = va_arg(args,  const void*);
                print((uint32_t)arg, 16);
                break;
            }
            case 's':
            {   
                const char* arg = va_arg(args, const char *);
                print(arg);
                break;
            }
            case 'f':
            {   
                break;
            } 
            default:
                int x = 0;
                uint32_t y = 1 / x; // invalid format type
                print(y);
                break;
        }
    }
}

void terminal::update_cursor()
{
    x = (x + 1) % VGA_WIDTH;

    if (y == VGA_HEIGHT - 1 && !x) {
        // shift rows
        for (int r = 0; r < VGA_HEIGHT - 1; r++) {
            for (int c = 0; c < VGA_WIDTH; c++) {
                *(addr + r * VGA_WIDTH + c) = *(addr + (r + 1) * VGA_WIDTH + c);
            }
        }

        // clear last row 
        for (int c = 0; c < VGA_WIDTH; c++) {
            *(addr + y * VGA_WIDTH + c) = 0;
        }

        return;
    }

    y = (y + !x) % VGA_HEIGHT;
}

void terminal::backspace()
{
    if (!y && !x)
        return;
    x = (x - 1 + VGA_WIDTH) % VGA_WIDTH;
    y = (y - (x == VGA_WIDTH - 1) + VGA_HEIGHT) % VGA_HEIGHT;
   *(addr + VGA_WIDTH * y + x) = 0;
}

void terminal::print(char ch, color_t background, color_t forground)
{
    switch (ch) {
    case '\n':
        x = VGA_WIDTH - 1;
        update_cursor();
        break;
    case '\b':
        backspace();
        break;
    default:
        *(addr + VGA_WIDTH * y + x) = ch | ((uint16_t)background << 12) | ((uint16_t)forground << 8);
        update_cursor();
        break;
    }
}

void terminal::print(void* addr, color_t background, color_t forground)
{
    terminal::print((uint32_t)addr, 16, background, forground);
}

void terminal::print(const char* string, color_t background, color_t forground)
{
    while (*string) {
        terminal::print(*string, background, forground);
        string++;
    }
}

void terminal::print(uint32_t n, uint8_t base, color_t background, color_t forground)
{
    assert(1 <= base && base <= 16 && "unsupported base");
    
    static char digits[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

    uint32_t div = 1, temp = n, _base = base;

    while (temp >= _base) {
        div *= _base;
        temp /= _base;
    }

    while (div) {
        terminal::print(digits[(n / div) % base], background, forground);
        div /= _base;
    }
}

void terminal::print(int32_t n, uint8_t base, color_t background, color_t forground)
{
    if (n < 0) {
        terminal::print('-');
        n = -n;
    }
    terminal::print((uint32_t)n, base, background, forground);
}
