#include "view.h"
#include "bufferbuilder.h"

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

#define RECT_OUTLINE_GEOM \
	"#version 150\n"\
	"\n"\
	"layout(lines) in;\n"\
	"layout(line_strip, max_vertices = 5) out;\n"\
	"\n"\
	"void main()\n"\
	"{\n"\
	"\tgl_Position = gl_in[0].gl_Position;\n"\
	"\tEmitVertex();\n"\
	"\tgl_Position = vec4(gl_in[0].gl_Position.x, gl_in[1].gl_Position.y, 0.0, 1.0);\n"\
	"\tEmitVertex();\n"\
	"\tgl_Position = gl_in[1].gl_Position;\n"\
	"\tEmitVertex();\n"\
	"\tgl_Position = vec4(gl_in[1].gl_Position.x, gl_in[0].gl_Position.y, 0.0, 1.0);\n"\
	"\tEmitVertex();\n"\
	"\tgl_Position = gl_in[0].gl_Position;\n"\
	"\tEmitVertex();\n"\
	"\tEndPrimitive();\n"\
	"}\n"

#define RECT_FILLED_GEOM \
	"#version 150\n"\
	"\n"\
	"layout(lines) in;\n"\
	"layout(triangle_strip, max_vertices = 4) out;\n"\
	"\n"\
	"void main()\n"\
	"{\n"\
	"\tgl_Position = gl_in[0].gl_Position;\n"\
	"\tEmitVertex();\n"\
	"\tgl_Position = vec4(gl_in[0].gl_Position.x, gl_in[1].gl_Position.y, 0.0, 1.0);\n"\
	"\tEmitVertex();\n"\
	"\tgl_Position = vec4(gl_in[1].gl_Position.x, gl_in[0].gl_Position.y, 0.0, 1.0);\n"\
	"\tEmitVertex();\n"\
	"\tgl_Position = gl_in[1].gl_Position;\n"\
	"\tEmitVertex();\n"\
	"\tEndPrimitive();\n"\
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

	if (!m_render_inited)
		PrepareRender();

	if (m_buffer_dirty)
		UpdateObjBoundsVBO();

	if (m_bounds_dirty)
	{
		if (m_use_gpu_transform)
		{
			GLfloat glbounds[] = {static_cast<GLfloat>(Float(m_bounds.x)), static_cast<GLfloat>(Float(m_bounds.y)), static_cast<GLfloat>(Float(m_bounds.w)), static_cast<GLfloat>(Float(m_bounds.h))};
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
	m_objbounds_vbo.Bind();
	m_bounds_ubo.Bind();
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

	// Filled Rectangles
	m_rect_filled_shader.Use();
	m_filled_ibo.Bind();
	glDrawElements(GL_LINES, m_rendered_filled*2, GL_UNSIGNED_INT, 0);

	// Rectangle Outlines
	m_rect_outline_shader.Use();
	m_outline_ibo.Bind();
	glDrawElements(GL_LINES, m_rendered_outline*2, GL_UNSIGNED_INT, 0);
	glDisableVertexAttribArray(0);
	if (m_colour.a < 1.0f)
	{
		glDisable(GL_BLEND);
	}
	m_cached_display.UnBind();
	m_cached_display.Blit();

}

struct GPUObjBounds
{
	float x0, y0;
	float x1, y1;
};

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

void View::PrepareRender()
{
	// TODO: Error check here.
	m_rect_outline_shader.AttachGeometryProgram(RECT_OUTLINE_GEOM);
	m_rect_outline_shader.AttachVertexProgram(RECT_VERT);
	m_rect_outline_shader.AttachFragmentProgram(RECT_FRAG);
	m_rect_outline_shader.Link();
	m_rect_outline_shader.Use();
	glUniform4f(m_rect_outline_shader.GetUniformLocation("colour"), m_colour.r, m_colour.g, m_colour.b, m_colour.a);

	m_rect_filled_shader.AttachGeometryProgram(RECT_FILLED_GEOM);
	m_rect_filled_shader.AttachVertexProgram(RECT_VERT);
	m_rect_filled_shader.AttachFragmentProgram(RECT_FRAG);
	m_rect_filled_shader.Link();
	m_rect_filled_shader.Use();
	glUniform4f(m_rect_filled_shader.GetUniformLocation("colour"), m_colour.r, m_colour.g, m_colour.b, m_colour.a);

	m_bounds_ubo.SetType(GraphicsBuffer::BufferTypeUniform);
	m_bounds_ubo.SetUsage(GraphicsBuffer::BufferUsageStreamDraw);

	m_outline_ibo.SetUsage(GraphicsBuffer::BufferUsageStaticDraw);
	m_outline_ibo.SetType(GraphicsBuffer::BufferTypeIndex);
	m_outline_ibo.Resize(m_document.ObjectCount() * 2 * sizeof(uint32_t));
	BufferBuilder<uint32_t> outline_builder(m_outline_ibo.Map(false, true, true), m_outline_ibo.GetSize());	

	m_filled_ibo.SetUsage(GraphicsBuffer::BufferUsageStaticDraw);
	m_filled_ibo.SetType(GraphicsBuffer::BufferTypeIndex);
	m_filled_ibo.Resize(m_document.ObjectCount() * 2 * sizeof(uint32_t));
	BufferBuilder<uint32_t> filled_builder(m_filled_ibo.Map(false, true, true), m_filled_ibo.GetSize());


	m_rendered_filled = m_rendered_outline = 0;
	uint32_t currentIndex = 0;
	for (unsigned id = 0; id < m_document.ObjectCount(); ++id)
	{
		if (m_document.m_objects.types[id] != RECT_FILLED)
		{
			outline_builder.Add(currentIndex++);
			outline_builder.Add(currentIndex++);
			m_rendered_outline++;
		}
		else
		{
			filled_builder.Add(currentIndex++);
			filled_builder.Add(currentIndex++);
			m_rendered_filled++;
		}

	}
	m_outline_ibo.UnMap();
	m_filled_ibo.UnMap();

	m_render_inited = true;
}
