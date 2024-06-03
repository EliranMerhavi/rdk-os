#include "isr.h"

#include "os/terminal/terminal.h"
#include "../panic.h"

#include "os/kernel/idt/idt.h"
#include "exception.h"
#include "os/kernel/core/io.h"

#include "../memory/paging.h"
#include "../task/task.h"

#include "../kernel.h"

#define INTERRUPTS_COUNT 256

handler_t             handlers[INTERRUPTS_COUNT];

extern "C"
__attribute__((cdecl))
void interrupt_handler(interrupt_frame_t* frame) 
{
    switch_to_kernel_page();

    if (handlers[frame->interrupt_number]) {
        handlers[frame->interrupt_number](frame);
    }
    
    if (task::current())
       task::switch_to_task_page(task::current());
    
    io::outb(0x20, 0x20); // set end of interrupt
}

extern "C" void* isr_stub_table[];

void isr::init() 
{
    exception::init();
    for (int32_t vector = 0; vector < INTERRUPTS_COUNT; vector++) 
    {
        idt::set_descriptor(vector, isr_stub_table[vector]);
    }

    idt::enable_interrupts();
}

void isr::_register(uint16_t interrupt_number, handler_t handler)
{   
    if (interrupt_number < 0 || interrupt_number >= INTERRUPTS_COUNT) {
        panic("isr::add_handler(): invalid interrupt_number");
    }

    handlers[interrupt_number] = handler;
}
