#include <cstdlib>
#include <cstdio>
#include <iostream>

#include "arbint.h"

using namespace std;
using namespace IPDF;

int main(int argc, char ** argv)
{
	int64_t test[] = {12L, 5L};
	test[1] = 0xE000000000000000;
	int64_t size = sizeof(test)/sizeof(int64_t);
	
	int64_t mul = 2L;
	
	int64_t overflow = mul_digits(test, mul, size);
	
	for (int64_t i = 0; i < size; ++i)
	{
		printf("digit[%li] = %.16lx\n", i, test[i]);
	}
	printf("Overflow is %.16lx\n", overflow);
}
