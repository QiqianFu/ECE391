
# mp1.S - missile-command version
# Good luck, and have fun!

.data
	
    # Constants for accessing the fields of a struct missile, 
    # struct missile is defined in rtc.h and mp1.h

	NEXT 	  = 0
	X	  = 4
	Y	  = 8
	VX	  = 12
	VY	  = 16
	DEST_X	  = 20
	DEST_Y	  = 24
	EXPLODED  = 28
	C	  = 32

    # Character to draw for an explosion - '@'

	EXPLOSION = 64


# Data shared between this file and rtc.c helper functions

# This '.globl' directive makes these symbols visible externally
.globl mp1_missile_list, base_alive, mp1_score

mp1_missile_list:	.long 0x0	# Head of list pointer
base_alive:		.long 0x0	# Array of 3 bytes, plus a padding byte
mp1_score:		.long 0x0	# Player's current score


# Data private to this  file

base_pic:     	    .string "/^^^\\"	# Picture of a live base
dead_base_pic: 	    .string "xxxxx"	# Picture of a dead base
crosshairs_x:		.long 0x0	# X-position of the crosshairs
crosshairs_y:		.long 0x0	# Y-position of the crosshairs


.text

# void mp1_poke(void);
# You can use this function to write to video memory.
#
# Interface: Register-based arguments (not C-style)
#    Inputs: %cl  - The byte you wish to write
#            %eax - Offset from the start of video memory that you wish
#                   to write to
#   Outputs: Text-mode video screen is written to at location %eax with
#            the byte in %cl
# Registers: Clobbers EDX

mp1_poke:
		movl    vmem_base_addr(,1),%edx
		movb    %cl,(%edx,%eax,1)
		ret

#   ----------------- Exported functions ---------------------

# void mp1_rtc_tasklet(unsigned long garbage);
# Performs three tasks:
#	(1) updates the list of missiles (implement this in update_missiles,
#           below, and call it from here).
#	(2) Redraw the bases - they may have been overwritten with missiles
#	(3) Redraw the crosshairs - it may have been overwritten with missiles
# Inputs   : none
# Outputs  : none
# Registers: Standard C calling convention

.globl mp1_rtc_tasklet
mp1_rtc_tasklet:
	pushl %ebp
	movl %esp, %ebp
	pushl %ebx
	pushl %esi
	pushl %edi
	
	call update_missiles
	call redraw_city
	call redraw_crosshairs

	popl %edi
	popl %esi
	popl %ebx
	leave
		ret



# int mp1_ioctl(unsigned long arg, unsigned int cmd)
# The dispatch function for the MP1 ioctls - should use the cmd argument
# and a jumptable to execute one of the specific ioctls implemented below.
# Inputs   : unsigned long arg - parameter to the mp1_ioctl_....
#          : unsigned int cmd  - specifies which mp1_ioctl_... function 
#          :                     to execute
# Outputs  : Returns an integer - depends on which ioctl() function is called
# Registers: Standard C calling convention




.globl mp1_ioctl
mp1_ioctl:
	pushl %ebp
	movl %esp, %ebp
	pushl %ebx
	pushl %esi
	pushl %edi
	movl 12(%ebp),%eax
	cmpl $0,%eax
	jl FAIL_CMD
	cmpl $4,%eax
	jg FAIL_CMD
	movl jump_table(,%eax,0x4),%ecx
	jmp *%ecx

FAIL_CMD:
	movl $-1,%eax
	popl %edi
	popl %esi
	popl %ebx
	leave
	ret	

jump_table:
		.long	mp1_ioctl_startgame
		.long	mp1_ioctl_addmissile
		.long	mp1_ioctl_movexhairs
		.long	mp1_ioctl_getstatus
		.long	mp1_ioctl_endgame

# ----------------- Functions private to this file -------------------

update_missiles:
	pushl %ebp									# save all the registers
	movl %esp, %ebp
	pushl %eax
	pushl %ebx
	pushl %ecx
	pushl %edx
	pushl %esi
	pushl %edi
	movl mp1_missile_list,%ebx		
MULTI_TRAVERSE:									# ebx: current pointer to the missile
	cmpl $0, %ebx
	je NO_MORE_ELEMENT                			# check if the list is empty or not
	movl X(%ebx),%esi
	shrl $16,%esi
	movl Y(%ebx),%edi
	shrl $16,%edi
	pushl %esi									# esi is X, edi is Y
	pushl %edi
	pushl $32									# draw one time (x,y,' ')
	call draw_one_time
	addl $12,%esp
												# no matter what's the next status, the old one must be erased from the screen 
	

