#include "gdt.h"
#include "config.h"
#include <stdint.h>
#include "../task/tss.h"

#include "config.h"

namespace gdt
{
    struct entry_t
    {
        uint16_t limit_low;
        uint16_t base_low;
        uint8_t  base_middle;
        uint8_t  access;
        uint8_t  flags_limit_hi;
        uint8_t  base_high;
    } __attribute__((packed));

    struct gdtr_t
    {
        uint16_t limit;
        gdt::entry_t* ptr;
    } __attribute__((packed));

    enum access
    {
        GDT_ACCESS_CODE_READABLE                = 0x02,
        GDT_ACCESS_DATA_WRITEABLE               = 0x02,

        GDT_ACCESS_CODE_CONFORMING              = 0x04,
        GDT_ACCESS_DATA_DIRECTION_NORMAL        = 0x00,
        GDT_ACCESS_DATA_DIRECTION_DOWN          = 0x04,

        GDT_ACCESS_DATA_SEGMENT                 = 0x10,
        GDT_ACCESS_CODE_SEGMENT                 = 0x18,

        GDT_ACCESS_DESCRIPTOR_TSS               = 0x00,

        GDT_ACCESS_RING0                        = 0x00,
        GDT_ACCESS_RING1                        = 0x20,
        GDT_ACCESS_RING2                        = 0x40,
        GDT_ACCESS_RING3                        = 0x60,

        GDT_ACCESS_PRESENT                      = 0x80
    };

    enum flags 
    {
        GDT_FLAG_64BIT                          = 0x20,
        GDT_FLAG_32BIT                          = 0x40,
        GDT_FLAG_16BIT                          = 0x00,

        GDT_FLAG_GRANULARITY_1B                 = 0x00,
        GDT_FLAG_GRANULARITY_4K                 = 0x80,
    };

    // Helper macros
    #define GDT_LIMIT_LOW(limit)                ((uint16_t)((limit) & 0xFFFF))
    #define GDT_BASE_LOW(base)                  ((uint16_t)((base) & 0xFFFF))
    #define GDT_BASE_MIDDLE(base)               ((uint8_t)(((base) >> 16) & 0xFF))

    #define GDT_FLAGS_LIMIT_HI(limit, flags)    ((uint8_t)((((limit) >> 16) & 0xF) | ((flags) & 0xF0)))
    #define GDT_BASE_HIGH(base)                 ((uint8_t)(((base) >> 24) & 0xFF))

    #define GDT_ENTRY(base, limit, access, flags) {                     \
        GDT_LIMIT_LOW(limit),                                           \
        GDT_BASE_LOW(base),                                             \
        GDT_BASE_MIDDLE(base),                                          \
        ((uint8_t)access),                                              \
        GDT_FLAGS_LIMIT_HI(limit, flags),                               \
        GDT_BASE_HIGH(base)                                             \
    }

    namespace 
    {
        tss::tss_t tss_entry;
        gdt::entry_t table[6] = 
        {
            // NULL descriptor
            GDT_ENTRY(0, 0, 0, 0),

            // Kernel mode 32-bit code segment
            GDT_ENTRY(0,
                0xFFFFF,
                GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_CODE_SEGMENT | GDT_ACCESS_CODE_READABLE,
                GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K),

            // Kernel mode 32-bit data segment
            GDT_ENTRY(0,
                0xFFFFF,
                GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_DATA_SEGMENT | GDT_ACCESS_DATA_WRITEABLE,
                GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K),

            // User mode 32-bit code segment
            GDT_ENTRY(0,
                0xFFFFF,
                0xF8,
                GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K),

            // User mode 32-bit data segemnt
            GDT_ENTRY(0,
                0xFFFFF,
                0xF2,
                GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K)
        };
    }
}

extern "C"
void __attribute__((cdecl)) gdt_load(gdt::gdtr_t* gdtr, uint16_t kernel_code_segment, uint16_t kernel_data_segment);

void gdt::init()
{
    uint32_t tss_base   = (uint32_t)&tss_entry,
             tss_limit  = sizeof(tss_entry),
             tss_access = 0x89,
             tss_flags  = 0x00;
    
    table[5] = GDT_ENTRY(tss_base, tss_limit, tss_access, tss_flags);

    gdt::gdtr_t gdtr = { sizeof(table) - 1, table };
    gdt_load(&gdtr, GDT_KERNEL_CODE_SEGMENT, GDT_KERNEL_DATA_SEGMENT);
    tss::init(&tss_entry, GDT_TSS);
}
