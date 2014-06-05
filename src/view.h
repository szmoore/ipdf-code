#ifndef _VIEW_H
#define _VIEW_H

#include "ipdf.h"
#include "document.h"
#include "graphicsbuffer.h"
#include "framebuffer.h"
#include "shaderprogram.h"

namespace IPDF
{
	class View
	{
		public:
			View(Document & document, const Rect & bounds = Rect(0,0,1,1), const Colour & colour = Colour(0.f,0.f,0.f,1.f)) 
				: m_use_gpu_transform(false), m_bounds_dirty(true), m_buffer_dirty(true), m_render_inited(false), m_document(document), m_bounds(bounds), m_colour(colour) 
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
			void PrepareRender();
			void UpdateObjBoundsVBO();
			void DrawGrid();
			bool m_use_gpu_transform;
			bool m_bounds_dirty;
			bool m_buffer_dirty;
			bool m_render_inited;
			ShaderProgram m_rect_outline_shader;
			ShaderProgram m_rect_filled_shader;
			ShaderProgram m_circle_filled_shader;
			// Stores the view bounds.
			GraphicsBuffer m_bounds_ubo;
			// Stores the bounds for _all_ objects.
			GraphicsBuffer m_objbounds_vbo;
			// Stores indices into the objbounds vbo for each type of object.
			GraphicsBuffer m_outline_ibo;	// Rectangle outline
			GraphicsBuffer m_filled_ibo;	// Filled rectangle
			GraphicsBuffer m_circle_ibo;	// Filled circle
			FrameBuffer m_cached_display;
			Document & m_document;
			Rect m_bounds;
			Colour m_colour;
			uint32_t m_rendered_filled;
			uint32_t m_rendered_outline;
			uint32_t m_rendered_circle;
	};
}

#endif //_VIEW_H
