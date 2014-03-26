#include "main.h"
#include <unistd.h> // Because we can.
int main(int argc, char ** argv)
{	
	Document doc;
	if (argc > 1)
	{
		for (int i = 2; i < argc; ++i)
		{
			if (fork() == 0) doc.Load(argv[i]);
		}
		doc.Load(argv[1]);
	}
	MainLoop(doc);
	return 0;
}
