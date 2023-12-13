#ifndef _PIT_H
#define _PIT_H

#include "types.h"
#include "lib.h"


#define PIT_MODE 0x00
#define PIT_DATA_PORT 0x40
#define PIT_CMD_PORT 0x43
#define FREQ_DIVDER 11932 /* frequncy divider for 1.193182MHz to get 100HZ */

void PIT_init();
void PIT_handler();

#endif
