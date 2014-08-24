#ifndef _IPDF_H
#define _IPDF_H

#include "common.h"
#include "real.h"
#include "bezier.h"
#include "rect.h"

#define C_RED Colour(1,0,0,1)
#define C_GREEN Colour(0,1,0,1)
#define C_BLUE Colour(0,0,1,1)
#define C_BLACK Colour(0,0,0,1);

namespace IPDF
{

	inline Real Random(Real max=1, Real min=0)
	{
		return min + (max-min) * (Real(rand() % (int)1e6) / Real(1e6));
	}

	typedef unsigned ObjectID;
	/** Type of object
     * NOTE: Extra entry in the enum so we can use this as an array index
	 */
	typedef enum 
	{
		CIRCLE_FILLED = 0, 
		RECT_FILLED,
		RECT_OUTLINE,
		BEZIER,
		GROUP,
		NUMBER_OF_OBJECT_TYPES
	} ObjectType;

	enum DocChunkTypes
	{
		CT_NUMOBJS,
		CT_OBJTYPES,
		CT_OBJBOUNDS,
		CT_OBJINDICES,
		CT_OBJBEZIERS,
		CT_OBJGROUPS
	};



	
	
	struct Colour
	{
		float r; float g; float b; float a;
		Colour() = default;
		Colour(float _r, float _g, float _b, float _a) : r(_r), g(_g), b(_b), a(_a) {}
	};
	
	struct Group
	{
		unsigned start;
		unsigned end;
		Colour shading;
	};

	struct Objects
	{
		/** Used by all objects **/
		std::vector<ObjectType> types; // types of objects
		std::vector<Rect> bounds; // rectangle bounds of objects
		/** Used by BEZIER and GROUP to identify data position in relevant vector **/
		std::vector<unsigned> data_indices;
		/** Used by BEZIER only **/
		std::vector<Bezier> beziers; // bezier curves - look up by data_indices
		/** Used by GROUP only **/
		std::vector<Group> groups;
	};

	class View;
}


#endif //_IPDF_H
