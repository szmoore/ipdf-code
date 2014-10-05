#include "document.h"
#include "bezier.h"
#include <cstdio>
#include <fstream>

#include "../contrib/pugixml-1.4/src/pugixml.cpp"
#include "transformationtype.h"

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
	m_quadtree.nodes.push_back(QuadTreeNode{QUADTREE_EMPTY, QUADTREE_EMPTY, QUADTREE_EMPTY, QUADTREE_EMPTY, QUADTREE_EMPTY, QTC_UNKNOWN, 0, ObjectCount(), -1});
	m_quadtree.root_id = 0;
}

int Document::ClipObjectToQuadChild(int object_id, QuadTreeNodeChildren type)
{
	switch (m_objects.types[object_id])
	{
	case RECT_FILLED:
	case RECT_OUTLINE:
	case PATH:
		{
		Rect obj_bounds = TransformToQuadChild(m_objects.bounds[object_id], type);
		if (obj_bounds.x < 0)
		{
			obj_bounds.w += obj_bounds.x;
			obj_bounds.x = 0;
		}
		if (obj_bounds.y < 0)
		{
			obj_bounds.h += obj_bounds.y;
			obj_bounds.y = 0;
		}
		if (obj_bounds.x + obj_bounds.w > 1)
		{
			obj_bounds.w += (1 - (obj_bounds.x + obj_bounds.w));
		}
		if (obj_bounds.y + obj_bounds.h > 1)
		{
			obj_bounds.h += (1 - (obj_bounds.y + obj_bounds.h));
		}
		m_objects.bounds.push_back(obj_bounds);
		m_objects.types.push_back(m_objects.types[object_id]);
		m_objects.data_indices.push_back(m_objects.data_indices[object_id]);
		return 1;
		}
	case BEZIER:
		{
		// If we're entirely within the quadtree node, no clipping need occur.
		if (ContainedInQuadChild(m_objects.bounds[object_id], type))
		{
			m_objects.bounds.push_back(TransformToQuadChild(m_objects.bounds[object_id], type));
			m_objects.types.push_back(m_objects.types[object_id]);
			m_objects.data_indices.push_back(m_objects.data_indices[object_id]);
			return 1;
		}
		Rect clip_bezier_bounds = TransformRectCoordinates(m_objects.bounds[object_id], TransformFromQuadChild(Rect{0,0,1,1}, type)); 
		std::vector<Bezier> new_curves = m_objects.beziers[m_objects.data_indices[object_id]].ClipToRectangle(clip_bezier_bounds);
		for (size_t i = 0; i < new_curves.size(); ++i)
		{
			Rect new_bounds = TransformToQuadChild(m_objects.bounds[object_id], type);
			Bezier new_curve_data = new_curves[i].ToAbsolute(TransformToQuadChild(m_objects.bounds[object_id],type));
			new_bounds = new_curve_data.SolveBounds();
			Debug("New bounds: %s", new_bounds.Str().c_str());
			new_curve_data = new_curve_data.ToRelative(new_bounds);
			unsigned index = AddBezierData(new_curve_data);
			m_objects.bounds.push_back(new_bounds);
			m_objects.types.push_back(BEZIER);
			m_objects.data_indices.push_back(index);
		}
		return new_curves.size();
		}
	default:
		Debug("Adding %s -> %s", m_objects.bounds[object_id].Str().c_str(), TransformToQuadChild(m_objects.bounds[object_id], type).Str().c_str());
		m_objects.bounds.push_back(TransformToQuadChild(m_objects.bounds[object_id], type));
		m_objects.types.push_back(m_objects.types[object_id]);
		m_objects.data_indices.push_back(m_objects.data_indices[object_id]);
		return 1;
	}
	return 0;
}
QuadTreeIndex Document::GenQuadChild(QuadTreeIndex parent, QuadTreeNodeChildren type)
{
	QuadTreeIndex new_index = m_quadtree.nodes.size();
	Debug("-------------- Generating Quadtree Node %d (parent %d, type %d) ----------------------", new_index, parent, type);
	m_quadtree.nodes.push_back(QuadTreeNode{QUADTREE_EMPTY, QUADTREE_EMPTY, QUADTREE_EMPTY, QUADTREE_EMPTY, parent, type, 0, 0, -1});

	m_quadtree.nodes[new_index].object_begin = m_objects.bounds.size();
	for (QuadTreeIndex overlay = parent; overlay != -1; overlay = m_quadtree.nodes[overlay].next_overlay)
	{
		for (unsigned i = m_quadtree.nodes[overlay].object_begin; i < m_quadtree.nodes[overlay].object_end; ++i)
		{
			if (IntersectsQuadChild(m_objects.bounds[i], type))
			{
				m_count += ClipObjectToQuadChild(i, type);
			}
		}
	}
	m_quadtree.nodes[new_index].object_end = m_objects.bounds.size();
	// No objects are dirty.
	m_quadtree.nodes[new_index].object_dirty = m_objects.bounds.size();
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

void Document::OverlayQuadChildren(QuadTreeIndex orig_parent, QuadTreeIndex parent, QuadTreeNodeChildren type)
{
	QuadTreeIndex new_index = m_quadtree.nodes.size();
	Debug("-------------- Generating Quadtree Node %d (orig %d parent %d, type %d) ----------------------", new_index, orig_parent, parent, type);
	m_quadtree.nodes.push_back(QuadTreeNode{QUADTREE_EMPTY, QUADTREE_EMPTY, QUADTREE_EMPTY, QUADTREE_EMPTY, orig_parent, type, 0, 0, -1});

	m_quadtree.nodes[new_index].object_begin = m_objects.bounds.size();
	for (unsigned i = m_quadtree.nodes[parent].object_dirty; i < m_quadtree.nodes[parent].object_end; ++i)
	{
		if (IntersectsQuadChild(m_objects.bounds[i], type))
		{
			m_count += ClipObjectToQuadChild(i, type);
		}
	}
	m_quadtree.nodes[new_index].object_end = m_objects.bounds.size();
	QuadTreeIndex orig_node = -1;
	switch (type)
	{
		case QTC_TOP_LEFT:
			orig_node = m_quadtree.nodes[orig_parent].top_left = new_index;
			break;
		case QTC_TOP_RIGHT:
			orig_node = m_quadtree.nodes[orig_parent].top_right = new_index;
			break;
		case QTC_BOTTOM_LEFT:
			orig_node = m_quadtree.nodes[orig_parent].bottom_left = new_index;
			break;
		case QTC_BOTTOM_RIGHT:
			orig_node = m_quadtree.nodes[orig_parent].bottom_right = new_index;
			break;
		default:
			Fatal("Tried to overlay a QuadTree child of invalid type!");
	}
	if (orig_node == -1)
		Fatal("Tried to overlay a QuadTree child that didn't exist!");

	// Add us to the node's overlay linked list.
	QuadTreeIndex prev_overlay = orig_node;
	while (m_quadtree.nodes[prev_overlay].next_overlay != -1);
	m_quadtree.nodes[prev_overlay].next_overlay = new_index;

	// Recurse into any extant children.
	if (m_quadtree.nodes[orig_node].top_left != -1)
		OverlayQuadChildren(orig_node, new_index, QTC_TOP_LEFT);
	if (m_quadtree.nodes[orig_node].top_right != -1)
		OverlayQuadChildren(orig_node, new_index, QTC_TOP_RIGHT);
	if (m_quadtree.nodes[orig_node].bottom_left != -1)
		OverlayQuadChildren(orig_node, new_index, QTC_BOTTOM_LEFT);
	if (m_quadtree.nodes[orig_node].bottom_right != -1)
		OverlayQuadChildren(orig_node, new_index, QTC_BOTTOM_RIGHT);

	m_quadtree.nodes[new_index].object_dirty = m_quadtree.nodes[new_index].object_end;
}

void Document::PropagateQuadChanges(QuadTreeIndex node)
{
	for(QuadTreeIndex overlay = node; overlay != -1; overlay = m_quadtree.nodes[overlay].next_overlay)
	{
		// We don't care about clean overlays.
		if (m_quadtree.nodes[overlay].object_dirty == m_quadtree.nodes[overlay].object_end) continue;
		// Recurse into any extant children.
		if (m_quadtree.nodes[node].top_left != -1)
			OverlayQuadChildren(node, overlay, QTC_TOP_LEFT);
		if (m_quadtree.nodes[node].top_right != -1)
			OverlayQuadChildren(node, overlay, QTC_TOP_RIGHT);
		if (m_quadtree.nodes[node].bottom_left != -1)
			OverlayQuadChildren(node, overlay, QTC_BOTTOM_LEFT);
		if (m_quadtree.nodes[node].bottom_right != -1)
			OverlayQuadChildren(node, overlay, QTC_BOTTOM_RIGHT);

		m_quadtree.nodes[overlay].object_dirty = m_quadtree.nodes[overlay].object_end;
	}
}

// Reparent a quadtree node, making it the "type" child of a new node.
QuadTreeIndex Document::GenQuadParent(QuadTreeIndex child, QuadTreeNodeChildren type)
{
	QuadTreeIndex new_index = m_quadtree.nodes.size();
	m_quadtree.nodes.push_back(QuadTreeNode{QUADTREE_EMPTY, QUADTREE_EMPTY, QUADTREE_EMPTY, QUADTREE_EMPTY, -1, QTC_UNKNOWN, 0, 0, -1});

	m_quadtree.nodes[new_index].object_begin = m_objects.bounds.size();
	for (QuadTreeIndex overlay = child; overlay != -1; overlay = m_quadtree.nodes[overlay].next_overlay)
	{
		for (unsigned i = m_quadtree.nodes[overlay].object_begin; i < m_quadtree.nodes[overlay].object_end; ++i)
		{
			m_objects.bounds.push_back(TransformFromQuadChild(m_objects.bounds[i], type));
			m_objects.types.push_back(m_objects.types[i]);
			m_objects.data_indices.push_back(m_objects.data_indices[i]);
			m_count++;
		}
	}
	m_quadtree.nodes[new_index].object_end = m_objects.bounds.size();
	m_quadtree.nodes[new_index].object_dirty = m_objects.bounds.size();
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
			
		case CT_OBJPATHS:
			Debug("Path data...");
			Warn("Not handled because lazy");
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

unsigned Document::AddPath(unsigned start_index, unsigned end_index, const Colour & fill, const Colour & stroke)
{
	Path path(m_objects, start_index, end_index, fill, stroke);
	unsigned data_index = AddPathData(path);
	Rect bounds = path.SolveBounds(m_objects);
	unsigned result = Add(PATH, bounds,data_index);
	m_objects.paths[data_index].m_index = result;
	//Debug("Added path %u -> %u (%u objects) colour {%u,%u,%u,%u}, stroke {%u,%u,%u,%u}", start_index, end_index, (end_index - start_index), fill.r, fill.g, fill.b, fill.a, stroke.r, stroke.g, stroke.b, stroke.a);
	return result;
}

/**
 * Add a Bezier using Absolute coords
 */
unsigned Document::AddBezier(const Bezier & bezier)
{
	Rect bounds = bezier.SolveBounds();
	Bezier data = bezier.ToRelative(bounds); // Relative
	if (data.ToAbsolute(bounds) != bezier)
	{
		Warn("%s != %s", data.ToAbsolute(bounds).Str().c_str(),
			bezier.Str().c_str());
		Warn("ToAbsolute on ToRelative does not give original Bezier");
	}
	unsigned index = AddBezierData(data);
	return Add(BEZIER, bounds, index);
}

unsigned Document::Add(ObjectType type, const Rect & bounds, unsigned data_index, QuadTreeIndex qti)
{
	m_objects.types.push_back(type);
	m_objects.bounds.push_back(bounds);
	m_objects.data_indices.push_back(data_index);
#ifndef QUADTREE_DISABLED
	if (qti != -1)
	{
		QuadTreeIndex new_qti = qti;
		while (m_quadtree.nodes[new_qti].next_overlay != -1)
		{
			if (m_count == m_quadtree.nodes[new_qti].object_end+1)
			{
				m_quadtree.nodes[new_qti].object_end++;
				goto done;
			}
			new_qti = m_quadtree.nodes[new_qti].next_overlay;
		}
		{
			QuadTreeIndex overlay = m_quadtree.nodes.size();
			Debug("Adding new overlay, nqti = %d, overlay = %d", new_qti, overlay);
			m_quadtree.nodes.push_back(m_quadtree.nodes[qti]);
			m_quadtree.nodes[overlay].object_begin = m_count;
			// All objects are dirty.
			m_quadtree.nodes[overlay].object_dirty = m_count;
			m_quadtree.nodes[overlay].object_end = m_count+1;
			m_quadtree.nodes[overlay].next_overlay = -1;
			m_quadtree.nodes[new_qti].next_overlay = overlay;
			new_qti = overlay;
		}
done: // matches is not amused, but sulix is nice and moved it inside the #ifdef for him.
		m_count++;
		PropagateQuadChanges(qti);
	}
#else // Although empty documents do always render correctly at any zoom,
		// I suspect the markers will not be amused
	m_count++;
#endif
	return m_count; // Why can't we just use the size of types or something?
}

unsigned Document::AddBezierData(const Bezier & bezier)
{
	m_objects.beziers.push_back(bezier);
	return m_objects.beziers.size()-1;
}

unsigned Document::AddPathData(const Path & path)
{
	m_objects.paths.push_back(path);
	return m_objects.paths.size()-1;
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



// Behold my amazing tokenizing abilities
static string & GetToken(const string & d, string & token, unsigned & i, const string & delims = "()[],{}<>;:=")
{
	token.clear();
	while (i < d.size() && iswspace(d[i]))
	{
		++i;
	}
	
	while (i < d.size())
	{
		if (iswspace(d[i]) || strchr(delims.c_str(),d[i]) != NULL)
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

static void GetXYPair(const string & d, Real & x, Real & y, unsigned & i,const string & delims = "()[],{}<>;:=")
{
	string token("");
	while (GetToken(d, token, i, delims) == ",");
	x = RealFromStr(token);
	if (GetToken(d, token, i, delims) != ",")
	{
		Fatal("Expected \",\" seperating x,y pair");
	}
	y = RealFromStr(GetToken(d,token,i,delims));
}

static bool GetKeyValuePair(const string & d, string & key, string & value, unsigned & i, const string & delims = "()[],{}<>;:=")
{
	key = "";
	string token;
	while (GetToken(d, token, i, delims) == ":" || token == ";");
	key = token;
	if (GetToken(d, token, i, delims) != ":")
	{
		Error("Expected \":\" seperating key:value pair");
		return false;
	}
	value = "";
	GetToken(d, value, i, delims);
	return true;
}

static void TransformXYPair(Real & x, Real & y, const SVGMatrix & transform)
{
	Real x0(x);
	x = transform.a * x + transform.c * y + transform.e;
	y = transform.b * x0 + transform.d * y + transform.f;
}

void Document::ParseSVGTransform(const string & s, SVGMatrix & transform)
{
	//Debug("Parsing transform %s", s.c_str());
	string token;
	string command;
	unsigned i = 0;
	
	while (i < s.size())
	{
		GetToken(s, command, i);
		if (command == "," || command == "" || command == ":")
		{
			if (i < s.size())
				GetToken(s, command, i);
			else
				return;
		}
		//Debug("Token is \"%s\"", command.c_str());
	
		SVGMatrix delta = {1,0,0,0,1,0};
	
	
		assert(GetToken(s,token, i) == "(");
		if (command == "translate")
		{
			GetXYPair(s, delta.e, delta.f, i);
			assert(GetToken(s,token, i) == ")");	
		}
		else if (command == "matrix")
		{
			GetXYPair(s, delta.a, delta.b,i);
			GetXYPair(s, delta.c, delta.d,i);
			GetXYPair(s, delta.e, delta.f,i);
			assert(GetToken(s,token, i) == ")");	
		}
		else if (command == "scale")
		{
			delta.a = RealFromStr(GetToken(s,token,i));
			GetToken(s, token, i);
			if (token == ",")
			{
				delta.d = RealFromStr(GetToken(s,token,i));
				assert(GetToken(s, token, i) == ")");
			}
			else
			{
				delta.d = delta.a;
				assert(token == ")");
			}
			
		}
		else
		{
			Warn("Unrecognised transform \"%s\", using identity", command.c_str());
		}
	
		//Debug("Old transform is {%f,%f,%f,%f,%f,%f}", transform.a, transform.b, transform.c, transform.d,transform.e,transform.f);
		//Debug("Delta transform is {%f,%f,%f,%f,%f,%f}", delta.a, delta.b, delta.c, delta.d,delta.e,delta.f);
	
		SVGMatrix old(transform);
		transform.a = old.a * delta.a + old.c * delta.b;
		transform.c = old.a * delta.c + old.c * delta.d;
		transform.e = old.a * delta.e + old.c * delta.f + old.e;
	
		transform.b = old.b * delta.a + old.d * delta.b;
		transform.d = old.b * delta.c + old.d * delta.d;
		transform.f = old.b * delta.e + old.d * delta.f + old.f;
	
		//Debug("New transform is {%f,%f,%f,%f,%f,%f}", transform.a, transform.b, transform.c, transform.d,transform.e,transform.f);
	}
}

inline Colour ParseColourString(const string & colour_str)
{
	Colour c(0,0,0,0);
	if (colour_str == "red")
		c = {255,0,0,255};
	else if (colour_str == "blue")
		c = {0,0,255,255};
	else if (colour_str == "green")
		c = {0,255,0,255};
	else if (colour_str == "black")
		c = {0,0,0,255};
	else if (colour_str == "white")
		c = {255,255,255,255};
	else if (colour_str.size() == 7 && colour_str[0] == '#')
	{
		//Debug("Parse colour string: \"%s\"", colour_str.c_str());
		char comp[3] = {colour_str[1], colour_str[2], '\0'};
		c.r = strtoul(comp, NULL, 16);
		comp[0] = colour_str[3]; comp[1] = colour_str[4];
		c.g = strtoul(comp, NULL, 16);
		comp[0] = colour_str[5]; comp[1] = colour_str[6];
		c.b = strtoul(comp, NULL, 16);
		c.a = 255;
		//Debug("Colour is: %u, %u, %u, %u", c.r, c.g, c.b, c.a);
	}
	return c;
}

void Document::ParseSVGNode(pugi::xml_node & root, SVGMatrix & parent_transform)
{
	//Debug("Parse node <%s>", root.name());

	
	// Centre the SVGs
	if (strcmp(root.name(),"svg") == 0)
	{
		Real ww = RealFromStr(root.attribute("width").as_string());
		Real hh = RealFromStr(root.attribute("height").as_string());
		parent_transform.e -= parent_transform.a * ww/Real(2);
		parent_transform.f -= parent_transform.d * hh/Real(2);
	}
	
	for (pugi::xml_node child = root.first_child(); child; child = child.next_sibling())
	{
		SVGMatrix transform(parent_transform);	
		pugi::xml_attribute attrib_trans = child.attribute("transform");
		if (!attrib_trans.empty())
		{
			ParseSVGTransform(attrib_trans.as_string(), transform);
		}
		
		
		
		if (strcmp(child.name(), "svg") == 0 || strcmp(child.name(),"g") == 0
			|| strcmp(child.name(), "group") == 0)
		{
			
			ParseSVGNode(child, transform);
			continue;
		}
		else if (strcmp(child.name(), "path") == 0)
		{
			string d = child.attribute("d").as_string();
			//Debug("Path data attribute is \"%s\"", d.c_str());
			bool closed = false;
			pair<unsigned, unsigned> range = ParseSVGPathData(d, transform, closed);
			if (true && range.first < m_count && range.second < m_count)//(closed)
			{
				
				string colour_str("");
				map<string, string> style;
				if (child.attribute("style"))
				{
					ParseSVGStyleData(child.attribute("style").as_string(), style);
				}
				
				// Determine shading colour
				if (child.attribute("fill"))
				{
					colour_str = child.attribute("fill").as_string();
				}
				else if (style.find("fill") != style.end())
				{
					colour_str = style["fill"];
				}
				Colour fill = ParseColourString(colour_str);
				Colour stroke = fill;
			
				if (child.attribute("stroke"))
				{
					colour_str = child.attribute("stroke").as_string();
					stroke = ParseColourString(colour_str);
				}
				else if (style.find("stroke") != style.end())
				{
					colour_str = style["stroke"];
					stroke = ParseColourString(colour_str);
				}
				
				
				// Determin shading alpha
				if (child.attribute("fill-opacity"))
				{
					fill.a = 255*child.attribute("fill-opacity").as_float();
				}
				else if (style.find("fill-opacity") != style.end())
				{
					fill.a = 255*strtod(style["fill-opacity"].c_str(), NULL);
				}
				if (child.attribute("stroke-opacity"))
				{
					stroke.a = 255*child.attribute("stroke-opacity").as_float();
				}
				else if (style.find("stroke-opacity") != style.end())
				{
					stroke.a = 255*strtod(style["stroke-opacity"].c_str(), NULL);
				}
				AddPath(range.first, range.second, fill, stroke);
			}
			
		}
		else if (strcmp(child.name(), "line") == 0)
		{
			Real x0(child.attribute("x1").as_float());
			Real y0(child.attribute("y1").as_float());
			Real x1(child.attribute("x2").as_float());
			Real y1(child.attribute("y2").as_float());
			TransformXYPair(x0,y0,transform);
			TransformXYPair(x1,y1,transform);
			AddBezier(Bezier(x0,y0,x1,y1,x1,y1,x1,y1));
		}
		else if (strcmp(child.name(), "rect") == 0)
		{
			Real coords[4];
			const char * attrib_names[] = {"x", "y", "width", "height"};
			for (size_t i = 0; i < 4; ++i)
				coords[i] = child.attribute(attrib_names[i]).as_float();
			
			Real x2(coords[0]+coords[2]);
			Real y2(coords[1]+coords[3]);
			TransformXYPair(coords[0],coords[1],transform); // x, y, transform
			TransformXYPair(x2,y2,transform);
			coords[2] = x2 - coords[0];
			coords[3] = y2 - coords[1];
			
			bool outline = !(child.attribute("fill") && strcmp(child.attribute("fill").as_string(),"none") != 0);
			Add(outline?RECT_OUTLINE:RECT_FILLED, Rect(coords[0], coords[1], coords[2], coords[3]),0);
		}
		else if (strcmp(child.name(), "circle") == 0)
		{
			Real cx = child.attribute("cx").as_float();
			Real cy = child.attribute("cy").as_float();
			Real r = child.attribute("r").as_float();
			
			Real x = (cx - r);
			Real y = (cy - r);
			TransformXYPair(x,y,transform);
			Real w = Real(2)*r*transform.a; // width scales
			Real h = Real(2)*r*transform.d; // height scales
			
			
			Rect rect(x,y,w,h);
			Add(CIRCLE_FILLED, rect,0);
			Debug("Added Circle %s", rect.Str().c_str());			
		}
		else if (strcmp(child.name(), "text") == 0)
		{
			Real x = child.attribute("x").as_float();
			Real y = child.attribute("y").as_float();
			TransformXYPair(x,y,transform);
			Debug("Add text \"%s\"", child.child_value());
			AddText(child.child_value(), 0.05, x, y);
		}
	}
}

void Document::ParseSVGStyleData(const string & style, map<string, string> & results)
{
	unsigned i = 0;
	string key;
	string value;
	while (i < style.size() && GetKeyValuePair(style, key, value, i))
	{
		results[key] = value;
	}
}

/**
 * Parse an SVG string into a rectangle
 */
void Document::ParseSVG(const string & input, const Rect & bounds)
{
	using namespace pugi;
	
	xml_document doc_xml;
	xml_parse_result result = doc_xml.load(input.c_str());
	
	if (!result)
		Error("Couldn't parse SVG input - %s", result.description());
		
	Debug("Loaded XML - %s", result.description());
	SVGMatrix transform = {bounds.w, 0,bounds.x, 0,bounds.h,bounds.y};
	ParseSVGNode(doc_xml, transform);
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
		Error("Couldn't load \"%s\" - %s", filename.c_str(), result.description());
		
	Debug("Loaded XML from \"%s\" - %s", filename.c_str(), result.description());
	
	input.close();
						// a c e, b d f
	SVGMatrix transform = {bounds.w,0 ,bounds.x, 0,bounds.h,bounds.y};
	ParseSVGNode(doc_xml, transform);
}



// Fear the wrath of the tokenizing svg data
// Seriously this isn't really very DOM-like at all is it?
pair<unsigned, unsigned> Document::ParseSVGPathData(const string & d, const SVGMatrix & transform, bool & closed)
{
	closed = false;
	Real x[4] = {0,0,0,0};
	Real y[4] = {0,0,0,0};
	
	string token("");
	string command("m");
	
	Real x0(0);
	Real y0(0);
	
	unsigned i = 0;
	unsigned prev_i = 0;
	
	bool start = false;
	

	static string delims("()[],{}<>;:=LlHhVvmMqQzZcC");

	pair<unsigned, unsigned> range(m_count, m_count);
	
	while (i < d.size() && GetToken(d, token, i, delims).size() > 0)
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
			//Debug("Construct moveto command");
			Real dx = RealFromStr(GetToken(d,token,i,delims));
			assert(GetToken(d,token,i,delims) == ",");
			Real dy = RealFromStr(GetToken(d,token,i,delims));
			
			x[0] = (relative) ? x[0] + dx : dx;
			y[0] = (relative) ? y[0] + dy : dy;
			
			x0 = x[0];
			y0 = y[0];
			//Debug("mmoveto %f,%f", Float(x[0]),Float(y[0]));
			command = (command == "m") ? "l" : "L";
		}
		else if (command == "c" || command == "C" || command == "q" || command == "Q")
		{
			//Debug("Construct curveto command");
			Real dx = RealFromStr(GetToken(d,token,i,delims));
			assert(GetToken(d,token,i,delims) == ",");
			Real dy = RealFromStr(GetToken(d,token,i,delims));
			
			x[1] = (relative) ? x[0] + dx : dx;
			y[1] = (relative) ? y[0] + dy : dy;
			
			dx = RealFromStr(GetToken(d,token,i,delims));
			assert(GetToken(d,token,i,delims) == ",");
			dy = RealFromStr(GetToken(d,token,i,delims));
			
			x[2] = (relative) ? x[0] + dx : dx;
			y[2] = (relative) ? y[0] + dy : dy;
			
			if (command != "q" && command != "Q")
			{
				dx = RealFromStr(GetToken(d,token,i,delims));
				assert(GetToken(d,token,i,delims) == ",");
				dy = RealFromStr(GetToken(d,token,i,delims));
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
			
			Real x3(x[3]);
			Real y3(y[3]);
			for (int j = 0; j < 4; ++j)
				TransformXYPair(x[j],y[j], transform);

			range.second = AddBezier(Bezier(x[0],y[0],x[1],y[1],x[2],y[2],x[3],y[3]));
			
			//Debug("[%u] curveto %f,%f %f,%f %f,%f", index, Float(x[1]),Float(y[1]),Float(x[2]),Float(y[2]),Float(x[3]),Float(y[3]));
			
			x[0] = x3;
			y[0] = y3;

			
		}
		else if (command == "l" || command == "L" || command == "h" || command == "H" || command == "v" || command == "V")
		{
			//Debug("Construct lineto command, relative %d", relative);
		
			Real dx = RealFromStr(GetToken(d,token,i,delims));
			Real dy = 0;
			if (command == "l" || command == "L")
			{
				assert(GetToken(d,token,i,delims) == ",");
				dy = RealFromStr(GetToken(d,token,i,delims));
			}
			else if (command == "v" || command == "V")
			{
				swap(dx,dy);
			}
			
			x[1] = (relative) ? x[0] + dx : dx;
			y[1] = (relative) ? y[0] + dy : dy;
			if (command == "v" || command == "V")
			{
				x[1] = x[0];
			}
			else if (command == "h" || command == "H")
			{
				y[1] = y[0];
			}
			
			Real x1(x[1]);
			Real y1(y[1]);
			
			TransformXYPair(x[0],y[0],transform);
			TransformXYPair(x[1],y[1],transform);


			range.second = AddBezier(Bezier(x[0],y[0],x[1],y[1],x[1],y[1],x[1],y[1]));
			
			//Debug("[%u] lineto %f,%f %f,%f", index, Float(x[0]),Float(y[0]),Float(x[1]),Float(y[1]));
			
			x[0] = x1;
			y[0] = y1;

		}
		else if (command == "z" || command == "Z")
		{
			//Debug("Construct returnto command");
			x[1] = x0;
			y[1] = y0;
			x[2] = x0;
			y[2] = y0;
			x[3] = x0;
			y[3] = y0;
			
			Real x3(x[3]);
			Real y3(y[3]);
			for (int j = 0; j < 4; ++j)
				TransformXYPair(x[j],y[j], transform);

			range.second = AddBezier(Bezier(x[0],y[0],x[1],y[1],x[2],y[2],x[3],y[3]));
			//Debug("[%u] returnto %f,%f %f,%f", index, Float(x[0]),Float(y[0]),Float(x[1]),Float(y[1]));
			
			x[0] = x3;
			y[0] = y3;
			command = "m";
			closed = true;
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
	return range;
}

void Document::SetFont(const string & font_filename)
{
	if (m_font_data != NULL)
	{
		free(m_font_data);
	}
	
	FILE *font_file = fopen(font_filename.c_str(), "rb");
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
		
	Real x0(x);
	//Real y0(y);
	int ascent = 0, descent = 0, line_gap = 0;
	stbtt_GetFontVMetrics(&m_font, &ascent, &descent, &line_gap);
	Real font_scale = scale;
	font_scale /= Real(ascent - descent);
	Real y_advance = Real(font_scale) * Real(ascent - descent + line_gap);
	for (unsigned i = 0; i < text.size(); ++i)
	{
		if (text[i] == '\n')
		{
			y += y_advance;
			x = x0;
		}
		if (!isprint(text[i]))
			continue;
			
		int advance_width = 0, left_side_bearing = 0, kerning = 0;
		stbtt_GetCodepointHMetrics(&m_font, text[i], &advance_width, &left_side_bearing);
		if (i >= 1)
		{
			kerning = stbtt_GetCodepointKernAdvance(&m_font, text[i-1], text[i]);
		}
		x += font_scale * Real(kerning);
		AddFontGlyphAtPoint(&m_font, text[i], font_scale, x, y);
		x += font_scale * Real(advance_width);
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
	unsigned start_index = m_count;
	unsigned end_index = m_count;
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
		
		switch(instructions[i].type)
		{
		// Move To
		case STBTT_vmove:
			break;
		// Line To
		case STBTT_vline:
			end_index = AddBezier(Bezier(old_x + x, old_y + y, old_x + x, old_y + y, current_x + x, current_y + y, current_x + x, current_y + y));
			break;
		// Quadratic Bezier To:
		case STBTT_vcurve:
			// Quadratic -> Cubic:
			// - Endpoints are the same.
			// - cubic1 = quad0+(2/3)*(quad1-quad0)
			// - cubic2 = quad2+(2/3)*(quad1-quad2)
			end_index = AddBezier(Bezier(old_x + x, old_y + y, old_x + Real(2)*(inst_cx-old_x)/Real(3) + x, old_y + Real(2)*(inst_cy-old_y)/Real(3) + y,
						current_x + Real(2)*(inst_cx-current_x)/Real(3) + x, current_y + Real(2)*(inst_cy-current_y)/Real(3) + y, current_x + x, current_y + y));
			break;
		}
	}
	
	if (start_index < m_count && end_index < m_count)
	{
		AddPath(start_index, end_index);
	}
	//Debug("Added Glyph \"%c\" at %f %f, scale %f", (char)character, Float(x), Float(y), Float(scale));

	stbtt_FreeShape(font, instructions);
}

void Document::TransformObjectBounds(const SVGMatrix & transform)
{
	for (unsigned i = 0; i < m_count; ++i)
	{
		TransformXYPair(m_objects.bounds[i].x, m_objects.bounds[i].y, transform);
		m_objects.bounds[i].w *= transform.a;
		m_objects.bounds[i].h *= transform.d;
	}
}

void Document::TranslateObjects(const Real & dx, const Real & dy, ObjectType type)
{
	#ifdef TRANSFORM_BEZIERS_TO_PATH
		for (unsigned i = 0; i < m_objects.paths.size(); ++i)
		{
			Path & p = m_objects.paths[i];
			p.m_bounds.x += dx;
			p.m_bounds.y += dy;
		}
		return;
	#endif	
	
	for (unsigned i = 0; i < m_count; ++i)
	{
		if (type == NUMBER_OF_OBJECT_TYPES || m_objects.types[i] == type)
		{
			m_objects.bounds[i].x += dx;
			m_objects.bounds[i].y += dy;
		}
	}
}

void Document::ScaleObjectsAboutPoint(const Real & x, const Real & y, const Real & scale_amount, ObjectType type)
{
	#ifdef TRANSFORM_BEZIERS_TO_PATH
		for (unsigned i = 0; i < m_objects.paths.size(); ++i)
		{
			Path & p = m_objects.paths[i];
			p.m_bounds.w /= scale_amount;
			p.m_bounds.h /= scale_amount;
			p.m_bounds.x -= x;
			p.m_bounds.x /= scale_amount;
			p.m_bounds.x += x;
			p.m_bounds.y -= y;
			p.m_bounds.y /= scale_amount;
			p.m_bounds.y += y;
		}
		return;
	#endif
	
	for (unsigned i = 0; i < m_count; ++i)
	{
		if (type != NUMBER_OF_OBJECT_TYPES && m_objects.types[i] != type)
			continue;
		
		m_objects.bounds[i].w /= scale_amount;
		m_objects.bounds[i].h /= scale_amount;
		//m_objects.bounds[i].x = x + (m_objects.bounds[i].x-x)/scale_amount;
		//m_objects.bounds[i].y = y + (m_objects.bounds[i].y-x)/scale_amount;
		m_objects.bounds[i].x -= x;
		m_objects.bounds[i].x /= scale_amount;
		m_objects.bounds[i].x += x;
		
		m_objects.bounds[i].y -= y;
		m_objects.bounds[i].y /= scale_amount;
		m_objects.bounds[i].y += y;
	}

}


