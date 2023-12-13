#include "syscall.h"
#include "process.h"
#include "fs.h"
#include "rtc.h"
#include "terminal.h"
#include "keyboard.h"
#include "sig.h"
#include "syscall_wrap.h"
#include "sched.h"

int32_t CurPID=-1;
int32_t CurTID=0;

/* int32_t halt
 * Description: terminates a process, returning the specified value to its parent process
 * Input: task status for the execute
 * Return Value: if fails return -1, if success return 0
 *               
*/
int32_t halt (uint8_t arg) {
    cli();
    uint32_t ret_arg = (uint32_t)arg;
    PCB_t *p = getPCBAddrByPID(CurPID);
    if (p->TaskState == 0) {
        sti();
        return -1;
    }
    int32_t i;
    for(i = 0; i < MAXFILE; i++){
        if (p->fdArray[i].File_flags == 1){
            p->fdArray[i].File_flags = 0;
            p->fdArray[i].File_op.Close_ptr(i);
            p->fdArray[i].File_pos = 0;
        }
    }
    p->TaskState = 0;
    the_terminal[SchedID].active_process_ID = p->ParentPID;
    if (p->ParentPID == -1){
        printf("Exit Base Shell Warning\n");
        CurPID = -1;
        sti();
        standard_execute((uint8_t*)"shell");
        return 0;
    }
    int32_t parent_pid = p->ParentPID;
    tss.ss0 = KERNEL_DS;
    tss.esp0 = Eight_MB - PCB8K * parent_pid - 4;
    PageDirectory[directory_index].Field = (getPhysAddrByPID(parent_pid) >> 12);
    // printf("delete PID = %d, restore PID = %d\n", CurPID, parent_pid);
    // printf("restore esp = %x restore ebp = %x\n", esp0, ebp0);

    CurPID = parent_pid;
    asm volatile("movl %0, %%eax    \n\t"
                 "movl %%eax, %%cr3 \n\t"
                  :
                  :"r"(&PageDirectory)
                  :"eax");    

    uint32_t CurEBP;
    asm volatile(
        "movl %%ebp, %0     \n\t"
        "movl %1, %%eax     \n\t"
        :"=r" (CurEBP)
        :"r" (ret_arg)
        :"eax"
    );

    /* Restore the process's hardware context for 'iret' in syscall wrap to get it back! */
    HardwareContext_t *RestoreContext = (HardwareContext_t *)(CurEBP + 20);
    memcpy((void *)RestoreContext, (void *)(&p->ProcessContext), HWSize);
    // RestoreContext->HardwareSS = p->ProcessContext.HardwareSS;
    // RestoreContext->HardwareESP = p->ProcessContext.HardwareESP;
    // RestoreContext->HardwareEFLAGS = p->ProcessContext.HardwareEFLAGS;
    // RestoreContext->HardwareCS = p->ProcessContext.HardwareCS;
    // RestoreContext->ReturnAddr = p->ProcessContext.ReturnAddr;
    sti();
    return 0;
}




