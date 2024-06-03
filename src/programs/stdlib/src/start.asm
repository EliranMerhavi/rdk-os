[bits 32]

global _start 
extern c_start
global get_args: function


section .asm
_start:
    call c_start
    ret

; arguments_t* get_args();
get_args:
    push ebp 
    mov ebp, esp 
    mov eax, 0x00
    int 0x80 
    pop ebp 
    ret 
