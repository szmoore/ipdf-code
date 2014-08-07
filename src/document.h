#ifndef _DOCUMENT_H
#define _DOCUMENT_H

#include "ipdf.h"
#include "quadtree.h"

namespace IPDF
{
	class Document
	{
		public:
			Document(const std::string & filename = "") : m_objects(), m_count(0) {Load(filename);}
			virtual ~Document() {}
			
			void LoadSVG(const std::string & filename, const Rect & bounds = Rect(0,0,1,1));

			void Load(const std::string & filename = "");
			void Save(const std::string & filename);
			void DebugDumpObjects();

			unsigned ObjectCount() const {return m_count;}
			inline const Objects & GetObjects() const {return m_objects;}

			bool operator==(const Document & equ) const;
			bool operator!=(const Document & equ) const {return !(this->operator==(equ));}

			void Add(ObjectType type, const Rect & bounds, unsigned data_index = 0);
			unsigned AddBezierData(const Bezier & bezier);
			
			void AddPathFromString(const std::string & d, const Rect & bounds);

#ifndef QUADTREE_DISABLED
			inline const QuadTree& GetQuadTree() { if (m_quadtree.root_id == QUADTREE_EMPTY) { GenBaseQuadtree(); } return m_quadtree; }
			QuadTreeIndex GenQuadChild(QuadTreeIndex parent, QuadTreeNodeChildren type);
			QuadTreeIndex GenQuadParent(QuadTreeIndex child, QuadTreeNodeChildren mytype);
#endif

		private:
			friend class View;
			Objects m_objects;
#ifndef QUADTREE_DISABLED
			QuadTree m_quadtree;
			void GenBaseQuadtree();
#endif
			unsigned m_count;
			

	};
}

#endif //_DOCUMENT_H
