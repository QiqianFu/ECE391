#ifndef _PAGING_H
#define _PAGING_H

#include "types.h"
#include "lib.h"
#include "x86_desc.h"

#define MAXN_IDX 1024 /* The maximum number of indexs for page directory and page table */
#define ALIGN_4KB 4096 /* The align number for 4kb */
#define VMEM_IDX 0xB8 /* The index of video memory, it takes up only one page for now (80 * 25 * 2 < 4096) */
#define SC1 0xBA
#define SC2 0xBB
#define SC3 0xBC
#define KERNEL_ADDR 0x400000 /* 4MB, the start address for kernel code*/
#define V128 0x8000000 /* Virtual address 128MB */
#define V128IDX 0x20 /* Index 32, for V128 address (32 of 4MB page) */
#define _4MB 0x400000
#define _4KB 0x1000
#define _128MB 0x8000000

/* Struct for Page Directory Entry, order is important, see manual page 114-116.
   This structure reserved Dirty & Global bit, so is universal for 4KB pages and 4MB pages */
typedef struct PageDirectoryEntry {
    union{
        uint32_t val;
        struct {
            uint32_t Present_flag       : 1; /* Present flag, 1 bit */
            uint32_t RW_flag            : 1; /* Read/Write flag, 1 bit */
            uint32_t Privilege_flag     : 1; /* User/Supervisor flag, 1 bit */
            uint32_t PWT_flag           : 1; /* Page Write-Through flag, 1 bit */
            uint32_t PCD_flag           : 1; /* Page Cache Disable flag, 1 bit */
            uint32_t Accessed_flag      : 1; /* Accessed flag, 1 bit */
            uint32_t Dirty_flag         : 1; /* Dirty flag, 1 bit, used for extended paging */
            uint32_t Pagesize_flag      : 1; /* Page Size flag, 1 bit, applies only to Page Directory */
            uint32_t Gloabl_flag        : 1; /* Global flag, 1 bit, used for extended paging */            
            uint32_t Available          : 3; /* Remaining bits for the 32 bit patterns */
            uint32_t Field              : 20; /* Field, 20 bits */
        } __attribute__ ((packed));
    };
} PDE_t;

/* Struct for Page Table Entry, order is important, see manual page 114-116 */
typedef struct PageTableEntry {
    union{
        uint32_t val;
        struct {
            uint32_t Present_flag       : 1; /* Present flag, 1 bit */
            uint32_t RW_flag            : 1; /* Read/Write flag, 1 bit */
            uint32_t Privilege_flag     : 1; /* User/Supervisor flag, 1 bit */
            uint32_t PWT_flag           : 1; /* Page Write-Through flag, 1 bit */
            uint32_t PCD_flag           : 1; /* Page Cache Disable flag, 1 bit */
            uint32_t Accessed_flag      : 1; /* Accessed flag, 1 bit */
            uint32_t Dirty_flag         : 1; /* Dirty flag, 1 bit */
            uint32_t PAT_flag           : 1; /* PAT flag, 1 bit, applies only to Page Table */
            uint32_t Gloabl_flag        : 1; /* Global flag, 1 bit */            
            uint32_t Available          : 3; /* Remaining bits for the 32 bit patterns */
            uint32_t Field              : 20; /* Field, 20 bits */
        } __attribute__ ((packed));
    };
} PTE_t;

extern PDE_t PageDirectory[MAXN_IDX];
extern PTE_t PageTable[MAXN_IDX];
extern PTE_t VIDEO_MEM_TABLE[MAXN_IDX];

void Page_init();
void allocate_address(uint32_t entry); 
void map_virtual2phy_table(PTE_t* which_table,int32_t ter_id,int32_t virtual_add,uint32_t pre);

#endif
