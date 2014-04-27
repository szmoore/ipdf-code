#include "view.h"

#include "gl_core44.h"

using namespace IPDF;
using namespace std;
#define RECT_VERT \
	"#version 140\n"\
	"#extension GL_ARB_shading_language_420pack : require\n"\
	"#extension GL_ARB_explicit_attrib_location : require\n"\
	"\n"\
	"layout(std140, binding=0) uniform ViewBounds\n"\
	"{\n"\
	"\tfloat bounds_x;\n"\
	"\tfloat bounds_y;\n"\
	"\tfloat bounds_w;\n"\
	"\tfloat bounds_h;\n"\
	"};\n"\
	"\n"\
	"layout(location = 0) in vec2 position;\n"\
	"\n"\
	"void main()\n"\
	"{\n"\
	"\tvec2 transformed_position;\n"\
	"\ttransformed_position.x = (position.x - bounds_x) / bounds_w;\n"\
	"\ttransformed_position.y = (position.y - bounds_y) / bounds_h;\n"\
	"\t// Transform to clip coordinates (-1,1, -1,1).\n"\
	"\tgl_Position.x = (transformed_position.x*2) - 1;\n"\
	"\tgl_Position.y = 1 - (transformed_position.y*2);\n"\
	"\tgl_Position.z = 0.0;\n"\
	"\tgl_Position.w = 1.0;\n"\
	"}\n"

#define RECT_FRAG \
	"#version 140\n"\
	"\n"\
	"out vec4 output_colour;\n"\
	"\n"\
	"uniform vec4 colour;\n"\
	"\n"\
	"void main()\n"\
	"{\n"\
	"\toutput_colour = colour;\n"\
	"}\n"

void View::Translate(Real x, Real y)
{
	x *= m_bounds.w;
	y *= m_bounds.h;
	m_bounds.x += x;
	m_bounds.y += y;
	Debug("View Bounds => %s", m_bounds.Str().c_str());
	if (!m_use_gpu_transform)
	{
		m_buffer_dirty = true;
	}
	m_bounds_dirty = true;
}

void View::ScaleAroundPoint(Real x, Real y, Real scaleAmt)
{
	// x and y are coordinates in the window
	// Convert to local coords.
	x *= m_bounds.w;
	y *= m_bounds.h;
	x += m_bounds.x;
	y += m_bounds.y;
	
	//Debug("Mouse wheel event %f %f %f\n", Float(x), Float(y), Float(scaleAmt));
	
	Real top = y - m_bounds.y;
	Real left = x - m_bounds.x;
	
	top *= scaleAmt;
	left *= scaleAmt;
	
	m_bounds.x = x - left;
	m_bounds.y = y - top;
	m_bounds.w *= scaleAmt;
	m_bounds.h *= scaleAmt;
	Debug("View Bounds => %s", m_bounds.Str().c_str());
	if (!m_use_gpu_transform)
		m_buffer_dirty = true;
	m_bounds_dirty = true;
}

Rect View::TransformToViewCoords(const Rect& inp) const
{
	Rect out;
	out.x = (inp.x - m_bounds.x) / m_bounds.w;
	out.y = (inp.y - m_bounds.y) / m_bounds.h;

	out.w = inp.w / m_bounds.w;
	out.h = inp.h / m_bounds.h;
	return out;
}

void View::DrawGrid()
{
	//TODO: Implement this with OpenGL 3.1+
#if 0
	// Draw some grid lines at fixed pixel positions
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, 1.0, 1.0, 0.0, -1.f, 1.f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glColor4f(0.9,0.9,0.9,0.1);
	const float num_lines = 50.0;
	for (float i = 0; i < num_lines; ++i)
	{
		glBegin(GL_LINES);
		glVertex2f(i*(1.0/num_lines), 0.0);
		glVertex2f(i*(1.0/num_lines), 1.0);
		glEnd();
		glBegin(GL_LINES);
		glVertex2f(0.0,i*(1.0/num_lines));
		glVertex2f(1.0,i*(1.0/num_lines));
		glEnd();
	
	}
#endif
}

void View::Render(int width, int height)
{
	if (width != m_cached_display.GetWidth() || height != m_cached_display.GetHeight())
	{
		m_cached_display.Create(width, height);
		m_bounds_dirty = true;
	}

	if (!m_bounds_dirty)
	{
		m_cached_display.UnBind();
		m_cached_display.Blit();
		return;
	}
	m_cached_display.Bind();
	m_cached_display.Clear();

	if (m_buffer_dirty)
		ReRender();

	if (m_bounds_dirty)
	{
		if (m_use_gpu_transform)
		{
			GLfloat glbounds[] = {Float(m_bounds.x), Float(m_bounds.y), Float(m_bounds.w), Float(m_bounds.h)};
			m_bounds_ubo.Upload(sizeof(float)*4, glbounds);
		}
		else
		{
			GLfloat glbounds[] = {0.0f, 0.0f, 1.0f, 1.0f};
			m_bounds_ubo.Upload(sizeof(float)*4, glbounds);
		}
	}
	m_bounds_dirty = false;

	if (m_colour.a < 1.0f)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	m_vertex_buffer.Bind();
	m_index_buffer.Bind();
	m_bounds_ubo.Bind();
	m_rect_shader.Use();
	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(0xFFFFFFFF);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glDrawElements(GL_TRIANGLE_STRIP, m_rendered_filled * 5, GL_UNSIGNED_INT, 0);
	glDrawElements(GL_LINE_LOOP, m_rendered_outline*5, GL_UNSIGNED_INT,(void*)(sizeof(uint32_t)*m_rendered_filled*5));
	glDisableVertexAttribArray(0);
	glDisable(GL_PRIMITIVE_RESTART);
	if (m_colour.a < 1.0f)
	{
		glDisable(GL_BLEND);
	}
	m_cached_display.UnBind();
	m_cached_display.Blit();

}

