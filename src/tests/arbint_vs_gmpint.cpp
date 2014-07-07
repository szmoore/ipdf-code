#include <gmp.h>
#include <stdlib.h>
#include <stdio.h>

#include "log.h"

#include "gmpint.h"
#include "arbint.h"

#define TEST_CASES 100

using namespace IPDF;

int main(int argc, char ** argv)
{
	for (unsigned i = 0; i < TEST_CASES; ++i)
	{
		uint64_t a = rand();
	
		Arbint arb_a(a);
		Gmpint gmp_a(a);
		
		uint64_t b = rand();
		
		for (unsigned j = 0; j < 5; ++j)
		{
			arb_a *= b;
			gmp_a *= b;
		}
		
		for (unsigned j = 0; j < 5; ++j)
		{
			arb_a /= b;
			gmp_a /= b;
		}
		
		
	}
	return 0;
}
