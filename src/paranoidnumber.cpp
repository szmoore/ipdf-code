#include "paranoidnumber.h"

#include <sstream>
#include <fenv.h>
#include "log.h"
#include <cassert>
#include <iostream>

// here be many copy paste bugs

using namespace std;
namespace IPDF
{


#ifdef PARANOID_USE_ARENA
ParanoidNumber::Arena ParanoidNumber::g_arena;
#endif //PARANOID_USE_ARENA

ParanoidNumber::~ParanoidNumber()
{
	for (int i = 0; i < NOP; ++i)
	{
		for (auto n : m_next[i])
			delete n;
	}
}

ParanoidNumber::ParanoidNumber(const string & str) : m_value(0), m_next()
{
	#ifdef PARANOID_SIZE_LIMIT
		m_size = 0;
	#endif
	#ifdef PARANOID_CACHE_RESULTS
	m_cached_result = NAN;
	#endif
	
	int dp = 0;
	int end = 0;
	bool negate = str[0] == '-';
	if (negate)
	{
		dp++;
		end++;
	}
	while (str[dp] != '\0' && str[dp] != '.')
	{
		++dp;
		++end;
	}
	while (str[end] != '\0')
		++end;
	ParanoidNumber m(1);
	for (int i = dp-1; i >= negate; --i)
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
	
	if (negate)
		Negate();
	
	#ifdef PARANOID_COMPARE_EPSILON
		double d = strtod(str.c_str(), NULL);
		CompareForSanity(d, d);
	#endif
}

ParanoidNumber & ParanoidNumber::operator=(const ParanoidNumber & a)
{
	//assert(this != NULL);
	
	#ifdef PARANOID_SIZE_LIMIT
		m_size = a.m_size;
	#endif
	
	m_value = a.m_value;
	#ifdef PARANOID_CACHE_RESULT
	m_cached_result = a.m_cached_result;
	#endif
	for (int i = 0; i < NOP; ++i)
	{
		for (auto n : m_next[i])
			delete n;
		m_next[i].clear();
		for (auto n : a.m_next[i])
			m_next[i].push_back(new ParanoidNumber(*n));
	}
	#ifdef PARANOID_COMPARE_EPSILON
		CompareForSanity(a.Digit(),a.Digit());
	#endif
	return *this;
}


string ParanoidNumber::Str() const
{
	
	//assert(this != NULL);
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
bool TrustingOp<long double>(long double & a, const long double & b, Optype op)
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


ParanoidNumber & ParanoidNumber::operator+=(const digit_t & a)
{
	#ifdef PARANOID_COMPARE_EPSILON
		digit_t compare = Digit();
		compare += a;
	#endif
	delete Operation(new ParanoidNumber(a), ADD);
	Simplify(SUBTRACT);
	Simplify(ADD);
	#ifdef PARANOID_COMPARE_EPSILON
		CompareForSanity(compare, a);
	#endif
	return *this;
}


ParanoidNumber & ParanoidNumber::operator-=(const digit_t & a)
{
	#ifdef PARANOID_COMPARE_EPSILON
		digit_t compare = Digit();
		compare -= a;
	#endif
	delete Operation(new ParanoidNumber(a), SUBTRACT);
	Simplify(ADD);
	Simplify(SUBTRACT);
	#ifdef PARANOID_COMPARE_EPSILON
		CompareForSanity(compare, a);
	#endif
	return *this;
}

ParanoidNumber & ParanoidNumber::operator*=(const digit_t & a)
{
	#ifdef PARANOID_COMPARE_EPSILON
		digit_t compare = Digit();
		compare *= a;
	#endif
	delete Operation(new ParanoidNumber(a), MULTIPLY);
	Simplify(DIVIDE);
	Simplify(MULTIPLY);
	#ifdef PARANOID_COMPARE_EPSILON
		CompareForSanity(compare, a);
	#endif
	return *this;
}


ParanoidNumber & ParanoidNumber::operator/=(const digit_t & a)
{
	#ifdef PARANOID_COMPARE_EPSILON
		digit_t compare = Digit();
		compare /= a;
	#endif
	delete Operation(new ParanoidNumber(a), DIVIDE);
	Simplify(MULTIPLY);
	Simplify(DIVIDE);
	#ifdef PARANOID_COMPARE_EPSILON
		CompareForSanity(compare, a);
	#endif
	return *this;
}


ParanoidNumber & ParanoidNumber::operator+=(const ParanoidNumber & a)
{
	#ifdef PARANOID_COMPARE_EPSILON
		digit_t compare = Digit();
		compare += a.Digit();
	#endif
	delete Operation(new ParanoidNumber(a), ADD);
	Simplify(SUBTRACT);
	Simplify(ADD);
	#ifdef PARANOID_COMPARE_EPSILON
		CompareForSanity(compare, a.Digit());
	#endif
	return *this;
}


ParanoidNumber & ParanoidNumber::operator-=(const ParanoidNumber & a)
{
	#ifdef PARANOID_COMPARE_EPSILON
		digit_t compare = Digit();
		compare -= a.Digit();
	#endif
	delete Operation(new ParanoidNumber(a), SUBTRACT);
	Simplify(ADD);
	Simplify(SUBTRACT);
	#ifdef PARANOID_COMPARE_EPSILON
		CompareForSanity(compare, a.Digit());
	#endif
	return *this;
}

ParanoidNumber & ParanoidNumber::operator*=(const ParanoidNumber & a)
{
	#ifdef PARANOID_COMPARE_EPSILON
		digit_t compare = Digit();
		compare *= a.Digit();
	#endif
	delete Operation(new ParanoidNumber(a), MULTIPLY);
	Simplify(DIVIDE);
	Simplify(MULTIPLY);
	#ifdef PARANOID_COMPARE_EPSILON
		CompareForSanity(compare, a.Digit());
	#endif
	return *this;
}


ParanoidNumber & ParanoidNumber::operator/=(const ParanoidNumber & a)
{
	#ifdef PARANOID_COMPARE_EPSILON
		digit_t compare = Digit();
		compare /= a.Digit();
	#endif
	delete Operation(new ParanoidNumber(a), DIVIDE);
	Simplify(MULTIPLY);
	Simplify(DIVIDE);
	#ifdef PARANOID_COMPARE_EPSILON
		CompareForSanity(compare, a.Digit());
	#endif
	return *this;
}

ParanoidNumber & ParanoidNumber::operator=(const digit_t & a)
{

	for (int i = 0; i < NOP; ++i)
	{
		for (auto n : m_next[i])
			delete n;
		m_next[i].clear();
	}
	m_value = a;
	#ifdef PARANOID_CACHE_RESULT
	m_cached_result = a;
	#endif

	#ifdef PARANOID_COMPARE_EPSILON
		CompareForSanity(a,a);
	#endif
	
	return *this;
}

// a + b
ParanoidNumber * ParanoidNumber::OperationTerm(ParanoidNumber * b, Optype op, ParanoidNumber ** merge_point, Optype * merge_op)
{
	////assert(b->SanityCheck());
	#ifdef PARANOID_CACHE_RESULTS
	m_cached_result = NAN;
	#endif
	#ifdef PARANOID_SIZE_LIMIT
		if (m_size >= PARANOID_SIZE_LIMIT)
		{
			this->operator=(this->Digit());
			if (op == ADD)
				m_value += b->Digit();
			else
				m_value -= b->Digit();
			m_size = 0;
			Debug("Cut off %p", this);
			return b;
		}
		//Debug("At size limit %d", m_size);
	#endif 
	

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
		
		//assert(SanityCheck());
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
				delete (OperationTerm(add, addop));
			}
			Optype subop = (op == ADD) ? SUBTRACT : ADD;
			for (auto sub : b->m_next[SUBTRACT])
				delete (OperationTerm(sub, subop));
				
			b->m_next[ADD].clear();
			b->m_next[SUBTRACT].clear();
			//assert(SanityCheck());
			return b;
		}
	}

	
	
	bool parent = (merge_point == NULL);
	ParanoidNumber * merge = this;
	Optype mop = op;
	//assert(mop != NOP); // silence compiler warning
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
			//assert(SanityCheck());
			return b;
		}
		
	}
	for (auto next : m_next[op])
	{
		if (next->OperationTerm(b, fwd, merge_point, merge_op) == b)
		{
			//assert(SanityCheck());
			return b;
		}
	}
	

	
	
	if (parent)
	{
		//merge->m_next[*merge_op].push_back(b);
		m_next[op].push_back(b);
		#ifdef PARANOID_SIZE_LIMIT
			m_size += 1+b->m_size;
		#endif	
	}
	else
	{
		if (m_next[op].size() == 0)
		{
			*merge_point = this;
			*merge_op = op;
		}
	}

	//assert(SanityCheck());

	return NULL;
}

