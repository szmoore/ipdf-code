.section .text
.globl div_digits
.type div_digits, @function

# div_digits(digits, div, size, res)
# divides an arbint in digits by uint64 div into res, returns remainder
# res may alias digits
# digits = rdi, div = rsx, size = rdx, res = rcx,
div_digits:
	movq %rdx, %r8
	leaq -8(%rdi,%r8,8), %rdi	# We want to point to the end of the buffer (LSB)
	leaq -8(%rcx,%r8,8), %rcx	# We want to point to the end of the buffer (LSB)
	movq $0, %rdx
loop:
	movq (%rdi), %rax
	divq %rsi			# rdx:rax/rsi => rax, rdx:rax%rsi => rdx
	movq %rax, (%rcx)
	dec %r8
	leaq -8(%rdi), %rdi
	leaq -8(%rcx), %rcx
	jnz loop
end:
	movq %rdx, %rax			# return the remainder
	ret
	
	
