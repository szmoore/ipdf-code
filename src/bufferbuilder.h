#ifndef _BUFFERBUILDER_H
#define _BUFFERBUILDER_H

namespace IPDF
{
	/*
	 * Utility to add stuff to a buffer. 
	 */
	template <typename T>
	class BufferBuilder
	{
	public:
		BufferBuilder(void *data, size_t size) : m_bufferData((T*)data), m_bufferSize(size), m_bufferOffset(0) {};
		// Append an item to the buffer, returning its index.
		size_t Add(const T& item) {m_bufferData[m_bufferOffset] = item; m_bufferOffset++; return m_bufferOffset-1;}
		bool Free(size_t num = 1) const {return ((m_bufferOffset + num) * sizeof(T)) < m_bufferSize;}
	private:
		T *m_bufferData;
		size_t m_bufferSize; // In bytes, 'cause why make things easy?
		size_t m_bufferOffset; // In elements, 'cause why make things consistant?
	};

}

#endif // _SCREEN_H
