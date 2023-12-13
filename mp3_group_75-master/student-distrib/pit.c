#include "pit.h"
#include "idt.h"
#include "i8259.h"
#include "sched.h"
#include "sig.h"

int32_t PIT_cnt = 0;

/* void PIT_init
 * Input: None
 * Ouput: None
 * Description: set up PIT interrupt to 10ms
 */
void PIT_init() {
    outb(PIT_MODE, PIT_CMD_PORT);
    outb(FREQ_DIVDER % 256, PIT_DATA_PORT);
    outb(FREQ_DIVDER / 256, PIT_DATA_PORT);
    enable_irq(0);
}


/* void PIT_handler
 * Input: None
 * Ouput: None
 * Description: the handler function after catching a PIT interrupt
 */
void PIT_handler() {
    cli();
    PIT_cnt++;
    if (PIT_cnt == 1000) {
        PIT_cnt = 0;
        SendSig(ALARM);
    }
    scheduler();
    send_eoi(0);
    sti();
}
