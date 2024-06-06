#include "stdio.h"
#include "rdk_os.h"

int main(int argc, char** argv)
{
    
    int fd = fopen("0:/hello.txt", "r");

    if (!fd) {
        printf("%s cannot be openned\n", "0:/hello.txt");
        return 0;
    }

    file_stat_t stat; 

    int res = fstat(fd, &stat);

    if (IS_ERROR(res)) {
        fclose(fd);
        return -1;
    }

    printf("file_size: %d\n", stat.filesize);

    fclose(fd); 

    /*
    if (argc != 2) {
        printf("expected 1 argument got %d", argc);        
        return -1;
    }

    const char* filename = argv[1];

    int fd = fopen(filename, "r");

    if (!fd) {
        printf("%s cannot be openned", filename);
        return 0;
    }
    
    file_stat_t stat; 

    res = fstat(fd, &stat);
    if (IS_ERROR(res)) {
        fclose(fd);
        return -1;
    }

    char* data = malloc(stat.filesize);
    
    if (!data) {
        fclose(fd);
        return -1;
    }

    if (fread(data, stat.filesize, 1, fd) != 1) {
        fclose(fd);
        return -1;
    }

    fclose(fd);

    if (!validate(data)) {        
        return -1;
    }

    printf("%s", data);

    return 0;
    */
    return 0;
}
