#include "stdlib_systemcalls.h"
#include "status.h"

#include "../task/task.h"
#include "../task/process.h"

#include "os/terminal/terminal.h"

#include <stddef.h>

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
    /*const char* command  = (const char*)task::get_stack_item(task::current(), 0);
    system_arguments_t args;
    const char* filename = args.argv[0];

    process::process_id_t pid = process::load(filename, &args);

    if (IS_ERROR(pid)) { 
        return (void*)pid;
    }

    task::_switch(process->task);
    task::_return(&process->task->registers);
    */
    /*  TODO:
        process::process_id_t pid = process::load(filename);
        process::inject_args(pid, command->argc, commnad->argv);
        if (IS_ERROR(pid)) { 
            terminal::printf("error: %d", pid);
            panic("failed to load 0:/blank.elf");
        }

        task::switch(process->task);
        task::return(&process->task->registers);

        return status;            
    */

    return 0;
}

void* systemcall_exit(interrupt_frame_t* frame)
{
    process::process_id_t pid = task::current()->pid;
    process::terminate(pid);
    task::_switch(task::current());
    return 0;
}