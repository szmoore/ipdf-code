#include "main.h"
#include "real.h"
#include <cmath>
#include <cassert>
#include <list>
#include <bitset>
#include <iostream>
#include <cfloat>
#include <fenv.h>
#include "paranoidnumber.h"

using namespace std;
using namespace IPDF;

int main(int argc, char ** argv)
{
	Debug("FLT_MAX = %.40f", FLT_MAX);
	Debug("FLT_MIN = %.40f", FLT_MIN);
	Debug("FLT_EPSILON = %.40f", FLT_EPSILON);
	
	ParanoidNumber a("0.3");
	Debug("start at %s", a.Str().c_str());
	cout << "0.3 ";
	float fa = 0.3;
	double da = 0.3;
	while (cin.good())
	{
		char op;
		double db;
		cin >> op >> db;
		float fb(db);
		ParanoidNumber b(fb);
		switch (op)
		{
			case '+':
				a += b;
				fa += fb;
				da += db;
				break;
			case '-':
				a -= b;
				fa -= fb;
				da -= db;
				break;
			case '*':
				a *= b;
				fa *= fb;
				da *= db;
				break;
			case '/':
				a /= b;
				fa /= fb;
				da /= db;
				break;
		}
			
		Debug("a is: %s", a.Str().c_str());
		Debug("a as double: %.40f\n", a.ToDouble());
		Debug("floats give: %.40f\n", fa);
		Debug("double gives: %.40f\n", da);
		
		
	}
}
