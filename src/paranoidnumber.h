#ifndef _PARANOIDNUMBER_H
#define _PARANOIDNUMBER_H

#include <list>
#include <cfloat>
#include <map>
#include <string>
#include "log.h"



namespace IPDF
{
	class ParanoidNumber
	{
		public:
			typedef enum {ADD, SUBTRACT, MULTIPLY, DIVIDE} Optype;
			
			ParanoidNumber(float value=0, Optype type = ADD) : m_value(value), m_op(type), m_next_term(NULL), m_next_factor(NULL)
			{
				
			}
			
			ParanoidNumber(const ParanoidNumber & cpy) : m_value(cpy.m_value), m_op(cpy.m_op), m_next_term(NULL), m_next_factor(NULL)
			{
				if (cpy.m_next_term != NULL)
				{
					m_next_term = new ParanoidNumber(*(cpy.m_next_term));
				}
				if (cpy.m_next_factor != NULL)
				{
					m_next_factor = new ParanoidNumber(*(cpy.m_next_factor));
				}
			}
			
			ParanoidNumber(const ParanoidNumber & cpy, Optype type) : m_value(cpy.m_value), m_op(type), m_next_term(NULL), m_next_factor(NULL)
			{
				if (cpy.m_next_term != NULL)
				{
					m_next_term = new ParanoidNumber(*(cpy.m_next_term));
				}
				if (cpy.m_next_factor != NULL)
				{
					m_next_factor = new ParanoidNumber(*(cpy.m_next_factor));
				}
			}
			
			ParanoidNumber(const char * str);
			ParanoidNumber(const std::string & str) : ParanoidNumber(str.c_str()) {}
			
			virtual ~ParanoidNumber()
			{
				if (m_next_term != NULL)
					delete m_next_term;
				if (m_next_factor != NULL)
					delete m_next_factor;
			}
			
			template <class T> T Convert() const;
			double ToDouble() const {return Convert<double>();}
			float ToFloat() const {return Convert<float>();}
			
			bool Floating() const {return (m_next_term == NULL && m_next_factor == NULL);}
			bool Sunken() const {return !Floating();} // I could not resist...
			
			ParanoidNumber & operator+=(const ParanoidNumber & a);
			ParanoidNumber & operator-=(const ParanoidNumber & a);
			ParanoidNumber & operator*=(const ParanoidNumber & a);
			ParanoidNumber & operator/=(const ParanoidNumber & a);
			ParanoidNumber & operator=(const ParanoidNumber & a);
			
			
			bool operator<(const ParanoidNumber & a) const {return ToDouble() < a.ToDouble();}
			bool operator<=(const ParanoidNumber & a) const {return this->operator<(a) || this->operator==(a);}
			bool operator>(const ParanoidNumber & a) const {return !(this->operator<=(a));}
			bool operator>=(const ParanoidNumber & a) const {return !(this->operator<(a));}
			bool operator==(const ParanoidNumber & a) const {return ToDouble() == a.ToDouble();}
			bool operator!=(const ParanoidNumber & a) const {return !(this->operator==(a));}
			
			ParanoidNumber operator+(const ParanoidNumber & a) const
			{
				ParanoidNumber result(*this);
				result += a;
				return result;
			}
			ParanoidNumber operator-(const ParanoidNumber & a) const
			{
				ParanoidNumber result(*this);
				result -= a;
				return result;
			}
			ParanoidNumber operator*(const ParanoidNumber & a) const
			{
				ParanoidNumber result(*this);
				result *= a;
				return result;
			}
			ParanoidNumber operator/(const ParanoidNumber & a) const
			{
				ParanoidNumber result(*this);
				result /= a;
				return result;
			}
			
			std::string Str() const;
			static char OpChar(Optype op) 
			{
				static char opch[] = {'+','-','*','/'};
				return opch[(int)op];
			}
		
		private:
			void Simplify();
			void SimplifyTerms();
			void SimplifyFactors();
			
			
			float m_value;
			Optype m_op;
			ParanoidNumber * m_next_term;
			ParanoidNumber * m_next_factor;
			
			

			
			

	};


template <class T>
T ParanoidNumber::Convert() const
{
	T value = (m_op == SUBTRACT) ? -m_value : m_value;
	const ParanoidNumber * n = m_next_factor;
	if (n != NULL)
	{
		switch (n->m_op)
		{
			case MULTIPLY:
				value *= n->Convert<T>();
				break;
			case DIVIDE:
				value /= n->Convert<T>();
				break;
			default:
				Fatal("Shouldn't happen");
				break;
		}
	}
	n = m_next_term;
	if (n != NULL)
	{
		switch (n->m_op)
		{
			case ADD:
			case SUBTRACT:
				value += n->Convert<T>();
				break;
			default:
				Fatal("Shouldn't happen");
		}
	}
	return value;
}


}

#endif //_PARANOIDNUMBER_H

