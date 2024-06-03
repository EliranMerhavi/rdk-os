#include "file.h"

#include "fs.h"
#include "memory.h"
#include "path.h"

#include "../drivers/disk/disk.h"

#include "status.h"
#include "config.h"
#include "error.h"
#include "os/terminal/terminal.h"

namespace file
{
    namespace 
    {
        file::descriptor_t file_descriptors[MAX_FILE_DESCRIPTORS];
        bool is_taken[MAX_FILE_DESCRIPTORS];
    }

    file_mode_t from_string(const char* mode);
}

void file::init()
{
    memset(is_taken, false, sizeof(is_taken));
}

int file::create_descriptor(file::descriptor_t** res)
{
    int i;

    for (i = 0; i < MAX_FILE_DESCRIPTORS && is_taken[i]; i++)
        ;

    if (i == MAX_FILE_DESCRIPTORS)
        return ERROR(ENOMEM);
    
    *res = &file_descriptors[i]; 
    (*res)->index = i + 1;
    is_taken[i] = false;
    
    return 0;
}

file::descriptor_t* file::descriptor(size_t fd)
{
    int index = fd - 1;
    if (fd <= 0 || fd >= MAX_FILE_DESCRIPTORS || is_taken[index])
        return nullptr;

    return &file_descriptors[index];
}


file_mode_t file::from_string(const char *mode)
{
    file_mode_t res;
    switch (mode[0])
    {
    case 'r':
        res = file_mode_t::FILE_MOD_READ;
        break;
    case 'w':
        res = file_mode_t::FILE_MOD_WRITE;
        break;
    case 'a':
        res = file_mode_t::FILE_MOD_APPEND;
        break;
    default:
        res = file_mode_t::FILE_MOD_INVALID;
        break;
    }
    return res;
}

int fopen(const char* filename, const char* mode)
{
    path::path_t* root_path = path::parse(filename);
    disk::id_t disk_id = root_path->drive_number;
   
    if (!root_path || !root_path->head)
    {
        return 0;
    }
    fs::filesystem_t* filesystem = disk::get_filesystem(disk_id);
    
    if (!filesystem)
    {
        return 0;
    }

    file_mode_t m = file::from_string(mode);

    if (m == file_mode_t::FILE_MOD_INVALID)
    {
        return 0;
    }
        
    void* descriptor_private_data = filesystem->interface->open(disk_id, root_path->head, m);

    if (IS_ERROR(descriptor_private_data))
    {
        return 0;
    }

    file::descriptor_t* descriptor;   
    
    if (IS_ERROR(file::create_descriptor(&descriptor)))
    {
        return 0;
    }
      

    descriptor->disk_id = disk_id;
    descriptor->private_data = descriptor_private_data; 

    return descriptor->index;
}

int fread(void* ptr, uint32_t block_size, uint32_t block_count, int fd)
{  
    if (block_size == 0 || block_count == 0 || fd < 1) {
        return ERROR(EINVARG);
    }

    file::descriptor_t* descriptor = file::descriptor(fd); 
    
    if (!descriptor)
        return ERROR(EINVARG);

    disk::id_t disk_id = descriptor->disk_id;

    return disk::get_filesystem(disk_id)->interface->read(disk_id, descriptor->private_data, block_size, block_count, (char*)ptr);
}

int fseek(int fd, int offset, file_seek_mode_t whence)    
{
    file::descriptor_t* descriptor = file::descriptor(fd);

    if (!descriptor) {
        return ERROR(EIO);
    }

    disk::id_t disk_id = descriptor->disk_id;

    return disk::get_filesystem(disk_id)->interface->seek(descriptor->private_data, offset, whence);
}

int fstat(int fd, file_stat_t* stat)
{
    file::descriptor_t* descriptor = file::descriptor(fd);
    
    if (!descriptor) {
        return ERROR(EIO);
    }

    disk::id_t disk_id = descriptor->disk_id;

    return disk::get_filesystem(disk_id)->interface->stat(disk_id, descriptor->private_data, stat);
}

int fclose(int fd) 
{
    file::descriptor_t* descriptor = file::descriptor(fd);
    
    if (!descriptor) {
        return ERROR(EIO);
    }

    disk::id_t disk_id = descriptor->disk_id;

    int res = disk::get_filesystem(disk_id)->interface->close(descriptor->private_data);   
    
    if (IS_ERROR(res)) {
        return res;
    }
    
    file::is_taken[descriptor->index - 1] = false;

    return 0;
}
