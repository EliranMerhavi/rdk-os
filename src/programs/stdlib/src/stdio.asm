[bits 32]

section .asm

; I/O STUFF ;
global putchar: function
global getchar: function

; int getchar();
getchar:
    push ebp 
    mov ebp, esp
    mov eax, 0x30      ; getkey commnad
    int 0x80 
    pop ebp 
    ret

; void putchar(char c);
putchar:
    push ebp 
    mov ebp, esp 
    mov eax, 0x31          ; putchar commnad 
    push dword [ebp+8]
    int 0x80 
    add esp, 4 
    pop ebp 
    ret



; FILE I/O STUFF ;

global fopen: function
global fread: function
global fseek: function
global fstat: function
global fclose: function 

; int fopen(const char* filename, const char* mode);
fopen:
    push ebp
    mov ebp, esp
    push dword [ebp+8]  ;
    push dword [ebp+12] ;
    mov eax, 0xf1       ; fopen command
    int 0x80
    add esp, 8
    pop ebp
    ret

; int fread(void* ptr, uint32_t block_size, uint32_t block_count, int fd);
fread:
    push ebp
    mov ebp, esp
    push dword [ebp+8]
    push dword [ebp+12]
    push dword [ebp+16]
    push dword [ebp+20]
    mov eax, 0xf2       ; fread command
    int 0x80
    add esp, 16
    pop ebp
    ret

; int fseek(int fd, int offset, file_seek_mode_t whence); 
fseek:
    push ebp
    mov ebp, esp
    push dword [ebp+8] ;
    push dword [ebp+8] ;
    push dword [ebp+8] ;
    mov eax, 0xf3      ; fseek command
    int 0x80
    add esp, 4
    pop ebp
    ret

; int fstat(int fd, file_stat_t* stat);
fstat:
    push ebp
    mov ebp, esp
    push dword [ebp+8]  ;
    push dword [ebp+12] ;
    mov eax, 0xf4      ; fstat command
    int 0x80
    add esp, 8
    pop ebp
    ret

; int fclose(int fd);
fclose:
    push ebp
    mov ebp, esp
    push dword [ebp+8] ; push int fd
    mov eax, 0xf5      ; fclose command
    int 0x80
    add esp, 4
    pop ebp
    ret
