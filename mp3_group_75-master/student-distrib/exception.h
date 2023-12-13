#ifndef EXCEPTION_H
#define EXCEPTION_H

#ifndef ASM
#include "types.h"
#include "lib.h"
#include "x86_desc.h"
#include "syscall.h"

void division_by_zero();
void single_step_interrupt();
void nmi();
void breakpoint();
void overflow();
void bound_range_exceeded();
void invalid_opcode();
void coprocessor_not_available();
void double_fault();
void coprocessor_segment_overrun();
void invalid_task_state_segment();
void segment_not_present();
void stack_segment_fault();
void general_protection_fault();
void page_fault();
void x87_fp_exception();
void alignment_check();
void machine_check();
void simd_fp_exception();

void exception_handler(char* msg);


/*static char* exception_message[20] = {
    "Division by 0",
    "Single-step interrupt",
    "NMI",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Coprocessor not available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid Task State Segment",
    "Segment not present",
    "Stack Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "x87 Floating Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception"
};*/

#endif /* ASM */

#endif /* EXCEPTION_H */
