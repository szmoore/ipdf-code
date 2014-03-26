#include "common.h"

#include "document.h"
#include "view.h"
#include "screen.h"

using namespace std;
using namespace IPDF;

int main(int argc, char ** argv)
{
	Document doc;
	doc.Add(0.5, 0.5, 0.5, 0.5);

	View view(doc);

	Screen scr;

	while (scr.PumpEvents())
	{
		view.Render();
		scr.Present();
	}

	return 0;
}
