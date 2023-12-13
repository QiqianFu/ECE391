#ifndef _SYSCALL_WRAP_H
#define _SYSCALL_WRAP_H

#include "types.h"
#define HALT_NUM 1
#define EXECUTE_NUM 2
#define READ_NUM 3
#define WRITE_NUM 4
#define OPEN_NUM 5
#define CLOSE_NUM 6
#define GETARGS_NUM 7
#define VIDMAP_NUM 8
#define SET_HANDLER_NUM 9
#define SIGRETURN_NUM 10

#ifndef ASM
int32_t standard_halt (uint8_t status);
int32_t standard_execute (const uint8_t* command);
int32_t standard_read (int32_t fd, void* buf, int32_t nbytes);
int32_t standard_write (int32_t fd, const void* buf, int32_t nbytes);
int32_t standard_open (const uint8_t* filename);
int32_t standard_close (int32_t fd);
int32_t standard_getargs (uint8_t* buf, int32_t nbytes);
int32_t standard_vidmap (uint8_t** screen_start);
int32_t standard_set_handler (int32_t signum, void* handler);
int32_t standard_sigreturn (void);

#endif /* ASM */
#endif /* _SYSCALL_WRAP_H */
