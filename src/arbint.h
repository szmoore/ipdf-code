#ifndef _ARBINT_H
#define _ARBINT_H

#include <vector>

namespace IPDF
{
	class Arbint
	{
		public:
			Arbint(int64_t i);
			~Arbint() {}
			Arbint(const Arbint & cpy);

			std::string Str() const;
		
		private:		
			std::vector<uint32_t> m_words;
			bool m_sign;
			
			
			
	};	
}

#endif //_ARBINT_H
