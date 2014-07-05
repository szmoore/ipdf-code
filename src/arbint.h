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
			Arbint(unsigned n, digit_t d0, ...);
			Arbint(const std::string & str, const std::string & base="0123456789");
			~Arbint() {}
			Arbint(const Arbint & cpy);
			
			
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
	
			
			bool operator==(const Arbint & equ) const;
			
			
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
			inline bool operator!=(const Arbint & equ) const 
			{
				return !this->operator==(equ);
			}
			
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
}

}
#endif //_ARBINT_H
