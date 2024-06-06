#pragma once 
#include "config.h"

struct c_arguments_t {
    int argc;
    char **argv;
};

namespace process
{
    class arguments_t {
    public:
        arguments_t();
        arguments_t(const char* filename);

        void add(const char* argument);

        c_arguments_t* c_arguments();

        const char* filename() const;
        int argc() const;

        const char* operator[](int i) const;

        void on_process_terminate();
    private:
        char m_args_data[MAX_ARGS][MAX_ARG_SIZE + 1];
        c_arguments_t m_c_args;
    };
}

