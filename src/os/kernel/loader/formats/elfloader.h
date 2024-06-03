#pragma once

#include "elf.h"
#include "config.h"

namespace elf32
{
    struct file_t 
    {
        char filename[MAX_PATH];

        int in_memory_size;
        void* elf_physical_memory;
        
        void* virtual_base_addr;
        void* virtual_end_addr;

        void* physical_base_addr;
        void* physical_end_addr;

        elf_header_t* header();
        void* physical_addr(elf32::phdr_t* phdr);
    };

    int load(const char* filename, elf32::file_t* res);
    void close(elf32::file_t* _file);
}
