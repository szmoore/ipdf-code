/**
 * @file objectrenderer.cpp
 * @brief Implements ObjectRenderer and derived classes
 */

#include "objectrenderer.h"
#include "view.h"

using namespace std;

namespace IPDF
{

/**
 * ObjectRenderer constructor
 * Note the ShaderProgram constructor which compiles the shaders for GPU rendering (if they exist)
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
void ObjectRenderer::RenderUsingGPU()
{
	if (!m_shader_program.Valid())
		Warn("Shader is invalid (objects are of type %d)", m_type);
	m_shader_program.Use();
	m_ibo.Bind();
	glDrawElements(GL_LINES, m_indexes.size()*2, GL_UNSIGNED_INT, 0);
}

/**
 * Helper structuretransforms coordinates to pixels
 */

ObjectRenderer::CPURenderBounds::CPURenderBounds(const Rect & bounds, const View & view, const CPURenderTarget & target)
{
	Rect view_bounds = view.TransformToViewCoords(bounds);
	x = view_bounds.x * target.w;
	y = view_bounds.y * target.h;
	w = view_bounds.w * target.w;
	h = view_bounds.h * target.h;
}

/**
 * Default implementation for rendering using CPU
 */
void ObjectRenderer::RenderUsingCPU(const Objects & objects, const View & view, const CPURenderTarget & target)
{
	Error("Cannot render objects of type %d on CPU", m_type);
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
void RectFilledRenderer::RenderUsingCPU(const Objects & objects, const View & view, const CPURenderTarget & target)
{
	for (unsigned i = 0; i < m_indexes.size(); ++i)
	{
		CPURenderBounds bounds(objects.bounds[m_indexes[i]], view, target);
		for (int x = max(0, bounds.x); x < min(bounds.x+bounds.w, target.w); ++x)
		{
			for (int y = max(0, bounds.y); y < min(bounds.y+bounds.h, target.h); ++y)
			{
				int index = (x+target.w*y)*4;
				target.pixels[index+0] = 0;
				target.pixels[index+1] = 0;
				target.pixels[index+2] = 0;
				target.pixels[index+3] = 255;
			}
		}
	}
}

/**
 * Rectangle (outine)
 */
void RectOutlineRenderer::RenderUsingCPU(const Objects & objects, const View & view, const CPURenderTarget & target)
{
	for (unsigned i = 0; i < m_indexes.size(); ++i)
	{
		CPURenderBounds bounds(objects.bounds[m_indexes[i]], view, target);
		for (int x = max(0, bounds.x); x < min(bounds.x+bounds.w, target.w); ++x)
		{
			int top = (x+target.w*max(0, bounds.y))*4;
			int bottom = (x+target.w*min(bounds.y+bounds.h, target.h))*4;
			for (int j = 0; j < 3; ++j)
			{
				target.pixels[top+j] = 0;
				target.pixels[bottom+j] = 0;
			}
			target.pixels[top+3] = 255;
			target.pixels[bottom+3] = 255;
		}

		for (int y = max(0, bounds.y); y < min(bounds.y+bounds.h, target.h); ++y)
		{
			int left = (max(0, bounds.x)+target.w*y)*4;
			int right = (min(bounds.x+bounds.w, target.w)+target.w*y)*4;
			for (int j = 0; j < 3; ++j)
			{
				target.pixels[left+j] = 0;
				target.pixels[right+j] = 0;
			}
			target.pixels[left+3] = 255;
			target.pixels[right+3] = 255;
			
		}
	}
}

/**
 * Circle (filled)
 */
void CircleFilledRenderer::RenderUsingCPU(const Objects & objects, const View & view, const CPURenderTarget & target)
{
	for (unsigned i = 0; i < m_indexes.size(); ++i)
	{
		CPURenderBounds bounds(objects.bounds[m_indexes[i]], view, target);
		int centre_x = bounds.x + bounds.w / 2;
		int centre_y = bounds.y + bounds.h / 2;
		
		Debug("Centre is %d, %d", centre_x, centre_y);
		Debug("Bounds are %d,%d,%d,%d", bounds.x, bounds.y, bounds.w, bounds.h);
		Debug("Windos is %d,%d", target.w, target.h);
		for (int x = max(0, bounds.x); x < min(bounds.x+bounds.w, target.w); ++x)
		{
			for (int y = max(0, bounds.y); y < min(bounds.y + bounds.h, target.h); ++y)
			{
				double dx = 2.0*(double)(x - centre_x)/(double)(bounds.w);
				double dy = 2.0*(double)(y - centre_y)/(double)(bounds.h);
				int index = (x+target.w*y)*4;
				
				if (dx*dx + dy*dy <= 1.0)
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

}
