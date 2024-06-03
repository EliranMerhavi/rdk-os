#include "rdk_os.h"

extern int main(int argc, char** argv);
arguments_t* get_args();

void c_start() {
    arguments_t* args = get_args();
    int res = main(args->argc, args->argv);
    /* error handling */
    switch (res) 
    {

    }
}
