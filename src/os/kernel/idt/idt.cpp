#include "idt.h"
#include "config.h"
#include "memory.h"

extern "C" void idt_load(idt::idtr_t* idtr);

namespace idt
{
    descriptor_t descriptors[TOTAL_INTERRUPTS];
    idtr_t idtr; 
}

void idt::init() 
{
    memset(idt::descriptors, 0, sizeof(idt::descriptors));
    idt::idtr.limit = sizeof(idt::descriptors) - 1;
    idt::idtr.base = (uint32_t)idt::descriptors;
    
    idt_load(&idt::idtr);
}

void idt::set_descriptor(int interrupt_number, void* address)
{
    idt::descriptor_t& descriptor = idt::descriptors[interrupt_number];

    descriptor.offset_1 = (uint32_t)address & 0x0000ffff;
    descriptor.selector = GDT_KERNEL_CODE_SEGMENT; // kernel code selector
    descriptor.zero = 0;
    descriptor.type_attr = 0xEE;
    descriptor.offset_2 = (uint32_t)address >> 16;
}

void idt::disable_interrupts()
{
    __asm__ ("cli;");
}

void idt::enable_interrupts()
{
    __asm__ ("sti;");
}