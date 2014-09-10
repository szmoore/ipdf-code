#include "view.h"
#include "bufferbuilder.h"
#include "screen.h"
#include "gl_core44.h"

#ifndef CONTROLPANEL_DISABLED
	#include "controlpanel.h"
#endif //CONTROLPANEL_DISABLED

using namespace IPDF;
using namespace std;

/**
 * Constructs a view
 * Allocates memory for ObjectRenderers
 * @param document - The document to associate the View with
 * @param bounds - Initial bounds of the View
 * @param colour - Colour to use for rendering this view. TODO: Make sure this actually works, or just remove it
 */
View::View(Document & document, Screen & screen, const Rect & bounds, const Colour & colour)
	: m_use_gpu_transform(USE_GPU_TRANSFORM), m_use_gpu_rendering(USE_GPU_RENDERING), m_bounds_dirty(true), m_buffer_dirty(true), 
		m_render_dirty(true), m_document(document), m_screen(screen), m_cached_display(), m_bounds(bounds), m_colour(colour), m_bounds_ubo(), 
		m_objbounds_vbo(), m_object_renderers(NUMBER_OF_OBJECT_TYPES), m_cpu_rendering_pixels(NULL),
		m_perform_shading(USE_SHADING), m_show_bezier_bounds(false), m_show_bezier_type(false),
		m_show_fill_points(false), m_show_fill_bounds(false)
{
	Debug("View Created - Bounds => {%s}", m_bounds.Str().c_str());

	screen.SetView(this); // oh dear...

	// Create ObjectRenderers - new's match delete's in View::~View
	//TODO: Don't forget to put new renderers here or things will be segfaultastic
	m_object_renderers[RECT_FILLED] = new RectFilledRenderer();
	m_object_renderers[RECT_OUTLINE] = new RectOutlineRenderer();
	m_object_renderers[CIRCLE_FILLED] = new CircleFilledRenderer();
	m_object_renderers[BEZIER] = new BezierRenderer();
	m_object_renderers[PATH] = new PathRenderer();

	// To add rendering for a new type of object;
	// 1. Add enum to ObjectType in ipdf.h
	// 2. Implement class inheriting from ObjectRenderer using that type in objectrenderer.h and objectrenderer.cpp
	// 3. Add it here
	// 4. Profit


#ifndef QUADTREE_DISABLED
	m_quadtree_max_depth = 1;
	m_current_quadtree_node = document.GetQuadTree().root_id;
#endif
}

/**
 * Destroy a view
 * Frees memory used by ObjectRenderers
 */
View::~View()
{
	for (unsigned i = 0; i < m_object_renderers.size(); ++i)
	{
		delete m_object_renderers[i]; // delete's match new's in constructor
	}
	m_object_renderers.clear();
	delete [] m_cpu_rendering_pixels;
}

/**
 * Translate the view
 * @param x, y - Amount to translate
 */
void View::Translate(Real x, Real y)
{
	x *= m_bounds.w;
	y *= m_bounds.h;
	m_bounds.x += x;
	m_bounds.y += y;
	//Debug("View Bounds => %s", m_bounds.Str().c_str());
	if (!m_use_gpu_transform)
		m_buffer_dirty = true;
	m_bounds_dirty = true;
}

/**
 * Set View bounds
 * @param bounds - New bounds
 */
void View::SetBounds(const Rect & bounds)
{
	m_bounds.x = bounds.x;
	m_bounds.y = bounds.y;
	m_bounds.w = bounds.w;
	m_bounds.h = bounds.h;
	if (!m_use_gpu_transform)
		m_buffer_dirty = true;
	m_bounds_dirty = true;
}

/**
 * Scale the View at a point
 * @param x, y - Coordinates to scale at (eg: Mouse cursor position)
 * @param scale_amount - Amount to scale by
 */
