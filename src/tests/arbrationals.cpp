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

	Rational<Arbint> a(1, 2);
	Rational<Arbint> b(3, 9);
	Rational<Arbint> c(b);
	
	Rational<Arbint> d(c);
	c *= a;
	Debug("%s * %s = %s", d.Str().c_str(), a.Str().c_str(), c.Str().c_str());
	return 0;

}
