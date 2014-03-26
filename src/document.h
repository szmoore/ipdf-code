#ifndef _DOCUMENT_H
#define _DOCUMENT_H

#include "ipdf.h"

namespace IPDF
{
	class Document
	{
		public:
			Document() : m_objects(), m_count(0) {Load();}
			virtual ~Document() {}

			void Load(const std::string & filename = "");
			void Add(Real x, Real y, Real w, Real h);

			unsigned ObjectCount() {return m_count;}

		private:
			friend class View;
			Objects m_objects;
			unsigned m_count;
	};
}

#endif //_DOCUMENT_H
