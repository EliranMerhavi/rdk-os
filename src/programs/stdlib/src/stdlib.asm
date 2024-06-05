[bits 32]

section .asm

global malloc:function 
global free:function 
global system:function 
global exit:function

; void* malloc(size_t size)
malloc:
    push ebp 
    mov ebp, esp
    push dword [ebp+8]  ; push size_t size
    mov eax, 0x20       ; malloc commnad
    int 0x80 
    add esp, 4
    pop ebp
    ret

; void free(void* ptr)
free:
    push ebp 
    mov ebp, esp
    push dword [ebp+8] ; push void* ptr
    mov eax, 0x21      ; free command 
    int 0x80 
    add esp, 4
    pop ebp 
    ret 

; PROCESS STUFF 


; int system(const char* command);
system:
    push ebp 
    mov ebp, esp
    push dword [ebp+8] ; push const char* command 
    mov eax, 0x22      ; system command 
    int 0x80 
    add esp, 4
    pop ebp 
    ret 

; void exit();
exit:
    push ebp 
    mov ebp, esp 
    mov eax, 0x23
    int 0x80 
    pop ebp 
    ret
