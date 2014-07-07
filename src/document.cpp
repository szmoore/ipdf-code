#include "document.h"
#include "bezier.h"
#include <cstdio>

using namespace IPDF;
using namespace std;

//TODO: Make this work for variable sized Reals

// Loads an std::vector<T> of size num_elements from a file.
template<typename T>
static void LoadStructVector(FILE *src_file, size_t num_elems, std::vector<T>& dest)
{
	size_t structsread = 0;
	dest.resize(num_elems);
	structsread = fread(dest.data(), sizeof(T), num_elems, src_file);
	if (structsread != num_elems)
		Fatal("Only read %u structs (expected %u)!", structsread, num_elems);
}

// Saves an std::vector<T> to a file. Size must be saves separately.
template<typename T>
static void SaveStructVector(FILE *dst_file, std::vector<T>& src)
{
	size_t written = 0;
	written = fwrite(src.data(), sizeof(T), src.size(), dst_file);
	if (written != src.size())
		Fatal("Only wrote %u structs (expected %u)!", written, src.size());
}

static void WriteChunkHeader(FILE *dst_file, DocChunkTypes type, uint32_t size)
{
	size_t written = 0;
	written = fwrite(&type, sizeof(type), 1, dst_file);
	if (written != 1)
		Fatal("Could not write Chunk header! (ID)");
	written = fwrite(&size, sizeof(size), 1, dst_file);
	if (written != 1)
		Fatal("Could not write Chunk header (size)!");
}

static bool ReadChunkHeader(FILE *src_file, DocChunkTypes& type, uint32_t& size)
{
	if (fread(&type, sizeof(DocChunkTypes), 1, src_file) != 1)
		return false;
	if (fread(&size, sizeof(uint32_t), 1, src_file) != 1)
		return false;
	return true;
}

void Document::Save(const string & filename)
{
	Debug("Saving document to file \"%s\"...", filename.c_str());
	FILE * file = fopen(filename.c_str(), "w");
	if (file == NULL)
		Fatal("Couldn't open file \"%s\" - %s", filename.c_str(), strerror(errno));

	size_t written;
	Debug("Number of objects (%u)...", ObjectCount());
	WriteChunkHeader(file, CT_NUMOBJS, sizeof(m_count));
	written = fwrite(&m_count, sizeof(m_count), 1, file);
	if (written != 1)
		Fatal("Failed to write number of objects!");

	Debug("Object types...");
	WriteChunkHeader(file, CT_OBJTYPES, m_objects.types.size() * sizeof(ObjectType));
	SaveStructVector<ObjectType>(file, m_objects.types);

	Debug("Object bounds...");
	WriteChunkHeader(file, CT_OBJBOUNDS, m_objects.bounds.size() * sizeof(Rect));
	SaveStructVector<Rect>(file, m_objects.bounds);

	Debug("Object data indices...");
	WriteChunkHeader(file, CT_OBJINDICES, m_objects.data_indices.size() * sizeof(unsigned));
	SaveStructVector<unsigned>(file, m_objects.data_indices);
	
	Debug("Bezier data...");
	WriteChunkHeader(file, CT_OBJBEZIERS, m_objects.beziers.size() * sizeof(uint8_t));
	SaveStructVector<Bezier>(file, m_objects.beziers);

	int err = fclose(file);
	if (err != 0)
		Fatal("Failed to close file \"%s\" - %s", filename.c_str(), strerror(err));

	Debug("Successfully saved %u objects to \"%s\"", ObjectCount(), filename.c_str());
}

#ifndef QUADTREE_DISABLED

void Document::GenBaseQuadtree()
{
	m_quadtree.nodes.push_back(QuadTreeNode{QUADTREE_EMPTY, QUADTREE_EMPTY, QUADTREE_EMPTY, QUADTREE_EMPTY, QUADTREE_EMPTY, QTC_UNKNOWN, 0, ObjectCount()});
	m_quadtree.root_id = 0;
}

#endif

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

	DocChunkTypes chunk_type;
	uint32_t chunk_size;
	while (ReadChunkHeader(file, chunk_type, chunk_size))
	{
		switch(chunk_type)
		{
		case CT_NUMOBJS:
			read = fread(&m_count, sizeof(m_count), 1, file);
			if (read != 1)
				Fatal("Failed to read number of objects!");
			Debug("Number of objects: %u", ObjectCount());
			break;
		case CT_OBJTYPES:
			Debug("Object types...");
			LoadStructVector<ObjectType>(file, chunk_size/sizeof(ObjectType), m_objects.types);
			break;
		case CT_OBJBOUNDS:
			Debug("Object bounds...");
			LoadStructVector<Rect>(file, chunk_size/sizeof(Rect), m_objects.bounds);
			break;
		case CT_OBJINDICES:
			Debug("Object data indices...");
			LoadStructVector<unsigned>(file, chunk_size/sizeof(unsigned), m_objects.data_indices);
			break;
		case CT_OBJBEZIERS:
			Debug("Bezier data...");
			LoadStructVector<Bezier>(file, chunk_size/sizeof(Bezier), m_objects.beziers);
			break;
		}
	}
	Debug("Successfully loaded %u objects from \"%s\"", ObjectCount(), filename.c_str());
#ifndef QUADTREE_DISABLED
	if (m_quadtree.root_id == QUADTREE_EMPTY)
	{
		GenBaseQuadtree();
	}
#endif
}

void Document::Add(ObjectType type, const Rect & bounds, unsigned data_index)
{
	m_objects.types.push_back(type);
	m_objects.bounds.push_back(bounds);
	m_objects.data_indices.push_back(data_index);
	++m_count; // Why can't we just use the size of types or something?
}

unsigned Document::AddBezierData(const Bezier & bezier)
{
	m_objects.beziers.push_back(bezier);
	return m_objects.beziers.size()-1;
}


void Document::DebugDumpObjects()
{
	Debug("Objects for Document %p are:", this);
	for (unsigned id = 0; id < ObjectCount(); ++id)
	{
		Debug("%u. \tType: %u\tBounds: %s", id, m_objects.types[id], m_objects.bounds[id].Str().c_str());
	}
}

bool Document::operator==(const Document & equ) const
{
	return (ObjectCount() == equ.ObjectCount() 
		&& memcmp(m_objects.bounds.data(), equ.m_objects.bounds.data(), ObjectCount() * sizeof(Rect)) == 0
		&& memcmp(m_objects.data_indices.data(), equ.m_objects.data_indices.data(), ObjectCount() * sizeof(unsigned)) == 0
		&& memcmp(m_objects.beziers.data(), equ.m_objects.beziers.data(), m_objects.beziers.size() * sizeof(Bezier)) == 0);
}
