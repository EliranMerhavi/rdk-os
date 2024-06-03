#pragma once
#include <stdint.h>
#include <stddef.h>

namespace idt
{
    struct descriptor_t
    {
        uint16_t offset_1; // offset bits 0 - 15
        uint16_t selector; // selector in our gdt
        uint8_t zero;      // does nothing. unused set to zero
        uint8_t type_attr; // descriptor type and attributes
        uint16_t offset_2; // offset bits 16 - 31
    } __attribute__((packed));

    struct idtr_t
    {
        uint16_t limit; // size of descriptor table - 1
        uint32_t base; // base address of start of the idt 
    } __attribute__((packed));
    
    void init();
    void set_descriptor(int interrupt_number, void* address);
    void enable_interrupts();
    void disable_interrupts();
}
