/**
 * @file arbrationals.cpp
 * @brief Tests Rational<Arbint>
 */
 
#include "rational.h"
#include "arbint.h"

#define TEST_CASES 100

using namespace std;
using namespace IPDF;

int main(int argc, char ** argv)
{
	for (unsigned i = 0; i < TEST_CASES; ++i)
	{
		Debug("Cycle %u",i);
		Arbint p1 = 1L;//rand();
		Arbint q1 = 2L;//rand();
		Arbint p2 = rand();
		Arbint q2 = rand();
		Debug("Construct rationals");
		Rational<Arbint> a(p1, q1);
		Debug("Constructed a %u\n",i);
		Rational<Arbint> b(p2, q2);
		Debug("Constructed b %u\n",i);
		
		
	}
	printf("Tests done");
}
