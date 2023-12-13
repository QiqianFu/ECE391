## checkpoint 1 
1. forgot to uncomment the sti() in kernel              takes 2 hours to solve 
2. forgot to initilize keyboard interrupt in kerne      takes 45 mins to solve
3. forgot to reset the priority of the keyboard to 0, which is the highest priority takes 20 mins


## checkpoint 2
1. While calculating the a point to a struct, forgot to convert appropriate type
2. infinite loop in logarithm base 2 helper function --> wasn't changing while loop condition variable (1 hour)

## checkpoint 3
1. Didn't mention the "Little Endian" way of storing data, which lead to issues in calculating entry address and doing executable sanity check
2. Make a mistake to enable the paging for Virtual address 128MB, I use index 128 instead of index 32. It causes page fault. (the big page is 4MB in size, so it should be 128MB / 4MB == 32)
3. Forget to flush tlb in execute (but remeber to do it in halt), it causes a bug at a weird stage, and it is so hard to find out.

## checkpoint 4
1. Don't realize that the screen_start should be set to the exact same page of the virtual address that setted the video memory page table

## checkpoint 5
1. Forget to remap the virtual entry memory (128 MB) back to the correct program image in scheduler
2. The cursor and keyboard text information used to be a mess.
    Bug fixed1: The way of printing a cursor is so different from printing a char (VGA). Need to redraw it after
        every terminal swtich.
    Bug fixed2: It is important to be clear of Something you want to print in the screen and something you want
        to print in a program. For keyboard, we need to print the message on this terminal, so we should modify
        terminal info with "CurTID"; but for functions like ter-write that is raised in a syscall write, it should modify the process's own screen, so use SchedID in those functions instead.
3. Forget to remap vidmap for user-program