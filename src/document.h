#ifndef _DOCUMENT_H
#define _DOCUMENT_H

#include "ipdf.h"
#include "quadtree.h"

#include "../contrib/pugixml-1.4/src/pugixml.hpp"
#include "stb_truetype.h"

typedef struct stbtt_fontinfo stbtt_fontinfo;

namespace IPDF
{
	struct SVGMatrix
	{
		Real a; // width
		Real c; // skew x by y
		Real e; // translate x
		
		Real b; // skew y by x
		Real d; // height
		Real f; // translate y
	};
	// SVG matrix transforms (x,y) <- (a x' + c y' + e, b x' + d y' + f)
	// Equivelant to OpenGL 3d matrix transform ((a, c, e) (b, d, f) (0,0,1))
	
	class Document
	{
		public:
			Document(const std::string & filename = "", const std::string & font_filename = "DejaVuSansMono.ttf") : m_objects(), m_count(0), m_font_data(NULL), m_font()
			{
				Load(filename);
				if (font_filename != "")
					SetFont(font_filename);
			}
			virtual ~Document() 
			{
				free(m_font_data);
			}
			
			

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
			void ParseSVGNode(pugi::xml_node & root, SVGMatrix & transform);
			/** Parse an SVG path with string **/
			void ParseSVGPathData(const std::string & d, const SVGMatrix & transform);
			
			/** Modify an SVG transformation matrix **/
			static void ParseSVGTransform(const std::string & s, SVGMatrix & transform);

			/** Font related functions **/
			void SetFont(const std::string & font_filename);
			void AddText(const std::string & text, Real scale, Real x, Real y);
			
			void AddFontGlyphAtPoint(stbtt_fontinfo *font, int character, Real scale, Real x, Real y);

#ifndef QUADTREE_DISABLED
			inline const QuadTree& GetQuadTree() { if (m_quadtree.root_id == QUADTREE_EMPTY) { GenBaseQuadtree(); } return m_quadtree; }
			QuadTreeIndex GenQuadChild(QuadTreeIndex parent, QuadTreeNodeChildren type);
			QuadTreeIndex GenQuadParent(QuadTreeIndex child, QuadTreeNodeChildren mytype);
			// Returns the number of objects the current object formed when clipped, the objects in question are added to the end of the document.
			int ClipObjectToQuadChild(int object_id, QuadTreeNodeChildren type);
#endif

		private:
			friend class View;
			Objects m_objects;
#ifndef QUADTREE_DISABLED
			QuadTree m_quadtree;
			void GenBaseQuadtree();
#endif
			unsigned m_count;
			unsigned char * m_font_data;
			stbtt_fontinfo m_font;
		
			

	};
}

#endif //_DOCUMENT_H