/* Execute
 * Description: execute the file input by user
 * Input: command input by user
 * Return Value: if fails return -1, if success return 0
 *               
*/
int32_t execute(const uint8_t* command){
    cli();
    uint8_t file_name_buffer[FILENAME_LEN+1] = {'\0'};
    uint8_t comm_name_buffer[MAX_BUFFER+1] =   {'\0'};
    int32_t i = 0 , j = 0 , k = 0 , t = 0;
    while(command[i] == ' ') i++;
    j = i;
    while(command[j] != '\0' && command[j] != ' ') j++;
    if ( j - i > FILENAME_LEN || j - i == 0 ) return -1;
    t = j; 
    while(command[t] == ' ') t++;
    k = t;
    while(command[k] != '\0' && command[k] != ' ') k++;
    if ( k - t > MAX_BUFFER ) return -1;
    memcpy(file_name_buffer, (char*)(command+i),j-i);
    memcpy(comm_name_buffer, (char*)(command+t),k-t);
    
    Dentry_t Cur_Dentry;
    if (read_dentry_by_name(file_name_buffer, &Cur_Dentry) == -1) {
        printf("Unrecognize file name during syscall execute!\n");
        sti();
        return -1;
    } /* file name sanity check */

    int32_t NewPID = PIDAlloc();
    if (NewPID == -1) {
        printf("Reach maximum process limit!\n");
        sti();
        return -1;
    } /* maximum process number check */
    
    uint8_t buff[5];
    read_data(Cur_Dentry.Inode_num, 0, buff, 4);
    if (buff[0] != 0x7f || buff[1] != 0x45 || buff[2] != 0x4c || buff[3] != 0x46) {
        printf("This isn't an executable!\n");
        sti();
        return -1;
    }
    uint32_t entry_point = 0;
    uint32_t PhysAddr = getPhysAddrByPID(NewPID);
    // printf("NewPID = %d phys addr = %x\n", NewPID, PhysAddr);
    /* Set up corresponding paging */
    
    PageDirectory[V128IDX].Present_flag = 1;
    PageDirectory[V128IDX].Field = (PhysAddr >> 12);
    PageDirectory[V128IDX].Privilege_flag = 1;
    asm volatile(
        "movl %0, %%eax    \n\t" 
        "movl %%eax, %%cr3 \n\t"
        :
        :"r"(&PageDirectory)
        :"eax"
    ); 
    read_data(Cur_Dentry.Inode_num, 24, buff, 4);
    /* Pay attention to the Little endien order */
    for (i = 3; i >= 0; i--) entry_point = (entry_point << 8) + buff[i];
    // printf("entry_point = %x\n", entry_point);
    printf("Create Process PID [%d]\n", NewPID);
    read_data(Cur_Dentry.Inode_num, 0, (uint8_t *)ENTRY, PROGSIZE);
    PCB_t *p = getPCBAddrByPID(NewPID);
    p->TaskState = 1;
    p->PID = NewPID;
    p->ParentPID = CurPID;
    the_terminal[SchedID].active_process_ID = NewPID;
    CurPID = NewPID;
    uint32_t KerEBP;
    asm volatile(
        "movl %%ebp, %0;"
        : "=r" (KerEBP)
        : /* no input */
        : "memory"
    );
    memcpy((void *)(&p->ProcessContext), (void *)(KerEBP + 20), HWSize);
    for (i = 0; i < MAXSIG; i++) p->SigQue[i] = NOSIG;
    p->SigHandler[DIV_ZERO] = p->SigHandler[SEGFAULT] = p->SigHandler[INTERRUPT] = (void *)SigKill;
    p->SigHandler[ALARM] = p->SigHandler[USER1] = (void *)SigIgnore;
    p->SigLock = 0;
    /* set up stdin and stdout */
    p->fdArray[0].File_flags = 1;
    p->fdArray[0].File_inode = Cur_Dentry.Inode_num;
    p->fdArray[0].File_op.Close_ptr = badcall_close;
    p->fdArray[0].File_op.Open_ptr = badcall_open;
    p->fdArray[0].File_op.Read_ptr = ter_read;
    p->fdArray[0].File_op.Write_ptr = badcall_write;
    p->fdArray[0].File_pos = 0;
    p->fdArray[1].File_flags = 1;
    p->fdArray[1].File_inode = Cur_Dentry.Inode_num;
    p->fdArray[1].File_op.Close_ptr = badcall_close;
    p->fdArray[1].File_op.Open_ptr = badcall_open;
    p->fdArray[1].File_op.Read_ptr = badcall_read;
    p->fdArray[1].File_op.Write_ptr = ter_write;
    p->fdArray[1].File_pos = 0;
    // printf("executing %s..\n", Cur_Dentry.FileName);
    /* reserved for cp4, argument take in */
    strcpy((int8_t *)p->arg, (int8_t *)comm_name_buffer);
    tss.ss0 = KERNEL_DS;
    tss.esp0 = Eight_MB - PCB8K * NewPID - 4;

    /* set up correct info for iret to the user program */
    HardwareContext_t *KerContext = (HardwareContext_t *)(KerEBP + 20);
    KerContext->HardwareSS = USER_DS;
    KerContext->HardwareESP = USERSTACK;
    KerContext->HardwareCS = USER_CS;
    KerContext->ReturnAddr = entry_point;
    sti();
    return 0;
}

/*
    int32_t read (int32_t fd, void *buf, int32_t nbytes)
    system call read
    Input: fd: index for the file descriptor; *buf: store the data content;
           nbytes: bytes need to write;
    Return value: Bytes read if success, -1 if fail;
    Side effect: Read the file content into buf;
*/
int32_t read (int32_t fd, void *buf, int32_t nbytes) {
    int32_t res = 0;
    if (fd < 0 || fd >= MAXFILE) return -1;
    /* fd sanity check */
    if (nbytes <= 0) return -1; /* nbytes sanity check */
    PCB_t *p = getPCBAddrByPID(CurPID);
    if (p == NULL) return -1;
    if (p->fdArray[fd].File_op.Read_ptr == NULL) return -1; /* PCB sanity check */
    if (p->fdArray[fd].File_flags == 0) return -1;
    /* check if the file was open (read can't success if not even opened) */
    res = p->fdArray[fd].File_op.Read_ptr(fd, buf, nbytes);
    p->fdArray[fd].File_pos += res;
    return res;
}

