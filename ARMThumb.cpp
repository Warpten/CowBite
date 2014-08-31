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

#include <stdlib.h>
#include "Console.h"

#include "ARM.h"
#include "ALU.h"
#include "BIOS.h"
#include "ARMThumb.h"

//This is a special version of a noop that prints to the console.
//Invented by joat.  Slick!
void ARM_dprintThumb() {
	if ((arm.r[0] == 0xC0DED00D) && (arm.r[1] == 0)) {
		u32 address = arm.r[2];
		u32 index = address >> 24;
		u32 offset = address & 0x00FFFFFF;
		if (offset >= gbaMem.memorySize[index]) {
			Console_print("Error:  Attempting to dprint from inaccessible region of memory.\n");
		} else {
			//Console_print("dprint: ");
			Console_print((char*)(gbaMem.u8Mem[index]+offset));
		}
	}
	ARM_nextInstructionThumb();
}

/////////////////////////////////////////////////////////////
//Move shifted register operations
//////////////////////////////////////////////////////////////
//lsr (equivalent to movs with a imm shift)
void ARM_lslThumb() {
	register u32 shiftAmt = (arm.currentInstruction >> 6) & 0x1F;
	register u32 value = ALU_barrelShiftS( RSTHUMB(arm.currentInstruction), shiftAmt, SHIFT_LSL );
	RDTHUMB(arm.currentInstruction) = value;
	ALU_setNZFlags(value);
	ARM_nextInstructionThumb();
}

//lsr (equivalent to movs with a imm shift)
void ARM_lsrThumb() {
	register u32 shiftAmt = (arm.currentInstruction >> 6) & 0x1F;
	register u32 value = ALU_barrelShiftS( RSTHUMB(arm.currentInstruction), shiftAmt, SHIFT_LSR );
	RDTHUMB(arm.currentInstruction) = value;
	ALU_setNZFlags(value);
	ARM_nextInstructionThumb();
}

//lsr (equivalent to movs with a imm shift)
void ARM_asrThumb() {
	register u32 shiftAmt = (arm.currentInstruction >> 6) & 0x1F;
	register u32 value = ALU_barrelShiftS( RSTHUMB(arm.currentInstruction), shiftAmt, SHIFT_ASR );
	RDTHUMB(arm.currentInstruction) = value;
	ALU_setNZFlags(value);
	ARM_nextInstructionThumb();
}

////////////////////////////////////////////////////////
//Add/subtract
////////////////////////////////////////////////////////////////

//add (equivalent to ARM adds)
void ARM_addThumb() {
	register u32 tempA = RSTHUMB(arm.currentInstruction);
	register u32 tempB = RNTHUMB(arm.currentInstruction);
	register u32 tempC = RDTHUMB(arm.currentInstruction) = tempA + tempB;
	ALU_setAddFlags(tempA, tempB, tempC);
	ARM_nextInstructionThumb();
}

//sub (equivalent to ARM subs)
void ARM_subThumb() {
	register u32 tempA = RSTHUMB(arm.currentInstruction);
	register u32 tempB = RNTHUMB(arm.currentInstruction);
	register u32 tempC = RDTHUMB(arm.currentInstruction) = tempA - tempB;
	ALU_setSubFlags(tempA, tempB, tempC);
	ARM_nextInstructionThumb();
}

//addi (equivalent to ARM addsi)
void ARM_addiThumb() {
	register u32 tempA = RSTHUMB(arm.currentInstruction);
	register u32 tempB = (arm.currentInstruction >> 6) & 0x7;
	register u32 tempC = RDTHUMB(arm.currentInstruction) = tempA + tempB;
	ALU_setAddFlags(tempA, tempB, tempC);
	ARM_nextInstructionThumb();
}

//subi (equivalent to ARM subsi)
void ARM_subiThumb() {
	register u32 tempA = RSTHUMB(arm.currentInstruction);
	register u32 tempB = (arm.currentInstruction >> 6) & 0x7;
	register u32 tempC = RDTHUMB(arm.currentInstruction) = tempA - tempB;
	ALU_setSubFlags(tempA, tempB, tempC);
	ARM_nextInstructionThumb();
}


