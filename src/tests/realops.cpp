/**
 * Test mathematical operations on the Real type and consistency with double
 */

#include "main.h"
#include "real.h"
#include "progressbar.h"

using namespace std;
using namespace IPDF;

#define TEST_CASES 1000

static double g_totalerror = 0;

bool NotEqual(double a, double b, double threshold=1e-4)
{
	double error = fabs(a-b);
	g_totalerror += error;
	return (error > threshold);
}

int main(int argc, char ** argv)
{
	srand(time(NULL));
	DebugRealInfo();
	
	unsigned failures = 0;
	Real acumulate(0);
	double dacumulate = 0;
	
	for (unsigned i = 0; i < TEST_CASES; ++i)
	{
		ProgressBar(i, TEST_CASES, 50);
		//Debug("Test %u of %u", i, TEST_CASES);
		double da = (double)(rand() + 1) / (double)(rand() + 1);
		double db = (double)(rand() + 1) / (double)(rand() + 1);
		
		if (rand() % 2 == 0)
			da = -da;
		if (rand() % 2 == 0)
			db = -db;
		
		Real a(da);
		Real b(db);
		Real aa(a);
		Real bb(b);
		unsigned old_failures = failures;
		if (NotEqual(Double(a), Double(aa)))
		{
			failures++;
			Warn("a != Real(a); %f vs %f", Double(a), Double(aa));
		}
		if (NotEqual(Double(b), Double(bb)))
		{
			failures++;
			Warn("b != Real(b); %f vs %f", Double(b), Double(bb));
		}
		
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
		Real abeforeop(a);
		if (NotEqual(Double(a+=b), da+=db))
		{
			failures++;			
			Warn("a += b = %f should be %f, a before op was %f", Double(a), da, Double(abeforeop));
		}
		abeforeop = a;
		if (NotEqual(Double(a-=b), da-=db))
		{
			failures++;			
			Warn("a -= b = %f should be %f, a before op was %f", Double(a), da, Double(abeforeop));
		}
		abeforeop = a;
		if (NotEqual(Double(a*=b), da*=db))
		{
			failures++;			
			Warn("a *= b = %f should be %f, a before op was %f", Double(a), da, Double(abeforeop));
		}
		abeforeop = a;
		if (NotEqual(Double(a/=b), da/=db))
		{
			failures++;
			Warn("a /= b = %f should be %f, a before op was %f", Double(a), da, Double(abeforeop));
		}
		if (NotEqual(Double(a*0.0 + 1.0), da*0.0 + 1.0))
		{
			failures++;
			Warn("a * 0 = %f should be %f, a before op was %f", Double(a), da, Double(abeforeop));
		}		

		if (NotEqual(Double(a=b), da=db))
		{
			failures++;
			Warn("a = b = %f should be %f, a before op was %f", Double(a), da, Double(abeforeop));
		}
		
		if (NotEqual(Double(-a), -da))
		{
			failures++;
			Warn("-a = %f should be %f, a before op was %f", Double(-a), -da, Double(abeforeop));
		}
		
		if (NotEqual(Double(Sqrt(a)), Sqrt(da)))
		{
			failures++;
			Warn("Sqrt(a) = %f should be %f, a before op was %f", Double(Sqrt(a)), Sqrt(da), Double(abeforeop));
		}		
		
		if (NotEqual(Double(a), da))
		{
			failures++;
			Warn("a = %f, should be %f, a before ops was %f", Double(a), da, Double(abeforeop));
		}
		
		switch (rand() % 4)
		{
			case 0:
				acumulate += a;
				dacumulate += da;
				break;
			case 1:
				acumulate -= a;
				dacumulate -= da;
				break;
			case 2:
				acumulate *= a;
				dacumulate *= da;
				break;
			case 3:
				acumulate /= a;
				dacumulate /= da;
				break;
		}
		if (NotEqual(Double(acumulate), dacumulate))
		{
			Warn("Accumulated result %.30lf vs %.30lf is wrong", Double(acumulate), dacumulate);
			failures++;
		}
		
		if (failures > old_failures)
		{
			Error("%u failures on case %u da = %f, db = %f, a = %f, b = %f, aa = %f, bb = %f", failures-old_failures, i, da, db, Double(a), Double(b), Double(aa), Double(bb));
			#if REAL == REAL_RATIONAL || REAL == REAL_RATIONAL_ARBINT
				Debug("\tStrings are a = %s, b = %s, aa = %s, bb = %s", a.Str().c_str(), b.Str().c_str(), aa.Str().c_str(), bb.Str().c_str());
			#endif
		}
	}
	Debug("Completed %u test cases with total of %u operations, %u failures", TEST_CASES, 18*TEST_CASES, failures);
	Debug("Total accumulated difference between Real and Double operations was %f", g_totalerror);
	Debug("Real: %.40lf", Double(acumulate));
	Debug("Doub: %.40lf", dacumulate);
	Debug("Diff: %.40lf", Double(acumulate) - dacumulate);

}
