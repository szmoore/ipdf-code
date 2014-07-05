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
			typedef int64_t digit_t;
		
			Arbint(digit_t i);
			Arbint(const std::vector<digit_t> & digits);
			Arbint(unsigned n, digit_t d0, ...);
			Arbint(const std::string & str, const std::string & base="0123456789");
			~Arbint() {}
			Arbint(const Arbint & cpy);
			
			digit_t AsDigit() const 
			{
				return (m_sign) ? -m_digits[0] : m_digits[0];
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
				return (this->operator==(leq) || this->operator<(leq));
			}
			inline bool operator>(const Arbint & grea) const
			{
				return !(this->operator<=(grea));
			}
			
			bool IsZero() const;
			
			unsigned Shrink();
		private:		
				
			Arbint & AddBasic(const Arbint & add);
			Arbint & SubBasic(const Arbint & sub);
	
			std::vector<digit_t> m_digits;
			bool m_sign;
			void Zero();
			
	};	

extern "C"
{
	typedef int64_t digit_t;
	digit_t add_digits(digit_t * dst, digit_t * add, digit_t size);
	digit_t sub_digits(digit_t * dst, digit_t * add, digit_t size);
	digit_t mul_digits(digit_t * dst, digit_t mul, digit_t size);
}

}
#endif //_ARBINT_H
