#include "elfloader.h"

#include "../../vfs/fs.h"
#include "../../vfs/file.h"

#include "../../memory/heap/kheap.h"
#include "../../memory/paging.h"

#include "elf.h"
#include "string.h"
#include "status.h"
#include "memory.h"

#include "os/terminal/terminal.h"

namespace elf32
{
    namespace
    {
        const char s_signature[] = {0x7f, 'E', 'L', 'F'};
    }

    bool valid_signature(void *buffer);

    bool valid_class(elf32::elf_header_t *header);
    bool valid_encoding(elf32::elf_header_t *header);
    bool is_executable(elf32::elf_header_t *header);
    bool has_program_header(elf32::elf_header_t *header);

    int validate(elf32::elf_header_t *header);
    int process(elf32::file_t *_file);

    int process_pheaders(elf32::file_t *_file);
    int process_pheader(elf32::file_t *_file, elf32::phdr_t *pheader);
    int process_pheader_pt_load(elf32::file_t *_file, elf32::phdr_t *pheader);
}

elf32::elf_header_t *elf32::file_t::header()
{
    return (elf32::elf_header_t *)this->elf_physical_memory;
}

void* elf32::file_t::physical_addr(elf32::phdr_t* phdr)
{
    return (void*)((char*)this->elf_physical_memory + phdr->p_offset);
}

bool elf32::valid_signature(void *buffer)
{
    return memcmp(buffer, s_signature, sizeof(s_signature)) == 0;
}

bool elf32::valid_class(elf32::elf_header_t *header)
{
    return header->e_ident[EI_CLASS] == ELFCLASSNONE || header->e_ident[EI_CLASS] == ELFCLASS32;
}

bool elf32::valid_encoding(elf32::elf_header_t *header)
{
    return header->e_ident[EI_DATA] == ELFDATANONE || header->e_ident[EI_CLASS] == ELFDATA2LSB;
}

bool elf32::is_executable(elf32::elf_header_t *header)
{
    return header->e_type == ET_EXEC && header->e_entry >= PROGRAM_VIRTUAL_ADDRESS;
}

bool elf32::has_program_header(elf32::elf_header_t *header)
{
    return header->has_pheader();
}

int elf32::validate(elf32::elf_header_t *header)
{
    return (valid_signature(header) && 
            valid_class(header) && 
            valid_encoding(header) && 
            has_program_header(header)) 
        ? ALL_OK : ERROR(EINFORMAT);
}

int elf32::load(const char *filename, elf32::file_t *res)
{
    int fd = fopen(filename, "r");

    if (!fd)
    {
        return ERROR(EIO);
    }

    file_stat_t stat;
    int status = fstat(fd, &stat);

    if (IS_ERROR(status))
    {
        fclose(fd);
        return status;
    }
    
    res->elf_physical_memory = kzalloc(stat.filesize);

    if (!res->elf_physical_memory) 
    {
        fclose(fd);
        return ERROR(EIO);
    }

    status = fread(res->elf_physical_memory, stat.filesize, 1, fd);
      
    fclose(fd);

    if (status != 1)
    {
        kfree(res->elf_physical_memory);
        return status;
    }

    status = elf32::process(res);

    if (IS_ERROR(status))
    {
        kfree(res->elf_physical_memory);
        return status;
    }

    return 0;
}

int elf32::process_pheader_pt_load(elf32::file_t *_file, elf32::phdr_t *pheader)
{
    if (_file->virtual_base_addr >= (void*)pheader->p_vaddr || _file->virtual_base_addr == 0x00)
    {
        _file->virtual_base_addr  = (void*)pheader->p_vaddr;
        _file->physical_base_addr = (void*)((int)_file->elf_physical_memory + pheader->p_offset);
    }

    uint32_t end_virtual_addr = pheader->p_vaddr + pheader->p_filesz;

    if (_file->virtual_end_addr <= (void*)(end_virtual_addr) || _file->virtual_end_addr == 0x00)
    {
        _file->virtual_end_addr  = (void*)end_virtual_addr;
        _file->physical_end_addr = (void*)((int)_file->elf_physical_memory + (pheader->p_offset + pheader->p_filesz));
    }

    return 0;
}

int elf32::process(elf32::file_t *_file)
{
    int status = 0;
    elf_header_t *header = _file->header();

    status = elf32::validate(header);

    if (IS_ERROR(status))
    {
        return status;
    }

    status = elf32::process_pheaders(_file);

    if (IS_ERROR(status))
    {
        return status;
    }

    return 0;
}

/*
    struct elf_header_t 
    {
        uint8_t e_ident[EI_NIDENT];
        half_t  e_type;
        half_t  e_machine;
        half_t  e_version;
        half_t  e_entry;
        off_t   e_phoff;
        off_t   e_shoff;
        word_t  e_flags;
        half_t  e_ehsize;
        half_t  e_phentsize;
        half_t  e_phnum;
        half_t  e_shentsize;
        half_t  e_shnum;
        half_t  e_shstrndx;
        

        shdr_t* sheader();
        shdr_t* section(int index);
        
        phdr_t* pheader();
        bool    has_pheader() const;
        phdr_t* program(int index);

        char*   str_table();
    } __attribute__((packed));
*/

int elf32::process_pheaders(elf32::file_t *_file)
{
    int status = 0;
    elf_header_t *header = _file->header();
    
    for (int i = 0; i < header->e_phnum; i++)
    {
        elf32::phdr_t *pheader = header->pheader(i);
        status = elf32::process_pheader(_file, pheader);
        if (IS_ERROR(status))
        {
            return status;
        }
    }

    return status;
}

int elf32::process_pheader(elf32::file_t *_file, elf32::phdr_t *pheader)
{
    int status = 0;

    switch (pheader->p_type)
    {
    case PT_LOAD:
        status = elf32::process_pheader_pt_load(_file, pheader);
        break;
    }

    return status;
}

void elf32::close(elf32::file_t *_file)
{
    kfree(_file->elf_physical_memory);
    kfree(_file);
}
