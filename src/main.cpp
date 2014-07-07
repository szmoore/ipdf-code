#include "main.h"
#include <unistd.h> // Because we can.
int main(int argc, char ** argv)
{	
	#ifndef __STDC_IEC_559__
       	Warn("__STDC_IEC_559__ not defined. IEEE 754 floating point not fully supported.\n");
	#endif


	Debug("Compiled with REAL = %d => \"%s\" sizeof(Real) == %d bytes", REAL, g_real_name[REAL], sizeof(Real));

	Document doc;
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
		doc.Load(input_filename);
	}
	else 
	{
		doc.AddBezierData(Bezier(0,0,0,1,1,0));
		//doc.AddBezierData(Bezier(0,0,1,0,0,1));
		//doc.AddBezierData(Bezier(0,0,1,1,1,0));
		//doc.AddBezierData(Bezier(0,1,1,0,0,1));
		
		
		
		for(int x = 0; x < 8; ++x)
		{
			
			for (int y = 0; y < 8; ++y)
			{
				//doc.Add(static_cast<IPDF::ObjectType>((x^y)%3), Rect(0.2+x-4.0,0.2+y-4.0,0.6,0.6));
				//doc.Add(BEZIER, Rect(0.2+x-4.0, 0.2+y-4.0, 0.6,0.6), (x^y)%3);
			}
		}
		doc.Add(BEZIER, Rect(0.1,0.1,0.8,0.8), 0);
	}
	Debug("Start!");
	Rect bounds(b[0],b[1],b[2],b[3]);

	if (mode == LOOP)
		MainLoop(doc, bounds, c);
	else if (mode == OUTPUT_TO_BMP)
		OverlayBMP(doc, input_bmp, output_bmp, bounds, c);
	return 0;
}
