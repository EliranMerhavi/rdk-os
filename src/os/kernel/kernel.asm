[bits 32]

global _start
; void global_registers();
global kernel_registers
extern _Z11kernel_mainv ; label for the kernel_main function 

CODE_SEG equ 0x08
DATA_SEG equ 0x10

_start:
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov ebp, 0x00200000
    mov esp, ebp
    
    ; enable the A20 line
    in al, 0x92
    or al, 2
    out 0x92, al
    
    ; remap the master PIC
    mov al, 0b00010001
    out 0x20, al

    mov al, 0x20
    out 0x21, al 

    mov al, 0b00000001
    out 0x21, al

    ; end remap of the master PIC 

    call _Z11kernel_mainv 
    jmp $

kernel_registers:
    mov ax, 0x10 
    mov ds, ax
    mov es, ax
    mov gs, ax
    mov fs, ax
    ret 

times 512-($-$$) db 0
