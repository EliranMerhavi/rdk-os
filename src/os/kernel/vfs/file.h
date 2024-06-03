#pragma once

#include "stddef.h"
#include "fs.h"

namespace file
{
    struct descriptor_t
    {
        // descriptor index
        int index;    
        
        // private data for internal file descriptor
        void* private_data;

        disk::id_t disk_id;    
    }; 

    void init();

    int create_descriptor(descriptor_t** res);
    file::descriptor_t* descriptor(size_t fd);

}

int fopen(const char* filename, const char* mode);
int fread(void* ptr, uint32_t block_size, uint32_t block_count, int fd);
int fseek(int fd, int offset, file_seek_mode_t whence); 
int fstat(int fd, file_stat_t* stat);
int fclose(int fd);