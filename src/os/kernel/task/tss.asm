section .asm

; void tss_load(int tss_segment);
global tss_load

tss_load:
    push ebp
    mov ebp, esp
    mov ax, [ebp+8]
    ltr ax
    pop ebp
    ret