void View::ScaleAroundPoint(Real x, Real y, Real scale_amount)
{
	// x and y are coordinates in the window
	// Convert to local coords.
	x *= m_bounds.w;
	y *= m_bounds.h;
	x += m_bounds.x;
	y += m_bounds.y;
	
	Real top = y - m_bounds.y;
	Real left = x - m_bounds.x;
	
	top *= scale_amount;
	left *= scale_amount;
	
	m_bounds.x = x - left;
	m_bounds.y = y - top;
	m_bounds.w *= scale_amount;
	m_bounds.h *= scale_amount;
	//Debug("Scale at {%s, %s} by %s View Bounds => %s", x.Str().c_str(), y.Str().c_str(), scale_amount.Str().c_str(), m_bounds.Str().c_str());
	if (!m_use_gpu_transform)
		m_buffer_dirty = true;
	m_bounds_dirty = true;
}

/**
 * Transform a point in the document to a point relative to the top left corner of the view
 * This is the CPU coordinate transform code; used only if the CPU is doing coordinate transforms
 * @param inp - Input Rect {x,y,w,h} in the document
 * @returns output Rect {x,y,w,h} in the View
 */
Rect View::TransformToViewCoords(const Rect& inp) const
{
	Rect out;
	out.x = (inp.x - m_bounds.x) / m_bounds.w;
	out.y = (inp.y - m_bounds.y) / m_bounds.h;
	out.w = inp.w / m_bounds.w;
	out.h = inp.h / m_bounds.h;
	return out;
}

/**
 * Render the view
 * Updates FrameBuffer if the document, object bounds, or view bounds have changed, then Blits it
 * Otherwise just Blits the cached FrameBuffer
 * @param width - Width of View to render
 * @param height - Height of View to render
 */
void View::Render(int width, int height)
{
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION,42,-1, "Beginning View::Render()");
	// View dimensions have changed (ie: Window was resized)
	int prev_width = m_cached_display.GetWidth();
	int prev_height = m_cached_display.GetHeight();
	if (width != prev_width || height != prev_height)
	{
		m_cached_display.Create(width, height);
		m_bounds_dirty = true;
	}

	// View bounds have not changed; blit the FrameBuffer as it is
	if (!m_bounds_dirty)
	{
		m_cached_display.UnBind();
		m_cached_display.Blit();
		glPopDebugGroup();
		return;
	}
	m_cached_display.Bind(); //NOTE: This is redundant; Clear already calls Bind
	m_cached_display.Clear();

