#include "document.h"
#include "bezier.h"
#include <cstdio>
#include <fstream>

#include "../contrib/pugixml-1.4/src/pugixml.cpp"

#include "stb_truetype.h"

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
	GenQuadChild(0, QTC_TOP_LEFT);
	GenQuadParent(0, QTC_BOTTOM_RIGHT);
}

QuadTreeIndex Document::GenQuadChild(QuadTreeIndex parent, QuadTreeNodeChildren type)
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

// Reparent a quadtree node, making it the "type" child of a new node.
QuadTreeIndex Document::GenQuadParent(QuadTreeIndex child, QuadTreeNodeChildren type)
{
	QuadTreeIndex new_index = m_quadtree.nodes.size();
	m_quadtree.nodes.push_back(QuadTreeNode{QUADTREE_EMPTY, QUADTREE_EMPTY, QUADTREE_EMPTY, QUADTREE_EMPTY, -1, QTC_UNKNOWN, 0, 0});

	m_quadtree.nodes[new_index].object_begin = m_objects.bounds.size();
	for (unsigned i = m_quadtree.nodes[child].object_begin; i < m_quadtree.nodes[child].object_end; ++i)
	{
		m_objects.bounds.push_back(TransformFromQuadChild(m_objects.bounds[i], type));
		m_objects.types.push_back(m_objects.types[i]);
		m_objects.data_indices.push_back(m_objects.data_indices[i]);
		m_count++;
	}
	m_quadtree.nodes[new_index].object_end = m_objects.bounds.size();
	switch (type)
	{
		case QTC_TOP_LEFT:
			m_quadtree.nodes[new_index].top_left = child;
			break;
		case QTC_TOP_RIGHT:
			m_quadtree.nodes[new_index].top_right = child;
			break;
		case QTC_BOTTOM_LEFT:
			m_quadtree.nodes[new_index].bottom_left = child;
			break;
		case QTC_BOTTOM_RIGHT:
			m_quadtree.nodes[new_index].bottom_right = child;
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




void Document::ParseSVGNode(pugi::xml_node & root, const Rect & bounds, Real & width, Real & height)
{
	Debug("Parse node <%s>", root.name());
	pugi::xml_attribute attrib_w = root.attribute("width");
	pugi::xml_attribute attrib_h = root.attribute("height");
	if (!attrib_w.empty())
		width = attrib_w.as_float() * bounds.w;
	if (!attrib_h.empty())
		height = attrib_h.as_float() * bounds.h;
			
	for (pugi::xml_node child = root.first_child(); child; child = child.next_sibling())
	{

		
		if (strcmp(child.name(), "svg") == 0 || strcmp(child.name(),"g") == 0
			|| strcmp(child.name(), "group") == 0)
		{
			//TODO: Handle translates etc here
			ParseSVGNode(child, bounds, width, height);
			continue;
		}
		else if (strcmp(child.name(), "path") == 0)
		{
			string d = child.attribute("d").as_string();
			Debug("Path data attribute is \"%s\"", d.c_str());
			ParseSVGPathData(d, Rect(bounds.x,bounds.y,width,height));
		}
		else if (strcmp(child.name(), "rect") == 0)
		{
			Real coords[4];
			const char * attrib_names[] = {"x", "y", "width", "height"};
			for (size_t i = 0; i < 4; ++i)
				coords[i] = child.attribute(attrib_names[i]).as_float();
			
			bool outline = !(child.attribute("fill"));
			Add(outline?RECT_OUTLINE:RECT_FILLED, Rect(coords[0]/width + bounds.x, coords[1]/height + bounds.y, coords[2]/width, coords[3]/height),0);
		}
		else if (strcmp(child.name(), "circle") == 0)
		{
			Real cx = child.attribute("cx").as_float();
			Real cy = child.attribute("cy").as_float();
			Real r = child.attribute("r").as_float();
			
			Real x = (cx - r)/width + bounds.x; 
			Real y = (cy - r)/height + bounds.y; 
			Real w = Real(2)*r/width; 
			Real h = Real(2)*r/height;
			
			Rect rect(x,y,w,h);
			Add(CIRCLE_FILLED, rect,0);
			Debug("Added Circle %s", rect.Str().c_str());			
		}
	}
}

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
	Real width(1);
	Real height(1);
	ParseSVGNode(doc_xml, bounds,width,height);
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
	//Debug("Got token \"%s\"", token.c_str());
	return token;
}


// Fear the wrath of the tokenizing svg data
// Seriously this isn't really very DOM-like at all is it?
void Document::ParseSVGPathData(const string & d, const Rect & bounds)
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
			Real dx = Real(strtod(GetToken(d,token,i).c_str(),NULL)) / bounds.w;
			assert(GetToken(d,token,i) == ",");
			Real dy = Real(strtod(GetToken(d,token,i).c_str(),NULL)) / bounds.h;
			
			x[0] = (relative) ? x[0] + dx : dx;
			y[0] = (relative) ? y[0] + dy : dy;
			

			
			Debug("mmoveto %f,%f", Float(x[0]),Float(y[0]));
			command = (command == "m") ? "l" : "L";
		}
		else if (command == "c" || command == "C" || command == "q" || command == "Q")
		{
			Debug("Construct curveto command");
			Real dx = Real(strtod(GetToken(d,token,i).c_str(),NULL))/bounds.w;
			assert(GetToken(d,token,i) == ",");
			Real dy = Real(strtod(GetToken(d,token,i).c_str(),NULL))/bounds.h;
			
			x[1] = (relative) ? x[0] + dx : dx;
			y[1] = (relative) ? y[0] + dy : dy;
			
			dx = Real(strtod(GetToken(d,token,i).c_str(),NULL)) / bounds.w;
			assert(GetToken(d,token,i) == ",");
			dy = Real(strtod(GetToken(d,token,i).c_str(),NULL)) / bounds.h;
			
			x[2] = (relative) ? x[0] + dx : dx;
			y[2] = (relative) ? y[0] + dy : dy;
			
			if (command != "q" && command != "Q")
			{
				dx = Real(strtod(GetToken(d,token,i).c_str(),NULL)) / bounds.w;
				assert(GetToken(d,token,i) == ",");
				dy = Real(strtod(GetToken(d,token,i).c_str(),NULL)) / bounds.h;
				x[3] = (relative) ? x[0] + dx : dx;
				y[3] = (relative) ? y[0] + dy : dy;
			}
			else
			{
				x[3] = x[2];
				y[3] = y[2];
				Real old_x1(x[1]), old_y1(y[1]);
				x[1] = x[0] + Real(2) * (old_x1 - x[0])/ Real(3);
				y[1] = y[0] + Real(2) * (old_y1 - y[0])/ Real(3);
				x[2] = x[3] + Real(2) * (old_x1 - x[3])/ Real(3);
				y[2] = y[3] + Real(2) * (old_y1 - y[3])/ Real(3);
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
		
			Real dx = Real(strtod(GetToken(d,token,i).c_str(),NULL)) / bounds.w;
			assert(GetToken(d,token,i) == ",");
			Real dy = Real(strtod(GetToken(d,token,i).c_str(),NULL)) / bounds.h;
			
			x[1] = (relative) ? x[0] + dx : dx;
			y[1] = (relative) ? y[0] + dy : dy;
			
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

void Document::SetFont(const string & font_filename)
{
	if (m_font_data != NULL)
	{
		free(m_font_data);
	}
	
	FILE *font_file = fopen("DejaVuSansMono.ttf", "rb");
	fseek(font_file, 0, SEEK_END);
	size_t font_file_size = ftell(font_file);
	fseek(font_file, 0, SEEK_SET);
	m_font_data = (unsigned char*)malloc(font_file_size);
	size_t read = fread(m_font_data, 1, font_file_size, font_file);
	if (read != font_file_size)
	{
		Fatal("Failed to read font data from \"%s\" - Read %u bytes expected %u - %s", font_filename.c_str(), read, font_file_size, strerror(errno));
	}
	fclose(font_file);
	stbtt_InitFont(&m_font, m_font_data, 0);
}

void Document::AddText(const string & text, Real scale, Real x, Real y)
{
	if (m_font_data == NULL)
	{
		Warn("No font loaded");
		return;
	}
		
	float font_scale = stbtt_ScaleForPixelHeight(&m_font, scale);
	Real x0(x);
	//Real y0(y);
	for (unsigned i = 0; i < text.size(); ++i)
	{
		if (text[i] == '\n')
		{
			y += 0.5*scale;
			x = x0;
		}
		if (!isprint(text[i]))
			continue;
			
		AddFontGlyphAtPoint(&m_font, text[i], font_scale, x, y);
		x += 0.5*scale;
	}
}

void Document::AddFontGlyphAtPoint(stbtt_fontinfo *font, int character, Real scale, Real x, Real y)
{
	int glyph_index = stbtt_FindGlyphIndex(font, character);

	// Check if there is actully a glyph to render.
	if (stbtt_IsGlyphEmpty(font, glyph_index))
	{
		return;
	}

	stbtt_vertex *instructions;
	int num_instructions = stbtt_GetGlyphShape(font, glyph_index, &instructions);

	Real current_x(0), current_y(0);

	for (int i = 0; i < num_instructions; ++i)
	{
		// TTF uses 16-bit signed ints for coordinates:
		// with the y-axis inverted compared to us.
		// Convert and scale any data.
		Real inst_x = Real(instructions[i].x)*scale;
		Real inst_y = Real(instructions[i].y)*-scale;
		Real inst_cx = Real(instructions[i].cx)*scale;
		Real inst_cy = Real(instructions[i].cy)*-scale;
		Real old_x(current_x), old_y(current_y);
		current_x = inst_x;
		current_y = inst_y;
		unsigned bezier_index;
		switch(instructions[i].type)
		{
		// Move To
		case STBTT_vmove:
			break;
		// Line To
		case STBTT_vline:
			bezier_index = AddBezierData(Bezier(old_x + x, old_y + y, old_x + x, old_y + y, current_x + x, current_y + y, current_x + x, current_y + y));
			Add(BEZIER,Rect(0,0,1,1),bezier_index);
			break;
		// Quadratic Bezier To:
		case STBTT_vcurve:
			// Quadratic -> Cubic:
			// - Endpoints are the same.
			// - cubic1 = quad0+(2/3)*(quad1-quad0)
			// - cubic2 = quad2+(2/3)*(quad1-quad2)
			bezier_index = AddBezierData(Bezier(old_x + x, old_y + y, old_x + Real(2)*(inst_cx-old_x)/Real(3) + x, old_y + Real(2)*(inst_cy-old_y)/Real(3) + y,
						current_x + Real(2)*(inst_cx-current_x)/Real(3) + x, current_y + Real(2)*(inst_cy-current_y)/Real(3) + y, current_x + x, current_y + y));
			Add(BEZIER,Rect(0,0,1,1),bezier_index);
			break;
		}
	}

	stbtt_FreeShape(font, instructions);
}