////////////////////////////////////////////////////////////////
//Move/compare/add/subtract immediate
//////////////////////////////////////////////////////////////////////

//movImmediate (equivalent to movsi)
void ARM_movImmediateThumb() {
	register u32 value = arm.currentInstruction & 0xFF;
	register u32 rd = (arm.currentInstruction >> 8) & 0x7;
	arm.r[rd] = value;
	ALU_setNZFlags(value);
	ARM_nextInstructionThumb();
}

//cmpImmediate (equivalent to cmpsi)
void ARM_cmpImmediateThumb() {
	register u32 tempB = arm.currentInstruction & 0xFF;
	register u32 rd = (arm.currentInstruction >> 8) & 0x7;
	register u32 tempA = arm.r[rd];
	register u32 tempC = tempA - tempB;
	ALU_setSubFlags(tempA, tempB, tempC);
	ARM_nextInstructionThumb();
}

//addImmediate (equivalent to ARM addsi, differs from addi in that it is "unary")
void ARM_addImmediateThumb() {
	register u32 tempB = arm.currentInstruction & 0xFF;
	register u32 rd = (arm.currentInstruction >> 8) & 0x7;
	register u32 tempA = arm.r[rd];
	register u32 tempC = arm.r[rd] = tempA + tempB;
	ALU_setAddFlags(tempA, tempB, tempC);
	ARM_nextInstructionThumb();
}

//subImmediate (equivalent to ARM subsi, differs from subi in that it is "unary")
void ARM_subImmediateThumb() {
	register u32 tempB = arm.currentInstruction & 0xFF;
	register u32 rd = (arm.currentInstruction >> 8) & 0x7;
	register u32 tempA = arm.r[rd];
	register u32 tempC = arm.r[rd] = tempA - tempB;
	ALU_setSubFlags(tempA, tempB, tempC);
	ARM_nextInstructionThumb();
}


//////////////////////////////////////////////////////////////////////////////
//ALU operations
///////////////////////////////////////////////////////////////////////////////

//and (equivalent to ARM ands)
void ARM_andThumb() {
	register u32 rd = (arm.currentInstruction & 0x7);
	register u32 value = arm.r[rd] = arm.r[rd] & RSTHUMB(arm.currentInstruction);
	ALU_setNZFlags(value);
	ARM_nextInstructionThumb();
}

//eor (equivalent to ARM eors)
void ARM_eorThumb() {
	register u32 rd = (arm.currentInstruction & 0x7);
	register u32 value = arm.r[rd] = arm.r[rd] ^ RSTHUMB(arm.currentInstruction);
	ALU_setNZFlags(value);
	ARM_nextInstructionThumb();
}

//lsl (equivalent to ARM movs with reg shift)
void ARM_lslRegThumb() {
	register u32 rd = (arm.currentInstruction & 0x7);
	register u32 value = arm.r[rd] = ALU_barrelRegShiftS (arm.r[rd], RSTHUMB(arm.currentInstruction), SHIFT_LSL);
	ALU_setNZFlags(value);
	ARM_nextInstructionThumb();
}

//lsr (equivalent to ARM movs with reg shift)
void ARM_lsrRegThumb() {
	register u32 rd = (arm.currentInstruction & 0x7);
	register u32 value = arm.r[rd] = ALU_barrelRegShiftS (arm.r[rd], RSTHUMB(arm.currentInstruction), SHIFT_LSR);
	ALU_setNZFlags(value);
	ARM_nextInstructionThumb();
}

//asr (equivalent to ARM movs with reg shift)
void ARM_asrRegThumb() {
	register u32 rd = (arm.currentInstruction & 0x7);
	register u32 value = arm.r[rd] = ALU_barrelRegShiftS (arm.r[rd], RSTHUMB(arm.currentInstruction), SHIFT_ASR);
	ALU_setNZFlags(value);
	ARM_nextInstructionThumb();
}

