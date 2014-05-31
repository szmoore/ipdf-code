#include "common.h"
#include "screen.h"

#include "gl_core44.h"
#include <fcntl.h> // for access(2)
#include <unistd.h> // for access(2)

#include "bufferbuilder.h"
#include "shaderprogram.h"

#define BASICTEX_VERT \
	"#version 140\n"\
	"#extension GL_ARB_shading_language_420pack : require\n"\
	"#extension GL_ARB_explicit_attrib_location : require\n"\
	"\n"\
	"layout(std140, binding=0) uniform Viewport\n"\
	"{\n"\
	"\tfloat width;\n"\
	"\tfloat height;\n"\
	"};\n"\
	"\n"\
	"layout(location = 0) in vec2 position;\n"\
	"layout(location = 1) in vec2 tex_coord;\n"\
	"\n"\
	"out vec2 fp_tex_coord;\n"\
	"\n"\
	"void main()\n"\
	"{\n"\
	"\t// Transform to clip coordinates (-1,1, -1,1).\n"\
	"\tgl_Position.x = (position.x*2/width) - 1;\n"\
	"\tgl_Position.y = 1 - (position.y*2/height);\n"\
	"\tgl_Position.z = 0.0;\n"\
	"\tgl_Position.w = 1.0;\n"\
	"\tfp_tex_coord = tex_coord;\n"\
	"}\n"

#define BASICTEX_FRAG \
	"#version 140\n"\
	"\n"\
	"in vec2 fp_tex_coord;\n"\
	"\n"\
	"out vec4 output_colour;\n"\
	"\n"\
	"uniform sampler2D tex;\n"\
	"uniform vec4 colour;\n"\
	"\n"\
	"void main()\n"\
	"{\n"\
	"\toutput_colour = colour;\n"\
	"\toutput_colour.a = texture(tex, fp_tex_coord).r;\n"\
	"}\n"

using namespace IPDF;
using namespace std;

static void opengl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* msg, const void *data)
{
	Error("OpenGL Error (%d): %s", id, msg);
}


