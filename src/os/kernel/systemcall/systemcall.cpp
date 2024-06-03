#include "systemcall.h"
#include "../kernel.h"

#include "config.h"
#include "../panic.h"

#include "../isr/isr.h"

#include "../task/task.h"
#include "../task/process.h"

#include "rdk_os_systemcalls.h"
#include "stdio_systemcalls.h"
#include "stdlib_systemcalls.h"

#include "os/terminal/terminal.h"

void* systemcall_get_args(interrupt_frame_t* frame);

namespace systemcall
{
    namespace 
    {
        systemcall_handler_t s_systemcalls[SYSTEMCALLS_COUNT];
    }

    void isr_handler(interrupt_frame_t* frame);
}

void systemcall::init()
{
    systemcall::_register(system_commands::system_get_args, systemcall_get_args );
 
    // rdk_os system commands
    systemcall::_register(system_commands::system_print   , systemcall_print    );
    systemcall::_register(system_commands::system_exit    , systemcall_exit     );
    
    // stdlib system commands
    systemcall::_register(system_commands::system_malloc  , systemcall_malloc   );
    systemcall::_register(system_commands::system_free    , systemcall_free     );
    systemcall::_register(system_commands::system_system  , systemcall_system   );
    
    // stdio system commands
    systemcall::_register(system_commands::system_getchar , systemcall_getchar  );
    systemcall::_register(system_commands::system_putchar , systemcall_putchar  );
    systemcall::_register(system_commands::system_fopen   , systemcall_fopen    );
    systemcall::_register(system_commands::system_fread   , systemcall_fread    );
    systemcall::_register(system_commands::system_fseek   , systemcall_fseek    );
    systemcall::_register(system_commands::system_fstat   , systemcall_fstat    );
    systemcall::_register(system_commands::system_fclose  , systemcall_fclose   );

    isr::_register(0x80, systemcall::isr_handler);
}

void systemcall::_register(int command_id, systemcall_handler_t call)
{
    if (command_id < 0 || command_id >= SYSTEMCALLS_COUNT) {
        panic("systemcall::add_system_call(): invalid command_id");
    }
    s_systemcalls[command_id] = call;
}

void systemcall::isr_handler(interrupt_frame_t* frame)
{
    // eax stores the command_id at entry and at exit stores the result
    int command_id = frame->eax;
    void* res = 0;

    task::save_current(frame);

    if (command_id < 0 || command_id >= SYSTEMCALLS_COUNT) {
        panic("systemcall::isr_handler(): invalid command_id");
    }   

    systemcall_handler_t systemcall = s_systemcalls[command_id];

    if (systemcall) {
        res = systemcall(frame);
    }
    
    frame->eax = (uint32_t)res;
}

void* systemcall_get_args(interrupt_frame_t* frame) {
    process::process_t* _process = process::get(task::current()->pid);
    return (void*)&_process->args;
}
