#include "main.h"
#include <unistd.h> // Because we can.

#include "controlpanel.h"

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <fenv.h>


int main(int argc, char ** argv)
{	
	#ifndef __STDC_IEC_559__
       	Warn("__STDC_IEC_559__ not defined. IEEE 754 floating point not fully supported.\n");
	#endif

	// We want to crash if we ever get a NaN.
	feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);

	Debug("Compiled with REAL = %d => \"%s\" sizeof(Real) == %d bytes", REAL, g_real_name[REAL], sizeof(Real));

	Document doc("","fonts/ComicSans.ttf");
	srand(time(NULL));

	enum {OUTPUT_TO_BMP, LOOP} mode = LOOP;
	
	
	Colour c(0,0,0,1);
	const char * input_bmp = NULL;
	const char * output_bmp = NULL;
	const char * input_filename = NULL;
	float b[4] = {0,0,1,1};

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
					Fatal("No input argument following -o switch");
				input_bmp = argv[i];
				if (++i >= argc)
					Fatal("No output argument following -o switch");
				output_bmp = argv[i];

				break;
			case 'c':
			{
				Debug("Reading paint colour");
				for (int j = 1; j <= 4; ++j)
				{
					if (i+j >= argc)
						Fatal("No %d colour component following -c switch", j);
					char * e;
					float * comp = (j == 1) ? (&c.r) : ((j == 2) ? (&c.g) : ((j == 3) ? (&c.b) : &(c.a)));
					*comp = strtof(argv[i+j], &e);
					if (*e != '\0')
						Fatal("Colour component %d not a valid float", j); 
				}
				i += 4;
				break;
			}
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
		}	
	}

	if (input_filename != NULL)
	{
		doc.LoadSVG(input_filename, Rect(0,0,Real(1)/Real(800),Real(1)/Real(600)));
	}
	else 
	{
		doc.Add(RECT_OUTLINE, Rect(0,0,0,0),0); // hack to stop segfault if document is empty (:S)
	}
	Debug("Start!");
	Rect bounds(b[0],b[1],b[2],b[3]);
	
	Screen scr;
	View view(doc,scr, bounds);

	#ifndef CONTROLPANEL_DISABLED
		ControlPanel::RunArgs args = {argc, argv, view, doc, scr};
		SDL_Thread * cp_thread = SDL_CreateThread(ControlPanel::Run, "ControlPanel", &args);
		if (cp_thread == NULL)
		{
			Error("Couldn't create ControlPanel thread: %s", SDL_GetError());
		}
	#endif //CONTROLPANEL_DISABLED

	if (mode == LOOP)
		MainLoop(doc, scr, view);
	else if (mode == OUTPUT_TO_BMP) //TODO: Remove this shit
		OverlayBMP(doc, input_bmp, output_bmp, bounds, c);
		
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
	return 0;
}
