/**
 * @file gmpint.h
 * @brief Wraps to GMP mpq_t type using inlines
 */

#ifndef _GMPRAT_H
#define _GMPRAT_H

#include <gmp.h>
#include <string>
#include <climits>

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
			if (std::isinf(p))
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
		
		double LogE() const
		{
			// log(a/b) = log(a) - log(b)
			// And if a is represented in base B as:
			// a = a_N B^N + a_{N-1} B^{N-1} + ... + a_0
			// => log(a) \approx log(a_N B^N)
			// = log(a_N) + N log(B)
			// where B is the base; ie number of values representable with one digit, ie: ULONG_MAX
			
			static double logB = log(ULONG_MAX); // compiler should optimise this anyway?
			
			// Undefined logs (should probably return NAN if -ve, not -INFINITY, but meh)
			if (mpz_get_ui(mpq_numref(m_op)) == 0 || mpz_sgn(mpq_numref(m_op)) < 0)
				return -INFINITY;				
	
			// Log of numerator
			double lognum = log(mpq_numref(m_op)->_mp_d[abs(mpq_numref(m_op)->_mp_size) - 1]);
			lognum += (abs(mpq_numref(m_op)->_mp_size)-1) * logB;
			
			// Subtract log of denominator, if it exists
			// Note that denominator is not explicitly set to 1, this caused a lot of headache
			if (abs(mpq_denref(m_op)->_mp_size) > 0)
			{
				lognum -= log(mpq_denref(m_op)->_mp_d[abs(mpq_denref(m_op)->_mp_size)-1]);
				lognum -= (abs(mpq_denref(m_op)->_mp_size)-1) * logB;
			}
			return lognum;
		}
		
		double Log10() const {return LogE() / log(10.0);}

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
		
		size_t Size() const
		{
			return sizeof(uint64_t) * (mpq_numref(m_op)->_mp_alloc + mpq_denref(m_op)->_mp_alloc);
		}
		
	private:
		friend std::ostream& operator<<(std::ostream& os, const Gmprat & fith);
		mpq_t m_op;
};	

inline std::ostream & operator<<(std::ostream & os, const Gmprat & fith)
{
	os << fith.Str();
	return os;
}

inline std::string Str(const Gmprat & g) {return g.Str();}
inline double Log10(const Gmprat & g) {return g.Log10();}
inline size_t Size(const Gmprat & g) {return g.Size();}
inline Gmprat Abs(const Gmprat & g) {return g.Abs();}


#endif //_GMPRAT_H

