#pragma once

#include "stdint.h"

enum class color_t 
{
    BLACK,
    BLUE,
    GREEN,
    CYAN,
    RED,
    MAGENTA,
    BROWN,
    LIGHT_GRAY,
    DARK_GRAY,
    LIGHT_BLUE,
    LIGHT_GREEN,
    LIGHT_CYAN,
    LIGHT_RED,
    LIGHT_MAGENTA,
    YELLOW,
    WHITE
};

namespace terminal
{
    void init();
    
    void printf(const char* format, ...);
    void print(void* addr, color_t background=color_t::BLACK, color_t forground=color_t::WHITE);    
    void print(const char* string, color_t background=color_t::BLACK, color_t forground=color_t::WHITE);
    void print(char ch, color_t background=color_t::BLACK, color_t forground=color_t::WHITE);
    void print(uint32_t n, uint8_t base=10, color_t background=color_t::BLACK, color_t forground=color_t::WHITE);
    void print(int32_t n, uint8_t base=10, color_t background=color_t::BLACK, color_t forground=color_t::WHITE);
}
