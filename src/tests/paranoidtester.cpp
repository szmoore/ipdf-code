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

string RandomNumberAsString(int digits = 6)
{
	string result("");
	int dp = 1+(rand() % 3);
	for (int i = 0; i < digits; ++i)
	{
		if (i == dp)
		{
			result += ".";
			continue;
		}
		result += ('0'+rand() % 10);
	}
	return result;
}

#define TEST_CASES 10000

int main(int argc, char ** argv)
{

	string number(RandomNumberAsString());
	ParanoidNumber a(number);
	float fa = strtof(number.c_str(), NULL);
	double da = strtod(number.c_str(), NULL);
	long double lda = strtold(number.c_str(), NULL);
	
	if (fabs(a.ToDouble() - da) > 1e-6)
	{
		Error("double %lf, pn %lf {%s}", da, a.ToDouble(), a.Str().c_str());
		Fatal("Didn't construct correctly off %s", number.c_str());
	}
	
	char opch[] = {'+','-','*','/'};
	
	for (int i = 0; i < TEST_CASES; ++i)
	{
		number = RandomNumberAsString();
		ParanoidNumber b(number);
		float fb = strtof(number.c_str(), NULL);
		double db = strtod(number.c_str(), NULL);
		long double ldb = strtold(number.c_str(), NULL);
		int op = (rand() % 4);
		ParanoidNumber olda(a);
		double oldda(da);
		switch (op)
		{
			case 0:
				a += b;
				fa += fb;
				da += db;
				lda += ldb;
				break;
			case 1:
				a -= b;
				fa -= fb;
				da -= db;
				lda -= ldb;
				break;
			case 2:
				a *= b;
				fa *= fb;
				da *= db;
				lda *= ldb;
				break;
			case 3:
				a /= b;
				fa /= fb;
				da /= db;
				lda /= ldb;
				break;
		}
		if (fabs(a.ToDouble() - da) > 1.0 )
		{
			Error("Op %i: ParanoidNumber probably doesn't work", i);
			Error("Operation: %lf %c %lf", oldda, opch[op], db);
			Error("As PN: %lf %c %lf", olda.ToDouble(), opch[op], b.ToDouble());
			Fatal("%lf, expected aboout %lf", a.ToDouble(), da);
		}
	}
	printf("ParanoidNumber: {%s} = %.40lf\n", a.Str().c_str(), a.ToDouble());
	printf("float: %.40f\n", fa);
	printf("double: %.40lf\n", da);
	printf("long double: %.40Lf\n", lda);
}
