#ifndef _RTC_H
#define _RTC_H

#include "lib.h"
#include "syscall.h"

//https://wiki.osdev.org/RTC for details on RTC implementation

#define RTC_DATA_PORT 0x71
#define RTC_CMD_PORT 0x70
#define RTC_REG_C 0x0C
#define RTC_REG_B 0x8B
#define RTC_REG_A 0x8A
#define RTC_IRQ 8

#define MIN_RATE 3
#define MAX_RATE 15
#define MIN_FREQ 2
#define MAX_FREQ 8192
#define MAX_STABLE_FREQ 512

extern void interrupt_rtc();
extern void init_rtc();
void setfreq_rtc(int32_t freq);

int32_t read_rtc(int32_t fd, void* buf, int32_t nbytes);
int32_t write_rtc (int32_t fd, const void* buf, int32_t nbytes);
int32_t open_rtc (const uint8_t* filename);
int32_t close_rtc (int32_t fd);

int32_t log2(int32_t x);



#endif
