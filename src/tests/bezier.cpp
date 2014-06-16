/**
 * Tester
 * Bezier Curves - Compute them and put on Bitmaps
 */
#include "main.h"
#include <cmath>
#include <ctime> // for performance measurements
#include <unordered_map> // hashtable
#include <map>
#include "screen.h"

#include "pointstobitmap.h"

using namespace std;
using namespace IPDF;

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
template <class T> T Bernstein(int k, int n, const T & u)
{
	return T(BinomialCoeff(n, k)) * pow(u, k) * pow(T(1.0) - u, n-k);
}

/**
 * Evaluate a Bezier at a single point u
 */
template <class T> pair<T, T> Bezier(const vector<pair<T, T> > & control, const T & u)
{
	pair<T,T> result(0,0);
	for (size_t k = 0; k < control.size(); ++k)
	{
		T bez(Bernstein<T>(k, control.size()-1, u));
		result.first += control[k].first * bez;
		result.second += control[k].second * bez;
	}
	return result;
}

/**
 * Form a Bezier curve from the control points using the specified number of intervals
 */
template <class T> vector<pair<T, T> > BezierCurve(const vector<pair<T, T> > & control, int intervals = 200)
{
	vector<pair<T, T> > result(intervals+1);
	T dU = T(1)/intervals;
	T u = 0;
	for (int i = 0; i <= intervals; ++i)
	{
		result[i] = Bezier<T>(control, u);
		u += dU;
	}
	return result;
}



/**
 * Make a circle using Beziers
 * Actually just makes one Bezier for a quadrant and then mirrors it for the others
 */
template <class T> vector<pair<T, T> > BezierCircle(const T & scale = 1, int intervals = 50)
{
	T k = T(4) * (pow(T(2), T(0.5)) - T(1)) / T(3); // k = 4/3 * (2^(1/2) - 1) because maths says so
	vector<pair<T, T> >control(4);
	control[0] = pair<T,T>(0,scale);
	control[1] = pair<T,T>(k*scale, scale);
	control[2] = pair<T,T>(scale, k*scale);
	control[3] = pair<T,T>(scale, 0);

	auto points = BezierCurve<T>(control, intervals);
	
	points.reserve(intervals*4);
	for (int i = 0; i < intervals; ++i)
	{
		points.push_back(pair<T,T>(-points[i].first, points[i].second));
	}
	for (int i = 0; i < intervals; ++i)
	{
		points.push_back(pair<T,T>(points[i].first, -points[i].second));
	}
	for (int i = 0; i < intervals; ++i)
	{
		points.push_back(pair<T,T>(-points[i].first, -points[i].second));
	}
	
	return points;
}

/**
 * Test Bezier Curve with a scale applied to the *control* points
 */
template <class T> vector<pair<T,T> > TestCurve(const T & scale = 1, long intervals = 50)
{
	vector<pair<T,T> > control(3);
	control[0] = pair<T,T>(0,0);
	control[1] = pair<T,T>(0.4*scale, 0.8*scale);
	control[2] = pair<T,T>(scale,scale);
	return BezierCurve<T>(control, intervals);
}


/**
 * Test the Beziers over a range of scales. Can print the Nth bezier to stdout
 */
void TestBeziers(long intervals = 50, long double start = 1e-38, long double end = 1e38, int print = -1)
{
	int count = 0;
	// Choose our Bezier here
	//#define TESTBEZIER TestCurve
	#define TESTBEZIER BezierCircle

	for (auto scale = start; scale < end; scale *= 10)
	{
		auto f = TESTBEZIER<float>(scale, intervals);
		auto d = TESTBEZIER<double>(scale, intervals);
		auto l = TESTBEZIER<long double>(scale, intervals);

		// Make bitmap(s)
		stringstream s;
		s << "bezier" << count << "_" << scale << ".bmp";
		PointsToBitmap<float>(f, scale, 5*(count++),0,0,s.str().c_str(),false);

		#if REAL > REAL_LONG_DOUBLE
			auto r = TESTBEZIER<Real>(scale, intervals);
		#endif
		if (count++ == print)
		{
			for (auto i = 0; i < f.size(); ++i)
			{
				printf("%d\t%.50lf\t%.50lf\t%.50llf\t%.50llf", f[i].first, f[i].second, d[i].first, d[i].second, l[i].first, l[i].second);
				#if REAL > REAL_LONG_DOUBLE
					printf("\t%.50lf\t%.50lf", r[i].first, r[i].second);
				#endif 	
				printf("\n");
			}
		}

		// optional: Overlay images instead
	}
}

/**
 * main
 */
int main(int argc, char ** argv)
{	
	TestBeziers(200);
	// Makes a movie, not really that exciting though
	//system("for i in *.bmp; do convert $i -filter Point -resize x600 +antialias $i; done");
	//system("ffmpeg -i bezier%d.bmp -framerate 2 -vcodec mpeg4 bezier.mp4");
}

