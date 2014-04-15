#ifndef _SCREEN_H
#define _SCREEN_H

#include <SDL.h>

#include <functional>

namespace IPDF
{
	/*
	 * The "Screen" class handles managing the OS window (using SDL2).
	 */
	class Screen
	{
	public:
		Screen();
		~Screen();

		// 'Pumps' the system event queue.
		// Returns 'false' if the program should quit.
		bool PumpEvents();

		// Finishes rendering a frame, and presents it on the screen.
		void Present();

		// Get the current width/height of the window's viewport.
		int ViewportWidth() const { return m_viewport_width; }
		int ViewportHeight() const { return m_viewport_height; }
		
		// Handle mouse input.
		typedef std::function<void(int x, int y, int button, int wheel)> MouseHandler;
		void SetMouseHandler(MouseHandler handler)
		{
			m_mouse_handler = handler;
		}
		
		enum MouseCursors
		{
			CursorArrow,
			CursorWait,
			CursorWaitArrow,
			CursorMove,
			CursorHand
		};
		void SetMouseCursor(MouseCursors cursor);

		void ScreenShot(const char * filename) const;
	private:
		void ResizeViewport(int width, int height);
		
		MouseHandler m_mouse_handler;
		int m_last_mouse_x;
		int m_last_mouse_y;

		int m_viewport_width;
		int m_viewport_height;
		SDL_Window *m_window;
		SDL_GLContext m_gl_context;
	};

}

#endif // _SCREEN_H
