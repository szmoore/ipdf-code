#include "main.h"
#include <unistd.h> // Because we can.
int main(int argc, char ** argv)
{	
	Document doc;
	srand(time(NULL));
	if (argc > 1)
	{
		for (int i = 2; i < argc; ++i)
		{
			if (fork() == 0) doc.Load(argv[i]);
		}
		doc.Load(argv[1]);
	}
	else
	{
		Debug("Add random object");
		//doc.Add(RECT_FILLED, Rect(Random()*0.5, Random()*0.5, Random()*0.5, Random()*0.5));
		doc.Add(RECT_FILLED, Rect(0.25,0.25, 0.5, 0.5));
	}
	MainLoop(doc);
	return 0;
}
