#include "disk_stream.h"
#include "disk.h"
#include "config.h"
#include "os/terminal/terminal.h"
#include "status.h"

disk::stream_t::stream_t(disk::id_t disk_id) 
    : m_disk_id(disk_id), m_pos(0)
{
    
}


int disk::stream_t::seek(size_t pos)
{
    m_pos = pos;
    return 0;
}

int disk::stream_t::read(void* buffer, size_t n)
{
    int res = 0;
    res = disk::read(m_disk_id, m_pos, n, buffer);
    if (IS_ERROR(res)) 
    {
        return res;
    }
    return seek(m_pos + n);
}
