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

			void Load(const std::string & filename = "");
			void Save(const std::string & filename);
			void DebugDumpObjects();

			unsigned ObjectCount() const {return m_count;}
			inline const Objects & GetObjects() const {return m_objects;}

			bool operator==(const Document & equ) const;
			bool operator!=(const Document & equ) const {return !(this->operator==(equ));}

			void Add(ObjectType type, const Rect & bounds, unsigned data_index = 0);
			unsigned AddBezierData(const Bezier & bezier);

#ifndef QUADTREE_DISABLED
			inline const QuadTree& GetQuadTree() { if (m_quadtree.root_id == QUADTREE_EMPTY) { GenBaseQuadtree(); } return m_quadtree; }
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
