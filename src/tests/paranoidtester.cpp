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
#include "progressbar.h"

using namespace std;
using namespace IPDF;

string RandomNumberAsString(int max_digits = 12)
{
	string result("");
	int digits = 1+(rand() % max_digits);
	int dp = (rand() % digits)+1;
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

bool CloseEnough(double d, ParanoidNumber & p)
{
	double pd = p.ToDouble();
		
	if (d == 0)
		return fabs(pd) <= 1e-6;
	return fabs((fabs(pd - d) / d)) <= 1e-6;
}

#define TEST_CASES 1000

int main(int argc, char ** argv)
{
	srand(time(NULL));
	string number(RandomNumberAsString());
	ParanoidNumber a(number);
	float fa = strtof(number.c_str(), NULL);
	double da = strtod(number.c_str(), NULL);
	double diff = 0;
	long double lda = strtold(number.c_str(), NULL);
	
	if (fabs(a.ToDouble() - da) > 1e-6)
	{
		Error("double %lf, pn %lf {%s}", da, a.ToDouble(), a.Str().c_str());
		Fatal("Didn't construct correctly off %s", number.c_str());
	}
	
	char opch[] = {'+','-','*','/'};
	int opcount[] = {0,0,0,0};
	for (int i = 0; i < TEST_CASES; ++i)
	{
		ProgressBar(i, TEST_CASES); fprintf(stderr, "%.30lf (%d)+ (%d)- (%d)* (%d)/ [Paranoia %ld]",diff, opcount[0],opcount[1],opcount[2],opcount[3], ParanoidNumber::Paranoia());
		number = RandomNumberAsString();
		ParanoidNumber b(number);
		float fb = strtof(number.c_str(), NULL);
		double db = strtod(number.c_str(), NULL);
		if (db == 0)
		{
			--i;
			continue;
		}
		long double ldb = strtold(number.c_str(), NULL);
		int op = rand() % 4;//= 2*(rand() % 2);
		while (op == 1)
			op = rand() % 4;
		//if (rand() % 2 == 0)
			//op = 2+(rand() % 2);
			
		opcount[op]++;
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
		diff = 100.0*(fabs(a.ToDouble() - da) / da);
		if (!CloseEnough(da, a))
		{
			Error("Op %i: ParanoidNumber probably doesn't work", i);
			Error("Operation: %lf %c %lf", oldda, opch[op], db);
			Error("As PN: %lf %c %lf", olda.ToDouble(), opch[op], b.ToDouble());
			Error("PN String: %s", a.Str().c_str());
			Error("Diff is %.40lf", diff);
			Fatal("%.40lf, expected aboout %.40lf", a.ToDouble(), da);
			
			
		}
		
	}
	printf("ParanoidNumber: {%s} = %.40lf\n", a.Str().c_str(), a.ToDouble());
	printf("float: %.40f\n", fa);
	printf("double: %.40lf\n", da);
	printf("long double: %.40Lf\n", lda);
	printf("diff %.40lf\n", diff);
}
