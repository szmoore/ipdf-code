#ifndef _VIEW_H
#define _VIEW_H

#include "ipdf.h"
#include "document.h"

namespace IPDF
{
	class View
	{
		public:
			View(Document & document, const Rect & bounds = Rect(0,0,1,1), const Colour & colour = Colour(0.f,0.f,0.f,1.f)) 
				: m_document(document), m_bounds(bounds), m_colour(colour) {}
			virtual ~View() {}

			void Render();
			
			void Translate(Real x, Real y);
			void ScaleAroundPoint(Real x, Real y, Real scaleAmt);
			
			const Rect& GetBounds() const { return m_bounds; }
		
		private:
			void DrawGrid();
			Document & m_document;
			Rect m_bounds;
			Colour m_colour;
	};
}

#endif //_VIEW_H
