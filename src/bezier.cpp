#include "bezier.h"

#include <unordered_map>
#include <cmath>
#include <algorithm>

using namespace std;

namespace IPDF
{

vector<Real> SolveQuadratic(const Real & a, const Real & b, const Real & c, const Real & min, const Real & max)
{
	vector<Real> roots; roots.reserve(2);
	if (a == 0 && b != 0)
	{
		roots.push_back(-c/b);
		return roots;
	}
	Real disc(b*b - Real(4)*a*c);
	if (disc < 0)
	{
		return roots;
	}
	else if (disc == 0)
	{
		Real x(-b/Real(2)*a);
		if (x >= min && x <= max)
			roots.push_back(x);
		return roots;
	}
	
	Real x0((-b - Sqrt(b*b - Real(4)*a*c))/(Real(2)*a));
	Real x1((-b + Sqrt(b*b - Real(4)*a*c))/(Real(2)*a));
	if (x0 > x1)
	{
		Real tmp(x0);
		x0 = x1;
		x1 = tmp;
	}
	if (x0 >= min && x0 <= max)
		roots.push_back(x0);
	if (x1 >= min && x1 <= max)
		roots.push_back(x1);
	return roots;
}

/**
 * Finds the root (if it exists) in a monotonicly in(de)creasing segment of a Cubic
 */

static void CubicSolveSegment(vector<Real> & roots, const Real & a, const Real & b, const Real & c, const Real & d, Real & tl, Real & tu, const Real & delta)
{
	Real l = a*tl*tl*tl + b*tl*tl + c*tl + d;
	Real u = a*tu*tu*tu + b*tu*tu + c*tu + d;
	if ((l < 0 && u < 0) || (l > 0 && u > 0))
		Debug("Discarding segment (no roots) l = %f (%f), u = %f (%f)", Double(tl), Double(l), Double(tu), Double(u));
		//return;
	
	bool negative = (u < l); // lower point > 0, upper point < 0
	Debug("%ft^3 + %ft^2 + %ft + %f is negative (%f < %f) %d", Double(a),Double(b),Double(c),Double(d),Double(u),Double(l), negative);
	while (tu - tl > delta)
	{
		Real t(tu+tl);
		t /= 2;
		Real m = a*t*t*t + b*t*t + c*t + d;
		if (m > 0)
		{
			if (negative)
				tl = t;
			else
				tu = t;
		}
		else if (negative)
		{
			tu = t;
		}
		else
		{
			tl = t;
		}
		//Debug("Delta is %f (%f - %f -> %f)", tu-tl, tu, tl, t);
	}
	roots.push_back(tl);
}
vector<Real> SolveCubic(const Real & a, const Real & b, const Real & c, const Real & d, const Real & min, const Real & max, const Real & delta)
{
	vector<Real> roots; roots.reserve(3);
	Real tu(max);
	Real tl(min);
	vector<Real> turns(SolveQuadratic(a*3, b*2, c));
	Debug("%u turning points", turns.size());
	for (unsigned i = 1; i < turns.size(); ++i)
	{
		tu = turns[i];
		CubicSolveSegment(roots, a, b, c, d, tl, tu,delta);
		tl = turns[i];
	}
	tu = max;
	CubicSolveSegment(roots, a, b, c, d, tl, tu,delta);
	return roots;
}

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
	Real a = ((p1-p2)*3 + p3 - p0);
	Real b = (p2 - p1*2 + p0)*2;
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
	if (b*b - a*c*4 < 0)
	{
		//Debug("No real roots");
		return pair<Real, Real>(0,1);
	}
	vector<Real> tsols = SolveQuadratic(a, b, c);
	if (tsols.size() == 1)
		return pair<Real,Real>(tsols[0], tsols[0]);
	else if (tsols.size() == 0)
		return pair<Real, Real>(0,1);
	
	return pair<Real,Real>(tsols[0], tsols[1]);
	
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

vector<Real> Bezier::SolveXParam(const Real & x) const
{
	Real d(x0 - x);
	Real c((x1 - x0)*Real(3));
	Real b((x2 - x1)*Real(3) - c);
	Real a(x3 -x0 - c - b);
	vector<Real> results(SolveCubic(a, b, c, d));
	for (unsigned i = 0; i < results.size(); ++i)
	{
		Vec2 p;
		Evaluate(p.x, p.y, results[i]);
	}
	return results;
}


vector<Real> Bezier::SolveYParam(const Real & y) const
{
	Real d(y0 - y);
	Real c((y1 - y0)*Real(3));
	Real b((y2 - y1)*Real(3) - c);
	Real a(y3 -y0 - c - b);
	vector<Real> results(SolveCubic(a, b, c, d));
	for (unsigned i = 0; i < results.size(); ++i)
	{
		Vec2 p;
		Evaluate(p.x, p.y, results[i]);
	}
	return results;
}

vector<Vec2> Bezier::Evaluate(const vector<Real> & u) const
{
	vector<Vec2> result(u.size());
	for (unsigned i = 0; i < u.size(); ++i)
	{
		Evaluate(result[i].x, result[i].y, u[i]);
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
