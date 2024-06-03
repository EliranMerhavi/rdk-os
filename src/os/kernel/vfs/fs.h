#pragma once
#include "stdint.h"
#include "path.h"

namespace disk
{
    using id_t = uint32_t;
}

enum file_seek_mode_t
{
    SEEK_SET,
    SEEK_CUR,
    SEEK_END
};

enum file_mode_t
{
    FILE_MOD_READ,
    FILE_MOD_WRITE,
    FILE_MOD_APPEND,
    FILE_MOD_INVALID
};

enum file_stat_flags_t
{
    FILE_STATE_READ_ONLY = 0b00000001
};

struct file_stat_t
{
    uint32_t flags;
    uint32_t filesize;
};

namespace fs
{
    struct filesystem_t;

    using resolve_function_t = int (*)(disk::id_t disk_id);
    using open_function_t    = void *(*)(disk::id_t disk_id, path::part_t *path, file_mode_t mode);
    using read_function_t    = int (*)(disk::id_t disk, void *descriptor_private_data, uint32_t block_size, uint32_t block_count, char *buffer);
    using seek_function_t    = int (*)(void *descriptor_private_data, uint32_t offset, file_seek_mode_t seek_mode);
    using stat_function_t    = int (*)(disk::id_t disk, void *descriptor_private_data, file_stat_t *stat);
    using close_function_t   = int (*)(void *descriptor_private_data);

    struct filesystem_interface_t
    {
        resolve_function_t  resolve;
        open_function_t     open;
        read_function_t     read;
        seek_function_t     seek;
        stat_function_t     stat;
        close_function_t    close;

        char name[200];
    };

    struct filesystem_t
    {
        filesystem_interface_t *interface;
        void *private_data;
        filesystem_t();
    };

    void init();
    void insert(const filesystem_interface_t &filesystem_interface);
    void resolve(disk::id_t disk);
}
