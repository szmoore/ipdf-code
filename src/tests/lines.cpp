#include "main.h"

#include "pointstobitmap.h"

using namespace std;

/**
 * NOTE: I am using vector<pair<T,T> > because it is convenient; don't have to round to pixels, can print to stdout and gnuplot, etc
 * 		vector<pair<T,T> > should obviously not be used in a practical implementation; directly access the pixel buffer
 */

template <class T> vector<pair<T, T> > LineDDA(const T & x1, const T & y1, const T & x2, const T & y2)
{
	T dx = x2 - x1;
	T dy = y2 - y1;
	uint64_t steps = lround(max(abs(dx), abs(dy)));
	T xInc = T(dx)/T(steps);
	T yInc = T(dy)/T(steps);
	T x = x1; T y = y1;
	vector<pair<T,T> > result(steps);
	for (uint64_t k = 0; k <= steps; ++k)
	{
		result[k] = pair<T,T>(round(x), round(y));
		x += xInc;
		y += yInc;
	}
	return result;
}
/**
 * Only works for 0 < m < 1
 */
template <class T> vector<pair<T, T> > LineBresenham(const T & x1, const T & y1, const T & x2, const T & y2)
{
	T dx = abs(x2 - x1);
	T dy = abs(y2 - y1);
	T p = 2*dy - dx;
	T two_dy = 2*dy;
	T two_dxdy = 2*(dy-dx);

	T x; T y; T xEnd;
	if (x1 > x2)
	{
		x = x2;
		y = y2;
		xEnd = x1;
	}
	else
	{
		x = x1;
		y = y1;
		xEnd = x2;
	}
	vector<pair<T,T> > result;
	while (x < xEnd)
	{
		x += 1;
		if (p < 0)
			p += two_dy;
		else
		{
			y += 1;
			p += two_dxdy;
		}
		result.emplace_back(pair<T,T>(x,y));
	}
	return result;
}

int main(int argc, char ** argv)
{
	auto f = LineDDA<float>(0,0,400,200);
	PointsToBitmap<float>(f, 1, 0,0,0,"lines.bmp");
	auto i = LineBresenham<int>(0,100,400,300);
	i.push_back(pair<int,int>(0,0));
	PointsToBitmap<int>(i, 1, 255,0,0,"lines.bmp", true);
}