//adc (equivalent to ARM adcs)
void ARM_adcThumb() {
	register u32 rd = (arm.currentInstruction & 0x7);
	register u32 tempA = arm.r[rd];
	register u32 tempB = RSTHUMB(arm.currentInstruction);
	register u32 tempC = arm.r[rd] = tempA + tempB + arm.C;
	//Should I be adding tempC onto tempB first?
	ALU_setAddFlags(tempA, tempB, tempC);
	ARM_nextInstructionThumb();
}

//sbc (equivalent to ARM sbcs)
void ARM_sbcThumb() {
	register u32 rd = (arm.currentInstruction & 0x7);
	register u32 tempA = arm.r[rd];
	register u32 tempB = RSTHUMB(arm.currentInstruction);
	register u32 tempC = arm.r[rd] = tempA - tempB - !arm.C;
	//Should I be adding arm.C onto tempB first?  What if it carries?
	ALU_setSubFlags(tempA, tempB, tempC);
	ARM_nextInstructionThumb();
}

//ror (equivalent to ARM movs with reg shift)
void ARM_rorThumb() {
	register u32 rd = (arm.currentInstruction & 0x7);
	register u32 value = arm.r[rd] = ALU_barrelRegShiftS (arm.r[rd], RSTHUMB(arm.currentInstruction), SHIFT_ROR);
	ALU_setNZFlags(value);
	ARM_nextInstructionThumb();
}

//tst (equivalent to ARM tsts)
void ARM_tstThumb() {
	register u32 rd = (arm.currentInstruction & 0x7);
	register u32 value = arm.r[rd] & RSTHUMB(arm.currentInstruction);
	ALU_setNZFlags(value);
	ARM_nextInstructionThumb();
}

//neg (equivalent to ARM rsbs rd, rs, 0)
void ARM_negThumb() {
	register u32 tempA = RSTHUMB(arm.currentInstruction);
	register u32 tempC = RDTHUMB(arm.currentInstruction) = 0 - tempA;
	ALU_setSubFlags(0, tempA, tempC);
	ARM_nextInstructionThumb();
}

//cmp (equivalent to ARM cmps)
void ARM_cmpThumb() {
	register u32 rd = (arm.currentInstruction & 0x7);
	register u32 tempA = arm.r[rd];
	register u32 tempB = RSTHUMB(arm.currentInstruction);
	register u32 tempC = tempA - tempB ;
	ALU_setSubFlags(tempA, tempB, tempC);
	ARM_nextInstructionThumb();
}

//cmn (equivalent to ARM cmns)
void ARM_cmnThumb() {
	register u32 rd = (arm.currentInstruction & 0x7);
	register u32 tempA = arm.r[rd];
	register u32 tempB = RSTHUMB(arm.currentInstruction);
	register u32 tempC = tempA + tempB ;
	ALU_setAddFlags(tempA, tempB, tempC);
	ARM_nextInstructionThumb();
}

//orr (equivalent to ARM orrs)
void ARM_orrThumb() {
	register u32 rd = (arm.currentInstruction & 0x7);
	register u32 value = arm.r[rd] = arm.r[rd] | RSTHUMB(arm.currentInstruction);
	ALU_setNZFlags(value);
	ARM_nextInstructionThumb();
}

//mul (equivalent to ARM muls)
void ARM_mulThumb() {
	register u32 rd = (arm.currentInstruction & 0x7);
	register u32 tempA = arm.r[rd];
	register u32 tempB = RSTHUMB(arm.currentInstruction);
	register u32 tempC = arm.r[rd] = tempB * tempA ;
	ALU_setNZFlags(tempC);
	ARM_nextInstructionThumb();
}

//bic (equivalent to ARM bics)
void ARM_bicThumb() {
	register u32 rd = (arm.currentInstruction & 0x7);
	register u32 value = arm.r[rd] = arm.r[rd] & (~RSTHUMB(arm.currentInstruction));
	ALU_setNZFlags(value);
	ARM_nextInstructionThumb();
}

//mvn (equivalent to ARM mvns)
void ARM_mvnThumb() {
	register u32 rd = (arm.currentInstruction & 0x7);
	register u32 value = arm.r[rd] = ~RSTHUMB(arm.currentInstruction);
	ALU_setNZFlags(value);
	ARM_nextInstructionThumb();
}


