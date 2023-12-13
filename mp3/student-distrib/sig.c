#include "sig.h"
#include "process.h"
#include "syscall.h"
#include "int_wrap.h"
#include "paging.h"
#include "syscall_wrap.h"
#include "terminal.h"

extern int32_t CurPID;
extern int32_t CurSID;

/*
    void SendSig(int32_t signum)
    Description: Send a signal numbered signum to process
    Input: signum: the type of signal
    Return value: None;
    Side effect: Modify p->SigQue
*/
void SendSig(int32_t signum) {
    cli();
    if (signum < 0 || signum > 4 || CurPID < 0) {
        sti();
        return;
    }
    int32_t tmp_pid;
    if (signum == INTERRUPT) tmp_pid = the_terminal[CurTID].active_process_ID;
    else tmp_pid = CurPID;
    PCB_t *p = getPCBAddrByPID(tmp_pid);
    if (p == NULL) {
        sti();
        return;
    }
    // if (p->SigMask[signum] == 0) return;
    int32_t SigIdx;
    for (SigIdx = 0; SigIdx < MAXSIG; SigIdx++) {
        if (p->SigQue[SigIdx] == NOSIG) {
            break;
        }
    }
    if (SigIdx == MAXSIG){
        sti();
        return;
    } /* Above are some sanity checks, and allocate an available signal index */
    // printf("Send Sig SigQue[%d] = %d\n", SigIdx, signum);
    p->SigQue[SigIdx] = signum;
    sti();
}

/*
    void HandleSig
    Description: Handle the process's pending signal (one at a time)
    Input: None
    Return value: None;
    Side effect: Create User Stack frame, call individual signal handler function
*/
void HandleSig() {
    if (CurPID < 0) return;
    PCB_t *p = getPCBAddrByPID(CurPID);
    if (p == NULL) return;
    if (p->SigLock == 1) return;
    /* Sanity checks */
    
    signal_t CurSig = p->SigQue[0];
    if (CurSig != NOSIG) {
        p->SigLock = 1;
        int32_t i;
        uint32_t KerEBP;
        for (i = 1; i < MAXSIG; i++) p->SigQue[i - 1] = p->SigQue[i];
        p->SigQue[MAXSIG - 1] = NOSIG;
        asm volatile(
            "movl %%ebp, %0;"
            : "=r" (KerEBP)
            : /* no input */
            : "memory"
        );
        HardwareContext_t* KerContext = (HardwareContext_t *)(KerEBP + 8);
        uint32_t UserESP = KerContext->HardwareESP;
        if (UserESP < _128MB) {
            p->SigLock = 0;
            p->SigHandler[CurSig](CurSig);
            return;
        }

        uint32_t prog_len = raise_end - raise_sigreturn;
        uint32_t retaddr = UserESP - prog_len;
        memcpy((void *)(UserESP - prog_len), (void *)raise_sigreturn, prog_len); /* Execute Sigreturn */
        memcpy((void *)(UserESP - prog_len - HWSize), (void *)KerContext, HWSize); /* HW context */
        memcpy((void *)(UserESP - prog_len - HWSize - 4), (void *)(&CurSig), 4); /* signum */
        memcpy((void *)(UserESP - prog_len - HWSize - 8), (void *)(&retaddr), 4); /* Entrance to Sigreturn*/
        /* Key part1 setting up User stack frame */
        
        UserESP = UserESP - prog_len - HWSize - 8;
        KerContext->ReturnAddr = (uint32_t)p->SigHandler[CurSig];
        KerContext->HardwareESP = UserESP;
        KerContext->HardwareSS = USER_DS;
        KerContext->HardwareCS = USER_CS;
        /* Key part2 adjusting Kernel Hardware context to go to handler function */
    } /* Detect if signal exists */
}

/*
    void SigIgnore(int32_t signum)
    Description: Default Sginal Ignore handler
    Input: signum: the type of signal
    Return value: None;
    Side effect: None
*/
void SigIgnore(int32_t signum) {
    printf("Default SigIgnore handler called, signum: ");
    switch (signum) {
        case 0: printf("0\n"); break;
        case 1: printf("1\n"); break;
        case 2: printf("2\n"); break;
        case 3: printf("3\n"); break;
        case 4: printf("4\n"); break;
        default: printf("invalid\n"); break;
    }
    return;
}

/*
    void SigKill(int32_t signum)
    Description: Default Sginal Kill handler
    Input: signum: the type of signal
    Return value: None;
    Side effect: Halt the current program
*/
void SigKill(int32_t signum) {
    printf("Default SigKill handler called, signum: ");
    switch (signum) {
        case 0: printf("0\n"); break;
        case 1: printf("1\n"); break;
        case 2: printf("2\n"); break;
        case 3: printf("3\n"); break;
        case 4: printf("4\n"); break;
        default: printf("invalid\n"); break;
    }
    standard_halt(0);
}
