#ifndef _TRANSFORMATIONTYPE_H
#define _TRANSFORMATIONTYPE_H

#ifdef QUADTREE_DISABLED
//#define TRANSFORM_OBJECTS_NOT_VIEW
//#define TRANSFORM_BEZIERS_TO_PATH
#endif

#include "gmprat.h"
#include <mpreal.h>
#include "real.h"

namespace IPDF
{
	
#ifdef TRANSFORM_BEZIERS_TO_PATH
#if PATHREAL == REAL_SINGLE
	typedef float PReal;
#elif PATHREAL == REAL_DOUBLE
	typedef double PReal;
#elif PATHREAL == REAL_LONG_DOUBLE
	typedef long double PReal;
#elif PATHREAL == REAL_MPFRCPP
	typedef mpfr::mpreal PReal;
#elif PATHREAL == REAL_GMPRAT
	typedef Gmprat PReal;
#endif
#else
	typedef Real PReal;
#endif

typedef PReal VReal;


#ifdef TRANSFORM_BEZIERS_TO_PATH

#if PATHREAL == REAL_MPFRCPP

#if REALTYPE != REAL_MPFRCPP
	#include <mpreal.h>

	inline double Double(const mpfr::mpreal & r) {return r.toDouble();}
	inline float Float(const mpfr::mpreal & r) {return r.toDouble();}
	inline int64_t Int64(const mpfr::mpreal & r) {return r.toLong();}
	inline mpfr::mpreal Sqrt(const mpfr::mpreal & r) {return mpfr::sqrt(r, mpfr::mpreal::get_default_rnd());}
	inline mpfr::mpreal Abs(const mpfr::mpreal & r) {return mpfr::abs(r, mpfr::mpreal::get_default_rnd());}
	//inline mpfr::mpreal RealFromStr(const char * str) {return mpfr::mpreal(strtod(str, NULL));}
	inline std::string Str(const mpfr::mpreal & a) {std::stringstream s; s << a; return s.str();}
	inline size_t Size(const mpfr::mpreal & a) {return a.get_prec();}
	inline mpfr::mpreal Log10(const mpfr::mpreal & a) {return mpfr::log10(a);}		
	inline mpfr::mpreal Exp(const mpfr::mpreal & a) {return mpfr::pow(2.817, a);}

#endif
#endif
#endif

}
#endif

