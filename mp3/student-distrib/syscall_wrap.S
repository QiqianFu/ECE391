#define ASM 1

#include "syscall_wrap.h"

#define WRAP_CALL(name,number)   \
.GLOBL name                   ;\
name:   PUSHL	%EBX          ;\
	MOVL	$number,%EAX  ;\
	MOVL	8(%ESP),%EBX  ;\
	MOVL	12(%ESP),%ECX ;\
	MOVL	16(%ESP),%EDX ;\
	INT	$0x80         ;\
	POPL	%EBX          ;\
	RET

WRAP_CALL(standard_halt,HALT_NUM)
WRAP_CALL(standard_execute,EXECUTE_NUM)
WRAP_CALL(standard_read,READ_NUM)
WRAP_CALL(standard_write,WRITE_NUM)
WRAP_CALL(standard_open,OPEN_NUM)
WRAP_CALL(standard_close,CLOSE_NUM)
WRAP_CALL(standard_getargs,GETARGS_NUM)
WRAP_CALL(standard_vidmap,VIDMAP_NUM)
WRAP_CALL(standard_set_handler,SET_HANDLER_NUM)
WRAP_CALL(standard_sigreturn,SIGRETURN_NUM)
