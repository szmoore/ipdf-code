#ifndef _PARANOIDNUMBER_H
#define _PARANOIDNUMBER_H

#include <list>
#include <cfloat>
#include <map>
#include <string>
#include "log.h"
#include <fenv.h>

#define PARANOID_DIGIT_T int8_t // we could theoretically replace this with a template
								// but let's not do that...

namespace IPDF
{
	typedef enum {ADD, SUBTRACT, MULTIPLY, DIVIDE} Optype;

	/** Performs an operation, returning if the result was exact **/
	// NOTE: DIFFERENT to ParanoidOp (although that wraps to this...)
	template <class T> bool TrustingOp(T & a, const T & b, Optype op);

	/** Performs an operation _only_ if the result would be exact **/
	template <class T> bool ParanoidOp(T & a, const T & b, Optype op)
	{
		T cpy(a);
		if (TrustingOp<T>(cpy, b, op))
		{
			a = cpy;
			return true;
		}
		return false;
	}


	template <> bool TrustingOp<float>(float & a, const float & b, Optype op);
	template <> bool TrustingOp<double>(double & a, const double & b, Optype op);
	template <> bool TrustingOp<int8_t>(int8_t & a, const int8_t & b, Optype op);
	
	// Attempt to comine two terms: a*b + c*d or a/b + c/d
	template <class T> bool CombineTerms(T & aa, Optype aop, T & bb, T & cc, Optype cop, T & dd)
	{
		T a(aa); T b(bb); T c(cc); T d(dd);
		if (aop == MULTIPLY && cop == MULTIPLY) // a*b + c*d
		{
			if ((ParanoidOp<T>(c, b, DIVIDE) || ParanoidOp(d, b, DIVIDE))
				&& TrustingOp<T>(c, d, MULTIPLY) && TrustingOp<T>(a,c,ADD)
				&& TrustingOp<T>(a, b, MULTIPLY)) // (a + (cd)/b) * b
			{
				aa = a;
				bb = 1;
				cc = 1;
				dd = 1;
				return true;
			}
			if ((ParanoidOp<T>(a, d, DIVIDE) || ParanoidOp(b, d, DIVIDE))
				&& TrustingOp<T>(a, b, MULTIPLY) && TrustingOp<T>(a,c,ADD)
				&& TrustingOp<T>(a, d, MULTIPLY)) // ((ab)/d + c)*d
			{
				aa = a;
				bb = 1;
				cc = 1;
				dd = 1;
				return true;
			}
			return false;
		}
		else if (aop == DIVIDE && cop == DIVIDE)
		{
			if (TrustingOp<T>(a, d, MULTIPLY) && TrustingOp<T>(c, b, MULTIPLY)
				&& TrustingOp<T>(a, c, ADD) && TrustingOp<T>(b, d, MULTIPLY))
			{
				cc = 1;
				dd = 1;
				if (ParanoidOp<T>(a, b, DIVIDE))
				{
					aa = a;
					bb = 1;
					return true;
				}
				aa = a;
				bb = b;
				return true;
			}
			return false;
		}
		return false;
	}

	class ParanoidNumber
	{
		
		public:
			typedef PARANOID_DIGIT_T digit_t;

			ParanoidNumber(digit_t value=0, Optype type = ADD) : m_value(value), m_op(type), m_next_term(NULL), m_next_factor(NULL)
			{
				Construct();
			}
			
			ParanoidNumber(const ParanoidNumber & cpy) : m_value(cpy.m_value), m_op(cpy.m_op), m_next_term(NULL), m_next_factor(NULL)
			{
				if (cpy.m_next_term != NULL)
				{
					m_next_term = new ParanoidNumber(*(cpy.m_next_term));
				}
				if (cpy.m_next_factor != NULL)
				{
					m_next_factor = new ParanoidNumber(*(cpy.m_next_factor));
				}
				Construct();
			}
			
			ParanoidNumber(const ParanoidNumber & cpy, Optype type) : ParanoidNumber(cpy)
			{
				m_op = type;
			}
			
