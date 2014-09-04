#ifndef _PARANOIDNUMBER_H
#define _PARANOIDNUMBER_H

#include <list>
#include <cfloat>
#include <map>
#include <string>

namespace IPDF
{
	class ParanoidNumber
	{
		public:
			typedef enum {ADD, SUBTRACT, MULTIPLY, DIVIDE} Optype;
			
			ParanoidNumber(float value=0, Optype type = ADD) : m_value(value), m_op(type), m_next(NULL)
			{
				
			}
			
			ParanoidNumber(const ParanoidNumber & cpy) : m_value(cpy.m_value), m_op(cpy.m_op), m_next(NULL)
			{
				if (cpy.m_next != NULL)
				{
					m_next = new ParanoidNumber(*(cpy.m_next));
				}
			}
			
			ParanoidNumber(const ParanoidNumber & cpy, Optype type) : ParanoidNumber(cpy)
			{
				m_op = type;
			}
			
			ParanoidNumber(const char * str);
			
			virtual ~ParanoidNumber()
			{
				if (m_next != NULL)
					delete m_next;
			}
			
			
			double ToDouble() const;
			
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
		
		private:
			void Simplify();
			ParanoidNumber * InsertAfter(ParanoidNumber * insert);
			ParanoidNumber * InsertAfter(float value, Optype op);
			
			float m_value;
			Optype m_op;
			ParanoidNumber * m_next;

			
			

	};


}

#endif //_PARANOIDNUMBER_H

