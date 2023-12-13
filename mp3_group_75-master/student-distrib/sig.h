#ifndef _SIG_H
#define _SIG_H

#include "lib.h"
#include "types.h"

#define DetectTime 0.2
#define HWSize sizeof(HardwareContext_t) /* HardwareContext struct has 17 uint32_t in total */

typedef enum {
    NOSIG = -1,
    DIV_ZERO,
    SEGFAULT,
    INTERRUPT,
    ALARM,
    USER1,
    MAXSIG
} signal_t;

typedef struct HardwareContext {
    uint32_t HardwareEBX;
    uint32_t HardwareECX;
    uint32_t HardwareEDX;
    uint32_t HardwareESI;
    uint32_t HardwareEDI;
    uint32_t HardwareEBP;
    uint32_t HardwareEAX;
    uint32_t HardwareDS;
    uint32_t HardwareES;
    uint32_t HardwareFS;
    uint32_t IRQVector;
    uint32_t ErrorCode_Dummy;
    uint32_t ReturnAddr;
    uint32_t HardwareCS;
    uint32_t HardwareEFLAGS;
    uint32_t HardwareESP;
    uint32_t HardwareSS;
} HardwareContext_t;

void SendSig(int32_t signum);
void HandleSig();
void SigIgnore(int32_t signum);
void SigKill(int32_t signum);

extern void raise_sigreturn(void);
extern void raise_end(void);

#endif
