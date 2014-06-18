#include "bezier.h"

#include <unordered_map>
#include <cmath>

using namespace std;

namespace IPDF
{

/**
 * Factorial
 * Use dynamic programming / recursion
 */
int Factorial(int n)
{
	static unordered_map<int, int> dp;
	static bool init = false;
	if (!init)
	{
		init = true;
		dp[0] = 1;
	}
	auto it = dp.find(n);
	if (it != dp.end())
		return it->second;
	int result = n*Factorial(n-1);
	dp[n] = result;
	return result;
}

/**
 * Binomial coefficients
 */
int BinomialCoeff(int n, int k)
{
	return Factorial(n) / Factorial(k) / Factorial(n-k);
}

/**
 * Bernstein Basis Polynomial
 */
Real Bernstein(int k, int n, const Real & u)
{
	return Real(BinomialCoeff(n, k)) * pow(u, k) * pow(Real(1.0) - u, n-k);
}

}
