	.file	1 "prime1gcc.c"
	.set	nobopt

 # GNU C 2.6.3 [AL 1.1, MM 40] RISC NEWS-OS compiled by GNU C

 # Cc1 defaults:

 # Cc1 arguments (-G value = 8, Cpu = 3000, ISA = 1):
 # -quiet -dumpbase -O4 -o

gcc2_compiled.:
__gnu_compiled_c:
	.text
	.align	2
	.globl	getPrime
	.align	2
	.globl	main

	.comm	candidat,4

	.comm	quotient,4

	.comm	remaindr,4

	.comm	index,4

	.comm	nth,4

	.comm	primenum,4

	.comm	loopend,4

	.comm	primeNumbers,400

	.text
	.ent	getPrime
getPrime:
	.frame	$sp,32,$31		# vars= 0, regs= 3/0, args= 16, extra= 0
	.mask	0x80030000,-8
	.fmask	0x00000000,0
	subu	$sp,$sp,32
	sw	$16,16($sp)
	move	$16,$4
	sw	$17,20($sp)
	move	$17,$5
	li	$2,0x00000002		# 2
	sw	$31,24($sp)
	sw	$2,0($16)
	li	$2,0x00000001		# 1
	li	$3,0x00000003		# 3
	sw	$2,nth
	slt	$2,$2,$17
	sw	$3,candidat
	beq	$2,$0,$L3
	li	$7,0x00000001		# 1
$L4:
	lw	$6,candidat
	lw	$8,nth
	sw	$7,remaindr
	sw	$0,index
	sw	$0,loopend
$L7:
	lw	$5,index
	#nop
	sll	$2,$5,2
	addu	$2,$2,$16
	lw	$2,0($2)
	#nop
	div	$4,$6,$2
	mult	$4,$2
	sw	$2,primenum
	sw	$4,quotient
	mflo	$2
	#nop
	#nop
	subu	$3,$6,$2
	sw	$3,remaindr
	.set	noreorder
	.set	nomacro
	bne	$3,$0,$L8
	mult	$4,$4
	.set	macro
	.set	reorder

	sw	$7,loopend
$L8:
	mflo	$2
	#nop
	#nop
	slt	$2,$2,$6
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L18
	addu	$2,$5,1
	.set	macro
	.set	reorder

	sw	$7,loopend
$L18:
	sw	$2,index
	slt	$2,$2,$8
	bne	$2,$0,$L5
	sw	$7,loopend
$L5:
	lw	$2,loopend
	#nop
	beq	$2,$0,$L7
	beq	$3,$0,$L12
	lw	$2,nth
	lw	$4,candidat
	sll	$3,$2,2
	addu	$3,$3,$16
	addu	$2,$2,1
	sw	$4,0($3)
	sw	$2,nth
$L12:
	lw	$2,candidat
	lw	$3,nth
	addu	$2,$2,2
	slt	$3,$3,$17
	sw	$2,candidat
	bne	$3,$0,$L4
$L3:
	sw	$0,nth
	blez	$17,$L1
$L16:
	lw	$2,nth
	#nop
	sll	$2,$2,2
	addu	$2,$2,$16
	lw	$4,0($2)
	jal	print
	lw	$2,nth
	#nop
	addu	$2,$2,1
	sw	$2,nth
	slt	$2,$2,$17
	bne	$2,$0,$L16
$L1:
	lw	$31,24($sp)
	lw	$17,20($sp)
	lw	$16,16($sp)
	addu	$sp,$sp,32
	j	$31
	.end	getPrime
	.ent	main
main:
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, extra= 0
	.mask	0x80000000,-8
	.fmask	0x00000000,0
	subu	$sp,$sp,24
	sw	$31,16($sp)
	jal	__main
	la	$4,primeNumbers
	.set	noreorder
	.set	nomacro
	jal	getPrime
	li	$5,0x00000064		# 100
	.set	macro
	.set	reorder

	move	$2,$0
	lw	$31,16($sp)
	addu	$sp,$sp,24
	j	$31
	.end	main
