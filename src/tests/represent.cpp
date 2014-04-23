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
 * Performs the inverse of BitsToReal
 * Hopefully
 * It chooses the normalised representation
 */
template <size_t EMAX, size_t PREC, size_t BASE = 2> void BitsFromReal(const Real & x, void * data)
{
	if (PREC + EMAX > 62)
		Fatal("Can't do more than 62 bits (asked for %d + %d = %d)", PREC, EMAX, PREC+EMAX);

	uint64_t integer = (uint64_t)(abs(x));
	uint64_t fraction = 0L;
	int64_t exponent = 0L;

	int64_t exp_bias = (1LL << (EMAX-1) - 1);

	Real f = Real(abs(x)) - Real(integer);
	//f = 0.375; integer = 12;
	// Get active fractional bits
	for (size_t i = 0; i <= PREC && f > 0; ++i)
	{
		//Debug("%d, %lf", i, Float(f));
		if (f >= 1)
		{	
			fraction |= (1LL << (PREC-i));
			f -= 1.0L;	
			//Debug("1");
		}
		//else
			//Debug("0");
		f *= 2.0L;
	}

	//Debug("Integer: %lx", integer);
	int offset = 0; while ((integer >> (offset++) != 0) && (offset < 8*sizeof(integer))); --offset;
	//Debug("Offset: %d", offset);
	//Debug("Fraction: %lx", fraction);	
	exponent += offset-1;
	uint64_t mantissa = fraction;	
	if (offset > 0)
	{
		//Debug("Mantissa %.2lx", mantissa);	
		mantissa = (integer << PREC) | fraction;
		//Debug("Mantissa %.2lx", mantissa);	
		mantissa = mantissa >> (offset-1);
		//Debug("Mantissa %.2lx", mantissa);	
		//mantissa = mantissa >> (offset-1);
	}
	else
	{
		mantissa = mantissa << 1;
	}
	//Debug("Mantissa %.2lx", mantissa);	

	//Debug("Exponent: %lx %li", exponent, exponent);
	if (exponent < -exp_bias)
	{
		mantissa = mantissa >> (-exp_bias - exponent);
		exponent = -exp_bias;
	}
	//Debug("Exponent: %lx %li", exponent, exponent);	
	exponent = exp_bias + exponent;
	//Real(exponent + 1) - Real(1LL << (EMAX-1))
	//Debug("Exponent: %lx %li", exponent, exponent);
	Real M = (exponent != 0);
	for (size_t i = 1; i < PREC; ++i)
	{
		if ((mantissa >> PREC-i) & 1)
			M += Real(1) / powl(BASE, i);
	}
	//Debug("M is %lf", Float(M));

	// Now copy the bits in (fun times)
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
	*c |= ((x < 0) << (i%8));
	
	
}



int main(int argc, char ** argv)
{
	printf("# Convert custom floats to a Real\n");
	printf("# a\thex(a)\tReal(a)\tdelta(last2)\n");

	typedef pair<uint8_t, Real> Pear;
	list<Pear> space;
	uint16_t a0 = 0x0000;
	for (uint16_t a = a0; a < a0+0x00FF; ++a)
	{
		Real x = BitsToReal<5,10>(&a);
		uint16_t b = 0; BitsFromReal<5,10>(x, &b);
		Real y = BitsToReal<5,10>(&b);
		if (y != x)
		{
			Warn("%x -> %lf -> %x -> %lf", a, Float(x), b, Float(y));
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
