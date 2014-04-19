#include "common.h"
#include "screen.h"

#include "SDL_opengl.h"

using namespace IPDF;
using namespace std;

Screen::Screen()
{
	SDL_Init(SDL_INIT_VIDEO);
	m_window = SDL_CreateWindow("IPDF", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

	if (!m_window)
	{
		Fatal("Couldn't create window!");
	}

	m_gl_context = SDL_GL_CreateContext(m_window);

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
				m_mouse_handler(evt.button.x, evt.button.y, evt.button.state, 0);
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
	SDL_GL_SwapWindow(m_window);
}

void Screen::ScreenShot(const char * filename) const
{
	Debug("Attempting to save BMP to file %s", filename);

	int w = ViewportWidth();
	int h = ViewportHeight();
	unsigned char * pixels = new unsigned char[w*h*4];
	if (pixels == NULL)
		Fatal("Failed to allocate %d x %d x 4 = %d pixel array", w, h, w*h*4);
	glReadBuffer(GL_FRONT);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
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


	GLuint texID;
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexImage2D(GL_TEXTURE_2D, 0, bmp->format->BytesPerPixel, w, h, 0, texture_format, GL_UNSIGNED_BYTE, bmp->pixels);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, 1.0, 1.0, 0.0, -1.f, 1.f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glBegin(GL_QUADS);
		glTexCoord2i(0,0); glVertex2f(0,0);
		glTexCoord2i(1,0); glVertex2f(1,0);
		glTexCoord2i(1,1); glVertex2f(1,1);
		glTexCoord2i(0,1); glVertex2f(0,1);
	glEnd();

	glDisable(GL_TEXTURE_2D);
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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 1024,1024, 0, GL_ALPHA, GL_UNSIGNED_BYTE, font_atlas_data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	m_debug_font_size = font_size;
}

void Screen::DebugFontClear()
{
	m_debug_font_x = m_debug_font_y = 0;
	DebugFontPrint("\n");
}

void Screen::DebugFontPrint(const char* str)
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
 	glOrtho(0,ViewportWidth(), ViewportHeight(), 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	
	
	
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindTexture(GL_TEXTURE_2D, m_debug_font_atlas);
	glBegin(GL_QUADS);
	while (*str) {
		if (*str >= 32 && *str < 128) {
			stbtt_aligned_quad q;
			stbtt_GetBakedQuad(m_debug_font_rects, 1024,1024, *str-32, &m_debug_font_x,&m_debug_font_y,&q,1);
			glTexCoord2f(q.s0,q.t0); glVertex2f(q.x0,q.y0);
			glTexCoord2f(q.s1,q.t0); glVertex2f(q.x1,q.y0);
			glTexCoord2f(q.s1,q.t1); glVertex2f(q.x1,q.y1);
			glTexCoord2f(q.s0,q.t1); glVertex2f(q.x0,q.y1);
		}
		else if (*str == '\n')
		{
			m_debug_font_x = 0;
			m_debug_font_y += m_debug_font_size;
		}
		++str;
	}
	glEnd();
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
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
