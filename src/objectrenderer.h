/**
 * @file objectrenderer.h
 * @brief Definition of ObjectRenderer class
 */

#include "ipdf.h"
#include "graphicsbuffer.h"
#include "shaderprogram.h"
#include "bufferbuilder.h"

namespace IPDF
{
	/**
 	 * Abstract Base class representing how a particular type of object will be rendered
 	 * Includes GPU rendering and CPU rendering
	 * For GPU rendering, pass GLSL shader source files to constructor in the constructor of a base class
	 *	To leave unimplemented, just pass NULL filename strings
	 * For CPU rendering, implement RenderUsingCPU in the base class
	 *  To leave unimplemented, just call ObjectRenderer::RenderUsingCPU in the base class
 	 * The View class uses ObjectRenderer's; see view.h
	 */
	class ObjectRenderer
	{
		public:
			/** Construct the ObjectRenderer **/
			ObjectRenderer(const ObjectType & type, const char * vert_glsl_file, const char * frag_glsl_file, const char * geom_glsl_file = "");
			virtual ~ObjectRenderer() {}

			/**
			 * Use the GPU to render the objects - GLSL shader approach
			 * This way is definitely faster, but subject to the GPU's limitations on precision
 			 */
			void RenderUsingGPU();

			/** 
			 * Use the CPU to render the objects - "make a bitmap and convert it to a texture" approach
			 * This way is definitely slower, but gives us more control over the number representations than a GPU
			 */
			virtual void RenderUsingCPU() = 0;
			
			
			
			const ObjectType m_type; /** Type of objects **/
		private:
			friend class View; //View is a friendly fellow in the world of IPDF
			void PrepareBuffers(unsigned max_size);
			void FinaliseBuffers();
			void AddObjectToBuffers(unsigned index);			


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
			virtual void RenderUsingCPU() {ObjectRenderer::RenderUsingCPU();} //TODO: Implement
	};
	/** Renderer for outlined rectangles **/
	class RectOutlineRenderer : public ObjectRenderer
	{
		public:
			RectOutlineRenderer() : ObjectRenderer(RECT_OUTLINE, "shaders/rect_vert.glsl", "shaders/rect_frag.glsl", "shaders/rect_outline_geom.glsl") {}
			virtual ~RectOutlineRenderer() {}
			virtual void RenderUsingCPU() {ObjectRenderer::RenderUsingCPU();} //TODO: Implement
	};
	/** Renderer for filled circles **/
	class CircleFilledRenderer : public ObjectRenderer
	{
		public:
			CircleFilledRenderer() : ObjectRenderer(CIRCLE_FILLED, "shaders/rect_vert.glsl", "shaders/circle_frag.glsl", "shaders/circle_filled_geom.glsl") {}
			virtual ~CircleFilledRenderer() {}
			virtual void RenderUsingCPU() {ObjectRenderer::RenderUsingCPU();} //TODO: Implement
	};
}
