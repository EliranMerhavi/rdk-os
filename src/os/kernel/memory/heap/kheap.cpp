#include "kheap.h"
#include "heap.h"
#include "config.h"
#include "os/terminal/terminal.h"

#include "memory.h"

heap_t       kernel_heap;
heap_table_t kernel_heap_table;

void kheap_init()
{
    int total_table_entries = HEAP_SIZE_BYTES / HEAP_BLOCK_SIZE;
    kernel_heap_table.entries = (HEAP_BLOCK_TABLE_ENTRY*)(KHEAP_TABLE_ADDRESS);
    kernel_heap_table.total = total_table_entries;

    void* end = (void*)(KHEAP_ADDRESS + HEAP_SIZE_BYTES);
    int res = kernel_heap.create((void*)KHEAP_ADDRESS, end, &kernel_heap_table); 

    if (res < 0)
        terminal::print("Failed to create heap\n");
}

void* kmalloc(size_t size)
{
    return kernel_heap.malloc(size);
}

void *kzalloc(size_t size)
{
    void* res = kernel_heap.malloc(size);
    memset(res, 0, size);
    return res;
}

void kfree(void* ptr)
{
    kernel_heap.free(ptr);
} 
