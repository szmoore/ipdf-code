#include "main.h"
#include "real.h"

using namespace std;
using namespace IPDF;

#define TEST_CASES 100

bool NotEqual(double a, double b, double threshold=1e-4)
{
	return (fabs(a-b) > threshold);
}

int main(int argc, char ** argv)
{
	srand(time(NULL));
	
	unsigned failures = 0;
	for (unsigned i = 0; i < TEST_CASES; ++i)
	{
		double da = (double)(rand()) / (double)(rand());
		double db = (double)(rand()) / (double)(rand());
		
		Real a(da);
		Real b(db);
		
		
		
		if (NotEqual(Double(a), da))
		{
			failures++;
			Warn("a != da; %f vs %f", Double(a), da);
		}
		if (NotEqual(Double(b), db))
		{
			failures++;			
			Warn("b != db; %f vs %f", Double(b), db);
		}
		if (NotEqual(Double(a+b), da+db))
		{
			failures++;			
			Warn("a + b = %f should be %f", Double(a+b), da+db);
		}
		if (NotEqual(Double(a-b), da-db))
		{
			failures++;			
			Warn("a - b = %f should be %f", Double(a-b), da-db);
		}
		if (NotEqual(Double(a*b), da*db))
		{
			failures++;			
			Warn("a * b = %f should be %f", Double(a*b), da*db);
		}
		if (NotEqual(Double(a/b), da/db))
		{
			failures++;			
			Warn("a / b = %f should be %f", Double(a/b), da/db);
		}		

		if (NotEqual(Double(a), da))
		{
			failures++;			
			Warn("a has changed after +-*/ from %f to %f", da, Double(a));
		}
		if (NotEqual(Double(b), db))
		{
			failures++;			
			Warn("b has changed after +-*/ from %f to %f", db, Double(b));
		}

		if (NotEqual(Double(a+=b), da+=db))
		{
			failures++;			
			Warn("a += b = %f should be %f", Double(a), da);
		}
		if (NotEqual(Double(a-=b), da-=db))
		{
			failures++;			
			Warn("a -= b = %f should be %f", Double(a), da);
		}
		if (NotEqual(Double(a*=b), da*=db))
		{
			failures++;			
			Warn("a *= b = %f should be %f", Double(a), da);
		}
		if (NotEqual(Double(a/=b), da/=db))
		{
			failures++;
			Warn("a /= b = %f should be %f", Double(a), da);
		}		
	}
	Debug("Completed %u test cases with total of %u operations, %u failures", TEST_CASES, 12*TEST_CASES, failures);

}
