#include "process.h"

#include "memory.h"
#include "string.h"

#include "status.h"
#include "../panic.h"

#include "../vfs/file.h"
#include "../drivers/keyboard/keyboard.h"

#include "../memory/paging.h"
#include "../memory/heap/kheap.h"
#include "os/terminal/terminal.h"

#include "task.h"

#include "../loader/formats/elfloader.h"
#include "../loader/formats/elf.h"

/*
    using load_function_t = int(*)(const char* filename, process::process_t* _process);
    using map_memory_t    = int(*)(process:process_t* _process);

    struct file_format_interface_t
    {
        load_function_t* load;
        map_memory_t*    map_memory;
    };
    
    struct file_format_interface_t
    {
        virtual int load(const char* filename, process::process_t* _process) = 0;
        virtual int map_memory(process::processs_t* _process) = 0;
    };
*/

namespace process
{
    namespace 
    {
        process_id_t s_current;
        process_t    s_processes[MAX_PROCESSES];
        bool         s_is_taken[MAX_PROCESSES];
    }

    int load_data(const char* filename, process::process_t* _process);
    int load_binary(const char* filename, process::process_t* _process);
    int load_elf(const char* filename, process::process_t* _process);

    int map_memory(process::process_t* _process);
    int map_memory_binary(process::process_t* _process);
    int map_memory_elf(process::process_t* _process);

    void keyboard_listener(const keyboard::event_t& event);
    int load_at(const char* filename, system_arguments_t* args, int slot);
}

process::process_t::process_t() 
{
    memset(this, 0, sizeof(process::process_t));
}

void process::init() 
{
    memset(s_is_taken, false, sizeof(s_is_taken));
    s_current = -1;
    keyboard::insert(keyboard_listener);
}

void process::keyboard_listener(const keyboard::event_t& event)
{
    // TODO: need to explain why process::current()
    // TODO: lecture 82
    
    process::process_t* _process = process::get(process::current());
    
    if (IS_ERROR(_process))
        return;
    
    char key = event.key;

    if (event.state != keyboard::keystate_t::CLICKED)
        return;
    
    _process->keyboard.push(key);   
}

char process::keyboard_pop()
{
    // TODO: need to explain why task::current()
    // TODO: lecture 82
    if (!task::current())
        return 0;

    process::process_t* _process = process::get(task::current()->pid);

    if (_process->keyboard.empty())
        return KEYBOARD_BUFFER_EMPTY; 

    char ch = _process->keyboard.front();   
    _process->keyboard.pop();
    
    return ch;
}

process::process_id_t process::current() 
{
    return s_current;
}

process::process_id_t process::next()
{
    process::process_id_t res;

    for (res = 0; res < MAX_PROCESSES && !s_is_taken[res]; res++)
        ;
    
    if (res == MAX_PROCESSES)
        return -1;

    return res;
}

int process::_switch(process::process_id_t pid)
{
    s_current = pid;
    return 0; 
}

void* process::malloc(process::process_id_t pid, size_t size)
{
    void* ptr = kzalloc(size);
    process::process_t* _process = process::get(pid);

    if (!ptr) {
        return 0;
    }
    
    int index = 0;

    for (index = 0; index < PROGRAM_MAX_ALLOCATIONS && _process->allocations[index].address; index++)
        ;

    if (index == PROGRAM_MAX_ALLOCATIONS)
        return (void*)ERROR(ENOMEM);
    
    int res = paging::map_to(
        _process->task->page_directory, 
        ptr, 
        ptr, 
        paging::align_address((void*)((uint32_t)ptr + size)), 
        PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL | PAGING_IS_WRITEABLE
    );
    
    if (IS_ERROR(res)) {
        kfree(ptr);
        return nullptr;
    }

    _process->allocations[index].address = ptr;
    _process->allocations[index].size    = size;

    return ptr;
}

void process::free(process::process_id_t pid, void *ptr)
{
    process::process_t* _process = process::get(pid);

    int index = 0;

    for (index = 0; index < PROGRAM_MAX_ALLOCATIONS && _process->allocations[index].address != ptr; index++)
        ;

    if (index == PROGRAM_MAX_ALLOCATIONS)
        return;

    int size = _process->allocations[index].size;

    int res = paging::map_to(
        _process->task->page_directory, 
        ptr, 
        ptr, 
        paging::align_address((void*)((uint32_t)ptr + size)), 
        0x00 
    );

    if (IS_ERROR(res))
        return;

    _process->allocations[index].address = nullptr;

    kfree(ptr);
}

process::process_t* process::get(process::process_id_t pid) 
{
    int index = (process_id_t)pid;

    if (index < 0 || index >= MAX_PROCESSES)
        return (process::process_t*)ERROR(EINVARG);

    return &s_processes[index];
}

process::process_id_t process::load(const char* filename, system_arguments_t* args)
{
    int slot;

    for (slot = 0; slot < MAX_PROCESSES && s_is_taken[slot]; slot++)
        ;

    if (slot == MAX_PROCESSES)
        return ERROR(EISTKN);
    
    int res = process::load_at(filename, args, slot);

    if (IS_ERROR(res))
        return res;

    return slot;
}

