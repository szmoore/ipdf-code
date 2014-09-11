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



QuadTreeIndex QuadTree::GetNeighbour(QuadTreeIndex start, int xdir, int ydir)
{
	if (!xdir && !ydir) return start;

	// Try moving to the right if that's easy.
	if (xdir > 0)
	{
		switch (nodes[start].child_type)
		{
		case QTC_TOP_LEFT:
		case QTC_BOTTOM_LEFT:
			QuadTreeIndex newNode;
			if (nodes[start].child_type == QTC_TOP_LEFT)
				newNode = node[nodes[start].parent].top_right;
			else
				newNode = node[nodes[start].parent].bottom_right;
				
			return GetNeighbour(newNode, xdir - 1, ydir);
		case QTC_TOP_RIGHT:
		case QTC_BOTTOM_RIGHT:
			QuadTreeIndex right_parent = GetNeighbour(nodes[start].parent, 1, 0);
			if (right_parent == -1) return -1;
			QuadTreeIndex newNode;
			if (nodes[start].child_type == QTC_TOP_RIGHT)
				newNode = node[right_parent].top_left;
			else
				newNode = node[right_parent].bottom_left;
			return GetNeighbour(newNode, xdir - 1, ydir);

		}
	}

	// Try moving to the left.
	if (xdir < 0)
	{
		switch (nodes[start].child_type)
		{
		case QTC_TOP_RIGHT:
		case QTC_BOTTOM_RIGHT:
			QuadTreeIndex newNode;
			if (nodes[start].child_type == QTC_TOP_RIGHT)
				newNode = node[nodes[start].parent].top_left;
			else
				newNode = node[nodes[start].parent].bottom_left;
				
			return GetNeighbour(newNode, xdir + 1, ydir);
		case QTC_TOP_LEFT:
		case QTC_BOTTOM_LEFT:
			QuadTreeIndex left_parent = GetNeighbour(nodes[start].parent, -1, 0);
			if (left_parent == -1) return -1;
			QuadTreeIndex newNode;
			if (nodes[start].child_type == QTC_TOP_LEFT)
				newNode = node[left_parent].top_right;
			else
				newNode = node[left_parent].bottom_right;
			return GetNeighbour(newNode, xdir + 1, ydir);

		}
	}
	
	// Try moving to the bottom.
	if (ydir > 0)
	{
		switch (nodes[start].child_type)
		{
		case QTC_TOP_LEFT:
		case QTC_TOP_RIGHT:
			QuadTreeIndex newNode;
			if (nodes[start].child_type == QTC_TOP_LEFT)
				newNode = node[nodes[start].parent].bottom_left;
			else
				newNode = node[nodes[start].parent].bottom_right;
				
			return GetNeighbour(newNode, xdir, ydir - 1);
		case QTC_BOTTOM_LEFT:
		case QTC_BOTTOM_RIGHT:
			QuadTreeIndex bottom_parent = GetNeighbour(nodes[start].parent, 0, 1);
			if (bottom_parent == -1) return -1;
			QuadTreeIndex newNode;
			if (nodes[start].child_type == QTC_BOTTOM_LEFT)
				newNode = node[bottom__parent].top_left;
			else
				newNode = node[bottom_parent].top_right;
			return GetNeighbour(newNode, xdir, ydir - 1);

		}
	}

	// Try moving up, towards the sky.
	if (ydir < 0)
	{
		switch (nodes[start].child_type)
		{
		case QTC_BOTTOM_LEFT:
		case QTC_BOTTOM_RIGHT:
			QuadTreeIndex newNode;
			if (nodes[start].child_type == QTC_BOTTOM_LEFT)
				newNode = node[nodes[start].parent].top_left;
			else
				newNode = node[nodes[start].parent].top_right;
				
			return GetNeighbour(newNode, xdir, ydir + 1);
		case QTC_TOP_LEFT:
		case QTC_BOTTOM_LEFT:
			QuadTreeIndex top_parent = GetNeighbour(nodes[start].parent, 0, -1);
			if (top_parent == -1) return -1;
			QuadTreeIndex newNode;
			if (nodes[start].child_type == QTC_TOP_LEFT)
				newNode = node[top_parent].bottom_left;
			else
				newNode = node[top_parent].bottom_right;
			return GetNeighbour(newNode, xdir, ydir + 1);

		}
	}
	return -1;
}

}

#endif
