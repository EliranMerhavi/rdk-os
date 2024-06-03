#include "fat16.h"
#include "status.h"
#include "config.h"

#include "os/terminal/terminal.h"
#include "os/kernel/memory/heap/kheap.h"
#include "../../drivers/disk/disk.h"

#include "string.h"
#include "stdint.h"
#include "memory.h"

#define fat16_SIGNATURE         0x29
#define fat16_FAT_ENTRY_SIZE    0x04
#define fat16_BAD_SECTOR        0x0FFFFFF7
#define fat16_UNUSED            0x00000000

#define cluster_to_sector(private_data, cluster) (private_data->root_directory.ending_sector_pos + ((cluster - 2) * private_data->record.sectors_per_cluster))
#define get_first_cluster(item) ((uint32_t)(item->first_cluster_number_high16) | item->first_cluster_number_low16)


// fat directory entry attributes bitmask

namespace fat16
{
    enum directory_attributes
    {
        READ_ONLY = 0x01,
        HIDDEN    = 0x02,
        SYSTEM    = 0x04,
        VOLUME_ID = 0x08,
        SUBDIRECTORY = 0x10,
        ARCHIVE   = 0x20,
        DEVICE    = 0x40,
        RESERVED  = 0x80
    };

    enum class item_t
    {
        DIRECTORY  = 0,
        _FILE      = 1
    };


    struct fat_time_t 
    {
        uint8_t hours   : 5;
        uint8_t minutes : 6;
        uint8_t seconds : 5; // need to be multiplied by 2
    } __attribute__((packed));
    
    struct fat_date_t
    {
        uint8_t year    : 7;
        uint8_t month   : 4;
        uint8_t day     : 5;
    } __attribute__((packed));

    struct directory_item_t
    {
        uint8_t  filename[8];
        uint8_t  ext[3];
        uint8_t  attributes;
        uint8_t  reserved_nt;
        uint8_t  creation_time_in_tenth_of_seconds;
        fat_time_t creation_time;                                                                                                   
        fat_date_t creation_date;            
        fat_date_t last_accessed_date;        
        uint16_t first_cluster_number_high16;
        fat_time_t last_modification_time;     
        fat_date_t last_modification_date;  
        uint16_t first_cluster_number_low16;
        uint32_t size_in_bytes;
    };

    struct directory_t
    {
        directory_item_t* items;
        uint32_t items_count;
        uint32_t sectors_pos;
        uint32_t ending_sector_pos;

        static void free(directory_t* directory);
    };

    struct fat_item_t 
    {
        union {
            fat16::directory_item_t* item;
            fat16::directory_t*      directory;
        };

        fat16::item_t type;

        static void free(fat_item_t* item);
    };

    struct file_descriptor_t
    {
        fat_item_t* item;
        uint32_t pos;
    };

    struct extended_boot_record_t
    {
        uint8_t  drive_number;
        uint8_t  reserved_2;
        uint8_t  signature;
        uint32_t volume_id;
        uint8_t  volume_label[11];
        uint8_t  system_identifier[8];
    } __attribute__((packed));

    struct boot_record_t
    {
        uint8_t  short_jmp_instruction[3];
        uint8_t  oem_identifier[8];
        uint16_t bytes_per_sector;
        uint8_t  sectors_per_cluster;
        uint16_t reserved_sectors;
        uint8_t  fat_copies;
        uint16_t root_dir_entries;
        uint16_t number_of_sectors;
        uint8_t  media_type;
        uint16_t sectors_per_fat;
        uint16_t sectors_per_track;
        uint16_t number_of_heads;
        uint32_t hidden_sectors;
        uint32_t sectors_big;
    } __attribute__((packed));


    struct private_data_t
    {
    public:
        boot_record_t record;
        extended_boot_record_t extended_record;
        directory_t root_directory;
        
        // used to stream data clusters
        disk::stream_t cluster_read_stream;
        // used to stream the file allocation table 
        disk::stream_t fat_read_stream;

        // used in situations where we stream the directory 
        disk::stream_t directory_stream;
    };

    private_data_t* new_private_data(disk::id_t disk_id); 
}

namespace fat16
{   

