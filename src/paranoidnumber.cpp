#include "paranoidnumber.h"

#include <sstream>
#include <fenv.h>
#include "log.h"
#include <iostream>

using namespace std;
namespace IPDF
{
int64_t ParanoidNumber::g_count = 0;

ParanoidNumber::ParanoidNumber(const char * str) : m_value(0), m_op(ADD), m_next_term(NULL), m_next_factor(NULL)
{
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
		//Debug("m is %s", m.Str().c_str());
		//Debug("Add %s", b.Str().c_str());
		this->operator+=(b);
		//Debug("Now at %s", Str().c_str());
		m*=10;
	}
	ParanoidNumber n(1);
	for (int i = dp+1; i < end; ++i)
	{
		n/=10;
		ParanoidNumber b(str[i]-'0');
		//Debug("%s * %s", b.Str().c_str(), n.Str().c_str());
		b*=n;
		//Debug("b -> %s", b.Str().c_str());
		//Debug("Add %s", b.Str().c_str());
		this->operator+=(b);
		//Debug("Now at %s", Str().c_str());

	}
	//Debug("Constructed {%s} from %s (%f)", Str().c_str(), str, ToDouble());	
}

ParanoidNumber & ParanoidNumber::operator=(const ParanoidNumber & a)
{
	//TODO: Optimise
	delete m_next_term;
	delete m_next_factor;
	m_op = a.m_op;
	if (a.m_next_term != NULL)
	{
		m_next_term = new ParanoidNumber(*(a.m_next_term));
	}
	if (a.m_next_factor != NULL)
	{
		m_next_factor = new ParanoidNumber(*(a.m_next_factor));
	}
	return *this;
}

ParanoidNumber & ParanoidNumber::operator+=(const ParanoidNumber & a)
{
	
	if (m_next_factor == NULL && a.Floating())
	{
		if (ParanoidOp<digit_t>(m_value, a.m_value, ADD))
		{
			Simplify();
			return *this;
		}
	}
	ParanoidNumber * nt = m_next_term;
	ParanoidNumber * nf = m_next_factor;
	
	ParanoidNumber ca(a);
	if (m_next_factor != NULL)
	{
		if (m_next_factor->m_op == MULTIPLY)
			ca /= (*m_next_factor);
		else
			ca *= (*m_next_factor);
			
		if (ca.Floating())
		{
			m_next_factor = NULL;
			m_next_term = NULL;
			operator+=(ca);
			m_next_factor = nf;
			m_next_term = nt;
			Simplify();
			return *this;
		}
		
	}
	
	m_next_term = new ParanoidNumber(a, ADD);
	ParanoidNumber * t = m_next_term;
	while (t->m_next_term != NULL)
		t = t->m_next_term;
	t->m_next_term = nt;
	//Debug("Simplify {%s} after add", Str().c_str());
	Simplify();
	return *this;
}

ParanoidNumber & ParanoidNumber::operator-=(const ParanoidNumber & a)
{
	// this = v + t + (a)
	// -> v + (a) + t
	if (m_next_factor == NULL && a.Floating())
	{
		if (ParanoidOp<digit_t>(m_value, a.m_value, ADD))
		{
			Simplify();
			return *this;
		}
	}

	ParanoidNumber * nt = m_next_term;
	ParanoidNumber * nf = m_next_factor;
	
	ParanoidNumber ca(a, SUBTRACT);
	if (m_next_factor != NULL)
	{
		if (m_next_factor->m_op == MULTIPLY)
			ca /= (*m_next_factor);
		else
			ca *= (*m_next_factor);
			
		if (ca.Floating())
		{
			m_next_factor = NULL;
			m_next_term = NULL;
			operator-=(ca);
			m_next_factor = nf;
			m_next_term = nt;
			Simplify();
			return *this;
		}
		
	}
	
	m_next_term = new ParanoidNumber(a,SUBTRACT);
	ParanoidNumber * t = m_next_term;
	while (t->m_next_term != NULL)
	{
		t->m_op = SUBTRACT;
		t = t->m_next_term;
	}
	t->m_op = SUBTRACT;
	//Debug("next term {%s}", m_next_term->Str().c_str());
	t->m_next_term = nt;
	//Debug("Simplify {%s} after sub", Str().c_str());
	Simplify();
	return *this;
}

