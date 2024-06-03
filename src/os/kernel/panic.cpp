#include "panic.h"
#include "os/terminal/terminal.h"

void panic(const char* msg)
{
    terminal::print(msg);
    for (;;);
}
