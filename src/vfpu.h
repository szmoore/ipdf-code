/**
 * @file vfpu.h
 * @brief Implements a terrible and hacky interface to use a virtual FPU to do floating point operations
 */

#ifndef _VFPU_H
#define _VFPU_H

#include <bitset>
namespace VFPU
{
	extern int Start(const char * vcd_output = NULL); // Starts the VFPU
	extern int Halt(); // Halts the VFPU
	typedef enum {ADD=0x000, SUB=0x001, MULT=0x010, DIV=0x011, SQRT=0x100} Opcode;
	typedef enum {EVEN=0x00, ZERO=0x01, UP=0x10, DOWN=0x11} Rmode; // Rounding mode; to even, towards zero, always up, always down
	typedef std::bitset<32> Register;
	extern Register Exec(const Register & a, const Register & b, Opcode op, Rmode rmode = EVEN); // operate with registers
	extern float Exec(float a, float b, Opcode op, Rmode rmode = EVEN); //converts floats into registers and back
	
	class Float
	{
		public:
			Float(float f = 0) : m_value(f) 
			{
				static bool init = false;
				if (!init)
				{
					init = true;
					VFPU::Start();
				}
			}
			Float(const Float & cpy) : m_value(cpy.m_value) {}
			virtual ~Float() 
			{

			}
			
			Float & operator+=(const Float & op)
			{
				m_value = Exec(m_value, op.m_value, ADD);
				return *this;
			}
			Float & operator-=(const Float & op)
			{
				m_value = Exec(m_value, op.m_value, SUB);
				return *this;
			}
			Float & operator*=(const Float & op)
			{
				m_value = Exec(m_value, op.m_value, MULT);
				return *this;
			}
			Float & operator/=(const Float & op)
			{
				m_value = Exec(m_value, op.m_value, DIV);
				return *this;
			}
			
			Float operator+(const Float & op) const {Float f(*this); f+=op; return f;}
			Float operator-(const Float & op) const {Float f(*this); f-=op; return f;}
			Float operator*(const Float & op) const {Float f(*this); f*=op; return f;}
			Float operator/(const Float & op) const {Float f(*this); f/=op; return f;}
			
			bool operator==(const Float & op) const
			{
				Float f(op);
				f -= *this;
				return (f.m_value == 0);				
			}
			bool operator!=(const Float & op) const {return !this->operator==(op);}
			bool operator<(const Float & op) const
			{
				Float f(op);
				f -= *this;
				return (f.m_value > 0);
			}
			bool operator<=(const Float & op) const
			{
				Float f(op);
				f -= *this;
				return (f.m_value >= 0);				
			}
			bool operator>(const Float & op) const {return !this->operator<=(op);}
			bool operator>=(const Float & op) const {return !this->operator<(op);}
			
			float m_value;
			
	};
	
	inline Float pow(const Float & a, const Float & b)
	{
		return Float(pow(a.m_value, b.m_value));
	}
}

#endif //_VFPU_H


