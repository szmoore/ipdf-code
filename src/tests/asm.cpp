#include <cstdlib>
#include <cstdio>
#include <iostream>

using namespace std;



int main(int argc, char ** argv)
{
	uint32_t a = 4294967295;
	uint32_t b = 0;
	uint32_t r = 0;
	bool c = addc(a, b, &r);
	printf("%u + %u = %u (%u)\n", a, b, r, (uint32_t)c);
	
}
