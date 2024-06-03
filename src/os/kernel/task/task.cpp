#include "task.h"
#include "memory.h"
#include "process.h"
#include "status.h"
#include "config.h"

#include "../memory/heap/kheap.h"
#include "../panic.h"
#include "os/terminal/terminal.h"
#include "../kernel.h"
#include "string.h"

#include "../loader/formats/elfloader.h"

extern "C"
void __attribute__((cdecl)) restore_general_purpose_registers(task::registers_t* registers);

extern "C"
void __attribute__((cdecl)) return_task(task::registers_t* registers);

extern "C"
void __attribute((cdecl)) user_registers();

namespace task 
{
    namespace 
    {
        task_t* s_current = nullptr;
        task_t* s_tail = nullptr,* s_head = nullptr;
    }

    void* get_stack_item(task::task_t* task, int index);
}

void task::run_first()
{
    if (!s_head) {
        panic("task::run_first(): not current task");
    }

    task::_switch(s_head);
    return_task(&s_head->registers);
}

int task::_switch(task_t *task)
{
    if (!task)
        return ERROR(EINVARG);
    s_current = task;
    paging::switch_directory(task->page_directory);
    return 0;
}

int task::switch_to_task_page(task::task_t* task) 
{
    user_registers();
    task::_switch(task);
    return 0;
}

task::task_t* task::current() 
{
    return s_current;
}

task::task_t* task::next()
{
    task::task_t* current_task = task::current();

    if (!current_task) {
        return s_head;
    }

    return current_task->next;
}

void task::save_current(interrupt_frame_t* frame)
{
    if (!task::current()) {
        panic("task::save_current(): not current task");
    }

    task::task_t* task    = task::current();

    task->registers.eip   = frame->eip;
    task->registers.cs    = frame->cs;
    task->registers.flags = frame->eflags;
    task->registers.esp   = frame->esp;
    task->registers.ss    = frame->ss;
    task->registers.ebp   = frame->ebp;
    task->registers.ebx   = frame->ebx;
    task->registers.ecx   = frame->ecx;
    task->registers.edi   = frame->edi;
    task->registers.edx   = frame->edx;
    task->registers.esi   = frame->esi;
}

int task::copy_string_from(task::task_t* task, void* virtual_addr, void* physical_addr, int _max)
{
    if (_max >= PAGING_PAGE_SIZE) {
        return ERROR(EINVARG); 
    }

    int res = 0;
    char* tmp = (char*)kzalloc(_max);

    if (!tmp) {
        return ERROR(ENOMEM);
    }

    paging::directory_t* task_directory = task->page_directory;
    uint32_t old_entry = paging::get(task_directory, tmp);
    uint8_t flags     = PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL;

    res = paging::set(task_directory, tmp, (uint32_t)tmp | flags);
    
    if (IS_ERROR(res)) {
        kfree(tmp);
        return res;
    }

    paging::switch_directory(task_directory);
    
    strncpy(tmp, (const char*)virtual_addr, _max);

    res = paging::set(task_directory, tmp, old_entry);
    
    if (IS_ERROR(res)) {
        kfree(tmp);
        return res;
    }

    strncpy((char*)physical_addr, tmp, _max);

    return res;
}

task::task_t* task::create(process::process_id_t pid) 
{
    task_t* task = (task_t*)kzalloc(sizeof(task_t));
    
    if (!task)
        return (task::task_t*)ERROR(EIO);
    
    memset(task, 0, sizeof(task_t));
    task->page_directory = paging::new_four_gb_directory(PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);

    if (!task->page_directory) {
        kfree(task);
        return (task::task_t*)ERROR(EIO);
    }
    
    process::process_t* _process = process::get(pid);

    task->registers.eip = (_process->file_type == process::file_type_t::BINARY) ? PROGRAM_VIRTUAL_ADDRESS : _process->elf_file.header()->e_entry; 
    task->registers.ss  = GDT_USER_DATA_SEGMENT;
    task->registers.cs  = GDT_USER_CODE_SEGMENT;
    task->registers.esp = PROGRAM_VIRTUAL_STACK_ADDRESS_START;
    task->pid       = pid;

    if (s_head == nullptr) {
        s_current = s_head = s_tail = task;
        return s_tail;
    }
    
    s_tail->next = task;
    task->prev = s_tail;
    s_tail = task;
    return s_tail;
}

void task::free(task::task_t* task)  
{
    paging::free(task->page_directory);
    task::remove(task);
    kfree(task);
}

void task::remove(task_t *task)
{
    if (task->prev) {
        task->prev->next = task->next;
    }
    else if (task == s_head) {
        s_head = task->next;
    }
    else if (task == s_tail) {
        s_tail = task->prev;
    }
    else if (task == s_current){
        s_current = task->next ? task->next : s_head;
    }
}

void* task::get_stack_item(task::task_t* task, int index) 
{
    void* result = 0;

    uint32_t* esp = (uint32_t*)task->registers.esp;
    
    task::switch_to_task_page(task);
    result = (void*)esp[index];
    switch_to_kernel_page();

    return result;
}
