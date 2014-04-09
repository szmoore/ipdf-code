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
	scr.SetMouseHandler([&](int x, int y, int buttons, int wheel)
	{
		static bool oldButtonDown = false;
		static int oldx, oldy;
		if (buttons && !oldButtonDown)
		{
			// We're beginning a drag.
			oldButtonDown = true;
			oldx = x;
			oldy = y;
			scr.SetMouseCursor(Screen::CursorMove);
		}
		if (buttons)
		{
			view.Translate(Real(oldx-x)/Real(scr.ViewportWidth()), Real(oldy-y)/Real(scr.ViewportHeight()));
		}
		else
		{
			oldButtonDown = false;
			scr.SetMouseCursor(Screen::CursorArrow);
		}
		oldx = x;
		oldy = y;
		
		if (wheel)
		{
			view.ScaleAroundPoint(Real(x)/Real(scr.ViewportWidth()),Real(y)/Real(scr.ViewportHeight()), expf(-wheel/20.f));
		}
	}
	);
	while (scr.PumpEvents())
	{
		view.Render();
		scr.Present();
	}
}
