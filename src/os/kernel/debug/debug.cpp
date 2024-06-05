#include "debug.h"
#include "../isr/isr.h"
#include "os/terminal/terminal.h"

namespace debug
{
    void isr_handler(interrupt_frame_t* frame);
}


void debug::init() 
{
    isr::_register(0x79, debug::isr_handler);  // used for debugging assembly code
}


void debug::isr_handler(interrupt_frame_t* frame)
{
    terminal::printf("eax: %x\n", frame->eax);
    terminal::printf("ss: %x\n", frame->ss);
    terminal::printf("cs: %x\n", frame->cs);
        
}
