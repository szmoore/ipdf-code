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

#include "rational.h"

using namespace std;

namespace IPDF
{
	
/** Absolute value hackery **/
template <> Arbint Tabs(const Arbint & a)
{
	//Debug("Called");
	return a.Abs();
}


Arbint::Arbint(int64_t i) : m_digits(1), m_sign(i < 0)
{
	m_digits[0] = llabs(i);
}

Arbint::Arbint(unsigned n, digit_t d0, ...) : m_digits(n), m_sign(false)
{
	va_list ap;
	va_start(ap, d0);
	if (n >= 1)
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
	m_digits = cpy.m_digits;
	m_sign = cpy.m_sign;
	return *this;
}

void Arbint::Zero()
{
	m_digits.resize(1, 0L);
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
	remainder = 0;
	result = 0;
	for (int i = 8*sizeof(digit_t)*m_digits.size(); i >= 0; --i)
	{
		remainder <<= 1;
		if (GetBit(i))
			remainder.BitSet(0);
		else
			remainder.BitClear(0);
		if (remainder >= div)
		{
			remainder -= div;
			result.BitSet(i);
		}
	}
	result.m_sign = !(m_sign == div.m_sign);
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
	Arbint cpy(*this);
	
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
	if (m_sign != equ.m_sign) 
		return false;
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
	//ss << std::hex << std::setfill('0');
	for (unsigned i = 0; i < m_digits.size(); ++i)
	{
		if (i != 0) ss << ',';
		//ss << std::setw(2*sizeof(digit_t)) << static_cast<digit_t>(m_digits[i]);
		ss << static_cast<digit_t>(m_digits[i]);
	}
	return ss.str();
}

Arbint & Arbint::operator>>=(unsigned amount)
{
	// Shift by whole number of digits
	unsigned whole = amount/(8*sizeof(digit_t));
	unsigned old_size = m_digits.size();
	
	if (whole >= old_size)
	{
		m_digits.resize(1,0L);
		m_digits[0] = 0L;
		return *this;
	}
	memmove(m_digits.data(), m_digits.data()+whole, sizeof(digit_t)*(old_size-whole));
	m_digits.resize(old_size-whole, 0L);
	
	// Shift by partial amount
	amount = amount %(8*sizeof(digit_t));
	if (amount == 0)
		return *this;
	
	digit_t underflow = 0L;
	for (int i = (int)(m_digits.size()-1); i >= 0; --i)
	{
		unsigned shl = (8*sizeof(digit_t)-amount);
		digit_t next_underflow = (m_digits[i] << shl);
		//digit_t mask_upper = ~(0L >> amount);
		m_digits[i] = (m_digits[i] >> amount);// & mask_upper;
		m_digits[i] |= underflow;
		underflow = next_underflow;
	}
	return *this;
}

Arbint & Arbint::operator<<=(unsigned amount)
{
	// Shift by whole number of digits
	unsigned whole = amount/(8*sizeof(digit_t));
	unsigned old_size = m_digits.size();
	m_digits.resize(m_digits.size() + whole);
	memmove(m_digits.data()+whole, m_digits.data(), sizeof(digit_t)*old_size);
	memset(m_digits.data(), 0L, whole*sizeof(digit_t));
	
	
	
	// Partial shifting
	amount = amount % (8*sizeof(digit_t));
	if (amount == 0)
		return *this;
		
	//Debug("Shift by %u from %u", amount, whole);
	digit_t overflow = 0L;
	for (unsigned i = whole; i < m_digits.size(); ++i)
	{
		//Debug("Digit is %.16lx", m_digits[i]);
		unsigned shr = (8*sizeof(digit_t)-amount);
		//Debug("shr is %u", shr);
		digit_t next_overflow = (m_digits[i] >> shr);
		//Debug("Next overflow %.16lx", next_overflow);
		m_digits[i] <<= amount;
		//Debug("Before overflow %.16lx", m_digits[i]);
		m_digits[i] |= overflow;
		overflow = next_overflow;
	}
	if (overflow != 0L)
		m_digits.push_back(overflow);
		
	return *this;
}

bool Arbint::GetBit(unsigned i) const
{
	unsigned digit = i/(8*sizeof(digit_t));
	if (digit >= m_digits.size())
		return false;
		
	i = i % (8*sizeof(digit_t));
	
	return (m_digits[digit] & (1L << i));
	
}

void Arbint::BitClear(unsigned i)
{
	unsigned digit = i/(8*sizeof(digit_t));
	if (digit >= m_digits.size())
		return;
	i = i % (8*sizeof(digit_t));
	m_digits[digit] &= ~(1L << i);	
}

void Arbint::BitSet(unsigned i)
{
	unsigned digit = i/(8*sizeof(digit_t));
	if (digit >= m_digits.size())
	{
		m_digits.resize(digit+1, 0L);
	}
	i = i % (8*sizeof(digit_t));
	m_digits[digit] |= (1L << i);		
}

}
