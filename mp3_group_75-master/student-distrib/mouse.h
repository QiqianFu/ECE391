#ifndef MOUSE_H
#define MOUSE_H
#include "lib.h"

#define data_import     0x60
#define data_auxport    0x64
#define enable_aux_com  0xA8
#define get_compaq      0x20
#define preceding       0xD4
#define set_defaults    0xF6
#define enable_packet   0xF4
#define ack             0xFA
#define x_overflow      0x40
#define y_overflow      0x80
#define x_sign          0x10
#define y_sign          0x20
#define mouse_irq       12
#define MOUSE_CHECK     0x08

extern int32_t x_position;
extern int32_t y_position;

void mouse_init();
void wait_for_read();
void wait_for_send();
void send_command(uint8_t input);
void wait_ack();
void update_position();
void mouse_handler();

#endif
