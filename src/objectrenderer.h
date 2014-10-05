/**
 * @file objectrenderer.h
 * @brief Definition of ObjectRenderer class
 */

#ifndef _OBJECT_RENDERER_H
#define _OBJECT_RENDERER_H

#include "ipdf.h"
#include "graphicsbuffer.h"
#include "shaderprogram.h"
#include "bufferbuilder.h"
#include <cstdint>

#define BEZIER_CPU_DECASTELJAU

namespace IPDF
{
	class View;
	/**
 	 * Abstract Base class representing how a particular type of object will be rendered
 	 * Includes GPU rendering and CPU rendering
	 * For GPU rendering, pass GLSL shader source files to constructor in the constructor of a base class
	 *	To leave unimplemented, just pass NULL filename strings
	 * For CPU rendering, implement RenderUsingCPU in the derived class
	 *  To leave unimplemented, just call ObjectRenderer::RenderUsingCPU in the derived class
 	 * The View class uses ObjectRenderer's; see view.h
	 */
	class ObjectRenderer
	{
		public:
			/** Construct the ObjectRenderer **/
			ObjectRenderer(const ObjectType & type, const char * vert_glsl_file="", const char * frag_glsl_file="", const char * geom_glsl_file = "");
			virtual ~ObjectRenderer() {}

			/**
			 * Use the GPU to render the objects - GLSL shader approach
			 * This way is definitely faster, but subject to the GPU's limitations on precision
 			 */
			virtual void RenderUsingGPU(unsigned first_obj_id, unsigned last_obj_id);

			/** 
			 * Use the CPU to render the objects - "make a bitmap and convert it to a texture" approach
			 * This way is definitely slower, but gives us more control over the number representations than a GPU
			 */

			struct CPURenderTarget
			{
				uint8_t * pixels;
				int64_t w;
				int64_t h;
				
				
				
			};
			
			static Colour GetColour(const CPURenderTarget & target, int64_t x, int64_t y)
			{
				int64_t index = 4*(x+y*target.w);
				if (index < 0 || index >= 4*(target.w*target.h))
					return Colour(0,0,0,0);
				return Colour(target.pixels[index+0],target.pixels[index+1],target.pixels[index+2],target.pixels[index+3]);
			}
			
			static void SetColour(const CPURenderTarget & target, int64_t x, int64_t y, const Colour & c)
			{
				int64_t index = 4*(x+y*target.w);
				if (index < 0 || index >= 4*(target.w*target.h))
					return;
				
				target.pixels[index+0] = c.r;
				target.pixels[index+1] = c.g;
				target.pixels[index+2] = c.b;
				target.pixels[index+3] = c.a;
			}
			
			struct PixelBounds
			{
				int64_t x; int64_t y; int64_t w; int64_t h;
				PixelBounds(const Rect & bounds);
			};
			
			typedef std::pair<int64_t, int64_t> PixelPoint;

			static Rect CPURenderBounds(const Rect & bounds, const View & view, const CPURenderTarget & target);
			static PixelPoint CPUPointLocation(const Vec2 & point, const View & view, const CPURenderTarget & target);

			static void SaveBMP(const CPURenderTarget & target, const char * filename);


			virtual void RenderUsingCPU(Objects & objects, const View & view, const CPURenderTarget & target, unsigned first_obj_id, unsigned last_obj_id) = 0;
			
			
			
			const ObjectType m_type; /** Type of objects **/
		protected:
			friend class View; //View is a friendly fellow in the world of IPDF
			void PrepareBuffers(unsigned max_size);
			void FinaliseBuffers();
			void AddObjectToBuffers(unsigned index);			
		
			/** Helper for CPU rendering that will render a line using Bresenham's algorithm. Do not use the transpose argument. **/
			static void RenderLineOnCPU(int64_t x0, int64_t y0, int64_t x1, int64_t y1, const CPURenderTarget & target, const Colour & colour = Colour(0,0,0,1), bool transpose = false);
			
			static void FloodFillOnCPU(int64_t x0, int64_t y0, const PixelBounds & bounds, const CPURenderTarget & target, const Colour & fill, const Colour & stroke=Colour(0,0,0,0));

