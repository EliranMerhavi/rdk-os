#include "exception.h"
#include "isr.h"
#include "os/terminal/terminal.h"

#include "../task/task.h"
#include "../task/process.h"

namespace exception
{
    void exception_handler(interrupt_frame_t* frame);
}

void exception::init()
{
    for (int i = 0; i <= 31; i++)
        isr::_register(i, exception_handler);
}

void exception::exception_handler(interrupt_frame_t* frame)
{ 
    process::terminate(task::current()->pid);
    task::_switch(task::next());
    return;

    switch (frame->interrupt_number) {
    case 0:
        terminal::print("INTERRUPT EXCEPTION: division by zero #DE\n");
        break;
    case 1:
        terminal::print("INTERRUPT EXCEPTION: debug #DB\n");
        break;
    case 2:
        terminal::print("INTERRUPT EXCEPTION: non-maskable interrupt\n");
        break;
    case 3:
        terminal::print("INTERRUPT EXCEPTION: break point #BP\n");
        break;
    case 4:
        terminal::print("INTERRUPT EXCEPTION: overflow #BR");
        break;
    case 5:
        terminal::print("INTERRUPT EXCEPTION: bound range exceeded #BR\n");
        break;
    case 6:
        terminal::print("INTERRUPT EXCEPTION: invalid opcode #UD\n");
        break;
    case 7:
        terminal::print("INTERRUPT EXCEPTION: device not available #NM\n");
        break;
    case 8:
        terminal::print("INTERRUPT EXCEPTION: double fault #DF\n");
        break;
    //case 9:
    //    terminal::print("INTERRUPT EXCEPTION: ");
    //   break;
    case 10:
        terminal::print("INTERRUPT EXCEPTION: invalid tss #TS\n");
        break;
    case 11:
        terminal::print("INTERRUPT EXCEPTION: segment not present #NP\n");
        break;
    case 12:
        terminal::print("INTERRUPT EXCEPTION: stack-segment fault #SS\n");
        break;
    case 13:
        terminal::print("INTERRUPT EXCEPTION: general protection fault #GP\n");
        break;
    case 14:
        terminal::print("INTERRUPT EXCEPTION: page fault #PF\n");
        break;
    case 16:
        terminal::print("INTERRUPT EXCEPTION: x87 floating-point exception #MF\n");
        break;
    case 17:
        terminal::print("INTERRUPT EXCEPTION: alignment check #AC\n");
        break;
    case 18:
        terminal::print("INTERRUPT EXCEPTION: machince check #MC\n");
        break;
    case 19:
        terminal::print("INTERRUPT EXCEPTION: SIMD float-point exception #XM/#F\n");
        break;
    case 20:
        terminal::print("INTERRUPT EXCEPTION: virtualizition exception #VE\n");
        break;
    case 21:
        terminal::print("INTERRUPT EXCEPTION: control protection exception #CP\n");
        break;
    case 28:
        terminal::print("INTERRUPT EXCEPTION: hypervisor injection exception #HV\n");
        break;
    case 29:
        terminal::print("INTERRUPT EXCEPTION: VMM communication exception #VC\n");
        break;
    case 30:
        terminal::print("INTERRUPT EXCEPTION: security exception #SX\n");
        break;
    default:
        terminal::print("INTERRUPT EXCEPTION: unhandled exception");
    }



    terminal::print("halting...");
    __asm__ ("cli; hlt;");
}
