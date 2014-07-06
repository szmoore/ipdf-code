/**
 * @file arbint.h
 * @brief Arbitrary sized integer declarations
 * @see arbint.cpp
 */
 
#ifndef _ARBINT_H
#define _ARBINT_H

#include "common.h"

namespace IPDF
{
	class Arbint
	{
		public:
			typedef uint64_t digit_t;
		
			Arbint(int64_t i);
			Arbint(const std::vector<digit_t> & digits);
			Arbint(unsigned n, digit_t d0, ...);
			Arbint(const std::string & str, const std::string & base="0123456789");
			virtual ~Arbint() {}
			Arbint(const Arbint & cpy);
			
			int64_t AsDigit() const
			{
				int64_t digit = (m_digits.size() == 1) ? m_digits[0] : 0x7FFFFFFFFFFFFFFF;
				return (m_sign) ? -digit : digit;
			}
			
			inline bool Sign() const {return m_sign;}
			inline char SignChar() const {return (m_sign) ? '-' : '+';}
			std::string DigitStr() const;

			std::string Str(const std::string & base="0123456789") const;
			inline std::string Str(const char * base) const
			{
				return Str(std::string(base));
			}
			
			Arbint & operator=(const Arbint & equ);
			Arbint & operator+=(const Arbint & add);
			Arbint & operator-=(const Arbint & sub);
			Arbint & operator*=(const Arbint & mul);
			void Division(const Arbint & div, Arbint & result, Arbint & modulo) const;
			
			Arbint & operator<<=(unsigned amount);
			Arbint & operator>>=(unsigned amount);
			
			inline Arbint operator+(const Arbint & add) const 
			{
				Arbint a(*this);
				a += add;
				return a;
			}
			inline Arbint operator-(const Arbint & add) const 
			{
				Arbint a(*this);
				a -= add;
				return a;
			}

			inline Arbint operator-()
			{
				Arbint a(*this);
				a.m_sign = !a.m_sign;
				return a;
			}
			
			inline Arbint operator*(const Arbint & mul) const
			{
				Arbint a(*this);
				a *= mul;
				return a;
			}
			
			inline Arbint & operator/=(const Arbint & div)
			{
				Arbint result(0L);
				Arbint remainder(0L);
				this->Division(div, result, remainder);
				this->operator=(result);
				return *this;
			}
			inline Arbint operator/(const Arbint & div)
			{
				Arbint cpy(*this);
				cpy /= div;
				return cpy;
			}
			inline Arbint operator%(const Arbint & div)
			{
				Arbint result(0L);
				Arbint remainder(0L);
				this->Division(div, result, remainder);
				return remainder;
			}
			
			bool operator==(const Arbint & equ) const;
			bool operator<(const Arbint & less) const;

			inline bool operator!=(const Arbint & equ) const 
			{
				return !(this->operator==(equ));
			}
			inline bool operator<=(const Arbint & leq) const 
			{
				return (this->operator==(leq) || this->operator<(leq));
			}
			
			inline bool operator>=(const Arbint & leq) const 
			{
				return (this->operator==(leq) || this->operator>(leq));
			}
			inline bool operator>(const Arbint & grea) const
			{
				return !(this->operator<=(grea));
			}
			inline Arbint operator>>(unsigned amount) const
			{
				Arbint result(*this);
				result >>= amount;
				return result;
			}
			inline Arbint operator<<(unsigned amount) const
			{
				Arbint result(*this);
				result <<= amount;
				return result;
			}
			bool IsZero() const;
			
			inline operator double() const 
			{
				double acc = 0;
				for(int i = m_digits.size()-1; i >= 0; --i)
				{
					acc += (double)m_digits[i];
					acc *= (double)UINT64_MAX + 1.0;
				}
				if (m_sign) acc *= -1;
				return acc;
			}
			inline operator int64_t() const {return AsDigit();}
			//inline operator int() const {return int(AsDigit());}
			
			unsigned Shrink();
			
			inline Arbint Abs() const {Arbint a(*this); a.m_sign = false; return a;}
		private:		
				
			Arbint & AddBasic(const Arbint & add);
			Arbint & SubBasic(const Arbint & sub);
			
			bool GetBit(unsigned i) const;
			void BitClear(unsigned i);
			void BitSet(unsigned i);
			
	
			std::vector<digit_t> m_digits;
			bool m_sign;
			void Zero();
			
	};	



extern "C"
{
	typedef uint64_t digit_t;
	digit_t add_digits(digit_t * dst, digit_t * add, digit_t size);
	digit_t sub_digits(digit_t * dst, digit_t * add, digit_t size);
	digit_t mul_digits(digit_t * dst, digit_t mul, digit_t size);
	digit_t div_digits(digit_t * dst, digit_t div, digit_t size, digit_t * rem);
}



}
#endif //_ARBINT_H
