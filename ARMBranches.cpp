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

#include "ARM.h"
#include "Support.h"
#include "BIOS.h"

#include "ARMBranches.h"



//////////////////////////////////////////////
//ARM_branchFunction
//Returns a pointer to a branching function based on the given instruction
///////////////////////////////////////////////
void* ARM_branchFunction (u32 instruction) {
	int l = 0; int n = 0;

	
	l = SUPPORT_GETBIT24(instruction);	//If it's a branch and link
	n = SUPPORT_GETBIT23(instruction);	//If there's a 1 in the sign bit, it's negative.

	if (l)
		if (n) return ARM_bln;
		else return ARM_bl;
	else
		if (n) return ARM_bn;
		else return ARM_b;
		
		
}

////////////////////////////////////////////////////////
//Branch Instructions
///////////////////////////////////////////////////////

//regular branch
void ARM_b () {
	u32 address = arm.r[15] + ((arm.currentInstruction & 0xFFFFFF) << 2);	//Add the offset
	if ( (address & 0x00FFFFFF) >= gbaMem.memorySize[(address >> 24)]) {
		sprintf(console.tempBuf, "Illegal branch to location 0x%08X\n", address);
		Console_print(console.tempBuf);
		arm.paused = 1;
		return;
	}
	arm.r[15] = address;
	ARM_skipInstruction();							//flush queue.
}

//branch and link
void ARM_bl () {
	arm.r[14] = arm.r[15] -4;
	u32 address = arm.r[15] + ((arm.currentInstruction & 0xFFFFFF) << 2);	//Add the offset
	if ( (address & 0x00FFFFFF) >= gbaMem.memorySize[(address >> 24)]) {
		sprintf(console.tempBuf, "Illegal branch to location 0x%08X\n", address);
		Console_print(console.tempBuf);
		arm.paused = 1;
		return;
	}
	arm.r[15] = address;
	ARM_skipInstruction();							//flush queue.
}

//branch with negative offset
void ARM_bn () {
	register int offset = arm.currentInstruction & 0xFFFFFF;
	offset = offset << 8;	//Shift all the way left
	offset = offset >> 6;	//Shift right (but not all the way) retaining sign.
	u32 address = arm.r[15] + offset;	
	if ( (address & 0x00FFFFFF) >= gbaMem.memorySize[(address >> 24)]) {
		sprintf(console.tempBuf, "Illegal branch to location 0x%08X\n", address);
		Console_print(console.tempBuf);
		arm.paused = 1;
		return;
	}
	arm.r[15] = address;
	ARM_skipInstruction();							//flush queue.
}

//branch with negative offset and link
void ARM_bln () {
	arm.r[14] = arm.r[15] -4;
	register int offset = arm.currentInstruction & 0xFFFFFF;
	offset = offset << 8;	//Shift all the way left
	offset = offset >> 6;	//Shift right (but not all the way) retaining sign.
	u32 address = arm.r[15] + offset;	
	if ( (address & 0x00FFFFFF) >= gbaMem.memorySize[(address >> 24)]) {
		sprintf(console.tempBuf, "Illegal branch to location 0x%08X\n", address);
		Console_print(console.tempBuf);
		arm.paused = 1;
		return;
	}
	arm.r[15] = address;
	ARM_skipInstruction();							//flush queue.
}

//branch and exchange
void ARM_bx () {
	u32 rn = arm.currentInstruction & 0xF;
	register u32 address = arm.r[rn];
	//address &= 0xFFFFFFFE;	//Make the last bit 0
	
	if ( (address & 0x00FFFFFF) >= gbaMem.memorySize[(address >> 24)]) {
		sprintf(console.tempBuf, "Illegal branch to location 0x%08X\n", address);
		Console_print(console.tempBuf);
		arm.paused = 1;
		return;
	}
	arm.r[15] = address & 0xFFFFFFFE;

	//Check to see if it's time to switch to thumb mode.
	if ( address & 0x1) {
		arm.thumbMode = 1;
		arm.CPSR = arm.CPSR | 0x20;
		ARM_skipInstructionThumb();

	} else {
		arm.thumbMode = 0;
		arm.CPSR = arm.CPSR & 0xFFFFFFDF;
		ARM_skipInstruction();							//flush queue.
	}
}

/////////////////////////////////////////////////////
//Software interrupt
//////////////////////////////////////////////////////
void ARM_swi() {
	u32 code = arm.currentInstruction & 0x00FFFFFF;
	//arm.r[14] = arm.r[15] -4;	//Save to link register.
	
	
	//Switch off for the correct instruction.
	BIOS_exec(code);

}