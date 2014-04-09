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
			if (m_mouse_handler)
			{
				m_mouse_handler(evt.motion.x, evt.motion.y,evt.motion.state, 0);
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			if (m_mouse_handler)
			{
				m_mouse_handler(evt.button.x, evt.button.y, evt.button.state, 0);
			}
			break;
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

