#ifndef _VIEW_H
#define _VIEW_H

#include "ipdf.h"
#include "document.h"
#include "graphicsbuffer.h"
#include "framebuffer.h"

namespace IPDF
{
	class View
	{
		public:
			View(Document & document, const Rect & bounds = Rect(0,0,1,1), const Colour & colour = Colour(0.f,0.f,0.f,1.f)) 
				: m_document(document), m_bounds(bounds), m_colour(colour), m_use_gpu_transform(false), m_bounds_dirty(true), m_buffer_dirty(true)
			{
				Debug("View Created - Bounds => {%s}", m_bounds.Str().c_str());
			}
			virtual ~View() {}

			void Render(int width = 0, int height = 0);
			
			void Translate(Real x, Real y);
			void ScaleAroundPoint(Real x, Real y, Real scaleAmt);
			
			Rect TransformToViewCoords(const Rect& inp) const;
			
			const Rect& GetBounds() const { return m_bounds; }
			
			const bool UsingGPUTransform() const { return m_use_gpu_transform; }
			void ToggleGPUTransform() { m_use_gpu_transform = (!m_use_gpu_transform); m_bounds_dirty = true; m_buffer_dirty = true; }
		
		private:
			void ReRender();
			void DrawGrid();
			bool m_use_gpu_transform;
			bool m_bounds_dirty;
			bool m_buffer_dirty;
			GraphicsBuffer m_vertex_buffer;
			GraphicsBuffer m_index_buffer;
			FrameBuffer m_cached_display;
			Document & m_document;
			Rect m_bounds;
			Colour m_colour;
			uint32_t m_rendered_filled;
			uint32_t m_rendered_outline;
	};
}

#endif //_VIEW_H
