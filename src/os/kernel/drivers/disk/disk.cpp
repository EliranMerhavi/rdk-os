#include "disk.h"

#include "status.h"
#include "config.h"
#include "memory.h"

#include "os/kernel/isr/isr.h"
#include "os/kernel/core/io.h"

#include "os/terminal/terminal.h"

#define ATA_PRIMARY_BASE 0x1f0
#define ATA_SECONDARY_BASE 0x170

namespace disk
{
    namespace 
    {
        bool has_secondary;
    }

    fs::filesystem_t disk_filesystems[2];
}

void disk::init()
{
    has_secondary = false;    
    fs::resolve(0);    
}

fs::filesystem_t* disk::get_filesystem(disk::id_t disk_id)
{
    if (!validate_disk_id(disk_id))
        return nullptr;
    
    fs::filesystem_t* filesystem = &disk_filesystems[disk_id];
    return filesystem;    
}

int disk::read(disk::id_t disk_id, uint32_t address, uint32_t bytes_count, void* buffer)
{
    if (!validate_disk_id(disk_id))
        return ERROR(EIO);

 
    char buf[SECTOR_SIZE];

    int start_pos = address % SECTOR_SIZE;
    int lba = (address - start_pos) / SECTOR_SIZE;
    
    disk::read_sectors(disk_id, lba, 1, buf);

    char* ptr = (char *)buffer;

    for (uint32_t i = 0; i < bytes_count; i++) {
        int buf_pos = (start_pos + i) % 512;
        if (!buf_pos) {
            disk::read_sectors(disk_id, lba, 1, buf);
            lba++;
        }
        ptr[i] = buf[buf_pos];
    }

    return 0;
}

int disk::read_sectors(disk::id_t disk_id, uint32_t lba, uint8_t sectors_count, void *buffer)
{   
    if (!validate_disk_id(disk_id))
        return ERROR(EIO);

    int base = disk_id ? ATA_SECONDARY_BASE : ATA_PRIMARY_BASE;

    io::outb(base + 6, (uint8_t)((lba >> 24) | 0xE0));
    io::outb(base + 2, sectors_count);
    io::outb(base + 3, (uint8_t)(lba));
    io::outb(base + 4, (uint8_t)(lba >> 8));
    io::outb(base + 5, (uint8_t)(lba >> 16));
    io::outb(base + 7, 0x20);

    uint16_t* ptr = (uint16_t*)buffer;

    for (int b = 0; b < sectors_count; b++) {
        // wait for the bufffer to be ready

        while (!(io::inb(base + 7) & 0x08));

        // copy from hard disk to memory 
        for (int i = 0; i < 256; i++) {
            *ptr = io::inw(base + 0);
            ptr++;
        }
    }

    return 0;
}

int disk::sectors_to_absolute(int sector) 
{
    return sector * SECTOR_SIZE;
}

bool disk::validate_disk_id(disk::id_t disk_id)
{
    return (!disk_id || (disk_id == 1 && has_secondary));
}