    int   resolve(disk::id_t disk_id);
    void* open(disk::id_t disk_id, path::part_t* part, file_mode_t mode);
    int   read(disk::id_t disk_id, void* descriptor_private_data, uint32_t block_size, uint32_t block_count, char* buffer);
    int   seek(void* descriptor_private_data, uint32_t offset, file_seek_mode_t seek_mode);    
    int   stat(disk::id_t disk, void* descriptor_private_data, file_stat_t* stat); 
    int   close(void* descriptor_private_data);

    int load_root_directory(fat16::private_data_t* data);

    int get_items_count(fat16::private_data_t* data, uint32_t directory_start_sector);

    fat16::fat_item_t* get_directory_entry(disk::id_t disk_id, path::part_t* filepath);
    fat16::fat_item_t* get_item_in_directory(fat16::private_data_t* private_data, fat16::directory_t* directory, const char* name);
    fat16::fat_item_t* new_fat_item_for_directory_item(fat16::private_data_t* private_data, fat16::directory_item_t* item);

    fat16::directory_t* load_directory(fat16::private_data_t* private_data, fat16::directory_item_t* item);
    int get_entry(fat16::private_data_t* private_data, int cluster);

    void to_proper_string(char ** out, const char* in);
    void get_full_relative_filename(fat16::directory_item_t* item, char* out, int max_len);    

    int get_cluster_for_offset(fat16::private_data_t* private_data, int starting_cluster, int offset);
    int read_internal(fat16::private_data_t* private_data, int starting_cluster, int offset, int total, char* out);
}

fs::filesystem_interface_t fat16::init()
{
    fs::filesystem_interface_t res {
        .resolve = fat16::resolve,
        .open    = fat16::open,
        .read    = fat16::read,
        .seek    = fat16::seek,
        .stat    = fat16::stat,
        .close   = fat16::close
    };

    strcpy(res.name, "fat16");

    return res;
}

fat16::private_data_t* fat16::new_private_data(disk::id_t disk_id)
{
    private_data_t* res = (private_data_t*)kzalloc(sizeof(private_data_t));
    res->fat_read_stream = disk::stream_t(disk_id);
    res->directory_stream = disk::stream_t(disk_id);
    res->cluster_read_stream = disk::stream_t(disk_id);

    return res;
}

int fat16::resolve(disk::id_t disk_id)
{
    int res = 0;

    fs::filesystem_t* fs = disk::get_filesystem(disk_id);
    fat16::private_data_t* private_data = fat16::new_private_data(disk_id);
    fs->private_data = private_data;

    disk::stream_t stream(disk_id);

    if (IS_ERROR(stream.read(&private_data->record, sizeof(private_data->record))) || 
        IS_ERROR(stream.read(&private_data->extended_record, sizeof(private_data->extended_record))))
    {
        res = ERROR(EIO);
    } 
    else if (private_data->extended_record.signature != 0x29)
    {
        res = ERROR(EFSNOTUS);
    }
    else if (IS_ERROR(fat16::load_root_directory(private_data)))
    {
        res = ERROR(EIO);
    }

    if (IS_ERROR(res)) {
        kfree(fs->private_data);
        fs->private_data = nullptr;
    }

    return res;
}

void* fat16::open(disk::id_t disk_id, path::part_t* filepath, file_mode_t mode)
{   
    if (mode != file_mode_t::FILE_MOD_READ) 
        return (void*)ERROR(ERDONLY);
    
    fat16::file_descriptor_t* descriptor = (fat16::file_descriptor_t*)kzalloc(sizeof(fat16::file_descriptor_t));

    if (!descriptor) 
        return (void*)ERROR(ENOMEM);

    descriptor->item = fat16::get_directory_entry(disk_id, filepath);

    if (!descriptor->item) 
        return (void*)ERROR(EIO);
    
    descriptor->pos = 0;

    return descriptor;
}

