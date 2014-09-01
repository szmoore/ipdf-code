/**
 * @file objectrenderer.cpp
 * @brief Implements ObjectRenderer and derived classes
 */

#include "objectrenderer.h"
#include "view.h"
#include <vector>
#include <queue>
#include <stack>

using namespace std;

namespace IPDF
{

/**
 * ObjectRenderer constructor
 * Note we cannot compile the shaders in the ShaderProgram constructor
 *  because the Screen class needs to initialise GL first and it has a
 * 	ShaderProgram member
 */
ObjectRenderer::ObjectRenderer(const ObjectType & type, 
		const char * vert_glsl_file, const char * frag_glsl_file, const char * geom_glsl_file)
		: m_type(type), m_shader_program(), m_indexes(), m_buffer_builder(NULL)
{
	m_shader_program.InitialiseShaders(vert_glsl_file, frag_glsl_file, geom_glsl_file);
	m_shader_program.Use();
	glUniform4f(m_shader_program.GetUniformLocation("colour"), 0,0,0,1); //TODO: Allow different colours
}

/**
 * Render using GPU
 */
void ObjectRenderer::RenderUsingGPU(unsigned first_obj_id, unsigned last_obj_id)
{
	// If we don't have anything to render, return.
	if (first_obj_id == last_obj_id) return;
	// If there are no objects of this type, return.
	if (m_indexes.empty()) return;
	unsigned first_index = 0;
	while (m_indexes.size() > first_index && m_indexes[first_index] < first_obj_id) first_index ++;
	unsigned last_index = first_index;
	while (m_indexes.size() > last_index && m_indexes[last_index] < last_obj_id) last_index ++;

	m_shader_program.Use();
	m_ibo.Bind();
	glDrawElements(GL_LINES, (last_index-first_index)*2, GL_UNSIGNED_INT, (GLvoid*)(2*first_index*sizeof(uint32_t)));
}


/**
 * Default implementation for rendering using CPU
 */
void ObjectRenderer::RenderUsingCPU(const Objects & objects, const View & view, const CPURenderTarget & target, unsigned first_obj_id, unsigned last_obj_id)
{
	Error("Cannot render objects of type %d on CPU", m_type);
	//TODO: Render a rect or something instead?
}

/**
 * Prepare index buffers for both CPU and GPU rendering to receive indexes (but don't add any yet!)
 */
void ObjectRenderer::PrepareBuffers(unsigned max_objects)
{
	if (m_buffer_builder != NULL) // We already have a BufferBuilder
	{
		Fatal("Has been called before, without FinaliseBuffers being called since!");
	}
	// Empty and reserve the indexes vector (for CPU rendering)
	m_indexes.clear();
	m_indexes.reserve(max_objects); //TODO: Can probably make this smaller? Or leave it out? Do we care?

	// Initialise and resize the ibo (for GPU rendering)
	m_ibo.Invalidate();
	m_ibo.SetUsage(GraphicsBuffer::BufferUsageStaticDraw);
	m_ibo.SetType(GraphicsBuffer::BufferTypeIndex);
	m_ibo.Resize(max_objects * 2 * sizeof(uint32_t));
	// BufferBuilder is used to construct the ibo
	m_buffer_builder = new BufferBuilder<uint32_t>(m_ibo.Map(false, true, true), m_ibo.GetSize()); // new matches delete in ObjectRenderer::FinaliseBuffers

}

/**
 * Add object index to the buffers for CPU and GPU rendering
 */
void ObjectRenderer::AddObjectToBuffers(unsigned index)
{
	if (m_buffer_builder == NULL) // No BufferBuilder!
	{
		Fatal("Called without calling PrepareBuffers");
	}
	m_buffer_builder->Add(2*index); // ibo for GPU rendering
	m_buffer_builder->Add(2*index+1);
	m_indexes.push_back(index); // std::vector of indices for CPU rendering
}

/**
 * Finalise the index buffers for CPU and GPU rendering
 */
void ObjectRenderer::FinaliseBuffers()
{
	if (m_buffer_builder == NULL) // No BufferBuilder!
	{
		Fatal("Called without calling PrepareBuffers");
	}
	// For GPU rendering, UnMap the ibo
	m_ibo.UnMap();
	// ... and delete the BufferBuilder used to create it
	delete m_buffer_builder; // delete matches new in ObjectRenderer::PrepareBuffers
	m_buffer_builder = NULL;
	
	// Nothing is necessary for CPU rendering
}


/**
 * Rectangle (filled)
 */
void RectFilledRenderer::RenderUsingCPU(const Objects & objects, const View & view, const CPURenderTarget & target, unsigned first_obj_id, unsigned last_obj_id)
{
	for (unsigned i = 0; i < m_indexes.size(); ++i)
	{
		if (m_indexes[i] < first_obj_id) continue;
		if (m_indexes[i] >= last_obj_id) continue;
		PixelBounds bounds(CPURenderBounds(objects.bounds[m_indexes[i]], view, target));
		FloodFillOnCPU(bounds.x+1, bounds.y+1, bounds, target, Colour(0,0,0,1));
		/*
		for (int64_t x = max((int64_t)0, bounds.x); x <= min(bounds.x+bounds.w, target.w-1); ++x)
		{
			for (int64_t y = max((int64_t)0, bounds.y); y <= min(bounds.y+bounds.h, target.h-1); ++y)
			{
				int index = (x+target.w*y)*4;
				target.pixels[index+0] = 0;
				target.pixels[index+1] = 0;
				target.pixels[index+2] = 0;
				target.pixels[index+3] = 255;
			}
		}
		*/
	}
}

/**
 * Rectangle (outine)
 */
void RectOutlineRenderer::RenderUsingCPU(const Objects & objects, const View & view, const CPURenderTarget & target, unsigned first_obj_id, unsigned last_obj_id)
{
	//Debug("Render %u outlined rectangles on CPU", m_indexes.size());
	for (unsigned i = 0; i < m_indexes.size(); ++i)
	{
		if (m_indexes[i] < first_obj_id) continue;
		if (m_indexes[i] >= last_obj_id) continue;
		PixelBounds bounds(CPURenderBounds(objects.bounds[m_indexes[i]], view, target));
		
		// Using bresenham's lines now mainly because I want to see if they work
		// top
		ObjectRenderer::RenderLineOnCPU(bounds.x, bounds.y, bounds.x+bounds.w, bounds.y, target);
		// bottom
		ObjectRenderer::RenderLineOnCPU(bounds.x, bounds.y+bounds.h, bounds.x+bounds.w, bounds.y+bounds.h, target);
		// left
		ObjectRenderer::RenderLineOnCPU(bounds.x, bounds.y, bounds.x, bounds.y+bounds.h, target);
		// right
		ObjectRenderer::RenderLineOnCPU(bounds.x+bounds.w, bounds.y, bounds.x+bounds.w, bounds.y+bounds.h, target);

		// Diagonal for testing (from bottom left to top right)
		//ObjectRenderer::RenderLineOnCPU(bounds.x,bounds.y+bounds.h, bounds.x+bounds.w, bounds.y,target, C_BLUE);
		//ObjectRenderer::RenderLineOnCPU(bounds.x+bounds.w, bounds.y+bounds.h, bounds.x, bounds.y, target,C_GREEN);
	}
}

/**
 * Circle (filled)
 */
void CircleFilledRenderer::RenderUsingCPU(const Objects & objects, const View & view, const CPURenderTarget & target, unsigned first_obj_id, unsigned last_obj_id)
{
	for (unsigned i = 0; i < m_indexes.size(); ++i)
	{
		if (m_indexes[i] < first_obj_id) continue;
		if (m_indexes[i] >= last_obj_id) continue;
		PixelBounds bounds(CPURenderBounds(objects.bounds[m_indexes[i]], view, target));
		int64_t centre_x = bounds.x + bounds.w / 2;
		int64_t centre_y = bounds.y + bounds.h / 2;
		
		//Debug("Centre is %d, %d", centre_x, centre_y);
		//Debug("Bounds are %d,%d,%d,%d", bounds.x, bounds.y, bounds.w, bounds.h);
		//Debug("Windos is %d,%d", target.w, target.h);
		for (int64_t x = max((int64_t)0, bounds.x); x <= min(bounds.x+bounds.w, target.w-1); ++x)
		{
			for (int64_t y = max((int64_t)0, bounds.y); y <= min(bounds.y + bounds.h, target.h-1); ++y)
			{
				Real dx(2); dx *= Real(x - centre_x)/Real(bounds.w);
				Real dy(2); dy *= Real(y - centre_y)/Real(bounds.h);
				int64_t index = (x+target.w*y)*4;
				
				if (dx*dx + dy*dy <= Real(1))
				{
					target.pixels[index+0] = 0;
					target.pixels[index+1] = 0;
					target.pixels[index+2] = 0;
					target.pixels[index+3] = 255;
				}
			}
		}
	}
}

Rect ObjectRenderer::CPURenderBounds(const Rect & bounds, const View & view, const CPURenderTarget & target)
{
	Rect result = view.TransformToViewCoords(bounds);
	result.x *= Real(target.w);
	result.y *= Real(target.h);
	result.w *= Real(target.w);
	result.h *= Real(target.h);
	return result;
}

ObjectRenderer::PixelPoint ObjectRenderer::CPUPointLocation(const Vec2 & point, const View & view, const CPURenderTarget & target)
{
	// hack...
	Rect result = view.TransformToViewCoords(Rect(point.x, point.y,1,1));
	int64_t x = result.x*target.w;
	int64_t y = result.y*target.h;
	return PixelPoint(x,y);
}
	

/**
 * Bezier curve
 * Not sure how to apply De'Casteljau, will just use a bunch of Bresnham lines for now.
 */
void BezierRenderer::RenderUsingCPU(const Objects & objects, const View & view, const CPURenderTarget & target, unsigned first_obj_id, unsigned last_obj_id)
{
	//Warn("Rendering Beziers on CPU. Things may explode.");
	for (unsigned i = 0; i < m_indexes.size(); ++i)
	{
		if (m_indexes[i] < first_obj_id) continue;
		if (m_indexes[i] >= last_obj_id) continue;
		const Rect & bounds = objects.bounds[m_indexes[i]];
		PixelBounds pix_bounds(CPURenderBounds(bounds,view,target));

		Bezier control(objects.beziers[objects.data_indices[m_indexes[i]]].ToAbsolute(bounds),CPURenderBounds(Rect(0,0,1,1), view, target));
		//Debug("%s -> %s via %s", objects.beziers[objects.data_indices[m_indexes[i]]].Str().c_str(), control.Str().c_str(), bounds.Str().c_str());
		// Draw a rectangle around the bezier for debugging the bounds rectangle calculations
		if (view.ShowingObjectBounds())
		{
			ObjectRenderer::RenderLineOnCPU(pix_bounds.x, pix_bounds.y, pix_bounds.x+pix_bounds.w, pix_bounds.y, target, Colour(1,0,0,1));
			ObjectRenderer::RenderLineOnCPU(pix_bounds.x, pix_bounds.y+pix_bounds.h, pix_bounds.x+pix_bounds.w, pix_bounds.y+pix_bounds.h, target, Colour(0,1,0,1));
			ObjectRenderer::RenderLineOnCPU(pix_bounds.x, pix_bounds.y, pix_bounds.x, pix_bounds.y+pix_bounds.h, target, Colour(1,0,0,1));
			ObjectRenderer::RenderLineOnCPU(pix_bounds.x+pix_bounds.w, pix_bounds.y, pix_bounds.x+pix_bounds.w, pix_bounds.y+pix_bounds.h, target, Colour(0,1,0,1));
		}
		// Draw lines between the control points for debugging
		//ObjectRenderer::RenderLineOnCPU((int64_t)control.x0, (int64_t)control.y0, (int64_t)control.x1, (int64_t)control.y1,target);
		//ObjectRenderer::RenderLineOnCPU((int64_t)control.x1, (int64_t)control.y1, (int64_t)control.x2, (int64_t)control.y2,target);
										

		
	
		unsigned blen =	min(max(2U, (unsigned)(target.w/view.GetBounds().w)), min((unsigned)(pix_bounds.w+pix_bounds.h)/4 + 1, 100U));
		
		// DeCasteljau Divide the Bezier
		queue<Bezier> divisions;
		divisions.push(control);
		while(divisions.size() < blen)
		{
			Bezier & current = divisions.front();
			if (current.GetType() == Bezier::LINE)
			{
				--blen;
				continue;
			}
			divisions.push(current.DeCasteljauSubdivideRight(Real(1)/Real(2)));	
			divisions.push(current.DeCasteljauSubdivideLeft(Real(1)/Real(2)));
			divisions.pop();
			
			//Debug("divisions %u", divisions.size());
		}
		while (divisions.size() > 0)
		{
			Bezier & current = divisions.front();
			RenderLineOnCPU(current.x0, current.y0, current.x3, current.y3, target, Colour(0,0,0,!view.PerformingShading()));
			divisions.pop();
		}
		
		/* Draw the Bezier by sampling
		Real invblen(1); invblen /= blen;
		Real t(invblen);
		Vec2 v0;
		Vec2 v1;
		control.Evaluate(v0.x, v0.y, 0);
		for (int64_t j = 1; j <= blen; ++j)
		{
			control.Evaluate(v1.x, v1.y, t);
			
			ObjectRenderer::RenderLineOnCPU(v0.x, v0.y, v1.x, v1.y, target, Colour(0,0,0,!view.PerformingShading()));
			//ObjectRenderer::SetColour(target, x[0], y[0], Colour(1,0,0,1));
			//ObjectRenderer::SetColour(target, x[1], y[1], Colour(0,0,1,1));
			t += invblen;
			v0 = v1;
		}
		*/
	}
}

void BezierRenderer::PrepareBezierGPUBuffer(const Objects& objects)
{
	m_bezier_coeffs.SetType(GraphicsBuffer::BufferTypeTexture);
	m_bezier_coeffs.SetUsage(GraphicsBuffer::BufferUsageDynamicDraw);
	m_bezier_coeffs.Resize(objects.beziers.size()*sizeof(GPUBezierCoeffs));
	BufferBuilder<GPUBezierCoeffs> builder(m_bezier_coeffs.Map(false, true, true), m_bezier_coeffs.GetSize());


	for (unsigned i = 0; i < objects.beziers.size(); ++i)
	{
		const Bezier & bez = objects.beziers[i];
		
		GPUBezierCoeffs coeffs = {
			Float(bez.x0), Float(bez.y0),
			Float(bez.x1), Float(bez.y1),
			Float(bez.x2), Float(bez.y2),
			Float(bez.x3), Float(bez.y3)
			};
		builder.Add(coeffs);
	}
	m_bezier_coeffs.UnMap();
	glGenTextures(1, &m_bezier_buffer_texture);
	glBindTexture(GL_TEXTURE_BUFFER, m_bezier_buffer_texture);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_RG32F, m_bezier_coeffs.GetHandle());

