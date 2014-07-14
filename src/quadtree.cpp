#ifndef QUADTREE_REMOVED
#include "quadtree.h"

namespace IPDF {

Rect TransformToQuadChild(const Rect& src, QuadTreeNodeChildren child_type)
{
	Rect dst;
	dst.x *= 2;
	dst.y *= 2;
	dst.w *= 2;
	dst.h *= 2;
	if (child_type == QTC_BOTTOM_LEFT || child_type == QTC_BOTTOM_RIGHT)
	{
		dst.x -= 2;
	}
	if (child_type == QTC_TOP_RIGHT || child_type == QTC_BOTTOM_RIGHT)
	{
		dst.y -= 2;
	}
	return dst;
}

Rect TransformFromQuadChild(const Rect& src, QuadTreeNodeChildren child_type)
{
	Rect dst;
	dst.x *= 0.5;
	dst.y *= 0.5;
	dst.w *= 0.5;
	dst.h *= 0.5;
	if (child_type == QTC_BOTTOM_LEFT || child_type == QTC_BOTTOM_RIGHT)
	{
		dst.x += 0.5;
	}
	if (child_type == QTC_TOP_RIGHT || child_type == QTC_BOTTOM_RIGHT)
	{
		dst.y += 0.5;
	}
	return dst;
}

}

#endif
