[org 0x7c00]
[bits 16]

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

KERNEL_BLOCK_COUNT equ 4

jmp short start
nop

; fat 32 header

OEM_identifier:         db 'RDK-OS  '
bytes_per_sector:       dw 512
sectors_per_cluster:    db 0x80
reserved_sectors:       dw 200
fat_copies:             db 0x02
root_dir_entries:       dw 0x40
total_sectors:          dw 0 
media_type:             db 0xF8
sectors_per_fat:        dw 0x100
sectors_per_track:      dw 0x20
number_of_heads:        dw 0x40
hidden_sectors:         dd 0x00
sectors_big:            dd 0x773594

; extended BPB 
ebr_drive_number:       db 0x80                    ; 0x00 floppy, 0x80 hdd, useless
                        db 0                    ; reserved
ebr_signature:          db 0x29
ebr_volume_id:          db 0x12, 0x34, 0x56, 0x78   ; serial number, value doesn't matter
ebr_volume_label:       db 'RDK-OS     '        ; 11 bytes, padded with spaces
ebr_system_id:          db 'FAT16   '           ; 8 bytes


start:
    jmp 0:main

main:
    cli
    mov ax, 0x00
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00
    sti   

.load_protected:
    cli 
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    jmp CODE_SEG:protected_main

; GDT
gdt_start:
gdt_null:
    dd 0
    dd 0
; offset 0x8
gdt_code:
    dw 0xffff 
    dw 0
    db 0
    db 0x9a
    db 0b11001111
    db 0
; offset 0x10
gdt_data:
    dw 0xffff
    dw 0
    db 0
    db 0x92
    db 0b11001111
    db 0
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

[bits 32]
protected_main:
    mov eax, 1
    mov ecx, KERNEL_BLOCK_COUNT
    
    mov edi, 0x0100000

.loop:
    push ecx    
    mov ecx, 250

    call ata_lba_read
    
    pop ecx 
    add edi, 0x1f400
    loop .loop 

    jmp CODE_SEG:0x0100000

ata_lba_read:
    ; params:
    ;   eax - lba adress
    ;   ecs - number of sectors to read
    ;   edi - adress to load the data

    mov ebx, eax ; backup the lba 
    
    ; send the highest 8 bits of the lba to hard disk controller
    shr eax, 24
    or eax, 0xE0 ; select the master drive
    mov dx, 0x1F6
    out dx, al
    ; finished sending the highest 8 bits of the lba

    ; send the total sectors to read
    mov eax, ecx
    mov dx, 0x1F2
    out dx, al
    ; finished sending the total sectors to read
    
    ; send more bits of the lba
    mov eax, ebx ; restore the backup lba
    mov dx, 0x1F3 
    out dx, al
    ; finished sending more bits of the lba

    ; send more bits of the lba
    mov eax, ebx ; restore the backup lba
    mov dx, 0x1F4
    shr eax, 8
    out dx, al
    ; finished sending more bits of the lba
    
    ; send upper 16 bits of the lba
    mov eax, ebx ; restore the lba
    mov dx, 0x1F5
    shr eax, 16
    out dx, al
    ; finished sending upper 16 bits of the lba 

    mov dx, 0x1F7
    mov al, 0x20
    out dx, al
    
    ; read all sectors into memory 
.next_sector:
    push ecx

; checking if we need to read
.try_again:
    mov dx, 0x1F7
    in al, dx
    test al, 8
    jz .try_again 

; we need to read 256 words at a time
    mov ecx, 256
    mov dx, 0x1F0
    rep insw
    pop ecx
    loop .next_sector
; end of reading sectors
    ret

times 510 - ($ - $$) db 0
dw 0xAA55