Screen::Screen()
{
	SDL_Init(SDL_INIT_VIDEO);
	m_window = SDL_CreateWindow("IPDF", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

	if (!m_window)
	{
		Fatal("Couldn't create window!");
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	m_gl_context = SDL_GL_CreateContext(m_window);

	ogl_LoadFunctions();

	// Why is this so horribly broken?
	if (ogl_IsVersionGEQ(3,0))
	{
		Fatal("We require OpenGL 3.1, but you have version %d.%d!",ogl_GetMajorVersion(), ogl_GetMinorVersion());
	}

	if (!SDL_GL_ExtensionSupported("GL_ARB_shading_language_420pack"))
	{
		Fatal("Your system does not support the ARB_shading_language_420pack extension, which is required.");
	}

	if (!SDL_GL_ExtensionSupported("GL_ARB_explicit_attrib_location"))
	{
		Fatal("Your system does not support the ARB_explicit_attrib_location extension, which is required.");
	}

	m_frame_begin_time = SDL_GetPerformanceCounter();
	m_last_frame_time = 0;
	m_last_frame_gpu_timer = 0;
	glGenQueries(1, &m_frame_gpu_timer);
	glBeginQuery(GL_TIME_ELAPSED, m_frame_gpu_timer);

	glDebugMessageCallback(opengl_debug_callback, 0);

	GLuint default_vao;
	glGenVertexArrays(1, &default_vao);
	glBindVertexArray(default_vao);

	//TODO: Error checking.
	m_texture_prog.AttachVertexProgram(BASICTEX_VERT);
	m_texture_prog.AttachFragmentProgram(BASICTEX_FRAG);
	m_texture_prog.Link();
	m_texture_prog.Use();

	// We always want to use the texture bound to texture unit 0.
	GLint texture_uniform_location = m_texture_prog.GetUniformLocation("tex");
	glUniform1i(texture_uniform_location, 0);

	m_colour_uniform_location = m_texture_prog.GetUniformLocation("colour");

	m_viewport_ubo.SetUsage(GraphicsBuffer::BufferUsageDynamicDraw);
	m_viewport_ubo.SetType(GraphicsBuffer::BufferTypeUniform);

	m_debug_font_atlas = 0;

	ResizeViewport(800, 600);

	Clear();
	Present();
	

}

Screen::~Screen()
{
	SDL_GL_DeleteContext(m_gl_context);
	SDL_DestroyWindow(m_window);
	SDL_Quit();
}

void Screen::Clear(float r, float g, float b, float a)
{
	glClearColor(r,g,b,a);
	glClear(GL_COLOR_BUFFER_BIT);
	DebugFontClear();
}

void Screen::ResizeViewport(int width, int height)
{
	glViewport(0, 0, width, height);
	m_viewport_width = width;
	m_viewport_height = height;
	GLfloat viewportfloats[] = {(float)width, (float)height};
	m_viewport_ubo.Upload(sizeof(float)*2, viewportfloats);
}

bool Screen::PumpEvents()
{
	SDL_Event evt;
	bool no_quit_requested = true;
	while (SDL_PollEvent(&evt))
	{
		switch (evt.type)
		{
		case SDL_QUIT:
			no_quit_requested = false;
			break;
		case SDL_WINDOWEVENT:
			switch (evt.window.event)
			{
			case SDL_WINDOWEVENT_RESIZED:
			case SDL_WINDOWEVENT_SIZE_CHANGED:
				ResizeViewport(evt.window.data1, evt.window.data2);
				break;
			}
			break;
		case SDL_MOUSEMOTION:
			m_last_mouse_x = evt.motion.x;
			m_last_mouse_y = evt.motion.y;
			if (m_mouse_handler)
			{
				m_mouse_handler(evt.motion.x, evt.motion.y,evt.motion.state, 0);
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			m_last_mouse_x = evt.button.x;
			m_last_mouse_y = evt.button.y;
			if (m_mouse_handler)
			{
				m_mouse_handler(evt.button.x, evt.button.y, evt.button.state?evt.button.button:0, 0);
			}
			break;
		case SDL_MOUSEWHEEL:
			if (m_mouse_handler)
			{
				m_mouse_handler(m_last_mouse_x, m_last_mouse_y, 0, evt.wheel.y);
			}
			break;
		case SDL_KEYDOWN:
		{
			Debug("Key %c down", (char)evt.key.keysym.sym);
			if (isalnum((char)evt.key.keysym.sym))
			{
				char filename[] = "0.bmp";
				filename[0] = (char)evt.key.keysym.sym;
				ScreenShot(filename);
			}
			break;
		}
		default:
			break;
		}
	}
	return no_quit_requested;
}

void Screen::SetMouseCursor(Screen::MouseCursors cursor)
{
	SDL_SystemCursor system_cursor_id = SDL_SYSTEM_CURSOR_ARROW;
	switch (cursor)
	{
	case CursorArrow: system_cursor_id = SDL_SYSTEM_CURSOR_ARROW; break;
	case CursorWait: system_cursor_id = SDL_SYSTEM_CURSOR_WAIT; break;
	case CursorWaitArrow: system_cursor_id = SDL_SYSTEM_CURSOR_WAITARROW; break;
	case CursorMove: system_cursor_id = SDL_SYSTEM_CURSOR_SIZEALL; break;
	case CursorHand: system_cursor_id = SDL_SYSTEM_CURSOR_HAND; break;
	default: break;
	}
	SDL_Cursor *system_cursor = SDL_CreateSystemCursor(system_cursor_id);
	SDL_SetCursor(system_cursor);
	//TODO: Check if we need to free the system cursors.
}

void Screen::Present()
{
	if (m_debug_font_atlas)
		DebugFontFlush();
	m_last_frame_time = SDL_GetPerformanceCounter() - m_frame_begin_time;
	glEndQuery(GL_TIME_ELAPSED);
	SDL_GL_SwapWindow(m_window);
	m_frame_begin_time = SDL_GetPerformanceCounter();
	if (m_last_frame_gpu_timer)
		glDeleteQueries(1, &m_last_frame_gpu_timer);
	m_last_frame_gpu_timer = m_frame_gpu_timer;
	glGenQueries(1, &m_frame_gpu_timer);
	glBeginQuery(GL_TIME_ELAPSED, m_frame_gpu_timer);
}

double Screen::GetLastFrameTimeGPU() const
{
	if (!m_last_frame_gpu_timer)
		return 0;
	uint64_t frame_time_ns;
	glGetQueryObjectui64v(m_last_frame_gpu_timer, GL_QUERY_RESULT, &frame_time_ns);
	return frame_time_ns/1000000000.0;
}

void Screen::ScreenShot(const char * filename) const
{
	Debug("Attempting to save BMP to file %s", filename);

	int w = ViewportWidth();
	int h = ViewportHeight();
	unsigned char * pixels = new unsigned char[w*h*4];
	if (pixels == NULL)
		Fatal("Failed to allocate %d x %d x 4 = %d pixel array", w, h, w*h*4);

	for (int y = 0; y < h; ++y)
	{
		glReadPixels(0,h-y-1,w, 1, GL_RGBA, GL_UNSIGNED_BYTE, &pixels[y*w*4]);
	}

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	SDL_Surface * surf = SDL_CreateRGBSurfaceFrom(pixels, w, h, 8*4, w*4, 0x000000ff,0x0000ff00,0x00ff0000,0xff000000);
#else
	SDL_Surface * surf = SDL_CreateRGBSurfaceFrom(pixels, w, h, 8*4, w*4, 0xff000000,0x00ff0000,0x0000ff00,0x000000ff);
#endif
	if (surf == NULL)
		Fatal("Failed to create SDL_Surface from pixel data - %s", SDL_GetError());

	GLenum texture_format = (surf->format->Rmask == 0x000000FF) ? GL_RGBA : GL_BGRA;
	Debug("SDL_Surface %d BytesPerPixel, format %d (RGB = %d, BGR = %d, RGBA = %d, BGRA = %d)", surf->format->BytesPerPixel, texture_format, GL_RGB, GL_BGR, GL_RGBA, GL_BGRA);

	if (SDL_SaveBMP(surf, filename) != 0)
		Fatal("SDL_SaveBMP failed - %s", SDL_GetError());
	
	SDL_FreeSurface(surf);
	delete [] pixels;
	Debug("Succeeded!");
}

/**
 * Render a BMP
 * NOT PART OF THE DOCUMENT FORMAT
 */
void Screen::RenderBMP(const char * filename) const
{
	if (access(filename, R_OK) == -1)
	{
		Error("No such file \"%s\" - Nothing to render - You might have done this deliberately?", filename);
		return;
	}
	SDL_Surface * bmp = SDL_LoadBMP(filename);
	if (bmp == NULL)
		Fatal("Failed to load BMP from %s - %s", filename, SDL_GetError());

	int w = bmp->w;
	int h = bmp->h;

	GLenum texture_format; 
	switch (bmp->format->BytesPerPixel)
	{
		case 4: //contains alpha
			texture_format = (bmp->format->Rmask == 0x000000FF) ? GL_RGBA : GL_BGRA;
			break;
		case 3: //does not contain alpha
			texture_format = (bmp->format->Rmask == 0x000000FF) ? GL_RGB : GL_BGR;	
			break;
		default:
			Fatal("Could not understand SDL_Surface format (%d colours)", bmp->format->BytesPerPixel);
			break;	
	}

	//Debug("SDL_Surface %d BytesPerPixel, format %d (RGB = %d, BGR = %d, RGBA = %d, BGRA = %d)", bmp->format->BytesPerPixel, texture_format, GL_RGB, GL_BGR, GL_RGBA, GL_BGRA);

	m_texture_prog.Use();
	GraphicsBuffer quad_vertex_buffer;
	quad_vertex_buffer.SetUsage(GraphicsBuffer::BufferUsageStaticDraw);
	quad_vertex_buffer.SetType(GraphicsBuffer::BufferTypeVertex);
	GLfloat quad[] = { 
		0, 0, 0, 0,
		1, 0, (float)ViewportWidth(), 0,
		1, 1, (float)ViewportWidth(), (float)ViewportHeight(),
		0, 1, 0, (float)ViewportHeight()
	};
	quad_vertex_buffer.Upload(sizeof(GLfloat) * 16, quad);
	quad_vertex_buffer.Bind();
	m_viewport_ubo.Bind();

	glUniform4f(m_colour_uniform_location, 1.0f, 1.0f, 1.0f, 1.0f);

	GLuint texID;
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexImage2D(GL_TEXTURE_2D, 0, bmp->format->BytesPerPixel, w, h, 0, texture_format, GL_UNSIGNED_BYTE, bmp->pixels);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float)));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), 0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	SDL_FreeSurface(bmp);	
}

