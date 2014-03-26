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
	glOrtho(Float(m_bounds.x), Float(m_bounds.x)+Float(m_bounds.w), Float(m_bounds.y) + Float(m_bounds.h), Float(m_bounds.y), -1.f, 1.f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glColor4f(0.f,0.f,0.f,1.f);
	glBegin(GL_QUADS);
	for (unsigned id = 0; id < m_document.ObjectCount(); ++id)
	{
		if (m_document.m_objects.types[id] == RECT_FILLED)
			continue;
		Rect obj_bounds = m_document.m_objects.bounds[id];
		glVertex2f(Float(obj_bounds.x), Float(obj_bounds.y));
		glVertex2f(Float(obj_bounds.x) + Float(obj_bounds.w), Float(obj_bounds.y));
		glVertex2f(Float(obj_bounds.x) + Float(obj_bounds.w), Float(obj_bounds.y) + Float(obj_bounds.h));
		glVertex2f(Float(obj_bounds.x), Float(obj_bounds.y) + Float(obj_bounds.h));
	}
	glEnd();

	for (unsigned id = 0; id < m_document.ObjectCount(); ++id)
	{
		if (m_document.m_objects.types[id] == RECT_OUTLINE)
			continue;
		Rect obj_bounds = m_document.m_objects.bounds[id];
		glBegin(GL_LINE_LOOP);
		glVertex2f(Float(obj_bounds.x), Float(obj_bounds.y));
		glVertex2f(Float(obj_bounds.x) + Float(obj_bounds.w), Float(obj_bounds.y));
		glVertex2f(Float(obj_bounds.x) + Float(obj_bounds.w), Float(obj_bounds.y) + Float(obj_bounds.h));
		glVertex2f(Float(obj_bounds.x), Float(obj_bounds.y) + Float(obj_bounds.h));
		glEnd();
	}

}
