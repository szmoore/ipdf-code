#ifndef _RATIONAL_H
#define _RATIONAL_H

/**
 * A really shoddy implementation of Rational numbers
 */

#include "common.h"
#include <cmath>
#include <cassert>

namespace IPDF
{

/* Recursive version  of GCD
template <class T>
T gcd(const T & a, const T & b)
{
	if (a == 1 || a == 0) return 1;
	if (b == 0) return a;
	if (b == a) return a;
	
	if (a > b) return gcd(a-b,b);
	return gcd(a, b-a);
}
*/

/** Greatest Common Divisor of p and q **/
template <class T>
T gcd(const T & p, const T & q)
{
	T g(1);
	T big(p);
	T small(q);
	if (p < q)
	{
		big = q;
		small = p;
	}
	if (small == 0)
		return g;
	while ((g = big % small) > 0)
	{
		big = small;
		small = g;
	}
	return small;
}	 

template <class T = int64_t>
struct Rational
{
	/** Construct from a double.**/
	Rational(double d=0) : P(d*1e6), Q(1e6) // Possibly the worst thing ever...
	{
		Simplify();
		CheckAccuracy(d, "Construct from double");
	}

	Rational(const T & _P, const T & _Q) : P(_P), Q(_Q)
	{
		Simplify();
	}

	Rational(const Rational & cpy) : P(cpy.P), Q(cpy.Q)
	{
		Simplify();
	}

	void Simplify()
	{
		if (Q < 0) 
		{
			P = -P;
			Q = -Q;
		}
		if (P == 0)
		{
			Q = 1;
			return;
		}
		T g = gcd(llabs(P),llabs(Q));
		P /= g;
		Q /= g;
	}

	bool operator==(const Rational & r)  const
	{
		if (P == r.P && Q == r.Q) return true;
		return ToDouble() == r.ToDouble();
	}


	bool operator<(const Rational & r) const {return (P*r.Q < r.P * Q);}
	bool operator>(const Rational & r) const {return !(*this < r);}
	bool operator<=(const Rational & r) const {return *this == r || *this < r;}
	bool operator>=(const Rational & r) const {return *this == r || *this > r;}
	bool operator!=(const Rational & r) const {return !(*this == r);}

	Rational operator+(const Rational & r) const 
	{
		Rational result = (r.P == 0) ? Rational(P,Q) : Rational(P*r.Q + r.P*Q, Q*r.Q);
		result.CheckAccuracy(ToDouble() + r.ToDouble(),"+");
		return result;
	}
	Rational operator-(const Rational & r) const 
	{
		Rational result = (r.P == 0) ? Rational(P,Q) : Rational(P*r.Q - r.P*Q, Q*r.Q);
		result.CheckAccuracy(ToDouble() - r.ToDouble(),"-");
		return result;
	}
	Rational operator*(const Rational & r) const 
	{
		Rational result(P * r.P, Q * r.Q);
		if (!result.CheckAccuracy(ToDouble() * r.ToDouble(),"*"))
		{
			Debug("This is %s (%f) and r is %s (%f)", Str().c_str(), ToDouble(), r.Str().c_str(), r.ToDouble());
		}
		return result;
	}
	Rational operator/(const Rational & r) const 
	{
		Rational result(P * r.Q, Q*r.P);
		if (!result.CheckAccuracy(ToDouble() / r.ToDouble(),"/"))
		{
			Debug("This is %s (%f) and r is %s (%f)", Str().c_str(), ToDouble(), r.Str().c_str(), r.ToDouble());
		}
		return result;
	}	

	/** To cheat, use these **/
	//Rational operator+(const Rational & r) const {return Rational(ToDouble()+r.ToDouble());}
	//Rational operator-(const Rational & r) const {return Rational(ToDouble()-r.ToDouble());}
	//Rational operator*(const Rational & r) const {return Rational(ToDouble()*r.ToDouble());}
	//Rational operator/(const Rational & r) const {return Rational(ToDouble()/r.ToDouble());}

	Rational & operator=(const Rational & r) {P = r.P; Q = r.Q; return *this;}
	Rational & operator+=(const Rational & r) {this->operator=(*this+r); return *this;}
	Rational & operator-=(const Rational & r) {this->operator=(*this-r); return *this;}
	Rational & operator*=(const Rational & r) {this->operator=(*this*r); return *this;}
	Rational & operator/=(const Rational & r) {this->operator=(*this/r); return *this;}

	double ToDouble() const {return (double)(P) / (double)(Q);}
	bool CheckAccuracy(double d, const char * msg, double threshold = 1e-3) const
	{
		double result = fabs(ToDouble() - d) / d;
		if (result > threshold)
		{
			Warn("(%s) : Rational %s (%f) is not close enough at representing %f (%f vs %f)", msg, Str().c_str(), ToDouble(), d, result, threshold);
			return false;
		}
		return true;
	}
	std::string Str() const
	{
		std::stringstream s;
		s << (int64_t)P << "/" << (int64_t)Q;
		return s.str();
	}
	
	T P;
	T Q;
};

inline Rational<int64_t> pow(const Rational<int64_t> & a, const Rational<int64_t> & b)
{
	//TODO:Implement properly
	int64_t P = std::pow((double)a.P, b.ToDouble());
	int64_t Q = std::pow((double)a.Q, b.ToDouble());
	return Rational<int64_t>(P, Q);
}


}

#endif //_RATIONAL_H