ParanoidNumber * ParanoidNumber::OperationFactor(ParanoidNumber * b, Optype op, ParanoidNumber ** merge_point, Optype * merge_op)
{
	#ifdef PARANOID_CACHE_RESULTS
	m_cached_result = NAN;
	#endif
	#ifdef PARANOID_SIZE_LIMIT
		if (m_size >= PARANOID_SIZE_LIMIT)
		{
			this->operator=(this->Digit());
			if (op == MULTIPLY)
				m_value *= b->Digit();
			else
				m_value /= b->Digit();
			m_size = 0;
			
			Debug("Cut off %p", this);
			return b;
			
		}
	#endif	

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
				delete (n);
			m_next[i].clear();
			swap(m_next[i], b->m_next[i]);
		}
		//assert(SanityCheck());
		return b;
	}
	if (b->Floating() && b->m_value == 1)
		return b;
	if (b->Floating() && b->m_value == 0 && op == MULTIPLY)
	{
		operator=(*b);
		return b;
	}

		
	if (NoTerms() && b->NoTerms())
	{
		if (ParanoidOp<digit_t>(m_value, b->m_value, op))
		{
			Optype mulop = (op == MULTIPLY) ? MULTIPLY : DIVIDE;
			for (auto mul : b->m_next[MULTIPLY])
			{
				delete(OperationFactor(mul, mulop));
			}
			Optype divop = (op == MULTIPLY) ? DIVIDE : MULTIPLY;
			for (auto div : b->m_next[DIVIDE])
				delete(OperationFactor(div, divop));
				
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

	ParanoidNumber * cpy_b = new ParanoidNumber(*b);
	for (auto prev : m_next[invop])
	{
		if (prev->OperationFactor(b, rev, merge_point, merge_op) == b)
		{
			for (auto add : m_next[ADD])
				delete(add->OperationFactor(new ParanoidNumber(*cpy_b), op));
			for (auto sub : m_next[SUBTRACT])
				delete(sub->OperationFactor(new ParanoidNumber(*cpy_b), op));
				
			delete(cpy_b);
			return b;
		}
	}
	for (auto next : m_next[op])
	{
		if (next->OperationFactor(b, fwd, merge_point, merge_op) == b)
		{
			for (auto add : m_next[ADD])
			{
				delete(add->OperationFactor(new ParanoidNumber(*cpy_b), op));
			}
			for (auto sub : m_next[SUBTRACT])
			{
				delete(sub->OperationFactor(new ParanoidNumber(*cpy_b), op));
			}
			delete(cpy_b);
			return b;
		}
	}
	
	if (parent)
	{
		//assert(b != NULL);
		m_next[op].push_back(b);
		for (auto add : m_next[ADD])
			delete(add->OperationFactor(new ParanoidNumber(*cpy_b), op));
		for (auto sub : m_next[SUBTRACT])
			delete(sub->OperationFactor(new ParanoidNumber(*cpy_b), op));
			
		#ifdef PARANOID_SIZE_LIMIT
			m_size += 1+b->m_size;
		#endif	
	}
	//assert(SanityCheck());


	
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
		
	//assert(SanityCheck());
	vector<ParanoidNumber*> next;
	next.clear();
	swap(m_next[op], next);
	m_next[op].clear();
	//assert(m_next[op].size() == 0);
	//assert(SanityCheck());
	Optype fwd = op;
	if (op == DIVIDE)
		fwd = MULTIPLY;
	else if (op == SUBTRACT)
		fwd = ADD;
		
		
	vector<ParanoidNumber*> hold[2];
	if (op == MULTIPLY || op == DIVIDE)
	{
		swap(m_next[ADD], hold[0]);
		swap(m_next[SUBTRACT], hold[1]);
	}
	
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
				#ifdef PARANOID_SIZE_LIMIT
					m_size -= (1+result->m_size);
				#endif
				*m = NULL;
				delete(result);
			}
		}
	}
	
	
	
	for (auto n : next)
	{
		if (n != NULL)
		{		
			#ifdef PARANOID_SIZE_LIMIT
				if (Operation(n, op) == n)
				{
					m_size -= (1+n->m_size);
					delete n;
				}
			#else	
				delete(Operation(n, op));
			#endif 
		}
	}
	
	if (op == MULTIPLY || op == DIVIDE)
	{
		swap(m_next[ADD], hold[0]);
		swap(m_next[SUBTRACT], hold[1]);
	}
	
	set<ParanoidNumber*> s;
	//if (!SanityCheck(s))
	//{
	//	Error("Simplify broke Sanity");
	//}
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

	// Get around the absurd requirement that const correctness be observed.
	#ifdef PARANOID_CACHE_RESULTS
	digit_t & result = ((ParanoidNumber*)(this))->m_cached_result;
	
	if (!isnan(float(result))) // le sigh ambiguous function compiler warnings
		return result;
	#else
		digit_t result;
	#endif
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
		if (div->Digit() == 0)
		{
			Error("Divide by zero");
			return false;
		}
	}
	return true;
}

