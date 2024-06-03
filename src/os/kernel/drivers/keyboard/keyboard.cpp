#include "keyboard.h"

#include "../../task/process.h"
#include "../../panic.h"

#include "ps2.h"

namespace keyboard
{
    namespace 
    {
        keyboard::keyboard_interface_t s_interfaces[MAX_KEYBOARD_DRIVERS];
        int                            s_keyboard_count;

        keyboard::listener_t           s_listeners[MAX_KEYBOARD_LISTENERS];
        int                            s_listeners_count;
    }
}


void keyboard::init()
{
    s_keyboard_count = 0;
    s_listeners_count = 0;

    keyboard::insert(ps2::init());
}

int keyboard::insert(const keyboard_interface_t& interface)
{
    if (s_keyboard_count == MAX_KEYBOARD_DRIVERS)
        panic("keyboard::insert(const keyboard_interface_t& interface): max keyboard interfaces reached");
    
    s_interfaces[s_keyboard_count] = interface;
    s_keyboard_count++;

    return interface.init();
}

void keyboard::insert(const keyboard::listener_t& listener)
{
    if (s_listeners_count == MAX_KEYBOARD_LISTENERS)
        panic("keyboard::insert(const listener_t& listener): max keyboard interface reached");
    
    s_listeners[s_listeners_count] = listener;
    s_listeners_count++;
}

void keyboard::notify(const keyboard::event_t& event)
{
    for (int i = 0; i < s_listeners_count; i++) {
        s_listeners[i](event);
    }
}

