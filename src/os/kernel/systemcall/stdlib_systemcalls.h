#pragma once

struct interrupt_frame_t;

void* systemcall_malloc(interrupt_frame_t* frame);
void* systemcall_free(interrupt_frame_t* frame);
void* systemcall_system(interrupt_frame_t* frame);
void* systemcall_exit(interrupt_frame_t* frame);
