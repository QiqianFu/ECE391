#include "terminal.h"
#include "syscall.h"
#include "process.h"
#include "paging.h"
#include "i8259.h"
#include "syscall.h"
#include "syscall_wrap.h"

#define VRAM 0xB8000

/* void scheduler
 * Input: None
 * Ouput: None
 * Description: Switch to process running on next terminal, called by PIT_handler
 */

//References the current terminal in which a particular process is undergoing execution
//intialized to the first terminal
int32_t SchedID=0;

void scheduler(){

    int32_t next_schedid = (SchedID + 1) % TERMINAL_PAGE; //cycle to next terminal
    uint32_t old_ebp, old_esp;
    uint32_t new_ebp, new_esp;

    //sanity check
    if (next_schedid < 0 || next_schedid >= 3)
         return;

    PCB_t *current_process = getPCBAddrByPID(CurPID);

    //check that process exists
    if (current_process){
        
        //save esp and ebp of current process' kernel stack
        asm volatile(
            "movl %%ebp, %0     \n\t"
            "movl %%esp, %1     \n\t"
            : "=r" (old_ebp), "=r" (old_esp)
            : /* no input */
            : "memory"
        );

        current_process->ebp0 = old_ebp;
        current_process->esp0 = old_esp;
        // printf("Before switch PID = %d, stored ebp = %x\n", CurPID, old_ebp);
    }

    //update
    /* the_terminal[SchedID].active_process_ID = CurPID; */
    SchedID = next_schedid;
    CurPID = the_terminal[SchedID].active_process_ID ;

    /*
        If CurTID == SchedID
            Map 0xB8000 (active VRAM ) to itself to display the terminal
        Else
            Map 0xB8000 to one of the three terminal buffers (0xBA, 0xBB, 0xBC)
    */
    if (CurTID == SchedID) {
        map_virtual2phy_table(PageTable, 0, VRAM, 0);
        map_virtual2phy_table(VIDEO_MEM_TABLE, 0, 0, 1);
    }
    else {
        map_virtual2phy_table(PageTable, SchedID + 2, VRAM, 0);
        map_virtual2phy_table(VIDEO_MEM_TABLE, SchedID + 2, 0, 1);
    }
    //no current process in terminal, create base shell
    if (CurPID == -1){
        send_eoi(0);
        sti();
        standard_execute((uint8_t*)"shell");
    }
    else{

        PCB_t *target_process = getPCBAddrByPID(CurPID);
        // printf("CurPID = %d, stored ebp = %x\n", CurPID, target_process->ebp0);
        //check that process exists
         if (target_process){
            //see halt syscall, switch to process in next terminal
            tss.esp0 = Eight_MB - PCB8K * CurPID - 4;
            tss.ss0 = KERNEL_DS;

            PageDirectory[V128IDX].Field = (getPhysAddrByPID(CurPID) >> 12);
            asm volatile(
                "movl %0, %%eax    \n\t" 
                "movl %%eax, %%cr3 \n\t"
                :
                :"r"(&PageDirectory)
                :"eax"
            ); 
            
            //restore esp and ebp of target process' kernel stack
            new_ebp = target_process->ebp0;
            new_esp = target_process->esp0;

            asm volatile(
                "movl %0, %%ebp     \n\t"    
                "movl %1, %%esp      \n\t"
                : /* no output */
                : "r" (new_ebp), "r" (new_esp)
                : "memory"
            );
         }


    }




}
