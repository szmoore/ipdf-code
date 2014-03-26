#include "document.h"

using namespace IPDF;
using namespace std;

void Document::Load(const string & filename)
{
	m_objects.bounds.clear();
	m_count = 0;
	if (filename == "")
		return;
}

void Document::Add(Real x, Real y, Real w, Real h)
{
	m_objects.bounds.push_back(Rect(x, y, w, h));
	m_count++;
}
