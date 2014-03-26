#ifndef _DOCUMENT_H
#define _DOCUMENT_H

#include "ipdf.h"

namespace IPDF
{
	class Document
	{
		public:
			Document(const std::string & filename = "") : m_objects(), m_count(0) {Load(filename);}
			virtual ~Document() {}

			void Load(const std::string & filename = "");
			void Save(const std::string & filename);
			void Add(Real x, Real y, Real w, Real h);
			void DebugDumpObjects();

			unsigned ObjectCount() const {return m_count;}

			bool operator==(const Document & equ) const;
			bool operator!=(const Document & equ) const {return !(this->operator==(equ));}

		private:
			friend class View;
			Objects m_objects;
			unsigned m_count;
	};
}

#endif //_DOCUMENT_H
