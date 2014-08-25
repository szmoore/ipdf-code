#ifndef _GROUP_H
#define _GROUP_H

#include <vector>
#include <algorithm>

namespace IPDF
{
	
	struct Colour
	{
		float r; float g; float b; float a;
		Colour() = default;
		Colour(float _r, float _g, float _b, float _a) : r(_r), g(_g), b(_b), a(_a) {}
	};
	
	class Objects;
	
	struct Group
	{
		Group(unsigned _start, unsigned _end, unsigned _index, const Colour & _fill = Colour(0.8,0.8,0.8,1))
			: m_start(_start), m_end(_end), m_index(_index), m_fill(_fill)
		{
			
		}
		unsigned m_start;
		unsigned m_end;
		unsigned m_index;
		Colour m_fill;		
	};

}
#endif //_GROUP_H
