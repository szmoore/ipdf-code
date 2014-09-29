#ifndef _RECT_H
#define _RECT_H

#include "common.h"
#include "real.h"

namespace IPDF
{
	struct Rect
	{
		Real x; Real y; Real w; Real h;
		//Rect() = default; // Needed so we can fread/fwrite this struct
		Rect(Real _x=0, Real _y=0, Real _w=1, Real _h=1) : x(_x), y(_y), w(_w), h(_h) {}
		std::string Str() const
		{
			std::stringstream s;
			// float conversion needed because it is fucking impossible to get ostreams working with template classes
			s << "{" << Float(x) << ", " << Float(y) << ", " << Float(x + w) << ", " << Float(y + h) << " (w: " << Float(w) <<", h: " << Float(h) <<")}";
			return s.str();
		}
		inline bool PointIn(Real pt_x, Real pt_y) const
		{
			if (pt_x <= x) return false;
			if (pt_y <= y) return false;
			if (pt_x >= x + w) return false;
			if (pt_y >= y + h) return false;
			return true;
		}

		inline bool Intersects(const Rect& other) const
		{
			if (x + w < other.x) return false;
			if (y + h < other.y) return false;
			if (x > other.x + other.w) return false;
			if (y > other.y + other.h) return false;
			return true;
		}
	};

	inline Rect TransformRectCoordinates(const Rect& view, const Rect& r)
	{
		Rect out;
		Real w = (view.w == Real(0))?Real(1):view.w;
		Real h = (view.h == Real(0))?Real(1):view.h;
		out.x = (r.x - view.x) / w; //r.x = out.x *w + view.x
		out.y = (r.y - view.y) / h; // r.y = out.y*h + view.y
		out.w = r.w / w; // r.w = out.w * w
		out.h = r.h / h; // r.h = out.h * h
		return out;
	}

	inline Vec2 TransformPointCoordinates(const Rect& view, const Vec2& v)
	{
		Vec2 out;
		out.x = (v.x - view.x) / view.w;
		out.y = (v.y - view.y) / view.h;
		return out;
	}


}

#endif //_RECT_H