/////////////////////////////////////////////////////////////////////////////////
//Hi regiester operations/branch exchange
//Note that I have made the hi operations "slower" in order to require fewer of them.
//Could be optimized by creating four permutations, one for each combination of hi/low.
//Also note that only cmp sets the condition codes in this group.
///////////////////////////////////////////////////////////////////////

//addhi (equivalent to ARM add, no S)
void ARM_addhiThumb() {
	register u32 rd = SUPPORT_GETRDTHUMB(arm.currentInstruction) | (SUPPORT_GETBIT7(arm.currentInstruction) >> 4);	//slow
	register u32 rs = SUPPORT_GETRSTHUMB(arm.currentInstruction) | (SUPPORT_GETBIT6(arm.currentInstruction) >> 3);	//slower
	arm.r[rd] = arm.r[rd] + arm.r[rs];
	if (rd == 15) { 
		arm.r[15] = arm.r[15] & 0xFFFFFFFE;	//Make sure they don't set bit 0 to 1
		ARM_skipInstructionThumb(); 
	} else ARM_nextInstructionThumb();
}

//cmphi (equivalent to ARM cmps)
void ARM_cmphiThumb() {
	register u32 rd = SUPPORT_GETRDTHUMB(arm.currentInstruction) | (SUPPORT_GETBIT7(arm.currentInstruction) >> 4);	//slow
	register u32 rs = SUPPORT_GETRSTHUMB(arm.currentInstruction) | (SUPPORT_GETBIT6(arm.currentInstruction) >> 3);	//slower
	register u32 tempA = arm.r[rd];
	register u32 tempB = arm.r[rs];
	register u32 tempC = tempA - tempB;
	ALU_setSubFlags (tempA, tempB, tempC);
	ARM_nextInstructionThumb();
}

//movhi (equivalent to ARM mov, no S)
void ARM_movhiThumb() {
	//u32 crap = arm.r[15];
	register u32 rd = SUPPORT_GETRDTHUMB(arm.currentInstruction) | (SUPPORT_GETBIT7(arm.currentInstruction) >> 4);	//slow
	register u32 rs = SUPPORT_GETRSTHUMB(arm.currentInstruction) | (SUPPORT_GETBIT6(arm.currentInstruction) >> 3);	//slower
	arm.r[rd] = arm.r[rs];
	if (rd == 15) { 
		arm.r[15] = arm.r[15] & 0xFFFFFFFE;	//Make sure they don't set bit 0 to 1.
		ARM_skipInstructionThumb(); 
	} else ARM_nextInstructionThumb();
}

//bxhi (equivalent to ARM bx)
void ARM_bxhiThumb() {
	//u32 crap = arm.r[15];
	register u32 rs = SUPPORT_GETRSTHUMB(arm.currentInstruction) | (SUPPORT_GETBIT6(arm.currentInstruction) >> 3);	//slower
	register u32 address = arm.r[rs];
	if ( (address & 0x00FFFFFF) >= gbaMem.memorySize[(address >> 24)]) {
		sprintf(console.tempBuf, "Illegal branch to location 0x%08X\n", address);
		Console_print(console.tempBuf);
		arm.paused = 1;
		return;
	}
	arm.r[15] = address & 0xFFFFFFFE;
	//Check to see if it's time to switch to arm mode.
	if ( address & 0x1) {
		arm.thumbMode = 1;
		arm.CPSR = arm.CPSR | 0x20;	//Set T bit
		ARM_skipInstructionThumb();

	} else {
		arm.thumbMode = 0;
		arm.CPSR = arm.CPSR & 0xFFFFFFDF;	//Unset T bit
		ARM_skipInstruction();							//flush queue.
	}
}