JUDGE:											# first part
	movl EXPLODED(%ebx),%ecx
	cmpl $0,%ecx
	jne REACH_DEST_EXPLODING		  			# if exploded not euqal 0, then explode. We need to draw it, and if explosion done ,remove it from the list


	movl VX(%ebx),%esi
	movl X(%ebx),%edx				          	# update the x y, x + which is vx to esi, to check if out of range 
	addl %edx,%esi
	shrl $16,%esi
	cmpl $0,%esi								# esi: x+dx
	jl REMOVE_MISSILE
	cmpl $79,%esi
	jg REMOVE_MISSILE
	movl VY(%ebx),%edi
	movl Y(%ebx),%edx				          	# update the x y, x + which is vx to esi, to check if out of range 
	addl %edx,%edi
	shrl $16,%edi								# edi: y+dy
	cmpl $0,%edi
	jl REMOVE_MISSILE
	cmpl $24,%edi
	jg REMOVE_MISSILE
	movl VX(%ebx),%edi							# update x and y since they are not out of range
	movl X(%ebx),%edx				          	
	addl %edx,%edi
	movl %edi,X(%ebx)

	movl VY(%ebx),%esi
	movl Y(%ebx),%edx				          	
	addl %edx,%esi
	movl %esi,Y(%ebx)

	sarl $16,%edi
	sarl $16,%esi
	cmpl %edi,DEST_X(%ebx)
	jne REDRAW_MISSILE
	cmpl %esi,DEST_Y(%ebx)						# not reach, not exploding and not reach dest and not out of range
	jne REDRAW_MISSILE

	jmp REACH_DEST_EXPLODING					# only x and y fit dest_x and dest_y at the same time means reaching destination

NO_MORE_ELEMENT:
	popl %edi
	popl %esi
	popl %edx
	popl %ecx
	popl %ebx
	popl %eax									# callee saved
	leave
	ret

REDRAW_MISSILE:									# update the missile on the screen 
	movl X(%ebx),%esi
	movl Y(%ebx),%edi
	shrl $16,%esi
	shrl $16,%edi
	pushl %esi
	pushl %edi 
	pushl C(%ebx)
	call draw_one_time
	addl $4, %esp
	popl %edi
	popl %esi 
	movl NEXT(%ebx),%ecx
	movl %ecx,%ebx
	jmp MULTI_TRAVERSE


REACH_DEST_EXPLODING:
	movl $0,VX(%ebx)
	movl $0,VY(%ebx)
	pushl %ebx
	pushl %ecx
	pushl %edx
	pushl %ebx
	call missile_explode					# if the missile reach the dest or is currently exploding, then check cause city or other explode
	movl %eax,%edi
	addl $12,%esp
	popl %ebx
	cmpl  $0,%edi
	je   NO_EXPLOSION
	pushl %eax								# there's an explosion. Notify the user
	pushl %ecx
	pushl %edx						
	call mp1_notify_user
	popl  %edx
	popl  %ecx
	popl  %eax	
NO_EXPLOSION:
	addl $-1, EXPLODED(%ebx)
	cmpl $0, EXPLODED(%ebx)
	jle REMOVE_MISSILE						# since it's still exploding , we need to redraw it
	movl X(%ebx),%esi
	shrl $16,%esi
	pushl %esi
	movl Y(%ebx),%edi
	shrl $16,%edi
	pushl %edi
	pushl $64
	call draw_one_time						# 64 is @
	addl $12,%esp
	movl NEXT(%ebx),%ecx
	movl %ecx,%ebx
	jmp MULTI_TRAVERSE

REMOVE_MISSILE:						# the explosion is done and need to remove. but we have replace the missile with space, so we just need to remove it from list 
	pushl NEXT(%ebx)
	pushl %ebx
	call mp1_remove_missile
	addl $4,%esp
	popl %ebx						# the former eax is not exsit, now the eax is next(former eax)
	jmp MULTI_TRAVERSE

mp1_remove_missile:
	pushl %ebp
	movl %esp, %ebp
	pushl %eax
	pushl %ebx
	pushl %ecx
	pushl %edx
	pushl %esi
	pushl %edi

	movl 8(%ebp), %ecx				# now ecx is the missile waiting to be removed 
	
	movl mp1_missile_list,%edx

	cmpl %ecx,%edx
	je RESET_HEAD					# equal means remove head

RELINK_THE_LIST:					# then it is in the middle, to remove middle ones, relink the list first
	movl NEXT(%edx),%esi
	cmpl %esi,%ecx
	je RELINK_DONE
	movl %esi,%edx
	jmp RELINK_THE_LIST

