#include "real.h"

namespace IPDF
{
	// Maps the REAL to a string
	const char * g_real_name[] = {
		"single",
		"double",
		"long double",
		"single [fast2sum]", //TODO REMOVE DOESN'T DO ANYTHING USEFUL
		"Rational<int64_t>", 
		"Rational<Arbint>"
	};
	
	template <> Gmpint Tabs(const Gmpint & a)
	{
		return a.Abs();
	}

}
