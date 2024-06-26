#include "stdio.h"
#include "stdlib.h"
#include "rdk_os.h"
#include "string.h"
#include "minmax.h"

typedef enum { false, true } bool;

struct {
    bool output_while_typing;
} config;

static char line[MAX_LINE];

void readline(char* line);

int main(int argc, char** argv)
{
    //int status;

    config.output_while_typing = true; 

    do {
        printf(">");
        readline(line);
        system(line);
        // TODO: handle status
    } while (true);

    return 0;
}

void readline(char* line) {
    memset(line, 0, sizeof(line));
    int i = 0; 
    int count = 0;

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

        count = max(0, count + (key != '\b') - (key == '\b'));

        line[i] = key;
    }
    
    line[i] = '\0';

    print("\n");
}