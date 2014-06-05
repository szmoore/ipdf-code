#ifndef _IPDF_H
#define _IPDF_H

#include "common.h"
#include "real.h"

#define C_RED Colour(1,0,0,1)
#define C_GREEN Colour(0,1,0,1)
#define C_BLUE Colour(0,0,1,1)

namespace IPDF
{

	inline Real Random(Real max=1, Real min=0)
	{
		return min + (max-min) * (Real(rand() % (int)1e6) / Real(1e6));
	}

	typedef unsigned ObjectID;
	typedef enum {RECT_FILLED, RECT_OUTLINE, CIRCLE_FILLED} ObjectType;

	enum DocChunkTypes
	{
		CT_NUMOBJS,
		CT_OBJTYPES,
		CT_OBJBOUNDS
	};

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

	struct Colour
	{
		float r; float g; float b; float a;
		Colour() = default;
		Colour(float _r, float _g, float _b, float _a) : r(_r), g(_g), b(_b), a(_a) {}
	};

	struct Objects
	{
		std::vector<ObjectType> types;		
		std::vector<Rect> bounds;
	};

	class View;
}


#endif //_IPDF_H
