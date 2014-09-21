#include "stresstest.h"
#include "real.h"
#include "progressbar.h"

using namespace std;
using namespace IPDF;

int main(int argc, char ** argv)
{
	Debug("Repeated Adds and Subtracts - Should give zero");
	
	for (int i = 1; i < 100; ++i)
	{
		for (int j = 1; j < 100; ++j)
		{
			Real result = AddSub(0, i, Real(1)/Real(j));
			if (result != Real(0))
				Warn("Result of %lf != %lf (zero)", Double(result), 0.0);
				
			printf("%d\t%lf\t%lf", i, Real(1)/Real(j), Double(result));
		}
	}
	
	Debug("Repeated Multiplications and Divisions - Should give one");
	for (int i = 1; i < 100; ++i)
	{
		for (int j = 1; j < 100; ++j)
		{
			Real result = MulDiv(1, i, Real(1)/Real(j));
			if (result != Real(1))
				Warn("Result of %lf != %lf (one)", Double(result), 1.0);
				
			printf("%d\t%lf\t%lf", i, Real(1)/Real(j), Double(result));
		}
	}

}

