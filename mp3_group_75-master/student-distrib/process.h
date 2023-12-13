#ifndef _PROCESS_H
#define _PROCESS_H

#include "fs.h"
#include "lib.h"
#include "types.h"
#include "sig.h"

#define MAXFILE 8
#define MAXARG 128
#define PCB8K 8192
#define MAXPROCESS 6
#define PROGSTART 0x00800000
#define BIGPAGE4M 0x00400000
#define USERSTACK (0x08400000 - 4)

typedef void (*SigFunc) (int32_t);
typedef struct PCB {
    int32_t PID;
    int32_t ParentPID;
    uint8_t TaskState;
    uint32_t esp0;
    uint32_t ebp0;
    int8_t arg[MAXARG];
    HardwareContext_t ProcessContext;
    signal_t SigQue[MAXSIG];
     /* The ordered queue for signals, up to MAXSIG but different meaning */
    SigFunc SigHandler[MAXSIG]; /* Function pointer for signal handler */
    int8_t SigLock;
    FileDescriptor_t fdArray[MAXFILE];

} PCB_t;

PCB_t* getPCBAddrByPID(int32_t pid);
uint32_t getPhysAddrByPID(int32_t pid);
int32_t PIDAlloc();
void pcb_init();
void switching_ter(int ter_id);
#endif
