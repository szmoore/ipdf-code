/**
 * @file objectrenderer.cpp
 * @brief Implements ObjectRenderer and derived classes
 */

#include "objectrenderer.h"

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
 * Default implementation for rendering using CPU
 */
void ObjectRenderer::RenderUsingCPU()
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

}