RELINK_DONE:
	pushl NEXT(%ecx)
	pushl %edx
	pushl %ecx
	call mp1_free
	addl $4,%esp
	popl %edx			        	# new ecx is stored
	popl %ecx
	movl %ecx,NEXT(%edx)
	jmp REMOVE_DONE

RESET_HEAD:
	movl NEXT(%edx),%ecx
	movl %ecx,mp1_missile_list
	pushl %edx
	call mp1_free
	popl %edx
	jmp REMOVE_DONE

REMOVE_DONE:
	popl %edi
	popl %esi
	popl %edx
	popl %ecx
	popl %ebx
	popl %eax
	leave
	ret	

redraw_city:
	pushl %ebp
	movl %esp, %ebp
	pushl %eax
	pushl %ebx
	pushl %ecx
	pushl %edx
	pushl %esi
	pushl %edi

	movl base_alive,%eax
	movl %eax,%edx
	shll $24,%edx						# shoule be like 0x ff 00 00 00 
	shrl $24,%edx
	movl $17,%ecx
	cmpl $0,%edx
	movl $0,%esi
	je DRAW_DEAD
	jmp DRAW_ALIVE
REDRAW_TRAVERSE1:
	cmpl $17,%ecx
	je CONTINUEVER1
	cmpl $37,%ecx
	jge REDRAW_TRAVERSE2
CONTINUEVER1:
	movl %eax,%edx
	shll $16,%edx						# should be like 0x ff ff 00 00
	shrl $24,%edx						# should be like 0x 00 00 00 ff 
	movl $37,%ecx
	cmpl $0,%edx
	movl $0,%esi
	je DRAW_DEAD
	jmp DRAW_ALIVE
REDRAW_TRAVERSE2:
	cmpl $57,%ecx
	je DRAWCITYDONE
	movl %eax,%edx
	shrl $16,%edx						# edx should be like 0x 00 ff ff ff
	movl $57,%ecx
	cmpl $0,%edx
	movl $0,%esi
	je DRAW_DEAD
	jmp DRAW_ALIVE


DRAW_ALIVE:
	lea base_pic,%edi
DRAW_ALIVE_TRAVERSE:
	movl %ecx,%ebx
	addl %esi,%ebx
	pushl %ebx
	pushl $24
	pushl (%edi)
	call draw_one_time
	addl $12,%esp
	incl %esi
	incl %edi
	cmpl $5,%esi
	je REDRAW_TRAVERSE1
	jmp DRAW_ALIVE_TRAVERSE

DRAW_DEAD:
	lea dead_base_pic,%edi
DRAW_DEAD_TRAVERSE:
	movl %ecx,%ebx
	addl %esi,%ebx
	pushl %ebx
	pushl $24
	pushl (%edi)
	call draw_one_time
	addl $12,%esp
	incl %esi							   # decrement ecx with -1
	incl %edi
	cmpl $5,%esi
	je REDRAW_TRAVERSE1
	jmp DRAW_DEAD_TRAVERSE

DRAWCITYDONE:
	popl %edi
	popl %esi
	popl %edx
	popl %ecx
	popl %ebx
	popl %eax
	leave
	ret


mp1_ioctl_startgame:
	movl $0x00FFFFFF, base_alive
	movl $40,crosshairs_x
	movl $12,crosshairs_y
	popl %edi
	popl %esi
	popl %ebx
	leave
	ret

mp1_ioctl_addmissile:
	pushl %ecx
	pushl %edx
	movl 8(%ebp),%ebx		# ebx is stored with the pointer
	pushl $36
	call mp1_malloc
	addl $4,%esp
	cmpl $0,%eax    		# eax is now the new missile
	je FAIL_MALLOC
	movl %eax,%ecx
	pushl %eax
	pushl $36
	pushl %ebx
	pushl %eax
	call mp1_copy_from_user
	addl $12,%esp
	cmpl $0,%eax

	jne COPY_TO_USER_FAIL
	popl %eax
	movl mp1_missile_list,%edx
	movl %edx,NEXT(%eax)
	movl %eax,mp1_missile_list

	movl $0,%eax
	popl %edx
	popl %ecx
	popl %edi
	popl %esi
	popl %ebx
	leave
	ret

FAIL_MALLOC:
	movl $-1,%eax
	popl %edi
	popl %esi
	popl %ebx
	leave
	ret	
	
COPY_TO_USER_FAIL:
	pushl %ecx
	call mp1_free
	addl $4,%esp
	jmp FAIL_MALLOC

