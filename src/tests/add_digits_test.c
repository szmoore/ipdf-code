/**
 * Compile and linking:
 * gcc -c add_digits.s
 * gcc -c add_digits_test.c
 * gcc -o add_digits_test add_digits_test.o add_digits.o
 */
 
//TODO: Move to C++

#include <stdlib.h>
#include <stdio.h>

int64_t add_digit(int64_t * a, int64_t * b);

int main(int argc, char ** argv)
{
	int64_t s1[] = {5,6,7,0xFFFFFFFFFFFFFFFF,0};
	int64_t s2[] = {7,1,5,1L,0};
	
	int size = sizeof(s1)/sizeof(int64_t);
	
	printf("Before adding s1 and s2:\n");
	int i;
	for (i = 0; i < size; ++i)
	{
		printf("s1[%d] = %.16lx\t", i, s1[i]);
		printf("s2[%d] = %.16lx\n", i, s2[i]);
	}
	
	add_digits(s1, s2, size);
	printf("\nAfter adding s1 and s2:\n");
	for (i = 0; i < size; ++i)
	{
		printf("s1[%d] = %.16lx\t", i, s1[i]);
		printf("s2[%d] = %.16lx\n", i, s2[i]);
	}
	
	
}
