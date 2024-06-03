#include "path.h"
#include "string.h"
#include "memory.h"
#include "config.h"
#include "status.h"
#include "os/kernel/memory/heap/kheap.h"


#include "os/terminal/terminal.h"

namespace path
{
   bool is_valid(const char* path);
}

path::path_t* path::parse(const char* path)
{
    if (!is_valid(path))
        return nullptr;
    
    path::path_t* p = (path::path_t *)kmalloc(sizeof(path::path_t));
    
    // parsing drive number
    p->drive_number = *path - '0';
    path += 3;
    
    const char* start = path; 
    path::part_t* last = p->head = (path::part_t *)kmalloc(sizeof(path::part_t));

    // parsing directories
    while (*path) 
    {
        if (*path != '/') {
            path++;
            continue;
        }

        uint32_t n = path - start;
        last->val = (char *)kmalloc(n + 1);
        last->next = (path::part_t *)kmalloc(sizeof(path::part_t)); 
        memcpy(last->val, start, n);
        last->val[n] = '\0';
        last = last->next;
        start = path + 1;
        while (*path == '/')
            path++;
    }

    uint32_t n = path - start;
    if (!n) {
        last->next = nullptr;
        return p;
    }

    // parsing filename
    last->val = (char *)kmalloc(n + 1);
    memcpy(last->val, start, n);
    last->val[n] = '\0';
    last->next = nullptr;
    return p;
}

void path::free(path::path_t* p)
{
    path::part_t* next = nullptr, *temp = p->head;
    
    while (temp) 
    {
        next = temp->next;
        kfree(temp->val);
        kfree(temp);
        temp = next;
    }

    kfree(p);
}

bool path::is_valid(const char* path)
{
    size_t len = strnlen(path, MAX_PATH); 
    return (len >= 3) && isdigit(path[0]) && !memcmp((void*)(path + 1), ":/", 2);
}

