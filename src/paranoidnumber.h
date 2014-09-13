#ifndef _PARANOIDNUMBER_H
#define _PARANOIDNUMBER_H

#include <list>
#include <cfloat>
#include <map>
#include <string>
#include "log.h"
#include <fenv.h>

#define PARANOID_DIGIT_T float // we could theoretically replace this with a template
								// but let's not do that...

namespace IPDF
{
	typedef enum {ADD, SUBTRACT, MULTIPLY, DIVIDE, NOP} Optype;
	inline Optype InverseOp(Optype op)
	{
		return ((op == ADD) ? SUBTRACT :
				(op == SUBTRACT) ? ADD :
				(op == MULTIPLY) ? DIVIDE :
				(op == DIVIDE) ? MULTIPLY :
				(op == NOP) ? NOP : NOP);
	}
	inline Optype AdjacentOp(Optype op)
	{
		return ((op == ADD) ? MULTIPLY :
				(op == SUBTRACT) ? DIVIDE :
				(op == MULTIPLY) ? ADD :
				(op == DIVIDE) ? SUBTRACT :
				(op == NOP) ? NOP : NOP);
	}	
	
	inline char OpChar(int op) 
	{
		static char opch[] = {'+','-','*','/'};
		return (op < NOP && op >= 0) ? opch[op] : '?';
	}
	

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
	
	/**
	 * A ParanoidNumber
	 * Idea: Perform regular floating point arithmetic but rearrange operations to only ever use exact results
	 * Memory Usage: O(all of it)
	 * CPU Usage: O(all of it)
	 * Accuracy: O(gives better result for 0.3+0.3+0.3, gives same result for everything else, or worse result)
	 * 
	 * The ParanoidNumber basically stores 4 linked lists which can be split into two "dimensions"
	 *  1. Terms to ADD and terms to SUBTRACT
	 *  2. Factors to MULTIPLY and DIVIDE
	 * Because ADD and SUBTRACT are inverse operations and MULTIPLY and DIVIDE are inverse operations
	 * See paranoidnumber.cpp and the ParanoidNumber::Operation function
	 */
	class ParanoidNumber
	{
		
		public:
			typedef PARANOID_DIGIT_T digit_t;

			ParanoidNumber(digit_t value=0) : m_value(value)
			{
				Construct();
			}
			
			ParanoidNumber(const ParanoidNumber & cpy) : m_value(cpy.m_value)
			{
				Construct();
				for (int i = 0; i < NOP; ++i)
				{
					if (cpy.m_next[i] != NULL)
						m_next[i] = new ParanoidNumber(*(cpy.m_next[i]));
				}
			}
			
			ParanoidNumber(const char * str);
			ParanoidNumber(const std::string & str) : ParanoidNumber(str.c_str()) {Construct();}
			
			virtual ~ParanoidNumber();
			
			inline void Construct() 
			{
				for (int i = 0; i < NOP; ++i)
					m_next[i] = NULL;
				g_count++;
			}
			
			
			template <class T> T Convert() const;
			template <class T> T AddTerms(T value = T(0)) const;
			template <class T> T MultiplyFactors(T value = T(1)) const;
			template <class T> T Head() const {return (m_op == SUBTRACT) ? T(-m_value) : T(m_value);}
			

			
			
			double ToDouble() const {return Convert<double>();}
			float ToFloat() const {return Convert<float>();}
			digit_t Digit() const {return Convert<digit_t>();}
			
			bool Floating() const 
			{
				for (int i = 0; i < NOP; ++i)
				{
					if (m_next[i] != NULL)
						return false;
				}
				return true;
			}
			bool Sunken() const {return !Floating();} // I could not resist...
			
			bool Pure(Optype op) const
			{
				if (op == ADD || op == SUBTRACT)
					return (m_next[MULTIPLY] == NULL && m_next[DIVIDE] == NULL);
				return (m_next[ADD] == NULL && m_next[SUBTRACT] == NULL);
			}
			
			ParanoidNumber & operator+=(const ParanoidNumber & a);
			ParanoidNumber & operator-=(const ParanoidNumber & a);
			ParanoidNumber & operator*=(const ParanoidNumber & a);
			ParanoidNumber & operator/=(const ParanoidNumber & a);
			ParanoidNumber & operator=(const ParanoidNumber & a);
			
			
			ParanoidNumber * Operation(ParanoidNumber * b, Optype op, ParanoidNumber ** parent = NULL);
			bool Simplify(Optype op);
			
			
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

		
			static int64_t Paranoia() {return g_count;}
			
			std::string PStr() const;
		
		private:
			static int64_t g_count;
			void Simplify();
			void SimplifyTerms();
			void SimplifyFactors();
			
			
			digit_t m_value;
			Optype m_op;
			ParanoidNumber * m_next[4]; // Next by Operation
	};

template <class T>
T ParanoidNumber::AddTerms(T value) const
{
	ParanoidNumber * add = m_next[ADD];
	ParanoidNumber * sub = m_next[SUBTRACT];
	while (add != NULL && sub != NULL)
	{
		value += add->m_value * add->MultiplyFactors<T>();
		value -= sub->m_value * sub->MultiplyFactors<T>();
		add = add->m_next[ADD];
		sub = sub->m_next[SUBTRACT];
	}
	while (add != NULL)
	{
		value += add->m_value * add->MultiplyFactors<T>();
		add = add->m_next[ADD];
	}
	while (sub != NULL)
	{
		value -= sub->m_value * sub->MultiplyFactors<T>();
		sub = sub->m_next[SUBTRACT];;
	}
	return value;
}

template <class T>
T ParanoidNumber::MultiplyFactors(T value) const
{
	ParanoidNumber * mul = m_next[MULTIPLY];
	ParanoidNumber * div = m_next[DIVIDE];
	while (mul != NULL && div != NULL)
	{
		value *= (mul->m_value + mul->AddTerms<T>());
		value /= (div->m_value + div->AddTerms<T>());
		mul = mul->m_next[MULTIPLY];
		div = div->m_next[DIVIDE];
	}
	while (mul != NULL)
	{
		value *= (mul->m_value + mul->AddTerms<T>());
		mul = mul->m_next[MULTIPLY];
	}
	while (div != NULL)
	{
		value /= (div->m_value + div->AddTerms<T>());
		div = div->m_next[DIVIDE];
	}
	return value;
}



template <class T>
T ParanoidNumber::Convert() const
{
	return MultiplyFactors<T>(m_value) + AddTerms<T>(0);
}



}

#endif //_PARANOIDNUMBER_H

