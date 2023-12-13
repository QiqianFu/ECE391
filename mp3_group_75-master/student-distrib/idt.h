#ifndef IDT_H
#define IDT_H

#ifndef ASM
    void init_idt();
#endif /* ASM */

#define PIT_VEC 0x20
#define KEYBOARD_VEC 0x21
#define DEVICE_RTC_VEC 0x28
#define SYSTEM_CALL_VEC 0x80
#define MOUSE_VEX       0x2C


#endif /* IDT_H */
