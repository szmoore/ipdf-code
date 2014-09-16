#ifndef _PARANOIDNUMBER_H
#define _PARANOIDNUMBER_H

#include <list>
#include <cfloat>
#include <map>
#include <string>
#include "log.h"
#include <fenv.h>
#include <vector>
#include <cmath>

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

			ParanoidNumber(digit_t value=0) : m_value(value), m_cached_result(value)
			{
				Construct();
			}
			
			ParanoidNumber(const ParanoidNumber & cpy) : m_value(cpy.m_value), m_cached_result(cpy.m_cached_result)
			{
				Construct();
				for (int i = 0; i < NOP; ++i)
				{
					for (auto next : cpy.m_next[i])
						m_next[i].push_back(new ParanoidNumber(*next));
				}
			}
			
			ParanoidNumber(const char * str);
			ParanoidNumber(const std::string & str) : ParanoidNumber(str.c_str()) {}
			
			virtual ~ParanoidNumber();
			
			inline void Construct() 
			{
				g_count++;
			}
			
			
			template <class T> T Convert() const;
			digit_t GetFactors();
			digit_t GetTerms();
		

			double ToDouble() {return (double)Digit();}
			digit_t Digit();
			
			bool Floating() const 
			{
				return NoFactors() && NoTerms();
			}
			bool Sunken() const {return !Floating();} // I could not resist...
			
			bool NoFactors() const {return (m_next[MULTIPLY].size() == 0 && m_next[DIVIDE].size() == 0);}
			bool NoTerms() const {return (m_next[ADD].size() == 0 && m_next[SUBTRACT].size() == 0);}
			
			ParanoidNumber & operator+=(const ParanoidNumber & a);
			ParanoidNumber & operator-=(const ParanoidNumber & a);
			ParanoidNumber & operator*=(const ParanoidNumber & a);
			ParanoidNumber & operator/=(const ParanoidNumber & a);
			ParanoidNumber & operator=(const ParanoidNumber & a);
			
			ParanoidNumber * OperationTerm(ParanoidNumber * b, Optype op, ParanoidNumber ** merge_point = NULL, Optype * mop = NULL);
			ParanoidNumber * OperationFactor(ParanoidNumber * b, Optype op, ParanoidNumber ** merge_point = NULL, Optype * mop = NULL);
			ParanoidNumber * TrivialOp(ParanoidNumber * b, Optype op);
			ParanoidNumber * Operation(ParanoidNumber * b, Optype op, ParanoidNumber ** merge_point = NULL, Optype * mop = NULL);
			bool Simplify(Optype op);
			bool FullSimplify();
			
			bool operator<(ParanoidNumber & a) {return ToDouble() < a.ToDouble();}
			bool operator<=(ParanoidNumber & a) {return this->operator<(a) || this->operator==(a);}
			bool operator>(ParanoidNumber & a) {return !(this->operator<=(a));}
			bool operator>=(ParanoidNumber & a) {return !(this->operator<(a));}
			bool operator==(ParanoidNumber & a) {return ToDouble() == a.ToDouble();}
			bool operator!=(ParanoidNumber & a) {return !(this->operator==(a));}
			
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

			ParanoidNumber * CopyTerms()
			{
				ParanoidNumber * copy = new ParanoidNumber(*this);
				copy->m_value = 0;
				copy->Simplify(ADD);
				copy->Simplify(SUBTRACT);
				return copy;
			}
			
			ParanoidNumber * CopyFactors()
			{
				ParanoidNumber * copy = new ParanoidNumber(*this);
				copy->m_value = 1;
				copy->Simplify(MULTIPLY);
				copy->Simplify(DIVIDE);
				return copy;
			}


			static int64_t Paranoia() {return g_count;}
			
			std::string PStr() const;
		
		private:
			static int64_t g_count;
			void Simplify();
			void SimplifyTerms();
			void SimplifyFactors();
			
			
			digit_t m_value;
			Optype m_op;
			std::vector<ParanoidNumber*> m_next[4];
			digit_t m_cached_result;
			bool m_cache_valid;
	};
	



template <class T>
T ParanoidNumber::Convert() const
{
	if (!isnan(m_cached_result))
		return (T)m_cached_result;
	T value(m_value);
	for (auto mul : m_next[MULTIPLY])
	{
		value *= mul->Convert<T>();
	}
	for (auto div : m_next[DIVIDE])
	{
		value /= div->Convert<T>();
	}
	for (auto add : m_next[ADD])
		value += add->Convert<T>();
	for (auto sub : m_next[SUBTRACT])
		value -= sub->Convert<T>();
	return value;
}



}

#endif //_PARANOIDNUMBER_H

