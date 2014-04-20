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
				: m_document(document), m_bounds(bounds), m_colour(colour), m_use_gpu_transform(false) {}
			virtual ~View() {}

			void Render();
			
			void Translate(Real x, Real y);
			void ScaleAroundPoint(Real x, Real y, Real scaleAmt);
			
			Rect TransformToViewCoords(const Rect& inp) const;
			
			const Rect& GetBounds() const { return m_bounds; }
			
			const bool UsingGPUTransform() const { return m_use_gpu_transform; }
			void ToggleGPUTransform() { m_use_gpu_transform = (!m_use_gpu_transform); }
		
		private:
			void DrawGrid();
			bool m_use_gpu_transform;
			Document & m_document;
			Rect m_bounds;
			Colour m_colour;
	};
}

#endif //_VIEW_H
