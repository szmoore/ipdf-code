#include "main.h"
#include "real.h"
#include <cmath>
#include <cassert>
#include <list>
#include <bitset>
#include <iostream>

using namespace std;
using namespace IPDF;

int main(int argc, char ** argv)
{
	while (cin.good())
	{
		double da; double db;
		char op;
		cin >> da >> op >> db;
		
		Real a(da);
		Real b(db);
		
		Real c;
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
		
		cout << Double(c) << '\n';
		
	}
}
