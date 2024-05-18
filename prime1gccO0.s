	.file	1 "prime1gcc.c"

 # GNU C 2.6.3 [AL 1.1, MM 40] RISC NEWS-OS compiled by GNU C

 # Cc1 defaults:

 # Cc1 arguments (-G value = 8, Cpu = 3000, ISA = 1):
 # -quiet -dumpbase -O0 -o

gcc2_compiled.:
__gnu_compiled_c:
	.text
	.align	2
	.globl	getPrime
	.ent	getPrime
getPrime:
	.frame	$fp,24,$31		# vars= 0, regs= 2/0, args= 16, extra= 0
	.mask	0xc0000000,-4
	.fmask	0x00000000,0
	subu	$sp,$sp,24
	sw	$31,20($sp)
	sw	$fp,16($sp)
	move	$fp,$sp
	sw	$4,24($fp)
	sw	$5,28($fp)
	lw	$2,24($fp)
	li	$3,0x00000002		# 2
	sw	$3,0($2)
	li	$2,0x00000001		# 1
	sw	$2,nth
	li	$2,0x00000003		# 3
	sw	$2,candidat
$L2:
	lw	$2,nth
	lw	$3,28($fp)
	slt	$2,$2,$3
	bne	$2,$0,$L4
	j	$L3
$L4:
	li	$2,0x00000001		# 1
	sw	$2,remaindr
	sw	$0,index
	sw	$0,loopend
$L5:
	lw	$2,loopend
	beq	$2,$0,$L7
	j	$L6
$L7:
	lw	$2,index
	move	$3,$2
	sll	$2,$3,2
	lw	$3,24($fp)
	addu	$2,$2,$3
	move	$3,$2
	lw	$2,0($3)
	sw	$2,primenum
	lw	$2,candidat
	lw	$3,primenum
	div	$2,$2,$3
	sw	$2,quotient
	lw	$2,quotient
	lw	$3,primenum
	mult	$2,$3
	mflo	$2
	lw	$3,candidat
	subu	$2,$3,$2
	sw	$2,remaindr
	lw	$2,remaindr
	bne	$2,$0,$L8
	li	$2,0x00000001		# 1
	sw	$2,loopend
$L8:
	lw	$2,quotient
	lw	$3,quotient
	mult	$2,$3
	mflo	$2
	lw	$3,candidat
	slt	$2,$2,$3
	beq	$2,$0,$L9
	li	$2,0x00000001		# 1
	sw	$2,loopend
$L9:
	lw	$2,index
	addu	$3,$2,1
	sw	$3,index
	lw	$2,index
	lw	$3,nth
	slt	$2,$2,$3
	bne	$2,$0,$L10
	li	$2,0x00000001		# 1
	sw	$2,loopend
$L10:
	j	$L5
$L6:
	lw	$2,remaindr
	beq	$2,$0,$L11
	lw	$2,nth
	move	$3,$2
	sll	$2,$3,2
	lw	$3,24($fp)
	addu	$2,$2,$3
	move	$3,$2
	lw	$2,candidat
	sw	$2,0($3)
	lw	$2,nth
	addu	$3,$2,1
	sw	$3,nth
$L11:
	lw	$2,candidat
	addu	$3,$2,2
	sw	$3,candidat
	j	$L2
$L3:
	sw	$0,nth
$L12:
	lw	$2,nth
	lw	$3,28($fp)
	slt	$2,$2,$3
	bne	$2,$0,$L14
	j	$L13
$L14:
	lw	$2,nth
	move	$3,$2
	sll	$2,$3,2
	lw	$3,24($fp)
	addu	$2,$2,$3
	move	$3,$2
	lw	$4,0($3)
	jal	print
	lw	$2,nth
	addu	$3,$2,1
	sw	$3,nth
	j	$L12
$L13:
	j	$L1
$L1:
	move	$sp,$fp			# sp not trusted here
	lw	$31,20($sp)
	lw	$fp,16($sp)
	addu	$sp,$sp,24
	j	$31
	.end	getPrime
	.align	2
	.globl	main
	.ent	main
main:
	.frame	$fp,24,$31		# vars= 0, regs= 2/0, args= 16, extra= 0
	.mask	0xc0000000,-4
	.fmask	0x00000000,0
	subu	$sp,$sp,24
	sw	$31,20($sp)
	sw	$fp,16($sp)
	move	$fp,$sp
	jal	__main
	la	$4,primeNumbers
	li	$5,0x00000064		# 100
	jal	getPrime
	move	$2,$0
	j	$L15
$L15:
	move	$sp,$fp			# sp not trusted here
	lw	$31,20($sp)
	lw	$fp,16($sp)
	addu	$sp,$sp,24
	j	$31
	.end	main

	.comm	candidat,4

	.comm	quotient,4

	.comm	remaindr,4

	.comm	index,4

	.comm	nth,4

	.comm	primenum,4

	.comm	loopend,4

	.comm	primeNumbers,400
