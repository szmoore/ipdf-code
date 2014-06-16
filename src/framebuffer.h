#ifndef _FRAMEBUFFER_H
#define _FRAMEBUFFER_H

#include <SDL.h>
#include "gl_core44.h"


namespace IPDF
{
	/**
	 * The "FrameBuffer" class represents an offscreen render target. 
	 * FrameBuffer::Create needs to be called to initialise it; constructor is trivial
	 */
	class FrameBuffer
	{
	public:
		FrameBuffer() : m_render_texture(0), m_render_fbo(0), m_width(0), m_height(0) {}
		~FrameBuffer() { Destroy(); }
		void Create(int w, int h);
		void Destroy();
		void Bind(); // set as render target
		void UnBind(); // set render target to screen
		void Blit(); // blit this FrameBuffer to current render target
		void Clear(float r=1.0, float g=1.0, float b=1.0, float a=1.0);
		int GetWidth() { return m_width; }
		int GetHeight() { return m_height; }
	private:
		GLuint m_render_texture;
		GLuint m_render_fbo;
		int m_width;
		int m_height;
	};

}

#endif // _SCREEN_H
