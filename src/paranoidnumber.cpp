#include "paranoidnumber.h"

#include <sstream>
#include <fenv.h>
#include "log.h"
#include <iostream>

using namespace std;
namespace IPDF
{
	

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
	Debug("Constructed {%s} from %s (%f)", Str().c_str(), str, ToDouble());	
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
	// this = v + t + (a)
	// -> v + (a) + t
	
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
	Debug("{%s} *= {%s}", Str().c_str(), a.Str().c_str());
	// this = (vf + t) * (a)
	ParanoidNumber * nf = m_next_factor;
	m_next_factor = new ParanoidNumber(a, MULTIPLY);
	ParanoidNumber * t = m_next_factor;
	while (t->m_next_factor != NULL)
		t = t->m_next_factor;
	t->m_next_factor = nf;
	if (m_next_term != NULL)
		m_next_term->operator*=(a);
	//Debug("Simplify after mul");
	Debug("Simplify {%s}", Str().c_str());
	Simplify();
	return *this;
}


ParanoidNumber & ParanoidNumber::operator/=(const ParanoidNumber & a)
{
	//Debug("Called %s /= %s", Str().c_str(), a.Str().c_str());
	// this = (vf + t) * (a)
	ParanoidNumber * nf = m_next_factor;
	m_next_factor = new ParanoidNumber(a, DIVIDE);
	ParanoidNumber * t = m_next_factor;
	while (t->m_next_factor != NULL)
		t = t->m_next_factor;
	t->m_next_factor = nf;
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
		ParanoidNumber * bprev = a;
		ParanoidNumber * b = a->m_next_term;
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
			float f = a->m_value;
			feclearexcept(FE_ALL_EXCEPT);
			switch (b->m_op)
			{
				case ADD:
					f += b->m_value;
					break;
				case SUBTRACT:
					f -= b->m_value;
					break;
				default:
					Fatal("Unexpected %c in term list...", OpChar(b->m_op));
					break;
			}
			if (!fetestexcept(FE_ALL_EXCEPT))
			{
				a->m_value = f;
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
			float f = a->m_value;
			feclearexcept(FE_ALL_EXCEPT);
			switch (b->m_op)
			{
				case MULTIPLY:
					if (a->m_op != DIVIDE) 
						f *= b->m_value;
					else
						f /= b->m_value;
					break;
				case DIVIDE:
					if (a->m_op != DIVIDE)
						f /= b->m_value;
					else
						f *= b->m_value;
					break;
				default:
					Fatal("Unexpected %c in factor list...",OpChar(b->m_op));
					break;
			}
			if (!fetestexcept(FE_ALL_EXCEPT))
			{
				
				a->m_value = f;
				bprev->m_next_factor = b->m_next_factor;
				b->m_next_factor = NULL;
				delete b;
				b = bprev;
			}
			//else
				//Debug("Failed to simplify %f %c %f", a->m_value, OpChar(b->m_op), b->m_value);
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
	
	s << m_value;
	
	if (m_next_factor != NULL)
	{
		result += OpChar(m_op);
		result += "(";
		result += s.str();
		result += m_next_factor->Str();
		result += ")";
	}
	else
	{
		result += OpChar(m_op);
		result += s.str();
	}
		
	if (m_next_term != NULL)
	{
		result += " ";
		result += m_next_term->Str();
	}
	return result;
}

}
