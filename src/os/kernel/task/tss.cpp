#include "tss.h"
#include "../gdt/gdt.h"
#include "memory.h"
#include "config.h"

extern "C" __attribute__((cdecl))
void tss_load(int tss_segment);

void tss::init(tss::tss_t* tss_entry, int tss_segment) 
{
    memset(tss_entry, 0, sizeof(tss::tss_t));
    tss_entry->esp0 = 0x600000;
    tss_entry->ss0  = GDT_KERNEL_DATA_SEGMENT;

    tss_load(tss_segment);
}