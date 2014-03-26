#include "document.h"

#include <cstdio>

using namespace IPDF;
using namespace std;

void Document::Save(const string & filename)
{
	Debug("Saving document to file \"%s\"...", filename.c_str());
	FILE * file = fopen(filename.c_str(), "w");
	if (file == NULL)
		Fatal("Couldn't open file \"%s\" - %s", filename.c_str(), strerror(errno));

	size_t written;
	Debug("Number of objects (%u)...", ObjectCount());
	written = fwrite(&m_count, sizeof(m_count), 1, file);
	if (written != 1)
		Fatal("Failed to write number of objects!");

	Debug("Object bounds...");
	written = fwrite(m_objects.bounds.data(), sizeof(Rect), m_objects.bounds.size(), file);
	if (written != ObjectCount())
		Fatal("Only wrote %u objects!", written);

	int err = fclose(file);
	if (err != 0)
		Fatal("Failed to close file \"%s\" - %s", filename.c_str(), strerror(err));

	Debug("Successfully saved %u objects to \"%s\"", ObjectCount(), filename.c_str());
}

void Document::Load(const string & filename)
{
	m_objects.bounds.clear();
	m_count = 0;
	if (filename == "")
	{
		Debug("Loaded empty document.");
		return;
	}
	Debug("Loading document from file \"%s\"", filename.c_str());
	FILE * file = fopen(filename.c_str(), "r");
	if (file == NULL)
		Fatal("Couldn't open file \"%s\"", filename.c_str(), strerror(errno));

	size_t read;
	read = fread(&m_count, sizeof(m_count), 1, file);
	if (read != 1)
		Fatal("Failed to read number of objects!");
	Debug("Number of objects: %u", ObjectCount());

	m_objects.bounds.resize(ObjectCount());
	
	Debug("Object bounds...");
	read = fread(m_objects.bounds.data(), sizeof(Rect), m_objects.bounds.size(), file);
	if (read != ObjectCount())
		Fatal("Only read %u objects!", read);
	
	Debug("Successfully loaded %u objects from \"%s\"", ObjectCount(), filename.c_str());
}

void Document::Add(Real x, Real y, Real w, Real h)
{
	m_objects.bounds.push_back(Rect(x, y, w, h));
	m_count++;
}

void Document::DebugDumpObjects()
{
	Debug("Objects for Document %p are:", this);
	for (unsigned id = 0; id < ObjectCount(); ++id)
	{
		Debug("%u.\t%s", id, m_objects.bounds[id].Str().c_str());
	}
}

bool Document::operator==(const Document & equ) const
{
	return (ObjectCount() == equ.ObjectCount() && memcmp(m_objects.bounds.data(), equ.m_objects.bounds.data(), ObjectCount() * sizeof(Rect)) == 0);
}
