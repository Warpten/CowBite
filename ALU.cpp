/*
    CowBite GBA Emulator/Debugger
    Copyright (C) 2002 Thomas Happ

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    CowBite contact info:
    Thomas Happ
    SorcererXIII@yahoo.com
    http://cowbite.emuunlim.com
*/

#include <stdlib.h>	//supposedly contains _rotr

#include "ALU.h"
#include "ARM.h"
#include "Support.h"


//calculate -- use to perform ALU operations
//u32 ARM::ALU_calculate (u32* destReg, u32 a, u32 b, u32 opcode, int S) {
u32 ALU_calculate(int S) {
	register u32 a = *(arm.r+arm.rn);
	register u32 b = arm.op2;
	register u32* destReg = arm.r+arm.rd;

	int result = 1;	//Initialize result to something non-negative, non zero;
	
	switch (arm.opcode) {
		case OP_AND:	//Boolean And
						result = a & b;
						*destReg = result;
						break;

		case OP_EOR:	result = a ^ b;
						*destReg = result;
						break;

		case OP_SUB:	result = a - b;
						*destReg = result;
						//ALU_setSubFlags (a, b, result);
						ALU_SETSUBFLAGS(a, b, result);
						break;

		case OP_RSB:	result = b - a;
						*destReg = result;
						//ALU_setSubFlags (b, a, result);
						ALU_SETSUBFLAGS(a, b, result);
						break;

		case OP_ADD:	result = a + b;
						*destReg = result;
						//ALU_setAddFlags (a, b, result);
						ALU_SETADDFLAGS(a, b, result);
						break;

		//For these we want to get the carry bit of the arm.CPSR
		case OP_ADC:	result = a + b + SUPPORT_GETBIT29(arm.CPSR);
						*destReg = result;
						//ALU_setAddFlags (a, b, result);
						ALU_SETADDFLAGS(a, b, result);
						break;
		
		case OP_SBC:	result = a - b - !SUPPORT_GETBIT29(arm.CPSR);
						*destReg = result;
						//ALU_setSubFlags (a, b, result);
						ALU_SETSUBFLAGS(a, b, result);
						break;

		case OP_RSC:	result = b - a - !SUPPORT_GETBIT29(arm.CPSR);
						*destReg = result;
						//ALU_setSubFlags (b, a, result);
						ALU_SETSUBFLAGS(a, b, result);
						break;
		//ToDO:  Only execute these if S is set, otherwise execute MSR.
		//For these we do not want to modify the result register
		case OP_TST:	result = a & b;
						break;

		case OP_TEQ:	result = a ^ b;
						break;

		case OP_CMP:	result = a - b;
						//ALU_setSubFlags (a, b, result);
						ALU_SETSUBFLAGS(a, b, result);
						break;

		case OP_CMN:	result = a + b;
						//ALU_setAddFlags (a, b, result);
						ALU_SETADDFLAGS(a, b, result);
						break;
		
		
		case OP_ORR:	result = a | b;
						*destReg = result;
						break;

		case OP_MOV:	result = b;
						*destReg = result;
						break;

		case OP_BIC:	result = a & (~b);
						break;

		case OP_MVN:	result = ~b;
						*destReg = result;
						break;
		default:
						break;
	}
	

	//If the S bit was set, set the appropriate arm.CPSR flags
	if (S) ALU_setNZFlags (result);
	
	return result;
}//ALU_Calculate

