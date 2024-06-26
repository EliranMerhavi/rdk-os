#include "stdio_systemcalls.h"
#include "systemcall.h"

#include "../task/task.h"
#include "../task/process.h"
#include "../vfs/file.h"

#include "os/terminal/terminal.h"
#include "memory.h"


void* systemcall_getchar(interrupt_frame_t* frame)
{
    char ch = process::keyboard_pop();
    ch = (ch == KEYBOARD_BUFFER_EMPTY) ? 0 : ch;
    return (void*)((int)ch);
}

void* systemcall_putchar(interrupt_frame_t* frame)
{
    char ch = (char)(int)task::get_stack_item(task::current(), 0);
    terminal::print(ch);
    return 0;
}

/* FILE I/O STUFF */

void* systemcall_fopen(interrupt_frame_t* frame)
{
    char kernel_filename[MAX_ARG_SIZE];
    char kernel_mode[MAX_ARG_SIZE];

    void* filename  = task::get_stack_item(task::current(), 0);
    void* mode      = task::get_stack_item(task::current(), 1);
    
    memset(kernel_filename, 0, sizeof(kernel_filename));
    memset(kernel_mode, 0, sizeof(kernel_mode));
    
    task::read(task::current(), filename, kernel_filename, sizeof(kernel_filename));
    task::read(task::current(), mode, kernel_mode, sizeof(kernel_mode));

    return (void*)fopen(kernel_filename, kernel_mode);
}

void* systemcall_fread(interrupt_frame_t* frame)
{

    void* ptr            = (void*)task::get_stack_item(task::current(), 0);
    uint32_t block_size  = (uint32_t)task::get_stack_item(task::current(), 1);
    uint32_t block_count = (uint32_t)task::get_stack_item(task::current(), 2);
    int fd               = (int)task::get_stack_item(task::current(), 3);
    
    return (void*)fread(ptr, block_size, block_count, fd);
}

void* systemcall_fseek(interrupt_frame_t* frame)
{
    int fd = (int)task::get_stack_item(task::current(), 0);
    int offset = (int)task::get_stack_item(task::current(), 1);
    file_seek_mode_t whence = (file_seek_mode_t)(int)task::get_stack_item(task::current(), 2);
    
    return (void*)fseek(fd, offset, whence);
}

void* systemcall_fstat(interrupt_frame_t* frame)
{
    int fd = (int)task::get_stack_item(task::current(), 0);
    file_stat_t* stat = (file_stat_t*)task::get_stack_item(task::current(), 1);

    return (void*)fstat(fd, stat);
}

void* systemcall_fclose(interrupt_frame_t* frame)
{
    int fd = (int)task::get_stack_item(task::current(), 0);
    return (void*)fclose(fd);
}
