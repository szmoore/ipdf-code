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

ParanoidNumber::ParanoidNumber(const string & str) : m_value(0), m_cached_result(0), m_cache_valid(false), m_next()
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
	assert(this != NULL);
	m_value = a.m_value;
	m_cached_result = a.m_cached_result;
	for (int i = 0; i < NOP; ++i)
	{
		for (auto n : m_next[i])
			delete n;
		m_next[i].clear();
		for (auto n : a.m_next[i])
			m_next[i].push_back(new ParanoidNumber(*n));
	}
		/*
		for (unsigned j = 0; j < m_next[i].size() && j < a.m_next[i].size(); ++j)
		{
			if (a.m_next[i][j] != NULL)
				m_next[i][j]->operator=(*(a.m_next[i][j]));
		}
		
		for (unsigned j = a.m_next[i].size(); j < m_next[i].size(); ++j)
		{
			delete m_next[i][j];
		}
		m_next[i].resize(a.m_next[i].size());
		*/
	//}	
	return *this;
}


string ParanoidNumber::Str() const
{
	
	assert(this != NULL);
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
			if (b == 0)
			{
				a = (a >= 0) ? INFINITY : -INFINITY;
				return false;
			}
			
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
			if (b == 0)
			{
				a = (a >= 0) ? INFINITY : -INFINITY;
				return false;
			}
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
	
	assert(this != NULL);
	delete Operation(SafeConstruct(a), ADD);
	Simplify(ADD);
	Simplify(SUBTRACT);
	return *this;
}


ParanoidNumber & ParanoidNumber::operator-=(const ParanoidNumber & a)
{
	delete Operation(SafeConstruct(a), SUBTRACT);
	Simplify(SUBTRACT);
	Simplify(ADD);
	return *this;
}

ParanoidNumber & ParanoidNumber::operator*=(const ParanoidNumber & a)
{
	delete Operation(SafeConstruct(a), MULTIPLY);
	Simplify(MULTIPLY);
	Simplify(DIVIDE);	
	return *this;
}


ParanoidNumber & ParanoidNumber::operator/=(const ParanoidNumber & a)
{
	delete Operation(SafeConstruct(a), DIVIDE);
	Simplify(MULTIPLY);
	Simplify(DIVIDE);
	return *this;
}

// a + b
ParanoidNumber * ParanoidNumber::OperationTerm(ParanoidNumber * b, Optype op, ParanoidNumber ** merge_point, Optype * merge_op)
{
	if (!SanityCheck())
	{
		Fatal("What...");
	}
	assert(b->SanityCheck());
	
	m_cached_result = nan("");
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
		
		assert(SanityCheck());
		return b;
	}
	if (b->Floating() && b->m_value == 0) // a + 0 = a
		return b;
		

	
	if ((NoFactors() && b->NoFactors())
		|| (GetFactors() == b->GetFactors()))
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
			assert(SanityCheck());
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
		{
			assert(SanityCheck());
			return b;
		}
		
	}
	for (auto next : m_next[op])
	{
		if (next->OperationTerm(b, fwd, merge_point, merge_op) == b)
		{
			assert(SanityCheck());
			return b;
		}
	}
	

	
	
	if (parent)
	{
		//merge->m_next[*merge_op].push_back(b);
		m_next[op].push_back(b);
	}
	else
	{
		if (m_next[op].size() == 0)
		{
			*merge_point = this;
			*merge_op = op;
		}
	}

	assert(SanityCheck());
	return NULL;
}