			ParanoidNumber(const char * str);
			ParanoidNumber(const std::string & str) : ParanoidNumber(str.c_str()) {Construct();}
			
			virtual ~ParanoidNumber()
			{
				if (m_next_term != NULL)
					delete m_next_term;
				if (m_next_factor != NULL)
					delete m_next_factor;
				g_count--;
			}
			
			inline void Construct() {g_count++;}
			
			
			template <class T> T Convert() const;
			template <class T> T AddTerms() const;
			template <class T> T MultiplyFactors() const;
			template <class T> T Head() const {return (m_op == SUBTRACT) ? T(-m_value) : T(m_value);}
			

			
			
			double ToDouble() const {return Convert<double>();}
			float ToFloat() const {return Convert<float>();}
			digit_t Digit() const {return Convert<digit_t>();}
			
			bool Floating() const {return (m_next_term == NULL && m_next_factor == NULL);}
			bool Sunken() const {return !Floating();} // I could not resist...
			
			ParanoidNumber & operator+=(const ParanoidNumber & a);
			ParanoidNumber & operator-=(const ParanoidNumber & a);
			ParanoidNumber & operator*=(const ParanoidNumber & a);
			ParanoidNumber & operator/=(const ParanoidNumber & a);
			ParanoidNumber & operator=(const ParanoidNumber & a);
			
			
			bool operator<(const ParanoidNumber & a) const {return ToDouble() < a.ToDouble();}
			bool operator<=(const ParanoidNumber & a) const {return this->operator<(a) || this->operator==(a);}
			bool operator>(const ParanoidNumber & a) const {return !(this->operator<=(a));}
			bool operator>=(const ParanoidNumber & a) const {return !(this->operator<(a));}
			bool operator==(const ParanoidNumber & a) const {return ToDouble() == a.ToDouble();}
			bool operator!=(const ParanoidNumber & a) const {return !(this->operator==(a));}
			
			ParanoidNumber operator+(const ParanoidNumber & a) const
			{
				ParanoidNumber result(*this);
				result += a;
				return result;
			}
			ParanoidNumber operator-(const ParanoidNumber & a) const
			{
				ParanoidNumber result(*this);
				result -= a;
				return result;
			}
			ParanoidNumber operator*(const ParanoidNumber & a) const
			{
				ParanoidNumber result(*this);
				result *= a;
				return result;
			}
			ParanoidNumber operator/(const ParanoidNumber & a) const
			{
				ParanoidNumber result(*this);
				result /= a;
				return result;
			}
			
			std::string Str() const;
			static char OpChar(Optype op) 
			{
				static char opch[] = {'+','-','*','/'};
				return opch[(int)op];
			}
		
			static int64_t Paranoia() {return g_count;}
		
		private:
			static int64_t g_count;
			void Simplify();
			void SimplifyTerms();
			void SimplifyFactors();
			
			
			digit_t m_value;
			Optype m_op;
			ParanoidNumber * m_next_term;
			ParanoidNumber * m_next_factor;
	};

template <class T>
T ParanoidNumber::AddTerms() const
{
	T value(0);
	for (ParanoidNumber * a = m_next_term; a != NULL; a = a->m_next_term)
	{
		value += a->Head<T>() * a->MultiplyFactors<T>();
	}
	return value;
}

template <class T>
T ParanoidNumber::MultiplyFactors() const
{
	T value(1);
	for (ParanoidNumber * a = m_next_factor; a != NULL; a = a->m_next_factor)
	{
		if (a->m_op == DIVIDE)
			value /= (a->Head<T>() + a->AddTerms<T>());	
		else
			value *= (a->Head<T>() + a->AddTerms<T>());	
	}
	return value;
}



template <class T>
T ParanoidNumber::Convert() const
{
	return Head<T>() * MultiplyFactors<T>() + AddTerms<T>();
}



}

#endif //_PARANOIDNUMBER_H

