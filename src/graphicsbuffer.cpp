#include "graphicsbuffer.h"
#include "log.h"

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
		SDL_assert(false && "Unknown buffer usage type."); //WTF?
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
	case GraphicsBuffer::BufferTypeTexture:
		return GL_TEXTURE_BUFFER;
	case GraphicsBuffer::BufferTypeDrawIndirect:
		return GL_DRAW_INDIRECT_BUFFER;
	default:
		return GL_COPY_READ_BUFFER;
	}
}

GraphicsBuffer::GraphicsBuffer()
{
	m_invalidated = true;
	m_map_pointer = NULL;
	m_buffer_size = 0;
	m_buffer_shape_dirty = true;
	m_buffer_handle = 0;
	m_buffer_usage = BufferUsageDynamicDraw;
	m_faking_map = false;
	m_name = "Unnamed Buffer";
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

void GraphicsBuffer::SetName(const char *name)
{
	m_name = name;
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
	if (!m_buffer_shape_dirty)
	{
		// Orphan the block of memory we're pointing to.
		Upload(m_buffer_size, NULL);
	}
	// Apparently not supported.
	//glInvalidateBufferData(m_buffer_handle);
}

bool GraphicsBuffer::RecreateBuffer(const void *data)
{
	// If the buffer is not dirty, don't recreate it.
	if (!m_buffer_shape_dirty) return false;
	// If the buffer is mapped, don't recreate it.
	if (!m_faking_map && m_map_pointer) return false;
	// If the buffer has data in it we need, don't recreate it.
	if (!m_invalidated) return false;
	if (m_buffer_handle)
	{
		glDeleteBuffers(1, &m_buffer_handle);
	}
	glGenBuffers(1, &m_buffer_handle);
	glObjectLabel(GL_BUFFER, m_buffer_handle, -1, m_name);
	m_buffer_shape_dirty = false;
	if (m_buffer_size)
		Upload(m_buffer_size, data);
	return true;
}

void* GraphicsBuffer::Map(bool read, bool write, bool invalidate)
{
	GLbitfield access = ((read)?GL_MAP_READ_BIT:0) | ((write)?GL_MAP_WRITE_BIT:0) | ((invalidate)?GL_MAP_INVALIDATE_BUFFER_BIT:0);
	GLenum target = BufferTypeToGLType(m_buffer_type);

	if (invalidate)
	{
		m_invalidated = true;

		// Intel's Mesa driver does not rename the buffer when we map with GL_MAP_INVALIDATE_BUFFER_BIT,
		// resulting in the CPU stalling waiting for rendering from the buffer to complete on the GPU.
		// We manually force the buffer to be renamed here to avoid this.
		m_buffer_shape_dirty = true;
	}

	if (m_map_pointer)
		Warn("Tried to map already mapped buffer!");	


	if (!read && m_buffer_usage == BufferUsage::BufferUsageStaticDraw)
	{
		m_map_pointer = malloc(m_buffer_size);
		m_faking_map = true;
		return m_map_pointer;
	}

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
	
	// If we're not mapped, unmapping is a no-op.
	if (!m_map_pointer)
		return;

	if (m_faking_map)
	{
		Upload(m_buffer_size, m_map_pointer);
		free(m_map_pointer);
		m_map_pointer = NULL;
		m_invalidated = false;
		m_faking_map = false;
		return;
	}
	
	Bind();
	glUnmapBuffer(target);
	m_map_pointer = NULL;
	m_invalidated = false;
}

void GraphicsBuffer::Upload(size_t length, const void* data)
{
	GLenum target = BufferTypeToGLType(m_buffer_type);
	
	GLenum usage = BufferUsageToGLUsage(m_buffer_usage);

	m_invalidated = true;
	m_buffer_size = length;
	if (!RecreateBuffer(data))
	{
		Bind();
		glBufferData(target, length+1, data, usage);
	}
	if (data != NULL)
		m_invalidated = false;
}

void GraphicsBuffer::UploadRange(size_t length, intptr_t offset, const void* data)
{
	GLenum target = BufferTypeToGLType(m_buffer_type);

	RecreateBuffer();
	
	Bind();
	glBufferSubData(target, offset, length, data);
	m_invalidated = false;
}

void GraphicsBuffer::Resize(size_t length)
{
	if (m_invalidated && m_buffer_size >= length)
	{
		m_buffer_size = length;
	}
	else
	{
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Resizing buffer.");
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH, 0, NULL, GL_TRUE);
		// Create a new buffer and copy the old data into it.
		UnMap();
		GLuint old_buffer = m_buffer_handle;	
		glGenBuffers(1, &m_buffer_handle);
		Upload(length, NULL);
		glBindBuffer(GL_COPY_READ_BUFFER, old_buffer);
		glBindBuffer(GL_COPY_WRITE_BUFFER, m_buffer_handle);
		glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, m_buffer_size);
		glDeleteBuffers(1, &old_buffer);
		m_buffer_size = length;
		glPopDebugGroup();
	}
}

void GraphicsBuffer::Bind() const
{
	if (m_buffer_type == BufferTypeUniform)
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_buffer_handle);
	else
		glBindBuffer(BufferTypeToGLType(m_buffer_type), m_buffer_handle);
}

void GraphicsBuffer::BindRange(size_t start, size_t size) const
{
	glBindBufferRange(BufferTypeToGLType(m_buffer_type), 0, m_buffer_handle, start, size);
}
