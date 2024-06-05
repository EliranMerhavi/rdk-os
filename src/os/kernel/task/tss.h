#pragma once
#include <stdint.h>

namespace tss 
{
    struct tss_t
    {
        uint32_t link;
        uint32_t esp0; // kernel stack pointer 
        uint32_t ss0;  // kernel stack segment
        uint32_t esp1; 
        uint32_t esp2;
        uint32_t sr3;
        uint32_t eip;
        uint32_t eflags;
        uint32_t eax, ecx, edx, ebx, esp, ebp, esi, edi;
        uint32_t es, cs, ss, ds, fs, gs;
        uint32_t ldtr;
        uint32_t iopb;
    } __attribute__((packed));

    void init(tss::tss_t *tss_entry, int tss_segment);
}