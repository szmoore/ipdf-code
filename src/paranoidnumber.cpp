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
	{
		for (auto n : m_next[i])
			delete n;
	}
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
		n/=10;
		ParanoidNumber b(str[i]-'0');
		b*=n;
		this->operator+=(b);
	}
}

ParanoidNumber & ParanoidNumber::operator=(const ParanoidNumber & a)
{
	m_value = a.m_value;
	for (int i = 0; i < NOP; ++i)
	{
		for (unsigned j = 0; j < m_next[i].size() && j < a.m_next[i].size(); ++j)
		{
			m_next[i][j]->operator=(*(a.m_next[i][j]));
		}
		
		for (unsigned j = a.m_next[i].size(); j < m_next[i].size(); ++j)
		{
			delete m_next[i][j];
		}
		m_next[i].resize(a.m_next[i].size());
	}	
	return *this;
}


string ParanoidNumber::Str() const
{
	string result("");
	stringstream s;
	s << (double)m_value;
	result += s.str();
	for (auto mul : m_next[MULTIPLY])
	{
		result += "*";
		if (!mul->Floating())
			result += "(" + mul->Str() + ")";
		else
			result += mul->Str();
	}
	for (auto div : m_next[DIVIDE])
	{
		result += "/";
		if (!div->Floating())
			result += "(" + div->Str() + ")";
		else
			result += div->Str();
	}	
	
	for (auto add : m_next[ADD])
	{
		result += "+";
		if (!add->Floating())
			result += "(" + add->Str() + ")";
		else
			result += add->Str();
	}
	for (auto sub : m_next[SUBTRACT])
	{
		result += "-";
		if (!sub->Floating())
			result += "(" + sub->Str() + ")";
		else
			result += sub->Str();
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

// a + b
ParanoidNumber * ParanoidNumber::OperationTerm(ParanoidNumber * b, Optype op, ParanoidNumber ** merge_point, Optype * merge_op)
{
			
	if (Floating() && m_value == 0) // 0 + b = b
	{
		m_value = b->m_value;
		if (op == SUBTRACT)
		{
			m_value = -m_value;
			swap(b->m_next[ADD], b->m_next[SUBTRACT]);
		}
		
		for (int i = 0; i < NOP; ++i)
		{
			m_next[i] = b->m_next[i];
			b->m_next[i].clear();
		}
		return b;
	}
	if (b->Floating() && b->m_value == 0) // a + 0 = a
		return b;
		

	
	if (NoFactors() && b->NoFactors())
	{
		if (ParanoidOp<digit_t>(m_value, b->m_value, op))
		{
			Optype addop = (op == ADD) ? ADD : SUBTRACT;
			for (auto add : b->m_next[ADD])
			{
				delete OperationTerm(add, addop);
			}
			Optype subop = (op == ADD) ? SUBTRACT : ADD;
			for (auto sub : b->m_next[SUBTRACT])
				delete OperationTerm(sub, subop);
				
			b->m_next[ADD].clear();
			b->m_next[SUBTRACT].clear();
			return b;
		}
	}


	
	
	bool parent = (merge_point == NULL);
	ParanoidNumber * merge = this;
	Optype mop = op;
	assert(mop != NOP); // silence compiler warning
	if (parent)
	{
		merge_point = &merge;
		merge_op = &mop;
	}
	else
	{
		merge = *merge_point;
		mop = *merge_op;
	}
		
	Optype invop = InverseOp(op); // inverse of p
	Optype fwd = op;
	Optype rev = invop;
	if (op == SUBTRACT)
	{
		fwd = ADD;
		rev = SUBTRACT;
	}
	
	for (auto prev : m_next[invop])
	{
		if (prev->OperationTerm(b, rev, merge_point, merge_op) == b)
			return b;
		
	}
	for (auto next : m_next[op])
	{
		if (next->OperationTerm(b, fwd, merge_point, merge_op) == b)
			return b;
	}
	

	
	
	if (parent)
	{
		merge->m_next[*merge_op].push_back(b);
	}
	else
	{
		if (m_next[op].size() == 0)
		{
			*merge_point = this;
			*merge_op = op;
		}
	}
	return NULL;
}

ParanoidNumber * ParanoidNumber::OperationFactor(ParanoidNumber * b, Optype op, ParanoidNumber ** merge_point, Optype * merge_op)
{
	
	if (Floating() && m_value == 0)
	{
		return b;
	}
	
	if (Floating() && m_value == 1 && op == MULTIPLY)
	{
		m_value = b->m_value;
		for (int i = 0; i < NOP; ++i)
		{
			for (auto n : m_next[i])
				delete n;
			m_next[i].clear();
			swap(m_next[i], b->m_next[i]);
		}
		return b;
	}
	if (b->Floating() && b->m_value == 1)
		return b;
		
	if (NoTerms() && b->NoTerms())
	{
		if (ParanoidOp<digit_t>(m_value, b->m_value, op))
		{
			Optype mulop = (op == MULTIPLY) ? MULTIPLY : DIVIDE;
			for (auto mul : b->m_next[MULTIPLY])
			{
				delete OperationFactor(mul, mulop);
			}
			Optype divop = (op == MULTIPLY) ? DIVIDE : MULTIPLY;
			for (auto div : b->m_next[DIVIDE])
				delete OperationFactor(div, divop);
				
			b->m_next[DIVIDE].clear();
			b->m_next[MULTIPLY].clear();
			return b;		
		}
	}
	
		
	bool parent = (merge_point == NULL);
	ParanoidNumber * merge = this;
	Optype mop = op;
	if (parent)
	{
		merge_point = &merge;
		merge_op = &mop;	
	}
	else
	{
		merge = *merge_point;
		mop = *merge_op;
	}
		
	Optype invop = InverseOp(op); // inverse of p
	Optype fwd = op;
	Optype rev = invop;
	if (op == DIVIDE)
	{
		fwd = MULTIPLY;
		rev = DIVIDE;
	}

	ParanoidNumber * cpy_b = NULL;
	
	if (m_next[ADD].size() > 0 || m_next[SUBTRACT].size() > 0)
	{
		cpy_b = new ParanoidNumber(*b);
	}
	
	for (auto prev : m_next[invop])
	{
		if (prev->OperationFactor(b, rev, merge_point, merge_op) == b)
		{
			for (auto add : m_next[ADD])
				delete add->OperationFactor(new ParanoidNumber(*cpy_b), op);
			for (auto sub : m_next[SUBTRACT])
				delete sub->OperationFactor(new ParanoidNumber(*cpy_b), op);
				
			delete cpy_b;
			return b;
		}
	}
	for (auto next : m_next[op])
	{
		if (next->OperationFactor(b, fwd, merge_point, merge_op) == b)
		{
			for (auto add : m_next[ADD])
				delete add->OperationFactor(new ParanoidNumber(*cpy_b), op);
			for (auto sub : m_next[SUBTRACT])
				delete sub->OperationFactor(new ParanoidNumber(*cpy_b), op);
			delete cpy_b;
			return b;
		}
	}
	
	if (parent)
	{
		m_next[op].push_back(b);
		for (auto add : m_next[ADD])
			delete add->OperationFactor(new ParanoidNumber(*cpy_b), op);
		for (auto sub : m_next[SUBTRACT])
			delete sub->OperationFactor(new ParanoidNumber(*cpy_b), op);
	}
	return NULL;	
}



/**
 * Performs the operation on a with argument b (a += b, a -= b, a *= b, a /= b)
 * @returns b if b can safely be deleted
 * @returns NULL if b has been merged with a
 * append indicates that b should be merged
 */
ParanoidNumber * ParanoidNumber::Operation(ParanoidNumber * b, Optype op, ParanoidNumber ** merge_point, Optype * merge_op)
{

	if (b == NULL)
		return NULL;

	
	if (op == SUBTRACT || op == ADD)
		return OperationTerm(b, op, merge_point, merge_op);
	if (op == MULTIPLY || op == DIVIDE)
		return OperationFactor(b, op, merge_point, merge_op);
	return b;
}



string ParanoidNumber::PStr() const
{
	stringstream s;
	for (int i = 0; i < NOP; ++i)
	{
		Optype f = Optype(i);
		s << this;
		for (auto n : m_next[f])
		{
			s << OpChar(f) << n->PStr();
		}
	}
	return s.str();
}

bool ParanoidNumber::Simplify(Optype op)
{
	vector<ParanoidNumber*> next(0);
	swap(m_next[op], next);
	for (auto n : next)
	{
		ParanoidNumber * result = Operation(n, op);
		if (result != NULL)
			delete result;
		else
			m_next[op].push_back(n);
	}
	return (next.size() > m_next[op].size());
}




}