			ShaderProgram m_shader_program; /** GLSL shaders for GPU **/
			GraphicsBuffer m_ibo; /** Index Buffer Object for GPU rendering **/
			std::vector<unsigned> m_indexes; /** Index vector for CPU rendering **/
			BufferBuilder<uint32_t> * m_buffer_builder; /** A BufferBuilder is temporarily used when preparing the ibo and std::vector **/
	};

	/** Renderer for filled rectangles **/
	class RectFilledRenderer : public ObjectRenderer
	{
		public:
			RectFilledRenderer() : ObjectRenderer(RECT_FILLED, "shaders/rect_vert.glsl", "shaders/rect_frag.glsl","shaders/rect_filled_geom.glsl") {}
			virtual ~RectFilledRenderer() {}
			virtual void RenderUsingCPU(Objects & objects, const View & view, const CPURenderTarget & target, unsigned first_obj_id, unsigned last_obj_id);
	};
	/** Renderer for outlined rectangles **/
	class RectOutlineRenderer : public ObjectRenderer
	{
		public:
			RectOutlineRenderer() : ObjectRenderer(RECT_OUTLINE, "shaders/rect_vert.glsl", "shaders/rect_frag.glsl", "shaders/rect_outline_geom.glsl") {}
			virtual ~RectOutlineRenderer() {}
			virtual void RenderUsingCPU(Objects & objects, const View & view, const CPURenderTarget & target, unsigned first_obj_id, unsigned last_obj_id);
	};
	/** Renderer for filled circles **/
	class CircleFilledRenderer : public ObjectRenderer
	{
		public:
			CircleFilledRenderer() : ObjectRenderer(CIRCLE_FILLED, "shaders/rect_vert.glsl", "shaders/circle_frag.glsl", "shaders/circle_filled_geom.glsl") {}
			virtual ~CircleFilledRenderer() {}
			virtual void RenderUsingCPU(Objects & objects, const View & view, const CPURenderTarget & target, unsigned first_obj_id, unsigned last_obj_id);
	};

	/** Renderer for bezier curves **/
	class BezierRenderer : public ObjectRenderer
	{
		public:
			BezierRenderer() : ObjectRenderer(BEZIER, "shaders/rect_vert.glsl", "shaders/rect_frag.glsl", "shaders/bezier_texbuf_geom.glsl") {}
			virtual ~BezierRenderer() {}
			virtual void RenderUsingGPU(unsigned first_obj_id, unsigned last_obj_id); 
			virtual void RenderUsingCPU(Objects & objects, const View & view, const CPURenderTarget & target, unsigned first_obj_id, unsigned last_obj_id);
			void PrepareBezierGPUBuffer(Objects & objects);
			
			static void RenderBezierOnCPU(const Bezier & relative, const Rect & bounds, const View & view, const CPURenderTarget & target, const Colour & c=Colour(0,0,0,255));
			
		private:
			GraphicsBuffer m_bezier_coeffs;
			GraphicsBuffer m_bezier_ids;
			struct GPUBezierCoeffs
			{
				float x0, y0;
				float x1, y1;
				float x2, y2;
				float x3, y3;
			};

			GLuint m_bezier_buffer_texture;
			GLuint m_bezier_id_buffer_texture;

	};
	
	/** Renderer for filled paths **/
	class PathRenderer : public ObjectRenderer
	{
		public:
			PathRenderer() : ObjectRenderer(PATH, "shaders/rect_vert.glsl", "shaders/rect_frag.glsl", "shaders/bezier_texbuf_geom.glsl") {}
			virtual ~PathRenderer() {}
			virtual void RenderUsingCPU(Objects & objects, const View & view, const CPURenderTarget & target, unsigned first_obj_id, unsigned last_obj_id);
			// do nothing on GPU
			virtual void RenderUsingGPU(unsigned first_obj_id, unsigned last_obj_id) {}

	};

	class FakeRenderer : public ObjectRenderer
	{
		public:
			FakeRenderer() : ObjectRenderer(PATH,NULL,NULL,NULL) {}
			~FakeRenderer() {}
			virtual void RenderUsingCPU(Objects & objects, const View & view, const CPURenderTarget & target, unsigned first_obj_id, unsigned last_obj_id) {}
			virtual void RenderUsingGPU(unsigned first_obj_id, unsigned last_obj_id) {}
	};
	
}

#endif //_OBJECT_RENDERER_H
