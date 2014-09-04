#include "main.h"
#include "real.h"
#include <cmath>
#include <cassert>
#include <list>
#include <bitset>
#include <iostream>
#include <cfloat>
#include <fenv.h>

using namespace std;
using namespace IPDF;

int main(int argc, char ** argv)
{
	Debug("FLT_MAX = %.40f", FLT_MAX);
	Debug("FLT_MIN = %.40f", FLT_MIN);
	Debug("FLT_EPSILON = %.40f", FLT_EPSILON);
	
	while (true)
	{
		float a; float b;
		char op;
		feclearexcept(FE_ALL_EXCEPT);
		cin >> a >> op >> b;
		
		float c;
		feclearexcept(FE_ALL_EXCEPT);
		switch (op)
		{
			case '+':
				c = a + b;
				break;
			case '-':
				c = a - b;
				break;
			case '*':
				c = a * b;
				break;
			case '/':
				c = a / b;
				break;
		}
		if (fetestexcept(FE_OVERFLOW))
			Debug("Overflow occured");
		else if (fetestexcept(FE_UNDERFLOW))
			Debug("Underflow occured");
		else if (fetestexcept(FE_INEXACT))
			Debug("Inexact result");
			
		printf("%.40f\n", c);
		
		
	}
}