/*
    int32_t write (int32_t fd, const void *buf, int32_t nbytes)
    system call write
    Input: fd: index for the file descriptor; *buf: store the data content;
           nbytes: bytes need to write;
    Return value: Bytes written if success, -1 if fail;
    Side effect: Fill the file with buf content;
*/
int32_t write (int32_t fd, const void *buf, int32_t nbytes) {
    if (fd < 0 || fd >= MAXFILE) return -1;
    /* fd sanity check */
    if (nbytes <= 0) return -1; /* nbytes sanity check */
    PCB_t *p = getPCBAddrByPID(CurPID);
    if (p == NULL) return -1;
    if (p->fdArray[fd].File_op.Write_ptr == NULL) return -1; /* PCB sanity check */
    if (p->fdArray[fd].File_flags == 0) return -1;
    /* check if the file was open (write can't success if not even opened) */
    return p->fdArray[fd].File_op.Write_ptr(fd, buf, nbytes);
}
/*
    int32_t open (const uint8_t *filename)
    system call open
    Input: the filename of the file to read
    Return value: the index for the file descriptor if success, -1 if fail;

*/
int32_t open (const uint8_t *filename) {
    Dentry_t Cur_Dentry; 
    int32_t fd;
    if (read_dentry_by_name(filename, &Cur_Dentry) == -1) return -1; /* filename sanity check */
    PCB_t *p = getPCBAddrByPID(CurPID);
    if (p == NULL) return -1; /* PCB sanity check */
    for (fd = 0; fd < MAXFILE; fd++) {
        if (p->fdArray[fd].File_flags == 0) {
            break;
        }
    }
    if (fd == MAXFILE) return -1;
    p->fdArray[fd].File_pos = 0;
    p->fdArray[fd].File_flags = 1;
    p->fdArray[fd].File_inode = Cur_Dentry.Inode_num;
    switch (Cur_Dentry.FileType)
    {
        case FILETYPE_RTC:
            p->fdArray[fd].File_op.Close_ptr = close_rtc;
            p->fdArray[fd].File_op.Open_ptr = open_rtc;
            p->fdArray[fd].File_op.Read_ptr = read_rtc;
            p->fdArray[fd].File_op.Write_ptr = write_rtc;
            break;
                
        case FILETYPE_DIR:
            p->fdArray[fd].File_op.Close_ptr = dir_close;
            p->fdArray[fd].File_op.Open_ptr = dir_open;
            p->fdArray[fd].File_op.Read_ptr = dir_read;
            p->fdArray[fd].File_op.Write_ptr = dir_write;
            break;

        case FILETYPE_REG:
            p->fdArray[fd].File_op.Close_ptr = file_close;
            p->fdArray[fd].File_op.Open_ptr = file_open;
            p->fdArray[fd].File_op.Read_ptr = file_read;
            p->fdArray[fd].File_op.Write_ptr = file_write;
            break;

        default:
            printf("Unrecognize file type during open system call!\n");
            return -1;
    }
    return fd;
}

/*
    int32_t close (int32_t fd)
    system call function
    Input: fd: index for the file descriptor;
    Return value: 0 if success, -1 if fail;
    Side effect: Close the file with index fd;
*/
int32_t close (int32_t fd) {
    if (fd <= 1 || fd >= MAXFILE) return -1;
    /* fd sanity check */
    PCB_t *p = getPCBAddrByPID(CurPID);
    if (p == NULL) return -1;
    if (p->fdArray[fd].File_op.Close_ptr == NULL) return -1; /* PCB sanity check */
    if (p->fdArray[fd].File_flags == 0) return -1;
    /* check if the file was open (close can't success if not even opened) */
    if (p->fdArray[fd].File_op.Close_ptr(fd) == -1) return -1;
    p->fdArray[fd].File_flags = p->fdArray[fd].File_pos = 0;
    return 0;
}

/*
    int32_t getsargs (uint8_t* buf, int32_t nbytes)
    system call function
    Input: *buf: store the data content;
           nbytes: bytes need to write;
    Return value: 0 if success, -1 if fail;
    Side effect: Copy PCB arg into buffer
*/
int32_t getargs (uint8_t* buf, int32_t nbytes) {
    // printf("calling getargs\n");
    if (buf == NULL) return -1; //sanity check
    if(nbytes <= 0) return -1;

    PCB_t *p = getPCBAddrByPID(CurPID);
    if (p == NULL) return -1; /* PCB sanity check */
    if (p->arg[0] == NULL) return -1; //does arg exist
    if (strlen(p->arg) > nbytes) return -1;
    strncpy((int8_t*)buf, (int8_t*)(p->arg), nbytes);
    // printf("buff = %s\n", buf);
    return 0;
}


