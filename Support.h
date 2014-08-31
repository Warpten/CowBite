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

#ifndef SUPPORT_H
#define SUPPORT_H

#include "Constants.h"



extern u32 *Support_aCosB;
extern u32 *Support_aSinB;

//////////////////////////////////////////////////////////////
//Supprot_init()
//Initializes variables used  for support.
inline void Support_init () {
	//Support_aCosB = new u32 [


}

inline u32 Support_getBits (u32 instruction, int startBit, int endBit) {	
			int leftShift = 31 - startBit;

			instruction = instruction << leftShift;	//Wipe of the beginning the instruction
			instruction = instruction >> leftShift;	//Put it back again.
			instruction = instruction >> endBit;		
			return instruction;
}

//These defines serve as additional "functions"
#define SUPPORT_BITCOMPARE(val1,mask,vals) ( !((val1 & mask) ^ vals) )

#define SUPPORT_GETRN(val) ((val >> 16) & 0xF)
#define SUPPORT_GETRD(val) ((val >> 12) & 0xF)
#define SUPPORT_GETRM(val) (val & 0xF)
#define SUPPORT_GETRC(val) ((val >> 8) & 0xF)
#define SUPPORT_GETRS(val) ((val >> 8) & 0xF)
#define SUPPORT_GETOP(val) ((val >> 21) & 0xF)

#define SUPPORT_GETRDTHUMB(val) (val & 0x7)
#define SUPPORT_GETRSTHUMB(val) ((val >> 3) & 0x7)
#define SUPPORT_GETRBTHUMB(val) ((val >> 3) & 0x7)
#define SUPPORT_GETRNTHUMB(val) ((val >> 6) & 0x7)
#define SUPPORT_GETROTHUMB(val) ((val >> 6) & 0x7)


//These are mostly to help ease the writing of 
//instruction code
#define RN(val) ( arm.r[SUPPORT_GETRN(val)]  )
#define RD(val) ( arm.r[SUPPORT_GETRD(val)]  )
#define RM(val) ( arm.r[SUPPORT_GETRM(val)]  )
#define RC(val) ( arm.r[SUPPORT_GETRC(val)]  )
#define RS(val) ( arm.r[SUPPORT_GETRS(val)]  )
#define IMM(val) ( val & 0x000000FF )
#define SHIFTTYPE(val) ( (val >> 5) & 0x3)

#define RDTHUMB(val) ( arm.r[SUPPORT_GETRDTHUMB(val)] )
#define RSTHUMB(val) ( arm.r[SUPPORT_GETRSTHUMB(val)] )
#define RBTHUMB(val) ( arm.r[SUPPORT_GETRBTHUMB(val)] )
#define RNTHUMB(val) ( arm.r[SUPPORT_GETRNTHUMB(val)] )
#define ROTHUMB(val) ( arm.r[SUPPORT_GETROTHUMB(val)] )

//In both mappy and boycott, it seems that if register rc contains a value greater than 
//1 byte, the return value is automatically 0; but the ARM docs indicate that the shifter
//just ignores the upper bytes, so I am masking them out.  As a result I get somewhat
//different behavior...
//NOTE THAT THESE DO NOT SET THE CARRY BIT
#define REGSHIFT(val) ALU_barrelRegShift ( RM(val), RC(val) & 0x000000FF, SHIFTTYPE(val) )
#define IMMSHIFT(val) ALU_barrelShift ( RM(val), (val >> 7) & 0x1F, SHIFTTYPE(val) )

//NOTE THAT HESE *DO* SET THE CARRY BIT
#define REGSHIFTS(val) ALU_barrelRegShiftS ( RM(val), RC(val) & 0x000000FF, SHIFTTYPE(val) )
#define IMMSHIFTS(val) ALU_barrelShiftS ( RM(val), (val >> 7) & 0x1F, SHIFTTYPE(val) )

#define IMMROT(val,shift)    (( val << (32 - shift)) | ( val >> shift) )

#define B0 0x1
#define B1 0x2
#define B2 0x4
#define B3 0x8
#define B4 0x10
#define B5 0x20
#define B6 0x40
#define B7 0x80
#define B8 0x100
#define B9 0x200
#define B10 0x400
#define B11 0x800
#define B12 0x1000
#define B13 0x2000
#define B14 0x4000
#define B15 0x8000
#define B16 0x10000
#define B17 0x20000
#define B18 0x40000
#define B19 0x80000
#define B20 0x100000
#define B21 0x200000
#define B22 0x400000
#define B23 0x800000
#define B24 0x1000000
#define B25 0x2000000
#define B26 0x4000000
#define B27 0x8000000
#define B28 0x10000000
#define B29 0x20000000
#define B30 0x40000000
#define B31 0x80000000
#define B32 0x100000000

#define SUPPORT_GETBIT0(val) ((val) & 0x1)
#define SUPPORT_GETBIT1(val) ((val) & 0x2)
#define SUPPORT_GETBIT2(val) ((val) & 0x4)
#define SUPPORT_GETBIT3(val) ((val) & 0x8)
#define SUPPORT_GETBIT4(val) ((val) & 0x10)
#define SUPPORT_GETBIT5(val) ((val) & 0x20)
#define SUPPORT_GETBIT6(val) ((val) & 0x40)
#define SUPPORT_GETBIT7(val) ((val) & 0x80)
#define SUPPORT_GETBIT8(val) ((val) & 0x100)
#define SUPPORT_GETBIT9(val) ((val) & 0x200)
#define SUPPORT_GETBIT10(val) ((val) & 0x400)
#define SUPPORT_GETBIT11(val) ((val) & 0x800)
#define SUPPORT_GETBIT12(val) ((val) & 0x1000)
#define SUPPORT_GETBIT13(val) ((val) & 0x2000)
#define SUPPORT_GETBIT14(val) ((val) & 0x4000)
#define SUPPORT_GETBIT15(val) ((val) & 0x8000)
#define SUPPORT_GETBIT16(val) ((val) & 0x10000)
#define SUPPORT_GETBIT17(val) ((val) & 0x20000)
#define SUPPORT_GETBIT18(val) ((val) & 0x40000)
#define SUPPORT_GETBIT19(val) ((val) & 0x80000)
#define SUPPORT_GETBIT20(val) ((val) & 0x100000)
#define SUPPORT_GETBIT21(val) ((val) & 0x200000)
#define SUPPORT_GETBIT22(val) ((val) & 0x400000)
#define SUPPORT_GETBIT23(val) ((val) & 0x800000)
#define SUPPORT_GETBIT24(val) ((val) & 0x1000000)
#define SUPPORT_GETBIT25(val) ((val) & 0x2000000)
#define SUPPORT_GETBIT26(val) ((val) & 0x4000000)
#define SUPPORT_GETBIT27(val) ((val) & 0x8000000)
#define SUPPORT_GETBIT28(val) ((val) & 0x10000000)
#define SUPPORT_GETBIT29(val) ((val) & 0x20000000)
#define SUPPORT_GETBIT30(val) ((val) & 0x40000000)
#define SUPPORT_GETBIT31(val) ( (val) & 0x80000000)






#endif


