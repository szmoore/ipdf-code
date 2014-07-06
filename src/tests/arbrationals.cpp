/**
 * @file arbrationals.cpp
 * @brief Tests Rational<Arbint>
 */
 
#include "rational.h"
#include "arbint.h"

#define TEST_CASES 100

using namespace std;
using namespace IPDF;

#include "rect.h"

int main(int argc, char ** argv)
{
	
	Rect rect(0,0,1,1);
	return 0;
	for (unsigned i = 0; i < TEST_CASES; ++i)
	{
		double d = (double)(rand()) / (double)(rand());
		Rational<Arbint> r(d);
		Debug("%f -> %s -> %s/%s", d, r.Str().c_str(), r.P.DigitStr().c_str(), r.Q.DigitStr().c_str());
		
		
		
	}
	printf("Tests done");
}
