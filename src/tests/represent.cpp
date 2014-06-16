#include "main.h"
#include "real.h"
#include <cmath>
#include <cassert>
#include <list>
#include <bitset>

using namespace std;
using namespace IPDF;

/**
 * This "tester" lets us construct Reals out of Floating Points with arbitrary sizes (but must be less than 64 bits currently)
 * Eventually it will also go the other way
 * This is originally for conceptual understanding but will ultimately become useful later on.
 * Bahahaha
 */


/**
 * From http://stackoverflow.com/questions/109023/how-to-count-the-number-of-set-bits-in-a-32-bit-integer
 */
long CountBits(long n) 
{     
	unsigned int c; // c accumulates the total bits set in v
	for (c = 0; n; c++)
		n &= n - 1; // clear the least significant bit set
	return c;
}

/**
 * Converts data represented as an (s, e, m) floating point number to a Real.
 * Now actually correct for floats bigger than 8 bits!
 * Now with 100% less memcpy and 100% more pointer arithmetic!
 * Now with IEEE style encodings! (How many flops does it take to represent a float this way? TOO MANY!)
 * Still probably breaks if you are not using an x86 based processor
 * s = sign
 * e = exponent (IEEE ``offset'' encoding)
 * m = mantissa (IEEE encoding)
 */
template <size_t EMAX, size_t PREC, size_t BASE = 2> Real BitsToReal(void * data)
{
	if (PREC + EMAX > 62) // we need 1 bit for the sign of the exponent, 1 bit for the sign
		Fatal("Can't do more than 62 bits (asked for %d + %d = %d)", PREC, EMAX, PREC+EMAX);

	size_t mbytes = ceil(double(PREC)/8.0);	
	size_t mtrunc = floor(double(PREC)/8.0);
	uint64_t mantissa = 0L;
	uint8_t * c = (uint8_t*)(data);
	for (size_t i = 0; i < mbytes; ++i)
	{
		mantissa |= *(c++) << (i*8);	
	}
	mantissa &= ~(~0L << PREC);

	size_t ebytes = ceil(double(EMAX)/8.0);
	uint64_t exponent = *(c-1) & (~0L << (PREC-8*mtrunc));
	for (size_t i = 0; i < ebytes; ++i)
	{
		exponent |= *(c++) << (i+1)*8;
	}
	exponent = exponent >> (PREC-8*mtrunc);
	bool sign = exponent & (1LL << EMAX);
	exponent &= ~(~0L << EMAX);

	// Floats are defined by
	// x = (-1)^s * m * B^{e}

	// In IEEE things get a little crazy though, the mantissa is stored reversed and the digits are summed indexed using negative powers of 2 (not positive)
	// So to conform with IEEE can't just treat the mantissa as an unsigned integer

	// Calculate the actual mantissa from the IEEE style encoding of the mantissa (sigh)
	// This is obviously horribly inefficient but I guess that encoding not designed for ... whatever it is I'm doing here
	// NOW WITH LESS TERRIBLE LOOPING THAT GIVES SLIGHTLY WRONG RESULTS! Order a copy today.
	Real M = (((exponent != 0) << PREC) + (uint64_t)(mantissa))/powl(BASE,PREC);
	
	//Debug("Mantissa: %x = %lf", mantissa, Float(M));
	//Debug("Exponent: %x = %u => %lf", exponent, exponent, Float(Real(exponent + 1) - Real(1LL << (EMAX-1))));
	//Debug("Sign: %x = %u", sign, sign);
	
	
	Real x = M * powl(Real(BASE), Real(exponent + 1)-Real(1LL << (EMAX-1)));
	return (sign) ? -x : x;
}

/**
 * Modifies fraction to contain the fractional part of x
 * Returns position of the first '1' bit
 */
size_t Fraction(Real x, uint64_t & fraction)
{
	x = abs(x) - (uint64_t)(abs(x));
	fraction = 0LL;
	size_t offset = 0;
	for (size_t i = 0; i < 8*sizeof(fraction); ++i)
	{
		x *= 2;
		if (x >= 1.0)
		{
			offset = (offset == 0) ? i : offset;
			fraction |= (1LL << (8*sizeof(fraction)-1-i));
			x -= 1.0;
		}
		if (x <= 0.0)
			break;
	}
	return fraction;
}

/**
 * Copy mantissa and exponent into data
 */
template <size_t EMAX, size_t PREC, size_t BASE = 2> void BitsFromRepr(bool sign, uint64_t mantissa, uint64_t exponent, void * data)
{
	uint8_t * c = (uint8_t*)(data);
	size_t i = 0;
	for (i = 0; i < PREC; ++i)
	{
		*c |= ((mantissa >> i) & 1) << (i%8);
		if ((i+1) % 8 == 0) 
		{
			++c;
		}
	}
	for (; i < PREC+EMAX; ++i)
	{
		*c |= ((exponent >> (i-PREC)) & 1) << (i%8);
		if ((i+1) % 8 == 0) 
		{
			++c;
		}
	}
	*c |= (sign << (i%8));
}




/**
 * Performs the inverse of BitsToReal
 * Hopefully
 * It chooses the normalised representation
 */
