#define ASM 1

.globl interrupt_pit_wrap   
.globl interrupt_rtc_wrap 
.globl interrupt_keyboard_wrap
.globl interrupt_mouse_wrap # extra mouse
.globl syscall_wrap
.globl common_exception_wrap
.globl division_by_zero_wrap
.globl single_step_interrupt_wrap
.globl nmi_wrap
.globl breakpoint_wrap
.globl overflow_wrap
.globl bound_range_exceeded_wrap
.globl invalid_opcode_wrap
.globl coprocessor_not_available_wrap
.globl double_fault_wrap
.globl coprocessor_segment_overrun_wrap
.globl invalid_task_state_segment_wrap
.globl segment_not_present_wrap
.globl stack_segment_fault_wrap
.globl general_protection_fault_wrap
.globl page_fault_wrap
.globl x87_fp_exception_wrap
.globl alignment_check_wrap
.globl machine_check_wrap
.globl simd_fp_exception_wrap
.globl raise_sigreturn
.globl raise_end
# 0x00 division_by_zero
# 0x01 single_step_interrupt
# 0x02 nmi
# 0x03 breakpoint
# 0x04 overflow
# 0x05 bound_range_exceeded
# 0x06 invalid_opcode
# 0x07 coprocessor_not_available
# 0x08 double_fault
# 0x09 coprocessor_segment_overrun
# 0x0A invalid_task_state_segment
# 0x0B segment_not_present
# 0x0C stack_segment_fault
# 0x0D general_protection_fault
# 0x0E page_fault
# 0x10 x87_fp_exception
# 0x11 alignment_check
# 0x12 machine_check
# 0x13 simd_fp_exception

exception_table:
        .long division_by_zero
        .long single_step_interrupt
        .long nmi
        .long breakpoint
        .long overflow
        .long bound_range_exceeded
        .long invalid_opcode
        .long coprocessor_not_available
        .long double_fault
        .long coprocessor_segment_overrun
        .long invalid_task_state_segment
        .long segment_not_present
        .long stack_segment_fault
        .long general_protection_fault
        .long page_fault
        .long 0x00
        .long x87_fp_exception
        .long alignment_check
        .long machine_check
        .long simd_fp_exception

common_exception_wrap:
        pushl %fs
        pushl %es
        pushl %ds
        pushl %eax
        pushl %ebp
        pushl %edi
        pushl %esi
        pushl %edx
        pushl %ecx
        pushl %ebx
        movl 40(%esp), %ebx
        call *exception_table(,%ebx,4)
        call HandleSig
        popl %ebx
        popl %ecx
        popl %edx
        popl %esi
        popl %edi
        popl %ebp
        popl %eax
        popl %ds
        popl %es
        popl %fs
        addl $8, %esp # pop out IRQ vec & dummy / error code
        iret

division_by_zero_wrap:
        addl $-4, %esp # dummy
        pushl $0x00
        jmp common_exception_wrap

single_step_interrupt_wrap:
        addl $-4, %esp # dummy
        pushl $0x01
        jmp common_exception_wrap

nmi_wrap:
        addl $-4, %esp # dummy
        pushl $0x02
        jmp common_exception_wrap

breakpoint_wrap:
        addl $-4, %esp # dummy
        pushl $0x03
        jmp common_exception_wrap

overflow_wrap:
        addl $-4, %esp # dummy
        pushl $0x04
        jmp common_exception_wrap

bound_range_exceeded_wrap:
        addl $-4, %esp # dummy
        pushl $0x05
        jmp common_exception_wrap

invalid_opcode_wrap:
        addl $-4, %esp # dummy
        pushl $0x06
        jmp common_exception_wrap

coprocessor_not_available_wrap:
        addl $-4, %esp # dummy
        pushl $0x07
        jmp common_exception_wrap

double_fault_wrap:
        pushl $0x08
        jmp common_exception_wrap

coprocessor_segment_overrun_wrap:
        addl $-4, %esp # dummy
        pushl $0x09
        jmp common_exception_wrap

invalid_task_state_segment_wrap:
        pushl $0x0A
        jmp common_exception_wrap

segment_not_present_wrap:
        pushl $0x0B
        jmp common_exception_wrap

stack_segment_fault_wrap:
        pushl $0x0C
        jmp common_exception_wrap

general_protection_fault_wrap:
        pushl $0x0D
        jmp common_exception_wrap

page_fault_wrap:
        pushl $0x0E
        jmp common_exception_wrap

x87_fp_exception_wrap:
        addl $-4, %esp # dummy
        pushl $0x10
        jmp common_exception_wrap

alignment_check_wrap:
        pushl $0x11
        jmp common_exception_wrap

machine_check_wrap:
        addl $-4, %esp # dummy
        pushl $0x12
        jmp common_exception_wrap

simd_fp_exception_wrap:
        addl $-4, %esp # dummy
        pushl $0x13
        jmp common_exception_wrap

