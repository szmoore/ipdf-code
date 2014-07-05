/**
 * @file arbint.cpp
 * @brief Arbitrary sized integer definitions
 * @see arbint.h
 * @see add_digits_asm.s
 * @see sub_digits_asm.s
 * @see mul_digits_asm.s
 */

#include "arbint.h"
#include <algorithm>
#include <cstring>


#include <string>
#include <iomanip>
#include <sstream>
#include <cstdarg>

using namespace std;

namespace IPDF
{

Arbint::Arbint(digit_t i) : m_digits(1), m_sign(i < 0)
{
	m_digits[0] = llabs(i);
}

Arbint::Arbint(unsigned n, digit_t d0, ...) : m_digits(n), m_sign(false)
{
	va_list ap;
	va_start(ap, d0);
	if (n > 1)
		m_digits[0] = d0;
	for (unsigned i = 1; i < n; ++i)
	{
		m_digits[i] = va_arg(ap, digit_t);
	}
	va_end(ap);
}

Arbint::Arbint(const Arbint & cpy) : m_digits(cpy.m_digits), m_sign(cpy.m_sign)
{
	
}

Arbint::Arbint(const vector<digit_t> & digits) : m_digits(digits), m_sign(false)
{
	
}

Arbint & Arbint::operator=(const Arbint & cpy)
{
	memmove(m_digits.data(), cpy.m_digits.data(), 
		sizeof(digit_t)*min(m_digits.size(), cpy.m_digits.size()));
	if (cpy.m_digits.size() > m_digits.size())
	{
		unsigned old_size = m_digits.size();
		m_digits.resize(cpy.m_digits.size());
		memset(m_digits.data()+old_size, 0, sizeof(digit_t)*m_digits.size()-old_size);
	}
	return *this;
}

void Arbint::Zero()
{
	memset(m_digits.data(), 0, sizeof(digit_t)*m_digits.size());
}

unsigned Arbint::Shrink()
{
	if (m_digits.size() <= 1)
		return 0;
	unsigned i;
	for (i = m_digits.size()-1; (i > 0 && m_digits[i] != 0L); --i);
	unsigned result = m_digits.size() - i;
	m_digits.resize(i);
	return result;
}

Arbint & Arbint::operator*=(const Arbint & mul)
{
	vector<digit_t> new_digits(m_digits.size(), 0L);
	new_digits.reserve(new_digits.size()+mul.m_digits.size());
	for (unsigned i = 0; i < mul.m_digits.size(); ++i)
	{
		vector<digit_t> step(m_digits.size()+i, 0L);
		memcpy(step.data()+i, m_digits.data(), sizeof(digit_t)*m_digits.size());
		
		digit_t overflow = mul_digits((digit_t*)step.data()+i, mul.m_digits[i], m_digits.size());
		if (overflow != 0L)
		{
			step.push_back(overflow);
		}
		new_digits.resize(max(new_digits.size(), step.size()), 0L);
		digit_t carry = add_digits((digit_t*)new_digits.data(), step.data(), step.size());
		if (carry != 0L)
		{
			new_digits.push_back(carry);
		}
	}
	
	m_digits.swap(new_digits);
	m_sign = !(m_sign == mul.m_sign);
	return *this;
}

void Arbint::Division(const Arbint & div, Arbint & result, Arbint & remainder) const
{
	//TODO: Optimise?
	remainder = *this;
	result = 0L;
	while ((remainder -= div) > Arbint(0L))
	{
		//Debug("Remainder %c%s", remainder.SignChar(), remainder.DigitStr().c_str());
		//Debug("Result %c%s + 1", result.SignChar(), result.DigitStr().c_str());
		result += 1;
	}
	remainder += div;
}

Arbint & Arbint::operator+=(const Arbint & add)
{
	if (m_sign == add.m_sign)
	{
		// -a + -b == -(a + b)
		return AddBasic(add);
	}
	
	if (m_sign)
	{
		// -a + b == -(a - b)
		m_sign = false;
		SubBasic(add);
		m_sign = !m_sign;
	}
	else
	{
		// a + -b == a - b
		SubBasic(add);
	}
	return *this;
}

Arbint & Arbint::operator-=(const Arbint & sub)
{
	if (m_sign == sub.m_sign)
		return SubBasic(sub);
	return AddBasic(sub);
}

Arbint & Arbint::AddBasic(const Arbint & add)
{
	if (add.m_digits.size() >= m_digits.size())
	{
		m_digits.resize(add.m_digits.size()+1,0L);
	}
	
	digit_t carry = add_digits((digit_t*)m_digits.data(), 
			(digit_t*)add.m_digits.data(), add.m_digits.size());
	if (carry != 0L)
		m_digits[m_digits.size()-1] = carry;
	else if (m_digits.back() == 0L)
		m_digits.resize(m_digits.size()-1);
	return *this;
}

Arbint & Arbint::SubBasic(const Arbint & sub)
{
	if (sub.m_digits.size() >= m_digits.size())
	{
		m_digits.resize(sub.m_digits.size(),0L);
	}
	digit_t borrow = sub_digits((digit_t*)m_digits.data(), 
			(digit_t*)sub.m_digits.data(), sub.m_digits.size());
		
		
	//TODO: Write ASM to do this bit?
	if (borrow != 0L)
	{
		m_sign = !m_sign;
		for (unsigned i = 0; i < m_digits.size(); ++i)
			m_digits[i] = -m_digits[i];
	}
	return *this;
}


string Arbint::Str(const string & base) const
{
	string s("");
	for (unsigned i = 0; i < m_digits.size(); ++i)
	{
		digit_t w = m_digits[i];
		do
		{
			digit_t q = w % 10;
			w /= 10;
			s += ('0' + q);
		}
		while (w > 0);
		if (i+1 < m_digits.size()) s += ",";	
	}
	reverse(s.begin(), s.end());
	return s;
}

bool Arbint::IsZero() const
{
	for (unsigned i = m_digits.size()-1; i > 0; --i)
	{
		if (m_digits[i] != 0L) return false;
	}
	return (m_digits[0] == 0L);
}

bool Arbint::operator==(const Arbint & equ) const
{
	if (m_sign != equ.m_sign) return false;
	unsigned min_size = m_digits.size();
	const Arbint * larger = &equ;
	if (m_digits.size() > equ.m_digits.size())
	{
		min_size = equ.m_digits.size();
		larger = this;
	}
	
	if (memcmp(m_digits.data(), equ.m_digits.data(), sizeof(digit_t)*min_size) != 0)
		return false;
	
	for (unsigned i = min_size; i < larger->m_digits.size(); ++i)
	{
		if (larger->m_digits[i] != 0L)
			return false;
	}
	return true;
}

bool Arbint::operator<(const Arbint & less) const
{
	Arbint cpy(*this);
	cpy -= less;
	return (cpy.m_sign && !cpy.IsZero());
}

string Arbint::DigitStr() const
{
	stringstream ss("");
	ss << std::hex << std::setfill('0');
	for (unsigned i = 0; i < m_digits.size(); ++i)
	{
		if (i != 0) ss << ',';
		ss << std::setw(2*sizeof(digit_t)) << static_cast<digit_t>(m_digits[i]);
	}
	return ss.str();
}

}
