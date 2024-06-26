[bits 32]
global gdt_load
gdt_load:   
    ; make new call frame
    push ebp             ; save old call frame
    mov ebp, esp         ; initialize new call frame

    cli
    
    ; load gdt
    mov eax, [ebp + 8]
    lgdt [eax]
    
    ; reload code segment
    mov eax, [ebp + 12]
    push eax
    push .reload_cs
    retf

.reload_cs:
    ; reload data segments
    mov ax, [ebp + 16]
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; restore old call frame
    mov esp, ebp
    pop ebp
    ret