ParanoidNumber & ParanoidNumber::operator*=(const ParanoidNumber & a)
{

	//if (m_value == 0)
	//		return *this;
	//Debug("{%s} *= {%s}", Str().c_str(), a.Str().c_str());
	// this = (vf + t) * (a)
	if (a.Floating() && ParanoidOp<digit_t>(m_value, a.m_value, MULTIPLY))
	{
		if (m_next_term != NULL)
			m_next_term->operator*=(a);
		Simplify();
		return *this;
	}
	
	ParanoidNumber * t = this;
	while (t->m_next_factor != NULL)
		t = t->m_next_factor;
	t->m_next_factor = new ParanoidNumber(a, MULTIPLY);

	if (m_next_term != NULL)
		m_next_term->operator*=(a);

	//Debug("Simplify {%s}", Str().c_str());
	Simplify();
	//Debug("Simplified to {%s}", Str().c_str());
	return *this;
}


ParanoidNumber & ParanoidNumber::operator/=(const ParanoidNumber & a)
{
		

		
	if (a.Floating() && ParanoidOp<digit_t>(m_value, a.m_value, DIVIDE))
	{
		if (m_next_term != NULL)
			m_next_term->operator/=(a);
		Simplify();
		return *this;
	}
	
	//Debug("Called %s /= %s", Str().c_str(), a.Str().c_str());
	// this = (vf + t) * (a)
	ParanoidNumber * t = this;
	while (t->m_next_factor != NULL)
	{
		t = t->m_next_factor;
	}
	t->m_next_factor = new ParanoidNumber(a, DIVIDE);

	if (m_next_term != NULL)
		m_next_term->operator/=(a);

	Simplify();
	return *this;
}



void ParanoidNumber::SimplifyTerms()
{ 

	//Debug("Simplify {%s}", Str().c_str()); 
	if (m_next_term == NULL)
	{
		//Debug("No terms!");
		return;
	}

	for (ParanoidNumber * a = this; a != NULL; a = a->m_next_term)
	{
		ParanoidNumber * b = a->m_next_term;
		if (a->m_next_factor != NULL)
		{
			continue;
		}
		
		ParanoidNumber * bprev = a;
		while (b != NULL)
		{
			//Debug("Simplify factors of %s", b->Str().c_str());
			b->SimplifyFactors();
			if (b->m_next_factor != NULL)
			{
				bprev = b;
				b = b->m_next_term;
				continue;
			}
			bool simplify = false;
			simplify = ParanoidOp<digit_t>(a->m_value, b->Head<digit_t>(), ADD);
			if (simplify)
			{
				bprev->m_next_term = b->m_next_term;
				b->m_next_term = NULL;
				delete b;
				b = bprev;
			}
			
			bprev = b;
			b = b->m_next_term;
		}
	}
}

void ParanoidNumber::SimplifyFactors()
{ 

	//Debug("Simplify {%s}", Str().c_str()); 
	if (m_next_factor == NULL)
	{
		//Debug("No factors!");
		return;
	}

	for (ParanoidNumber * a = this; a != NULL; a = a->m_next_factor)
	{
		if ((a->m_op != ADD || a->m_op != SUBTRACT) && a->m_next_term != NULL)
			continue;
			
		ParanoidNumber * bprev = a;
		ParanoidNumber * b = a->m_next_factor;
		while (b != NULL)
		{
			b->SimplifyTerms();
			if (b->m_next_term != NULL)
			{
				bprev = b;
				b = b->m_next_factor;
				continue;
			}
		
			Optype op = b->m_op;
			if (a->m_op == DIVIDE)
			{
				op = (b->m_op == DIVIDE) ? MULTIPLY : DIVIDE;
			}
			
			if (ParanoidOp<digit_t>(a->m_value, b->m_value, op))
			{	

				bprev->m_next_factor = b->m_next_factor;
				b->m_next_factor = NULL;
				delete b;
				b = bprev;
			}
			bprev = b;
			b = b->m_next_factor;
		}
	}
}

void ParanoidNumber::Simplify()
{
	SimplifyFactors();
	SimplifyTerms();
}

string ParanoidNumber::Str() const
{
	string result("");
	stringstream s;
	s << (double)m_value;
	
	if (m_next_factor != NULL)
	{
		result += s.str();
		result += OpChar(m_next_factor->m_op);
		if (m_next_factor->m_next_term != NULL)
			result += "(" + m_next_factor->Str() + ")";
		else
			result += m_next_factor->Str();
	}
	else
	{
		result += s.str();
	}
		
	if (m_next_term != NULL)
	{
		result += " ";
		result += OpChar(m_next_term->m_op);
		result += m_next_term->Str();
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
	}
	a = (int8_t)(sa);
	return exact;
}

}
