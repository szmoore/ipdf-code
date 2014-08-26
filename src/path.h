#ifndef _PATH_H
#define _PATH_H

#include <vector>
#include <algorithm>
#include "rect.h"
#include "real.h"

namespace IPDF
{
	
	struct Colour
	{
		float r; float g; float b; float a;
		Colour() = default;
		Colour(float _r, float _g, float _b, float _a) : r(_r), g(_g), b(_b), a(_a) {}
		bool operator==(const Colour & c) const
		{
			return (r == c.r && g == c.g && b == c.b && a == c.a);
		}
		bool operator!=(const Colour & c) const {return !this->operator==(c);}
	};
	
	class Objects;
	
	struct Path
	{
		Path(const Objects & objects, unsigned _start, unsigned _end, const Colour & _fill = Colour(0.8,0.8,0.8,1));
		
		Rect SolveBounds(const Objects & objects) const;
		
		
		
		unsigned m_start; // First bounding Bezier index
		unsigned m_end; // Last (inclusive) '' ''
		unsigned m_index; // index into Objects array
		
		std::pair<Real,Real> m_top;
		std::pair<Real,Real> m_bottom;
		std::pair<Real,Real> m_left;
		std::pair<Real,Real> m_right;
		
		Colour m_fill;	// colour to fill with	
	};

}
#endif //_PATH_H
