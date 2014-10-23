#ifndef _REAL_H
#define _REAL_H

#include "common.h"
#include <cmath>
#include <cfloat>


#define REAL_SINGLE 0
#define REAL_DOUBLE 1
#define REAL_LONG_DOUBLE 2
#define REAL_VFPU 3
#define REAL_RATIONAL 4
#define REAL_RATIONAL_ARBINT 5
#define REAL_MPFRCPP 6
#define REAL_IRRAM 7
#define REAL_PARANOIDNUMBER 8
#define REAL_GMPRAT 9

#ifndef REALTYPE
	#error "REALTYPE was not defined!"
#endif

#define XSTR(x) STR(x)
#define STR(x) #x
//#pragma message "REALTYPE = " XSTR(REALTYPE)

#if REALTYPE == REAL_VFPU
	#include "vfpu.h"
#endif

#if REALTYPE == REAL_RATIONAL
	#include "rational.h"
#endif //REALTYPE

#if REALTYPE == REAL_RATIONAL_ARBINT
	#include "rational.h"
	#include "arbint.h"
	#include "gmpint.h"
#endif //REALTYPE

#if REALTYPE == REAL_MPFRCPP
	#include <mpreal.h>

#endif //REALTYPE

#if REALTYPE == REAL_IRRAM
	#include "../contrib/iRRAM/include/iRRAM/lib.h"
#endif

#if REALTYPE == REAL_PARANOIDNUMBER
	#include "paranoidnumber.h"
#endif

#if REALTYPE == REAL_GMPRAT
	#include "gmprat.h"
#endif 

namespace IPDF
{	
	extern const char * g_real_name[];

#if REALTYPE == REAL_SINGLE
	typedef float Real;
	inline Real RealFromStr(const char * str) {return strtof(str, NULL);}
	inline std::string Str(const Real & a) {std::stringstream s; s << a; return s.str();}
#elif REALTYPE == REAL_DOUBLE
	typedef double Real;
	inline Real RealFromStr(const char * str) {return strtod(str, NULL);}
	inline std::string Str(const Real & a) {std::stringstream s; s << a; return s.str();}
#elif REALTYPE == REAL_LONG_DOUBLE
	typedef long double Real;
	inline Real RealFromStr(const char * str) {return strtold(str, NULL);}
	inline std::string Str(const Real & a) {std::stringstream s; s << a; return s.str();}
#elif REALTYPE == REAL_VFPU
	typedef VFPU::VFloat Real;
	inline float Float(const Real & r) {return r.m_value;}
	inline double Double(const Real & r) {return r.m_value;}
	inline Real RealFromStr(const char * str) {return Real(strtod(str, NULL));}
#elif REALTYPE == REAL_RATIONAL
	typedef Rational<int64_t> Real;
	inline float Float(const Real & r) {return (float)r.ToDouble();}
	inline double Double(const Real & r) {return r.ToDouble();}
	inline Real RealFromStr(const char * str) {return Real(strtod(str, NULL));}
#elif REALTYPE == REAL_RATIONAL_ARBINT
	#define ARBINT Arbint // Set to Gmpint or Arbint here
	
	typedef Rational<ARBINT> Real;
	inline float Float(const Real & r) {return (float)r.ToDouble();}
	inline double Double(const Real & r) {return r.ToDouble();}
	inline int64_t Int64(const Real & r) {return r.ToInt64();}
	inline Rational<ARBINT> Sqrt(const Rational<ARBINT> & r) {return r.Sqrt();}
	inline Real RealFromStr(const char * str) {return Real(strtod(str, NULL));}
#elif REALTYPE == REAL_MPFRCPP
	typedef mpfr::mpreal Real;
	inline double Double(const Real & r) {return r.toDouble();}
	inline float Float(const Real & r) {return r.toDouble();}
	inline int64_t Int64(const Real & r) {return r.toLong();}
	inline Real Sqrt(const Real & r) {return mpfr::sqrt(r, mpfr::mpreal::get_default_rnd());}
	inline Real Abs(const Real & r) {return mpfr::abs(r, mpfr::mpreal::get_default_rnd());}
	inline Real RealFromStr(const char * str) {return Real(strtod(str, NULL));}
	inline std::string Str(const mpfr::mpreal & a) {std::stringstream s; s << a; return s.str();}
	inline size_t Size(mpfr::mpreal & a) {return a.get_prec();}
	inline mpfr::mpreal Log10(const mpfr::mpreal & a) {return mpfr::log10(a);}	
	inline mpfr::mpreal Exp(const mpfr::mpreal & a) {return mpfr::pow(2.817, a);}
	
#elif REALTYPE == REAL_IRRAM
	typedef iRRAM::REAL Real;
	inline double Double(const Real & r) {return r.as_double(53);}
	inline float Float(const Real & r) {return r.as_double(53);}
	inline int64_t Int64(const Real & r) {return (int64_t)r.as_double(53);}
	inline Real Sqrt(const Real & r) {return iRRAM::sqrt(r);}
	inline Real RealFromStr(const char * str) {return Real(strtod(str, NULL));}
#elif REALTYPE == REAL_PARANOIDNUMBER
	typedef ParanoidNumber Real;
	inline double Double(const Real & r) {return r.Digit();}
	inline float Float(const Real & r) {return r.Digit();}
	inline int64_t Int64(const Real & r) {return (int64_t)r.Digit();}
	inline Real Sqrt(const Real & r) {return Real(sqrt(r.Digit()));}
	inline Real RealFromStr(const char * str) {return Real(str);}	
	inline Real Abs(const Real & a) {return Real(fabs(a.Digit()));}
#elif REALTYPE == REAL_GMPRAT
	typedef Gmprat Real;
	inline double Double(const Real & r) {return r.ToDouble();}
	inline float Float(const Real & r) {return (float)(r.ToDouble());}
	inline int64_t Int64(const Real & r) {return (int64_t)r.ToDouble();}
	inline Real Sqrt(const Real & r) {return Real(sqrt(r.ToDouble()));}
	inline Real RealFromStr(const char * str) {return Real(strtod(str, NULL));}
	inline Real Abs(const Real & a) {return (a > Real(0)) ? a : Real(0)-a;}
	inline std::string Str(const Real & r) {return r.Str();}
	
#else
	#error "Type of Real unspecified."
#endif //REALTYPE

