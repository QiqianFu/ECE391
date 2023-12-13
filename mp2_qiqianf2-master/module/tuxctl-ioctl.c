/* tuxctl-ioctl.c
 *
 * Driver (skeleton) for the mp2 tuxcontrollers for ECE391 at UIUC.
 *
 * Mark Murphy 2006
 * Andrew Ofisher 2007
 * Steve Lumetta 12-13 Sep 2009
 * Puskar Naha 2013
 */

#include <asm/current.h>
#include <asm/uaccess.h>

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/file.h>
#include <linux/miscdevice.h>
#include <linux/kdev_t.h>
#include <linux/tty.h>
#include <linux/spinlock.h>

#include "tuxctl-ld.h"
#include "tuxctl-ioctl.h"
#include "mtcp.h"
#define Four 4



#define debug(str, ...) \
	printk(KERN_DEBUG "%s: " str, __FUNCTION__, ## __VA_ARGS__)
static int ready = 1;
typedef struct botton
{
	uint8_t status ;
	spinlock_t lock ;
}botton;
static botton the_botton;
static int flag;
unsigned long LED_SAVE;
const static unsigned char the_seven_information [16] = {0xE7, 0x06, 0xCB, 0x8F, 0x2E, 0xAD, 0xED, 0x86, 0xEF, 0xAE, 0xEE, 0x6D, 0xE1, 0x4F, 0xE9, 0xE8};
/************************ Protocol Implementation *************************/

/* tuxctl_handle_packet()
 * IMPORTANT : Read the header for tuxctl_ldisc_data_callback() in 
 * tuxctl-ld.c. It calls this function, so all warnings there apply 
 * here as well.
 */

void tuxctl_handle_packet (struct tty_struct* tty, unsigned char* packet)
{
    unsigned a, b, c;
    a = packet[0]; 
    b = packet[1]; 
    c = packet[2];
	switch(a){
//	Packet Format:
//		Byte 0 - MTCP_RESET
//		Byte 1 - reserved
//		Byte 2 - reserved
//  for MTCP_RESET, we just need to init the tux and set the previous time on LED
		case MTCP_RESET: 
			{
				unsigned char print_char[8];
				int i ;
				unsigned long mask_map ;
				print_char[0] = MTCP_BIOC_ON;
				print_char[1] = MTCP_LED_USR;
				print_char[2] = MTCP_LED_SET;
				print_char[3] = 0xFF;

				mask_map = 0x000F;

				for(i = 0 ; i < 4 ; i ++){
					print_char[i+4] = (mask_map & LED_SAVE) >> (i * Four);   //status[i] coresponding to 4 bit from the LED message 
					mask_map = mask_map<<Four;
				}
				tuxctl_ldisc_put(tty,print_char,8);

				return ;
			}



// 	Packet format:
//		Byte 0 - MTCP_BIOC_EVENT
//		byte 1  +-7---4--+--3-+--2-+--1---+---0--+
//			| 1 X X X | C  | B  |  A | START  |
//			+---------+---+---+---+----------+
//		byte 2  +-7---4---+---3---+---2--+--1--+--0--+
//			| 1 X X X | right | down | left | up |
//			+---------+-------+------+------+----+
//      what we want is status = R L D U C B A S, the set botton integer to current botton
		case MTCP_BIOC_EVENT: 	
			{
				uint8_t temp;
				spin_lock_irqsave(&(the_botton.lock),flag);
				the_botton.status = b & 0x0F;		
				temp = 0x00;
				temp |=  ((c & 0x9) << 4);	
				temp |=  ((c & 0x2) << (6-1));			// left at 1 to 6
				temp |=  ((c & 0x4) << (5-2));			// down at 2 to 5 
				the_botton.status |= temp;
				spin_unlock_irqrestore(&the_botton.lock,flag);
				
				return;
			}


//	Response to the tux when the MTC successfully completes a command.
		case MTCP_ACK:
			ready = 1 ;
			return ;
		default:				
			return; 

	}
}

/* tux_ioctl_init
 * Description: Called by user-level code that interact 
 * Input: No user input, tty as default input
 */
int tux_ioctl_init(struct tty_struct* tty){
	unsigned char print_char[2] = {MTCP_BIOC_ON, MTCP_LED_USR};
	if (ready == 0) return 0;
	the_botton.lock = SPIN_LOCK_UNLOCKED;
	the_botton.status = 0xFF;
	ready = 0 ;
	tuxctl_ldisc_put(tty,print_char,2);
	LED_SAVE = 0 ; 
	return 0 ;
}

/* tux_ioctl_button
 * Description: Takes a pinter for currently pressed botton
 * Input: a 32-bit pointer input
 */
int tux_ioctl_buttons(struct tty_struct* tty,unsigned long* pointer){
	if (pointer==NULL) return -EINVAL;
	spin_lock_irqsave(&the_botton.lock,flag);
	*pointer = (unsigned long)the_botton.status;
	spin_unlock_irqrestore(&the_botton.lock,flag);
	return 0;
}
/* tux_ioctl_set_led
 * Description: Specify the led numbers
 * Input: a 32-bit integer
 */
int tux_ioctl_set_led(struct tty_struct* tty, unsigned long arg){
	unsigned char LED_status[4];
	unsigned char message[6];
	uint8_t led_onff ;
	uint8_t de_onoff ;
	uint8_t mask_map2 ; 
	unsigned long mask_map;
	int i;

	mask_map =0x000F;
	if (ready==0) return 0;
	// spin_lock_irqsave(&the_botton)
	ready = 0;
	for(i = 0 ; i < 4 ; i ++){
		LED_status[i] = (mask_map & arg) >> (i * Four);   //status[i] coresponding to 4 bit from the LED message 
		mask_map = mask_map<<Four;
	}
	led_onff = (arg >> 16) & 0xF;			     // which led should be turned on
	de_onoff = (arg >> 24) & 0xF;			     // corresponding decimal points should be turned on


//      long 32-bit arg
//		__7___6___5___4____3______2______1______0___
// 		| X | X | X | X | LED3 | LED2 | LED1 | LED0 | 
// 		----+---+---+---+------+------+------+------+

// 	__7___6___5___4____3___2___1___0__
// 	| A | E | F | dp | G | C | B | D | 
// 	+---+---+---+----+---+---+---+---+ 
	mask_map2 = 0x01;
	for(i = 0 ; i < 4; i ++){						// lenth of 4
		if (led_onff & (mask_map2 << i)){			// led_off is a 4 bit number like 1101
			message[i+2] = the_seven_information[LED_status[i]];
		
			if (de_onoff & ((mask_map2) << i)){
				message[i+2] = message[i+2] | 0x10;
			}
		}else{
			message[i+2] = 0x0;
		}
		
	}
	message[0] = MTCP_LED_SET;  //	Set the User-set LED display values. These will be displayed on the	LED displays when the LED display is in USR mode 
	message[1] = 0x0F;
	tuxctl_ldisc_put(tty,message,6);

	LED_SAVE = arg;
	return 0 ;
		

}
/******** IMPORTANT NOTE: READ THIS BEFORE IMPLEMENTING THE IOCTLS ************
 *                                                                            *
 * The ioctls should not spend any time waiting for responses to the commands *
 * they send to the controller. The data is sent over the serial line at      *
 * 9600 BAUD. At this rate, a byte takes approximately 1 millisecond to       *
 * transmit; this means that there will be about 9 milliseconds between       *
 * the time you request that the low-level serial driver send the             *
 * 6-byte SET_LEDS packet and the time the 3-byte ACK packet finishes         *
 * arriving. This is far too long a time for a system call to take. The       *
 * ioctls should return immediately with success if their parameters are      *
 * valid.                                                                     *
 *                                                                            *
 ******************************************************************************/
int 
tuxctl_ioctl (struct tty_struct* tty, struct file* file, 
	      unsigned cmd, unsigned long arg)
{
    switch (cmd) {
	case TUX_INIT:			return tux_ioctl_init(tty);
	case TUX_BUTTONS:		return tux_ioctl_buttons(tty,(unsigned long* )arg);
	case TUX_SET_LED:		return tux_ioctl_set_led(tty,arg);
	case TUX_LED_ACK:		return 0;
	case TUX_LED_REQUEST:	return 0;
	case TUX_READ_LED: 		return 0;
	default:
	    return -EINVAL;
    }
}
