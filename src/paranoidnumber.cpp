#include "paranoidnumber.h"

#include <sstream>
#include <fenv.h>
#include "log.h"

using namespace std;
namespace IPDF
{
	

ParanoidNumber::ParanoidNumber(const char * str) : m_value(0), m_op(ADD), m_next(NULL)
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
		this->operator+=(b);
		m*=10;
	}
	ParanoidNumber n(1);
	for (int i = dp+1; i < end; ++i)
	{
		n/=10;
		ParanoidNumber b(str[i]-'0');
		b*=n;
		this->operator+=(b);
	}
	
}

ParanoidNumber * ParanoidNumber::InsertAfter(float value, Optype op)
{
	return InsertAfter(new ParanoidNumber(value, op));
}
	
ParanoidNumber * ParanoidNumber::InsertAfter(ParanoidNumber * insert)
{
	//Debug("Insert {%s} after {%f, %s}",insert->Str().c_str(),
	//	m_value, g_opstr[m_op]);
	ParanoidNumber * n = m_next;
	m_next = insert;
	
	ParanoidNumber * p = m_next;
	while (p->m_next != NULL)
		p = p->m_next;
	p->m_next = n;
	
	return m_next;
}


ParanoidNumber & ParanoidNumber::operator=(const ParanoidNumber & a)
{
	const ParanoidNumber * na = &a;
	ParanoidNumber * nb = this;
	ParanoidNumber * p = NULL;
	while (na != NULL && nb != NULL)
	{
		nb->m_value = na->m_value;
		nb->m_op = na->m_op;
		na = na->m_next;
		p = nb;
		nb = nb->m_next;
		
	}	
	
	while (na != NULL) // => nb == NULL
	{
		InsertAfter(na->m_value, na->m_op);
		na = na->m_next;
	}

	if (nb != NULL)
	{
		if (p != NULL)
			p->m_next = NULL;
		delete nb;
	}
	return *this;
}

ParanoidNumber & ParanoidNumber::operator+=(const ParanoidNumber & a)
{
	ParanoidNumber * insert = new ParanoidNumber(a, ADD);
	if (m_next == NULL || m_next->m_op == ADD || m_next->m_op == SUBTRACT)
	{
		InsertAfter(insert);
		Simplify();
		return *this;
	}
	
	if (m_next->m_op == MULTIPLY) // (a*b) + c == (a+[c/b]) * b
		insert->operator/=(*m_next);
	else
		insert->operator*=(*m_next); // (a/b) + c == (a+[c*b])/b
	
	if (insert->m_next != NULL) // neither of the above simplified
	{
		//Debug("{%s} did not simplify, change back to {%s}", insert->Str().c_str(), a.Str().c_str());
		insert->operator=(a); // Just add as is
		insert->m_op = ADD;
		ParanoidNumber * n = this;
		while (n->m_next != NULL)
			n = n->m_next;
		n->InsertAfter(insert);
	}
	else
	{
		InsertAfter(insert);
	}
	Simplify();
	return *this;
}
ParanoidNumber & ParanoidNumber::operator-=(const ParanoidNumber & a)
{
	ParanoidNumber * insert = new ParanoidNumber(a, SUBTRACT);
	if (m_next == NULL || m_next->m_op == ADD || m_next->m_op == SUBTRACT)
	{
		InsertAfter(insert);
		Simplify();
		return *this;
	}
	
	if (m_next->m_op == MULTIPLY) // (a*b) - c == (a-[c/b]) * b
		insert->operator/=(*m_next);
	else
		insert->operator*=(*m_next); // (a/b) - c == (a-[c*b])/b
	
	if (insert->m_next != NULL) // neither of the above simplified
	{
		//Debug("{%s} did not simplify, change back to {%s}", insert->Str().c_str(), a.Str().c_str());
		insert->operator=(a); // Just add as is
		insert->m_op = SUBTRACT;
		ParanoidNumber * n = this;
		while (n->m_next != NULL)
			n = n->m_next;
		n->InsertAfter(insert);
	}	
	else
	{
		InsertAfter(insert);
	}
	Simplify();
	return *this;
}

ParanoidNumber & ParanoidNumber::operator*=(const ParanoidNumber & a)
{
	ParanoidNumber * n = m_next;
	while (n != NULL)
	{
		if (n->m_op == ADD || n->m_op == SUBTRACT)
		{
			n->operator*=(a);
			break;
		}
		n = n->m_next;
	}
		
	InsertAfter(new ParanoidNumber(a, MULTIPLY));
	Simplify();
	return *this;
}


ParanoidNumber & ParanoidNumber::operator/=(const ParanoidNumber & a)
{
	ParanoidNumber * n = m_next;
	while (n != NULL)
	{
		if (n->m_op == ADD || n->m_op == SUBTRACT)
		{
			n->operator/=(a);
			break;
		}
		n = n->m_next;
	}
		
	InsertAfter(new ParanoidNumber(a, DIVIDE));
	Simplify();
	return *this;
}

double ParanoidNumber::ToDouble() const
{
	double value = (m_op == SUBTRACT) ? -m_value : m_value;
	const ParanoidNumber * n = m_next;
	while (n != NULL)
	{
		switch (n->m_op)
		{
			case ADD:
			case SUBTRACT:
				return value + n->ToDouble();
				break;
			case MULTIPLY:
				value *= n->m_value;
				break;
			case DIVIDE:
				value /= n->m_value;
				break;
		}
		n = n->m_next;
	}
	return value;
}

void ParanoidNumber::Simplify()
{
	ParanoidNumber * n = m_next;
	ParanoidNumber * p = this;
	
	while (n != NULL)
	{
		
		float a = p->m_value;
		switch (n->m_op)
		{
			case ADD:
				n->Simplify();
				feclearexcept(FE_ALL_EXCEPT);
				a += n->m_value;
				break;
			case SUBTRACT:
				n->Simplify();
				feclearexcept(FE_ALL_EXCEPT);
				a -= n->m_value;
				break;
			case MULTIPLY:
				feclearexcept(FE_ALL_EXCEPT);
				a *= n->m_value;
				break;
			case DIVIDE:
				feclearexcept(FE_ALL_EXCEPT);
				a /= n->m_value;
				break;
				
		}
		// can't merge p and n
		if (fetestexcept(FE_ALL_EXCEPT))
		{
			while (n != NULL && n->m_op != ADD && n->m_op != SUBTRACT)
				n = n->m_next;
			if (n != NULL)
				n->Simplify();
			return;
		}
		else
		{
			//  merge n into p
			p->m_value = a;
			p->m_next = n->m_next;
			n->m_next = NULL;
			delete n;
			n = p->m_next;		
		}
		//Debug("  -> {%s}", Str().c_str());
	}
}

string ParanoidNumber::Str() const
{
	string result("");
	switch (m_op)
	{
		case ADD:
			result += " +";
			break;
		case SUBTRACT:
			result += " -";
			break;
		case MULTIPLY:
			result += " *";
			break;
		case DIVIDE:
			result += " /";
			break;
	}
	stringstream s("");
	s << m_value;
	result += s.str();
	if (m_next != NULL)
		result += m_next->Str();
	return result;
}

}
