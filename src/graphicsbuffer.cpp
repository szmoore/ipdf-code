#include "graphicsbuffer.h"
#define GL_GLEXT_PROTOTYPES
#include <SDL_opengl.h>
#include <GL/glext.h>

using namespace IPDF;

static GLenum BufferUsageToGLUsage(GraphicsBuffer::BufferUsage buffer_usage)
{
	GLenum usage;
	switch (buffer_usage)
	{
	case GraphicsBuffer::BufferUsageStaticDraw:
		usage = GL_STATIC_DRAW;
		break;
	case GraphicsBuffer::BufferUsageStaticRead:
		usage = GL_STATIC_READ;
		break;
	case GraphicsBuffer::BufferUsageStaticCopy:
		usage = GL_STATIC_COPY;
		break;
	case GraphicsBuffer::BufferUsageDynamicDraw:
		usage = GL_DYNAMIC_DRAW;
		break;
	case GraphicsBuffer::BufferUsageDynamicRead:
		usage = GL_DYNAMIC_READ;
		break;
	case GraphicsBuffer::BufferUsageDynamicCopy:
		usage = GL_DYNAMIC_COPY;
		break;
	case GraphicsBuffer::BufferUsageStreamDraw:
		usage = GL_STREAM_DRAW;
		break;
	case GraphicsBuffer::BufferUsageStreamRead:
		usage = GL_STREAM_READ;
		break;
	case GraphicsBuffer::BufferUsageStreamCopy:
		usage = GL_STREAM_COPY;
		break;
	default:
		SDL_assert(false && "Unknown buffer usage type.");
		usage = GL_DYNAMIC_DRAW;
	}
	return usage;
}

static GLenum BufferTypeToGLType(GraphicsBuffer::BufferType buffer_type)
{
	switch (buffer_type)
	{
	case GraphicsBuffer::BufferTypeVertex:
		return GL_ARRAY_BUFFER;
	case GraphicsBuffer::BufferTypeIndex:
		return GL_ELEMENT_ARRAY_BUFFER;
	case GraphicsBuffer::BufferTypePixelPack:
		return GL_PIXEL_PACK_BUFFER;
	case GraphicsBuffer::BufferTypePixelUnpack:
		return GL_PIXEL_UNPACK_BUFFER;
	case GraphicsBuffer::BufferTypeUniform:
		return GL_UNIFORM_BUFFER;
	case GraphicsBuffer::BufferTypeDrawIndirect:
		return GL_DRAW_INDIRECT_BUFFER;
	default:
		return GL_COPY_READ_BUFFER;
	}
}

GraphicsBuffer::GraphicsBuffer()
{
	SetUsage(BufferUsageStaticDraw);
}

GraphicsBuffer::~GraphicsBuffer()
{
	if (m_map_pointer)
	{
		UnMap();
	}
	glDeleteBuffers(1, &m_buffer_handle);
}

void GraphicsBuffer::SetType(GraphicsBuffer::BufferType bufType)
{
	glGenBuffers(1, &m_buffer_handle);
	m_buffer_type = bufType;
}

void GraphicsBuffer::SetUsage(GraphicsBuffer::BufferUsage bufUsage)
{
	m_buffer_usage = bufUsage;
}

void* GraphicsBuffer::Map(bool read, bool write, bool invalidate)
{
	GLbitfield access = ((read)?GL_MAP_READ_BIT:0) | ((write)?GL_MAP_WRITE_BIT:0) | ((invalidate)?GL_MAP_INVALIDATE_BUFFER_BIT:0);
	GLenum target = BufferTypeToGLType(m_buffer_type);

	Bind();
	
	return glMapBufferRange(target, 0, m_buffer_size, access);
	
	//TODO: Emulate DSA
	//return glMapNamedBufferEXT(m_bufferHandle, access);
}

void* GraphicsBuffer::MapRange(int offset, int length, bool read, bool write, bool invalidate)
{
	GLbitfield access = ((read)?GL_MAP_READ_BIT:0) | ((write)?GL_MAP_WRITE_BIT:0) | ((invalidate)?GL_MAP_INVALIDATE_RANGE_BIT:0);
	GLenum target = BufferTypeToGLType(m_buffer_type);
	
	Bind();
	
	return glMapBufferRange(target, offset, length, access);

	//TODO: Emulate DSA
	//return glMapNamedBufferRangeEXT(m_bufferHandle, offset, length, access);
}

void GraphicsBuffer::UnMap()
{
	GLenum target = BufferTypeToGLType(m_buffer_type);
	
	Bind();
	glUnmapBuffer(target);
	//glUnmapNamedBufferEXT(m_bufferHandle);
}

void GraphicsBuffer::Upload(size_t length, const void* data)
{
	GLenum target = BufferTypeToGLType(m_buffer_type);
	
	GLenum usage = BufferUsageToGLUsage(m_buffer_usage);
	
	Bind();
	glBufferData(target, length, data, usage);
	m_buffer_size = length;
	//glNamedBufferDataEXT(m_bufferHandle, length, data, usage);
}

void GraphicsBuffer::UploadRange(size_t length, intptr_t offset, const void* data)
{
	GLenum target = BufferTypeToGLType(m_buffer_type);
	
	Bind();
	glBufferSubData(target, offset, length, data);
	//glNamedBufferSubDataEXT(m_bufferHandle, offset, length, data);
}

void GraphicsBuffer::Resize(size_t length)
{
	if (m_invalidated)
	{
		Upload(length, nullptr);
	}
	else
	{
		// Create a new buffer and copy the old data into it.
		UnMap();
		GLuint old_buffer = m_buffer_handle;	
		glGenBuffers(1, &m_buffer_handle);
		Upload(length, nullptr);
		glBindBuffer(GL_COPY_READ_BUFFER, old_buffer);
		glBindBuffer(GL_COPY_WRITE_BUFFER, m_buffer_handle);
		glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, m_buffer_size);
		glDeleteBuffers(1, &old_buffer);
	}

}

void GraphicsBuffer::Bind()
{
	if (m_buffer_type == BufferTypeUniform)
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_buffer_handle);
	else
		glBindBuffer(BufferTypeToGLType(m_buffer_type), m_buffer_handle);
}

