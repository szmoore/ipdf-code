/**
 * @file gmpint.h
 * @brief Wraps to GMP mpq_t type using inlines
 */

#ifndef _GMPRAT_H
#define _GMPRAT_H

#include <gmp.h>
#include <string>

class Gmprat
{
	public:
		Gmprat(double d=0) {mpq_init(m_op); mpq_set_d(m_op, d);}
		//Gmprat(int64_t p = 0, int64_t q = 1) {mpq_init(m_op); mpq_set_si(m_op, p, q);}
		//Gmprat(const std::string & str, int base=10) {mpq_init_set_str(m_op, str.c_str(), base);}
		Gmprat(const Gmprat & cpy) {mpq_init(m_op); mpq_set(m_op, cpy.m_op);}
		virtual ~Gmprat() {mpq_clear(m_op);}
		
		//operator int64_t() const {return mpq_get_si(m_op);}
		//operator uint64_t() const {return mpq_get_ui(m_op);}
		//operator double() const {return mpq_get_d(m_op);}
		double ToDouble() const {return mpq_get_d(m_op);}
		std::string Str(int base = 10) const
		{
			//TODO: Make less hacky, if we care.
			char * buff = mpq_get_str(NULL, 10, m_op);
			std::string result(buff);
			free(buff);
			return result;
		}
		
		Gmprat & operator=(const Gmprat & equ) {mpq_set(m_op, equ.m_op); return *this;}
		Gmprat & operator+=(const Gmprat & add) {mpq_add(m_op, m_op, add.m_op); return *this;}
		Gmprat & operator-=(const Gmprat & sub) {mpq_sub(m_op, m_op, sub.m_op); return *this;}
		Gmprat & operator*=(const Gmprat & mul) {mpq_mul(m_op, m_op, mul.m_op); return *this;}
		Gmprat & operator/=(const Gmprat & div) {mpq_div(m_op, m_op, div.m_op); return *this;}
		
		Gmprat operator+(const Gmprat & add) const {Gmprat a(*this); a += add; return a;}
		Gmprat operator-(const Gmprat & sub) const {Gmprat a(*this); a -= sub; return a;}
		Gmprat operator*(const Gmprat & mul) const {Gmprat a(*this); a *= mul; return a;}
		Gmprat operator/(const Gmprat & div) const {Gmprat a(*this); a /= div; return a;}
		//Gmprat operator%(const Gmprat & div) const {Gmprat a(*this); mpq_mod(a.m_op, a.m_op, div.m_op); return a;}
		Gmprat operator-() const {return (Gmprat(0L)-*this);}
 		
		
		bool operator==(const Gmprat & cmp) const {return mpq_cmp(m_op, cmp.m_op) == 0;}
		bool operator!=(const Gmprat & cmp) const {return mpq_cmp(m_op, cmp.m_op) != 0;}
		bool operator<(const Gmprat & cmp) const {return mpq_cmp(m_op, cmp.m_op) < 0;}
		bool operator>(const Gmprat & cmp) const {return mpq_cmp(m_op, cmp.m_op) > 0;}
		bool operator<=(const Gmprat & cmp) const {return mpq_cmp(m_op, cmp.m_op) <= 0;}
		bool operator>=(const Gmprat & cmp) const {return mpq_cmp(m_op, cmp.m_op) >= 0;}
		
		Gmprat Abs() const {Gmprat a(*this); mpq_abs(a.m_op, a.m_op); return a;}
		
		
	private:
		mpq_t m_op;
};	




#endif //_GMPRAT_H

