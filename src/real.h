#ifndef _REAL_H
#define _REAL_H

#include "common.h"

namespace IPDF
{

//#define REAL_FLOAT
#define REAL_DOUBLE
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
		Real(float r) : value(r) 
		{
			int & a = *((int*)(&value)); // um...
			// mask out extra bits in exponent
			 //1000 1111 1000 0000 0000 0011 1111 1111
			// Endianness matters
			a &= 0xFF3008F8;//0x8F8003FF;

		}	
	
		Real operator+(float f) {return Real(value+f);}
		Real operator-(float f) {return Real(value+f);}
		Real operator/(float f) {return Real(value/f);}
		Real operator*(float f) {return Real(value*f);}
		Real operator+(const Real & r) {return this->operator+(r.value);}
		Real operator-(const Real & r) {return this->operator-(r.value);}
		Real operator*(const Real & r) {return this->operator*(r.value);}
		Real operator/(const Real & r) {return this->operator/(r.value);}
		float value;
	};
	inline float Float(Real r) {return r.value;}

	inline std::ostream & operator<<(std::ostream & os, Real & r) {return os << r.value;} // yuk

#endif //REAL_HALF

}

#endif //_REAL_H
