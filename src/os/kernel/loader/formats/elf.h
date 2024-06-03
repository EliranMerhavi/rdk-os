#pragma once
#include <stdint.h>
#include <stddef.h>

#define PF_X         0x01 
#define PF_W         0x02 
#define PF_R         0x04 

#define PT_NULL      0 
#define PT_LOAD      1
#define PT_DYNAMIC   2
#define PT_INTERP    3
#define PT_NOTE      4
#define PT_SHLIB     5
#define PT_PHDR      6

#define SHT_NULL     0  
#define SHT_PROGBITS 1
#define SHT_SYMTAB   2
#define SHT_STRTAB   3
#define SHT_RELA     4
#define SHT_HASH     5
#define SHT_DYNAMIC  6
#define SHT_NOTE     7
#define SHT_NOBITS   8
#define SHT_REL      9
#define SHT_SHLIB    10
#define SHT_DYNSYM   11
#define SHT_LOPROC   12
#define SHT_HIPROC   13
#define SHT_LOUSER   14
#define SHT_HIUSER   15

#define ET_NONE      0 
#define ET_REL       1
#define ET_EXEC      2
#define ET_DYN       3
#define ET_CORE      4

#define EI_NIDENT    16
#define EI_CLASS     4
#define EI_DATA      5

#define ELFCLASSNONE 0
#define ELFCLASS32   1
#define ELFCLASS64   2

#define ELFDATANONE  0
#define ELFDATA2LSB  1
#define ELFDATA2MSB  2

#define SHN_UNDEF    0

namespace elf32
{
    using half_t     = uint16_t;
    using word_t     = uint32_t;
    using sword_t    =  int32_t;
    using addr_t     = uint32_t;
    using offset_t   =  int32_t;

    struct phdr_t
    {
        word_t    p_type;
        offset_t  p_offset;
        addr_t    p_vaddr;
        addr_t    p_paddr;
        word_t    p_filesz;
        word_t    p_memsz;
        word_t    p_flags;
        word_t    p_align;
    } __attribute__((packed));

    struct shdr_t 
    {
        word_t    sh_name;
        word_t    sh_type;
        word_t    sh_flags;
        addr_t    sh_addr;
        offset_t  sh_offset;
        word_t    sh_size;
        word_t    sh_link;
        word_t    sh_info;
        word_t    sh_addralign;
        word_t    sh_entsize;
    } __attribute__((packed));

    struct elf_header_t 
    {
        uint8_t  e_ident[EI_NIDENT];
        half_t   e_type;
        half_t   e_machine;
        word_t   e_version;
        addr_t   e_entry;
        offset_t e_phoff;
        offset_t e_shoff;
        word_t   e_flags;
        half_t   e_ehsize;
        half_t   e_phentsize;
        half_t   e_phnum;
        half_t   e_shentsize;
        half_t   e_shnum;
        half_t   e_shstrndx;
        
        shdr_t* sheaders();
        shdr_t* sheader(int index);
        
        phdr_t* pheaders();
        bool    has_pheader() const;
        phdr_t* pheader(int index);
        void* physical_addr(phdr_t* phdr);

        char*   str_table();
    } __attribute__((packed));

    struct dyn_t 
    {
        sword_t d_tag;
        union 
        {
            word_t d_val;
            addr_t d_ptr;
        } d_un;
    } __attribute__((packed));

    struct sym_t 
    {
        word_t  st_name;
        addr_t  st_value;
        word_t  st_size;
        uint8_t st_info;
        uint8_t st_other;
        half_t  st_shndx;
    } __attribute__((packed));
}