#ifndef QUADTREE_DISABLED
	if (m_bounds_dirty)
	{
		if (false && (m_bounds.x > 1.0 || m_bounds.x < 0.0 || m_bounds.y > 1.0 || m_bounds.y < 0.0 || m_bounds.w > 1.0 || m_bounds.h > 1.0))
		{
			//TODO: Generate a new parent node.
			if (m_document.GetQuadTree().nodes[m_current_quadtree_node].parent != QUADTREE_EMPTY)
			{
				m_bounds = TransformFromQuadChild(m_bounds, m_document.GetQuadTree().nodes[m_current_quadtree_node].child_type);
				m_current_quadtree_node = m_document.GetQuadTree().nodes[m_current_quadtree_node].parent;
			}
		}
		if (ContainedInQuadChild(m_bounds, QTC_TOP_LEFT))
		{
			if (m_document.GetQuadTree().nodes[m_current_quadtree_node].top_left == QUADTREE_EMPTY)
			{
				// We want to reparent into a child node, but none exist. Get the document to create one.
				m_document.GenQuadChild(m_current_quadtree_node, QTC_TOP_LEFT);
				m_render_dirty = true;
			}
			m_bounds = TransformToQuadChild(m_bounds, QTC_TOP_LEFT);
			m_current_quadtree_node = m_document.GetQuadTree().nodes[m_current_quadtree_node].top_left;
		}
		if (ContainedInQuadChild(m_bounds, QTC_TOP_RIGHT))
		{
			if (m_document.GetQuadTree().nodes[m_current_quadtree_node].top_right == QUADTREE_EMPTY)
			{
				// We want to reparent into a child node, but none exist. Get the document to create one.
				m_document.GenQuadChild(m_current_quadtree_node, QTC_TOP_RIGHT);
				m_render_dirty = true;
			}
			m_bounds = TransformToQuadChild(m_bounds, QTC_TOP_RIGHT);
			m_current_quadtree_node = m_document.GetQuadTree().nodes[m_current_quadtree_node].top_right;
		}
		if (ContainedInQuadChild(m_bounds, QTC_BOTTOM_LEFT))
		{
			if (m_document.GetQuadTree().nodes[m_current_quadtree_node].bottom_left == QUADTREE_EMPTY)
			{
				// We want to reparent into a child node, but none exist. Get the document to create one.
				m_document.GenQuadChild(m_current_quadtree_node, QTC_BOTTOM_LEFT);
				m_render_dirty = true;
			}
			m_bounds = TransformToQuadChild(m_bounds, QTC_BOTTOM_LEFT);
			m_current_quadtree_node = m_document.GetQuadTree().nodes[m_current_quadtree_node].bottom_left;
		}
		if (ContainedInQuadChild(m_bounds, QTC_BOTTOM_RIGHT))
		{
			if (m_document.GetQuadTree().nodes[m_current_quadtree_node].bottom_right == QUADTREE_EMPTY)
			{
				// We want to reparent into a child node, but none exist. Get the document to create one.
				m_document.GenQuadChild(m_current_quadtree_node, QTC_BOTTOM_RIGHT);
				m_render_dirty = true;
			}
			m_bounds = TransformToQuadChild(m_bounds, QTC_BOTTOM_RIGHT);
			m_current_quadtree_node = m_document.GetQuadTree().nodes[m_current_quadtree_node].bottom_right;
		}
	}
	m_screen.DebugFontPrintF("Current View QuadTree Node: %d (objs: %d -> %d)\n", m_current_quadtree_node, m_document.GetQuadTree().nodes[m_current_quadtree_node].object_begin,
				m_document.GetQuadTree().nodes[m_current_quadtree_node].object_end);

	Rect view_top_bounds = m_bounds;
	QuadTreeIndex tmp = m_current_quadtree_node;
	while (tmp != -1)
	{
		view_top_bounds = TransformFromQuadChild(view_top_bounds, m_document.GetQuadTree().nodes[tmp].child_type);
		tmp = m_document.GetQuadTree().nodes[tmp].parent;
	}
	m_screen.DebugFontPrintF("Equivalent View Bounds: %s\n", view_top_bounds.Str().c_str());
#endif

	if (!m_use_gpu_rendering)
	{
		// Dynamically resize CPU rendering target pixels if needed
		if (m_cpu_rendering_pixels == NULL || width*height > prev_width*prev_height)
		{
			delete [] m_cpu_rendering_pixels;
			m_cpu_rendering_pixels = new uint8_t[width*height*4];
			if (m_cpu_rendering_pixels == NULL)
				Fatal("Could not allocate %d*%d*4 = %d bytes for cpu rendered pixels", width, height, width*height*4);
		}
		// Clear CPU rendering pixels
		for (int i = 0; i < width*height*4; ++i)
			m_cpu_rendering_pixels[i] = 255;
	}
#ifdef QUADTREE_DISABLED
	RenderRange(width, height, 0, m_document.ObjectCount());
#else
	RenderQuadtreeNode(width, height, m_current_quadtree_node, m_quadtree_max_depth);
#endif
	if (!m_use_gpu_rendering)
	{
		m_screen.RenderPixels(0,0,width, height, m_cpu_rendering_pixels); //TODO: Make this work :(
		// Debug for great victory (do something similar for GPU and compare?)
		//ObjectRenderer::SaveBMP({m_cpu_rendering_pixels, width, height}, "cpu_rendering_last_frame.bmp");
	}
	m_cached_display.UnBind(); // resets render target to the screen
	m_cached_display.Blit(); // blit FrameBuffer to screen
	m_buffer_dirty = false;
	glPopDebugGroup();
	
