#include "main.h"
#include <unistd.h> // Because we can.

#include "controlpanel.h"

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <fenv.h>
#include <signal.h>

bool ignore_sigfpe = false;

void sigfpe_handler(int sig)
{
	if (!ignore_sigfpe)
		Fatal("Floating point exception!");
	exit(EXIT_SUCCESS);
}

int main(int argc, char ** argv)
{	
	signal(SIGFPE, sigfpe_handler);
	#if REALTYPE == REAL_IRRAM
	  iRRAM_initialize(argc,argv);
	#endif
	
	#ifndef __STDC_IEC_559__
       	Warn("__STDC_IEC_559__ not defined. IEEE 754 floating point not fully supported.\n");
	#endif

	// We want to crash if we ever get a NaN.
	feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);

	Debug("Compiled with REAL = %d => \"%s\" sizeof(Real) == %d bytes", REALTYPE, g_real_name[REALTYPE], sizeof(Real));

	Document doc("","fonts/ComicSans.ttf");
	srand(time(NULL));

	enum {OUTPUT_TO_BMP, LOOP} mode = LOOP;
	
	
	Colour c(0,0,0,1);
	
	const char * output_bmp = NULL;
	const char * input_filename = NULL;
	const char * input_text = NULL;
	float b[4] = {0,0,1,1};
	int max_frames = -1;
	bool hide_control_panel;
	bool lazy_rendering = true;
	

	
	Screen scr;
	View view(doc,scr, {0,0,1,1});
	
	if (!lazy_rendering)
		view.SetLazyRendering(false);
	
	int i = 0;
	while (++i < argc)
	{
		if (argv[i][0] != '-')
		{
			input_filename = argv[i];
			continue;		
		}
		switch (argv[i][1])
		{
			case 'o':
				mode = OUTPUT_TO_BMP;
				if (++i >= argc)
					Fatal("No output argument following -o switch");
				output_bmp = argv[i];
				hide_control_panel = true;
				break;
			case 'b':
			{
				Debug("Reading view bounds");
				for (int j = 1; j <= 4; ++j)
				{
					if (i+j >= argc)
						Fatal("No %d bounds component following -b switch", j);
					char * e;
					b[j-1] = strtof(argv[i+j], &e);
					if (*e != '\0')
						Fatal("Bounds component %d not a valid float", j); 
				}
				i += 4;
				break;
			}
			case 't':
			{
				if (++i >= argc)
					Fatal("No text input following -t switch");
				input_text = argv[i];
				Debug("Insert text: %s", input_text);
				break;
			}
			
			case 'r':
			{
				if (++i >= argc)
					Fatal("Expected \"gpu\" or \"cpu\" after -r switch");
				if (strcmp(argv[i], "gpu") == 0)
				{
					view.SetGPURendering(true);
				}
				else if (strcmp(argv[i], "cpu") == 0)
				{
					view.SetGPURendering(false);
				}
				else
				{
					Fatal("Expected \"gpu\" or \"cpu\" after -r switch, not \"%s\"", argv[i]);
				}
				break;
			}
			
			case 'T':
			{
				if (++i >= argc)
					Fatal("Expected \"gpu\" or \"cpu\" after -T switch");
				if (strcmp(argv[i], "gpu") == 0)
				{
					view.SetGPUTransform(true);
				}
				else if (strcmp(argv[i], "cpu") == 0)
				{
					view.SetGPUTransform(false);
				}
				else
				{
					Fatal("Expected \"gpu\" or \"cpu\" after -T switch, not \"%s\"", argv[i]);
				}
				break;
			}
			
			
			case 'l':
				view.SetLazyRendering(!view.UsingLazyRendering());
				break;
			
			case 'f':
				if (++i >= argc)
					Fatal("No frame number following -f switch");
				max_frames = strtol(argv[i], NULL, 10);
				hide_control_panel = true;
				break;
				
			case 'q':
				hide_control_panel = true;
				break;
					
		}	
	}

	Rect bounds(b[0],b[1],b[2],b[3]);
	view.SetBounds(bounds);
	if (input_filename != NULL)
	{
		
		doc.LoadSVG(input_filename, Rect(bounds.x+bounds.w/Real(2),bounds.y+bounds.h/Real(2),bounds.w/Real(800),bounds.h/Real(600)));
	}
	else if (input_text != NULL)
	{
		doc.AddText(input_text, bounds.h/Real(2), bounds.x, bounds.y+bounds.h/Real(2));
	}
	else
	{
		doc.Add(RECT_OUTLINE, Rect(0,0,0,0),0); // hack to stop segfault if document is empty (:S)
	}


	#ifndef CONTROLPANEL_DISABLED
	SDL_Thread * cp_thread = NULL;
	if (!hide_control_panel)
	{
		ControlPanel::RunArgs args = {argc, argv, view, doc, scr};
		cp_thread = SDL_CreateThread(ControlPanel::Run, "ControlPanel", &args);
		if (cp_thread == NULL)
		{
			Error("Couldn't create ControlPanel thread: %s", SDL_GetError());
		}
	}
	#endif //CONTROLPANEL_DISABLED

	if (mode == LOOP)
		MainLoop(doc, scr, view, max_frames);
	else if (mode == OUTPUT_TO_BMP) //TODO: Remove this shit
	{
		if (view.UsingGPURendering())
			OverlayBMP(doc, output_bmp, output_bmp, bounds, c);
		else
			view.SaveCPUBMP(output_bmp);
	}
		
	#ifndef CONTROLPANEL_DISABLED
		if (cp_thread != NULL)
		{
			int cp_return;
			qApp->quit(); // will close the control panel
			// (seems to not explode if the qApp has already been quit)
			SDL_WaitThread(cp_thread, &cp_return);
			Debug("ControlPanel thread returned %d", cp_return);
		}
	#endif //CONTROLPANEL_DISABLED
	
	ignore_sigfpe = true;
	return 0;
}
