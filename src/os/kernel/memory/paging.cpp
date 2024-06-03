#include "paging.h"
#include "heap/kheap.h"
#include "status.h"

#include "os/terminal/terminal.h"
extern "C" void paging_load_directory(void *directory);

namespace paging
{
    namespace
    {
        paging::directory_t *current_directory = nullptr;
    }

    int get_indices(void *virtual_addr, paging::directory_t *directory_index_out, uint32_t *table_index_out);
    int map_to(paging::directory_t *directory, void *virtual_addr, void *physical_addr, void *physical_end_addr, uint8_t flags);
}

paging::directory_t *paging::new_four_gb_directory(uint8_t flags)
{
    paging::directory_t *directory = (paging::directory_t *)kzalloc(sizeof(paging::directory_t) * PAGING_TOTAL_ENTRIES_PER_TABLE);
    int offset = 0;
    for (int i = 0; i < PAGING_TOTAL_ENTRIES_PER_TABLE; i++)
    {
        uint32_t *entry = (uint32_t *)kzalloc(sizeof(uint32_t) * PAGING_TOTAL_ENTRIES_PER_TABLE);
        for (int b = 0; b < PAGING_TOTAL_ENTRIES_PER_TABLE; b++)
        {
            entry[b] = (offset + (b * PAGING_PAGE_SIZE)) | flags;
        }
        offset += (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE);
        directory[i] = (paging::directory_t)((uint32_t)entry | flags | PAGING_IS_WRITEABLE);
    }

    return directory;
}

void paging::switch_directory(paging::directory_t *directory)
{
    paging_load_directory(directory);
    current_directory = directory;
}

int paging::set(paging::directory_t *directory, void *virtual_addr, uint32_t val)
{
    if (!paging::is_aligned(virtual_addr))
        return ERROR(EINVARG);

    int res = 0;

    paging::directory_t directory_index;
    uint32_t table_index;

    if ((res = paging::get_indices(virtual_addr, &directory_index, &table_index)) < 0)
        return res;

    uint32_t entry = directory[directory_index];
    uint32_t *table = (uint32_t *)(entry & 0xFFFFF000);
    
    table[table_index] = val;
    
    return res;
}

uint32_t paging::get(paging::directory_t *directory, void *virtual_addr)
{
    paging::directory_t directory_index;
    uint32_t table_index = 0;

    paging::get_indices(virtual_addr, &directory_index, &table_index);

    uint32_t entry = directory[directory_index];
    uint32_t *table = (uint32_t *)(entry & 0xFFFFF000);

    return table[table_index];
}

int paging::get_indices(void *virtual_addr, paging::directory_t *directory_index_out, uint32_t *table_index_out)
{
    if (!paging::is_aligned(virtual_addr))
    {
        return ERROR(EINVARG);
    }

    *directory_index_out = (paging::directory_t)((uint32_t)virtual_addr / (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE));
    *table_index_out = ((uint32_t)virtual_addr % (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE) / PAGING_PAGE_SIZE);

    return 0;
}

void paging::free(directory_t *directory)
{
    for (int i = 0; i < 1024; i++)
    {
        uint32_t entry = directory[i];
        uint32_t *table = (uint32_t *)(entry & 0xfffff000);
        kfree(table);
    }
    kfree(directory);
}

int paging::map_to(paging::directory_t *directory, void *virtual_addr, void *physical_addr, void *physical_end_addr, uint8_t flags)
{

    if (!paging::is_aligned(virtual_addr) ||
        !paging::is_aligned(physical_addr) ||
        !paging::is_aligned(physical_end_addr) ||
        physical_end_addr < physical_addr)
    {
        return ERROR(EINVARG);
    }

    uint32_t total_pages = ((char *)physical_end_addr - (char *)physical_addr) / PAGING_PAGE_SIZE;

    for (uint32_t i = 0; i < total_pages; i++)
    {
        if (IS_ERROR(paging::set(directory, virtual_addr, (uint32_t)physical_addr | flags)))
            break;
        virtual_addr = (char *)virtual_addr + PAGING_PAGE_SIZE;
        physical_addr = (char *)physical_addr + PAGING_PAGE_SIZE;
    }

    return 0;
}

bool paging::is_aligned(void *addr)
{
    return !((uint32_t)addr % PAGING_PAGE_SIZE);
}

void *paging::align_address(void *addr)
{
    if (paging::is_aligned(addr))
    {
        return addr;
    }
    return (void *)((uint32_t)addr + PAGING_PAGE_SIZE - ((uint32_t)addr % PAGING_PAGE_SIZE));
}

void *paging::align_to_lower_page(void *addr)
{
    uint32_t _addr = (uint32_t)addr;
    _addr -= (_addr % PAGING_PAGE_SIZE);
    return (void*)_addr;
}
