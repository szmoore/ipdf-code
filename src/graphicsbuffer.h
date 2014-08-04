#ifndef _GRAPHICSBUFFER_H
#define _GRAPHICSBUFFER_H

#include "SDL.h"
#include "gl_core44.h"


namespace IPDF
{
	/**
	 * Implementation of an OpenGL buffer, with some extra cleverness.
	 */
	class GraphicsBuffer
	{
	public:
		enum BufferType
		{
			BufferTypeVertex,		// A Vertex Buffer
			BufferTypeIndex,		// An Index Buffer
			BufferTypePixelPack,		// Pixel Pack buffer
			BufferTypePixelUnpack,
			BufferTypeUniform,		// Uniform/Constant buffer
			BufferTypeTexture,		// I was hoping to avoid this one.
			BufferTypeDrawIndirect
		};
		
		enum BufferUsage
		{
			BufferUsageStaticDraw,
			BufferUsageStaticRead,
			BufferUsageStaticCopy,
			BufferUsageDynamicDraw,
			BufferUsageDynamicRead,
			BufferUsageDynamicCopy,
			BufferUsageStreamDraw,
			BufferUsageStreamRead,
			BufferUsageStreamCopy
		};
		
		GraphicsBuffer();
		~GraphicsBuffer();
		
		void SetType(BufferType bufType);
		void SetUsage(BufferUsage bufUsage);
		
		void *Map(bool read, bool write, bool invalidate);
		void *MapRange(int offset, int length, bool read, bool write, bool invalidate);
		
		void UnMap();
		
		void Upload(size_t length, const void *data);
		void UploadRange(size_t length, intptr_t offset, const void *data);

		void Resize(size_t length);
		const size_t GetSize() const { return m_buffer_size; }

		void Invalidate();

		// WARNING: The buffer handle can change for (almost) no reason.
		// If you do _anything_ to the buffer, you'll need to call this
		// again to see if we've recreated it in a vain attempt to outsmart
		// the driver.
		GLuint GetHandle() const { return m_buffer_handle; }
		
		void Bind() const;
	private:
		bool RecreateBuffer(const void *data = NULL);
		GLuint m_buffer_handle;
		BufferType m_buffer_type;
		BufferUsage m_buffer_usage;
		void *m_map_pointer;
		size_t m_buffer_size;
		bool m_invalidated;
		bool m_buffer_shape_dirty;
		bool m_faking_map;
	};

}

#endif // _SCREEN_H