#ifndef CONTROLPANEL_DISABLED
	ControlPanel::Update();
#endif //CONTROLPANEL_DISABLED
	//Debug("Completed Render");
	
}

#ifndef QUADTREE_DISABLED
void View::RenderQuadtreeNode(int width, int height, QuadTreeIndex node, int remaining_depth)
{
	Rect old_bounds = m_bounds;
	if (node == QUADTREE_EMPTY) return;
	if (!remaining_depth) return;
	//Debug("Rendering QT node %d, (objs: %d -- %d)\n", node, m_document.GetQuadTree().nodes[node].object_begin, m_document.GetQuadTree().nodes[node].object_end);
	m_bounds_dirty = true;
	RenderRange(width, height, m_document.GetQuadTree().nodes[node].object_begin, m_document.GetQuadTree().nodes[node].object_end);

	m_bounds = TransformToQuadChild(old_bounds, QTC_TOP_LEFT);
	m_bounds_dirty = true;
	RenderQuadtreeNode(width, height, m_document.GetQuadTree().nodes[node].top_left, remaining_depth-1);
	m_bounds = TransformToQuadChild(old_bounds, QTC_TOP_RIGHT);
	m_bounds_dirty = true;
	RenderQuadtreeNode(width, height, m_document.GetQuadTree().nodes[node].top_right, remaining_depth-1);
	m_bounds = TransformToQuadChild(old_bounds, QTC_BOTTOM_LEFT);
	m_bounds_dirty = true;
	RenderQuadtreeNode(width, height, m_document.GetQuadTree().nodes[node].bottom_left, remaining_depth-1);
	m_bounds = TransformToQuadChild(old_bounds, QTC_BOTTOM_RIGHT);
	m_bounds_dirty = true;
	RenderQuadtreeNode(width, height, m_document.GetQuadTree().nodes[node].bottom_right, remaining_depth-1);
	m_bounds = old_bounds;
	m_bounds_dirty = true;
}
#endif

void View::RenderRange(int width, int height, unsigned first_obj, unsigned last_obj)
{
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 43, -1, "View::RenderRange()");
	if (m_render_dirty) // document has changed
		PrepareRender();

	if (m_buffer_dirty || m_bounds_dirty) // object bounds have changed
		UpdateObjBoundsVBO(first_obj, last_obj);

	if (m_use_gpu_transform)
	{
		GLfloat glbounds[] = {static_cast<GLfloat>(Float(m_bounds.x)), static_cast<GLfloat>(Float(m_bounds.y)), static_cast<GLfloat>(Float(m_bounds.w)), static_cast<GLfloat>(Float(m_bounds.h)),
					0.0, 0.0, static_cast<GLfloat>(width), static_cast<GLfloat>(height)};
		m_bounds_ubo.Upload(sizeof(float)*8, glbounds);
	}
	else
	{
		GLfloat glbounds[] = {0.0f, 0.0f, 1.0f, 1.0f,
					0.0f, 0.0f, float(width), float(height)};
		m_bounds_ubo.Upload(sizeof(float)*8, glbounds);
	}
	m_bounds_dirty = false;


	// Render using GPU
	if (m_use_gpu_rendering) 
	{
		if (m_colour.a < 1.0f)
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
		m_objbounds_vbo.Bind();
		m_bounds_ubo.Bind();
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	
		for (unsigned i = 0; i < m_object_renderers.size(); ++i)
		{
			m_object_renderers[i]->RenderUsingGPU(first_obj, last_obj);
		}
		
		glDisableVertexAttribArray(0);
		if (m_colour.a < 1.0f)
		{
			glDisable(GL_BLEND);
		}
	}
	else // Rasterise on CPU then blit texture to GPU
	{

		for (unsigned i = 0; i < m_object_renderers.size(); ++i)
		{
			m_object_renderers[i]->RenderUsingCPU(m_document.m_objects, *this, {m_cpu_rendering_pixels, width, height}, first_obj, last_obj);
		}
	}
	glPopDebugGroup();
}

