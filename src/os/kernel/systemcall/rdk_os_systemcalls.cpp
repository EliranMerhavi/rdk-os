#include "rdk_os_systemcalls.h"
#include "../task/process.h"
#include "string.h"
#include "memory.h"
#include "os/terminal/terminal.h"

void* systemcall_print(interrupt_frame_t* frame)
{
    void* buffer = task::get_stack_item(task::current(), 0);
    static char kernel_buffer[1024];
    memset(kernel_buffer, 0, sizeof(kernel_buffer));
    task::read(task::current(), buffer, kernel_buffer, sizeof(kernel_buffer));

    terminal::printf("%s", kernel_buffer);
    return 0;
}