mp1_ioctl_movexhairs:
	pushl crosshairs_x
	pushl crosshairs_y
	pushl $32
	call draw_one_time
	addl $12,%esp  				# clean the old crosshair
	movl 8(%ebp),%ecx 			# the coordinate
	movl %ecx,%esi
	movl %ecx,%edi
	sarl $16,%esi  				# esi store y
	shll $16,%edi
	sarl $16,%edi   			# edi store x 
	addl crosshairs_x,%edi
	addl crosshairs_y,%esi
	cmpl $0,%edi
	jl SETXTO0
	cmpl $79,%edi
	jg   SETXTO79
BACK_X:
	cmpl $0,%esi
	jl SETYTO0
	cmpl $24,%esi
	jg SETYTO24
BACK_Y:
	movl %edi,crosshairs_x
	movl %esi,crosshairs_y
	pushl crosshairs_x
	pushl crosshairs_y
	pushl $32
	call draw_one_time
	addl $12,%esp

	popl %edi
	popl %esi
	popl %ebx
	leave
	ret	


SETXTO0:
	movl $0,%edi
	jmp BACK_X

SETXTO79:
	movl $79,%edi
	jmp BACK_X

SETYTO0:
	movl $0,%esi
	jmp BACK_Y

SETYTO24:
	movl $24,%esi
	jmp BACK_Y



mp1_ioctl_getstatus:

	pushl $4
	call mp1_malloc
	addl $4,%esp
	cmpl $0,%eax
	je FAIL_MALLOC
	movl %eax,%esi
	xorl %ecx,%ecx
	movw mp1_score,%cx     				# only the last two bytes accouts
	movl base_alive,%edx
	shrl $16,%edx
	cmpl $0,%edx
	je NEXT_CITY
	movl $1,%edx
	shll $16,%edx
	addl %edx,%ecx
NEXT_CITY:
	movl base_alive,%edx
	shll $16,%edx
	shrl $24,%edx
	cmpl $0,%edx
	je NEXT_CITY2
	movl $1,%edx
	shll $17,%edx
	addl %edx,%ecx	
NEXT_CITY2:
	movl base_alive,%edx
	shll $24,%edx
	shrl $24,%edx
	cmpl $0,%edx
	je NEXT_CITY3
	movl $1,%edx
	shll $18,%edx
	addl %edx,%ecx		
NEXT_CITY3:
	movl %ecx,(%eax)
	pushl %ecx
	pushl %edx
	pushl $4
	pushl %eax
	movl 8(%ebp),%edx
	pushl %edx
	call mp1_copy_to_user
	addl $12,%esp
	cmpl $0,%eax
	jne COPY_FAIL
	movl $0,%eax
	popl %edx
	popl %ecx
	popl %edi
	popl %esi
	popl %ebx
	leave
	ret	
COPY_FAIL:
	movl $-1,%eax
	pushl %esi
	call mp1_free
	popl %esi
	popl %edi
	popl %esi
	popl %ebx
	leave
	ret	
	
mp1_ioctl_endgame:
	movl mp1_missile_list,%ebx
ENDGAME_TRAVERSE:
	cmpl $0,%ebx
	je FREE_DONE
	pushl NEXT(%ebx)
	pushl %ebx
	call mp1_free
	addl $4,%esp
	popl %ebx
	jmp ENDGAME_TRAVERSE
FREE_DONE:
	popl %edi
	popl %esi
	popl %ebx
	leave
	ret	

draw_one_time:
	pushl %ebp
	movl %esp, %ebp
	pushl %eax
	pushl %ebx
	pushl %ecx
	pushl %edx
	pushl %esi
	pushl %edi
	movl 16(%ebp),%esi   # row
	movl 12(%ebp),%edi   # column
	movl 8(%ebp),%ecx
	imull $80,%edi
	addl %edi,%esi
	imull $2,%esi
	movl %esi,%eax
	call mp1_poke
	popl %edi
	popl %esi
	popl %edx
	popl %ecx
	popl %ebx
	popl %eax
	leave
	ret

redraw_crosshairs:
	pushl %ebp
	movl %esp, %ebp
	pushl %eax
	pushl %ebx
	pushl %ecx
	pushl %edx
	pushl %esi
	pushl %edi
	movl crosshairs_x,%esi   # row
	movl crosshairs_y,%edi  # column
	movl $43,%ecx			# +
	imull $80,%edi
	addl %edi,%esi
	imull $2,%esi
	movl %esi,%eax
	call mp1_poke
	popl %edi
	popl %esi
	popl %edx
	popl %ecx
	popl %ebx
	popl %eax
	leave
	ret
