// All necessary declarations for the Tux Controller driver must be in this file

#ifndef TUXCTL_H
#define TUXCTL_H

#define TUX_SET_LED _IOR('E', 0x10, unsigned long)
#define TUX_READ_LED _IOW('E', 0x11, unsigned long*)
#define TUX_BUTTONS _IOW('E', 0x12, unsigned long*)
#define TUX_INIT _IO('E', 0x13)
#define TUX_LED_REQUEST _IO('E', 0x14)
#define TUX_LED_ACK _IO('E', 0x15)

#endif
void tuxctl_handle_packet (struct tty_struct* tty, unsigned char* packet);
int tux_ioctl_init(struct tty_struct* tty);
int tux_ioctl_buttons(struct tty_struct* tty,unsigned long* pointer);
int tux_ioctl_set_led(struct tty_struct* tty, unsigned long arg);
int tuxctl_ioctl (struct tty_struct* tty, struct file* file, unsigned cmd, unsigned long arg);