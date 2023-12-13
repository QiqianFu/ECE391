/*
 * tab:2
 *
 * edit_dist_asm.S - inplementation of computing edit distance between two (linked) lists
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement is
 * hereby granted, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * IN NO EVENT SHALL THE AUTHOR OR THE UNIVERSITY OF ILLINOIS BE LIABLE TO
 * ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
 * DAMAGES ARISING OUT  OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
 * EVEN IF THE AUTHOR AND/OR THE UNIVERSITY OF ILLINOIS HAS BEEN ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * THE AUTHOR AND THE UNIVERSITY OF ILLINOIS SPECIFICALLY DISCLAIM ANY
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE
 * PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND NEITHER THE AUTHOR NOR
 * THE UNIVERSITY OF ILLINOIS HAS ANY OBLIGATION TO PROVIDE MAINTENANCE,
 * SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS."
 *
 * Author:        Xiang Li
 * Version:       1
 * Creation Date: Sat Aug 20 2022
 * History:
 *    XL    1    Sat Aug 20 2022
 *        First written.
 */

.data
  UNUSED  = 0
  VAL   = 4
  NEXT = 8

.text
.global edit_dist_asm

# Edit Dist ASM (Assembly)
# Declaration is in edit_dist.h
#
# Registers:
#   EAX - Return Value
#   EBX - root1
#   BSI - root2

#   you may choose how to store your local/temp variables
#




edit_dist_asm:
  pushl %ebp
  movl %esp, %ebp
  # --- STUDENT CODE STARTS HERE ---
  pushl %EBX
  pushl %ESI
  pushl %EDI
  pushl %EBP
  MOVL 12(%EBP),%EBX  # EBX is now root1
  MOVL 8(%EBP),%ESI   # ESI is now root2
  CMPL $0x0,%EBX
  JNE CHECK1
  CMPL $0x0, %ESI
  JNE CHECK2
  MOVL $0x0, %EAX
  JMP DONE



CHECK1:                # use for root1 !=0
  CMPL $0x0, %ESI
  JNE CONTINUE
  pushl NEXT(%EBX)          # so that they can find the root
  pushl %ESI
  call edit_dist_asm                
  POPL %ESI
  POPL %EBX
  INCL %EAX
  JMP DONE

CHECK2:                # for root1=0 but root2 !=0
  pushl %EBX
  pushl NEXT(%ESI)
  call edit_dist_asm
  POPL %ESI
  POPL %EBX
  INCL %EAX
  JMP DONE

CONTINUE:
  MOVL 12(%EBP),%EBX  # EBX is now root1
  MOVL 8(%EBP),%ESI   # ESI is now root2
  MOVL VAL(%EBX),%ECX
  MOVL VAL(%ESI),%EDX
  CMPL %ECX,%EDX
  JNE IDR
  pushl NEXT(%EBX)          # so that they can find the root
  pushl NEXT(%ESI)
  call edit_dist_asm
  POPL %ESI
  POPL %EBX
  JMP DONE

IDR:
  pushl NEXT(%EBX)
  pushl %ESI
  call edit_dist_asm
  POPL %ESI
  POPL %EBX
  PUSHL %EAX          # the value of insert

  MOVL 12(%EBP),%EBX  # EBX is now root1
  MOVL 8(%EBP),%ESI   # ESI is now root2
  pushl %EBX
  pushl NEXT(%ESI)
  call edit_dist_asm
  POPL %ESI
  POPL %EBX
  PUSHL %EAX          # delete, because the program may use the register again

  MOVL 12(%EBP),%EBX  # EBX is now root1
  MOVL 8(%EBP),%ESI   # ESI is now root2
  pushl NEXT(%EBX)
  pushl NEXT(%ESI)
  call edit_dist_asm
  POPL %ESI
  POPL %EBX
  MOVL %EAX, %EDI      # replce

  POPL %EDX
  POPL %ECX


  CMPL %ECX,%EDX  
  JB SECOND
  CMPL %ECX,%EDI
  JB SECOND
  INCL %ECX
  MOVL %ECX, %EAX
  JMP DONE

SECOND:
  CMPL %EDX,%EDI
  JB THIRD
  INCL %EDX
  MOVL %EDX, %EAX
  JMP DONE
THIRD:
  INCL %EDI
  MOVL %EDI, %EAX
  JMP DONE
DONE:
  popl %EBP
  popl %EDI
  popl %ESI
  popl %EBX
  
    # --- STUDENT CODE ENDS HERE ---

  leave
  ret

