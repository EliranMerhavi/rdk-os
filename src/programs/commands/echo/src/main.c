#include "stdio.h"
#include "string.h"

int main(int argc, char** argv)
{
    char str[MAX_LINE + 1];
    int pos = 0;

    for (int i = 0; i < argc && pos < MAX_LINE; i++) {
        const char* word = argv[i];
        int j = 0;

        while (word[j] && j < MAX_LINE) {
            str[pos] = word[j];
            j++;
            pos++;
        }
    }

    str[pos] = '\0';

    printf("%s\n", str);
    
    return 0;
}
