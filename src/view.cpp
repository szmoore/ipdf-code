#include "view.h"

using namespace IPDF;
using namespace std;

void View::Render()
{
	Debug("Bounds are %s", m_bounds.Str().c_str());
	Debug("Objects are:");
	for (unsigned id = 0; id < m_document.ObjectCount(); ++id)
	{
		Debug("%u\t%s", id, m_document.m_objects.bounds[id].Str().c_str());
	}
}
