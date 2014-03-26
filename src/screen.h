#ifndef _SCREEN_H
#define _SCREEN_H

#include <SDL.h>

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
		int ViewportWidth() { return m_viewport_width; }
		int ViewportHeight() { return m_viewport_height; }
	private:
		void ResizeViewport(int width, int height);

		int m_viewport_width;
		int m_viewport_height;
		SDL_Window *m_window;
		SDL_GLContext m_gl_context;
	};
}

#endif // _SCREEN_H
