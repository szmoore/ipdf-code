/**
 * Get diff of bmps
 */

#include <SDL.h>
#include <cassert>
#include "ipdf.h"

using namespace std;
using namespace IPDF;


int main(int argc, char ** argv)
{
	SDL_Surface * orig = SDL_LoadBMP(argv[1]);
	SDL_Surface * diff = SDL_LoadBMP(argv[2]);
	
	assert(orig->w == diff->w && orig->h == diff->h);
	int w = diff->w;
	int h = diff->h;
	
	uint8_t * a = (uint8_t*)orig->pixels;
	uint8_t * b = (uint8_t*)diff->pixels;
	
	int total_diff = 0;

	int mean_dist = 0;
	int count_a = 0;
	int count_b = 0;
	for (int x = 0; x < w; ++x)
	{
		for (int y = 0; y < h; ++y)
		{
			if (a[4*(x + w*y)] == 0)
				++count_a;
			if (b[4*(x + w*y)] == 0)
				++count_b;
			
			if (a[4*(x + w*y)] != b[4*(x + w*y)] && b[4*(x+w*y)] == 0)				
			{
				total_diff++;
				// 

				int r = 1;
				for (r=1; r+x < w; ++r)
				{
					if (a[4*(x+r + w*y)] == 0)
						break;
				}
				int l = 1;
				for (l=1; x-l >= 0; ++l)
				{
					if (a[4*(x-l + w*y)] == 0)
						break;
				}
				int u = 1;
				for (u=1; y-u >= 0; ++u)
				{
					if (a[4*(x + w*(y-u))] == 0)
						break;
				}
				
				int d = 1;
				for (d=1; y+d < h; ++d)
				{
					if (a[4*(x + w*(y+d))] == 0)
						break;
				}
				
				mean_dist += min(min(d,u), min(r,l));
			}
		}
	}
	printf("%d\t%d\t%d\t%lf\n", count_a, count_b, total_diff, (double)mean_dist / (double)count_b);
}
