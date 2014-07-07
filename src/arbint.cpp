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
	
	unsigned shrunk = 0;
	while (m_digits.size() > 1 && m_digits[m_digits.size()-1] == 0L)
	{
		//Debug("Shrink 1");
		m_digits.pop_back();
		shrunk++;
	}
	return shrunk;
}

void Arbint::GrowDigit(digit_t new_msd)
{
	static unsigned total_grows = 0;
	static unsigned biggest_arbint = 1;
	m_digits.push_back(new_msd);
	total_grows++;
	if (m_digits.size() > biggest_arbint)
	{
		biggest_arbint = m_digits.size();
		Warn("New biggest Arbint of size %u", m_digits.size());
	}
	//Warn("Arbint grows digit (%.16lx), this->m_digits.size() = %u, total grown = %u", new_msd, m_digits.size(), ++total_grows);
	//if (total_grows++ > 10000)
	//{
	//	Fatal("Too many GrowDigit calls!");
	//}
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
			Debug("Add carry");
			GrowDigit(carry);
		}
	}
	
	m_digits.swap(new_digits);
	m_sign = !(m_sign == mul.m_sign);
	Shrink();
	return *this;
}

void Arbint::Division(const Arbint & div, Arbint & result, Arbint & remainder) const
{
	remainder = 0;
	result = 0;
	if (div.IsZero())
	{
		result = *this;
		return;
	}
	/* may break things (even more that is)
	else if (div.m_digits.size() == 1)
	{
		result.m_digits.resize(m_digits.size(), 0L);
		remainder = Arbint(div_digits((digit_t*)&m_digits[0], div.m_digits[0], m_digits.size(), result.m_digits.data()));
		result.m_sign = !(m_sign == div.m_sign);
		return;
	} */
	
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
	result.Shrink();
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
	Shrink();
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
	// Add any leading zeros to this number
	while (m_digits.size() < add.m_digits.size())
	{
		GrowDigit(0L);
	}
	//m_digits.resize(add.m_digits.size()+1,0L);
	
	digit_t carry = add_digits((digit_t*)m_digits.data(), 
			(digit_t*)add.m_digits.data(), add.m_digits.size());
			
	// This number had more digits but there is a carry left over
	if (carry != 0L && m_digits.size() > add.m_digits.size())
	{
		vector<digit_t> carry_digits(m_digits.size() - add.m_digits.size(), 0L);
		carry_digits[0] = carry;
		carry = add_digits((digit_t*)m_digits.data()+add.m_digits.size(), 
			(digit_t*)carry_digits.data(), m_digits.size()-add.m_digits.size());
	}
	
	// There is still a carry left over
	if (carry != 0L)
	{
		GrowDigit(carry);
	}
	Shrink();
	return *this;
}

Arbint & Arbint::SubBasic(const Arbint & sub)
{
	// Add leading zeros
	while (sub.m_digits.size() > m_digits.size())
	{
		GrowDigit(0L);
	}
	
	// Do subtraction on digits
	digit_t borrow = sub_digits((digit_t*)m_digits.data(), 
			(digit_t*)sub.m_digits.data(), sub.m_digits.size());

	// This number had more digits but there is a borrow left over	
	if (borrow != 0L && m_digits.size() > sub.m_digits.size())
	{
		vector<digit_t> borrow_digits(m_digits.size()-sub.m_digits.size(), 0L);
		borrow_digits[0] = borrow;
		borrow = sub_digits((digit_t*)m_digits.data()+sub.m_digits.size(), 
			(digit_t*)borrow_digits.data(), m_digits.size()-sub.m_digits.size());
	}
	
	// borrow is still set => number is negative
	if (borrow != 0L)
	{
		m_sign = !m_sign;
		for (unsigned i = 0; i < m_digits.size(); ++i)
			m_digits[i] = (~m_digits[i]);
		vector<digit_t> one_digits(m_digits.size(), 0L);
		one_digits[0] = 1L;
		add_digits((digit_t*)m_digits.data(), (digit_t*)one_digits.data(), m_digits.size());
	}
	Shrink();
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
	Shrink();
	return *this;
}

Arbint & Arbint::operator<<=(unsigned amount)
{
	// Shift by whole number of digits
	unsigned whole = amount/(8*sizeof(digit_t));
	unsigned old_size = m_digits.size();
	for (unsigned i = 0; i < whole; ++i)
	{
		//Debug("i = %u, whole = %u", i, whole);
		GrowDigit(0L);//m_digits.resize(m_digits.size() + whole);
	}
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
	Shrink();
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
	while (m_digits.size() < digit+1)
	{
		//Debug("Grow BitSet Size %u, digit %u", m_digits.size(), digit);
		GrowDigit(0L);
	}
		
	i = i % (8*sizeof(digit_t));
	m_digits[digit] |= (1L << i);		
}

}
