#pragma once
#include <stdint.h>

#include "config.h"
#include "task.h"

#include "../../ds/circular_queue.h"
#include "../loader/formats/elfloader.h"

#include "arguments.h"

#define KEYBOARD_BUFFER_EMPTY 127

namespace process
{
    enum file_type_t {
        ELF,
        BINARY
    };

    struct allocation_data_t 
    {
        void* address;
        int size;
    };

    using process_id_t = uint32_t;
    
    struct keyboard_buffer_t 
    {
        char buffer[KEYBOARD_BUFFER_SIZE];
        int head;
        int tail;
    };

    struct process_t
    {
        process_t();

        process_id_t id;
        char filename[MAX_PATH];
        file_type_t file_type;

        union
        {
            void* program_ptr;
            elf32::file_t elf_file;
        };

        uint32_t program_size;
        void* stack_ptr;
        task::task_t* task;
        allocation_data_t allocations[PROGRAM_MAX_ALLOCATIONS];
        circular_queue_t<char, KEYBOARD_BUFFER_SIZE> keyboard;
        arguments_t args;
    };

    void init();

    process_id_t load(const process::arguments_t& args);
    process_id_t current();

    int _switch(process_id_t pid);

    void* malloc(process::process_id_t pid, size_t size);
    void  free(process::process_id_t pid, void* ptr);

    process_t* get(process::process_id_t pid);
    process::process_id_t next();
    
    void terminate(process::process_id_t pid);

    char keyboard_pop();
}
