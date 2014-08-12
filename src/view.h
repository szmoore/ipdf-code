#ifndef _VIEW_H
#define _VIEW_H

#include "ipdf.h"
#include "document.h"
#include "framebuffer.h"
#include "objectrenderer.h"

#define USE_GPU_TRANSFORM true
#define USE_GPU_RENDERING true

namespace IPDF
{
	class Screen;
	/**
	 * The View class manages a rectangular view into the document.
	 * It is responsible for coordinate transforms and rendering the document.
	 * ObjectRenderer's for each type of Object should be created in the constructor.
	 */
	class View
	{
		public:
			View(Document & document, Screen & screen, const Rect & bounds = Rect(0,0,1,1), const Colour & colour = Colour(0.f,0.f,0.f,1.f));
			virtual ~View();

			void Render(int width = 0, int height = 0);
			
			void Translate(Real x, Real y);
			void ScaleAroundPoint(Real x, Real y, Real scale_amount);
			
			Rect TransformToViewCoords(const Rect& inp) const;
			
			const Rect& GetBounds() const { return m_bounds; }
			
			const bool UsingGPUTransform() const { return m_use_gpu_transform; } // whether view transform calculated on CPU or GPU
			const bool UsingGPURendering() const { return m_use_gpu_rendering; } // whether GPU shaders are used or CPU rendering
			void ToggleGPUTransform() { m_use_gpu_transform = (!m_use_gpu_transform); m_bounds_dirty = true; m_buffer_dirty = true; }
			void ToggleGPURendering() { m_use_gpu_rendering = (!m_use_gpu_rendering); m_bounds_dirty = true; m_buffer_dirty = true; }


			void ForceBoundsDirty() {m_bounds_dirty = true;}		
			void ForceBufferDirty() {m_buffer_dirty = true;}		
			void ForceRenderDirty() {m_render_dirty = true;}


		private:
			struct GPUObjBounds
			{
				float x0, y0;
				float x1, y1;
			} __attribute__((packed));

			void PrepareRender(); // call when m_render_dirty is true
			void UpdateObjBoundsVBO(unsigned first_obj, unsigned last_obj); // call when m_buffer_dirty is true

			void RenderRange(int width, int height, unsigned first_obj, unsigned last_obj);

			bool m_use_gpu_transform;
			bool m_use_gpu_rendering;
			bool m_bounds_dirty; // the view bounds has changed (occurs when changing view)
			bool m_buffer_dirty; // the object bounds have changed (also occurs when changing view, but only when not using GPU transforms)
			bool m_render_dirty; // the document has changed (occurs when document first loaded)
			Document & m_document;
			Screen & m_screen;
			FrameBuffer m_cached_display;
			Rect m_bounds;
			Colour m_colour;

			// Stores the view bounds.
			GraphicsBuffer m_bounds_ubo; //bounds_dirty means this one has changed
			// Stores the bounds for _all_ objects.
			GraphicsBuffer m_objbounds_vbo; //buffer_dirty means this one has changed

			// ObjectRenderers to be initialised in constructor
			// Trust me it will be easier to generalise things this way. Even though there are pointers.
			std::vector<ObjectRenderer*> m_object_renderers; 
			uint8_t * m_cpu_rendering_pixels; // pixels to be used for CPU rendering

#ifndef QUADTREE_DISABLED
			QuadTreeIndex m_current_quadtree_node;	// The highest node we will traverse.
			int m_quadtree_max_depth;		// The maximum quadtree depth.
			void RenderQuadtreeNode(int width, int height, QuadTreeIndex node, int remaining_depth);

#endif
	};
}

#endif //_VIEW_H
