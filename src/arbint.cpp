#include "arbint.h"
#include <algorithm>
#include <cstring>
using namespace std;

/*
static bool addc(uint32_t a, uint32_t b, uint32_t * r)
{
	volatile uint32_t carry = false;
	volatile uint32_t result = a + b;
	asm volatile
	(
		"jc 1f;"
		"mov $0, %%eax;"
		"1:"
		: "=a" (carry)
	);
	*r = result;	
	return (carry == 1);
}
*/

namespace IPDF
{

Arbint::Arbint(int64_t i) : m_words(2), m_sign(false)
{
	m_sign = i < 0;
	memcpy(m_words.data(), &i, sizeof(int64_t));
}

string Arbint::Str() const
{
	string s("");
	for (unsigned i = 0; i < m_words.size(); ++i)
	{
		uint32_t w = m_words[i];
		do
		{
			uint32_t q = w % 10;
			w /= 10;
			s += ('0' + q);
		}
		while (w > 0);
		if (i+1 < m_words.size()) s += ",";	
	}
	if (m_sign) s += '-';
	reverse(s.begin(), s.end());
	return s;
}

}
