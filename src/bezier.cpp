#include "bezier.h"

#include <unordered_map>
#include <cmath>
#include <algorithm>

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
	return Factorial(n) / (Factorial(k) * Factorial(n-k));
}

/**
 * Bernstein Basis Polynomial
 */
Real Bernstein(int k, int n, const Real & u)
{
	return Real(BinomialCoeff(n, k)) * Power(u, k) * Power(Real(1.0) - u, n-k);
}


/**
 * Returns the parametric parameter at the turning point(s)
 * In one coordinate direction
 */

pair<Real, Real> BezierTurningPoints(const Real & p0, const Real & p1, const Real & p2, const Real & p3)
{
	// straight line
	if (p1 == p2 && p2 == p3)
	{
		return pair<Real,Real>(0, 1);
	}
	Real a = (3*(p1-p2) + p3 - p0);
	Real b = 2*(p2 - 2*p1 + p0);
	Real c = (p1-p0);
	if (a == 0)
	{
		if (b == 0)
			return pair<Real, Real>(0,1);
		Real t = -c/b;
		if (t > 1) t = 1;
		if (t < 0) t = 0;
		return pair<Real, Real>(t, t);
	}
	//Debug("a, b, c are %f, %f, %f", Float(a), Float(b), Float(c));
	if (b*b - 4*a*c < 0)
	{
		//Debug("No real roots");
		return pair<Real, Real>(0,1);
	}
	pair<Real, Real> tsols = SolveQuadratic(a, b, c);
	if (tsols.first > 1) tsols.first = 1;
	if (tsols.first < 0) tsols.first = 0;
	if (tsols.second > 1) tsols.second = 1;
	if (tsols.second < 0) tsols.second = 0;
	return tsols;
}

inline bool CompRealByPtr(const Real * a, const Real * b) 
{
	return (*a) < (*b);
}

/**
 * Get top most *point* on Bezier curve
 */
pair<Real,Real> Bezier::GetTop() const
{
	pair<Real, Real> tsols = BezierTurningPoints(y0,y1,y2,y3);
	Real tx0; Real ty0;
	Real tx1; Real ty1;
	Evaluate(tx0, ty0, tsols.first);
	Evaluate(tx1, ty1, tsols.second);
	vector<const Real*> v(4);
	v[0] = &y0;
	v[1] = &y3;
	v[2] = &ty0;
	v[3] = &ty1;
	sort(v.begin(), v.end(), CompRealByPtr);
	pair<Real,Real> result;
	result.second = *v[0];
	if (v[0] == &y0)
	{
		result.first = x0;
	}
	else if (v[0] == &y3)
	{
		result.first = x3;
	}
	else if (v[0] == &ty0)
	{
		result.first = tx0;
	}
	else if (v[0] == &ty1)
	{
		result.first = tx1;
	}
	return result;
}

/**
 * Get bottom most *point* on Bezier curve
 */
pair<Real,Real> Bezier::GetBottom() const
{
	pair<Real, Real> tsols = BezierTurningPoints(y0,y1,y2,y3);
	Real tx0; Real ty0;
	Real tx1; Real ty1;
	Evaluate(tx0, ty0, tsols.first);
	Evaluate(tx1, ty1, tsols.second);
	vector<const Real*> v(4);
	v[0] = &y0;
	v[1] = &y3;
	v[2] = &ty0;
	v[3] = &ty1;
	sort(v.begin(), v.end(), CompRealByPtr);
	pair<Real,Real> result;
	result.second = *v[3];
	if (v[3] == &y0)
	{
		result.first = x0;
	}
	else if (v[3] == &y3)
	{
		result.first = x3;
	}
	else if (v[3] == &ty0)
	{
		result.first = tx0;
	}
	else if (v[3] == &ty1)
	{
		result.first = tx1;
	}
	return result;
}

/**
 * Get left most *point* on Bezier curve
 */
pair<Real,Real> Bezier::GetLeft() const
{
	pair<Real, Real> tsols = BezierTurningPoints(x0,x1,x2,x3);
	Real tx0; Real ty0;
	Real tx1; Real ty1;
	Evaluate(tx0, ty0, tsols.first);
	Evaluate(tx1, ty1, tsols.second);
	vector<const Real*> v(4);
	v[0] = &x0;
	v[1] = &x3;
	v[2] = &tx0;
	v[3] = &tx1;
	sort(v.begin(), v.end(), CompRealByPtr);
	pair<Real,Real> result;
	result.first = *v[0];
	if (v[0] == &x0)
	{
		result.second = y0;
	}
	else if (v[0] == &x3)
	{
		result.second = y3;
	}
	else if (v[0] == &tx0)
	{
		result.second = ty0;
	}
	else if (v[0] == &tx1)
	{
		result.second = ty1;
	}
	return result;
}


/**
 * Get left most *point* on Bezier curve
 */
pair<Real,Real> Bezier::GetRight() const
{
	pair<Real, Real> tsols = BezierTurningPoints(x0,x1,x2,x3);
	Real tx0; Real ty0;
	Real tx1; Real ty1;
	Evaluate(tx0, ty0, tsols.first);
	Evaluate(tx1, ty1, tsols.second);
	vector<const Real*> v(4);
	v[0] = &x0;
	v[1] = &x3;
	v[2] = &tx0;
	v[3] = &tx1;
	sort(v.begin(), v.end(), CompRealByPtr);
	pair<Real,Real> result;
	result.first = *v[3];
	if (v[3] == &x0)
	{
		result.second = y0;
	}
	else if (v[3] == &x3)
	{
		result.second = y3;
	}
	else if (v[3] == &tx0)
	{
		result.second = ty0;
	}
	else if (v[3] == &tx1)
	{
		result.second = ty1;
	}
	return result;
}

/**
 * Get Bounds Rectangle of Bezier
 */
Rect Bezier::SolveBounds() const
{
	Rect result;
	pair<Real, Real> tsols = BezierTurningPoints(x0, x1, x2, x3);
	
	Real tp0; Real tp1; Real o;
	Evaluate(tp0, o, tsols.first);
	Evaluate(tp1, o, tsols.second);
	
	//Debug("x: tp0 is %f tp1 is %f", Float(tp0), Float(tp1));
	
	vector<const Real*> v(4);
	v[0] = &x0;
	v[1] = &x3;
	v[2] = &tp0;
	v[3] = &tp1;
	
	// Not using a lambda to keep this compiling on cabellera
	sort(v.begin(), v.end(), CompRealByPtr);

	result.x = *(v[0]);
	result.w = *(v[3]) - result.x;

	// Do the same thing for y component (wow this is a mess)
	tsols = BezierTurningPoints(y0, y1, y2, y3);
	Evaluate(o, tp0, tsols.first);
	Evaluate(o, tp1, tsols.second);
	
	
	//Debug("y: tp0 is %f tp1 is %f", Float(tp0), Float(tp1));
	
	v[0] = &y0;
	v[1] = &y3;
	v[2] = &tp0;
	v[3] = &tp1;
	sort(v.begin(), v.end(), CompRealByPtr);
	
	result.y = *(v[0]);
	result.h = *(v[3]) - result.y;
	
	//Debug("Solved Bezier %s bounds as %s", Str().c_str(), result.Str().c_str());
	return result;
}

} // end namespace
