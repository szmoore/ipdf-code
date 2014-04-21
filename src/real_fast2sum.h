#ifndef _REAL_FAST2SUM_H
#define _REAL_FAST2SUM_H

#include <cmath>
// otherwise the abs() function won't work

namespace IPDF
{

	template <class T = float>
	struct RealF2S
	{
		RealF2S(const T & value = 0.0L) : m_value(value) {} //{Debug("Construct from value %f", m_value);}
		RealF2S(const RealF2S & cpy) : m_value(cpy.m_value) {} //{Debug("Copy construct from value %f", m_value);}

		RealF2S & operator+=(const RealF2S & a) {this->operator=(RealF2S(*this) + a); return *this;}
		RealF2S & operator-=(const RealF2S & a) {this->operator=(RealF2S(*this) - a); return *this;}
		RealF2S & operator*=(const RealF2S & a) {this->operator=(RealF2S(*this) * a); return *this;}
		RealF2S & operator/=(const RealF2S & a) {this->operator=(RealF2S(*this) / a); return *this;}

		RealF2S & operator=(const RealF2S & equ) {this->m_value = equ.m_value; return *this;}
		RealF2S & operator=(const T & equ) {this->m_value = equ.m_value; return *this;}

		T m_value;
	};

	

	template <class T> RealF2S<T> operator+(const RealF2S<T> & a, const RealF2S<T> & b)
	{
		// Use fast2sum
		if (abs(T(a.m_value)) < abs(T(b.m_value)))
			return b+a;
		T z = a.m_value + b.m_value;
		T w = z - a.m_value;
		T zz = b.m_value - w;
		return RealF2S<T>(z + zz);
	}
	template <class T> RealF2S<T> operator-(const RealF2S<T> & a, const RealF2S<T> & b)
	{
		// Use fast2sum
		return a + RealF2S<T>(-b.m_value);
	}

	template <class T> RealF2S<T> operator*(const RealF2S<T> & a, const RealF2S<T> & b)
	{
		return RealF2S<T>(a.m_value * b.m_value);
	}
	template <class T> RealF2S<T> operator/(const RealF2S<T> & a, const RealF2S<T> & b)
	{
		return RealF2S<T>(a.m_value / b.m_value);
	}


}

#endif //_REAL_FAST2SUM_H
