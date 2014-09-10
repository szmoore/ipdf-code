#include "real.h"

namespace IPDF
{
	// Maps the REAL to a string
	const char * g_real_name[] = {
		"single",
		"double",
		"long double",
		"VFPU",
		"Rational<int64_t>", 
		"Rational<Arbint>",
		"mpfrc++ real"
	};
	
#if REAL == REAL_RATIONAL_ARBINT
	template <> Gmpint Tabs(const Gmpint & a)
	{
		return a.Abs();
	}
#endif

}
