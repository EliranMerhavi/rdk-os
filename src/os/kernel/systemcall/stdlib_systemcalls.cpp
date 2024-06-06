#include "stdlib_systemcalls.h"

#include "status.h"
#include "config.h"

#include "../task/task.h"
#include "../task/process.h"

#include "os/terminal/terminal.h"

#include <stddef.h>
#include "memory.h"

void* systemcall_malloc(interrupt_frame_t* frame)
{
    size_t size = (size_t)task::get_stack_item(task::current(), 0);
    void* res = process::malloc(task::current()->pid, size);
    return res;
}

void* systemcall_free(interrupt_frame_t* frame)
{
    void* ptr = task::get_stack_item(task::current(), 0);
    process::free(task::current()->pid, ptr);
    return 0;
}

void* systemcall_system(interrupt_frame_t* frame)
{
    void* line = (void*)task::get_stack_item(task::current(), 0);
    static char kernel_line[MAX_LINE + 3];
    
    memset(kernel_line, 0, sizeof(kernel_line));

    task::read(task::current(), line, kernel_line + 3, sizeof(kernel_line) - 3);

    kernel_line[0] = '0';
    kernel_line[1] = ':';
    kernel_line[2] = '/';

    char arguemnt[MAX_ARG_SIZE + 1];

    process::arguments_t args;

    char* it1 = kernel_line;  

    while (*it1) {
        memset(arguemnt, 0, sizeof(arguemnt));
        char* it2 = arguemnt;

        while (*it1 && *it1 != ' ') {
            *it2     = *it1;
            it2++;
            it1++;
        }

        *it2 ='\0';
        args.add(arguemnt);

        while (*it1 == ' ')
            it1++;
    }
        
    process::process_id_t pid = process::load(args);
   
    if (IS_ERROR(pid)) { 
        return (void*)pid;
    }
    
    process::process_t* _process = process::get(pid);

    task::_switch(_process->task);
    task::_return(_process->task);
    
    return 0;
}

void* systemcall_exit(interrupt_frame_t* frame)
{
    process::terminate(task::current()->pid);

    if (task::has_next()) {
        task::_switch(task::next());
        task::_return(task::next());
    }
        
    return 0;
}