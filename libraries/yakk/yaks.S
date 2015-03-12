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

	# save flags
	in		r0,		_SFR_IO_ADDR(SREG)
	push	r0

	# save EIND and RAMPZ
	in		r0,		0x3b
	push	r0
	in		r0,		0x3c
	push	r0

	# save registers
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
	# store current SP in current TCB
	##########################################################################

	# x <- SP
	in		r26,	0x3d
	in		r27,	0x3e

	# y <- [YKCurTask] = YKCurTask->stack_ptr
	lds		r28,	YKCurTask
	lds		r29,	YKCurTask+1

	# [y] = [[YKCurTask]] = *(YKCurTask->stack_ptr) <- SP
	std		y+0,	r26
	std		y+1,	r27

	##########################################################################
	# load high priority TCB
	##########################################################################

	# YKCurTask = YKReadyList
	lds		r26,	YKReadyList
	lds		r27,	YKReadyList+1

	sts		YKCurTask,		r26
	sts		YKCurTask+1,	r27

	##########################################################################
	# load new SP from TCB
	##########################################################################

	# y <- [YKCurTask] = YKCurTask->stack_ptr
	lds		r28,	YKCurTask
	lds		r29,	YKCurTask+1

	# x <- [y] = [[YKCurTask]] = *(YKCurTask->stack_ptr)
	ldd		r26,	y+0
	ldd		r27,	y+1

	# SP = z = *(YKCurTask->stack_ptr)
	out		0x3d,	r26
	out		0x3e,	r27

	##########################################################################
	# restore context
	##########################################################################

	# restore registers
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

	# restore RAMPZ and EIND
	pop		r0
	out		0x3c,	r0
	pop		r0
	out		0x3b,	r0

	# restore flags
	pop		r0
	out		_SFR_IO_ADDR(SREG),		r0

	##########################################################################
	# execute new task
	##########################################################################

	ret
