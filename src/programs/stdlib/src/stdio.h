#ifndef STDIO_H
#define STDIO_H
#include <stdint.h>
#include <stddef.h>

typedef enum {
    SEEK_SET,
    SEEK_CUR,
    SEEK_END
} file_seek_mode_t;

typedef enum
{
    FILE_MOD_READ,
    FILE_MOD_WRITE,
    FILE_MOD_APPEND,
    FILE_MOD_INVALID
} file_mode_t;

typedef enum 
{
    FILE_STATE_READ_ONLY = 0b00000001
} file_stat_flags_t;

typedef struct 
{
    uint32_t flags;
    uint32_t filesize;
} file_stat_t;

int printf(const char* format, ...);
int putchar(char c);
int getchar();

int fopen(const char* filename, const char* mode);
int fread(void* ptr, uint32_t block_size, uint32_t block_count, int fd);
int fseek(int fd, int offset, file_seek_mode_t whence); 
int fstat(int fd, file_stat_t* stat);
int fclose(int fd);

#endif
