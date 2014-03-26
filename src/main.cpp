#include "common.h"

#include "ipdf.h"

using namespace std;
using namespace IPDF;

int main(int argc, char ** argv)
{
	Real a = 10;
	Real b = 20;
	Debug("a*b = %f", RealToFloat(a*b));
	return 0;
}
