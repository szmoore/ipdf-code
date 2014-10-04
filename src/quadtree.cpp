#ifndef QUADTREE_REMOVED
#include "quadtree.h"
#include "document.h"

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
	//Debug("%s is contained in %s\n", src.Str().c_str(), dst.Str().c_str());
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
	//Debug("%s is contained in %s... \n", src.Str().c_str(), dst.Str().c_str());
	return true;
}



QuadTreeIndex QuadTree::GetNeighbour(QuadTreeIndex start, int xdir, int ydir, Document *addTo) const
{
	if (!xdir && !ydir) return start;

	if (addTo && (nodes[start].parent == -1) && nodes[start].child_type != QTC_UNKNOWN)
	{
		Debug("Adding parent of node %d...", start);
		addTo->GenQuadParent(start, nodes[start].child_type);
	}

	QuadTreeIndex newNode;
	// Try moving to the right if that's easy.
	if (xdir > 0)
	{
		switch (nodes[start].child_type)
		{
		case QTC_TOP_LEFT:
		case QTC_BOTTOM_LEFT:
		{
			if (nodes[start].child_type == QTC_TOP_LEFT)
			{
				newNode = nodes[nodes[start].parent].top_right;
				if (addTo && newNode == -1)
				{
					newNode = addTo->GenQuadChild(nodes[start].parent, QTC_TOP_RIGHT);
				}
			}
			else
			{
				newNode = nodes[nodes[start].parent].bottom_right;
				if (addTo && newNode == -1)
				{
					newNode = addTo->GenQuadChild(nodes[start].parent, QTC_BOTTOM_RIGHT);
				}
			}	
			return GetNeighbour(newNode, xdir - 1, ydir, addTo);
		}
		case QTC_TOP_RIGHT:
		case QTC_BOTTOM_RIGHT:
		{
			QuadTreeIndex right_parent = GetNeighbour(nodes[start].parent, 1, 0, addTo);
			if (right_parent == -1) return -1;
			if (nodes[start].child_type == QTC_TOP_RIGHT)
			{
				newNode = nodes[right_parent].top_left;
				if (addTo && newNode == -1)
				{
					newNode = addTo->GenQuadChild(right_parent, QTC_TOP_LEFT);
				}
			}
			else
			{
				newNode = nodes[right_parent].bottom_left;
				if (addTo && newNode == -1)
				{
					newNode = addTo->GenQuadChild(right_parent, QTC_BOTTOM_LEFT);
				}
			}
			return GetNeighbour(newNode, xdir - 1, ydir, addTo);
		}
		default:
			return -1;

		}
	}

	// Try moving to the left.
	if (xdir < 0)
	{
		switch (nodes[start].child_type)
		{
		case QTC_TOP_RIGHT:
		case QTC_BOTTOM_RIGHT:
		{
			if (nodes[start].child_type == QTC_TOP_RIGHT)
			{
				newNode = nodes[nodes[start].parent].top_left;
				if (addTo && newNode == -1)
				{
					newNode = addTo->GenQuadChild(nodes[start].parent, QTC_TOP_LEFT);
				}
			}
			else
			{
				newNode = nodes[nodes[start].parent].bottom_left;
				if (addTo && newNode == -1)
				{
					newNode = addTo->GenQuadChild(nodes[start].parent, QTC_BOTTOM_LEFT);
				}
			}
				
			return GetNeighbour(newNode, xdir + 1, ydir, addTo);
		}
		case QTC_TOP_LEFT:
		case QTC_BOTTOM_LEFT:
		{
			QuadTreeIndex left_parent = GetNeighbour(nodes[start].parent, -1, 0, addTo);
			if (left_parent == -1) return -1;
			if (nodes[start].child_type == QTC_TOP_LEFT)
			{
				newNode = nodes[left_parent].top_right;
				if (addTo && newNode == -1)
				{
					newNode = addTo->GenQuadChild(left_parent, QTC_TOP_RIGHT);
				}
			}
			else
			{
				newNode = nodes[left_parent].bottom_right;
				if (addTo && newNode == -1)
				{
					newNode = addTo->GenQuadChild(left_parent, QTC_BOTTOM_RIGHT);
				}
			}
			return GetNeighbour(newNode, xdir + 1, ydir, addTo);
		}
		default:
			return -1;
		}
	}
	
	// Try moving to the bottom.
	if (ydir > 0)
	{
		switch (nodes[start].child_type)
		{
		case QTC_TOP_LEFT:
		case QTC_TOP_RIGHT:
		{
			if (nodes[start].child_type == QTC_TOP_LEFT)
			{
				newNode = nodes[nodes[start].parent].bottom_left;
				if (addTo && newNode == -1)
				{
					newNode = addTo->GenQuadChild(nodes[start].parent, QTC_BOTTOM_LEFT);
				}
			}
			else
			{
				newNode = nodes[nodes[start].parent].bottom_right;
				if (addTo && newNode == -1)
				{
					newNode = addTo->GenQuadChild(nodes[start].parent, QTC_BOTTOM_RIGHT);
				}
			}	
			return GetNeighbour(newNode, xdir, ydir - 1, addTo);
		}
		case QTC_BOTTOM_LEFT:
		case QTC_BOTTOM_RIGHT:
		{
			QuadTreeIndex bottom_parent = GetNeighbour(nodes[start].parent, 0, 1, addTo);
			if (bottom_parent == -1) return -1;
			if (nodes[start].child_type == QTC_BOTTOM_LEFT)
			{
				newNode = nodes[bottom_parent].top_left;
				if (addTo && newNode == -1)
				{
					newNode = addTo->GenQuadChild(bottom_parent, QTC_TOP_LEFT);
				}
			}
			else
			{
				newNode = nodes[bottom_parent].top_right;
				if (addTo && newNode == -1)
				{
					newNode = addTo->GenQuadChild(bottom_parent, QTC_TOP_RIGHT);
				}
			}
			return GetNeighbour(newNode, xdir, ydir - 1, addTo);
		}
		default:
			return -1;
		}
	}

	// Try moving up, towards the sky.
	if (ydir < 0)
	{
		switch (nodes[start].child_type)
		{
		case QTC_BOTTOM_LEFT:
		case QTC_BOTTOM_RIGHT:
		{
			if (nodes[start].child_type == QTC_BOTTOM_LEFT)
			{
				newNode = nodes[nodes[start].parent].top_left;
				if (addTo && newNode == -1)
				{
					newNode = addTo->GenQuadChild(nodes[start].parent, QTC_TOP_LEFT);
				}
			}
			else
			{
				newNode = nodes[nodes[start].parent].top_right;
				if (addTo && newNode == -1)
				{
					newNode = addTo->GenQuadChild(nodes[start].parent, QTC_TOP_RIGHT);
				}
			}	
			return GetNeighbour(newNode, xdir, ydir + 1, addTo);
		}
		case QTC_TOP_LEFT:
		case QTC_TOP_RIGHT:
		{
			QuadTreeIndex top_parent = GetNeighbour(nodes[start].parent, 0, -1, addTo);
			if (top_parent == -1) return -1;
			if (nodes[start].child_type == QTC_TOP_LEFT)
			{
				newNode = nodes[top_parent].bottom_left;
				if (addTo && newNode == -1)
				{
					newNode = addTo->GenQuadChild(top_parent, QTC_BOTTOM_LEFT);
				}
			}
			else
			{
				newNode = nodes[top_parent].bottom_right;
				if (addTo && newNode == -1)
				{
					newNode = addTo->GenQuadChild(top_parent, QTC_BOTTOM_RIGHT);
				}
			}
			return GetNeighbour(newNode, xdir, ydir + 1, addTo);
		}
		default:
			return -1;
		}
	}
	return -1;
}

}

#endif
