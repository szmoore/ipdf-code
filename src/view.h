#ifndef _VIEW_H
#define _VIEW_H

#include "ipdf.h"
#include "document.h"

namespace IPDF
{
	class View
	{
		public:
			View(Document & document) : m_document(document), m_bounds(0,0,1,1) {}
			virtual ~View() {}

			void Render();
			
			void Translate(Real x, Real y);
			void ScaleAroundPoint(Real x, Real y, Real scaleAmt);
		
		private:
			void DrawGrid();
			Document & m_document;
			Rect m_bounds;
	};
}

#endif //_VIEW_H
