#pragma once

#define MAX_PATH 108

#include "stdint.h"

namespace path
{
    struct part_t
    {
        char* val;
        part_t* next;
    };

    struct path_t 
    {
        uint32_t drive_number;
        part_t* head;
    };

    
    path_t* parse(const char* path);
    void free(path_t* path);
}

