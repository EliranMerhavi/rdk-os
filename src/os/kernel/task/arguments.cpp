#include "arguments.h"

#include "string.h"

#include "../memory/heap/kheap.h"

process::arguments_t::arguments_t(const char* filename) :
    process::arguments_t::arguments_t()
{
    add(filename);
}

process::arguments_t::arguments_t() : m_args_data(), m_c_args()
{
    m_c_args.argc = 0;
    m_c_args.argv = (char**)kzalloc(MAX_ARGS * sizeof(char*));
}


c_arguments_t* process::arguments_t::c_arguments()
{
    return &m_c_args; 
}

void process::arguments_t::add(const char* argument)
{   
    int& argc = m_c_args.argc; 
    strcpy(m_args_data[argc], argument);
    m_c_args.argv[argc] = m_args_data[argc];
    argc++;
}

const char* process::arguments_t::filename() const
{
    return (const char*)m_args_data[0];
}

int process::arguments_t::argc() const
{
    return m_c_args.argc;
}

const char* process::arguments_t::operator[](int i) const
{
    return (const char*)m_args_data[i];
}

void process::arguments_t::on_process_terminate()
{
    for (int i = 0; i < m_c_args.argc; i++) {
        kfree(m_c_args.argv[i]);       
    }

    kfree(m_c_args.argv);
}