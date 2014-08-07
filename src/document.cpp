#include "document.h"
#include "bezier.h"
#include <cstdio>
#include <fstream>

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
	GenQuadNode(0, QTC_TOP_LEFT);
}

QuadTreeIndex Document::GenQuadNode(QuadTreeIndex parent, QuadTreeNodeChildren type)
{
	QuadTreeIndex new_index = m_quadtree.nodes.size();
	m_quadtree.nodes.push_back(QuadTreeNode{QUADTREE_EMPTY, QUADTREE_EMPTY, QUADTREE_EMPTY, QUADTREE_EMPTY, parent, type, 0, 0});

	m_quadtree.nodes[new_index].object_begin = m_objects.bounds.size();
	for (unsigned i = m_quadtree.nodes[parent].object_begin; i < m_quadtree.nodes[parent].object_end; ++i)
	{
		if (ContainedInQuadChild(m_objects.bounds[i], type))
		{
			m_objects.bounds.push_back(TransformToQuadChild(m_objects.bounds[i], type));
			m_objects.types.push_back(m_objects.types[i]);
			m_objects.data_indices.push_back(m_objects.data_indices[i]);
			m_count++;
		}
	}
	m_quadtree.nodes[new_index].object_end = m_objects.bounds.size();
	switch (type)
	{
		case QTC_TOP_LEFT:
			m_quadtree.nodes[parent].top_left = new_index;
			break;
		case QTC_TOP_RIGHT:
			m_quadtree.nodes[parent].top_right = new_index;
			break;
		case QTC_BOTTOM_LEFT:
			m_quadtree.nodes[parent].bottom_left = new_index;
			break;
		case QTC_BOTTOM_RIGHT:
			m_quadtree.nodes[parent].bottom_right = new_index;
			break;
		default:
			Fatal("Tried to add a QuadTree child of invalid type!");
	}
	return new_index;
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


#include "../contrib/pugixml-1.4/src/pugixml.hpp"
#include "../contrib/pugixml-1.4/src/pugixml.cpp"

/**
 * Load an SVG into a rectangle
 */
void Document::LoadSVG(const string & filename, const Rect & bounds)
{
	using namespace pugi;
	
	xml_document doc_xml;
	ifstream input(filename.c_str(), ios_base::in);
	xml_parse_result result = doc_xml.load(input);
	
	if (!result)
		Fatal("Couldn't load \"%s\" - %s", filename.c_str(), result.description());
		
	Debug("Loaded XML - %s", result.description());
	
	input.close();

	// Combine all SVG tags into one thing because lazy
	for (xml_node svg : doc_xml.children("svg"))
	{
		Real width = svg.attribute("width").as_float() * bounds.w;
		Real height = svg.attribute("width").as_float() * bounds.h;
		
		
		// Rectangles
		Real coords[4];
		const char * attrib_names[] = {"x", "y", "width", "height"};
		for (pugi::xml_node rect : svg.children("rect"))
		{
			for (size_t i = 0; i < 4; ++i)
				coords[i] = rect.attribute(attrib_names[i]).as_float();
			
			bool outline = !(rect.attribute("fill"));
			Add(outline?RECT_OUTLINE:RECT_FILLED, Rect(coords[0]/width + bounds.x, coords[1]/height + bounds.y, coords[2]/width, coords[3]/height),0);
			Debug("Added rectangle");
		}		
		
		// Circles
		for (pugi::xml_node circle : svg.children("circle"))
		{
			Real cx = circle.attribute("cx").as_float();
			Real cy = circle.attribute("cy").as_float();
			Real r = circle.attribute("r").as_float();
			
			Real x = (cx - r)/width + bounds.x; 
			Real y = (cy - r)/height + bounds.y; 
			Real w = 2*r/width; 
			Real h = 2*r/height;
			
			Rect rect(x,y,w,h);
			Add(CIRCLE_FILLED, rect,0);
			Debug("Added Circle %s", rect.Str().c_str());

		}		
		
		// paths
		for (pugi::xml_node path : svg.children("path"))
		{
			
			string d = path.attribute("d").as_string();
			Debug("Path data attribute is \"%s\"", d.c_str());
			AddPathFromString(d, Rect(bounds.x,bounds.y,width,height));
			
		}
	}
	
	//Fatal("Done");
	
	

}

// Behold my amazing tokenizing abilities
static string & GetToken(const string & d, string & token, unsigned & i)
{
	token.clear();
	while (i < d.size() && iswspace(d[i]))
	{
		++i;
	}
	
	while (i < d.size())
	{
		if (d[i] == ',' || (isalpha(d[i]) && d[i] != 'e') || iswspace(d[i]))
		{
			if (token.size() == 0 && !iswspace(d[i]))
			{
				token += d[i++];
			}
			break;	
		}
		token += d[i++];
	}
	Debug("Got token \"%s\"", token.c_str());
	return token;
}


// Fear the wrath of the tokenizing svg data
// Seriously this isn't really very DOM-like at all is it?
void Document::AddPathFromString(const string & d, const Rect & bounds)
{
	Real x[4] = {0,0,0,0};
	Real y[4] = {0,0,0,0};
	
	string token("");
	string command("m");
	
	Real x0(0);
	Real y0(0);
	
	unsigned i = 0;
	unsigned prev_i = 0;
	
	bool start = false;
	
	while (i < d.size() && GetToken(d, token, i).size() > 0)
	{
		if (isalpha(token[0]))
			command = token;
		else
		{
			i = prev_i; // hax
			if(command == "")
				command = "L";
		}
		
		bool relative = islower(command[0]);
			
		if (command == "m" || command == "M")
		{
			Debug("Construct moveto command");
			Real dx = strtod(GetToken(d,token,i).c_str(),NULL) / bounds.w;
			assert(GetToken(d,token,i) == ",");
			Real dy = strtod(GetToken(d,token,i).c_str(),NULL) / bounds.h;
			
			x[0] = (relative) ? x[0] + dx : dx;
			y[0] = (relative) ? y[0] + dy : dy;
			

			
			Debug("mmoveto %f,%f", Float(x[0]),Float(y[0]));
			command = (command == "m") ? "l" : "L";
		}
		else if (command == "c" || command == "C" || command == "q" || command == "Q")
		{
			Debug("Construct curveto command");
			Real dx = strtod(GetToken(d,token,i).c_str(),NULL)/bounds.w;
			assert(GetToken(d,token,i) == ",");
			Real dy = strtod(GetToken(d,token,i).c_str(),NULL)/bounds.h;
			
			x[1] = (relative) ? x[0] + dx : dx;
			y[1] = (relative) ? y[0] + dy : dy;
			
			dx = strtod(GetToken(d,token,i).c_str(),NULL) / bounds.w;
			assert(GetToken(d,token,i) == ",");
			dy = strtod(GetToken(d,token,i).c_str(),NULL) / bounds.h;
			
			x[2] = (relative) ? x[0] + dx : dx;
			y[2] = (relative) ? y[0] + dy : dy;
			
			if (command != "q" && command != "Q")
			{
				dx = strtod(GetToken(d,token,i).c_str(),NULL) / bounds.w;
				assert(GetToken(d,token,i) == ",");
				dy = strtod(GetToken(d,token,i).c_str(),NULL) / bounds.h;
				x[3] = (relative) ? x[0] + dx : dx;
				y[3] = (relative) ? y[0] + dy : dy;
			}
			else
			{
				x[3] = x[2];
				y[3] = y[2];
			}
			
			unsigned index = AddBezierData(Bezier(x[0],y[0],x[1],y[1],x[2],y[2],x[3],y[3]));
			Add(BEZIER,Rect(0,0,1,1),index);
			
			
			Debug("[%u] curveto %f,%f %f,%f %f,%f", index, Float(x[1]),Float(y[1]),Float(x[2]),Float(y[2]),Float(x[3]),Float(y[3]));
			
			x[0] = x[3];
			y[0] = y[3];

			
		}
		else if (command == "l" || command == "L")
		{
			Debug("Construct lineto command");
		
			Real dx = strtod(GetToken(d,token,i).c_str(),NULL) / bounds.w;
			assert(GetToken(d,token,i) == ",");
			Real dy = strtod(GetToken(d,token,i).c_str(),NULL) / bounds.h;
			
			x[1] = (relative) ? x0 + dx : dx;
			y[1] = (relative) ? y0 + dy : dy;
			
			x[2] = x[1];
			y[2] = y[1];
			
			x[3] = x[1];
			y[3] = y[1];

			unsigned index = AddBezierData(Bezier(x[0],y[0],x[1],y[1],x[2],y[2],x[3],y[3]));
			Add(BEZIER,Rect(0,0,1,1),index);
			
			Debug("[%u] lineto %f,%f %f,%f", index, Float(x[0]),Float(y[0]),Float(x[1]),Float(y[1]));
			
			x[0] = x[3];
			y[0] = y[3];

		}
		else if (command == "z" || command == "Z")
		{
			Debug("Construct returnto command");
			x[1] = x0;
			y[1] = y0;
			x[2] = x0;
			y[2] = y0;
			x[3] = x0;
			y[3] = y0;
			
			unsigned index = AddBezierData(Bezier(x[0],y[0],x[1],y[1],x[2],y[2],x[3],y[3]));
			Add(BEZIER,Rect(0,0,1,1),index);
			
			Debug("[%u] returnto %f,%f %f,%f", index, Float(x[0]),Float(y[0]),Float(x[1]),Float(y[1]));
			
			x[0] = x[3];
			y[0] = y[3];
			command = "m";
		}
		else
		{
			Warn("Unrecognised command \"%s\", set to \"m\"", command.c_str());
			command = "m";
		}
		
		if (!start)
		{
			x0 = x[0];
			y0 = y[0];
			start = true;
		}
		prev_i = i;
	}
}
