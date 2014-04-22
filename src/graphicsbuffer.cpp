#include "graphicsbuffer.h"
#include "log.h"
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
	m_invalidated = true;
	m_map_pointer = nullptr;
	m_buffer_size = 0;
	m_buffer_shape_dirty = true;
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
	m_buffer_type = bufType;
}

void GraphicsBuffer::SetUsage(GraphicsBuffer::BufferUsage bufUsage)
{
	if (bufUsage != m_buffer_usage)
	{
		m_buffer_usage = bufUsage;
		m_buffer_shape_dirty = true;
	}
}

void GraphicsBuffer::Invalidate()
{
	m_invalidated = true;
	// Apparently not supported.
	//glInvalidateBufferData(m_buffer_handle);
}

void GraphicsBuffer::RecreateBuffer()
{
	// If the buffer is not dirty, don't recreate it.
	if (!m_buffer_shape_dirty) return;
	// If the buffer is mapped, don't recreate it.
	if (m_map_pointer) return;
	// If the buffer has data in it we need, don't recreate it.
	if (!m_invalidated) return;
	if (m_buffer_handle)
	{
		glDeleteBuffers(1, &m_buffer_handle);
	}
	glGenBuffers(1, &m_buffer_handle);
	m_buffer_shape_dirty = false;
	if (m_buffer_size)
		Upload(m_buffer_size, nullptr);
}

void* GraphicsBuffer::Map(bool read, bool write, bool invalidate)
{
	GLbitfield access = ((read)?GL_MAP_READ_BIT:0) | ((write)?GL_MAP_WRITE_BIT:0) | ((invalidate)?GL_MAP_INVALIDATE_BUFFER_BIT:0);
	GLenum target = BufferTypeToGLType(m_buffer_type);

	if (invalidate)
	       m_invalidated = true;

	if (m_map_pointer)
		Warn("Tried to map already mapped buffer!");	

	RecreateBuffer();

	Bind();
	
	m_map_pointer = glMapBufferRange(target, 0, m_buffer_size, access);
	
	return m_map_pointer;
}

void* GraphicsBuffer::MapRange(int offset, int length, bool read, bool write, bool invalidate)
{
	GLbitfield access = ((read)?GL_MAP_READ_BIT:0) | ((write)?GL_MAP_WRITE_BIT:0) | ((invalidate)?GL_MAP_INVALIDATE_RANGE_BIT:0);
	GLenum target = BufferTypeToGLType(m_buffer_type);

	if (m_map_pointer)
		Warn("Tried to map already mapped buffer!");	

	RecreateBuffer();

	Bind();
	
	m_map_pointer = glMapBufferRange(target, offset, length, access);
	return m_map_pointer;
}

void GraphicsBuffer::UnMap()
{
	GLenum target = BufferTypeToGLType(m_buffer_type);
	
	Bind();
	glUnmapBuffer(target);
	m_map_pointer = nullptr;
}

void GraphicsBuffer::Upload(size_t length, const void* data)
{
	GLenum target = BufferTypeToGLType(m_buffer_type);
	
	GLenum usage = BufferUsageToGLUsage(m_buffer_usage);

	m_invalidated = true;
	RecreateBuffer();
	
	Bind();
	glBufferData(target, length, data, usage);
	m_buffer_size = length;
}

void GraphicsBuffer::UploadRange(size_t length, intptr_t offset, const void* data)
{
	GLenum target = BufferTypeToGLType(m_buffer_type);

	RecreateBuffer();
	
	Bind();
	glBufferSubData(target, offset, length, data);
}

void GraphicsBuffer::Resize(size_t length)
{
	if (m_invalidated)
	{
		m_buffer_size = length;
		RecreateBuffer();	
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
		m_buffer_size = length;
	}
}

void GraphicsBuffer::Bind()
{
	if (m_buffer_type == BufferTypeUniform)
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_buffer_handle);
	else
		glBindBuffer(BufferTypeToGLType(m_buffer_type), m_buffer_handle);
}

