#ifndef _REALOPS_H
#define _REALOPS_H

// Test real operations

#include <cstdlib>

/**
 * Test adding and subtracting values in a range
 * This is an identity; the result should be zero.
 */ 
template <class T>
T AddSub(const T & start, const T & end, const T & delta = T(1))
{
	T result = 0;
	for (T a = start; a < end; a += delta)
	{
		result += a;
		for (T b = start; b < end; b += delta)
		{
			result -= b;
		}
		for (T b = start; b < end; b += delta)
		{
			result += b;
		}	
	}
	for (T a = start; a < end; a += delta)
	{
		result -= a;
		for (T b = start; b < end; b += delta)
		{
			result += b;
		}
		for (T b = start; b < end; b += delta)
		{
			result -= b;
		}
	}
	return result;
}

template <class T>
T MulDiv(const T & start, const T & end, const T & delta = T(1))
{
	T result = 1;
	for (T a = start; a < end; a += delta)
	{
		result *= a;
		for (T b = start; b < end; b += delta)
		{
			result /= b;
		}
		for (T b = start; b < end; b += delta)
		{
			result *= b;
		}	
	}
	for (T a = start; a < end; a += delta)
	{
		result /= a;
		for (T b = start; b < end; b += delta)
		{
			result *= b;
		}
		for (T b = start; b < end; b += delta)
		{
			result /= b;
		}
	}
	return result;
}



template <class T>
void RandomOp(T & a, int recurses=0, const T & max = 1e-6, const T & min = 1e-15)
{
	T b;
	if (recurses > 0)
		b = (min + (rand() % (int64_t)(max)));
	else
		b = RandomOps(recurses-1, max, min);	
	switch (rand() % 4)
	{
		case 0:
			a += b;
			break;
		case 1:
			a -= b;
			break;
		case 2:
			a *= b;
			break;
		case 3:
			a /= b;
			break;
	}
}

/**
 * Test replying repeated operations randomly
 */
template <class T>
T RandomOps(int ops, int recurses=0, const T & min = 1e-15 ,const T & max = 1e-6)
{
	T a = (min + (rand() % (int64_t)(max)));
	

	
	for (int i = 0; i < ops; ++i)
	{
		RandomOp(a, recurses, min, max);
	}
	return a;
}


/**
 * Test multiplying and dividing values in a range
 * Such that the result is one
 */
template <class T>
T RandomOp(const T & start, const T & end, const T & delta = T(1))
{
	T result = 1;
	for (T a = start; a < end; a += delta)
	{
		result *= a;
		for (T b = start; b < end; b += delta)
		{
			result /= b;
		}
		for (T b = start; b < end; b += delta)
		{
			result *= b;
		}	
	}
	for (T a = start; a < end; a += a)
	{
		result /= a;
		for (T b = start; b < end; b += delta)
		{
			result *= b;
		}
		for (T b = start; b < end; b += delta)
		{
			result /= b;
		}
	}
	return result;	
}


#endif //_REALOPS_H
