#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <cassert>
#include "arbint.h"

using namespace std;
using namespace IPDF;

#define TEST_CASES 100


int main(int argc, char ** argv)
{
	

	for (unsigned i = 0; i < TEST_CASES; ++i)
	{
		Arbint::digit_t a = rand();
		Arbint::digit_t b = rand();
		
		Arbint arb_a(a);
		Arbint arb_b(b);
		
		Debug("Test %u: a = %li, b = %li", i, a, b);
		
		Debug("a < b = %d", a < b);
		Debug("Arb a<b = %d", arb_a < arb_b);
		assert((arb_a < arb_b) == (a < b));
		
		Debug("a > b = %d", a > b);
		Debug("Arb a>b = %d", arb_a > arb_b);
		assert((arb_a > arb_b) == (a > b));
		
		Debug("a + b = %.16lx", a+b);
		Debug("Arb a+b = %s", (arb_a+arb_b).DigitStr().c_str());
		assert((arb_a+arb_b).AsDigit() == a + b);
		
		Debug("a - b = %li %.16lx", (a-b), (a-b));
		Debug("Arb a-b = %s %.16lx", (arb_a-arb_b).DigitStr().c_str(), (arb_a-arb_b).AsDigit());
		assert((arb_a-arb_b).AsDigit() == a - b);
		
		Debug("a * b = %.16lx", a*b);
		Debug("Arb a*b = %s", (arb_a*arb_b).DigitStr().c_str());
		assert((arb_a*arb_b).AsDigit() == a * b);
		
		Debug("a / b = %.16lx", a/b);
		Debug("Arb a/b = %s", (arb_a/arb_b).DigitStr().c_str());
		assert((arb_a/arb_b).AsDigit() == a / b);
		assert((arb_a%arb_b).AsDigit() == a % b);
	}
	printf("All single digit tests successful!\n");
	
	return 0;
}