void View::ReRender()
{
	static bool debug_output_done = false;
	if (!debug_output_done)
	{
		//m_document.DebugDumpObjects();
		debug_output_done = true;

		// TODO: Error check here.
		m_rect_shader.AttachVertexProgram(RECT_VERT);
		m_rect_shader.AttachFragmentProgram(RECT_FRAG);
		m_rect_shader.Link();
		m_rect_shader.Use();
		glUniform4f(m_rect_shader.GetUniformLocation("colour"), m_colour.r, m_colour.g, m_colour.b, m_colour.a);

		m_bounds_ubo.SetType(GraphicsBuffer::BufferTypeUniform);
		m_bounds_ubo.SetUsage(GraphicsBuffer::BufferUsageStreamDraw);

		m_vertex_buffer.SetType(GraphicsBuffer::BufferTypeVertex);
		m_index_buffer.SetUsage(GraphicsBuffer::BufferUsageStaticDraw);
		m_index_buffer.SetType(GraphicsBuffer::BufferTypeIndex);

		m_vertex_buffer.Upload(m_document.ObjectCount() * 8 * sizeof(float), NULL);
		m_index_buffer.Upload(m_document.ObjectCount() * 5 * sizeof(uint32_t), NULL);
	}
	m_rendered_filled = m_rendered_outline = 0;
	
	if (m_use_gpu_transform)
	{
		m_vertex_buffer.SetUsage(GraphicsBuffer::BufferUsageStaticDraw);
	}
	else
	{
		m_vertex_buffer.SetUsage(GraphicsBuffer::BufferUsageDynamicDraw);
	}


	//DrawGrid(); // Draw the gridlines



	float *vertexData = (float*)m_vertex_buffer.Map(false, true, true);
	uint32_t *indexData = (uint32_t*)m_index_buffer.Map(false, true, true);

	uint32_t currentIndex = 0;
	for (unsigned id = 0; id < m_document.ObjectCount(); ++id)
	{
		if (m_document.m_objects.types[id] != RECT_FILLED)
			continue;
		Rect obj_bounds;
		if (m_use_gpu_transform)
		{
			obj_bounds = m_document.m_objects.bounds[id];
		}
		else
		{
			obj_bounds = TransformToViewCoords(m_document.m_objects.bounds[id]);
		}
		*vertexData = Float(obj_bounds.x); vertexData++;
		*vertexData = Float(obj_bounds.y); vertexData++;
		*vertexData = Float(obj_bounds.x) + Float(obj_bounds.w); vertexData++;
		*vertexData = Float(obj_bounds.y); vertexData++;
		*vertexData = Float(obj_bounds.x) + Float(obj_bounds.w); vertexData++;
		*vertexData = Float(obj_bounds.y) + Float(obj_bounds.h); vertexData++;
		*vertexData = Float(obj_bounds.x); vertexData++;
		*vertexData = Float(obj_bounds.y) + Float(obj_bounds.h); vertexData++;

		*indexData = currentIndex; indexData++;
		*indexData = currentIndex+1; indexData++;
		*indexData = currentIndex+3; indexData++;
		*indexData = currentIndex+2; indexData++;
		*indexData = 0xFFFFFFFF; // Primitive restart.
		indexData++;
		currentIndex += 4;
		m_rendered_filled++;

	}
	
	for (unsigned id = 0; id < m_document.ObjectCount(); ++id)
	{
		if (m_document.m_objects.types[id] != RECT_OUTLINE)
			continue;
		Rect obj_bounds;
		if (m_use_gpu_transform)
		{
			obj_bounds = m_document.m_objects.bounds[id];
		}
		else
		{
			obj_bounds = TransformToViewCoords(m_document.m_objects.bounds[id]);
		}
		*vertexData = Float(obj_bounds.x); vertexData++;
		*vertexData = Float(obj_bounds.y); vertexData++;
		*vertexData = Float(obj_bounds.x) + Float(obj_bounds.w); vertexData++;
		*vertexData = Float(obj_bounds.y); vertexData++;
		*vertexData = Float(obj_bounds.x) + Float(obj_bounds.w); vertexData++;
		*vertexData = Float(obj_bounds.y) + Float(obj_bounds.h); vertexData++;
		*vertexData = Float(obj_bounds.x); vertexData++;
		*vertexData = Float(obj_bounds.y) + Float(obj_bounds.h); vertexData++;

		*indexData = currentIndex; indexData++;
		*indexData = currentIndex+1; indexData++;
		*indexData = currentIndex+2; indexData++;
		*indexData = currentIndex+3; indexData++;
		*indexData = 0xFFFFFFFF; // Primitive restart.
		indexData++;
		currentIndex += 4;
		m_rendered_outline++;
	}
	m_vertex_buffer.UnMap();
	m_index_buffer.UnMap();

	m_buffer_dirty = false;

}
