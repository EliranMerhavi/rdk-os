#include "stdio.h"
#include "stdlib.h"
#include "rdk_os.h"
#include "string.h"

#define MAX_LINE 1024 

typedef enum { false, true } bool;

struct {
    bool output_while_typing;
} config;

char line[MAX_LINE];

void readline(char* line);

int main(int argc, char** argv)
{
    config.output_while_typing = true; 
    int status = 1;

    do {
        print(">");
        memset(line, 0, sizeof(line));
        readline(line);
        int status = system(line);

        /*
            handle status
         */
    } while (status != EXIT);
    
    return 0;
}

void readline(char* line) {
    int i = 0; 

    for (i = 0; i < MAX_LINE - 1; i++) {
        char key = getchar();
        while (!key) {
            key = getchar();
        } 

        if (key == '\n') {
            break;
        }

        if (config.output_while_typing) {
            putchar(key);
        }

        if (key == '\b' && i >= 1) {
            line[i - 1] = '\0';
            i -= 2;
            continue;
        }

        line[i] = key;
    }
    
    line[i] = '\0';

    print("\n");
}
