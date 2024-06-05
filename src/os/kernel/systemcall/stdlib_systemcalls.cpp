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
    void* command = (void*)task::get_stack_item(task::current(), 0);
    static char kernel_buffer[MAX_LINE];

    memset(kernel_buffer, 0, sizeof(kernel_buffer));
    
    kernel_buffer[0] = '0';
    kernel_buffer[1] = ':';
    kernel_buffer[2] = '/';
    
    task::copy_string_from(task::current(), command, kernel_buffer + 3, sizeof(kernel_buffer));
    
    //system_arguments_t args;
    const char* filename = kernel_buffer;
    terminal::printf("%s\n", filename);
    
    process::process_id_t pid = process::load(filename, nullptr);
    
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
    terminal::printf("exit() called on %s\n", process::get(task::current()->pid)->filename);
    
    process::terminate(task::current()->pid);

    if (task::has_next()) {
        task::_switch(task::next());
    }
        
 
    return 0;
}