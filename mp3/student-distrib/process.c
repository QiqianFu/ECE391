#include "process.h"
#include "fs.h"
#include "paging.h"
#include "sig.h"
#include "syscall.h"
#include "terminal.h"
#include "syscall_wrap.h"
#include "lib.h"
#include "sched.h"
#define VIDEO       0xB8000


/* PCB_t getPCBAddrByPID
 * Input:  the current pid
 * Ouput: the address of the pcb of the pid if it does exist, or -1 if fails
 */
PCB_t* getPCBAddrByPID(int32_t pid) {
    if (pid < 0) return NULL;
    return (PCB_t *) (PROGSTART - (1 + pid) * PCB8K);
}

/* uint32_t getPhysAddrByPID
 * Input: the ID of the process that need to get its address
 * Ouput: the physical address of the pid
 */
uint32_t getPhysAddrByPID(int32_t pid) {
    return PROGSTART + pid * BIGPAGE4M;
}

/* int32_t PICAlloc
 * Input: None
 * Ouput: return the number of pid if there's still pid left, or -1 if fails
 */
int32_t PIDAlloc() {
    int32_t i;
    for (i = 0; i < MAXPROCESS; i++) {
        PCB_t *p = getPCBAddrByPID(i);
        if (p->TaskState == 0) {
            return i;
        }
    }
    return -1;
}


/* void pcb_init
 * Input: None
 * Ouput: initalize the PCBs to vacant
 */
void pcb_init(){
    int32_t i, j;
    for(i = 0 ; i < MAXPROCESS; i++){
        PCB_t *p = getPCBAddrByPID(i);
        p->ParentPID = -1;
        p->TaskState = 0 ;
        p->PID = -1; 
        for (j = 0; j < MAXSIG; j++) p->SigQue[j] = NOSIG;
        p->SigHandler[DIV_ZERO] = p->SigHandler[SEGFAULT] = p->SigHandler[INTERRUPT] = SigKill;
        p->SigHandler[ALARM] = p->SigHandler[USER1] = SigIgnore;
        p->SigLock = 0;
    }
}

/* void switching_ter
 * Input: the terminal id user want to switch to
 * Ouput: If success, copy the old terminal video memory to it's store place
 *        and copy the new terminal video memory to B8
 */

void switching_ter(int ter_id){
    if(ter_id != 1 && ter_id != 2 && ter_id !=0){
        return;
    }
    if(CurTID==ter_id){
        return;
    }
    /* sanity check */

    map_virtual2phy_table(PageTable, 0, VIDEO, 0);
    map_virtual2phy_table(VIDEO_MEM_TABLE, 0, 0, 1);

    /* Copy the current VRAM to backup buffer,
        and copy the correct backup buffer to VRAM to switch to a new screen view */
    memcpy((char*)(VIDEO+(CurTID+2)*_4KB), (char*)VIDEO , _4KB);
    memcpy((void*)VIDEO,(void*)(VIDEO+(ter_id+2)*_4KB),_4KB);
    draw_cur(the_terminal[ter_id].x_cor, the_terminal[ter_id].y_cor);
    CurTID = ter_id;

    /* Need a remap for the current SchedID */
    if (SchedID != CurTID) {
        map_virtual2phy_table(PageTable, SchedID + 2, VIDEO, 0);
        map_virtual2phy_table(VIDEO_MEM_TABLE, SchedID + 2, 0, 1);
    }
}
