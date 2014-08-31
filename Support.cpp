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

#include "Constants.h"

u32 *Support_aCosB;
u32 *Support_aSinB;

#include "Support.h"


//This class contains only support functions.  If you dislike the idea of
//using a class for support functions, tough luck!  I will likely de-class
//and do optimizations only after I am satisfied that the code works.

//int Support::bitCompare (u32 instruction, u32 mask, u32 values) {
//	return !((instruction & mask) ^ values);
//	
//}

//This gets specific bits of an instruction
//assumes 31 is the high bit, 0 the low bit.
//startbit should be greater than or equal to endbit.
/*inline u32 Support::getBits (u32 instruction, int startBit, int endBit) {
	int leftShift = 31 - startBit;

	instruction = instruction << leftShift;	//Wipe of the beginning the instruction
	instruction = instruction >> leftShift;	//Put it back again.
	instruction = instruction >> endBit;		
	return instruction;
}*/

//Same as above, just gets one bit
/*
u32 Support::getBit (u32 instruction, int bit) {
	int leftShift = 31 - bit;

	instruction = instruction << leftShift;	//Wipe off the beginning the instruction
	instruction = instruction >> leftShift;	//Put it back again.
	instruction = instruction >> bit;				
	return instruction;

}*/
