#include "common.h"

#include "document.h"
#include "view.h"
#include "screen.h"


using namespace std;
using namespace IPDF;

inline void MainLoop(Document & doc)
{
	View view(doc);
	Screen scr;
	while (scr.PumpEvents())
	{
		view.Render();
		scr.Present();
	}
}
