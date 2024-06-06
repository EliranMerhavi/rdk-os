[bits 32]

section .asm 

global restore_general_purpose_registers
global user_registers
global return_task

; void return_task(task::registers_t* user_registers);
return_task:
    mov ebp, esp
    
    ; 0x103a53

    ; PUSH USER DATA SEGMENT (SS WILL BE FINE)
    ; PUSH USER STACK ADDRESS
    ; PUSH FLAGS
    ; PUSH USER CODE SEGMENT
    ; PUSH USER EIP

    mov ebx, [ebp+4]                        ; ebx = user_registers 
    
    push dword [ebx+44]                     ; push user_registers->ss  
    push dword [ebx+40]                     ; push user_registers->esp
    
    pushf                                   ; push user flags - START
    pop eax 
    or eax, 0x200
    push eax                                ; push user flags - END

    push dword [ebx+32]                     ; push user_registers->cs  
    push dword [ebx+28]                     ; push user_registers->eip

    ; setup segment registers
    mov ax, [ebx+44]                        ; ax = registers->ss        
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push dword [ebp+4]                      ; push user_registers 
    call restore_general_purpose_registers
    add esp, 4
    
    ; enter user land 
    iretd


; void restore_general_purpose_registers(task::registers_t* registers);
restore_general_purpose_registers:
    push ebp
    mov ebp, esp

    mov ebx, [ebp + 8 ] ; ebx = registers->ebx
    mov edi, [ebx     ] ; edi = registers->edi
    mov esi, [ebx + 4 ] ; esi = registers->esi
    mov ebp, [ebx + 8 ] ; ebp = registers->ebp
    mov edx, [ebx + 16] ; edx = registers->edx
    mov ecx, [ebx + 20] ; ecx = registers->ecx
    mov eax, [ebx + 24] ; eax = registers->eax
    mov ebx, [ebx + 12] ; ebx = registers->ebx
    
    add esp, 4 
    ret


; void user_registers();
user_registers:
    mov ax, 0x23
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    ret
