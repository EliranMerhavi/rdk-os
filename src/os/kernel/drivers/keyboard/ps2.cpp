#include "ps2.h"
#include "keyboard.h"
#include "string.h"

#include "../../isr/isr.h"
#include "../../task/task.h"

#include "../../kernel.h"
#include "../../core/io.h"

#include "os/terminal/terminal.h"

#define is_released_scancode(scancode) (0x80 < scancode && scancode < 0xD9)

#define RELEASED_OFFSET 0x80
#define KEYS_COUNT 0x57

char kbdus[128] =
{
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8',    /* 9 */
    '9', '0', '-', '=', '\b',                         /* Backspace */
    0,                                                /* Tab */
    'q', 'w', 'e', 'r',                               /* 19 */
    't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',     /* Enter key */
    0,                                                /* 29   - Control */
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', /* 39 */
    '\'', '`', 0,                                     /* Left shift */
    '\\', 'z', 'x', 'c', 'v', 'b', 'n',               /* 49 */
    'm', ',', '.', '/', 0,                            /* Right shift */
    '*',
    0,   /* Alt */
    ' ', /* Space bar */
    0,   /* Caps lock */
    0,   /* 59 - F1 key ... > */
    0, 0, 0, 0, 0, 0, 0, 0,
    0, /* < ... F10 */
    0, /* 69 - Num lock*/
    0, /* Scroll Lock */
    0, /* Home key */
    0, /* Up Arrow */
    0, /* Page Up */
    '-',
    0, /* Left Arrow */
    0,
    0, /* Right Arrow */
    '+',
    0, /* 79 - End key*/
    0, /* Down Arrow */
    0, /* Page Down */
    0, /* Insert Key */
    0, /* Delete Key */
    0, 0, 0,
    0, /* F11 Key */
    0, /* F12 Key */
    0, /* All other keys are undefined */
};


namespace ps2
{
    namespace 
    {
        uint8_t keystates[KEYS_COUNT];
        uint8_t caps_flag;
    }

    int  interface_init();
    void irq_handler(interrupt_frame_t* frame);
}


keyboard::keyboard_interface_t ps2::init()
{
    keyboard::keyboard_interface_t res {
        .init = ps2::interface_init
    };

    strcpy(res.name, "PS2");

    return res;
}

int ps2::interface_init()
{
    isr::_register(IRQ1, ps2::irq_handler);
    
    for (uint8_t i = 0; i < KEYS_COUNT; i++) 
    {
        keystates[i] = keyboard::keystate_t::RELEASED;
    }
    caps_flag = 0;
    
    return 0;
}

void ps2::irq_handler(interrupt_frame_t* frame)
{  
    char key;
    uint8_t scancode = 0, released;

    if (!(io::inb(0x64) & 0x01))
        return;
    
    scancode = io::inb(0x60);
    released = is_released_scancode(scancode);
    scancode -=  released * RELEASED_OFFSET;
    key = kbdus[scancode];
    
    uint8_t& curr_state = keystates[scancode];

    // change the state
    curr_state =  released * keyboard::keystate_t::RELEASED + 
                 (!released && curr_state != keyboard::keystate_t::RELEASED) * keyboard::keystate_t::PRESSED + 
                 (!released && curr_state == keyboard::keystate_t::RELEASED) * keyboard::keystate_t::CLICKED;
    
    keyboard::event_t event{key, scancode, (keyboard::keystate_t)curr_state};
    
    keyboard::notify(event);
}