/* Vidmap: used to map the
 * text-mode video memory into user 
 * space at a pre-set virtual address
 *
 * Input: thte video memory address
 * Output: success or not 
 */
int32_t vidmap (uint8_t** screen_start) {
    printf("vidmap is working\n");
    if (screen_start==NULL) return -1;
    uint8_t** entry_uint8_lowerbound = (uint8_t**) _128MB;
    uint8_t** entry_uint8_upperbound = (uint8_t**) _128MB + PROGSIZE; 
    if (screen_start < entry_uint8_lowerbound || screen_start > entry_uint8_upperbound - 1) return -1;    // check if the screen add starts at 8MB~12MB
    uint32_t entry= (uint32_t)entry_uint8_upperbound;
    allocate_address(entry); 
    asm volatile(
        "movl %0, %%eax    \n\t" 
        "movl %%eax, %%cr3 \n\t"
        :
        :"r"(&PageDirectory)
        :"eax"
    ); 
    *screen_start = (uint8_t*)(entry_uint8_upperbound);
    return 0;
}

/*
    int32_t set_handler (int32_t signum, void* handler_address)
    Description: Modify the default signal handler function for signum
    Input: signum: the type of signal
    Return value: -1 if fail, 0 if success
    Side effect: modify p->SigHandler[signum]
*/
int32_t set_handler (int32_t signum, void* handler_address) {
    PCB_t *p = getPCBAddrByPID(CurPID);
    if (p == NULL) return -1;
    // if (handler_address != NULL) p->SigHandler[signum] = (SigFunc)handler_address;
    if (handler_address != NULL) p->SigHandler[signum] = handler_address;
    else {
        switch (signum) {
            case DIV_ZERO:
            case SEGFAULT:
            case INTERRUPT:
                p->SigHandler[signum] = SigKill;
                break;
            case ALARM:
            case USER1:
                p->SigHandler[signum] = SigIgnore;
                break;
            default:
                printf("Invalid signum! \n");
                return -1;
        }
    }
    return 0;
}

/*
    int32_t sigreturn(void)
    Description: return to the initial place (in user) where sighandler is raise
    Input: None
    Return value: -1 if fail, 0 if success
    Side effect: modify hardware context in kernel stack
*/
int32_t sigreturn (void) {
    HardwareContext_t *KerContext, *PreContext;
    uint32_t KerEBP;
    asm volatile(
        "movl %%ebp, %0;"
        : "=r" (KerEBP)
        : /* no input */
        : "memory"
    );
    KerContext = (HardwareContext_t *)(KerEBP + 20);
    uint32_t UserESP = KerContext->HardwareESP;
    PreContext = (HardwareContext_t *)(UserESP + 4);
    memcpy(KerContext, PreContext, HWSize);
    PCB_t *p = getPCBAddrByPID(CurPID);
    p->SigLock = 0;
    /*
        We use int $0x80 to raise sig return, the two stacks look like:
        Kernel Stack                        User Stack

        -------------                   -------------------
          HWcontextK                     Execute Sigreturn
        -------------                   -------------------
          edx(arg3)                          HWcontextU
        -------------                   -------------------
          ecx(arg2)                            signum
        -------------                   ------------------  <--esp(user)
          ebx(arg1)
        -------------              Here, we need to modify HWcontextK to return back to the origin user program
           ret addr             where cause a signal handler. The correct context is in HWcontextU since we have
        -------------           set up the user stack frame correctly.
           old ebp
        -------------  <--ebp(kernel)
    
    */
    return 0;
}

/*
    badcall_read
    Indicate a bad read system call
    Return -1 only
*/
int32_t badcall_read(int32_t fd, void *buf, int32_t nbytes) {
    return -1;
}

/*
    badcall_write
    Indicate a bad write system call
    Return -1 only
*/
int32_t badcall_write(int32_t fd, const void *buf, int32_t nbytes) {
    return -1;
}

/*
    badcall_open
    Indicate a bad open system call
    Return -1 only
*/
int32_t badcall_open(const uint8_t *filename) {
    return -1;
}

/*
    badcall_close
    Indicate a bad close system call
    Return -1 only
*/
int32_t badcall_close(int32_t fd) {
    return -1;
}
