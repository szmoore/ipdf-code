#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include "main.h"

#include "real.h"

using namespace IPDF;

/**
 * This is a C version of Kahan's proposed benchmark for determining the worst accuracy of a CPU
 * Taken from kahan1996iee754 - Lecture notes on the status of IEEE 754
 */


/**
 * Quadradic of the form p*x^2 - 2 q*x + r == 0
 * This function computes the roots x1 and x2 as accuractely as they are determined by {p, q, r}
 */
void Qdrtc(Real p, Real q, Real r, Real * x1, Real * x2)
{
	Real s = sqrt(q*q - p*r);
	Real S = q + copysignf(s, q);
	if (S == 0.f)
	{
		*x1 = r/p;
		*x2 = r/p;
		return;
	}
	*x1 = r/S;
	*x2 = S/p;	
}

/**
 * Kahan's magic Qtrial test thing
 * Calculate roots using Qdrtc and compare to known results
 * Return the smallest (?) of the errors
 */
Real Qtrial(Real r)
{
	Real p = r-Real(2);
	Real q = r-Real(1);
	Real result = Real(0);
	Debug("Qdrtc for r = %.30lf", Float(r));
	if (p <= Real(0))
		Fatal("Expect r > 2");
	else if (!((r-q)== Real(1) && (q-p) == Real(1)))
		Fatal("r too big for Qtrial %.30lf", Float(r));

	Real x1; Real x2;
	Qdrtc(p, q, r, &x1, &x2);
	Real e1 = -log2f(x1 - Real(1));
	Real e2 = -log2f((x2 - Real(1)) - Real(2)/p);
	
	result = min(e1, e2);
	Debug("gets %.30lf and %.30lf sig bits", Float(e1), Float(e2));
	if (x1 < Real(1))
		Debug(" and root %.30lf isn't at least 1", Float(x1));
	return result;
}

int main(int argc, char ** argv)
{
	Real x1; Real x2;
	Qdrtc(2,5,12,&x1,&x2);
	if (x1 != Real(2) || x2 != Real(3))
	{
		Fatal("Qdrtc(2, 5, 12) failed sanity check; x1 %.30lf x2 %.30lf", Float(x1), Float(x2));
	}

	size_t n = 12;
	Real r[n];
	r[0] = (1 << 12) + 2.0;
	r[1] = (1 << 12) + 2.25;
	r[2] = (16 << 8) + 1.0 + 1.0/(16<<4);
	r[3] = (1<<24) + 2.0;
	r[4] = (1<<24) + 3.0;
	r[5] = 94906267.0;
	r[6] = 94906267 + 0.25;
	r[7] = (1<<28) - 5.5;
	r[8] = (1<<28) - 4.5;
	r[9] = (1<<28) + 2.0;
	r[10] = (1<<28) + 2.25;
	r[11] = (16<<24) + 1 + 1.0/(16<<20);
//	r[12] = (1<<32) + 2.0;
//	r[13] = (1<<32) + 2.25;

	Real e = +INFINITY;
	Real t;
	for (size_t j = 0; j < n; ++j)
	{
		t = Qtrial(r[j]);
		if (t < e || t != t) e = t;
	}
	Debug("Worst accuracy is %.30lf sig. bits", Float(e));
	return 0;
}