int fat16::read(disk::id_t disk_id, void* descriptor_private_data, uint32_t block_size, uint32_t block_count, char* buffer)
{
    int res = 0;
    fat16::private_data_t* private_data = (fat16::private_data_t*)disk::get_filesystem(disk_id)->private_data;
    fat16::file_descriptor_t* descriptor = (fat16::file_descriptor_t*)descriptor_private_data;
    fat16::directory_item_t* item = descriptor->item->item;

    int offset = descriptor->pos;
    
    for (uint32_t i = 0; i < block_count; i++) {
        res = fat16::read_internal(private_data, get_first_cluster(item), offset, block_size, buffer);
        if (IS_ERROR(res)) {
            return res;
        }
        buffer += block_size;
        offset += block_size;
    }
    
    res = block_count;

    return res;
}

int fat16::seek(void* descriptor_private_data, uint32_t offset, file_seek_mode_t seek_mode)
{
    fat16::file_descriptor_t* descriptor = (fat16::file_descriptor_t*)descriptor_private_data;
    fat16::fat_item_t* item = descriptor->item;

    if (item->type != fat16::item_t::_FILE) {
        return ERROR(EINVARG);
    }
        
    fat16::directory_item_t* file_item = item->item;
    
    if (offset >= file_item->size_in_bytes) {
        return ERROR(EIO);
    }
    
    if (seek_mode == SEEK_END) {
        return ERROR(EUNIMP);
    }
    if (seek_mode != SEEK_SET && seek_mode != SEEK_CUR) {
        return ERROR(EIO);
    }

    descriptor->pos = (seek_mode == SEEK_SET) * offset + 
                      (seek_mode == SEEK_CUR) * (descriptor->pos + offset);

    return 0;
}

int fat16::stat(disk::id_t disk, void* descriptor_private_data, file_stat_t* stat)
{
    fat16::file_descriptor_t* descriptor = (fat16::file_descriptor_t*)descriptor_private_data;
    fat16::fat_item_t* item = descriptor->item;

    if (item->type != fat16::item_t::_FILE) {  // check if you can fstat a directory
        return ERROR(EINVARG);
    }

    fat16::directory_item_t* file_item = item->item;

    stat->filesize = file_item->size_in_bytes;
    stat->flags = 0;
    if (file_item->attributes & fat16::directory_attributes::READ_ONLY) {
        stat->flags |= file_stat_flags_t::FILE_STATE_READ_ONLY;
    }
    return 0;
}

int fat16::close(void* descriptor_private_data)
{
    fat16::file_descriptor_t* descriptor = (fat16::file_descriptor_t*)descriptor_private_data;   
    fat16::fat_item_t::free(descriptor->item);
    kfree(descriptor);
    return 0;
}

int fat16::load_root_directory(fat16::private_data_t* data)
{   
    const fat16::boot_record_t& record = data->record;
    
    int root_dir_sector_pos = (record.fat_copies * record.sectors_per_fat) + record.reserved_sectors;
    int root_directory_entries = record.root_dir_entries;
    int root_dir_size = root_directory_entries * sizeof(fat16::directory_item_t);
    
    int items_count = fat16::get_items_count(data, root_dir_sector_pos);
    
    fat16::directory_t& root = data->root_directory;

    fat16::directory_item_t* items = (fat16::directory_item_t*)kzalloc(sizeof(fat16::directory_item_t*));
    disk::stream_t& stream = data->directory_stream;
    
    
    if (IS_ERROR(stream.seek(disk::sectors_to_absolute(root_dir_sector_pos))) || 
        IS_ERROR(stream.read(items, root_dir_size)))
    {
        return ERROR(EIO);
    }
    
    root.items = items;
    root.items_count = items_count;
    root.sectors_pos = root_dir_sector_pos;
    root.ending_sector_pos = root_dir_sector_pos + root_dir_size / SECTOR_SIZE;
    
    return 0;
}

int fat16::get_items_count(fat16::private_data_t* data, uint32_t directory_start_sector)
{
    fat16::directory_item_t item,
                            empty_item;

    memset(&empty_item, 0, sizeof(empty_item));

    int res = 0;
    
    int directory_start_pos = disk::sectors_to_absolute(directory_start_sector);
    disk::stream_t& stream = data->directory_stream;

    if (stream.seek(directory_start_pos))
    {
        return ERROR(EIO);
    }
    
    while (1)
    {
        if (stream.read(&item, sizeof(item))) 
        {
            return ERROR(EIO);
        }

        if (item.filename[0] == 0x00)
        {
            break;
        }

        if (item.filename[0] == 0xE5) // unused
        {
            continue;
        }

        res++;
    }

    return res;
}

