#pragma once

enum system_commands
{
    system_get_args = 0x00,
    // rdk_os system commands
    system_print    = 0x10,
    // stdlib system commands
    system_malloc   = 0x20,
    system_free     = 0x21,
    system_system   = 0x22,
    system_exit     = 0x23,

    // stdio system commands
    system_getchar  = 0x30,
    system_putchar  = 0x31,

    system_fopen    = 0xf0,
    system_fread    = 0xf1,
    system_fseek    = 0xf2,
    system_fstat    = 0xf3,
    system_fclose   = 0xf4,
};

struct interrupt_frame_t;

using systemcall_handler_t = void*(*)(interrupt_frame_t* frame);

namespace systemcall 
{
    void init();
    void _register(int command_id, systemcall_handler_t call);
}