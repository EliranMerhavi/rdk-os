#pragma once

#include "config.h"
#include "../memory/paging.h"
#include "../isr/isr.h"

namespace process 
{
    using process_id_t = uint32_t;
}

namespace task 
{
    struct registers_t
    {
        uint32_t edi, esi, ebp, ebx, edx, ecx, eax;
        uint32_t eip; 
        uint32_t cs;
        uint32_t flags;
        uint32_t esp;
        uint32_t ss;
    };

    struct task_t
    {
        paging::directory_t*  page_directory;
        registers_t           registers;
        process::process_id_t pid;
        task_t*               next;
        task_t*               prev;
    };
    
    void run_first();
    int switch_to_task_page(task::task_t* task);
    
    task_t* current();
    task_t* next();

    void save_current(interrupt_frame_t* frame);
    
    task_t* create(process::process_id_t p);
    int     copy_string_from(task::task_t* task, void* virtual_addr, void* physical_addr, int _max);
    void*   get_stack_item(task::task_t* task, int index);
    int     _switch(task_t* task);
    void    remove(task_t* task);
    void    free(task_t* task);
}
