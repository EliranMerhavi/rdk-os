#include "elf.h"

elf32::shdr_t* elf32::elf_header_t::sheaders()
{
    return (elf32::shdr_t*)((int)this + this->e_shoff);
}

elf32::shdr_t* elf32::elf_header_t::sheader(int index)
{
    return &sheaders()[index];
}

elf32::phdr_t* elf32::elf_header_t::pheaders()
{
    if (!this->has_pheader())
        return nullptr;
    
    return (phdr_t*)((uint32_t)this + this->e_phoff);
}

elf32::phdr_t* elf32::elf_header_t::pheader(int index)
{
    return &pheaders()[index];
}

bool elf32::elf_header_t::has_pheader() const
{
    return this->e_phoff != 0;
}

char* elf32::elf_header_t::str_table()
{
    return (char*)this + sheader(this->e_shstrndx)->sh_offset;
}
