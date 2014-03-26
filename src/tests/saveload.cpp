#include "../common.h"

#include "../document.h"
#include "../view.h"

using namespace std;
using namespace IPDF;

unsigned test_objects = 4;

int main(int argc, char ** argv)
{
	srand(time(NULL));
	Document doc;
	for (unsigned id = 0; id < test_objects; ++id)
	{
		doc.Add(Random(), Random(), Random(), Random());
	}
	doc.Save("test.ipdf");

	Document equ("test.ipdf");
	//doc.Add(Random(), Random(), Random(), Random());
	if (doc != equ || equ != doc)
	{
		Error("Loaded document is not equivelant to saved document!");
		doc.DebugDumpObjects();
		equ.DebugDumpObjects();
	}
	

	return 0;
}