	m_bezier_ids.SetType(GraphicsBuffer::BufferTypeTexture);
	m_bezier_ids.SetUsage(GraphicsBuffer::BufferUsageDynamicDraw);
	m_bezier_ids.Upload(objects.data_indices.size() * sizeof(uint32_t), &objects.data_indices[0]);
	
	glGenTextures(1, &m_bezier_id_buffer_texture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_BUFFER, m_bezier_id_buffer_texture);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_R32I, m_bezier_ids.GetHandle());
	glActiveTexture(GL_TEXTURE0);
}

void BezierRenderer::RenderUsingGPU(unsigned first_obj_id, unsigned last_obj_id)
{
	if (!m_shader_program.Valid())
		Warn("Shader is invalid (objects are of type %d)", m_type);

	// If we don't have anything to render, return.
	if (first_obj_id == last_obj_id) return;
	// If there are no objects of this type, return.
	if (m_indexes.empty()) return;

	unsigned first_index = 0;
	while (m_indexes.size() > first_index && m_indexes[first_index] < first_obj_id) first_index ++;
	unsigned last_index = first_index;
	while (m_indexes.size() > last_index && m_indexes[last_index] < last_obj_id) last_index ++;

	m_shader_program.Use();
	glUniform1i(m_shader_program.GetUniformLocation("bezier_buffer_texture"), 0);
	glUniform1i(m_shader_program.GetUniformLocation("bezier_id_buffer_texture"), 1);
	m_ibo.Bind();
	glDrawElements(GL_LINES, (last_index-first_index)*2, GL_UNSIGNED_INT, (GLvoid*)(2*first_index*sizeof(uint32_t)));
}



