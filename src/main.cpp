#include "main.h"
#include <unistd.h> // Because we can.
int main(int argc, char ** argv)
{	
	Document doc;
	srand(time(NULL));

	enum {OUTPUT_TO_BMP, LOOP} mode = LOOP;
	
	Rect bounds(0,0,1,1);
	Colour c(0,0,0,1);
	const char * input_bmp = NULL;
	const char * output_bmp = NULL;
	const char * input_filename = NULL;

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
		}	
	}

	if (input_filename != NULL)
	{
		doc.Load(input_filename);
	}
	else 
	{
		doc.Add(RECT_FILLED, Rect(0.2,0.2,0.6,0.6));
	}

	if (mode == LOOP)
		MainLoop(doc, bounds, c);
	else if (mode == OUTPUT_TO_BMP)
		OverlayBMP(doc, input_bmp, output_bmp, bounds, c);
	return 0;
}
