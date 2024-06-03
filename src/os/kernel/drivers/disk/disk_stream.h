#pragma once

#include "stddef.h"
#include "stdint.h"

namespace disk
{
    using id_t = uint32_t;
    class stream_t 
    {
    public:
        stream_t(disk::id_t disk_id);
        int seek(size_t pos);
        int read(void* buffer, size_t n);
    private:
        disk::id_t m_disk_id;
        size_t m_pos;
    };
}
