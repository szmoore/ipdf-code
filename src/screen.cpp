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

}

Screen::~Screen()
{
	SDL_GL_DeleteContext(m_gl_context);
	SDL_DestroyWindow(m_window);
	SDL_Quit();
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
	SDL_Surface * info = SDL_GetWindowSurface(m_window);
	if (info == NULL)
	{
		Fatal("Failed to create info surface from m_window - %s", SDL_GetError());
	}
	
	unsigned num_pix = info->w * info->h * info->format->BytesPerPixel;
	unsigned char * pixels = new unsigned char[num_pix];
	if (pixels == NULL)
	{
		Fatal("Failed to allocate %u pixel array - %s", num_pix, strerror(errno));
	}

	SDL_Renderer * renderer = SDL_GetRenderer(m_window);
	if (renderer == NULL)
	{
		Fatal("Couldn't get renderer from m_window - %s", SDL_GetError());
	}
	if (SDL_RenderReadPixels(renderer, &(info->clip_rect), info->format->format, pixels, info->w * info->format->BytesPerPixel) != 0)
	{
		Fatal("SDL_RenderReadPixels failed - %s", SDL_GetError());
	}

	// This line is disgusting
	SDL_Surface * save = SDL_CreateRGBSurfaceFrom(pixels, info->w, info->h, info->format->BitsPerPixel, info->w * info->format->BytesPerPixel, 
											info->format->Rmask, info->format->Gmask, info->format->Bmask, info->format->Amask);
	if (save == NULL)
	{
		Fatal("Couldn't create SDL_Surface from renderer pixel data - %s", SDL_GetError());
	}
	if (SDL_SaveBMP(save, filename) != 0)
	{
		Fatal("SDL_SaveBMP to %s failed - %s", filename, SDL_GetError());
	}

	//SDL_DestroyRenderer(renderer);
	SDL_FreeSurface(save);
	SDL_FreeSurface(info);
	delete [] pixels;

	Debug("Succeeded!");


}
