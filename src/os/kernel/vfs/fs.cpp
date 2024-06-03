#include "fs.h"
#include "config.h"
#include "stddef.h"

#include "../panic.h"
#include "../drivers/disk/disk.h"

#include "file.h"
#include "fat/fat16.h"

namespace fs 
{
    namespace 
    {
        fs::filesystem_interface_t interfaces[MAX_FILESYSTEMS];
        size_t interfaces_count;
    }
}

fs::filesystem_t::filesystem_t()
    : interface(nullptr), private_data(nullptr)
{

}

void fs::init()
{
    file::init();
    interfaces_count = 0;
    
    fs::insert(fat16::init());
}

void fs::insert(const fs::filesystem_interface_t& filesystem)
{
    if (interfaces_count == MAX_FILESYSTEMS) {
        panic("fs::insert(): max file systems count reached");
    }

    interfaces[interfaces_count] = filesystem;
    interfaces_count++;
}

void fs::resolve(disk::id_t disk_id)
{
    if (!disk::validate_disk_id(disk_id))
        return;
        
    bool found = false;
    
    for (size_t i = 0; i < interfaces_count && !found; i++) {
        if (!interfaces[i].resolve(disk_id)) {
            disk::get_filesystem(disk_id)->interface = &interfaces[i];
            found = true;
        }
    }
}
