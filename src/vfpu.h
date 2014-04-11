#ifndef _VFPU_H
#define _VFPU_H

/**
 * Implements a terrible and hacky interface to use a virtual FPU to do floating point operations
 */

#include <bitset>

namespace VFPU
{
	extern int Start(const char * vcd_output = NULL); // Starts the VFPU
	extern int Halt(); // Halts the VFPU

/**
		-- 000 = add, 
		-- 001 = substract, 
		-- 010 = multiply, 
		-- 011 = divide,
		-- 100 = square root
		-- 101 = unused
		-- 110 = unused
		-- 111 = unused
 */
	typedef enum {ADD=0x000, SUB=0x001, MULT=0x010, DIV=0x011, SQRT=0x100} Opcode;
	typedef enum {EVEN=0x00, ZERO=0x01, UP=0x10, DOWN=0x11} Rmode;
	typedef std::bitset<32> Register;
	
	extern Register Exec(const Register & a, const Register & b, Opcode op, Rmode rmode = EVEN); // operate with registers
	extern float Exec(float a, float b, Opcode op, Rmode rmode = EVEN); //converts floats into registers and back
}

#endif //_VFPU_H


