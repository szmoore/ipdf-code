#include <cstdlib>
#include <cstdio>
#include <iostream>

#include "arbint.h"

using namespace std;
using namespace IPDF;

int main(int argc, char ** argv)
{
	Arbint a(100L);
	Arbint b(200L);
	
	Arbint c(b-a);
	printf("(%d), %s\n",c.Sign(), c.DigitStr().c_str());
}