/**
 * Render Path (shading)
 */
void PathRenderer::RenderUsingCPU(const Objects & objects, const View & view, const CPURenderTarget & target, unsigned first_obj_id, unsigned last_obj_id)
{
	if (!view.ShowingObjectBounds() && !view.PerformingShading())
		return;
		
	for (unsigned i = 0; i < m_indexes.size(); ++i)
	{
		if (m_indexes[i] < first_obj_id) continue;
		if (m_indexes[i] >= last_obj_id) continue;
		
		
		Rect bounds(CPURenderBounds(objects.bounds[m_indexes[i]], view, target));
		PixelBounds pix_bounds(bounds);
		const Path & path = objects.paths[objects.data_indices[m_indexes[i]]];
		if (path.m_fill.a == 0 || !view.PerformingShading())
			continue;
		
		for (unsigned f = 0; f < path.m_fill_points.size(); ++f)
		{
			PixelPoint fill_point(CPUPointLocation(path.m_fill_points[f], view, target));
			FloodFillOnCPU(fill_point.first, fill_point.second, pix_bounds, target, path.m_fill);
		}
		
		/*if (true)//(view.ShowingObjectBounds())
		{
			
			PixelPoint start(CPUPointLocation((path.m_top+path.m_left+path.m_right+path.m_bottom)/4, view, target));
			for (unsigned f = 0; f < path.m_fill_points.size(); ++f)
			{
				PixelPoint end(CPUPointLocation(path.m_fill_points[f], view, target));
				RenderLineOnCPU(start.first, start.second, end.first, end.second, target, Colour(0,0,1,1));
			}
		}
		*/
	
	}	
}

