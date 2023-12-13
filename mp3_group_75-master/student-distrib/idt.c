#include "idt.h"
#include "exception.h"
#include "int_wrap.h"


/*  Initialize the IDT */
//input: none
//Initializes each interrupt descriptor entry
// according to Intel Docs Chapter 5.11
//output: none
void init_idt(){
    int i;

    for (i = 0; i < NUM_VEC; ++i){              //NUM_VEC = 256
        idt[i].seg_selector = KERNEL_CS;        //initialize to kernel code kernel space
        idt[i].reserved4 = 0;
        if (0x20 <= i && i <= 0x2f) idt[i].reserved3 = 0;
        else idt[i].reserved3 = 1;
        idt[i].reserved2 = 1; 
        idt[i].reserved1 = 1;
        idt[i].reserved0 = 0;
        idt[i].present = 1;

        idt[i].size = 1;                        //handlers are 32 bit, not 16 bit

        if (i == SYSTEM_CALL_VEC)
            idt[i].dpl = 3;                     //privilege 3
        else 
            idt[i].dpl = 0;                     //privilege 0
    }

    SET_IDT_ENTRY(idt[0x00], division_by_zero_wrap);
    SET_IDT_ENTRY(idt[0x01], single_step_interrupt_wrap);
    SET_IDT_ENTRY(idt[0x02], nmi_wrap);
    SET_IDT_ENTRY(idt[0x03], breakpoint_wrap);
    SET_IDT_ENTRY(idt[0x04], overflow_wrap);
    SET_IDT_ENTRY(idt[0x05], bound_range_exceeded_wrap);
    SET_IDT_ENTRY(idt[0x06], invalid_opcode_wrap);
    SET_IDT_ENTRY(idt[0x07], coprocessor_not_available_wrap);
    SET_IDT_ENTRY(idt[0x08], double_fault_wrap);
    SET_IDT_ENTRY(idt[0x09], coprocessor_segment_overrun_wrap);
    SET_IDT_ENTRY(idt[0x0A], invalid_task_state_segment_wrap);
    SET_IDT_ENTRY(idt[0x0B], segment_not_present_wrap);
    SET_IDT_ENTRY(idt[0x0C], stack_segment_fault_wrap);
    SET_IDT_ENTRY(idt[0x0D], general_protection_fault_wrap);
    SET_IDT_ENTRY(idt[0x0E], page_fault_wrap);
    // idt[0x0F] reserved
    SET_IDT_ENTRY(idt[0x10], x87_fp_exception_wrap);
    SET_IDT_ENTRY(idt[0x11], alignment_check_wrap);
    SET_IDT_ENTRY(idt[0x12], machine_check_wrap);
    SET_IDT_ENTRY(idt[0x13], simd_fp_exception_wrap);
    // idt[0x14 to 0x1F] reserved 
    SET_IDT_ENTRY(idt[SYSTEM_CALL_VEC], syscall_wrap);
    SET_IDT_ENTRY(idt[PIT_VEC], interrupt_pit_wrap);


    SET_IDT_ENTRY(idt[DEVICE_RTC_VEC], interrupt_rtc_wrap);
    SET_IDT_ENTRY(idt[KEYBOARD_VEC], interrupt_keyboard_wrap);
    SET_IDT_ENTRY(idt[MOUSE_VEX], interrupt_mouse_wrap);
}

