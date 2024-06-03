#pragma once
#include <stdint.h>
#include "../../vfs/fs.h"
#include "disk_stream.h"

namespace disk
{
    enum class type_t 
    {
        REAL_DISK
    };
    using id_t = uint32_t;

    void init();

    fs::filesystem_t* get_filesystem(disk::id_t disk_id);

    int read(disk::id_t disk_id, uint32_t address, uint32_t bytes_count, void* buffer);
    int read_sectors(disk::id_t disk_id, uint32_t lba, uint8_t sectors_count, void* buffer);
    int sectors_to_absolute(int sector);

    bool validate_disk_id(disk::id_t disk_id);
}
