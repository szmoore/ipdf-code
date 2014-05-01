#include "main.h"
#include <cmath>

using namespace IPDF;
/**
 * Example from Kahan's "Why is Floating-Point Computation so Hard to Debug when it Goes Wrong?"
 * Published on Kahan's website, March 2007  http://www.cs.berkeley.edu/~wkahan/WrongR.pdf
 * In this example we compute h(x) = |x| ... except it is not!
 * Note that the errors are NOT due to catastrophic cancellation (no subtraction) or division (no division) or accumulated rounding (only 128 operations)
 * http://www.cs.berkeley.edu/~wkahan/WrongR.pdf
 */

Real h(Real x)
{
	Real y = abs(x);
	for (int k = 0; k < 128; ++k)
	{
		y = sqrt(y);
	}
	Real w = y;
	for (int k = 0; k < 128; ++k)
	{
		w = w*w;
	}
	return w;
}

int main(int argc, char ** argv)
{
	for (Real x = -2; x < 2; x+=1e-3)
	{
		printf("%lf\t%lf\t%lf\n", x, Float(h(x)), abs(Float(x)));
	}
	return 0;
}
