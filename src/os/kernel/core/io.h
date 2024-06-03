#pragma once
#include <stdint.h>


extern "C" unsigned char inb(unsigned short port);
extern "C" unsigned short inw(unsigned short port);

extern "C" void outb(unsigned short port, unsigned char val);
extern "C" void outw(unsigned short port, unsigned short val);

extern "C" void insw(uint16_t port, void *addr, unsigned int count);
extern "C" void outsw(uint16_t port, void *addr, unsigned int count);

namespace io
{
    static inline void outb(uint16_t port, uint8_t val) 
    {
        __asm__ volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) :"memory");
        /* There's an outb %al, $imm8  encoding, for compile-time constant port numbers that fit in 8b.  (N constraint).
     * Wider immediate constants would be truncated at assemble-time (e.g. "i" constraint).
     * The  outb  %al, %dx  encoding is the only option for all other cases.
     * %1 expands to %dx because  port  is a uint16_t.  %w1 could be used if we had the port number a wider C type */
    }

    static inline void outl(uint16_t port, uint32_t val) 
    {
        __asm__ volatile ( "outl %0, %1" : : "a"(val), "Nd"(port) :"memory");
        /* There's an outb %al, $imm8  encoding, for compile-time constant port numbers that fit in 8b.  (N constraint).
     * Wider immediate constants would be truncated at assemble-time (e.g. "i" constraint).
     * The  outb  %al, %dx  encoding is the only option for all other cases.
     * %1 expands to %dx because  port  is a uint16_t.  %w1 could be used if we had the port number a wider C type */
    }

    static inline uint8_t inb(uint16_t port)
    {
        uint8_t res;
        __asm__ volatile ( "inb %1, %0" 
                : "=a"(res)
                : "Nd"(port)
                : "memory");
        return res;
    }

    static inline uint16_t inw(uint16_t port) 
    {
        uint16_t res;
        __asm__ volatile ( "inw %1, %0" 
                : "=a"(res)
                : "Nd"(port)
                : "memory");
        return res;      
    }

    static inline uint32_t inl(uint16_t port)
    {
         uint32_t res;
        __asm__ volatile ( "inl %1, %0" 
                : "=a"(res)
                : "Nd"(port)
                : "memory");
        return res;      
    }

    static inline void insl(uint16_t port, void* buffer, uint32_t n) 
    {
        asm volatile (
            "cld;"
            "repne; insl;"
            : "=D" (buffer), "=c" (n)
            : "d" (port), "0" (buffer), "1" (n)
            : "memory", "cc");
    }

    static inline void wait()
    {
        io::outb(0x80, 0);
    }
}