/////////////////////////////////////////////////////////////////////
//PC-relative load
////////////////////////////////////////////////////////////////////
//ldrpc (equivalent to ARM LDR, Rd, [R15, #Imm] )
void ARM_ldrpcThumb() {
	register u32 rd = (arm.currentInstruction >> 8) & 0x7;
	register u32 offset = (arm.currentInstruction & 0xFF) << 2;
	u32 address = (arm.r[15] & 0xFFFFFFFFC) + offset;
	arm.r[rd] = Memory_readWord(address);
	ARM_nextInstructionThumb();
}

//////////////////////////////////////////////////////////////
//Load/store with register offset
//////////////////////////////////////////////////////////////

//ldrThumb (equivalent to ARM ldrwaPre without writeback)
void ARM_ldrThumb() {
	u32 address = RBTHUMB(arm.currentInstruction) + ROTHUMB(arm.currentInstruction);
	RDTHUMB(arm.currentInstruction) = Memory_readWord(address);				//Read the value
	ARM_nextInstructionThumb();
}

//ldrbThumb (equivalent to ARM ldrbaPre without writeback)
void ARM_ldrbThumb() {
	u32 address = RBTHUMB(arm.currentInstruction) + ROTHUMB(arm.currentInstruction);
	RDTHUMB(arm.currentInstruction) = Memory_readWord(address) & 0x000000FF;	//Read the value
	ARM_nextInstructionThumb();
}

//strThumb (equivalent to ARM strwaPre without writeback)
void ARM_strThumb() {
	u32 address = RBTHUMB(arm.currentInstruction) + ROTHUMB(arm.currentInstruction);
	Memory_writeWord(address, RDTHUMB(arm.currentInstruction));				//Write the value
	ARM_nextInstructionThumb();
}

//strbThumb (equivalent to ARM strbaPre without writeback)
void ARM_strbThumb() {
	u32 address = RBTHUMB(arm.currentInstruction) + ROTHUMB(arm.currentInstruction);
	Memory_writeByte(address, RDTHUMB(arm.currentInstruction));	//Write the value
	ARM_nextInstructionThumb();
}

////////////////////////////////////////////////////////
//Load/store signed byte/halfword
///////////////////////////////////////////////////////////

//strh (equivalent to ARM strhaPre)
void ARM_strhThumb() {
	u32 address = RBTHUMB(arm.currentInstruction) + ROTHUMB(arm.currentInstruction);
	Memory_writeHalfWord(address, RDTHUMB(arm.currentInstruction));				//Write the value
	ARM_nextInstructionThumb();
}

//ldrh (equivalent to ARM ldrhaPre)
void ARM_ldrhThumb() {
	u32 address = RBTHUMB(arm.currentInstruction) + ROTHUMB(arm.currentInstruction);
	RDTHUMB(arm.currentInstruction) = Memory_readHalfWord(address);				//Read the value
	ARM_nextInstructionThumb();
}

//ldsb (equivalent to ARM ldrsbaPre)
void ARM_ldsbThumb() {
	u32 address = RBTHUMB(arm.currentInstruction) + ROTHUMB(arm.currentInstruction);
	register s32 signedValue = Memory_readWord(address) & 0x000000FF;		//Read the value
	signedValue = signedValue << 24;
	signedValue = signedValue >> 24;
	RDTHUMB(arm.currentInstruction) = signedValue;
	ARM_nextInstructionThumb();
}

//ldsh (equivalent to ARM ldrshaPre)
void ARM_ldshThumb() {
	u32 address = RBTHUMB(arm.currentInstruction) + ROTHUMB(arm.currentInstruction);
	register s32 signedValue = Memory_readHalfWord(address);		//Read the value
	signedValue = signedValue << 16;
	signedValue = signedValue >> 16;
	RDTHUMB(arm.currentInstruction) = signedValue;
	ARM_nextInstructionThumb();
}

//////////////////////////////////////////////////////////////
//Load/store with immediate offset
//////////////////////////////////////////////////////////////

//ldriThumb (equivalent to ARM ldrwiaPre without writeback)
void ARM_ldriThumb() {
	u32 address = RBTHUMB(arm.currentInstruction) + ((arm.currentInstruction >> 4) & 0x7C);	//Shift and chop off bottom bits
	RDTHUMB(arm.currentInstruction) = Memory_readWord(address);				//Read the value
	ARM_nextInstructionThumb();
}

