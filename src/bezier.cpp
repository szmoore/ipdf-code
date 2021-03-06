#include "bezier.h"

#include <unordered_map>
#include <cmath>
#include <algorithm>



using namespace std;

namespace IPDF
{

vector<BReal> SolveQuadratic(const BReal & a, const BReal & b, const BReal & c, const BReal & min, const BReal & max)
{
	vector<BReal> roots; roots.reserve(2);
	if (a == BReal(0) && b != BReal(0))
	{
		roots.push_back(-c/b);
		return roots;
	}
	BReal disc(b*b - BReal(4)*a*c);
	if (disc < BReal(0))
	{
		return roots;
	}
	else if (disc == BReal(0))
	{
		BReal x(-b/BReal(2)*a);
		if (x >= min && x <= max)
			roots.push_back(x);
		return roots;
	}
	
	BReal x0((-b - Sqrt(b*b - BReal(4)*a*c))/(BReal(2)*a));
	BReal x1((-b + Sqrt(b*b - BReal(4)*a*c))/(BReal(2)*a));
	if (x0 > x1)
	{
		BReal tmp(x0);
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

static void CubicSolveSegment(vector<BReal> & roots, const BReal & a, const BReal & b, const BReal & c, const BReal & d, BReal & tl, BReal & tu, const BReal & delta)
{
	BReal l = a*tl*tl*tl + b*tl*tl + c*tl + d;
	BReal u = a*tu*tu*tu + b*tu*tu + c*tu + d;
	if ((l < BReal(0) && u < BReal(0)) || (l > BReal(0) && u > BReal(0)))
	{
		//Debug("Discarding segment (no roots) l = %f (%f), u = %f (%f)", Double(tl), Double(l), Double(tu), Double(u));
		//return;
	}
	
	bool negative = (u < l); // lower point > 0, upper point < 0
	//Debug("%ft^3 + %ft^2 + %ft + %f is negative (%f < %f) %d", Double(a),Double(b),Double(c),Double(d),Double(u),Double(l), negative);
	while (tu - tl > delta)
	{
		BReal t(tu+tl);
		t /= 2;
		BReal m = a*t*t*t + b*t*t + c*t + d;
		if (m > BReal(0))
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
vector<BReal> SolveCubic(const BReal & a, const BReal & b, const BReal & c, const BReal & d, const BReal & min, const BReal & max, const BReal & delta)
{
	vector<BReal> roots; roots.reserve(3);
	BReal tu(max);
	BReal tl(min);
	vector<BReal> turns(SolveQuadratic(a*BReal(3), b*BReal(2), c));
	//Debug("%u turning points", turns.size());
	for (unsigned i = 1; i < turns.size(); ++i)
	{
		if (tl > max) break;
		tu = std::min(turns[i],tu);
		CubicSolveSegment(roots, a, b, c, d, tl, tu,delta);
		tl = turns[i];
	}
	if (tu < max)
	{
		tu = max;
		CubicSolveSegment(roots, a, b, c, d, tl, tu,delta);
	}
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
BReal Bernstein(int k, int n, const BReal & u)
{
	return BReal(BinomialCoeff(n, k)) * Power(u, k) * Power(BReal(1.0) - u, n-k);
}


/**
 * Returns the parametric parameter at the turning point(s)
 * In one coordinate direction
 */

pair<BReal, BReal> BezierTurningPoints(const BReal & p0, const BReal & p1, const BReal & p2, const BReal & p3)
{
	// straight line
	if (p1 == p2 && p2 == p3)
	{
		return pair<BReal,BReal>(0, 1);
	}
	BReal a = ((p1-p2)*BReal(3) + p3 - p0);
	BReal b = (p2 - p1*BReal(2) + p0)*BReal(2);
	BReal c = (p1-p0);
	if (a == BReal(0))
	{
		if (b == BReal(0))
			return pair<BReal, BReal>(0,1);
		BReal t = -c/b;
		if (t > BReal(1)) t = 1;
		if (t < BReal(0)) t = 0;
		return pair<BReal, BReal>(t, t);
	}
	//Debug("a, b, c are %f, %f, %f", Float(a), Float(b), Float(c));
	if (b*b - a*c*BReal(4) < BReal(0))
	{
		//Debug("No real roots");
		return pair<BReal, BReal>(0,1);
	}
	vector<BReal> tsols = SolveQuadratic(a, b, c);
	if (tsols.size() == 1)
		return pair<BReal,BReal>(tsols[0], tsols[0]);
	else if (tsols.size() == 0)
		return pair<BReal, BReal>(0,1);
	
	return pair<BReal,BReal>(tsols[0], tsols[1]);
	
}

inline bool CompBRealByPtr(const BReal * a, const BReal * b) 
{
	return (*a) < (*b);
}

/**
 * Get top most *point* on Bezier curve
 */
pair<BReal,BReal> Bezier::GetTop() const
{
	pair<BReal, BReal> tsols = BezierTurningPoints(y0,y1,y2,y3);
	BReal tx0; BReal ty0;
	BReal tx1; BReal ty1;
	Evaluate(tx0, ty0, tsols.first);
	Evaluate(tx1, ty1, tsols.second);
	vector<const BReal*> v(4);
	v[0] = &y0;
	v[1] = &y3;
	v[2] = &ty0;
	v[3] = &ty1;
	sort(v.begin(), v.end(), CompBRealByPtr);
	pair<BReal,BReal> result;
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
pair<BReal,BReal> Bezier::GetBottom() const
{
	pair<BReal, BReal> tsols = BezierTurningPoints(y0,y1,y2,y3);
	BReal tx0; BReal ty0;
	BReal tx1; BReal ty1;
	Evaluate(tx0, ty0, tsols.first);
	Evaluate(tx1, ty1, tsols.second);
	vector<const BReal*> v(4);
	v[0] = &y0;
	v[1] = &y3;
	v[2] = &ty0;
	v[3] = &ty1;
	sort(v.begin(), v.end(), CompBRealByPtr);
	pair<BReal,BReal> result;
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
pair<BReal,BReal> Bezier::GetLeft() const
{
	pair<BReal, BReal> tsols = BezierTurningPoints(x0,x1,x2,x3);
	BReal tx0; BReal ty0;
	BReal tx1; BReal ty1;
	Evaluate(tx0, ty0, tsols.first);
	Evaluate(tx1, ty1, tsols.second);
	vector<const BReal*> v(4);
	v[0] = &x0;
	v[1] = &x3;
	v[2] = &tx0;
	v[3] = &tx1;
	sort(v.begin(), v.end(), CompBRealByPtr);
	pair<BReal,BReal> result;
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
pair<BReal,BReal> Bezier::GetRight() const
{
	pair<BReal, BReal> tsols = BezierTurningPoints(x0,x1,x2,x3);
	BReal tx0; BReal ty0;
	BReal tx1; BReal ty1;
	Evaluate(tx0, ty0, tsols.first);
	Evaluate(tx1, ty1, tsols.second);
	vector<const BReal*> v(4);
	v[0] = &x0;
	v[1] = &x3;
	v[2] = &tx0;
	v[3] = &tx1;
	sort(v.begin(), v.end(), CompBRealByPtr);
	pair<BReal,BReal> result;
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

vector<BReal> Bezier::SolveXParam(const BReal & x) const
{
	BReal d(x0 - x);
	BReal c((x1 - x0)*BReal(3));
	BReal b((x2 - x1)*BReal(3) - c);
	BReal a(x3 -x0 - c - b);
	vector<BReal> results(SolveCubic(a, b, c, d));
	for (unsigned i = 0; i < results.size(); ++i)
	{
		Vec2 p;
		Evaluate(p.x, p.y, results[i]);
	}
	return results;
}


vector<BReal> Bezier::SolveYParam(const BReal & y) const
{
	BReal d(y0 - y);
	BReal c((y1 - y0)*BReal(3));
	BReal b((y2 - y1)*BReal(3) - c);
	BReal a(y3 -y0 - c - b);
	vector<BReal> results(SolveCubic(a, b, c, d));
	for (unsigned i = 0; i < results.size(); ++i)
	{
		Vec2 p;
		Evaluate(p.x, p.y, results[i]);
	}
	return results;
}

vector<Vec2> Bezier::Evaluate(const vector<BReal> & u) const
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
BRect Bezier::SolveBounds() const
{
	BRect result;
	pair<BReal, BReal> tsols = BezierTurningPoints(x0, x1, x2, x3);
	
	BReal tp0; BReal tp1; BReal o;
	Evaluate(tp0, o, tsols.first);
	Evaluate(tp1, o, tsols.second);
	
	//Debug("x: tp0 is %f tp1 is %f", Float(tp0), Float(tp1));
	
	vector<const BReal*> v(4);
	v[0] = &x0;
	v[1] = &x3;
	v[2] = &tp0;
	v[3] = &tp1;
	
	// Not using a lambda to keep this compiling on cabellera
	sort(v.begin(), v.end(), CompBRealByPtr);

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
	sort(v.begin(), v.end(), CompBRealByPtr);
	
	result.y = *(v[0]);
	result.h = *(v[3]) - result.y;
	
	//Debug("Solved Bezier %s bounds as %s", Str().c_str(), result.Str().c_str());
	return result;
}

} // end namespace

