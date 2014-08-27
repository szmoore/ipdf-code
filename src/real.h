#ifndef _REAL_H
#define _REAL_H

#include "common.h"
#include <cmath>


#define REAL_SINGLE 0
#define REAL_DOUBLE 1
#define REAL_LONG_DOUBLE 2
#define REAL_VFPU 3
#define REAL_RATIONAL 4
#define REAL_RATIONAL_ARBINT 5

#ifndef REAL
	#error "REAL was not defined!"
#endif

#if REAL == REAL_VFPU
	#include "vfpu.h"
#endif

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
#elif REAL == REAL_VFPU
	typedef VFPU::VFloat Real;
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
	inline double Sqrt(double f) {return sqrt(f);}
	
	inline Real Power(const Real & a, int n)
	{
		if (n < 0)
		{
			return Power(Real(1)/a, -n);
		}
		Real r(1);
		for (int i = 0; i < n; ++i)
			r *= a;
		return r;
	}
	
	struct Vec2
	{
		Real x;
		Real y;
		Vec2() : x(0), y(0) {}
		Vec2(Real _x, Real _y) : x(_x), y(_y) {}
	
		bool operator==(const Vec2& other) const { return (x == other.x) && (y == other.y); }
		bool operator!=(const Vec2& other) const { return !(*this == other); }
		
		Vec2& operator=(const Vec2& other) { x = other.x; y = other.y; return *this; }
		Vec2& operator+=(const Vec2& other) { x += other.x; y += other.y; return *this; }
		Vec2& operator-=(const Vec2& other) { x -= other.x; y -= other.y; return *this; }
		Vec2& operator*=(const Real& lambda) { x *= lambda; y *= lambda; return *this; }
		Vec2& operator/=(const Real& lambda) { x /= lambda; y /= lambda; return *this; }

		Vec2 operator+(const Vec2& other) const { return Vec2(x + other.x, y + other.y); }
		Vec2 operator-(const Vec2& other) const { return Vec2(x - other.x, y - other.y); }
		Vec2 operator*(const Real& lambda) const { return Vec2(x * lambda, y * lambda); }
		Vec2 operator/(const Real& lambda) const { return Vec2(x / lambda, y / lambda); }

		const Real SquareLength() const { return (x*x + y*y); }
	
	};


}

#endif //_REAL_H
