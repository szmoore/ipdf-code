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

string RandomNumberAsString(int max_digits = 3)
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

bool CloseEnough(long double d, ParanoidNumber & p, long double eps = 1e-6)
{
	long double pd = p.Convert<long double>();
		
	if (d == 0)
		return fabs(pd) <= eps;
	return fabs((fabs(pd - d) / d)) <= eps;
}

void TestOp(ParanoidNumber & p, double & d, Optype op, const double amount)
{
	string p0str(p.Str());
	double p0 = p.ToDouble();
	switch (op)
	{
		case ADD:
			p += amount;
			d += amount;
			break;
		case SUBTRACT:
			p -= amount;
			d -= amount;
			break;
		case MULTIPLY:
			p *= amount;
			d *= amount;
			break;
		case DIVIDE:
			p /= amount;
			d /= amount;
			break;
		default:
			break;
	}
	if (!CloseEnough(d, p))
	{
		Debug("%lf %c= %lf failed", p0, OpChar(op), amount);
		Debug("%lf vs %lf", p.ToDouble(), d);
		Debug("Before: {%s}\n", p0str.c_str());
		Debug("After: {%s}\n", p.Str().c_str());
		Fatal(":-(");
	}

}

void TestAddSubIntegers(int max=100)
{
	Debug("Test add/sub integers 0 -> %i", max);
	ParanoidNumber p;
	double d(0);
	for (int a = 0; a < max; ++a)
	{
		TestOp(p, d, ADD, a);
		for (int b = 0; b < max; ++b)
		{
			TestOp(p, d, SUBTRACT, b);
		}
		for (int b = 0; b < max; ++b)
		{
			TestOp(p, d, ADD, b);
		}
	}
	for (int a = 0; a < max; ++a)
	{
		TestOp(p, d, SUBTRACT, a);
		for (int b = 0; b < max; ++b)
		{
			TestOp(p, d, ADD, b);
		}
		for (int b = 0; b < max; ++b)
		{
			TestOp(p, d, SUBTRACT, b);
		}
	}
	Debug("PN Yields: %.40lf", p.ToDouble());
	Debug("Doubles Yield: %.40lf", d);
	Debug("Complete!");

}

void TestMulDivIntegers(int max=50)
{
	Debug("Test mul/div integers 1 -> %i", max);
	ParanoidNumber p(1.0);
	double d(1.0);
	for (int a = 1; a < max; ++a)
	{
		TestOp(p, d, MULTIPLY, a);
		for (int b = 1; b < max; ++b)
		{
			TestOp(p, d, DIVIDE, b);
		}
		for (int b = 1; b < max; ++b)
		{
			TestOp(p, d, MULTIPLY, b);
		}
	}
	for (int a = 1; a < max; ++a)
	{
		TestOp(p, d, DIVIDE, a);
		for (int b = 1; b < max; ++b)
		{
			TestOp(p, d, MULTIPLY, b);
		}
		for (int b = 1; b < max; ++b)
		{
			TestOp(p, d, DIVIDE, b);
		}
	}
	Debug("PN Yields: %.40lf", p.ToDouble());
	Debug("Doubles Yield: %.40lf", d);
	Debug("Complete!");

}

void TestRandomisedOps(int test_cases = 1000, int ops_per_case = 1, int max_digits = 4)
{
	Debug("Test %i*%i randomised ops (max digits = %i)", test_cases, ops_per_case, max_digits);
	long double eps = 1e-6; //* (1e4*ops_per_case);
	for (int i = 0; i < test_cases; ++i)
	{
		string s = RandomNumberAsString(max_digits);
		ParanoidNumber a(s);
		
		double da(a.ToDouble());		
		for (int j = 1; j <= ops_per_case; ++j)
		{
			double da2(a.ToDouble());
			s = RandomNumberAsString(max_digits);
			ParanoidNumber b(s);
			double db(b.ToDouble());
	
		
	
			Optype op = Optype(rand() % 4);
			
			ParanoidNumber a_before(a);
			
		
			switch (op)
			{
			case ADD:
				a += b;
				da += db;
				da2 += db;
				break;
			case SUBTRACT:
				a -= b;
				da -= db;
				da2 -= db;
				break;
			case MULTIPLY:
				a *= b;
				da *= db;
				da2 *= db;
				break;
			case DIVIDE:
				if (db == 0)
				{
					--i;
				}
				else
				{
					a /= b;
					da /= db;
					da2 /= db;
				}
				break;
			case NOP:
				break;
			}
			if (!CloseEnough(da2, a, eps))
			{
				Error("{%s} %c= {%s}", a_before.Str().c_str(), OpChar(op), b.Str().c_str());
				Error("{%s}", a.Str().c_str());
				Error("double Yields: %.40lf", da);
				Error("PN Yields: %.40lf", a.ToDouble());
				Fatal("Failed on case %i", i*ops_per_case + j-1);
			}
		}
		if (!CloseEnough(da, a, eps))
		{
			Warn("double Yields: %.40lf", da);
			Warn("PN Yields: %.40lf", a.ToDouble());
		}
	}
	Debug("Complete!");

}

#define TEST_CASES 1000

int main(int argc, char ** argv)
{
	TestAddSubIntegers();
	TestMulDivIntegers();
	for (int i = 1; i <= 100; ++i)
		TestRandomisedOps(1000, i);
	return 0;
	srand(0);//time(NULL)); //always test off same set
	string number(RandomNumberAsString());
	ParanoidNumber a(number);

	float fa = strtof(number.c_str(), NULL);
	double da = strtod(number.c_str(), NULL);
	double diff = 0;
	long double lda = strtold(number.c_str(), NULL);
	Debug("a is %s", a.Str().c_str());
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
		if (!CloseEnough(lda, a))
		{
			Error("Op %i: ParanoidNumber probably doesn't work", i);
			Error("Operation: %lf %c %lf", oldda, opch[op], db);
			Error("As PN: %lf %c %lf", olda.ToDouble(), opch[op], b.ToDouble());
			Error("PN String before: %s", olda.Str().c_str());
			Error("PN String: %s", a.Str().c_str());
			Error("LONG double gives %.40llf", lda);
			Fatal("%.40llf, expected aboout %.40llf", a.Convert<long double>(), lda);
			
			
		}
		
	}
	printf("ParanoidNumber: {%s} = %.40lf\n", a.Str().c_str(), a.ToDouble());
	printf("float: %.40f\n", fa);
	printf("double: %.40lf\n", da);
	printf("long double: %.40Lf\n", lda);
	printf("diff %.40lf\n", diff);
}
