[bits 32]

section .asm

global print:function
global rdk_os_exit:function
global execute:function

; void print(const char* str)
print:
    push ebp
    mov ebp, esp
    push dword [ebp+8]
    mov eax, 0x10 ; print command
    int 0x80
    add esp, 4
    pop ebp
    ret

