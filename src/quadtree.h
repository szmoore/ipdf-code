#ifndef _QUADTREE_H
#define _QUADTREE_H


#ifndef QUADTREE_REMOVED


#include "common.h"
#include "ipdf.h"

namespace IPDF
{

	typedef int QuadTreeIndex;
	static const QuadTreeIndex QUADTREE_EMPTY = -1;

	enum QuadTreeNodeChildren
	{
		QTC_UNKNOWN,
		QTC_TOP_LEFT,
		QTC_TOP_RIGHT,
		QTC_BOTTOM_LEFT,
		QTC_BOTTOM_RIGHT
	};

	// Represents a single node in a quadtree.
	struct QuadTreeNode
	{
		// Indices of children nodes, QUADTREE_EMPTY if no such child.
		QuadTreeIndex top_left;
		QuadTreeIndex top_right;
		QuadTreeIndex bottom_left;
		QuadTreeIndex bottom_right;
		// Parent node id. QUADTREE_EMPTY if root.
		QuadTreeIndex parent;
		// Which child am I?
		QuadTreeNodeChildren child_type;
		// First object in the node.
		unsigned object_begin;
		// Last object in the node.
		unsigned object_end;
	};

	struct QuadTree
	{
		QuadTree() : root_id(QUADTREE_EMPTY) {}
		QuadTreeIndex root_id;
		std::vector<QuadTreeNode> nodes;
	};

	Rect TransformToQuadChild(const Rect& src, QuadTreeNodeChildren child_type);
	Rect TransformFromQuadChild(const Rect& src, QuadTreeNodeChildren child_type);
	bool ContainedInQuadChild(const Rect& src, QuadTreeNodeChildren child_type);
}

#else
#define QUADTREE_DISABLED
#endif

#endif
