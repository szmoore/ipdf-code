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
	Real M = (exponent != 0);
	for (size_t i = 1; i < PREC; ++i)
	{
		if ((mantissa >> PREC-i) & 1)
			M += Real(1) / powl(BASE, i);
	}
	
	
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
	
	// Combine integer and fraction into mantissa
	if (integer == 0)
	{
		//Debug("0. %lx (%d)", fraction, fraction_leading_zeroes);
		// Mantissa must have leading '1'; shift to get leading 1, then shift back to mantissa position
		mantissa = (fraction << fraction_leading_zeroes) >> (max_bits - PREC);
		biased_exponent -= fraction_leading_zeroes;
		// If the unbiased exponent would be negative use a denormalised number instead
		if (biased_exponent < -(1LL << (EMAX-1))+1)
		{
			mantissa = mantissa >> (-(1LL << (EMAX-1))+1 - biased_exponent);
			biased_exponent = -(1LL << (EMAX-1))+1;
		}
	}
	else
	{

		size_t integer_leading_zeroes = 0;
		for (integer_leading_zeroes = 0; integer_leading_zeroes < max_bits; ++integer_leading_zeroes)
		{
			if ((integer >> (max_bits-1-integer_leading_zeroes)) & 1LL)
				break;
		}
		//Debug("%.16lx . %.16lx (%li, %li)", integer,fraction,  integer_leading_zeroes, fraction_leading_zeroes);

		mantissa = (integer << (integer_leading_zeroes)); // shift integer part
		mantissa |= (fraction >> (max_bits-integer_leading_zeroes)); // append the fraction after it
		//Debug("%.16lx (%.16lx << %i)", mantissa,  fraction, (fraction_leading_zeroes - max_bits + integer_leading_zeroes));
		mantissa = mantissa >> (max_bits - PREC); // shift back to the mantissa position
		biased_exponent = (max_bits - integer_leading_zeroes); // calculate exponent
	}

	// If the unbiased exponent would be non-zero, the IEEE has the leading 1 as implied; remove it and decrease exponent to compensate
	if (biased_exponent != -(1LL << (EMAX-1))+1)
	{
		mantissa &= ~(1LL << max_bits - PREC);
		mantissa = mantissa << 1;
		biased_exponent -= 1;
	}

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

	typedef pair<uint16_t, Real> Pear;
	list<Pear> space;
	uint16_t a0 = 0x0000;
	for (uint16_t a = a0; a < 0xFFFF; ++a)
	{
		Real x = BitsToReal<5,10>(&a);
		uint16_t b = 0; BitsFromReal<5,10>(x, &b);
		Real y = BitsToReal<5,10>(&b);
		if (y != x)
		{
			Fatal("%x -> %lf -> %x -> %lf", a, Float(x), b, Float(y));
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
