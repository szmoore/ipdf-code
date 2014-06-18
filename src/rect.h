#ifndef _RECT_H
#define _RECT_H

#include "common.h"
#include "real.h"

namespace IPDF
{
	struct Rect
	{
		Real x; Real y; Real w; Real h;
		Rect() = default; // Needed so we can fread/fwrite this struct
		Rect(Real _x, Real _y, Real _w, Real _h) : x(_x), y(_y), w(_w), h(_h) {}
		std::string Str() const
		{
			std::stringstream s;
			// float conversion needed because it is fucking impossible to get ostreams working with template classes
			s << "{" << Float(x) << ", " << Float(y) << ", " << Float(w) << ", " << Float(h) << "}";
			return s.str();
		}
	};
}

#endif //_RECT_H
