.section .text
.globl sub_digits
.type sub_digits, @function

# Subtract two arrays of 64 bit digits, with carry, modifying the first argument
# Address at first argument %rdi is array to add and modify
# Address at second %rsi will be added (not modified)
# Third argument is counter of number of digits
# Result in %rax is the final result in the carry flag
# Exploits the fact that inc and dec do not affect the carry flag
sub_digits:
	loop:
		movq (%rsi), %rax # Temporarily store digit from second array
		sbbq %rax, (%rdi) # Subtract digits in second and first array, store in first
		dec %rdx # Decrement counter
		jz end_loop # We are done
		
		# Move to next element in the first array
		leaq 8(,%rdi,1), %rdi
		# Move to next element in the second array
		leaq 8(,%rsi,1), %rsi
		jmp loop # Repeat
	end_loop:
		movq $0, %rax
		jnc end
		movq $1, %rax
	end:
		ret # We are done
