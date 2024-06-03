#pragma once

#include <stdint.h>

namespace keyboard
{
    enum keystate_t
    {
        RELEASED,
        PRESSED,
        CLICKED
    };

    struct event_t
    {
        char key;
        uint8_t scancode;
        keystate_t state;
    };

    using listener_t = void (*)(const keyboard::event_t& event);
    
    using keyboard_init_function_t = int(*)();

    struct keyboard_interface_t
    {
        keyboard_init_function_t init;
        char name[200];
    };


    void init();
    int insert(const keyboard_interface_t& interface);

    void insert(const listener_t& listener);
    void notify(const event_t& event);
}
