#ifndef _SYSCALL_H
#define _SYSCALL_H

#include "lib.h"
#include "types.h"
#include "fs.h"
#include "process.h"

#define ENTRY 0x08048000
#define PROGSIZE 0x00400000
#define directory_index 32  
#define Eight_MB 0x800000
#define _128MB 0x8000000
extern int32_t CurPID;
extern int32_t CurTID;
int32_t halt (uint8_t status);
int32_t execute(const uint8_t* command);
int32_t read (int32_t fd, void *buf, int32_t nbytes);
int32_t write (int32_t fd, const void *buf, int32_t nbytes);
int32_t open (const uint8_t *filename);
int32_t close (int32_t fd);
int32_t getargs (uint8_t* buf, int32_t nbytes);
int32_t vidmap (uint8_t** screen_start);
int32_t set_handler (int32_t signum, void* handler_address);
int32_t sigreturn (void);

int32_t badcall_read(int32_t fd, void *buf, int32_t nbytes);
int32_t badcall_write(int32_t fd, const void *buf, int32_t nbytes);
int32_t badcall_open(const uint8_t *filename);
int32_t badcall_close(int32_t fd);
#endif
