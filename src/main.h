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

	double total_cpu_time = 0;
	double total_gpu_time = 0;
	double total_real_time = 0;
	struct timespec real_clock_start;
	struct timespec real_clock_now;
	struct timespec real_clock_prev;
	clock_gettime(CLOCK_MONOTONIC_RAW, &real_clock_start);
	real_clock_now = real_clock_start;
	double frames = 0;
	double data_rate = 1; // period between data output to stdout (if <= 0 there will be no output)
	uint64_t data_points = 0;
	setbuf(stdout, NULL);
	while (scr.PumpEvents())
	{
		real_clock_prev = real_clock_now;
		++frames;
		scr.Clear();
		//view.ForceBoundsDirty();
		//view.ForceBufferDirty();
		//view.ForceRenderDirty();

		view.Render(scr.ViewportWidth(), scr.ViewportHeight());

		double cpu_frame = scr.GetLastFrameTimeCPU();
		double gpu_frame = scr.GetLastFrameTimeGPU();
		clock_gettime(CLOCK_MONOTONIC_RAW, &real_clock_now);
		double real_frame = (real_clock_now.tv_sec - real_clock_prev.tv_sec) + 1e-9*(real_clock_now.tv_nsec - real_clock_prev.tv_nsec);


		total_real_time += real_frame; total_cpu_time += cpu_frame; total_gpu_time += gpu_frame;
		if (data_rate > 0 && total_real_time > data_rate*(data_points+1)) 
		{
			printf("%lu\t%f\t%f\t%f\t%f\t%f\t%f\n", (uint64_t)frames, total_real_time, total_cpu_time, total_gpu_time, real_frame, cpu_frame, gpu_frame);
			data_points++;
		}
		scr.DebugFontPrintF("Rendered frame %lu\n", (uint64_t)frames);
		scr.DebugFontPrintF("[CPU] Render took %lf ms (%lf FPS) (total %lf s, avg FPS %lf)\n", cpu_frame*1e3, 1.0/cpu_frame, total_cpu_time,frames/total_cpu_time);
		scr.DebugFontPrintF("[GPU] Render took %lf ms (%lf FPS) (total %lf s, avg FPS %lf)\n", gpu_frame*1e3, 1.0/gpu_frame, total_gpu_time, frames/total_gpu_time);
		scr.DebugFontPrintF("[REALTIME] Render+Present+Cruft took %lf ms (%lf FPS) (total %lf s, avg FPS %lf)\n", real_frame*1e3, 1.0/real_frame, total_real_time,frames/total_real_time);
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