	// Allow us to call Float on the primative types
	// Useful so I can template some things that could be either (a more complicated) Real or a primitive type
	// Mostly in the testers.
	inline float Float(double f) {return (float)f;}
	inline float Float(long double f) {return (float)(f);}
	inline double Double(float f) {return (double)f;}
	inline double Double(double f) {return (double)f;}
	inline double Double(long double f) {return (double)(f);}
	inline double Sqrt(double f) {return sqrt(f);}
	inline double Abs(double a) {return fabs(a);}
	inline double Log10(double a) {return log(a)/log(10.0);}
	inline size_t Size(float a) {return sizeof(a);}
	inline size_t Size(double a) {return sizeof(a);}
	
	
	// Don't cause an exception
	inline float ClampFloat(const Real & a)
	{
		double d = Double(a);
		float f = (fabs(d) < FLT_MAX) ? ((fabs(d) > FLT_MIN) ? (float)d : FLT_MIN) : FLT_MAX;
		return copysign(f, d);
	}

	inline int64_t Int64(double a)
	{
		if (a < INT64_MIN)
			return INT64_MIN;
		if (a > INT64_MAX)
			return INT64_MAX;
		return (int64_t)(a);
	}
	
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
		Vec2() : x(0.0), y(0.0) {}
		Vec2(Real _x, Real _y) : x(_x), y(_y) {}
		Vec2(const std::pair<Real, Real> & p) : x(p.first), y(p.second) {}
		#if REALTYPE != REAL_IRRAM
		Vec2(const std::pair<int64_t, int64_t> & p) : x(p.first), y(p.second) {}
		#else
		Vec2(const std::pair<int64_t, int64_t> & p) : x((int)p.first), y((int)p.second) {}
		// Apparently iRRAM didn't implement -= for a constant argument
		#endif
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

	//TODO: Make sure there is actually a RealFromStr(const char * str) function
	// 		Or this will recurse infinitely
	//		(If you remove this it will also break).
	inline Real RealFromStr(const std::string & str) {return RealFromStr(str.c_str());}


	// things stolen from wikipedia and googling
	inline const char * HumanScale(const Real & r)
	{
		double f = Double(r);
		if (f < 1e-36)
			return "RATHER SMALL";
		if (f < 1e-35)
			return "Plank Length";
		if (f < 1e-25)
			return "Turtles all the way";
		if (f < 1e-24)
			return "More turtles";
		if (f < 1e-23)
			return "Turtles";
		if (f < 1e-22)
			return "This small";
		if (f < 1e-21)
			return "To find things";
		if (f < 1e-20)
			return "It is pretty difficult";
		if (f < 1e-19)
			return "Not much";
		if (f < 1e-17)
			return "Weak Force";
		if (f < 1e-16)
			return "Proton";
		if (f < 1e-15)
			return "(Classical) Electron";
		if (f < 1e-11)
			return "Inter atomic (still)";
		if (f < 1e-10)
			return "Inter atomic";
		if (f < 1e-9)
			return "Atom";
		if (f < 1e-8)
			return "DNA";
		if (f < 1e-7)
			return "Virus";
		if (f < 1e-6)
			return "Light";
		if (f < 1e-5)
			return "Bacteria";
		if (f < 1e-4)
			return "4004 Transistor";
		if (f < 1e-3)
			return "Ant";
		if (f < 1e-2)
			return "Coin";
		if (f < 0.5)
			return "iPhone";
		if (f < 5)
			return "Person";
		if (f < 1e2)
			return "Building";
		if (f < 1e3)
			return "Football Field";
		if (f < 2e3)
			return "Mountain";
		if (f < 1e4)
			return "Clouds";
		if (f < 1e5)
			return "Countries";
		if (f < 1e6)
			return "Earth";
		if (f < 1e8)
			return "Between Earth and Moon";
		if (f < 1e9)
			return "Solar System";
		if (f < 1e13)
			return "Distance to nearest Star";
		if (f < 1e21)
			return "Milky Way";
		if (f < 1e26)
			return "Universe";
		if (f < 1e27)
			return "A bigger Universe";
		if (f < 1e28)
			return "Really big things";
		if (f < 1e29)
			return "Almost as big as...";
		if (f < 1e30)
			return "Wolfram's Magestic Ego";
		return "QUITE BIG";
		
	}

	inline void DebugRealInfo() 
	{
		Debug("Compiled with REAL = %d => \"%s\" sizeof(Real) == %d bytes", REALTYPE, g_real_name[REALTYPE], sizeof(Real));
		#if REALTYPE == REAL_PARANOIDNUMBER
			#ifdef PARANOID_SIZE_LIMIT
				Debug("Size limit of %d is being enforced", PARANOID_SIZE_LIMIT);
			#endif
		#endif
		#if REALTYPE == REAL_MPFRCPP
			Debug("Precision of MPFR is %d", mpfr_get_default_prec());
		#endif
		
		#ifdef TRANSFORM_BEZIERS_TO_PATH
		Debug("PathReal = %d => \"%s\"", PATHREAL, g_real_name[PATHREAL]);
		#endif
	}
}

#endif //_REAL_H
