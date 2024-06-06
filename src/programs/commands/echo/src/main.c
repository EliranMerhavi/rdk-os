#include "stdio.h"
#include "string.h"
#include "rdk_os.h"

int main(int argc, char** argv)
{
    print("(echo not working for now)\n");
    return 0;
    char str[MAX_LINE + 1];
    int pos = 0;

    for (int i = 1; i < argc && pos < MAX_LINE; i++) {
        const char* word = argv[i];
        int j = 0;

        while (word[j] && j < MAX_LINE) {
            str[pos] = word[j];
            j++;
            pos++;
        }
    }

    str[pos] = '\0';

    print(str);
    print("\n");
    
    //printf("%s\n", str);
    
    return 0;
}
