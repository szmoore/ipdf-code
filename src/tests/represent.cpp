#include "main.h"
#include "real.h"
#include <cmath>
#include <cassert>
#include <list>

using namespace std;
using namespace IPDF;

/**
 * This "tester" lets us construct Reals out of Floating Points with arbitrary sizes (but must be less than 64 bits currently)
 * Eventually it will also go the other way
 * This is originally for conceptual understanding but will ultimately become useful later on.
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
 * Converts data represented as an (se, e, m, sm) floating point number to a Real.
 * se = sign of exponent (1 bit)
 * e = exponent (EMAX bits)
 * m = mantissa (PREC bits)
 * sm = sign of mantissa (1 bit)
 * Total 1+EMAX+PREC+1 must not exceed 64 (at least, for the moment)
 */
template <unsigned EMAX, unsigned PREC, unsigned BASE = 2> Real BitsToReal(void * data)
{
	if (PREC + EMAX > 62) // we need 1 bit for the sign of the exponent, 1 bit for the sign
		Fatal("Can't do more than 62 bits (asked for %d + %d = %d)", PREC, EMAX, PREC+EMAX);
	
	// memcpy needs a whole number of bytes
	// This means we need to do tricky bit shifting to get rid of parts of the exp/mantissa that overlap
	size_t ebytes = ceil(double(EMAX+1.0) / 8.0); 	
	size_t mbytes = ceil(double(PREC+1.0)/8.0);
	size_t moffset = floor(double(EMAX+1.0) / 8.0); // offset to start of mantissa (nearest byte)
	//Debug("ebytes %d, mbytes %d, moffset %d", ebytes, mbytes, moffset);

	// Get the exponent and its sign
	uint64_t exponent = 0L;
	bool exp_sign = false;

	memcpy(&exponent, data, ebytes); // Copy exponent
	//Debug("exponent + garbage: %x", exponent);
	exp_sign = (1L << (8*ebytes-1)) & exponent;
	exponent &= ~(1L << (8*ebytes-1)); // mask out the sign
	//Debug("exponent - masked sign: %x", exponent);
	exponent = exponent >> (PREC+1); // shift out the extra bits (part of mantissa)
	assert(CountBits(exponent) <= EMAX); //TODO: Remove once sure it actually works //TODO: Need more reliable sanity checks probably

	// Get the mantissa and its sign
	uint64_t mantissa = 0L;
	bool sign = false;

	memcpy(&mantissa, ((uint8_t*)(data) + moffset), mbytes); // copy data
	//Debug("mantissa + garbage: %x", mantissa);
	sign = mantissa & (1L); // get sign
	mantissa = mantissa >> 1; // discard sign
	mantissa = (mantissa << (8*sizeof(mantissa) - PREC)) >> (8*sizeof(mantissa) - PREC);
	assert(CountBits(mantissa) <= PREC);
	
	/*
	Debug("EMAX %u, PREC %u, BASE %u", EMAX, PREC, BASE);
	Debug("EXP: %x", exponent);
	Debug("MANTISSA: %x", mantissa);
	Debug("EXP_SIGN: %x", (uint8_t)exp_sign);
	Debug("MANTISSA_SIGN: %x", (uint8_t)sign);
	*/

	Real Q = (exp_sign) ? pow(Real(BASE), -Real(exponent)) : pow(Real(BASE), Real(exponent));
	Real x = Real(mantissa) * Q;
	return (sign) ? -x : x;
}

/**
 * Performs the inverse of BitsToReal
 */
template <int EMAX, int PREC, int BASE = 2> void BitsFromReal(const Real & x, void * data)
{
	bool sign;
	uint64_t mantissa;
	uint64_t exponent;
	if (PREC + EMAX > 62)
		Fatal("Can't do more than 62 bits (asked for %d + %d = %d)", PREC, EMAX, PREC+EMAX);
	
	//TODO: Implement
	
}



int main(int argc, char ** argv)
{
	printf("# Convert custom floats to a Real\n");
	printf("# a\thex(a)\tReal(a)\tdelta(last2)\n");

	typedef pair<uint8_t, Real> Pear;
	list<Pear> space;

	for (uint8_t a = 0x00; a < 0xFF; ++a)
	{
		Real x = BitsToReal<2,4>(&a);
		space.push_back(pair<uint8_t, Real>(a, x));
	}
	space.sort([=](const Pear & a, const Pear & b){return a.second < b.second;});
	Real prev;
	for (list<Pear>::iterator i = space.begin(); i != space.end(); ++i)
	{
		printf("%u\t%x\t%f", i->first, i->first, Float(i->second));
		if (i != space.begin())
		{
			printf("\t%f\n", Float(i->second - prev));
		}	
		prev = i->second;	 
	}
}
