#include "view.h"
#include "bufferbuilder.h"
#include "screen.h"
#include "gl_core44.h"

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
		m_objbounds_vbo(), m_object_renderers(NUMBER_OF_OBJECT_TYPES), m_cpu_rendering_pixels(NULL)
{
	Debug("View Created - Bounds => {%s}", m_bounds.Str().c_str());

	// Create ObjectRenderers - new's match delete's in View::~View
	//TODO: Don't forget to put new renderers here or things will be segfaultastic
	m_object_renderers[RECT_FILLED] = new RectFilledRenderer();
	m_object_renderers[RECT_OUTLINE] = new RectOutlineRenderer();
	m_object_renderers[CIRCLE_FILLED] = new CircleFilledRenderer();
	m_object_renderers[BEZIER] = new BezierRenderer();

	// To add rendering for a new type of object;
	// 1. Add enum to ObjectType in ipdf.h
	// 2. Implement class inheriting from ObjectRenderer using that type in objectrenderer.h and objectrenderer.cpp
	// 3. Add it here
	// 4. Profit
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
	Debug("View Bounds => %s", m_bounds.Str().c_str());
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
	Debug("View Bounds => %s", m_bounds.Str().c_str());
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
		return;
	}

	// Bind FrameBuffer for rendering, and clear it


	if (m_render_dirty) // document has changed
		PrepareRender();

	if (m_buffer_dirty) // object bounds have changed
		UpdateObjBoundsVBO();

	if (m_use_gpu_transform)
	{
		GLfloat glbounds[] = {static_cast<GLfloat>(Float(m_bounds.x)), static_cast<GLfloat>(Float(m_bounds.y)), static_cast<GLfloat>(Float(m_bounds.w)), static_cast<GLfloat>(Float(m_bounds.h)),
					0.0, 0.0, 640.0, 480.0};
		m_bounds_ubo.Upload(sizeof(float)*8, glbounds);
	}
	else
	{
		GLfloat glbounds[] = {0.0f, 0.0f, 1.0f, 1.0f,
					0.0f, 0.0f, 640.0f, 480.0f};
		m_bounds_ubo.Upload(sizeof(float)*8, glbounds);
	}
	m_bounds_dirty = false;

	m_cached_display.Bind(); //NOTE: This is redundant; Clear already calls Bind
	m_cached_display.Clear();


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
			m_object_renderers[i]->RenderUsingGPU();
		}
		
		glDisableVertexAttribArray(0);
		if (m_colour.a < 1.0f)
		{
			glDisable(GL_BLEND);
		}
	}
	else // Rasterise on CPU then blit texture to GPU
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

		for (unsigned i = 0; i < m_object_renderers.size(); ++i)
		{
			m_object_renderers[i]->RenderUsingCPU(m_document.m_objects, *this, {m_cpu_rendering_pixels, width, height});
		}
		m_screen.RenderPixels(0,0,width, height, m_cpu_rendering_pixels); //TODO: Make this work :(
		// Debug for great victory (do something similar for GPU and compare?)
		ObjectRenderer::SaveBMP({m_cpu_rendering_pixels, width, height}, "cpu_rendering_last_frame.bmp");
	}
	m_cached_display.UnBind(); // resets render target to the screen
	m_cached_display.Blit(); // blit FrameBuffer to screen
}

void View::UpdateObjBoundsVBO()
{
	m_objbounds_vbo.Invalidate();
	m_objbounds_vbo.SetType(GraphicsBuffer::BufferTypeVertex);
	if (m_use_gpu_transform)
	{
		m_objbounds_vbo.SetUsage(GraphicsBuffer::BufferUsageStaticDraw);
	}
	else
	{
		m_objbounds_vbo.SetUsage(GraphicsBuffer::BufferUsageDynamicDraw);
	}
	m_objbounds_vbo.Resize(m_document.ObjectCount()*sizeof(GPUObjBounds));

	BufferBuilder<GPUObjBounds> obj_bounds_builder(m_objbounds_vbo.Map(false, true, true), m_objbounds_vbo.GetSize());

	for (unsigned id = 0; id < m_document.ObjectCount(); ++id)
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
	m_buffer_dirty = false;
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
		Debug("Object of type %d", type);
	}

	// Finish the buffers
	for (unsigned i = 0; i < m_object_renderers.size(); ++i)
	{
		m_object_renderers[i]->FinaliseBuffers();
	}
	dynamic_cast<BezierRenderer*>(m_object_renderers[BEZIER])->PrepareBezierGPUBuffer(m_document.m_objects);
	m_render_dirty = false;
}
