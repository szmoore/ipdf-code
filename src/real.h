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
#elif defined REAL_HALF
	struct Real
	{
		Real() = default;
		Real(double r) : value(r) 
		{
			int & a = *((int*)(&value)); // um...
			// mask out extra bits in exponent
			 //1000 1111 1000 0000 0000 0011 1111 1111
			// Endianness matters
			a &= 0xFFC001F1; //0x8F8003FF;

		}	
	
		Real operator+(float f) const {return Real(value+f);}
		Real operator-(float f) const {return Real(value+f);}
		Real operator/(float f) const {return Real(value/f);}
		Real operator*(float f) const {return Real(value*f);}
		Real operator+(const Real & r) const {return Real(this->value + r.value);}
		Real operator-(const Real & r) const {return Real(this->value - r.value);}
		Real operator*(const Real & r) const {return Real(this->value * r.value);}
		Real operator/(const Real & r) const {return Real(this->value / r.value);}
		Real & operator+=(const Real & r) {this->value += r.value; return *this;}
		Real & operator-=(const Real & r) {this->value -= r.value; return *this;}
		Real & operator/=(const Real & r) {this->value /= r.value; return *this;}
		Real & operator*=(const Real & r) {this->value *= r.value; return *this;}

		float value;
	};
	inline float Float(Real r) {return r.value;}

	inline std::ostream & operator<<(std::ostream & os, Real & r) {return os << r.value;} // yuk

#endif //REAL_HALF

}

#endif //_REAL_H
