#include "main.h"
#include "../../contrib/pugixml-1.4/src/pugixml.hpp"
#include "../../contrib/pugixml-1.4/src/pugixml.cpp"

using namespace std;

/**
 * This tester reads XML and uses it to produce rectangles in our viewer
 * We use pugixml because I really didn't feel like implementing an XML parser
 */
int main(int argc, char ** argv)
{
	pugi::xml_document doc_xml;
	Document doc;
	pugi::xml_parse_result result = doc_xml.load(cin); // load from stdin
	
	if (!result)
	{
		Fatal("XML from stdin has errors: %s", result.description());
	}

	Debug("pugixml load result was: %s", result.description());
	int count = 0;
	for (pugi::xml_node rect : doc_xml.children("rect"))
	{
			
		Real coords[4];
		const char * attrib_names[] = {"x", "y", "w", "h"};
		for (size_t i = 0; i < sizeof(attrib_names)/sizeof(char*); ++i)
		{
			coords[i] = rect.attribute(attrib_names[i]).as_float();
		}

		bool outline = false;
		outline = rect.attribute("outline").as_bool();

		Debug("rect node %d is {%lf, %lf, %lf, %lf} (%s)", count++, coords[0], coords[1], coords[2], coords[3], (outline) ? "outline" : "filled");
		doc.Add(outline?RECT_OUTLINE:RECT_FILLED, Rect(coords[0], coords[1], coords[2], coords[3]));
	}


	MainLoop(doc);

	

	return 0;
}
