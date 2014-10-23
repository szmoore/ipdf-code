#include "common.h"

#include "document.h"
#include "view.h"
#include "screen.h"
#include "debugscript.h"
#include <unistd.h>


using namespace std;
using namespace IPDF;


extern const char *script_filename;
extern bool make_movie; // whyyy
extern const char * program_name;

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

// It is the only way.
// Dear god what have I done
void RatCatcher(int x, int y, int buttons, int wheel, Screen * scr, View * view)
{
	static bool oldButtonDown = false;
	static int oldx, oldy;
	if (buttons == 3 && !oldButtonDown)
	{
		oldButtonDown = true;
		view->ToggleGPUTransform();
		oldx = x;
		oldy = y;
		return;
	}
	if (buttons == 2 && !oldButtonDown)
	{
		oldButtonDown = true;
		view->ToggleGPURendering();
		oldx = x;
		oldy = y;
	}
	if (buttons && !oldButtonDown)
	{
		// We're beginning a drag.
		oldButtonDown = true;
		oldx = x;
		oldy = y;
		scr->SetMouseCursor(Screen::CursorMove);
	}
	if (buttons)
	{
		view->Translate(Real(oldx-x)/Real(scr->ViewportWidth()), Real(oldy-y)/Real(scr->ViewportHeight()));
	}
	else
	{
		oldButtonDown = false;
		scr->SetMouseCursor(Screen::CursorArrow);
	}
	oldx = x;
	oldy = y;
		
	if (wheel)
	{
		view->ScaleAroundPoint(Real(x)/Real(scr->ViewportWidth()),Real(y)/Real(scr->ViewportHeight()), exp(Real(-wheel)/Real(20)));
	}
}


void MainLoop(Document & doc, Screen & scr, View & view, int max_frames = -1)
{
	// order is important... segfaults occur when screen (which inits GL) is not constructed first -_-
	

	//scr.DebugFontInit("fonts/DejaVuSansMono.ttf", 12);
	scr.DebugFontInit("fonts/DejaVuSansMono.ttf", 36);
	scr.SetMouseHandler(RatCatcher);

	ifstream tmp;
	istream * script_input = NULL;
	if (script_filename != NULL)
	{
		if (strcmp(script_filename, "stdin") == 0)
			script_input = &cin;
		else
		{
			tmp.open(script_filename);
			script_input = &tmp;
		}
	}
	DebugScript script(script_input);

	double total_cpu_time = 0;
	double total_gpu_time = 0;
	double total_real_time = 0;

	// MINGW doesn't support a lot of ctime stuff here
	#ifndef __MINGW32__	
	struct timespec real_clock_start;
	struct timespec real_clock_now;
	struct timespec real_clock_prev;
	clock_gettime(CLOCK_MONOTONIC_RAW, &real_clock_start);
	real_clock_now = real_clock_start;
	#endif


	double frames = 0;
	double data_rate = 0; // period between data output to stdout (if <= 0 there will be no output)
	uint64_t data_points = 0;
	setbuf(stdout, NULL);
	int frame_number = 0;
	while (scr.PumpEvents() && (max_frames < 0 || frame_number++ < max_frames))
	{
		#ifndef __MINGW32__
		real_clock_prev = real_clock_now;
		#endif
		++frames;
		scr.Clear();
		//view.ForceBoundsDirty();
		//view.ForceBufferDirty();
		//view.ForceRenderDirty();

		if (script_filename)
		{
			if (script.Execute(&view, &scr))
				return;
		}

		view.Render(scr.ViewportWidth(), scr.ViewportHeight());

		double cpu_frame = scr.GetLastFrameTimeCPU();
		double gpu_frame = scr.GetLastFrameTimeGPU();
		total_cpu_time += cpu_frame; total_gpu_time += gpu_frame;
		
		#ifndef __MINGW32__
		clock_gettime(CLOCK_MONOTONIC_RAW, &real_clock_now);
		double real_frame = (real_clock_now.tv_sec - real_clock_prev.tv_sec) + 1e-9*(real_clock_now.tv_nsec - real_clock_prev.tv_nsec);
		#else
		double real_frame = cpu_frame;
		#endif
		
		total_real_time += real_frame; 
		if (data_rate > 0 && total_real_time > data_rate*(data_points+1)) 
		{
			printf("%lu\t%f\t%f\t%f\t%f\t%f\t%f\n", (long unsigned int)frames, total_real_time, total_cpu_time, total_gpu_time, real_frame, cpu_frame, gpu_frame);
			data_points++;
		}
		

		
		scr.DebugFontPrintF("%s\n", program_name);
		scr.DebugFontPrintF("Top Left: (%s,%s)\n", Str(view.GetBounds().x).c_str(),Str(view.GetBounds().y).c_str());
		scr.DebugFontPrintF("Width: %s\n", Str(view.GetBounds().w).c_str());
		Real zoom(100);
		zoom = zoom/Real(view.GetBounds().w);
		scr.DebugFontPrintF("Zoom: %s %%\n", Str(zoom).c_str());
		scr.DebugFontPrintF("Similar size: %s\n", HumanScale(ClampFloat(Double(view.GetBounds().w))));
		
		#if 0
		scr.DebugFontPrintF("Rendered frame %lu\n", (uint64_t)frames);
		scr.DebugFontPrintF("Lazy Rendering = %d\n", view.UsingLazyRendering());
		if (cpu_frame > 0 && total_cpu_time > 0)
			scr.DebugFontPrintF("[CPU] Render took %lf ms (%lf FPS) (total %lf s, avg FPS %lf)\n", cpu_frame*1e3, 1.0/cpu_frame, total_cpu_time,frames/total_cpu_time);
		if (gpu_frame > 0 && total_gpu_time > 0)
			scr.DebugFontPrintF("[GPU] Render took %lf ms (%lf FPS) (total %lf s, avg FPS %lf)\n", gpu_frame*1e3, 1.0/gpu_frame, total_gpu_time, frames/total_gpu_time);
		
		if (real_frame > 0 && total_real_time > 0)
			scr.DebugFontPrintF("[REALTIME] Render took %lf ms (%lf FPS) (total %lf s, avg FPS %lf)\n", real_frame*1e3, 1.0/real_frame, total_real_time,frames/total_real_time);

		//scr.DebugFontPrintF("View bounds: %s\n", view.GetBounds().Str().c_str());
		scr.DebugFontPrintF("type of Real == %s\n", g_real_name[REALTYPE]);
		//#if REALTYPE == REAL_MPFRCPP
		//	scr.DebugFontPrintf("Precision: %s\nRounding: %s\n");
		//#endif 

		#ifdef TRANSFORM_OBJECTS_NOT_VIEW
		scr.DebugFontPrint("Doing cumulative coordinate transforms on Objects.\n");
		#else
		if (view.UsingGPUTransform())
		{
			scr.DebugFontPrint("Doing coordinate transform on the GPU.\n");
		}
		else
		{
			scr.DebugFontPrint("Doing coordinate transform on the CPU.\n");
		}
		#endif
		
		#ifdef TRANSFORM_BEZIERS_TO_PATH
			scr.DebugFontPrint("Beziers have been transformed to Path\n");
		#endif

		
		if (view.UsingGPURendering())
		{
			scr.DebugFontPrint("Doing rendering using GPU.\n");
		}
		else
		{
			scr.DebugFontPrint("Doing rendering using CPU.\n");
		}
		#endif // 0

		scr.Present();

		if (make_movie)
		{
			std::stringstream s;
			s << "frame" << frames << ".bmp";
			scr.ScreenShot(s.str().c_str());
		}		

	}
}
