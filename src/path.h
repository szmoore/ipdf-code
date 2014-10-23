#ifndef _PATH_H
#define _PATH_H

#include "transformationtype.h"
#include <vector>
#include <algorithm>
#include "rect.h"
#include "real.h"

#ifdef TRANSFORM_BEZIERS_TO_PATH
	#include "gmprat.h"
	#include "paranoidnumber.h"
#endif


namespace IPDF
{
	typedef TRect<PReal> PRect;
	
	struct Colour
	{
		uint8_t r; uint8_t g; uint8_t b; uint8_t a;
		Colour() = default;
		Colour(uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _a) : r(_r), g(_g), b(_b), a(_a) {}
		bool operator==(const Colour & c) const
		{
			return (r == c.r && g == c.g && b == c.b && a == c.a);
		}
		bool operator!=(const Colour & c) const {return !this->operator==(c);}
	};
	
	class Objects;
	class View;
	
	struct Path
	{
		Path(Objects & objects, unsigned _start, unsigned _end, const Colour & _fill = Colour(128,128,128,255), const Colour & _stroke = Colour(0,0,0,0));
		
		Rect SolveBounds(const Objects & objects);
		Rect & GetBounds(Objects & objects);
		std::vector<Vec2> & FillPoints(const Objects & objects, const View & view);
		
		// Is point inside shape?
		bool PointInside(const Objects & objects, const Vec2 & pt, bool debug=false) const;
		
		unsigned m_start; // First bounding Bezier index
		unsigned m_end; // Last (inclusive) '' ''
		unsigned m_index; // index into Objects array
		
		Vec2 m_top;
		Vec2 m_bottom;
		Vec2 m_left;
		Vec2 m_right;
		
		std::vector<Vec2> m_fill_points;
		
		PRect m_bounds;

		
		Colour m_fill;	// colour to fill with	
		Colour m_stroke; // colour to outline with
	};

}
#endif //_PATH_H
