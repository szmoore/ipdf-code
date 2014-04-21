#include "main.h"
#include "real.h"

using namespace std;
using namespace IPDF;


int main(int argc, char ** argv)
{
	srand(time(NULL));
	Real a = Random(100);
	Real b = Random(100);
	Debug("a = %f", Float(a));
	Debug("b = %f", Float(b));
	Debug("a + b = %f", Float(a + b));
	Debug("a - b = %f", Float(a - b));
	Debug("a * b = %f", Float(a * b));
	Debug("a / b = %f", Float(a / b));
	Debug("a += b => %f", Float(a += b));
	Debug("a -= b => %f", Float(a -= b));
	Debug("a *= b => %f", Float(a *= b));
	Debug("a /= b => %f", Float(a /= b));
	Debug("a = %f", Float(a));
	Debug("b = %f", Float(b));

}
