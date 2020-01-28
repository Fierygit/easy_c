.section	.rodata
.LC0:
	.string	"%d"
.LC1:
	.string	" "
.LC2:
	.string	"\n"
.LC3:
	.string	"%d\n"
.LC4:
	.string	"hello"
.LC5:
	.string	"world"
	.text
	.comm	p,4
	.globl	test
	.type  test, @function
test:
	pushl	%ebp
	movl	%esp,%ebp
	subl	$16,%esp
	# add
	movl	8(%ebp),%eax
	movl	12(%ebp),%edx
	addl	%edx,%eax
	movl	%eax,8(%esp)
	# asign
	movl	8(%esp),%esi
	leal	12(%esp),%edi
	movl	%esi,(%edi)
	# sub
	movl	12(%esp),%eax
	movl	$1,%edx
	subl	%edx,%eax
	movl	%eax,4(%esp)
	# PRINT
	subl	$8,%esp
	movl	12(%esp), %eax
	movl	%eax, 4(%esp)
	movl	$.LC3,0(%esp)
	call	printf
	addl	$8,%esp
	# sub
	movl	8(%ebp),%eax
	movl	$1,%edx
	subl	%edx,%eax
	movl	%eax,0(%esp)
	# return
	movl	0(%esp),%eax
	jmp 	.leave_test
	jmp	.leave_test
.leave_test:
	leave
	ret
	.globl	main
	.type  main, @function
main:
	pushl	%ebp
	movl	%esp,%ebp
	subl	$36,%esp
	# asign
	movl	$3,%esi
	leal	32(%esp),%edi
	movl	%esi,(%edi)
	# asign
	movl	$1,%esi
	leal	p,%edi
	movl	%esi,(%edi)
.Label0:
	 # if 
	movl	32(%esp),%eax
	cmpl	$0,%eax
	jne 	.Label1
	jmp 	.Label2
.Label1:
	# asign
	movl	32(%esp),%esi
	leal	20(%esp),%edi
	movl	%esi,(%edi)
	# asign
	movl	p,%esi
	leal	16(%esp),%edi
	movl	%esi,(%edi)
	# ARG
	subl	$4,%esp
	movl	20(%esp),%eax
	movl	%eax, 0(%esp)
	# ARG
	subl	$4,%esp
	movl	28(%esp),%eax
	movl	%eax, 0(%esp)
	# call 
	call	test
	addl	$8,%esp
	movl	%eax,12(%esp)
	# asign
	movl	12(%esp),%esi
	leal	32(%esp),%edi
	movl	%esi,(%edi)
	jmp 	.Label0
.Label2:
	# PRINT
	subl	$8,%esp
	movl	$.LC4,0(%esp)
	call	printf
	addl	$8,%esp
	# PRINT
	movl	$.LC1,0(%esp)
	call	printf
	addl	$8,%esp
	# PRINT
	subl	$8,%esp
	movl	$.LC5,0(%esp)
	call	printf
	addl	$8,%esp
	# PRINT
	movl	$.LC2,0(%esp)
	call	printf
	addl	$8,%esp
	 # if 
	movl	$1,%eax
	cmpl	$0,%eax
	jne 	.Label3
	jmp 	.Label4
.Label3:
	# asign
	movl	$2020,%esi
	leal	p,%edi
	movl	%esi,(%edi)
	# PRINT
	subl	$8,%esp
	movl	p, %eax
	movl	%eax, 4(%esp)
	movl	$.LC3,0(%esp)
	call	printf
	addl	$8,%esp
.Label4:
	# return
	movl	$0,%eax
	jmp 	.leave_main
	jmp	.leave_main
.leave_main:
	leave
	ret
