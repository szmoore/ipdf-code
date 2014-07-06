#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <cassert>
#include "arbint.h"

using namespace std;
using namespace IPDF;

#define TEST_CASES 100


void TestShifting(unsigned cases)
{
	for (unsigned c = 0; c < cases; ++c)
	{
		Arbint aa(1u, rand());
		Arbint bb(aa);
		for (unsigned i = 0; i < 100*sizeof(Arbint::digit_t); ++i)
		{
			bb <<= i;
			//Debug("i is %u bb is %c%s, a was %c%s", i, bb.SignChar(), bb.DigitStr().c_str(), aa.SignChar(), aa.DigitStr().c_str());
			bb >>= i;
			if (bb != aa)
			{
				Fatal("i is %u bb is %c%s, a was %c%s", i, bb.SignChar(), bb.DigitStr().c_str(), aa.SignChar(), aa.DigitStr().c_str());
			}
		}
	}
}

void TestMaths(unsigned cases)
{
	for (unsigned i = 0; i < cases; ++i)
	{
		int64_t a = rand();
		int64_t b = rand();
		
		Arbint arb_a(a);
		Arbint arb_b(b);
		
		//Debug("Test %u: a = %li, b = %li", i, a, b);
		
		//Debug("a < b = %d", a < b);
		//Debug("Arb a<b = %d", arb_a < arb_b);
		assert((arb_a < arb_b) == (a < b));
		
		//Debug("a > b = %d", a > b);
		//Debug("Arb a>b = %d", arb_a > arb_b);
		assert((arb_a > arb_b) == (a > b));
		
		//Debug("a + b = %.16lx", a+b);
		//Debug("Arb a+b = %s", (arb_a+arb_b).DigitStr().c_str());
		assert((arb_a+arb_b).AsDigit() == a + b);
		
		//Debug("a - b = %li %.16lx", (a-b), (a-b));
		//Debug("Arb a-b = %s %.16lx", (arb_a-arb_b).DigitStr().c_str(), (arb_a-arb_b).AsDigit());
		assert((arb_a-arb_b).AsDigit() == a - b);
		
		//Debug("a * b = %.16lx", a*b);
		//Debug("Arb a*b = %s", (arb_a*arb_b).DigitStr().c_str());
		assert((arb_a*arb_b).AsDigit() == a * b);
		
		//Debug("a / b = %.16lx", a/b);
		//Debug("Arb a/b = %s", (arb_a/arb_b).DigitStr().c_str());
		assert((arb_a/arb_b).AsDigit() == a / b);
		assert((arb_a%arb_b).AsDigit() == a % b);
	}	
}

void TestConstructFromDouble(unsigned cases)
{
	for (unsigned c = 0; c < cases; ++c)
	{
		int64_t test = (int64_t)(rand()*1e6);
		double d = (double)(test);
		Arbint a(d);
		Arbint b(test);
		if (a != b)
		{
			Fatal("test is %li, d is %f, a is %s, b is %s", test, d, a.DigitStr().c_str(), b.DigitStr().c_str());
		}
		
		
	}
}

int main(int argc, char ** argv)
{
	Debug("Shift testing...");
	TestShifting(TEST_CASES);
	Debug("Left/Right shift testing succeeded");

	Debug("Testing +-*/%");
	TestMaths(TEST_CASES);
	Debug("All (single digit) maths operator tests successful!");
	
	
	Debug("Testing construct from double");
	TestConstructFromDouble(TEST_CASES);
	Debug("Construct from double successful");
	return 0;
}
