/**
 * @file gmpint.h
 * @brief Wraps to GMP mpz_t type using inlines
 */

#ifndef _GMPINT_H
#define _GMPINT_H

#include <gmp.h>
#include <string>

class Gmpint
{
	public:
		Gmpint(int64_t i) {mpz_init_set_si(m_op, i);}
		Gmpint(const std::string & str, int base=10) {mpz_init_set_str(m_op, str.c_str(), base);}
		Gmpint(const Gmpint & cpy) {mpz_init(m_op); mpz_set(m_op, cpy.m_op);}
		virtual ~Gmpint() {} //TODO: Do we need to delete m_op somehow?
		
		
		operator int64_t() const {return mpz_get_si(m_op);}
		operator uint64_t() const {return mpz_get_ui(m_op);}
		operator double() const {return mpz_get_d(m_op);}
		std::string Str(int base = 10) const
		{
			//TODO: Make less hacky, if we care.
			char * buff = mpz_get_str(NULL, 10, m_op);
			std::string result(buff);
			free(buff);
			return result;
		}
		
		Gmpint & operator=(const Gmpint & equ) {mpz_set(m_op, equ.m_op); return *this;}
		Gmpint & operator+=(const Gmpint & add) {mpz_add(m_op, m_op, add.m_op); return *this;}
		Gmpint & operator-=(const Gmpint & sub) {mpz_sub(m_op, m_op, sub.m_op); return *this;}
		Gmpint & operator*=(const Gmpint & mul) {mpz_mul(m_op, m_op, mul.m_op); return *this;}
		Gmpint & operator/=(const Gmpint & div) {mpz_div(m_op, m_op, div.m_op); return *this;}
		
		Gmpint operator+(const Gmpint & add) const {Gmpint a(*this); a += add; return a;}
		Gmpint operator-(const Gmpint & sub) const {Gmpint a(*this); a -= sub; return a;}
		Gmpint operator*(const Gmpint & mul) const {Gmpint a(*this); a *= mul; return a;}
		Gmpint operator/(const Gmpint & div) const {Gmpint a(*this); a /= div; return a;}
		Gmpint operator%(const Gmpint & div) const {Gmpint a(*this); mpz_mod(a.m_op, a.m_op, div.m_op); return a;}
		Gmpint operator-() const {return (Gmpint(0L)-*this);}
 		
		
		bool operator==(const Gmpint & cmp) const {return mpz_cmp(m_op, cmp.m_op) == 0;}
		bool operator!=(const Gmpint & cmp) const {return mpz_cmp(m_op, cmp.m_op) != 0;}
		bool operator<(const Gmpint & cmp) const {return mpz_cmp(m_op, cmp.m_op) < 0;}
		bool operator>(const Gmpint & cmp) const {return mpz_cmp(m_op, cmp.m_op) > 0;}
		bool operator<=(const Gmpint & cmp) const {return mpz_cmp(m_op, cmp.m_op) <= 0;}
		bool operator>=(const Gmpint & cmp) const {return mpz_cmp(m_op, cmp.m_op) >= 0;}
		
		Gmpint Abs() const {Gmpint a(*this); mpz_abs(a.m_op, a.m_op); return a;}
		
		
	private:
		Gmpint(const mpz_t & op) : m_op(op) {}
		mpz_t m_op;
};	




#endif //_GMPINT_H