void View::UpdateObjBoundsVBO(unsigned first_obj, unsigned last_obj)
{
	//m_objbounds_vbo.Invalidate();
	m_objbounds_vbo.SetType(GraphicsBuffer::BufferTypeVertex);
	m_objbounds_vbo.SetName("Object Bounds VBO");
	if (m_use_gpu_transform)
	{
		m_objbounds_vbo.SetUsage(GraphicsBuffer::BufferUsageStaticDraw);
	}
	else
	{
		m_objbounds_vbo.SetUsage(GraphicsBuffer::BufferUsageDynamicCopy);
	}
	m_objbounds_vbo.Resize(m_document.ObjectCount()*sizeof(GPUObjBounds));

	BufferBuilder<GPUObjBounds> obj_bounds_builder(m_objbounds_vbo.MapRange(first_obj*sizeof(GPUObjBounds), (last_obj-first_obj)*sizeof(GPUObjBounds), false, true, true), m_objbounds_vbo.GetSize());

	for (unsigned id = first_obj; id < last_obj; ++id)
	{
		Rect obj_bounds;
		if (m_use_gpu_transform)
		{
			obj_bounds = m_document.m_objects.bounds[id];
		}
		else
		{
			obj_bounds = TransformToViewCoords(m_document.m_objects.bounds[id]);
		}
		GPUObjBounds gpu_bounds = {
			(float)Float(obj_bounds.x),
			(float)Float(obj_bounds.y),
			(float)Float(obj_bounds.x + obj_bounds.w),
			(float)Float(obj_bounds.y + obj_bounds.h)
		};
		obj_bounds_builder.Add(gpu_bounds);

	}
	m_objbounds_vbo.UnMap();
}
/**
 * Prepare the document for rendering
 * Will be called on View::Render if m_render_dirty is set
 * (Called at least once, on the first Render)
 */
void View::PrepareRender()
{
	Debug("Recreate buffers with %u objects", m_document.ObjectCount());
	// Prepare bounds vbo
	m_bounds_ubo.Invalidate();
	m_bounds_ubo.SetType(GraphicsBuffer::BufferTypeUniform);
	m_bounds_ubo.SetUsage(GraphicsBuffer::BufferUsageStreamDraw);
	m_bounds_ubo.SetName("m_bounds_ubo: Screen bounds.");
	
	// Instead of having each ObjectRenderer go through the whole document
	//  we initialise them, go through the document once adding to the appropriate Renderers
	//  and then finalise them
	// This will totally be efficient if we have like, a lot of distinct ObjectTypes. Which could totally happen. You never know.

	// Prepare the buffers
	for (unsigned i = 0; i < m_object_renderers.size(); ++i)
	{
		m_object_renderers[i]->PrepareBuffers(m_document.ObjectCount());
	}

	// Add objects from Document to buffers
	for (unsigned id = 0; id < m_document.ObjectCount(); ++id)
	{
		ObjectType type = m_document.m_objects.types[id];
		m_object_renderers.at(type)->AddObjectToBuffers(id); // Use at() in case the document is corrupt TODO: Better error handling?
		// (Also, Wow I just actually used std::vector::at())
		// (Also, I just managed to make it throw an exception because I'm a moron)
		//Debug("Object of type %d", type);
	}

	// Finish the buffers
	for (unsigned i = 0; i < m_object_renderers.size(); ++i)
	{
		m_object_renderers[i]->FinaliseBuffers();
	}
	dynamic_cast<BezierRenderer*>(m_object_renderers[BEZIER])->PrepareBezierGPUBuffer(m_document.m_objects);
	m_render_dirty = false;
}
