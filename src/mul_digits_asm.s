.section .text
.globl mul_digits
.type mul_digits, @function

# Multiply an array of 64 bit digits by *one* 64 bit digit, modifies the array in place
mul_digits:
	movq %rdx, %rcx # rdx is reserved for mulq, use rcx as counter
	movq $0, %r12 # Overflow register
	loop:
		movq %rsi, %rax # Value to multiply in %rax
		mulq (%rdi) # Multiply, stored in %rdx:%rax (ie: we get TWO digits)
		
		# Add overflow from previous operation
		add %r12, %rax
		# Upper digit gets saved as next overflow
		movq %rdx, %r12
		
		# Lower digit goes in current array position
		movq %rax, (%rdi)
		
		dec %rcx # Decrement counter
		jz end_loop # We are done
		
		# Move to next element in the array
		leaq 8(,%rdi,1), %rdi
		jmp loop # Repeat
		
	end_loop:
	end:
		movq %r12, %rax # Return overflow
		ret # We are done
