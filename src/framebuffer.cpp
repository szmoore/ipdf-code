#include "framebuffer.h"
#define GL_GLEXT_PROTOTYPES
#include <SDL_opengl.h>
#include <GL/glext.h>

using namespace IPDF;

void FrameBuffer::Create(int w, int h)
{
	if (m_render_texture)
	{
		Destroy();
	}
	m_width = w;
	m_height = h;
	glGenTextures(1, &m_render_texture);
	glGenFramebuffers(1, &m_render_fbo);

	glBindTexture(GL_TEXTURE_2D, m_render_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	glBindFramebuffer(GL_FRAMEBUFFER, m_render_fbo);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_render_texture, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void FrameBuffer::Destroy()
{
	if (!m_render_texture) return;
	glDeleteFramebuffers(1, &m_render_fbo);
	glDeleteTextures(1, &m_render_texture);
}

void FrameBuffer::Bind()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_render_fbo);
}

void FrameBuffer::UnBind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::Blit()
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_render_fbo);
	glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void FrameBuffer::Clear(float r, float g, float b, float a)
{
	Bind();
	glClearColor(r,g,b,a);
	glClear(GL_COLOR_BUFFER_BIT);
}

