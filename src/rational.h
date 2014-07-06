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
	Debug("Called on %li/%li", int64_t(a), int64_t(b));
	if (a == T(1) || a == T(0)) return T(1);
	if (b == T(0)) return a;
	if (b == a) 
	{
		Debug("Equal!");
		return a;
	}
	Debug("Not equal!");
	
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
	if (small == T(0))
		return g;
	while ((g = big % small) > T(0))
	{
		//Debug("big = %li, small = %li", int64_t(big), int64_t(small));
		big = small;
		small = g;
		//Debug("Loop %u", ++count);
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
		if (!CheckAccuracy(d, "Construct from double"))
		{
			//Fatal("Bwah bwah :(");
		}
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
		if (Q < T(0)) 
		{
			P = -P;
			Q = -Q;
		}
		if (P == T(0))
		{
			Q = T(1);
			return;
		}
		T g = gcd(T(llabs(P)),T(llabs(Q)));
		//Debug("Got gcd!");
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
		Rational result = (r.P == T(0)) ? Rational(P,Q) : Rational(P*r.Q + r.P*Q, Q*r.Q);
		if (!result.CheckAccuracy(ToDouble() * r.ToDouble(),"+"))
		{
			Debug("This is %s (%f) and r is %s (%f)", Str().c_str(), ToDouble(), r.Str().c_str(), r.ToDouble());
		}
		return result;
	}
	Rational operator-(const Rational & r) const 
	{
		Rational result = (r.P == T(0)) ? Rational(P,Q) : Rational(P*r.Q - r.P*Q, Q*r.Q);
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
		double result = fabs(ToDouble() - d);
		if (d != 0e0) result /= d;
		if (result > threshold)
		{
			Warn("(%s) : Rational %s (%f) is not close enough at representing %f (%f vs %f)", msg, Str().c_str(), ToDouble(), d, result, threshold);
			Backtrace();
			return false;
		}
		return true;
	}
	std::string Str() const
	{
		std::stringstream s;
		s << int64_t(P) << "/" << int64_t(Q);
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

