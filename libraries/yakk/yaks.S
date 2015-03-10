#include <avr/io.h>

.extern YKPrintLists

.global YKEnterMutex
YKEnterMutex:
	cli
	ret

.global YKExitMutex
YKExitMutex:
	sei
	ret

.global YKDispatcher
YKDispatcher:
	##########################################################################
	# save context
	##########################################################################

	push	r0
	push	r1
	push	r2
	push	r3
	push	r4
	push	r5
	push	r6
	push	r7
	push	r8
	push	r9
	push	r10
	push	r11
	push	r12
	push	r13
	push	r14
	push	r15
	push	r16
	push	r17
	push	r18
	push	r19
	push	r20
	push	r21
	push	r22
	push	r23
	push	r24
	push	r25
	push	r26
	push	r27
	push	r28
	push	r29
	push	r30
	push	r31

	##########################################################################
	# save flags
	##########################################################################

	in		r0,		_SFR_IO_ADDR(SREG)
	push	r0

	##########################################################################
	# push ptr to YKDispatcher2
	##########################################################################

	lds		r26,	YKDispatcher2
	lds		r27,	YKDispatcher2 + 1
	push	r26
	push	r27

	##########################################################################
	# store current SP in current TCB
	##########################################################################

	# z = SP
	in		r30,	0x3d
	in		r31,	0x3e

	# x = [YKCurTask]
	lds		r26,	YKCurTask
	lds		r27,	YKCurTask + 1

	# y = [x] = [[YKCurTask]] = YKCurTask->stack_ptr
	ld		r28,	x+
	ld		r29,	x+

	# [y] = [[[YKCurTask]]] = *(YKCurTask->stack_ptr) = z = SP
	st		y+,		r30
	st		y+,		r31

	##########################################################################
	# load high priority TCB
	##########################################################################

	# YKCurTask = YKReadyList
	lds		r26,	YKReadyList
	lds		r27,	YKReadyList + 1

	sts		YKCurTask,		r26
	sts		YKCurTask + 1,	r27

	##########################################################################
	# load new SP from TCB
	##########################################################################

	# x = [YKCurTask]
	lds		r26,	YKCurTask
	lds		r27,	YKCurTask + 1

	# y = [x] = [[YKCurTask]] = YKCurTask->stack_ptr
	ld		r28,	x+
	ld		r29,	x+

	# z = [y] = [[[YKCurTask]]] = *(YKCurTask->stack_ptr)
	ld		r30,	y+
	ld		r31,	y+

	# SP = z = *(YKCurTask->stack_ptr)
	out		0x3d,	r30
	out		0x3e,	r31

	ret

	# push	ax						;save current context
	# push	cx
	# push	dx
	# push	bx
	# push	bp
	# push	si
	# push	di
	# push	es
	# push	ds

	# pushf
	# mov		bx,	0x0000
	# push	bx
	# mov		bx, YKDispatcher2
	# push	bx

	# mov		bx,	[YKCurTask]			;store current SP in TCB
	# mov		[bx], sp

	# mov		bx,	[YKReadyList]		;load address of high priority TCB
	# mov		[YKCurTask], bx

	# mov		sp, [bx]				;load new sp

	# iret

YKDispatcher2:
	# restore new context
	pop		r0
	out		_SFR_IO_ADDR(SREG),		r0

	pop		r31
	pop		r30
	pop		r29
	pop		r28
	pop		r27
	pop		r26
	pop		r25
	pop		r24
	pop		r23
	pop		r22
	pop		r21
	pop		r20
	pop		r19
	pop		r18
	pop		r17
	pop		r16
	pop		r15
	pop		r14
	pop		r13
	pop		r12
	pop		r11
	pop		r10
	pop		r9
	pop		r8
	pop		r7
	pop		r6
	pop		r5
	pop		r4
	pop		r3
	pop		r2
	pop		r1
	pop		r0

	ret

	# pop		ds						;restore new context
	# pop		es
	# pop		di
	# pop		si
	# pop		bp
	# pop		bx
	# pop		dx
	# pop		cx
	# pop		ax

	# ret							;execute new task