template <size_t EMAX, size_t PREC, size_t BASE = 2> void BitsFromReal(Real x, void * data)
{
	if (PREC + EMAX > 62)
		Fatal("Can't do more than 62 bits (asked for %d + %d = %d)", PREC, EMAX, PREC+EMAX);

	uint64_t integer = (uint64_t)(abs(x));
	uint64_t fraction = 0LL;
	const size_t max_bits = 8*sizeof(uint64_t);
	bool sign = (x < 0);
	x = abs(x) - integer;
	
	size_t fraction_leading_zeroes = max_bits;
	for (size_t i = 0; i < max_bits; ++i)
	{
		x *= 2;
		if (x >= 1.0)
		{
			if (fraction_leading_zeroes > i)
				fraction_leading_zeroes = i;
			x -= 1;
			fraction |= (1LL << (max_bits-1-i));
		}
		if (x <= 0.0)
			break;
	}

	uint64_t mantissa = 0LL;
	int64_t biased_exponent = 0LL;
	
	size_t integer_leading_zeroes = 0;
	for (integer_leading_zeroes = 0; integer_leading_zeroes < max_bits; ++integer_leading_zeroes)
	{
		if ((integer >> (max_bits-1-integer_leading_zeroes)) & 1LL)
			break;
	}

	// 0|011 11|00 0000 0001
	// 0001
	// 0000 0000 0100
	// 10000 0000 0100
	if (integer != 0)
	{
		mantissa = (integer << (integer_leading_zeroes));
		mantissa |= (fraction >> (max_bits - integer_leading_zeroes));
	}
	else
	{
		mantissa = (fraction << (fraction_leading_zeroes));
	}
	//Debug("Mantissa with fraction %.16lx (%.16lx >><>?<< %lu)", mantissa, fraction, fraction_leading_zeroes);
	mantissa = (mantissa >> (max_bits - PREC - 1));
	//Debug("Mantissa is %.16lx (%.16lx . %.16lx)", mantissa, fraction, integer);
	biased_exponent = (integer != 0) ? (max_bits - integer_leading_zeroes) : (-fraction_leading_zeroes);
	if (biased_exponent < -(1LL << (EMAX-1))+1)
	{
		//Debug("Denormalising for glory or death! %li , %li", biased_exponent, -(1LL << (EMAX-1)) - biased_exponent + 1);
		mantissa = mantissa >> (-(1LL << (EMAX-1)) - biased_exponent + 1);
		biased_exponent = -(1LL << (EMAX-1))+1;
	}
	//Debug("Biased exponent is %li", biased_exponent);

	// If the unbiased exponent would be non-zero, the IEEE has the leading 1 as implied; remove it and decrease exponent to compensate
	if (biased_exponent != -(1LL << (EMAX-1))+1)
	{
		//Debug("Implied 1 in mantissa %.16lx", mantissa);
		mantissa &= ~(1LL << (PREC));
		//Debug("Implied 1 in mantissa %.16lx", mantissa);
		biased_exponent -= 1;
	}
	else
	{
		mantissa >>= 1;
	}
	//Debug("%.16lx", mantissa);

	uint64_t exponent = (uint64_t)(biased_exponent + (1LL << (EMAX-1))-1); // offset the exponent

	// Debug
	/*
	Real M = (exponent != 0);
	for (size_t i = 1; i < PREC; ++i)
	{
		if ((mantissa >> PREC-i) & 1)
			M += Real(1) / powl(BASE, i);
	}

	Debug("Exponent: %lx %lu -> %li", exponent, exponent, biased_exponent);
	Debug("Mantissa: %lx -> %f", mantissa, Float(M));
	Debug("Both: %lx", mantissa | (exponent << (PREC)));
	*/
	// Now copy the bits in
	BitsFromRepr<EMAX, PREC, BASE>(sign, mantissa, exponent, data);
	
}



int main(int argc, char ** argv)
{
	printf("# Convert custom floats to a Real\n");
	printf("# a\thex(a)\tReal(a)\tdelta(last2)\n");

	typedef uint8_t Bits;
	typedef pair<Bits, Real> Pear;
	const uint8_t E = 3;
	const uint8_t P = 4;
	
	list<Pear> space;
	Bits a0 = 0x00;
	for (Bits a = a0; a < 0xFF; ++a)
	{
		Real x = BitsToReal<E,P>(&a);
		Bits b = 0; BitsFromReal<E,P>(x, &b);
		Real y = BitsToReal<E,P>(&b);
		if (y != x)
		{
			Fatal("%x -> %.10lf -> %x -> %.10lf", a, Float(x), b, Float(y));
		}
		space.push_back(Pear(a, x));
	}
	space.sort([=](const Pear & a, const Pear & b){return a.second < b.second;});
	Real prev;
	for (list<Pear>::iterator i = space.begin(); i != space.end(); ++i)
	{
		printf("%u\t%.2x\t%.20lf", i->first, i->first, Float(i->second));
		if (i != space.begin())
			printf("\t%f", Float(i->second - prev));
		printf("\n");
		prev = i->second;	 
	}
}

//0|011 11|00 0000 0001 

//0|000 01|00 0000 0000
