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
	Debug("Sizeof ParanoidNumber::digit_t is %u", sizeof(ParanoidNumber::digit_t));
	Debug("Sizeof ParanoidNumber is %u", sizeof(ParanoidNumber));
	Debug("Sizeof double is %u", sizeof(double));
	Debug("Sizeof ParanoidNumber* %u", sizeof(ParanoidNumber*));

	string token("");
	cin >> token;	
	ParanoidNumber a(token.c_str());
	double da = a.ToDouble();
	float fa = da;
	while (cin.good())
	{
		Debug("a is {%s} \"%.40lf\"", a.Str().c_str(), a.ToDouble());
		char op;
		cin >> op;
		token = "";
		for (char c = cin.peek(); cin.good() && !iswspace(c); c = cin.peek())
		{
			if (c == '+' || c == '-' || c == '*' || c == '/')
			{
				break;
			}
			token += c;
			c = cin.get();
		}
		
		//Debug("String is \"%s\"", token.c_str());
		float fb = strtof(token.c_str(), NULL);
		double db = strtod(token.c_str(), NULL);
		ParanoidNumber b(token.c_str());
	
		Debug("b is {%s} \"%lf\"", b.Str().c_str(), b.ToDouble());
		Debug("db is %lf", db);
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
			
		Debug("a is: {%s}", a.Str().c_str());
		Debug("a as double: %.40lf", a.ToDouble());
		//Debug("a as float: %.40f", a.ToFloat());
		//Debug("a as int64_t: %ld", a.Convert<int64_t>());
		//Debug("floats give: %.40f", fa);
		Debug("double gives: %.40lf", da);
		
		
	}
}
