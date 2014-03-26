#ifndef _IPDF_H
#define _IPDF_H

#include "common.h"

namespace IPDF
{
	typedef float Real;
	
	inline float RealToFloat(Real r) {return r;}
	inline Real Random(Real max=1, Real min=0)
	{
		return min + (max-min) * ((Real)(rand() % (int)1e6) / 1e6);
	}

	typedef unsigned ObjectID;
	typedef enum {RECT_FILLED, RECT_OUTLINE} ObjectType;

	struct Rect
	{
		Real x; Real y; Real w; Real h;
		Rect() = default; // Needed so we can fread/fwrite this struct
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
		std::vector<ObjectType> types;		
		std::vector<Rect> bounds;
	};

	class View;
}


#endif //_IPDF_H
