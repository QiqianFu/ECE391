/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

/* Initialize the 8259 PIC */
//input:none
//output: none
void i8259_init(void) {
    outb(0xFF, MASTER_8259_DATA);            //set inturrupt mask
    outb(0xFF, SLAVE_8259_DATA);
    outb(ICW1, MASTER_8259_PORT);
    outb(ICW1, SLAVE_8259_PORT);
    outb(ICW2_MASTER, MASTER_8259_DATA);
    outb(ICW2_SLAVE, SLAVE_8259_DATA);
    outb(ICW3_MASTER, MASTER_8259_DATA);
    outb(ICW3_SLAVE, SLAVE_8259_DATA);
    outb(ICW4, MASTER_8259_DATA);
    outb(ICW4, SLAVE_8259_DATA);



    master_mask = 0xFF;//initialize interrupt mask
    slave_mask = 0xFF;

    enable_irq(irqNum);
}

/* Enable (unmask) the specified IRQ */
//input: int irq_num
//output: none
void enable_irq(uint32_t irq_num) {
    if (irq_num > MAX_irq){
        return;
    }
    if (irq_num > MorS){
        slave_mask = slave_mask & ~(1 << (irq_num - 8));
        outb(slave_mask, SLAVE_8259_DATA);
    }
    else{
        master_mask = master_mask & ~(1 << irq_num);
        outb(master_mask, MASTER_8259_DATA);
    }
}

/* Disable (mask) the specified IRQ */
//input: int irq_num
//output: none
void disable_irq(uint32_t irq_num) {
    if (irq_num > MAX_irq){
        return;
    }
    if (irq_num > MorS){
        slave_mask = slave_mask | (1 << (irq_num - 8));
        outb(slave_mask, SLAVE_8259_DATA);
    }
    else{
        master_mask = master_mask | (1 << irq_num);
        outb(master_mask, MASTER_8259_DATA);
    }
}

/* Send end-of-interrupt signal for the specified IRQ */
//input: int irq_num
//output: none
void send_eoi(uint32_t irq_num) {
    if (irq_num > MAX_irq){
        return;
    }
    if (irq_num > MorS){
        outb(EOI | (irq_num - 8), SLAVE_8259_PORT);
        outb(EOI | 2, MASTER_8259_PORT);
        return;
    }
    else{
        outb(EOI | irq_num, MASTER_8259_PORT);
    }
}
