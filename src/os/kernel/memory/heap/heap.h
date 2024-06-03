#pragma once

#include <stdint.h>
#include <stddef.h>

#define HEAP_BLOCK_TABLE_ENTRY_TAKEN 0x01
#define HEAP_BLOCK_TABLE_ENTRY_FREE 0x00

#define HEAP_BLOCK_HAS_NEXT 0b10000000
#define HEAP_BLOCK_IS_FIRST  0b01000000

using HEAP_BLOCK_TABLE_ENTRY = unsigned char;

struct heap_table_t
{
    HEAP_BLOCK_TABLE_ENTRY* entries;
    size_t total;
};

class heap_t
{
public:
    int create(void* ptr, void* end, heap_table_t* table);
    void* malloc(size_t size);
    void free(void* ptr);
private:
    void* malloc_blocks(uint32_t total_blocks);
    void  mark_blocks_taken(int start_block, int total_blocks);
    void  mark_blocks_free(int starting_block);
    
    int   get_start_block(uint32_t total_blocks) const;   
    int   address_to_block(void* address) const;
    void* block_to_address(int block) const;

private:
    heap_table_t* m_table;
    void* m_start_addr;
};
