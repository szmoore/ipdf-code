#include "stresstest.h"
#include "real.h"
#include "progressbar.h"

using namespace std;
using namespace IPDF;

int main(int argc, char ** argv)
{
	DebugRealInfo();
	Debug("Repeated Adds and Subtracts - Should give zero");
	
	clock_t first_clock = clock();
	clock_t elapsed = 0;
	for (int i = 1; i < 2; ++i)
	{
		for (int j = 1; j < 100; ++j)
		{
			clock_t start = clock();
			Real result = AddSub<Real>(0, i, Real(1)/Real(j));
			clock_t end = clock();
			elapsed += end - start;		
			printf("%d\t%d\t%.30lf\t%li\t%li\n", i, j, Double(result), end-start, elapsed);
			
		}
	}
	Debug("AddSub Total Time: %li", clock() - first_clock);

}