void Screen::DebugFontInit(const char *name, float font_size)
{
	unsigned char font_atlas_data[1024*1024];
	FILE *font_file = fopen(name, "rb");
	fseek(font_file, 0, SEEK_END);
	size_t font_file_size = ftell(font_file);
	fseek(font_file, 0, SEEK_SET);
	unsigned char *font_file_data = (unsigned char*)malloc(font_file_size);
	fread(font_file_data, 1, font_file_size, font_file);
	fclose(font_file);
	stbtt_BakeFontBitmap(font_file_data,0, font_size, font_atlas_data,1024,1024, 32,96, m_debug_font_rects);
	free(font_file_data);
	glGenTextures(1, &m_debug_font_atlas);
	glBindTexture(GL_TEXTURE_2D, m_debug_font_atlas);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 1024,1024, 0, GL_RED, GL_UNSIGNED_BYTE, font_atlas_data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	m_debug_font_size = font_size;

	m_debug_font_vertices.SetUsage(GraphicsBuffer::BufferUsageStreamDraw);
	m_debug_font_vertices.SetType(GraphicsBuffer::BufferTypeVertex);
	m_debug_font_vertices.Upload(8192, nullptr);
	m_debug_font_vertex_head = 0;

	m_debug_font_indices.SetUsage(GraphicsBuffer::BufferUsageStreamDraw);
	m_debug_font_indices.SetType(GraphicsBuffer::BufferTypeIndex);
	m_debug_font_indices.Resize(500);
	m_debug_font_index_head = 0;
}