//ldrbiThumb (equivalent to ARM ldrbiaPre without writeback)
void ARM_ldrbiThumb() {
	u32 address = RBTHUMB(arm.currentInstruction) + ((arm.currentInstruction >> 6) & 0x1F);
	RDTHUMB(arm.currentInstruction) = Memory_readWord(address) & 0x000000FF;	//Read the value
	ARM_nextInstructionThumb();
}

//strThumb (equivalent to ARM strwiaPre without writeback)
void ARM_striThumb() {
	u32 address = RBTHUMB(arm.currentInstruction) + ((arm.currentInstruction >> 4) & 0x7C);	//Shift and chop off bottom bits
	Memory_writeWord(address, RDTHUMB(arm.currentInstruction));				//Write the value
	ARM_nextInstructionThumb();
}

//strbThumb (equivalent to ARM strbiaPre without writeback)
void ARM_strbiThumb() {
	u32 address = RBTHUMB(arm.currentInstruction) + ((arm.currentInstruction >> 6) & 0x1F);
	Memory_writeByte(address, RDTHUMB(arm.currentInstruction));	//Write the value
	ARM_nextInstructionThumb();
}

////////////////////////////////////////////////////////
//Load/store immediate halfword
///////////////////////////////////////////////////////////

//strhi (equivalent to ARM strhiaPre)
void ARM_strhiThumb() {
	u32 address = RBTHUMB(arm.currentInstruction) + ((arm.currentInstruction >> 5) & 0x3E);
	Memory_writeHalfWord(address, RDTHUMB(arm.currentInstruction));				//Write the value
	ARM_nextInstructionThumb();
}

//ldrhi (equivalent to ARM ldrhiaPre)
void ARM_ldrhiThumb() {
	u32 address = RBTHUMB(arm.currentInstruction) + ((arm.currentInstruction >> 5) & 0x3E);
	RDTHUMB(arm.currentInstruction) = Memory_readHalfWord(address);				//Read the value
	ARM_nextInstructionThumb();
}

/////////////////////////////////////////////////////////////
//SP-relative load store
//(question:  is the stack pointer r13 or r7?)
/////////////////////////////////////////////////////////////

//ldrspThumb (equivalent to ARM ldrwiaPre without writeback)
void ARM_ldrspThumb() {
	u32 rd = (arm.currentInstruction >> 8) & 0x7;
	u32 address = arm.r[13] + ((arm.currentInstruction & 0xFF) << 2);	
	arm.r[rd] = Memory_readWord(address);				//Read the value
	ARM_nextInstructionThumb();
}

//strspThumb (equivalent to ARM strwiaPre without writeback)
void ARM_strspThumb() {
	u32 rd = (arm.currentInstruction >> 8) & 0x7;
	u32 address = arm.r[13] + ((arm.currentInstruction & 0xFF) << 2);
	Memory_writeWord(address, arm.r[rd]);				//Write the value
	ARM_nextInstructionThumb();
}

/////////////////////////////////////////////////////////////
//Load address
/////////////////////////////////////////////////////////////

//adrlpcThumb (equivalent to ARM addi)
void ARM_adrlpcThumb() {
	u32 rd = (arm.currentInstruction >> 8) & 0x7;
	arm.r[rd] = (arm.r[15] & 0xFFFFFFC) + ((arm.currentInstruction & 0xFF) << 2);	
	ARM_nextInstructionThumb();
}

//adrlspThumb (equivalent to ARM addi)
void ARM_adrlspThumb() {
	u32 rd = (arm.currentInstruction >> 8) & 0x7;
	arm.r[rd] = arm.r[13] + ((arm.currentInstruction & 0xFF) << 2);
	ARM_nextInstructionThumb();
}

//////////////////////////////////////////////////////////
//Add offset to stack pointer
////////////////////////////////////////////////////////

//addsp (equivalent to ARM addi)
void ARM_addspThumb() {
	arm.r[13] = arm.r[13] + ((arm.currentInstruction & 0x7F) << 2);
	ARM_nextInstructionThumb();
}

