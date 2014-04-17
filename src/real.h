#ifndef _REAL_H
#define _REAL_H

#include "common.h"

#define REAL_SINGLE 0
#define REAL_DOUBLE 1

#ifndef REAL
	#error "REAL was not defined!"
#endif


namespace IPDF
{	
	extern const char * g_real_name[];

#if REAL == REAL_SINGLE
	typedef float Real;
	inline float Float(Real r) {return r;}
#elif REAL == REAL_DOUBLE
	typedef double Real;
	inline double Float(Real r) {return r;}
#else
	#error "Type of Real unspecified."
#endif //REAL

}

#endif //_REAL_H
