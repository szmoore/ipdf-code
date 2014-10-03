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
		operator double() const {return mpq_get_d(m_op);}
		//operator float() const {return (float)ToDouble();}
		double ToDouble() const {return mpq_get_d(m_op);}
		std::string Str() const
		{
			//TODO: Make less hacky, if we care.
			// Convert to scientific notation
			if (Negative())
			{
				return "-" + operator-().Str();
			}
			double p = Log10();
			if (isinf(p))
				return "0";
				
			int P = (int)p;
			double C = pow(10.0, p - P);
			std::stringstream s;
			s << C;
			if (P != 0)
				s << "e"<< P;
			//s << "("<<ToDouble()<<")";
			return s.str();
		}
		
		double Log10() const
		{
			mpz_t num; mpz_init(num); mpq_get_num(num, m_op);
			mpz_t den; mpz_init(den); mpq_get_den(den, m_op);
			
			double lognum = 0;
			double logden = 0;
			while (mpz_sizeinbase(num, 10) > 10)
			{

				mpz_div_ui(num, num, 1e10);
				lognum += 10;
			}
			uint64_t n = mpz_get_ui(num);
			if (n == 0)
			{
				return -INFINITY;
			}
			lognum += log(n)/log(10.0);
			//Debug("%lu", mpz_get_ui(den));
			while (mpz_sizeinbase(den, 10) > 10)
			{
				mpz_div_ui(den, den, 1e10);
				logden += 10;
			}
			uint64_t d = mpz_get_ui(den);
			// if d is zero, its been rounded down we hope
			if (d != 0)
				logden += log(d)/log(10.0);
			
			return (lognum - logden);
		}
		

		bool Negative() const {return (mpz_sgn(mpq_numref(m_op)) < 0);}
		
		Gmprat & operator=(const Gmprat & equ) {mpq_set(m_op, equ.m_op); return *this;}
		Gmprat & operator=(const double & equ) {mpq_set_d(m_op, equ); return *this;}
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
		friend std::ostream& operator<<(std::ostream& os, const Gmprat & fith);
		mpq_t m_op;
};	

inline std::ostream & operator<<(std::ostream & os, const Gmprat & fith)
{
	os << fith.Str();
	return os;
}




#endif //_GMPRAT_H

