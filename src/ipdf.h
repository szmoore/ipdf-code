#ifndef _IPDF_H
#define _IPDF_H

#include "common.h"

namespace IPDF
{
	typedef float Real;
	
	inline float RealToFloat(Real r) {return r;}

	typedef unsigned ObjectID;

	struct Rect
	{
		Real x; Real y; Real w; Real h;
		Rect(Real _x, Real _y, Real _w, Real _h) : x(_x), y(_y), w(_w), h(_h) {}
		std::string Str() 
		{
			std::stringstream s;
			s << "{" << x << ", " << y << ", " << w << ", " << h << "}";
			return s.str();
		}
	};

	struct Objects
	{
		std::vector<Rect> bounds;
	};

	class View;
}


#endif //_IPDF_H
