#include "common.h"

#include "document.h"
#include "view.h"
#include "screen.h"
#include <unistd.h>


using namespace std;
using namespace IPDF;

inline void OverlayBMP(Document & doc, const char * input, const char * output, const Rect & bounds = Rect(0,0,1,1), const Colour & c = Colour(0.f,0.f,0.f,1.f))
{

	Screen scr;
	View view(doc, scr, bounds, c);
	if (input != NULL)
		scr.RenderBMP(input);
	view.Render();
	if (output != NULL)
		scr.ScreenShot(output);
	scr.Present();
}

inline void MainLoop(Document & doc, const Rect & bounds = Rect(0,0,1,1), const Colour & c = Colour(0.f,0.f,0.f,1.f))
{
	// order is important... segfaults occur when screen (which inits GL) is not constructed first -_-
	Screen scr;
	View view(doc,scr, bounds, c);
	scr.DebugFontInit("DejaVuSansMono.ttf");
	scr.SetMouseHandler([&](int x, int y, int buttons, int wheel) // [?] wtf
	{
		static bool oldButtonDown = false;
		static int oldx, oldy;
		if (buttons == 3 && !oldButtonDown)
		{
			oldButtonDown = true;
			view.ToggleGPUTransform();
			oldx = x;
			oldy = y;
			return;
		}
		if (buttons == 2 && !oldButtonDown)
		{
			oldButtonDown = true;
			view.ToggleGPURendering();
			oldx = x;
			oldy = y;
		}
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
		scr.Clear();
		view.Render(scr.ViewportWidth(), scr.ViewportHeight());
		scr.DebugFontPrintF("[CPU] Render took %lf ms (%lf FPS)\n", (scr.GetLastFrameTimeCPU())* 1000.0, 1.0/scr.GetLastFrameTimeCPU());
		scr.DebugFontPrintF("[GPU] Render took %lf ms (%lf FPS)\n", (scr.GetLastFrameTimeGPU())* 1000.0, 1.0/scr.GetLastFrameTimeGPU());
		scr.DebugFontPrintF("View bounds: %s\n", view.GetBounds().Str().c_str());
		if (view.UsingGPUTransform())
		{
			scr.DebugFontPrint("Doing coordinate transform on the GPU.\n");
		}
		else
		{
			scr.DebugFontPrint("Doing coordinate transform on the CPU.\n");
		}
		if (view.UsingGPURendering())
		{
			scr.DebugFontPrint("Doing rendering using GPU.\n");
		}
		else
		{
			scr.DebugFontPrint("Doing rendering using CPU.\n");
		}
		scr.Present();
	}
}
