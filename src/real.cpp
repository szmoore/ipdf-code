#include "real.h"
#include <fenv.h>

namespace IPDF
{
	// Maps the REALTYPE to a string
	const char * g_real_name[] = {
		"single",
		"double",
		"long double",
		"VFPU",
		"Rational<int64_t>", 
		"Rational<Arbint>",
		"mpfrc++ real",
		"iRRAM REAL"
	};
	
#if REALTYPE == REAL_RATIONAL_ARBINT
	template <> Gmpint Tabs(const Gmpint & a)
	{
		return a.Abs();
	}
#endif




}
