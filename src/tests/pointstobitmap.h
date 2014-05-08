#ifndef _POINTSTOBITMAP_H
#define _POINTSTOBITMAP_H

#include <SDL.h>

/**
 * Map vector of points onto a Bitmap
 * Because it was easier than working out the OpenGL stuff right now
 * Points will be mapped according to the bounding rectangle.
 * Should probably deal with possible aspect ratio difference... or just make sure to always use a square I guess
 */
template <class T> void PointsToBitmap(const vector<pair<T, T> > & points, const T & scale = 1, uint8_t r = 0, uint8_t g = 0, uint8_t b = 0, const char * filename = "bezier.bmp",  bool overlay = false, uint64_t width = 400, uint64_t height = 400)
{

	int ri = (SDL_BYTEORDER == SDL_LIL_ENDIAN) ? 0 : 3;
	int gi = (SDL_BYTEORDER == SDL_LIL_ENDIAN) ? 1 : 2;
	int bi = (SDL_BYTEORDER == SDL_LIL_ENDIAN) ? 2 : 1;
	//int ai = (SDL_BYTEORDER == SDL_LIL_ENDIAN) ? 3 : 0; // No alpha in BMP

	// Pixel buffer
	unsigned char * pixels = new unsigned char[width*height*4];

	// Overlay indicates we should load the bitmap into the pixel buffer first
	if (overlay)
	{
		SDL_Surface * bmp = SDL_LoadBMP(filename);
		if (bmp == NULL)
		{
			overlay = false; // bmp (probably) doesn't exist -> not an error (we hope)
		}
		else
		{
			width = bmp->w;
			height = bmp->h;
			for (int i = 0; i < width*height*(bmp->format->BytesPerPixel); ++i)
			{
				// We're assuming the BMP was stored in the same Byteorder as we will save it
				pixels[i] = *((unsigned char*)(bmp->pixels)+i); 
			}
		}
	}
	if (!overlay)
	{
		for (int i = 0; i < width*height*4; ++i)
			pixels[i] = 0xFF; // White pixels
	}


	typedef long double LD; // The temporary typedef, an underappreciated technique...

	// Named lambdas... this is getting worrying...
	auto lessx = [](const pair<T, T> & a, const pair<T, T> & b){return (a.first < b.first);};	
	auto lessy = [](const pair<T, T> & a, const pair<T, T> & b){return (a.second < b.second);};	
	
	// So I don't have to type as much here
	pair<T,T> left = *min_element(points.begin(), points.end(), lessx);
	pair<T,T> right = *max_element(points.begin(), points.end(), lessx);
	pair<T,T> bottom = *min_element(points.begin(), points.end(), lessy);
	pair<T,T> top = *max_element(points.begin(), points.end(),lessy);

	pair<LD,LD> min(left.first, bottom.second);
	pair<LD,LD> max(right.first, top.second);

	// Alternately, just do this:
	/* 
	pair<LD,LD> min(-scale, -scale);
	pair<LD,LD> max(scale, scale);
	*/

	// Map each point to a pixel position
	for (auto i = 0; i < points.size(); ++i)
	{
		// Do maths with long double; this way any artefacts are more likely due to the creation of the bezier itself than this mapping operation
		uint64_t x = llround(((LD)(points[i].first) - (LD)(min.first)) * (LD)(width-1)/((LD)max.first - (LD)min.first));
		uint64_t y = llround(((LD)(points[i].second) - (LD)(min.second)) * (LD)(height-1)/((LD)max.second - (LD)min.second));
		int index = 4*(y*width + x); // Get index into pixel array
		// Set colour
		pixels[index+ri] = r;
		pixels[index+gi] = g;
		pixels[index+bi] = b;
	}

	// Truly a thing of beauty
	SDL_Surface * surf = SDL_CreateRGBSurfaceFrom(pixels, width, height, 8*4, width*4, 
	#if SDL_BYTEORDER == SDL_LIL_ENDIAN
		0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000
	#else
		0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff
	#endif //SDL_BYTEORDER	
	);

	if (surf == NULL)
		Fatal("SDL_CreateRGBSurfaceFrom(pixels...) failed - %s", SDL_GetError());
	if (SDL_SaveBMP(surf, filename) != 0)
		Fatal("SDL_SaveBMP failed - %s", SDL_GetError());

	// Cleanup
	SDL_FreeSurface(surf);
	delete [] pixels;
}

#endif //_POINTSTOBITMAP_H
