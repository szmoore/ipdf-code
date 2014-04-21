#include "graphicsbuffer.h"
#define GL_GLEXT_PROTOTYPES
#include <SDL_opengl.h>
#include <GL/glext.h>

using namespace IPDF;

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
	GLenum target = (m_buffer_type == GraphicsBuffer::BufferTypeVertex)?GL_ARRAY_BUFFER:GL_ELEMENT_ARRAY_BUFFER;
	
	Bind();
	
	return glMapBufferRange(target, 0, m_buffer_size, access);
	
	//TODO: Emulate DSA
	//return glMapNamedBufferEXT(m_bufferHandle, access);
}

void* GraphicsBuffer::MapRange(int offset, int length, bool read, bool write, bool invalidate)
{
	GLbitfield access = ((read)?GL_MAP_READ_BIT:0) | ((write)?GL_MAP_WRITE_BIT:0) | ((invalidate)?GL_MAP_INVALIDATE_RANGE_BIT:0);
	GLenum target = (m_buffer_type == GraphicsBuffer::BufferTypeVertex)?GL_ARRAY_BUFFER:GL_ELEMENT_ARRAY_BUFFER;
	
	Bind();
	
	return glMapBufferRange(target, offset, length, access);

	//TODO: Emulate DSA
	//return glMapNamedBufferRangeEXT(m_bufferHandle, offset, length, access);
}

void GraphicsBuffer::UnMap()
{
	GLenum target = (m_buffer_type == GraphicsBuffer::BufferTypeVertex)?GL_ARRAY_BUFFER:GL_ELEMENT_ARRAY_BUFFER;
	
	Bind();
	glUnmapBuffer(target);
	//glUnmapNamedBufferEXT(m_bufferHandle);
}

void GraphicsBuffer::Upload(size_t length, const void* data)
{
	GLenum target = (m_buffer_type == GraphicsBuffer::BufferTypeVertex)?GL_ARRAY_BUFFER:GL_ELEMENT_ARRAY_BUFFER;
	
	GLenum usage;
	switch (m_buffer_usage)
	{
	case BufferUsageStaticDraw:
		usage = GL_STATIC_DRAW;
		break;
	case BufferUsageStaticRead:
		usage = GL_STATIC_READ;
		break;
	case BufferUsageStaticCopy:
		usage = GL_STATIC_COPY;
		break;
	case BufferUsageDynamicDraw:
		usage = GL_DYNAMIC_DRAW;
		break;
	case BufferUsageDynamicRead:
		usage = GL_DYNAMIC_READ;
		break;
	case BufferUsageDynamicCopy:
		usage = GL_DYNAMIC_COPY;
		break;
	case BufferUsageStreamDraw:
		usage = GL_STREAM_DRAW;
		break;
	case BufferUsageStreamRead:
		usage = GL_STREAM_READ;
		break;
	case BufferUsageStreamCopy:
		usage = GL_STREAM_COPY;
		break;
	default:
		SDL_assert(false && "Unknown buffer usage type.");
		usage = GL_DYNAMIC_DRAW;
	}
	
	Bind();
	glBufferData(target, length, data, usage);
	m_buffer_size = length;
	//glNamedBufferDataEXT(m_bufferHandle, length, data, usage);
}

void GraphicsBuffer::UploadRange(size_t length, intptr_t offset, const void* data)
{
	GLenum target = (m_buffer_type == GraphicsBuffer::BufferTypeVertex)?GL_ARRAY_BUFFER:GL_ELEMENT_ARRAY_BUFFER;
	
	Bind();
	glBufferSubData(target, offset, length, data);
	//glNamedBufferSubDataEXT(m_bufferHandle, offset, length, data);
}

void GraphicsBuffer::Bind()
{
	if (m_buffer_type == BufferTypeVertex)
		glBindBuffer(GL_ARRAY_BUFFER, m_buffer_handle);
	else if (m_buffer_type == BufferTypeIndex)
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffer_handle);
	else if (m_buffer_type == BufferTypePixelPack)
		glBindBuffer(GL_PIXEL_PACK_BUFFER, m_buffer_handle);
	else if (m_buffer_type == BufferTypePixelUnpack)
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_buffer_handle);
	else if (m_buffer_type == BufferTypeUniform)
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_buffer_handle);
	else if (m_buffer_type == BufferTypeDrawIndirect)
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_buffer_handle);
}

