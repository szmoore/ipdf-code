#include "common.h"

#include "document.h"
#include "view.h"
#include "screen.h"
#include <unistd.h>


using namespace std;
using namespace IPDF;

inline void OverlayBMP(Document & doc, const char * input, const char * output, const Rect & bounds = Rect(0,0,1,1), const Colour & c = Colour(0.f,0.f,0.f,1.f))
{
	View view(doc, bounds, c);
	Screen scr;
	scr.RenderBMP(input);
	view.Render();
	scr.Present();
	sleep(5);
	scr.RenderBMP(input);
	view.Render();
	scr.ScreenShot(output);
}

inline void MainLoop(Document & doc, const Rect & bounds = Rect(0,0,1,1), const Colour & c = Colour(0.f,0.f,0.f,1.f))
{
	View view(doc,bounds, c);
	Screen scr;
	scr.DebugFontInit("DejaVuSansMono.ttf");
	scr.SetMouseHandler([&](int x, int y, int buttons, int wheel) // [?] wtf
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

	double init_time = SDL_GetPerformanceCounter();
	while (scr.PumpEvents())
	{
		scr.Clear();
		view.Render();
		scr.DebugFontPrintF("[CPU] Render took %lf ms (%lf FPS)\n", (SDL_GetPerformanceCounter() - init_time)* 1000.0/SDL_GetPerformanceFrequency(), SDL_GetPerformanceFrequency()/(SDL_GetPerformanceCounter() - init_time));
		scr.DebugFontPrintF("View bounds: (%f, %f) - (%f, %f)\n", view.GetBounds().x, view.GetBounds().y, view.GetBounds().w, view.GetBounds().h);
		scr.Present();
		init_time = SDL_GetPerformanceCounter();
	}
}
