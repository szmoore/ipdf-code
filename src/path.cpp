#include "ipdf.h"
#include "path.h"
using namespace std;

namespace IPDF
{

Path::Path(const Objects & objects, unsigned start, unsigned end, const Colour & fill, const Colour & stroke)
	: m_start(start), m_end(end), m_fill(fill), m_stroke(stroke)
{
	Real xmin = 0; Real ymin = 0; 
	Real xmax = 0; Real ymax = 0;
	
	// Find the bounds coordinates
	//  and identify the top left and bottom right objects
	
	unsigned left;
	unsigned right;
	unsigned top;
	unsigned bottom;
	
	for (unsigned i = m_start; i <= m_end; ++i)
	{
		const Rect & objb = objects.bounds[i];
		
		if (i == m_start || objb.x < xmin)
		{
			xmin = objb.x;
			left = i;
		}
		if (i == m_start || (objb.x+objb.w) > xmax)
		{
			xmax = (objb.x+objb.w);
			right = i;
		}
			
		if (i == m_start || objb.y < ymin)
		{
			ymin = objb.y;
			top = i;
		}
		if (i == m_start || (objb.y+objb.h) > ymax)
		{
			ymax = (objb.y+objb.h);
			bottom = i;
		}
		
		// find fill points
		Vec2 pt;
		// left
		pt = Vec2(objb.x, objb.y+objb.h/Real(2));
		if (PointInside(objects, pt))
			m_fill_points.push_back(pt);
		// right
		pt = Vec2(objb.x+objb.w, objb.y+objb.h/Real(2));
		if (PointInside(objects, pt))
			m_fill_points.push_back(pt);
		// bottom
		pt = Vec2(objb.x+objb.w/Real(2), objb.y+objb.h);
		if (PointInside(objects, pt))
			m_fill_points.push_back(pt);
		// top
		pt = Vec2(objb.x+objb.w/Real(2), objb.y);
		if (PointInside(objects, pt))
			m_fill_points.push_back(pt);
			
		// topleft
		pt = Vec2(objb.x, objb.y);
		if (PointInside(objects, pt))
			m_fill_points.push_back(pt);
		// topright
		pt = Vec2(objb.x+objb.w, objb.y);
		if (PointInside(objects, pt))
			m_fill_points.push_back(pt);
		// bottom left
		pt = Vec2(objb.x, objb.y+objb.h);
		if (PointInside(objects, pt))
			m_fill_points.push_back(pt);
		// bottom right
		pt = Vec2(objb.x+objb.w, objb.y);
		if (PointInside(objects, pt))
			m_fill_points.push_back(pt);
			
		// mid
		pt = Vec2(objb.x+objb.w/Real(2), objb.y+objb.h/Real(2));
		if (PointInside(objects, pt))
			m_fill_points.push_back(pt);
		
		
	}
	
	// Get actual turning point coords of the 4 edge case beziers
	m_top = objects.beziers[objects.data_indices[top]].ToAbsolute(objects.bounds[top]).GetTop();
	m_bottom = objects.beziers[objects.data_indices[bottom]].ToAbsolute(objects.bounds[bottom]).GetBottom();
	m_left = objects.beziers[objects.data_indices[left]].ToAbsolute(objects.bounds[left]).GetLeft();
	m_right = objects.beziers[objects.data_indices[right]].ToAbsolute(objects.bounds[right]).GetRight();
	
	Vec2 pt = (m_top + m_bottom)/2;
	if (PointInside(objects, pt))
		m_fill_points.push_back(pt);
	pt = (m_left + m_right)/2;
	if (PointInside(objects, pt))
		m_fill_points.push_back(pt);
	pt = (m_left + m_right + m_top + m_bottom)/4;
	if (PointInside(objects, pt))
		m_fill_points.push_back(pt);
		
}


bool Path::PointInside(const Objects & objects, const Vec2 & pt, bool debug) const
{
	vector<Vec2> x_ints;
	vector<Vec2> y_ints;
	for (unsigned i = m_start; i <= m_end; ++i)
	{
		Bezier bez(objects.beziers[objects.data_indices[i]].ToAbsolute(objects.bounds[i]));
		vector<Vec2> xi(bez.SolveX(pt.x));
		vector<Vec2> yi(bez.SolveY(pt.y));
		x_ints.insert(x_ints.end(), xi.begin(), xi.end());
		y_ints.insert(y_ints.end(), yi.begin(), yi.end());
	}
	//Debug("Solved for intersections");
	unsigned bigger = 0;
	unsigned smaller = 0;
	for (unsigned i = 0; i < x_ints.size(); ++i)
	{
		if (debug)
				Debug("X Intersection %u at %f,%f vs %f,%f", i,x_ints[i].x, x_ints[i].y, pt.x, pt.y);
		if (x_ints[i].y >= pt.y)
		{
			
			++bigger;
		}
	}
	smaller = x_ints.size() - bigger;
	if (debug)
	{
		Debug("%u horizontal, %u bigger, %u smaller", x_ints.size(), bigger, smaller);
	}
	if (smaller % 2 == 0 || bigger % 2 == 0)
		return false;
		
	bigger = 0;
	smaller = 0;

	for (unsigned i = 0; i < y_ints.size(); ++i)
	{
		if (debug)
				Debug("Y Intersection %u at %f,%f vs %f,%f", i,x_ints[i].x, x_ints[i].y, pt.x, pt.y);
		if (y_ints[i].x >= pt.x)
		{
			
			++bigger;
		}
	}
	smaller = y_ints.size() - bigger;
	if (debug)
	{
		Debug("%u vertical, %u bigger, %u smaller", y_ints.size(), bigger, smaller);
	}
	if (smaller % 2 == 0 || bigger % 2 == 0)
		return false;
		
		
	return true;
}

Rect Path::SolveBounds(const Objects & objects) const
{
		return Rect(m_left.x, m_top.y, m_right.x-m_left.x, m_bottom.y-m_top.y);
}

}