int process::load_elf(const char* filename, process::process_t* _process)
{
    int res = 0;
    elf32::file_t _file;

    res = elf32::load(filename, &_file);

    if (IS_ERROR(res)) {
        return res;
    }

    _process->file_type = file_type_t::ELF;
    _process->elf_file  = _file;

    return res;
}

int process::load_binary(const char* filename, process::process_t* _process)
{
    int res = 0;

    int fd = fopen(filename, "r");

    if (!fd) {
        return ERROR(EIO);
    }
    
    file_stat_t stat;
    res = fstat(fd, &stat);
    if (IS_ERROR(res)) {
        fclose(fd);
        return res;
    }
    
    void* program_data = kzalloc(stat.filesize);
    
    if (!program_data) {
        fclose(fd);
        return ERROR(ENOMEM);
    }
    
    if (fread(program_data, stat.filesize, 1, fd) != 1) {
        fclose(fd);
        return ERROR(EIO);
    }

    fclose(fd);

    _process->file_type    = file_type_t::BINARY;
    _process->program_ptr  = program_data;
    _process->program_size = stat.filesize; 

    return res;
}

int process::load_data(const char* filename, process::process_t* _process)
{
    int res = 0;
    res = process::load_elf(filename, _process);
    if (!IS_ERROR(res))
        return res;
    res = process::load_binary(filename, _process);

    return res;
}

int process::map_memory_elf(process::process_t* _process)
{
    int res = 0;
   
    elf32::file_t elf_file = _process->elf_file;   
    elf32::elf_header_t* header = elf_file.header();
 
    for (int i = 0; i < header->e_phnum; i++) {
        elf32::phdr_t* phdr = header->pheader(i);

        void* phdr_physical_addr = elf_file.physical_addr(phdr);
        int flags = PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL;

        if (phdr->p_flags & PF_W) {
            flags |= PAGING_IS_WRITEABLE;
        }
        
        res = paging::map_to(
                _process->task->page_directory,
                paging::align_to_lower_page((void*)phdr->p_vaddr),
                paging::align_to_lower_page(phdr_physical_addr),
                paging::align_address((void *)((uint32_t)phdr_physical_addr + phdr->p_memsz)),
                flags);
     
        if (IS_ERROR(res)) {
            break;
        }
    }

    return res;
}

int process::map_memory_binary(process::process_t* _process)
{
    int res = 0;
    res = paging::map_to(
        _process->task->page_directory, 
        (void*)PROGRAM_VIRTUAL_ADDRESS, 
        _process->program_ptr, 
        paging::align_address((void*)((uint32_t)_process->program_ptr + _process->program_size)), 
        PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL | PAGING_IS_WRITEABLE
    );
    return res;
}

int process::map_memory(process::process_t* _process)
{
    int res = 0;

    switch (_process->file_type)
    {
        case file_type_t::ELF:
            res = process::map_memory_elf(_process);
            break;
        case file_type_t::BINARY:
            res = process::map_memory_binary(_process);
            break;
        default:
            panic("process::map_memory(_process): invalid filetype");
            break;
    }

    if (IS_ERROR(res)) {
        return res;
    }

    // mapping stack

    res = paging::map_to(
        _process->task->page_directory,
        PROGRAM_VIRTUAL_STACK_ADDRESS_END,
        _process->stack_ptr,
        paging::align_address((void*)((uint32_t)_process->stack_ptr + USER_PROGRAM_STACK_SIZE)),
        PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL | PAGING_IS_WRITEABLE
    );

    return res;
}

int process::load_at(const char* filename, system_arguments_t* args, int slot) 
{
    int res;
    task::task_t* _task = nullptr;
    
    process_t& _process = s_processes[slot];

    if (slot < 0 || slot >= MAX_PROCESSES) 
        return ERROR(EINVARG);
    
    if (s_is_taken[slot]) {
        return ERROR(EISTKN);
    }

    memset(&_process, 0, sizeof(_process));
    res = process::load_data(filename, &_process);
    
    if (IS_ERROR(res)) {
        return res;
    }
    
    _process.stack_ptr = kzalloc(USER_PROGRAM_STACK_SIZE);
    if (!_process.stack_ptr) {
        return ERROR(ENOMEM);    
    }

    strncpy(_process.filename, filename, sizeof(_process.filename));
    
    _process.id = slot;

    _task = task::create(_process.id);

    if (IS_ERROR(_task)) {
        return ERROR((uint32_t)_task);
    }
    
    _process.task = _task;
    res = process::map_memory(&_process);

    if (IS_ERROR(res)) {
        task::free(_task);
        return res;
    }
    
    if (args) {
        _process.args.argc = args->argc;
    
        for (int i = 0; i < args->argc; i++) {
            _process.args.argv[i] = args->argv[i];
        }
    }
     
    _process.keyboard = circular_queue_t<char, KEYBOARD_BUFFER_SIZE>();
    
    s_is_taken[slot] = true;
    
    return 0;
}

void process::terminate(process::process_id_t pid)
{
    process::process_t* _process = process::get(pid);

    for (int i = 0; i < PROGRAM_MAX_ALLOCATIONS; i++) {
        process::free(pid, _process->allocations[i].address);
    }

    /*
        process_free_program_data(pid)
        TODO: implement in the new way
    */

    kfree(_process->stack_ptr);
    task::free(_process->task);

    s_is_taken[pid] = false;
    
    if (s_current == pid) {
        process::_switch(process::next());
    }
}
