#ifndef QUADTREE_REMOVED
#include "quadtree.h"

namespace IPDF {

Rect TransformToQuadChild(const Rect& src, QuadTreeNodeChildren child_type)
{
	Rect dst = src;
	dst.x *= 2;
	dst.y *= 2;
	dst.w *= 2;
	dst.h *= 2;
	if (child_type == QTC_BOTTOM_LEFT || child_type == QTC_BOTTOM_RIGHT)
	{
		dst.y -= 1;
	}
	if (child_type == QTC_TOP_RIGHT || child_type == QTC_BOTTOM_RIGHT)
	{
		dst.x -= 1;
	}
	return dst;
}

Rect TransformFromQuadChild(const Rect& src, QuadTreeNodeChildren child_type)
{
	Rect dst = src;
	if (child_type == QTC_BOTTOM_LEFT || child_type == QTC_BOTTOM_RIGHT)
	{
		dst.y += 1;
	}
	if (child_type == QTC_TOP_RIGHT || child_type == QTC_BOTTOM_RIGHT)
	{
		dst.x += 1;
	}
	dst.x *= 0.5;
	dst.y *= 0.5;
	dst.w *= 0.5;
	dst.h *= 0.5;
	return dst;
}

bool IntersectsQuadChild(const Rect& src, QuadTreeNodeChildren child_type)
{
	Rect std = {0,0,1,1};
	Rect dst = TransformFromQuadChild(std, child_type);
	if (src.x + src.w < dst.x) return false;
	if (src.y + src.h < dst.y) return false;
	if (src.x > dst.x + dst.w) return false;
	if (src.y > dst.y + dst.h) return false;
	Debug("%s is contained in %s\n", src.Str().c_str(), dst.Str().c_str());
	return true;
}

bool ContainedInQuadChild(const Rect& src, QuadTreeNodeChildren child_type)
{
	Rect std = {0,0,1,1};
	Rect dst = TransformFromQuadChild(std, child_type);
	if (src.x < dst.x) return false;
	if (src.y < dst.y) return false;
	if (src.x + src.w > dst.x + dst.w) return false;
	if (src.y + src.h > dst.y + dst.h) return false;
	Debug("%s is contained in %s... \n", src.Str().c_str(), dst.Str().c_str());
	return true;
}

}

#endif
