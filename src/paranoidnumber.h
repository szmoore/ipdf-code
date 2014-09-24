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
#include <cassert> // it's going to be ok
#include <set>

#define PARANOID_DIGIT_T double // we could theoretically replace this with a template
								// but let's not do that...
								

//#define PARANOID_CACHE_RESULTS

//#define PARANOID_USE_ARENA
#define PARANOID_SIZE_LIMIT 0


// Define to compare all ops against double ops and check within epsilon
#define PARANOID_COMPARE_EPSILON 1e-6
#define CompareForSanity(...) this->ParanoidNumber::CompareForSanityEx(__func__, __FILE__, __LINE__, __VA_ARGS__)

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

			ParanoidNumber(PARANOID_DIGIT_T value=0) : m_value(value), m_next()
			{
				#ifdef PARANOID_SIZE_LIMIT
					m_size = 0;
				#endif
				#ifdef PARANOID_CACHE_RESULTS
					m_cached_result = value;
				#endif 
			}
			
			ParanoidNumber(const ParanoidNumber & cpy) : m_value(cpy.m_value), m_next()
			{
				
				#ifdef PARANOID_SIZE_LIMIT
					m_size = cpy.m_size;
				#endif
				#ifdef PARANOID_CACHE_RESULTS
					m_cached_result = cpy.m_cached_result;
				#endif 
				for (int i = 0; i < NOP; ++i)
				{
					for (auto next : cpy.m_next[i])
					{
						if (next != NULL) // why would this ever be null
							m_next[i].push_back(new ParanoidNumber(*next)); // famous last words...
					}
				}
				//assert(SanityCheck());
			}
			
			//ParanoidNumber(const char * str);
			ParanoidNumber(const std::string & str);// : ParanoidNumber(str.c_str()) {}
			
			virtual ~ParanoidNumber();

			
			bool SanityCheck(std::set<ParanoidNumber*> & visited) const;
			bool SanityCheck() const 
			{
				std::set<ParanoidNumber*> s; 
				return SanityCheck(s);
			}
			
			template <class T> T Convert() const;
			digit_t GetFactors() const;
			digit_t GetTerms() const;
		
			// This function is declared const purely to trick the compiler.
			// It is not actually const, and therefore, none of the other functions that call it are const either.
			digit_t Digit() const;
			
			// Like this one. It isn't const.
			double ToDouble() const {return (double)Digit();}
			
			// This one is probably const.
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
			
			ParanoidNumber & operator+=(const digit_t & a);
			ParanoidNumber & operator-=(const digit_t & a);
			ParanoidNumber & operator*=(const digit_t & a);
			ParanoidNumber & operator/=(const digit_t & a);
			ParanoidNumber & operator=(const digit_t & a);
			
			
			ParanoidNumber * OperationTerm(ParanoidNumber * b, Optype op, ParanoidNumber ** merge_point = NULL, Optype * mop = NULL);
			ParanoidNumber * OperationFactor(ParanoidNumber * b, Optype op, ParanoidNumber ** merge_point = NULL, Optype * mop = NULL);
			ParanoidNumber * TrivialOp(ParanoidNumber * b, Optype op);
			ParanoidNumber * Operation(ParanoidNumber * b, Optype op, ParanoidNumber ** merge_point = NULL, Optype * mop = NULL);
			bool Simplify(Optype op);
			bool FullSimplify();
			
			
			// None of these are actually const
			bool operator<(const ParanoidNumber & a) const {return ToDouble() < a.ToDouble();}
			bool operator<=(const ParanoidNumber & a) const {return this->operator<(a) || this->operator==(a);}
			bool operator>(const ParanoidNumber & a) const {return !(this->operator<=(a));}
			bool operator>=(const ParanoidNumber & a) const {return !(this->operator<(a));}
			bool operator==(const ParanoidNumber & a) const {return ToDouble() == a.ToDouble();}
			bool operator!=(const ParanoidNumber & a) const {return !(this->operator==(a));}
			
			ParanoidNumber operator-() const
			{
				ParanoidNumber neg(0);
				neg -= *this;
				return neg;
			}
			
			ParanoidNumber operator+(const ParanoidNumber & a) const
			{
				ParanoidNumber result(*this);
				a.SanityCheck();
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
				if (!result.SanityCheck())
				{
					Fatal("Blargh");
				}
				return result;
			}
			ParanoidNumber operator/(const ParanoidNumber & a) const
			{
				ParanoidNumber result(*this);
				result /= a;
				return result;
			}
			
			std::string Str() const;

			inline void CompareForSanityEx(const char * func, const char * file, int line, const digit_t & compare, const digit_t & arg, const digit_t & eps = PARANOID_COMPARE_EPSILON)
			{
				if (fabs(Digit() - compare) > eps)
				{
					Error("Called via %s(%lf) (%s:%d)", func, arg, file, line);
					Error("Failed: %s", Str().c_str());
					Fatal("This: %.30lf vs Expected: %.30lf", Digit(), compare);
				}
			}

			
			std::string PStr() const;
			
			#ifdef PARANOID_USE_ARENA
			void * operator new(size_t byes);
			void operator delete(void * p);
			#endif //PARANOID_USE_ARENA
		
		private:
		
			void Simplify();
			void SimplifyTerms();
			void SimplifyFactors();
			
			digit_t m_value;	
			#ifdef PARANOID_CACHE_RESULTS
				digit_t m_cached_result;
			#endif
			std::vector<ParanoidNumber*> m_next[4];
			#ifdef PARANOID_SIZE_LIMIT
				int64_t m_size;
			#endif //PARANOID_SIZE_LIMIT
			
			#ifdef PARANOID_USE_ARENA
			class Arena
			{
				public:
					Arena(int64_t block_size = 10000000);
					~Arena();
					
					void * allocate(size_t bytes);
					void deallocate(void * p);
					
				private:
					struct Block
					{
						void * memory;
						int64_t used;
					};
				
					std::vector<Block> m_blocks;
					int64_t m_block_size;
					
					void * m_spare;
				
			};
			
			static Arena g_arena;
			#endif //PARANOID_USE_ARENA

		
	};
	



template <class T>
T ParanoidNumber::Convert() const
{
	#ifdef PARANOID_CACHE_RESULTS
	if (!isnan((float(m_cached_result))))
		return (T)m_cached_result;
	#endif
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