void Screen::DebugFontClear()
{
	m_debug_font_x = m_debug_font_y = 0;
	if (!m_debug_font_atlas) return;
	DebugFontPrint("\n");
}

void Screen::DebugFontFlush()
{
	
		
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindTexture(GL_TEXTURE_2D, m_debug_font_atlas);

	m_texture_prog.Use();
	m_viewport_ubo.Bind();
	m_debug_font_vertices.Bind();
	m_debug_font_indices.Bind();
	glUniform4f(m_colour_uniform_location, 0,0,0,1);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(65535);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float)));
	glDrawElements(GL_TRIANGLE_STRIP, m_debug_font_index_head, GL_UNSIGNED_SHORT, 0);
	glDisable(GL_PRIMITIVE_RESTART);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);

	glDisable(GL_BLEND);

	m_debug_font_vertex_head = 0;
	m_debug_font_index_head = 0;
}

void Screen::DebugFontPrint(const char* str)
{
	if (!m_debug_font_atlas) return;

	struct fontvertex
	{
		float x, y, s, t;
	};

	BufferBuilder<fontvertex> vertexData(m_debug_font_vertices.MapRange(m_debug_font_vertex_head*sizeof(float), m_debug_font_vertices.GetSize() - m_debug_font_vertex_head*sizeof(float), false, true, true), m_debug_font_vertices.GetSize() - m_debug_font_vertex_head*sizeof(float));
	BufferBuilder<uint16_t> indexData(m_debug_font_indices.MapRange(m_debug_font_index_head*sizeof(uint16_t), m_debug_font_indices.GetSize() - m_debug_font_index_head*sizeof(uint16_t), false, true, true), m_debug_font_indices.GetSize() - m_debug_font_index_head*sizeof(uint16_t));

	size_t baseVertex = m_debug_font_vertex_head/4;
	while (*str) {
		if (!vertexData.Free(4) || !indexData.Free(5))
		{
			m_debug_font_indices.UnMap();
			m_debug_font_vertices.UnMap();
			DebugFontFlush();
			DebugFontPrint(str);
			return;
		}
		if (*str >= 32 && *str < 128) {
			stbtt_aligned_quad q;
			stbtt_GetBakedQuad(m_debug_font_rects, 1024,1024, *str-32, &m_debug_font_x,&m_debug_font_y,&q,1);
			size_t index = vertexData.Add({q.x0, q.y0, q.s0, q.t0});
			index += baseVertex;
			indexData.Add(index);
			index = vertexData.Add({q.x1, q.y0, q.s1, q.t0});
			index += baseVertex;
			indexData.Add(index);
			index = vertexData.Add({q.x0, q.y1, q.s0, q.t1});
			index += baseVertex;
			indexData.Add(index);
			index = vertexData.Add({q.x1, q.y1, q.s1, q.t1});
			index += baseVertex;
			indexData.Add(index);
			indexData.Add(65535);

			m_debug_font_vertex_head += 16;
			m_debug_font_index_head += 5;

		}
		else if (*str == '\n')
		{
			m_debug_font_x = 0;
			m_debug_font_y += m_debug_font_size;
		}
		++str;
	}
	m_debug_font_indices.UnMap();
	m_debug_font_vertices.UnMap();
	//DebugFontFlush();
}

void Screen::DebugFontPrintF(const char *fmt, ...)
{
	char buffer[BUFSIZ];
	va_list va;
	va_start(va, fmt);
	vsnprintf(buffer, BUFSIZ, fmt,va);
	va_end(va);
	DebugFontPrint(buffer);
}