fat16::fat_item_t* fat16::get_directory_entry(disk::id_t disk_id, path::part_t* filepath) {
    fat16::private_data_t* private_data = (fat16::private_data_t*)disk::get_filesystem(disk_id)->private_data;

    fat16::fat_item_t* current_item = 0;

    fat16::fat_item_t* root_item = fat16::get_item_in_directory(private_data, &private_data->root_directory, filepath->val);

    if (!root_item)
        return current_item;
    
    path::part_t* next_part = filepath->next;
    current_item = root_item;


    while (next_part) {
        if (current_item->type != fat16::item_t::DIRECTORY) {
            current_item = 0;
            break;
        }

        fat16::fat_item_t* tmp_item = fat16::get_item_in_directory(private_data, current_item->directory, next_part->val);
        fat16::fat_item_t::free(current_item);
        current_item = tmp_item;
        next_part = next_part->next;
    }
    
    return current_item;
}

fat16::fat_item_t* fat16::get_item_in_directory(fat16::private_data_t* private_data, fat16::directory_t* directory, const char* name)
{
    fat16::fat_item_t* item = 0;
    char tmp_filename[MAX_PATH], FAT_name[MAX_PATH];
    
    uint32_t i;

    // all fat16 names are upper case (maybe remove this section when creating a shell (because then you can see the filename in caps))
    for (i = 0; i < MAX_PATH && name[i]; i++)
        FAT_name[i] = toupper(name[i]); 
    // 
    
    for (uint32_t i = 0; i < directory->items_count; i++) {
        fat16::get_full_relative_filename(directory->items + i, tmp_filename, sizeof(tmp_filename)); 
        if (!strncmp(tmp_filename, FAT_name, sizeof(tmp_filename))) {
            item = fat16::new_fat_item_for_directory_item(private_data, &directory->items[i]);
            break;
        }
    }

    return item;
}

fat16::fat_item_t* fat16::new_fat_item_for_directory_item(fat16::private_data_t* private_data, fat16::directory_item_t* item)
{
    fat16::fat_item_t* res = (fat16::fat_item_t*)kzalloc(sizeof(fat16::fat_item_t));
    
    if (!item) 
        return 0;
    
    if (item->attributes & fat16::directory_attributes::SUBDIRECTORY)
    {
        res->directory = fat16::load_directory(private_data, item);
        res->type = fat16::item_t::DIRECTORY;
        return res;
    }
    
    res->type = fat16::item_t::_FILE;
    
    // clone the item    
    res->item = (fat16::directory_item_t*)kzalloc(sizeof(fat16::directory_item_t));
    
    if (!res->item)
        return 0;
    
    memcpy(res->item, item, sizeof(fat16::directory_item_t));

    return res;
}


fat16::directory_t* fat16::load_directory(fat16::private_data_t* private_data, fat16::directory_item_t* item)
{
    int res = 0;
    
    fat16::directory_t* directory = 0;

    if (!(item->attributes & fat16::directory_attributes::SUBDIRECTORY)) 
        return (fat16::directory_t*)ERROR(EINVARG);
    
    directory = (fat16::directory_t*)kzalloc(sizeof(fat16::directory_t*));

    if (!directory)
        return (fat16::directory_t*)ERROR(ENOMEM);
    
    int cluster = get_first_cluster(item);
    int cluster_sector = cluster_to_sector(private_data, cluster);
    int items_count = fat16::get_items_count(private_data, cluster_sector);
    
    directory->items_count = items_count;

    int directory_size = directory->items_count * sizeof(fat16::directory_item_t);

    directory->items = (fat16::directory_item_t*)kzalloc(directory_size);

    if (!directory->items) {
        fat16::directory_t::free(directory);
        return (fat16::directory_t*)ERROR(ENOMEM);
    }
    
    res = fat16::read_internal(private_data, cluster, 0x00, directory_size, (char*)directory->items);

    if (IS_ERROR(res)) {
        fat16::directory_t::free(directory);
        return (fat16::directory_t*)res;
    }

    return directory;
}

