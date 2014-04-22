/**
 * From Handbook of Floating-Point Arithmetic
 * Program 1.1 "A sequence that seems to converge to a wrong limit"
 * Modified to work with a template type and then print the results for our favourite types
 * I know it is O(N^2) when it should be O(N), but in terms of amount of typing it is O(much nicer this way for small values of N)
 */

#include <cstdio>
#include "real.h"

using namespace std;
using namespace IPDF;

template <class T> T s(T u, T v, int max)
{
	T w = 0.;
	for (int i = 3; i <= max; i++)
	{
		w = T(111.) - T(1130.)/v + T(3000.)/(v*u);
		u = v;
		v = w;
	}
	return w;
}

int main(void)
{
	double u0 = 2;
	double u1 = -4;
	printf("#n\tfloat\tdouble\tlong\tReal\n");
	for (int i = 3; i <= 30; ++i)
	{
		printf("%d\t%.15f\t%.15lf\t%.15llf\t%.15lf\n", i,
			s<float>(u0,u1,i), s<double>(u0,u1,i), s<long double>(u0,u1,i), Float(s<Real>(u0,u1,i)));
	}

	return 0;
}