interrupt_pit_wrap:
        addl $-4, %esp # dummy
        pushl $0x20
        pushl %fs
        pushl %es
        pushl %ds
        pushl %eax
        pushl %ebp
        pushl %edi
        pushl %esi
        pushl %edx
        pushl %ecx
        pushl %ebx                                      
        call PIT_handler
        call HandleSig                   
        popl %ebx
        popl %ecx
        popl %edx
        popl %esi
        popl %edi
        popl %ebp
        popl %eax
        popl %ds
        popl %es
        popl %fs
        addl $8, %esp # pop out IRQ vec & dummy / error code
        iret

interrupt_rtc_wrap:
        addl $-4, %esp # dummy
        pushl $0x28
        pushl %fs
        pushl %es
        pushl %ds
        pushl %eax
        pushl %ebp
        pushl %edi
        pushl %esi
        pushl %edx
        pushl %ecx
        pushl %ebx                                      
        call interrupt_rtc    
        call HandleSig                   
        popl %ebx
        popl %ecx
        popl %edx
        popl %esi
        popl %edi
        popl %ebp
        popl %eax
        popl %ds
        popl %es
        popl %fs
        addl $8, %esp # pop out IRQ vec & dummy / error code
        iret

interrupt_keyboard_wrap: #extra mouse
        addl $-4, %esp # dummy
        pushl $0x21
        pushl %fs
        pushl %es
        pushl %ds
        pushl %eax
        pushl %ebp
        pushl %edi
        pushl %esi
        pushl %edx
        pushl %ecx
        pushl %ebx                                      
        call keyboard_handler    
        call HandleSig                    
        popl %ebx
        popl %ecx
        popl %edx
        popl %esi
        popl %edi
        popl %ebp
        popl %eax
        popl %ds
        popl %es
        popl %fs
        addl $8, %esp # pop out IRQ vec & dummy / error code
        iret

interrupt_mouse_wrap:
        addl $-4, %esp # dummy
        pushl $0x2C
        pushl %fs
        pushl %es
        pushl %ds
        pushl %eax
        pushl %ebp
        pushl %edi
        pushl %esi
        pushl %edx
        pushl %ecx
        pushl %ebx                                      
        call mouse_handler    
        call HandleSig                    
        popl %ebx
        popl %ecx
        popl %edx
        popl %esi
        popl %edi
        popl %ebp
        popl %eax
        popl %ds
        popl %es
        popl %fs
        addl $8, %esp # pop out IRQ vec & dummy / error code
        iret

# 1. int32 t halt (uint8 t status);
# 2. int32 t execute (const uint8 t* command);
# 3. int32 t read (int32 t fd, void* buf, int32 t nbytes);
# 4. int32 t write (int32 t fd, const void* buf, int32 t nbytes);
# 5. int32 t open (const uint8 t* filename);
# 6. int32 t close (int32 t fd);
# 7. int32 t getargs (uint8 t* buf, int32 t nbytes);
# 8. int32 t vidmap (uint8 t** screen start);
# 9. int32 t set handler (int32 t signum, void* handler address);
# 10. int32 t sigreturn (void);
# the systell call table list
syscall_table:
        .long 0x00
        .long halt
        .long execute
        .long read 
        .long write 
        .long open
        .long close
        .long getargs 
        .long vidmap 
        .long set_handler 
        .long sigreturn

syscall_wrap:
        addl $-4, %esp # dummy
        pushl $0x80
        pushl %fs
        pushl %es
        pushl %ds
        pushl %eax
        pushl %ebp
        pushl %edi
        pushl %esi
        pushl %edx
        pushl %ecx
        pushl %ebx                
        cmpl $0, %eax  
        jle error_syscall_id
        cmpl $10, %eax
        jg  error_syscall_id
        je sigret_wrap
        pushl %edx
        pushl %ecx
        pushl %ebx # push 3 arguments
        call *syscall_table(,%eax,4)
        addl $12, %esp # pop out arguements
        popl %ebx
        popl %ecx
        popl %edx
        popl %esi
        popl %edi
        popl %ebp
        addl $4, %esp # we don't need the old eax value
        popl %ds
        popl %es
        popl %fs
        addl $8, %esp # pop out IRQ vec & dummy / error code                    
        iret    

error_syscall_id:
        movl $-1,%eax
        popl %ebx
        popl %ecx
        popl %edx
        popl %esi
        popl %edi
        popl %ebp
        addl $4, %esp
        popl %ds
        popl %es
        popl %fs
        addl $8, %esp # pop out IRQ vec & dummy / error code                                
        iret

sigret_wrap:
        pushl %edx
        pushl %ecx
        pushl %ebx # push 3 arguments
        call *syscall_table(,%eax,4)
        addl $12, %esp # pop out arguements
        popl %ebx
        popl %ecx
        popl %edx
        popl %esi
        popl %edi
        popl %ebp
        popl %eax # for sigret we need the origin value of eax
        popl %ds
        popl %es
        popl %fs
        addl $8, %esp # pop out IRQ vec & dummy / error code                    
        iret


raise_sigreturn:
        movl $10, %eax
        int $0x80
        ret
raise_end:
