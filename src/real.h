#ifndef _REAL_H
#define _REAL_H

#include "common.h"


#define REAL_SINGLE 0
#define REAL_DOUBLE 1
#define REAL_LONG_DOUBLE 2
#define REAL_SINGLE_FAST2SUM 3 //TODO: Remove, is FITH
#define REAL_RATIONAL 4
#define REAL_RATIONAL_ARBINT 5

#ifndef REAL
	#error "REAL was not defined!"
#endif

#if REAL >= REAL_SINGLE_FAST2SUM
	#include "real_fast2sum.h"
#endif //REAL

#if REAL == REAL_RATIONAL
	#include "rational.h"
#endif //REAL

#if REAL == REAL_RATIONAL_ARBINT
	#include "rational.h"
	#include "arbint.h"
	#include "gmpint.h"
#endif //REAL

namespace IPDF
{	
	extern const char * g_real_name[];

#if REAL == REAL_SINGLE
	typedef float Real;
#elif REAL == REAL_DOUBLE
	typedef double Real;
#elif REAL == REAL_LONG_DOUBLE
	typedef long double Real;
#elif REAL == REAL_SINGLE_FAST2SUM
	typedef RealF2S<float> Real;
	inline float Float(const Real & r) {return r.m_value;}
	inline double Double(const Real & r) {return r.m_value;}
#elif REAL == REAL_RATIONAL
	
	typedef Rational<int64_t> Real;
	inline float Float(const Real & r) {return (float)r.ToDouble();}
	inline double Double(const Real & r) {return r.ToDouble();}
#elif REAL == REAL_RATIONAL_ARBINT
	#define ARBINT Gmpint // Set to Gmpint or Arbint here
	
	typedef Rational<ARBINT> Real;
	inline float Float(const Real & r) {return (float)r.ToDouble();}
	inline double Double(const Real & r) {return r.ToDouble();}
	inline Rational<ARBINT> pow(const Rational<ARBINT> & a, const Rational<ARBINT> & b)
	{
		ARBINT P(std::pow(static_cast<double>(a.P), b.ToDouble()));
		ARBINT Q(std::pow(static_cast<double>(a.Q), b.ToDouble()));
		return Rational<ARBINT>(P,Q);
	}
#else
	#error "Type of Real unspecified."
#endif //REAL

	// Allow us to call Float on the primative types
	// Useful so I can template some things that could be either (a more complicated) Real or a primitive type
	// Mostly in the testers.
	inline float Float(float f) {return (float)f;}
	inline float Float(double f) {return (float)f;}
	inline float Float(long double f) {return (float)(f);}
	inline double Double(float f) {return (double)f;}
	inline double Double(double f) {return (double)f;}
	inline double Double(long double f) {return (double)(f);}
}

#endif //_REAL_H
