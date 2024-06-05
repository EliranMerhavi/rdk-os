#include "kernel.h"

#include "gdt/gdt.h"

#include "idt/idt.h"
#include "isr/isr.h"
#include "systemcall/systemcall.h"

#include "assert.h"
#include "string.h"

#include "memory/heap/kheap.h"
#include "memory/paging.h"
 
#include "drivers/disk/disk.h"
#include "drivers/disk/disk_stream.h"
#include "drivers/keyboard/keyboard.h"

#include "vfs/fs.h"
#include "vfs/file.h"

#include "task/task.h"
#include "task/process.h"

#include "memory.h"
#include "status.h"
#include "config.h"

#include "panic.h"
#include "debug/debug.h"

#include "os/terminal/terminal.h"

static paging::directory_t* kernel_directory = nullptr;

void kernel_main() 
{
    terminal::init();
    gdt::init();

    idt::init();
    systemcall::init();
    keyboard::init();

    isr::init();

    kheap_init();
    fs::init();

    disk::init();

    kernel_directory = paging::new_four_gb_directory(PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    paging::switch_directory(kernel_directory);
    enable_paging();
     
    process::init();
    debug::init();

    process::process_id_t pid = process::load("0:/shell.elf", nullptr);

    //process::process_id_t pid = process::load("0:/shell.elf", nullptr);

    if (IS_ERROR(pid)) { 
        terminal::printf("error: %d", pid);
        panic("failed to load 0:/shell.elf");
    }

    process::_switch(pid); 
    task::run_first();
    terminal::printf("kernel_main() ends\n");
    // end testing section
}

void switch_to_kernel_page() {
    kernel_registers();
    paging::switch_directory(kernel_directory);
}
