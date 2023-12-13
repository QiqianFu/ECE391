#ifndef INT_WRAP_H
#define INT_WRAP_H

#include "sig.h"

#ifndef ASM
void interrupt_pit_wrap(void);
void interrupt_rtc_wrap(void);
void interrupt_keyboard_wrap(void);
void interrupt_mouse_wrap(void); //extra mouse
void syscall_wrap(void);         // the eax is actually the input of the id of syscalls
void common_exception_wrap(void);
void division_by_zero_wrap(void);
void single_step_interrupt_wrap(void);
void nmi_wrap(void);
void breakpoint_wrap(void);
void overflow_wrap(void);
void bound_range_exceeded_wrap(void);
void invalid_opcode_wrap(void);
void coprocessor_not_available_wrap(void);
void double_fault_wrap(void);
void coprocessor_segment_overrun_wrap(void);
void invalid_task_state_segment_wrap(void);
void segment_not_present_wrap(void);
void stack_segment_fault_wrap(void);
void general_protection_fault_wrap(void);
void page_fault_wrap(void);
void x87_fp_exception_wrap(void);
void alignment_check_wrap(void);
void machine_check_wrap(void);
void simd_fp_exception_wrap(void);
void raise_sigreturn(void);
void raise_end(void);
#endif
#endif

