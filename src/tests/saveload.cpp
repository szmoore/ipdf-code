#include "main.h"
#include <unistd.h>
unsigned test_objects = 4;

void Cleanup()
{
	unlink("saveload.ipdf");
}

int main(int argc, char ** argv)
{
	Debug("TEST STARTING %s", argv[0]);
	atexit(Cleanup);
	srand(time(NULL));
	Document doc;
	for (unsigned id = 0; id < test_objects; ++id)
	{
		doc.Add((ObjectType)(rand() % 2), Rect(Random(), Random(), Random(), Random()));
	}
	doc.Save("saveload.ipdf");

	Document equ("saveload.ipdf");
	//doc.Add(Random(), Random(), Random(), Random());
	if (doc != equ || equ != doc)
	{
		Error("Loaded document is not equivelant to saved document!");
		doc.DebugDumpObjects();
		equ.DebugDumpObjects();
		Fatal("TEST FAILED");
	}
	
	doc.Add((ObjectType)(0), Rect());
	if (doc == equ)
	{
		Error("Modified document is still equilant to saved document!?");
		doc.DebugDumpObjects();
		equ.DebugDumpObjects();
		Fatal("TEST FAILED");
	}
	Debug("TEST SUCCESSFUL");
	// Cleanup

	return 0;
}


