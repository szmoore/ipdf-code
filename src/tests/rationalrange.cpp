#include "main.h"
#include "rational.h"
#include <cassert>
#include <list>

using namespace std;
using namespace IPDF;

int main(int argc, char ** argv)
{
	typedef uint64_t Uint;
	typedef pair<pair<Uint, Uint>, Rational<Uint> > RatPear;

	for (int i = 0; i < (int)(1e4); ++i)
	{
		double test = double(rand() % (int)(1e12)) * double(rand() % (int)(1e12)) / double(rand() % (int)1e12);
		Rational<Uint> r(test);
		printf("%f\t%f\t%f\t%lu\t%lu\n", test, r.ToDouble(), fabs(r.ToDouble() - test), r.P, r.Q);
	}

#if 0
	list<RatPear> space;
	for (Uint p = 0; p < 128; ++p)
	{
		for (Uint q = 0; q < 128; ++q)
		{
			space.push_back(RatPear(pair<Uint, Uint>(p,q), Rational<Uint>(p,q)));
		}
	}
	space.sort([=](const RatPear & a, const RatPear & b){return a.second < b.second;});
	for (auto i = space.begin(); i != space.end(); ++i)
	{
		if (i->second.Q != 0)
			printf("%li\t%li\t%li\t%li\t%f\n", i->first.first, i->first.second, i->second.P, i->second.Q, i->second.ToDouble());
	}
#endif
}