ParanoidNumber * ParanoidNumber::OperationFactor(ParanoidNumber * b, Optype op, ParanoidNumber ** merge_point, Optype * merge_op)
{
	assert(SanityCheck());
	assert(b->SanityCheck());
	m_cached_result = nan("");
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
		assert(SanityCheck());
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
			
			
			assert(SanityCheck());
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
		cpy_b = SafeConstruct(*b);
	}
	
	for (auto prev : m_next[invop])
	{
		if (prev->OperationFactor(b, rev, merge_point, merge_op) == b)
		{
			for (auto add : m_next[ADD])
				delete add->OperationFactor(SafeConstruct(*cpy_b), op);
			for (auto sub : m_next[SUBTRACT])
				delete sub->OperationFactor(SafeConstruct(*cpy_b), op);
				
			delete cpy_b;
			assert(SanityCheck());
			return b;
		}
	}
	for (auto next : m_next[op])
	{
		if (next->OperationFactor(b, fwd, merge_point, merge_op) == b)
		{
			for (auto add : m_next[ADD])
			{
				delete add->OperationFactor(SafeConstruct(*cpy_b), op);
			}
			for (auto sub : m_next[SUBTRACT])
			{
				delete sub->OperationFactor(SafeConstruct(*cpy_b), op);
			}
			delete cpy_b;
			assert(SanityCheck());
			return b;
		}
	}
	
	if (parent)
	{
		assert(b != NULL);
		m_next[op].push_back(b);
		for (auto add : m_next[ADD])
			delete add->OperationFactor(SafeConstruct(*cpy_b), op);
		for (auto sub : m_next[SUBTRACT])
			delete sub->OperationFactor(SafeConstruct(*cpy_b), op);
	}
	assert(SanityCheck());
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
	if (Floating())
		return false;
		
	assert(SanityCheck());
	vector<ParanoidNumber*> next;
	next.clear();
	swap(m_next[op], next);
	m_next[op].clear();
	assert(m_next[op].size() == 0);
	assert(SanityCheck());
	Optype fwd = op;
	if (op == DIVIDE)
		fwd = MULTIPLY;
	else if (op == SUBTRACT)
		fwd = ADD;
		
	
	for (vector<ParanoidNumber*>::iterator n = next.begin(); n != next.end(); ++n)
	{
		if (*n == NULL)
			continue;
		for (vector<ParanoidNumber*>::iterator m = n; m != next.end(); ++m)
		{
			if ((*m) == (*n))
				continue;
			if (*m == NULL)
				continue;
				
			ParanoidNumber * parent = this;
			Optype mop = op;
			ParanoidNumber * result = (*n)->Operation(*m, fwd, &parent, &mop);
			if (result != NULL)
			{
				*m = NULL;
				delete result;
			}
		}
		if (*n != NULL)
			delete Operation(*n, op);
	}
	set<ParanoidNumber*> s;
	if (!SanityCheck(s))
	{
		Error("Simplify broke Sanity");
	}
	return (next.size() > m_next[op].size());
}

bool ParanoidNumber::FullSimplify()
{
	bool result = false;
	result |= Simplify(MULTIPLY);
	result |= Simplify(DIVIDE);
	result |= Simplify(ADD);
	result |= Simplify(SUBTRACT);
	return result;
}

ParanoidNumber::digit_t ParanoidNumber::Digit() const
{
	if (!SanityCheck())
	{
		Fatal("Blargh");
	}
	//if (!isnan(m_cached_result))
	//	return m_cached_result;
		
	// Get around the absurd requirement that const correctness be observed.
	digit_t result;// = ((ParanoidNumber*)(this))->m_cached_result;
	result = m_value;
	for (auto mul : m_next[MULTIPLY])
	{
		result *= mul->Digit();
	}
	for (auto div : m_next[DIVIDE])
	{
		result /= div->Digit();
	}
	for (auto add : m_next[ADD])
		result += add->Digit();
	for (auto sub : m_next[SUBTRACT])
		result -= sub->Digit();
	return result;
		
}

ParanoidNumber::digit_t ParanoidNumber::GetFactors() const
{
	digit_t value = 1;
	for (auto mul : m_next[MULTIPLY])
		value *= mul->Digit();
	for (auto div : m_next[DIVIDE])
		value /= div->Digit();
	return value;
}


ParanoidNumber::digit_t ParanoidNumber::GetTerms() const
{
	digit_t value = 0;
	for (auto add : m_next[ADD])
		value += add->Digit();
	for (auto sub : m_next[SUBTRACT])
		value -= sub->Digit();
	return value;
}

bool ParanoidNumber::SanityCheck(set<ParanoidNumber*> & visited) const
{
	if (this == NULL)
	{
		Error("NULL pointer in tree");
		return false;
	}
		
	if (visited.find((ParanoidNumber*)this) != visited.end())
	{
		Error("I think I've seen this tree before...");
		return false;
	}
	
	visited.insert((ParanoidNumber*)this);
		
	for (auto add : m_next[ADD])
	{
		if (!add->SanityCheck(visited))
			return false;
	}
	for (auto sub : m_next[SUBTRACT])
	{
		if (!sub->SanityCheck(visited))
			return false;
	}
	for (auto mul : m_next[MULTIPLY])
	{
		if (!mul->SanityCheck(visited))
			return false;
	}
	
	for (auto div : m_next[DIVIDE])
	{
		if (!div->SanityCheck(visited))
			return false;
	}
	return true;
}

}
