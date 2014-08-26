#include "ipdf.h"
#include "path.h"
using namespace std;

namespace IPDF
{

Path::Path(const Objects & objects, unsigned start, unsigned end, const Colour & fill)
	: m_start(start), m_end(end), m_fill(fill)
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
	}
	
	// Get actual turning point coords of the 4 edge case beziers
	m_top = objects.beziers[objects.data_indices[top]].ToAbsolute(objects.bounds[top]).GetTop();
	m_bottom = objects.beziers[objects.data_indices[bottom]].ToAbsolute(objects.bounds[bottom]).GetBottom();
	m_left = objects.beziers[objects.data_indices[left]].ToAbsolute(objects.bounds[left]).GetLeft();
	m_right = objects.beziers[objects.data_indices[right]].ToAbsolute(objects.bounds[right]).GetRight();
	/*Debug("Top: %f, %f", m_top.first, m_top.second);
	Debug("Bottom: %f, %f", m_bottom.first, m_bottom.second);
	Debug("Left: %f, %f", m_left.first, m_left.second);
	Debug("Right: %f, %f", m_right.first, m_right.second);
	Debug("Left - Right: %f, %f", m_right.first - m_left.first, m_right.second - m_left.second);
	Debug("Top - Bottom: %f, %f", m_top.first - m_bottom.first, m_top.second - m_bottom.second);
	*/
}

Rect Path::SolveBounds(const Objects & objects) const
{
		return Rect(m_left.first, m_top.second, m_right.first-m_left.first, m_bottom.second-m_top.second);
}

}
