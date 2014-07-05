.section .text
.globl div_digits
.type div_digits, @function

# div_digits(digits, div, size, res, rem)
# divides an arbint in digits by uint64 div into res with remainder rem
# Either res or rem may alias digits
# digits = rdi, div = rsx, size = rdx, res = rcx, rem = r8
div_digits:
	movq %rdx, %r9
	leaq -8(%rdi,%r9,8), %rdi	# We want to point to the end of the buffer (LSB)
	leaq -8(%rcx,%r9,8), %rcx	# We want to point to the end of the buffer (LSB)
	leaq -8(%r8,%r9,8), %r8		# We want to point to the end of the buffer (LSB)
	movq $0, %rdx
loop:
	movq (%rdi), %rax
	divq %rsi			# rdx:rax/rsi => rax, rdx:rax%rsi => rdx
	movq %rax, (%rcx)
	movq %rdx, (%r8)
	dec %r9
	leaq -8(%rdi), %rdi
	leaq -8(%rcx), %rcx
	leaq -8(%r8), %r8
	jnz loop
end:
	ret
	
	
