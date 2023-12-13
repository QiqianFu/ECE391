#include "rtc.h"
#include "lib.h"
#include "i8259.h"
#include "sig.h"

#define RTC_PORT_DATA 0x71
#define RTC_PORT_CMD 0x70
#define RTC_REG_B 0x8B
#define RTC_REG_A 0x8A
#define RTC_REG_C 0x0C
#define RTC_FREQ_1024 1024



volatile uint32_t rtc_interrupted = 0;
static uint16_t curr_freq;

/* int32_t log2(int32_t x);
 * Inputs: int32_t x = operand 
 * Return Value: result of log base 2 function on operand
 *  Helper function used in setfreq_rtc */
int32_t log2(int32_t x){
    int32_t result = 0;
    while(x != 1){
        x /= 2;
        result++;
    }
    return result;
}



/* void interrupt_rtc(void);
 * Inputs: None
 * Return Value: None
 *  Function: RTC Interrupt Handler */
void interrupt_rtc(void){
    cli();
    rtc_interrupted = 1;
    //necessary for next interrupt to occur
    outb(RTC_REG_C, RTC_CMD_PORT); //select REG C
    inb(RTC_DATA_PORT); //throw away contents
    send_eoi(RTC_IRQ);
    sti();
}

/* void init_rtc();
 * Inputs: None
 * Return Value: None
 *  Function: Initialize the RTC */
void init_rtc(){
    outb(RTC_REG_B, RTC_CMD_PORT); //select REG B, disable NMI
    unsigned char prev = inb(RTC_DATA_PORT); //read the current value of register B
    outb(RTC_REG_B, RTC_CMD_PORT);        //set the index again (a read will reset the index to register D)
    outb(prev | 0x40, RTC_DATA_PORT);   //write the previous value ORed with 0x40. This turns on bit 6 of register B
    
    curr_freq = MAX_STABLE_FREQ;
    setfreq_rtc(curr_freq);
    enable_irq(RTC_IRQ);
    return;

}

/* void setfreq_rtc(int32_t freq);
 * Inputs: int32_t freq
 * Return Value: None
 *  Function: Set frequency as specified in documentation (see header file) */
void setfreq_rtc(int32_t freq){         
    //frequency = 32768 >> (rate - 1)
    // = 2^15 / 2^(rate - 1)
    unsigned char rate = MAX_RATE - log2(freq) + 1;    
    rate &= 0x0F; //mask bottom 4 bits
    cli(); //disable interrupts
    outb(RTC_REG_A, RTC_CMD_PORT); //select REG A, disable NMI
    unsigned char prev = inb(RTC_DATA_PORT); //get current value in REG A
    outb(RTC_REG_A, RTC_CMD_PORT); //select REG A
    outb((prev & 0xF0) | rate, RTC_DATA_PORT); //concatenate top 4 bits of prev with new rate
    sti(); //enable interrupts
    return;
}

/* int32_t read_rtc(int32_t fd, void* buf, int32_t nbytes);
 * Inputs: int32_t fd, void* buf, int32_t nbytes
 * Return Value: 0 on success
 *  Function: Read RTC */
int32_t read_rtc(int32_t fd, void* buf, int32_t nbytes){
    rtc_interrupted = 0;
    while(rtc_interrupted != 1){
        //wait until interrupted
    }   
    return 0;
}

/* int32_t write_rtc (int32_t fd, const void* buf, int32_t nbytes);
 * Inputs: int32_t fd, void* buf, int32_t nbytes
 * Return Value: 0 on success
 *  Function: Write RTC i.e. set frequency */
int32_t write_rtc (int32_t fd, const void* buf, int32_t nbytes){

    int32_t freq = *((int *) buf);

    cli();
    setfreq_rtc(freq);
    curr_freq = freq;
    sti();

    return 0;
}

/* int32_t open_rtc (const uint8_t* filename);
 * Inputs: const uint8_t* filename (N/A in this context)
 * Return Value: 0 on success
 *  Function: Initialize RTC with frequency 2 */
int32_t open_rtc (const uint8_t* filename){
    setfreq_rtc(2);
    curr_freq = 2;
    return 0;
}

/* int32_t close_rtc (int32_t fd);
 * Inputs: int32_t fd 
 * Return Value: 0 on success
 *  Function: Close RTC */
int32_t close_rtc (int32_t fd){
    //nothing to be done
    return 0;
}