//subsp (equivalent to ARM subi)
void ARM_subspThumb() { 
	arm.r[13] = arm.r[13] - ((arm.currentInstruction & 0x7F) << 2);
	ARM_nextInstructionThumb();
}

///////////////////////////////////////////////////////
//Push/pop register
//Note that this is assumed to always be full descending
//////////////////////////////////////////////////////

//pop (equivalent to ARM loadStoreMultiple)
void ARM_popThumb() {
	u32 regList;
	u32 instruction = arm.currentInstruction;	//We must save the instruction because instruction3 could change
										//if we refill the queue (duh?)
	u32 address = arm.r[13];
	//address &= 0xFFFFFFFC;	//Test
	//Get the register list
	regList = instruction & 0x000000FF;

	
	//The lowest numbered register is always stored at the lowest address...
	for (u32 i = 0; i<8; i++) {
		if ( (regList >> i) & 0x1) {	//If bit "i" is set
			//These operations are post indexed(?)
			
			//Always update the stack pointer.
			
			arm.r[i] = Memory_readWord(address & 0xFFFFFFFC);				//No rotation of unaligne dowrds
			address += 4;; //In a full descending stack, load multiples go down, store multiples go up			
			arm.r[13] = address;
		}//End of if
	}//End of for

//This would get placed at the end if the offset had been positive...
	if ( SUPPORT_GETBIT8(instruction)) {	//If we are also loading the PC...
		//I am not sure if there is supposed to be a state change when you pop the pc
		//and bit 0 is set.
		//In this instance i am not implementing one.  I don't think mappy does either.
		arm.r[15] = Memory_readWord(address & 0xFFFFFFFC) & 0xFFFFFFFE;
		address +=4;
		arm.r[13] = address;
		ARM_nextInstructionThumb();
	}


	ARM_nextInstructionThumb();
}

//push (equivalent to ARM loadStoreMultiple)
void ARM_pushThumb() {
	u32 regList;
	u32 address = arm.r[13];
//	address &= 0xFFFFFFFC;	//TEST
	//Get the register list
	regList = arm.currentInstruction & 0x000000FF;
	
	//This would get placed at the beginning if the offset had been negative ...
	if ( SUPPORT_GETBIT8(arm.currentInstruction)) {	//If we are also storing the LR . . .
		address -= 4;
		arm.r[13] = address;
		Memory_writeWord(address, arm.r[14]);
	}

	//The lowest numbered register is always stored at the lowest address...
	for (s32 i = 8; i >= 0; i--) {
		if ( (regList >> i) & 0x1) {	//If bit "i" is set
			//These operations are always pre indexed.
			address -= 4;	//In a full descending stack, load multiples go down, store multiples go up
			
			//Always update the stack pointer.
			arm.r[13] = address;
			Memory_writeWord(address, arm.r[i]  );
		}//End of if
	}//End of for


	ARM_nextInstructionThumb();
}

//////////////////////////////////////////////////////////////////////
//multiple load/store
///////////////////////////////////////////////////////////////////////

//ldmia (equivalent to ARM loadStoreMultiple)
void ARM_ldmiaThumb() {
	u32 regList;
	u32 rb = (arm.currentInstruction >> 8) & 0x7;
	u32 address = arm.r[rb];
	
//	address &= 0xFFFFFFFC;	//TEST

	//Get the register list
	regList = arm.currentInstruction & 0x000000FF;

	//The lowest numbered register is always stored at the lowest address...
	for (u32 i = 0; i < 8; i++) {
		if ( (regList >> i) & 0x1) {	//If bit "i" is set
			//These operations are always post indexed.(?)
						
			//Always update the base
			//LDMIA does not rotate words :)
			arm.r[i] = Memory_readWord(address & 0xFFFFFFFC);				//Read the value
			address += 4; 
			arm.r[rb] = address;
			
			
		}//End of if
	}//End of for

	ARM_nextInstructionThumb();
}

