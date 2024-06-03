#pragma once
#include <stdint.h>

enum IRQs
{
    IRQ0    = 32,
    IRQ1    = 33,
    IRQ2    = 34,
    IRQ3    = 35,
    IRQ4    = 36,
    IRQ5    = 37,
    IRQ6    = 38,
    IRQ7    = 39,
    IRQ8    = 40,
    IRQ9    = 41,
    IRQ10   = 42,
    IRQ11   = 43,
    IRQ12   = 44,
    IRQ13   = 45,
    IRQ14   = 46,
    IRQ15   = 47
};

struct interrupt_frame_t
{
    uint32_t ds;
    uint32_t edi, esi, ebp, kernel_esp, ebx, edx, ecx, eax;
    uint32_t interrupt_number, error;
    uint32_t eip, cs, eflags, esp, ss;
} __attribute__((packed));

using handler_t = void(*)(interrupt_frame_t* frame);

extern "C"
__attribute__((cdecl))
void interrupt_handler(interrupt_frame_t* frame);

namespace isr
{
    void init();
    void init_handlers();
    void _register(uint16_t interrupt_number, handler_t handler);
}
