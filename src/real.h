#ifndef _REAL_H
#define _REAL_H

#include "common.h"

namespace IPDF
{

#define REAL_SINGLE
//#define REAL_DOUBLE
//#define REAL_HALF

#ifdef REAL_SINGLE
	typedef float Real;
	inline float Float(Real r) {return r;}
#elif defined REAL_DOUBLE
	typedef double Real;
	inline double Float(Real r) {return r;}
#endif
}

#endif //_REAL_H
