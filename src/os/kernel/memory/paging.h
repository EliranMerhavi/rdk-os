#pragma once

#include <stdint.h>
#include <stddef.h>

#define PAGING_CACHE_DISABLED   0b00010000
#define PAGING_WRITE_THROUGH    0b00001000
#define PAGING_ACCESS_FROM_ALL  0b00000100
#define PAGING_IS_WRITEABLE     0b00000010
#define PAGING_IS_PRESENT        0b00000001

#define PAGING_TOTAL_ENTRIES_PER_TABLE 1024
#define PAGING_PAGE_SIZE 4096


extern "C" void enable_paging();

namespace paging
{
    enum directory_t : uint32_t {};     
    
    directory_t* new_four_gb_directory(uint8_t flags); 

    void     switch_directory(directory_t* directory);
    int      set(directory_t* directory, void* virtual_addr, uint32_t val);
    uint32_t get(paging::directory_t* directory, void* virtual_addr);
    
    int      map_to(paging::directory_t* directory, void* virtual_addr, void* physical_addr, void* physical_end_addr, uint8_t flags);
    int      map(paging::directory_t* directory, void* virtual_addr, void* physical_addr, uint8_t flags);
    void     free(directory_t* directory);

    bool is_aligned(void* addr);
    void* align_address(void* addr);
    void* align_to_lower_page(void* addr);
}
