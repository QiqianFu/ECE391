#include "exception.h"
#include "sig.h"
#define VIDEO       0xB8000
#define NUM_COLS    80
#define NUM_ROWS    25
#define ATTRIB      0x3

// static char* video_mem = (char *)VIDEO;
int32_t i;

void division_by_zero(){
    // exception_handler("Division by 0");
    printf("ERROR: Division by 0\n");
    SendSig(DIV_ZERO);
    // for (;;){}
}
void single_step_interrupt(){
    exception_handler("Single-step interrupt");
}
void nmi(){
    exception_handler("NMI");
}
void breakpoint(){
    exception_handler("Breakpoint");
}
void overflow(){
    exception_handler("Overflow");
}
void bound_range_exceeded(){
    exception_handler("Bound Range Exceeded");
}
void invalid_opcode(){
    exception_handler("Invalid Opcode");
}
void coprocessor_not_available(){
    exception_handler("Coprocessor not available");
}
void double_fault(){
    exception_handler("Double Fault");
}
void coprocessor_segment_overrun(){
    exception_handler("Coprocessor Segment Overrun");
}
void invalid_task_state_segment(){
    exception_handler("Invalid Task State Segment");
}
void segment_not_present(){
    exception_handler("Segment not present");
}
void stack_segment_fault(){
    exception_handler("Stack Segment Fault");
}
void general_protection_fault(){
    exception_handler("General Protection Fault");
}
void page_fault(){
    exception_handler("Page Fault");
}
void x87_fp_exception(){
    exception_handler("x87 Floating Point Exception");
}
void alignment_check(){
    exception_handler("Alignment Check");
}
void machine_check(){
    exception_handler("Machine Check");
}
void simd_fp_exception(){
    exception_handler("SIMD Floating-Point Exception");
}

void exception_handler(char* msg){
    printf("ERROR: %s\n", msg);
    SendSig(SEGFAULT);
    // for (;;){}
}

