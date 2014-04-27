#ifndef _SCREEN_H
#define _SCREEN_H

#include <SDL.h>

#include <functional>

#include "stb_truetype.h"
#include "graphicsbuffer.h"
#include "shaderprogram.h"

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

		// Clears the screen to a given colour.
		void Clear(float r=1.0, float g=1.0, float b=1.0, float a=1.0);

		// Finishes rendering a frame, and presents it on the screen.
		void Present();

		// Get the current width/height of the window's viewport.
		int ViewportWidth() const { return m_viewport_width; }
		int ViewportHeight() const { return m_viewport_height; }
		
		// Debug Font handling
		void DebugFontInit(const char *font_name, float font_size = 12);
		void DebugFontClear();
		void DebugFontPrint(const char *str);
		void DebugFontPrintF(const char *fmt, ...);
		
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
		void RenderBMP(const char * filename) const;
	private:
		void ResizeViewport(int width, int height);
		void DebugFontFlush();
		
		MouseHandler m_mouse_handler;
		int m_last_mouse_x;
		int m_last_mouse_y;

		int m_viewport_width;
		int m_viewport_height;
		SDL_Window *m_window;
		SDL_GLContext m_gl_context;
		ShaderProgram m_texture_prog;
		GLint m_colour_uniform_location;
		GraphicsBuffer m_viewport_ubo;
		stbtt_bakedchar m_debug_font_rects[96];
		unsigned int m_debug_font_atlas;
		float m_debug_font_x;
		float m_debug_font_y;
		float m_debug_font_size;
		GraphicsBuffer m_debug_font_vertices;
		GraphicsBuffer m_debug_font_indices;
		int m_debug_font_vertex_head;
		int m_debug_font_index_head;
	};

}

#endif // _SCREEN_H
