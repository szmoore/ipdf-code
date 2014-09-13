#include "paranoidnumber.h"

#include <sstream>
#include <fenv.h>
#include "log.h"
#include <cassert>
#include <iostream>

using namespace std;
namespace IPDF
{
int64_t ParanoidNumber::g_count = 0;


ParanoidNumber::~ParanoidNumber()
{
	g_count--;
	for (int i = 0; i < NOP; ++i)
		delete m_next[i];
}

ParanoidNumber::ParanoidNumber(const char * str) : m_value(0)
{
	Construct();
	int dp = 0;
	int end = 0;
	while (str[dp] != '\0' && str[dp] != '.')
	{
		++dp;
		++end;
	}
	while (str[end] != '\0')
		++end;
	ParanoidNumber m(1);
	for (int i = dp-1; i >= 0; --i)
	{
		ParanoidNumber b(str[i]-'0');
		b*=m;
		this->operator+=(b);
		m*=10;
	}
	ParanoidNumber n(1);
	for (int i = dp+1; i < end; ++i)
	{
		Debug("{%s} /= 10", n.Str().c_str());
		n/=10;
		Debug("{%s}", n.Str().c_str());
		ParanoidNumber b(str[i]-'0');
		b*=n;
		Debug("{%s} += {%s}", Str().c_str(), b.Str().c_str());
		this->operator+=(b);
	}
}

ParanoidNumber & ParanoidNumber::operator=(const ParanoidNumber & a)
{
	m_value = a.m_value;
	for (int i = 0; i < NOP; ++i)
	{
		if (a.m_next[i] == NULL)
		{
			if (m_next[i] != NULL)
				delete m_next[i];
			m_next[i] = NULL;
			continue;
		}
			
		if (m_next[i] != NULL)
		{
			m_next[i]->operator=(*(a.m_next[i]));
		}
		else
		{
			m_next[i] = new ParanoidNumber(*(a.m_next[i]));
		}
	}	
	return *this;
}


string ParanoidNumber::Str() const
{
	string result("");
	stringstream s;
	s << (double)m_value;
	result += s.str();
	if (m_next[MULTIPLY] != NULL)
	{
		result += "*";
		if (m_next[MULTIPLY]->m_next[ADD] != NULL || m_next[MULTIPLY]->m_next[SUBTRACT] != NULL)
			result += "(" + m_next[MULTIPLY]->Str() + ")";
		else
			result += m_next[MULTIPLY]->Str();
	}
	if (m_next[DIVIDE] != NULL)
	{
		result += "/";
		if (m_next[DIVIDE]->m_next[ADD] != NULL || m_next[DIVIDE]->m_next[SUBTRACT] != NULL)
			result += "(" + m_next[DIVIDE]->Str() + ")";
		else
			result += m_next[DIVIDE]->Str();
	}	
	
	if (m_next[ADD] != NULL)
	{
		result += "+";
		if (m_next[ADD]->m_next[MULTIPLY] != NULL || m_next[ADD]->m_next[DIVIDE] != NULL)
			result += "(" + m_next[ADD]->Str() + ")";
		else
			result += m_next[ADD]->Str();
	}
	if (m_next[SUBTRACT] != NULL)
	{
		result += "-";
		if (m_next[SUBTRACT]->m_next[MULTIPLY] != NULL || m_next[SUBTRACT]->m_next[DIVIDE] != NULL)
			result += "(" + m_next[SUBTRACT]->Str() + ")";
		else
			result += m_next[SUBTRACT]->Str();
	}
	

	return result;
}

template <>
bool TrustingOp<float>(float & a, const float & b, Optype op)
{
	feclearexcept(FE_ALL_EXCEPT);
	switch (op)
	{
		case ADD:
			a += b;
			break;
		case SUBTRACT:
			a -= b;
			break;
		case MULTIPLY:
			a *= b;
			break;
		case DIVIDE:
			a /= b;
			break;
		case NOP:
			break;
	}
	return !fetestexcept(FE_ALL_EXCEPT);
}

template <>
bool TrustingOp<double>(double & a, const double & b, Optype op)
{
	feclearexcept(FE_ALL_EXCEPT);
	switch (op)
	{
		case ADD:
			a += b;
			break;
		case SUBTRACT:
			a -= b;
			break;
		case MULTIPLY:
			a *= b;
			break;
		case DIVIDE:
			a /= b;
			break;
		case NOP:
			break;
	}
	return !fetestexcept(FE_ALL_EXCEPT);
}

template <>
bool TrustingOp<int8_t>(int8_t & a, const int8_t & b, Optype op)
{
	int16_t sa(a);
	bool exact = true;
	switch (op)
	{
		case ADD:
			sa += b;
			exact = (abs(sa) <= 127);
			break;
		case SUBTRACT:
			sa -= b;
			exact = (abs(sa) <= 127);
			break;
		case MULTIPLY:
			sa *= b;
			exact = (abs(sa) <= 127);
			break;
		case DIVIDE:
			exact = (b != 0 && sa > b && sa % b == 0);
			sa /= b;
			break;
		case NOP:
			break;
	}
	a = (int8_t)(sa);
	return exact;
}


ParanoidNumber & ParanoidNumber::operator+=(const ParanoidNumber & a)
{
	delete Operation(new ParanoidNumber(a), ADD);
	return *this;
}


ParanoidNumber & ParanoidNumber::operator-=(const ParanoidNumber & a)
{
	delete Operation(new ParanoidNumber(a), SUBTRACT);
	return *this;
}

ParanoidNumber & ParanoidNumber::operator*=(const ParanoidNumber & a)
{
	delete Operation(new ParanoidNumber(a), MULTIPLY);
	return *this;
}


ParanoidNumber & ParanoidNumber::operator/=(const ParanoidNumber & a)
{
	delete Operation(new ParanoidNumber(a), DIVIDE);
	return *this;
}

/**
 * Performs the operation on a with argument b (a += b, a -= b, a *= b, a /= b)
 * @returns b if b can safely be deleted
 * @returns NULL if b has been merged with a
 * append indicates that b should be merged
 */
ParanoidNumber * ParanoidNumber::Operation(ParanoidNumber * b, Optype op, ParanoidNumber ** parent)
{
	if (b == NULL)
		return NULL;
		
	Optype invop = InverseOp(op); // inverse of p
	ParanoidNumber * append_at = this;
	
	if (Floating())
	{
		if ((op == ADD || op == SUBTRACT) && (m_value == 0))
		{
			m_value = b->m_value;
			for (int i = 0; i < NOP; ++i)
			{
				m_next[i] = b->m_next[i];
				b->m_next[i] = NULL;
			}
			return b;
		}
		if ((op == MULTIPLY) && (m_value == 1))
		{
			m_value = b->m_value;
			for (int i = 0; i < NOP; ++i)
			{
				m_next[i] = b->m_next[i];
				b->m_next[i] = NULL;
			}
			return b;
			return b;
		}
		
	}
	
	if (b->Floating())
	{
		if ((op == ADD || op == SUBTRACT) && (b->m_value == 0))
			return b;
		if ((op == MULTIPLY || op == DIVIDE) && (b->m_value == 1))
			return b;
	}
	
	// Operation can be applied directly to the m_value of this and b
	// ie: op is + or - and this and b have no * or / children
	// or: op is * or / and this and b have no + or - children
	if (Pure(op) && (b->Pure(op))) 
	{
		if (ParanoidOp<digit_t>(m_value, b->m_value, op)) // op applied successfully...
		{	
			Simplify(op);
			Simplify(invop);
			for (int i = 0; i < NOP; ++i) // Try applying b's children to this
			{
				delete Operation(b->m_next[i], Optype(i));
				b->m_next[i] = NULL;
			}
			return b; // can delete b
		}
	}
	
	// Try to simplify the cases:
	// a + b*c == (a/c + b)*c
	// a + b/c == (a*c + b)/c
	else if ((op == ADD || op == SUBTRACT) &&
			(Pure(op) || b->Pure(op)))
	{
		
		Debug("Simplify: {%s} %c {%s}", Str().c_str(), OpChar(op), b->Str().c_str());
		Optype adj[] = {MULTIPLY, DIVIDE};
		for (int i = 0; i < 2; ++i)
		{

			Optype f = adj[i];
			Optype invf = InverseOp(f);
			
			Debug("Try %c", OpChar(f));
			
			if (m_next[f] == NULL && b->m_next[f] == NULL)
				continue;

			ParanoidNumber * tmp_a = new ParanoidNumber(*this);
			ParanoidNumber * tmp_b = new ParanoidNumber(*b);
				
		
			ParanoidNumber * af = (tmp_a->m_next[f] != NULL) ? new ParanoidNumber(*(tmp_a->m_next[f])) : NULL;
			ParanoidNumber * bf = (tmp_b->m_next[f] != NULL) ? new ParanoidNumber(*(tmp_b->m_next[f])) : NULL;
			
			Debug("{%s} %c {%s}", tmp_a->Str().c_str(), OpChar(op), tmp_b->Str().c_str());
			Debug("{%s} %c {%s}", tmp_a->Str().c_str(), OpChar(op), tmp_b->Str().c_str());
			if (tmp_a->Operation(af, invf) != af || tmp_b->Operation(bf, invf) != bf)
			{
				delete af;
				delete bf;
				delete tmp_a;
				delete tmp_b;
				continue;
			}
			Debug("{%s} %c {%s}", tmp_a->Str().c_str(), OpChar(op), tmp_b->Str().c_str());
			
			if (tmp_a->Operation(bf, invf) == bf && tmp_b->Operation(af, invf) == af) // a / c simplifies
			{  
				if (tmp_a->Operation(tmp_b, op) != NULL) // (a/c) + b simplifies
				{
					this->operator=(*tmp_a);
					if (bf != NULL)
						delete Operation(bf, f);
					if (af != NULL)
						delete Operation(af, f);
					delete tmp_a;
					delete tmp_b;
					return b; // It simplified after all!
				}
				else
				{
					tmp_b = NULL;
					delete af;
					delete bf;
				} 	
			}
			//Debug("tmp_a : %s", tmp_a->PStr().c_str());
			//Debug("tmp_b : %s", tmp_b->PStr().c_str());
			delete tmp_a;
			delete tmp_b;
		}
	}
	
		// See if operation can be applied to children of this in the same dimension
	{
		// (a / b) / c = a / (b*c)
		// (a * b) / c = a * (b/c)
		// (a / b) * c = a / (b/c)
		// (a * b) * c = a * (b*c)
		// (a + b) + c = a + (b+c)
		// (a - b) + c = a - (b-c)
		// (a + b) - c = a + (b-c)
		// (a - b) - c = a - (b+c)
		Optype fwd(op);
		Optype rev(invop);
		if (op == DIVIDE || op == SUBTRACT)
		{
			fwd = invop;
			rev = op;
		}
		// opposite direction first (because ideally things will cancel each other out...)
		if (m_next[invop] != NULL && m_next[invop]->Operation(b, rev, &append_at) != NULL)
			return b;
		// forward direction
		if (m_next[op] != NULL && m_next[op]->Operation(b, fwd, &append_at) != NULL) 
			return b;
	}
	
	// At this point, we have no choice but to merge 'b' with this ParanoidNumber
	
	// we are a child; the merge operation needs to be applied by the root, so leave
	if (parent != NULL) 
	{
		if (m_next[op] == NULL)
			*parent = this; // last element in list
		return NULL;
	}
	
	append_at->m_next[op] = b; // Merge with b
	
	// MULTIPLY and DIVIDE operations need to be performed on each term in the ADD/SUBTRACT dimension
	if (op == DIVIDE || op == MULTIPLY)
	{
		// apply the operation to each term
		if (m_next[ADD] != NULL) delete m_next[ADD]->Operation(new ParanoidNumber(*b), op);
		if (m_next[SUBTRACT] != NULL) delete m_next[SUBTRACT]->Operation(new ParanoidNumber(*b), op);
		
		// try and simplify this by adding the terms (you never know...)
		Simplify(ADD);
		Simplify(SUBTRACT);
	}
	// failed to simplify
	return NULL;
}

bool ParanoidNumber::Simplify(Optype op)
{
	ParanoidNumber * n = m_next[op];
	m_next[op] = NULL;
	if (Operation(n, Optype(op)))
	{
		delete n;
		return true;
	}
	else
	{
		m_next[op] = n;
		return false;
	}
}

string ParanoidNumber::PStr() const
{
	stringstream s;
	for (int i = 0; i < NOP; ++i)
	{
		Optype f = Optype(i);
		s << this << OpChar(f) << m_next[f] << "\n";
	}
	return s.str();
}





}
