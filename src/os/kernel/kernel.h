#pragma once

extern "C" __attribute__((cdecl)) 
void kernel_registers();
void switch_to_kernel_page();
void kernel_main();

