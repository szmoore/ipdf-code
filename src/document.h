#ifndef _DOCUMENT_H
#define _DOCUMENT_H

#include "ipdf.h"
#include "quadtree.h"

#include "../contrib/pugixml-1.4/src/pugixml.hpp"


typedef struct stbtt_fontinfo stbtt_fontinfo;

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
			
			
			
			
			/** SVG Related functions **/
			
			/** Load an SVG text file and add to the document **/
			void LoadSVG(const std::string & filename, const Rect & bounds = Rect(0,0,1,1));
			
			/** Parse an SVG node or SVG-group node, adding children to the document **/
			void ParseSVGNode(pugi::xml_node & root, const Rect & bounds, Real & width, Real & height);
			/** Parse an SVG path with string **/
			void ParseSVGPathData(const std::string & d, const Rect & bounds);

			void AddFontGlyphAtPoint(stbtt_fontinfo *font, int character, Real scale, Real x, Real y);

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
