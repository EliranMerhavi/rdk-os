#include "heap.h"
#include "config.h"
#include "status.h"
#include "memory.h"

static int validate_table(void* ptr, void* end, heap_table_t* table)
{
    size_t table_size = (size_t)((uint32_t)end - (uint32_t)ptr);
    size_t total_blocks = table_size / HEAP_BLOCK_SIZE;
    
    if (table->total != total_blocks)
        return ERROR(EINVARG);
    
    return 0;
}

static bool validate_alignment(void* ptr)
{
    return ((unsigned int)ptr % HEAP_BLOCK_SIZE) == 0;
}

static uint32_t align_value_to_upper(uint32_t val)
{
    return val + (val % HEAP_BLOCK_SIZE != 0) * (HEAP_BLOCK_SIZE - val % HEAP_BLOCK_SIZE);
}

static int heap_get_entry_type(HEAP_BLOCK_TABLE_ENTRY entry)
{
    return entry & 0x0f;
}

int heap_t::create(void* ptr, void* end, heap_table_t* table)
{
    int res = 0;

    if (!validate_alignment(ptr) || !validate_alignment(end))
        return ERROR(EINVARG);
    
    if ((res = validate_table(ptr, end, table)) < 0) 
        return res;


    memset(this, 0, sizeof(heap_t));
    m_start_addr = ptr;
    m_table = table;
 
    size_t table_size = sizeof(HEAP_BLOCK_TABLE_ENTRY) * table->total;
    memset(table->entries, HEAP_BLOCK_TABLE_ENTRY_FREE, table_size);

    return 0;
}

void* heap_t::malloc(size_t size)
{
    size_t aligned_size = align_value_to_upper(size);
    uint32_t total_blocks = aligned_size / HEAP_BLOCK_SIZE;
    return malloc_blocks(total_blocks);
}

void heap_t::free(void* ptr)
{
    mark_blocks_free(address_to_block(ptr));
}

int heap_t::get_start_block(uint32_t total_blocks) const
{
    heap_table_t* table = m_table;
    int bc = 0;
    int bs = -1;

    for (size_t i = 0; i < table->total; i++)
    {
        if (heap_get_entry_type(table->entries[i]) != HEAP_BLOCK_TABLE_ENTRY_FREE)
        {
            bc = 0;
            bs = -1;
            continue;
        }

        // If this is the first block
        if (bs == -1)
            bs = i;
        bc++;
        if (bc == (int)total_blocks)
            break;
    }

    if (bs == -1)
        return ERROR(ENOMEM);
    
    return bs;
}

void heap_t::mark_blocks_taken(int start_block, int total_blocks)
{
    int end_block = (start_block + total_blocks)-1;

    HEAP_BLOCK_TABLE_ENTRY entry = HEAP_BLOCK_TABLE_ENTRY_TAKEN | HEAP_BLOCK_IS_FIRST;
    if (total_blocks > 1)
    {
        entry |= HEAP_BLOCK_HAS_NEXT;
    }

    for (int i = start_block; i <= end_block; i++)
    {
        m_table->entries[i] = entry;
        entry = HEAP_BLOCK_TABLE_ENTRY_TAKEN;
        if (i != end_block -1)
        {
            entry |= HEAP_BLOCK_HAS_NEXT;
        }
    }
}

void* heap_t::malloc_blocks(uint32_t total_blocks)
{
    int start_block = this->get_start_block(total_blocks);
    if (start_block < 0)
        return nullptr;

    void* address = this->block_to_address(start_block);

    this->mark_blocks_taken(start_block, total_blocks);

    return address;
}

void heap_t::mark_blocks_free(int starting_block)
{
    heap_table_t* table = m_table;
    for (int i = starting_block; i < (int)table->total; i++)
    {
        HEAP_BLOCK_TABLE_ENTRY entry = table->entries[i];
        table->entries[i] = HEAP_BLOCK_TABLE_ENTRY_FREE;
        if (!(entry & HEAP_BLOCK_HAS_NEXT))
        {
            break;
        }
    }
}

int heap_t::address_to_block(void* address) const
{
    return ((int)((uint32_t)address - (uint32_t)m_start_addr)) / HEAP_BLOCK_SIZE;
}

void* heap_t::block_to_address(int block) const 
{
    return (void*)((uint32_t)m_start_addr + (block * HEAP_BLOCK_SIZE));
}
