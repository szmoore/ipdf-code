#include "view.h"

#include "SDL_opengl.h"

using namespace IPDF;
using namespace std;

void View::Render()
{
	static bool debug_output_done = false;
	if (!debug_output_done)
	{
		m_document.DebugDumpObjects();
		debug_output_done = true;
	}

	glClearColor(1.f,1.f,1.f,1.f);
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(m_bounds.x, m_bounds.x+m_bounds.w, m_bounds.y + m_bounds.h, m_bounds.y, -1.f, 1.f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glColor4f(0.f,0.f,0.f,1.f);
	glBegin(GL_QUADS);
	for (unsigned id = 0; id < m_document.ObjectCount(); ++id)
	{
		Rect obj_bounds = m_document.m_objects.bounds[id];
		glVertex2f(obj_bounds.x, obj_bounds.y);
		glVertex2f(obj_bounds.x + obj_bounds.w, obj_bounds.y);
		glVertex2f(obj_bounds.x + obj_bounds.w, obj_bounds.y + obj_bounds.h);
		glVertex2f(obj_bounds.x, obj_bounds.y + obj_bounds.h);
	}
	glEnd();

}
