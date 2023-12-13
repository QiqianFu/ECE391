#include "paging.h"
#include "process.h"

PDE_t PageDirectory[MAXN_IDX] __attribute__((aligned(ALIGN_4KB)));
PTE_t PageTable[MAXN_IDX] __attribute__((aligned(ALIGN_4KB)));
PTE_t VIDEO_MEM_TABLE[MAXN_IDX] __attribute__((aligned(ALIGN_4KB)));

/* Initialize the paging hierarchy
   input: None
   output: None */
void Page_init() {
    int32_t i;
    /* 0-4MB should set to 4KB size */
    PageDirectory[0].Accessed_flag = 0;
    PageDirectory[0].Available = 0;
    PageDirectory[0].Dirty_flag = 0;
    PageDirectory[0].Field = (((uint32_t)(&PageTable))/_4KB); /* Physical address to page table */
    PageDirectory[0].Gloabl_flag = 0;
    PageDirectory[0].Pagesize_flag = 0; /* Short page frame */
    PageDirectory[0].PCD_flag = 0;
    PageDirectory[0].Present_flag = 1; /* Page table in main memory */
    PageDirectory[0].Privilege_flag = 0; /* Only kernel mode can access */
    PageDirectory[0].PWT_flag = 0;
    PageDirectory[0].RW_flag = 1; /* Both read & write available */

    /* The rest of the page directory entries set to 4MB size */
    for (i = 1; i < MAXN_IDX; i++) {
        PageDirectory[i].Accessed_flag = 0;
        PageDirectory[i].Available = 0;
        PageDirectory[i].Dirty_flag = 0;
        PageDirectory[i].Field = 0; /* Don't map other spaces */
        PageDirectory[i].Gloabl_flag = 0;
        PageDirectory[i].Pagesize_flag = 1; /* Long page frame */
        PageDirectory[i].PCD_flag = 0;
        PageDirectory[i].Present_flag = 0; /* Not present for other spaces */
        PageDirectory[i].Privilege_flag = 0; /* Only kernel mode can access */
        PageDirectory[i].PWT_flag = 0;
        PageDirectory[i].RW_flag = 1; /* Both read & write available */
        if (i == 1) {
            /* Except for 4-8 MB */
            PageDirectory[i].Present_flag = 1;  /* Present */
            PageDirectory[i].Field = (KERNEL_ADDR >> 12); /* Physical address to itself */
        }
    }

    /* Set up the page table */
    for (i = 0; i < MAXN_IDX; i++) {
        PageTable[i].Accessed_flag = 0;
        PageTable[i].Available = 0;
        PageTable[i].Dirty_flag = 0;
        PageTable[i].Field = 0;
        PageTable[i].Gloabl_flag = 0;
        PageTable[i].PAT_flag = 0;
        PageTable[i].PCD_flag = 0;
        PageTable[i].Present_flag = 0; /* Not present for other spaces */
        PageTable[i].Privilege_flag = 0; /* Only kernel mode can access */
        PageTable[i].PWT_flag = 0;
        PageTable[i].RW_flag = 1; /* Both read & write available */
        if (i == VMEM_IDX || i == SC1 || i == SC2 || i == SC3) {
            /* Except for video memory */
            PageTable[i].Present_flag = 1; /* Present */
            PageTable[i].Field = i; /* Physical address to itself */
        }
    }

    /* Register settings
       CR3 -- the physical address of Page Directory
       CR4 bit 4 PSE flag, allow mixed size page
       CR0 bit 31, page enable
    */

    asm volatile(
        "pushl %%eax;               \
         movl  %0, %%eax;           \
         movl  %%eax, %%cr3;        \
         movl  %%cr4, %%eax;        \
         orl   $0x00000010, %%eax;  \
         movl  %%eax, %%cr4;        \
         movl  %%cr0, %%eax;        \
         orl   $0x80000000, %%eax;  \
         movl  %%eax, %%cr0;        \
         popl %%eax;"
        : /*no output*/
        : "r" (&PageDirectory)
        : "%eax"
    );

}

/* void allocate_address(uint32_t entry)
 * Input: uint32_t entry as the PageDirectory Virtual address
 * Ouput: None
 * Description: map the virtual address to VRAM physical address
 */
void allocate_address(uint32_t entry){
    uint32_t i = entry / _4MB; 
    PageDirectory[i].Accessed_flag = 0;
    PageDirectory[i].Available = 0;
    PageDirectory[i].Dirty_flag = 0;
    PageDirectory[i].Field = (((uint32_t)(&VIDEO_MEM_TABLE)) >> 12); /* Physical address to page table */
    PageDirectory[i].Gloabl_flag = 0;
    PageDirectory[i].Pagesize_flag = 0; /* Short page frame */
    PageDirectory[i].PCD_flag = 0;
    PageDirectory[i].Present_flag = 1; 
    PageDirectory[i].Privilege_flag = 1; 
    PageDirectory[i].PWT_flag = 0;
    PageDirectory[i].RW_flag = 1;
    VIDEO_MEM_TABLE[0].Field = VMEM_IDX;
    VIDEO_MEM_TABLE[0].Accessed_flag = 0;
    VIDEO_MEM_TABLE[0].Available = 0;
    VIDEO_MEM_TABLE[0].Dirty_flag = 0;
    VIDEO_MEM_TABLE[0].Gloabl_flag = 0;
    VIDEO_MEM_TABLE[0].PAT_flag = 0;
    VIDEO_MEM_TABLE[0].PCD_flag = 0;
    VIDEO_MEM_TABLE[0].Present_flag = 1;    /* Not present for other spaces */
    VIDEO_MEM_TABLE[0].Privilege_flag = 1;   /* Only user mode can access */
    VIDEO_MEM_TABLE[0].PWT_flag = 0;
    VIDEO_MEM_TABLE[0].RW_flag = 1;         /* Both read & write available */
}

/* void map_virtual2phy_table(PTE_t* which_table,int32_t ter_id,int32_t virtual_add, uint32_t pre)
 * Input: PTE_t* which_table in use, int32_t ter_id works as offset to actual VRAM 0xB8;
 * int32_t virtual_add is the V_Addr should be remapped, pre is the previlege mode
 * Ouput: None
 * Description: map virtual_add to (VideoMEM + (ter_id << 12)) Physical addr
 */

void map_virtual2phy_table(PTE_t* which_table,int32_t ter_id,int32_t virtual_add, uint32_t pre){
    int i = virtual_add /_4KB;
    which_table[i].Accessed_flag = 0;
    which_table[i].Available = 0;
    which_table[i].Dirty_flag = 0;
    which_table[i].Field = 0;
    which_table[i].Gloabl_flag = 0;
    which_table[i].PAT_flag = 0;
    which_table[i].PCD_flag = 0;
    which_table[i].Present_flag = 0; /* Not present for other spaces */
    which_table[i].Privilege_flag = pre ; /* Only kernel mode can access */
    which_table[i].PWT_flag = 0;
    which_table[i].RW_flag = 1; /* Both read & write available */
    // printf("i = %x, ter_id = %d, physica_add = %x\n", i, ter_id, virtual_add);
        /* Except for video memory */
    which_table[i].Present_flag = 1; /* Present */
    which_table[i].Field = VMEM_IDX + ter_id; /* B8,B9,BA are already used  */

    asm volatile( 
        "movl %0, %%eax    \n\t" 
        "movl %%eax, %%cr3 \n\t"
        :
        :"r"(&PageDirectory)
        :"eax"
    ); 
}
