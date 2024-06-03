#include "timer.h"
#include "../../task/task.h"
#include "../../isr/isr.h"

namespace timer
{
    void irq_handler(interrupt_frame_t* frame);
}

void timer::init()
{
    //isr::_register(IRQ0, irq_handler);    
}

void timer::irq_handler(interrupt_frame_t* frame)
{
    // REMINDER: there will always be next task (because the shell is run infinitely)
//    task::_switch(task::next());
}
