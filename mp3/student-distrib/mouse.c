#include "mouse.h"
#include "i8259.h"
#include "sig.h"
#include "syscall.h"
#include "process.h"

int32_t width = 80;
int32_t height = 25;
int32_t x_position = 0;
int32_t y_position = 0; 

void wait_for_send(){
    while(data_auxport & 0x2);
}

void wait_for_read(){
    while(data_import & 0x1);
}

void send_command(uint8_t input){
    wait_for_send();
    outb(preceding, data_auxport);
    wait_for_send();
    outb(input, data_import);
}

void wait_ack(){
    uint8_t input = 0;
    while(input != ack){
        input = inb(data_import);
    }
}

void mouse_init(){
    //Aux Input Enable Command
    wait_for_send();
    outb(enable_aux_com, data_auxport);

    //Set Compaq Status/Enable IRQ12
    wait_for_send();
    outb(get_compaq, data_auxport);\
    // wait_ack();
    wait_for_read();
    uint8_t status_byte = inb(data_import);
    status_byte = status_byte | 0x2;
    status_byte = status_byte & 0xDF;
    wait_for_send();
    outb(0x60, data_auxport);
    wait_for_send();
    outb(status_byte, data_import) ;
    // wait_ack();
    send_command(set_defaults);
    send_command(enable_packet);
    // wait_ack();
    x_position = 0;
    y_position = 0;
    //set sample rate
    //set resolution

    enable_irq(mouse_irq);
}

void update_position(){ 
    uint8_t byte_1 = inb(data_import);
    uint8_t byte_2 = inb(data_import);
    uint8_t byte_3 = inb(data_import);
    //overflow check
    if ((byte_1 & x_overflow) || (byte_1 & y_overflow) || (!(byte_1 & MOUSE_CHECK))){
        return;
    }
    //sign check
    if (byte_1 & x_sign){
        byte_2 = byte_2 | 0xFFFFFF00;
    }
    if (byte_1 & y_sign){
        byte_3 = byte_3 | 0xFFFFFF00;
    }

    x_position += byte_2;
    y_position += byte_3;
    if (x_position < 0) x_position = 0;
    if (y_position < 0) y_position = 0;
    if (x_position >= width) x_position = width;
    if (y_position >= height) y_position = height;

    if (byte_1 & 0x1){
        switching_ter((CurTID+1)%3);
    }
    else if (byte_1 & 0x2){
        switching_ter((CurTID+2)%3);
    }
}



void mouse_handler(){
    cli();
    update_position();
    send_eoi(mouse_irq);
    sti();
}
