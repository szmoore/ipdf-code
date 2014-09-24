#include "stresstest.h"
#include "real.h"
#include "progressbar.h"

using namespace std;
using namespace IPDF;

int main(int argc, char ** argv)
{
	DebugRealInfo();
	
	Debug("Repeated Multiplications and Divisions - Should give one");
	first_clock = clock();
	elapsed = 0;
	for (int i = 1; i < 2; ++i)
	{
		for (int j = 1; j < 100; ++j)
		{
			clock_t start = clock();
			Real result = MulDiv<Real>(1, i, Real(1)/Real(j));
			clock_t end = clock();
			printf("%d\t%d\t%.30lf\t%li\t%li\n", i, j, Double(result), end-start, elapsed);
		}
	}
	Debug("MulDiv Total time: %li", clock() - first_clock);

}

