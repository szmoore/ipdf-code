#ifndef _GRAPHICSBUFFER_H
#define _GRAPHICSBUFFER_H

#include <SDL.h>
#define GL_GLEXT_PROTOTYPES
#include <SDL_opengl.h>


namespace IPDF
{
	/*
	 * The "Screen" class handles managing the OS window (using SDL2).
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
			BufferTypeDrawIndirect,
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
		
		void Bind();
	private:
		GLuint m_buffer_handle;
		BufferType m_buffer_type;
		BufferUsage m_buffer_usage;
		void *m_map_pointer;
		size_t m_buffer_size;
		bool m_invalidated;
	};

}

#endif // _SCREEN_H
