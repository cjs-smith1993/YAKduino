resetISR:
	# push	ax
	# push	bx
	# push	cx
	# push	dx
	# push	si
	# push	di
	# push	bp
	# push	es
	# push	ds

	# call	YKEnterISR
	# sti
	# call	resetHandler
	# cli
	# mov	al,	0x20
	# out	0x20, al
	# call	YKExitISR

	# pop		ds
	# pop		es
	# pop		bp
	# pop		di
	# pop		si
	# pop		dx
	# pop		cx
	# pop		bx
	# pop		ax

	# iret
	ret

tickISR:
	# push	ax
	# push	bx
	# push	cx
	# push	dx
	# push	si
	# push	di
	# push	bp
	# push	es
	# push	ds

	# call	YKEnterISR
	# sti
	# call	YKTickHandler
	# call	mytick
	# cli
	# mov	al,	0x20
	# out	0x20, al
	# call	YKExitISR

	# pop		ds
	# pop		es
	# pop		bp
	# pop		di
	# pop		si
	# pop		dx
	# pop		cx
	# pop		bx
	# pop		ax

	# iret
	ret

keyboardISR:
	# push	ax
	# push	bx
	# push	cx
	# push	dx
	# push	si
	# push	di
	# push	bp
	# push	es
	# push	ds

	# call	YKEnterISR
	# sti
	# call	keyboardHandler
	# cli
	# mov	al,	0x20
	# out	0x20, al
	# call	YKExitISR

	# pop		ds
	# pop		es
	# pop		bp
	# pop		di
	# pop		si
	# pop		dx
	# pop		cx
	# pop		bx
	# pop		ax

	# iret
	ret