//stmia (equivalent to ARM loadStoreMultiple)
void ARM_stmiaThumb() {
	u32 regList;
	u32 rb = (arm.currentInstruction >> 8) & 0x7;
	u32 address = arm.r[rb];
	//address &= 0xFFFFFFFC;
	//Get the register list
	regList = arm.currentInstruction & 0x000000FF;
	
	//The lowest numbered register is always stored at the lowest address...
	for (u32 i = 0; i < 8; i++) {
		if ( (regList >> i) & 0x1) {	//If bit "i" is set
			//These operations are always post indexed.(?)
			
			//Always update the base
						
			Memory_writeWord(address,   arm.r[i]  );
			address += 4;	
			arm.r[rb] = address;
		}//End of if
	}//End of for

	ARM_nextInstructionThumb();
}


/////////////////////////////////////////////////////////////////
//Conditional branch
//Is the offset 8 bits or 10?  I do 8 bits here.
//////////////////////////////////////////////////////////////////

//bcond (equivalent to ARM b)
void ARM_bcondThumb () {
	u32 condition = (arm.currentInstruction >> 8) & 0xF;
	if ( arm.conditionTable[condition]() ) {
		s32 offset = (arm.currentInstruction & 0xFF) << 24;
		offset = offset >> 23;	//Get sign.
		u32 address = arm.r[15] + offset;
		if ( (address & 0x00FFFFFF) >= gbaMem.memorySize[(address >> 24)]) {
			sprintf(console.tempBuf, "Illegal branch to location 0x%08X\n", address);
			Console_print(console.tempBuf);
			arm.paused = 1;
			return;
		}
		arm.r[15] = address;
		ARM_skipInstructionThumb();							//flush queue.
	} else ARM_nextInstructionThumb();
}

///////////////////////////////////////////////////////////
//Software interrupt
//////////////////////////////////////////////////////////

//swi (equivalent to ARM swi)
void ARM_swiThumb() {
	u32 code = arm.currentInstruction & 0x000000FF;
	//arm.r[14] = arm.r[15] -2;	//Save next instr. to link register.
	//Switch off for the correct instruction.
	BIOS_exec(code);

}

//////////////////////////////////////////////////////////
//Unconditional branch
//////////////////////////////////////////////////////////

void ARM_bThumb() {
	s32 offset = (arm.currentInstruction & 0x7FF) << 21;
	offset = offset >> 20;	//Get sign, halfword align.
	u32 address = arm.r[15] + offset;
	if ( (address & 0x00FFFFFF) >= gbaMem.memorySize[(address >> 24)]) {
		sprintf(console.tempBuf, "Illegal branch to location 0x%08X\n", address);
		Console_print(console.tempBuf);
		arm.paused = 1;
		return;
	}
	arm.r[15] = address;
	ARM_skipInstructionThumb();							//flush queue.
}

/////////////////////////////////////////////////////
//Long branch with link
/////////////////////////////////////////////////////

void ARM_blThumb() {
	s32 offset;
	u32 nextAddress;
	if (SUPPORT_GETBIT11(arm.currentInstruction))	{	//If if this is the low part..
		offset = (arm.currentInstruction & 0x7FF) << 1;	//Do not get the sign bit because
		//offset = offset >> 20;						//this is the low half of a signed number.
		nextAddress = arm.r[15] - 1;	//Save the address of the next instruction, set bit 1.
		u32 address = arm.r[14] + offset;	//Should this be an add or an or?
		arm.r[14] = nextAddress;
		if ( (address & 0x00FFFFFF) >= gbaMem.memorySize[(address >> 24)]) {
			sprintf(console.tempBuf, "Illegal branch to location 0x%08X\n", address);
			Console_print(console.tempBuf);
			arm.paused = 1;
			return;
		}
		arm.r[15] = address;
		ARM_skipInstructionThumb();							//flush queue.

		//temp2 = 1;
	} else {
		offset = (arm.currentInstruction & 0x7FF) << 21;						//if this is the high part
		offset = offset >> 9;					//get sign, shift right (total shift of 12 bits left)
		arm.r[14] = arm.r[15] + offset;			//place value in link register.
		ARM_nextInstructionThumb();
		//temp2 = 0;
	}
	
	
}