/**
 * For debug, save pixels to bitmap
 */
void ObjectRenderer::SaveBMP(const CPURenderTarget & target, const char * filename)
{
	SDL_Surface * surf = SDL_CreateRGBSurfaceFrom(target.pixels, target.w, target.h, 8*4, target.w*4,
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
}

/**
 * Bresenham's lines
 */
void ObjectRenderer::RenderLineOnCPU(int64_t x0, int64_t y0, int64_t x1, int64_t y1, const CPURenderTarget & target, const Colour & colour, bool transpose)
{
	int64_t dx = x1 - x0;
	int64_t dy = y1 - y0;
	bool neg_m = (dy*dx < 0);
	dy = abs(dy);
	dx = abs(dx);

	// If positive slope > 1, just swap x and y
	if (dy > dx)
	{
		RenderLineOnCPU(y0,x0,y1,x1,target,colour,!transpose);
		return;
	}

	int64_t two_dy = 2*dy;
	int64_t p = two_dy - dx;
	int64_t two_dxdy = 2*(dy-dx);
	int64_t x; int64_t y; int64_t x_end;
	int64_t width = (transpose ? target.h : target.w);
	int64_t height = (transpose ? target.w : target.h);

	uint8_t rgba[4];
	rgba[0] = 255*colour.r;
	rgba[1] = 255*colour.g;
	rgba[2] = 255*colour.b;
	rgba[3] = 255*colour.a;

	if (x0 > x1)
	{
		x = x1;
		y = y1;
		x_end = x0;
	}
	else
	{
		x = x0;
		y = y0;
		x_end = x1;
	}

	if (x < 0)
	{
		if (x_end < 0) return;
		y = (neg_m ? y - (dy*-x)/dx : y + (dy*-x)/dx);
		x = 0;
	}
	
	if (x_end > width)
	{
		if (x > width) return;
		x_end = width-1;
	}

	// TODO: Avoid extra inner conditionals
	do
	{	
		if (x >= 0 && x < width && y >= 0 && y < height)
		{
			int64_t index = (transpose ? (y + x*target.w)*4 : (x + y*target.w)*4);
			for (int i = 0; i < 4; ++i)
				target.pixels[index+i] = rgba[i];
		}
		if (p < 0)
			p += two_dy;
		else
		{
			if (neg_m) --y; else ++y;
			p += two_dxdy;
		}
	} while (++x <= x_end);
}

void ObjectRenderer::FloodFillOnCPU(int64_t x, int64_t y, const PixelBounds & bounds, const CPURenderTarget & target, const Colour & fill)
{
	if (fill == Colour(1,1,1,1))
		return;
	queue<PixelPoint > traverse;
	traverse.push(PixelPoint(x,y));
	// now with 100% less stack overflows!
	while (traverse.size() > 0)
	{
		PixelPoint cur(traverse.front());
		traverse.pop();
		if (cur.first < 0 || cur.first < bounds.x || cur.first >= bounds.x+bounds.w || cur.first >= target.w ||
			cur.second < 0 || cur.second < bounds.y || cur.second >= bounds.y+bounds.h || cur.second >= target.h)
			continue;
		if (GetColour(target, cur.first, cur.second) != Colour(1,1,1,1))
			continue;
		SetColour(target, cur.first, cur.second, fill);
		

		traverse.push(PixelPoint(cur.first+1, cur.second));
		traverse.push(PixelPoint(cur.first-1, cur.second));
		traverse.push(PixelPoint(cur.first, cur.second-1));
		traverse.push(PixelPoint(cur.first, cur.second+1)); 
	}
}

}
