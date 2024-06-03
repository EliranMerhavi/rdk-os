#pragma once

struct interrupt_frame_t;

void* systemcall_getchar(interrupt_frame_t* frame);
void* systemcall_putchar(interrupt_frame_t* frame);

void* systemcall_fopen(interrupt_frame_t* frame);
void* systemcall_fread(interrupt_frame_t* frame);
void* systemcall_fseek(interrupt_frame_t* frame);
void* systemcall_fstat(interrupt_frame_t* frame);
void* systemcall_fclose(interrupt_frame_t* frame);