void ParanoidNumber::Negate()
{
	swap(m_next[ADD], m_next[SUBTRACT]);
	m_value = -m_value;
}

#ifdef PARANOID_USE_ARENA

void * ParanoidNumber::operator new(size_t s)
{
	return g_arena.allocate(s);
}

void ParanoidNumber::operator delete(void * p)
{
	g_arena.deallocate(p);
}

ParanoidNumber::Arena::Arena(int64_t block_size) : m_block_size(block_size), m_spare(NULL)
{
	m_blocks.push_back({malloc(block_size*sizeof(ParanoidNumber)),0});
}

ParanoidNumber::Arena::~Arena()
{
	for (auto block : m_blocks)
	{
		free(block.memory);
	}
	m_blocks.clear();
}

void * ParanoidNumber::Arena::allocate(size_t s)
{
	if (m_spare != NULL)
	{
		void * result = m_spare;
		m_spare = NULL;
		return result;
	}
		
	Block & b = m_blocks.back();
	void * result = (ParanoidNumber*)(b.memory) + (b.used++);
	if (b.used >= m_block_size)
	{
		m_block_size *= 2;
		Debug("Add block of size %d", m_block_size);
		m_blocks.push_back({malloc(m_block_size*sizeof(ParanoidNumber)), 0});
	}
	return result;
}

void ParanoidNumber::Arena::deallocate(void * p)
{
	m_spare = p;
}
#endif //PARANOID_USE_ARENA

}
