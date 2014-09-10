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
	Debug("start at {%s} = %lf", a.Str().c_str(), a.ToDouble());
	cout << "0.3 ";
	float fa = 0.3;
	double da = 0.3;
	while (cin.good())
	{
		char op;
		cin >> op;
		string token("");
		for (char c = cin.peek(); cin.good() && !iswspace(c); c = cin.peek())
		{
			if (c == '+' || c == '-' || c == '*' || c == '/')
			{
				break;
			}
			token += c;
			c = cin.get();
		}
		Debug("String is %s", token.c_str());
		float fb = strtof(token.c_str(), NULL);
		double db = strtod(token.c_str(), NULL);
		ParanoidNumber b(token.c_str());
		Debug("b is {%s} %lf", b.Str().c_str(), b.ToDouble());
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