int fat16::get_entry(fat16::private_data_t* private_data, int cluster)
{
    int res = -1;
    disk::stream_t& stream = private_data->fat_read_stream;
    
    uint32_t fat_table_position = private_data->record.reserved_sectors * SECTOR_SIZE;
    res = stream.seek(fat_table_position * cluster * fat16_FAT_ENTRY_SIZE);
    
    if (IS_ERROR(res)) {
        return res;
    }

    uint16_t buffer = 0;
    res = stream.read(&buffer, sizeof(buffer));
    
    if (IS_ERROR(res)) {
        return res;
    }

    res = buffer;
    
    return res;
}

int fat16::get_cluster_for_offset(fat16::private_data_t* private_data, int starting_cluster, int offset) 
{
    int res = 0;
    int size_of_cluster_bytes = private_data->record.sectors_per_cluster * SECTOR_SIZE;
    int cluster_to_use = starting_cluster;
    int clusters_ahead = offset / size_of_cluster_bytes;

    for (int i = 0; i < clusters_ahead; i++) {
        int entry = fat16::get_entry(private_data, cluster_to_use);

        if (entry == 0xFF8 || entry == 0xFFF) { 
            // we are at the last entry in the file 
            res = ERROR(EIO);
            goto out;
        }

        if (entry == fat16_BAD_SECTOR) {
            res = ERROR(EIO);
            goto out;
        }

        if (entry == 0xFF0 || entry == 0xFF6) { // is entry reserved sector 
            res = ERROR(EIO);
            goto out;
        }
        
        if (entry == 0x00) { // is the file allocation table is corrupted (entry cant be 0 for valid file allocation table)
            res = ERROR(EIO);
            goto out;
        }

        cluster_to_use = entry;
    }

    res = cluster_to_use;
out:
    return res;
}

int fat16::read_internal(fat16::private_data_t* private_data, int starting_cluster, int offset, int total, char* out)
{
    disk::stream_t& stream = private_data->cluster_read_stream;
    
    int size_of_cluster_bytes = private_data->record.sectors_per_cluster * SECTOR_SIZE;

    while (total > 0) {
        int cluster_to_use = fat16::get_cluster_for_offset(private_data, starting_cluster, offset);
        
        if (IS_ERROR(cluster_to_use)) {
            return cluster_to_use;
        }
        
        int offset_from_cluster = (offset % size_of_cluster_bytes);
        
        int starting_sector = cluster_to_sector(private_data, cluster_to_use);
        int starting_pos    = (starting_sector * SECTOR_SIZE) + offset_from_cluster;
        int total_to_read   = (total > size_of_cluster_bytes) ? size_of_cluster_bytes : total;

        if (IS_ERROR(stream.seek(starting_pos)) || 
            IS_ERROR(stream.read(out, total_to_read))) {
            return ERROR(EIO);
        }
        
        total -= total_to_read;
        offset += total_to_read;
        out += total_to_read;
    }
    
    return 0;
}

void fat16::to_proper_string(char ** out, const char* in)
{
    while (*in != 0x00 && *in != 0x20) {
        **out = *in;
        *out += 1;
        in += 1;
    }

    if (*in == 0x20) {
        **out = 0x00;
    }
}

void fat16::get_full_relative_filename(fat16::directory_item_t* item, char* out, int max_len)
{
    memset(out, 0x00, max_len);
    char* out_tmp = out;
    fat16::to_proper_string(&out_tmp, (const char*)item->filename);
    
    if (item->ext[0] != 0x00 && item->ext[0] != 0x20) {
        *out_tmp++ = '.';
        fat16::to_proper_string(&out_tmp, (const char*)item->ext);
    }    
}

void fat16::directory_t::free(directory_t* directory)
{
    if (!directory)
        return;
    
    if (directory->items)
        kfree(directory->items);

    kfree(directory);
}
 
void fat16::fat_item_t::free(fat_item_t* item)
{
    if (!item) 
        return;
    switch (item->type) {

        case fat16::item_t::DIRECTORY:
            fat16::directory_t::free(item->directory);
            break;

        case fat16::item_t::_FILE:

            break;
    }
}
