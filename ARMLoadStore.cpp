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
#include "ALU.h"
#include "Memory.h"	//Necessary because of inlining
#include "ARMLoadStore.h"




/////////////////////////////////////////
//loadStoreFucntion(u32 instruction)
//This returns a function based on the instruction passed in.
//The function can then be placed in a table and used again later.
////////////////////////////////////////////////////////
void* ARM_loadStoreFunction(u32 instruction) {
	u32 load, i, u, b, pre, w;

	load = SUPPORT_GETBIT20(instruction);	//If it's a load
	i = !SUPPORT_GETBIT25(instruction);	//if it's immediate
	u = SUPPORT_GETBIT23(instruction);	//if we add the offset
	pre = SUPPORT_GETBIT24(instruction);	//If it's a pre-indexed instruction
	b = SUPPORT_GETBIT22(instruction);		//if it's a byte transfer
	w = SUPPORT_GETBIT21(instruction);		//if we write back

	if (load) {
		if (b) 
			if (i)	
				if (u)
					if (pre) return ARM_ldrbiaPre;
					else return ARM_ldrbiaPost;
				else
					if (pre) return ARM_ldrbisPre;
					else return ARM_ldrbisPost;
			else 		//If it's a register
				if (u)
					if (pre) return ARM_ldrbaPre;
					else return ARM_ldrbaPost;
				else
					if (pre) return ARM_ldrbsPre;
					else return ARM_ldrbsPost;
		else
			if (i) 	//If it's immediate
				if (u)
					if (pre) return ARM_ldrwiaPre;
					else return ARM_ldrwiaPost;
				else
					if (pre) return ARM_ldrwisPre;
					else return ARM_ldrwisPost;
			else 		//If it's a register
				if (u)
					if (pre) return ARM_ldrwaPre;
					else return ARM_ldrwaPost;
				else
					if (pre) return ARM_ldrwsPre;
					else return ARM_ldrwsPost;
	
	} else {	//if it's a store		
		if (b) 
			if (i)	//If it's immediate
				if (u)
					if (pre) return ARM_strbiaPre;
					else return ARM_strbiaPost;
				else
					if (pre) return ARM_strbisPre;
					else return ARM_strbisPost;
			else 		//If it's a register
				if (u)
					if (pre) return ARM_strbaPre;
					else return ARM_strbaPost;
				else
					if (pre) return ARM_strbsPre;
					else return ARM_strbsPost;
		else
			if (i) 	//If it's immediate
				if (u)
					if (pre) return ARM_strwiaPre;
					else return ARM_strwiaPost;
				else
					if (pre) return ARM_strwisPre;
					else return ARM_strwisPost;
			else 		//If it's a register
				if (u)
					if (pre) return ARM_strwaPre;
					else return ARM_strwaPost;
				else
					if (pre) return ARM_strwsPre;
					else return ARM_strwsPost;
	}
	return ARM_defaultInstruction;
}

/////////////////////////////////////////
//loadStoreHalfwordFunction(u32 instruction)
//This returns a function based on the instruction passed in.
//The function can then be placed in a table and used again later.
////////////////////////////////////////////////////////
void* ARM_loadStoreHalfwordFunction(u32 instruction) {
	u32 load, b, s, i, u, pre, w, transferType;

	
	load = SUPPORT_GETBIT20(instruction);	//If it's a load
	i = SUPPORT_GETBIT22(instruction);	//if it's immediate
	u = SUPPORT_GETBIT23(instruction);	//if we add the offset
	pre = SUPPORT_GETBIT24(instruction);	//If it's a pre-indexed instrhuction
	w = SUPPORT_GETBIT21(instruction);		//if we write back
	transferType = (instruction >> 5) & 0x3;
	
	s = (transferType == 0x3);				//if it's a signed halfword...
		
	b = (transferType == 0x2);				//load sign extended byte
		

	//This is basically like the load store instructions, but
	//instead of having the byte option, you have either the signed
	//byte or the signed halfword byte load option.  There is no
	//separate store option for signed bytes and halfwords.	
	if (load) {
		if (b)	//if it's a sign extended byte...
			if (i)	
				if (u)
					if (pre) return ARM_ldrsbiaPre;
					else return ARM_ldrsbiaPost;
				else
					if (pre) return ARM_ldrsbisPre;
					else return ARM_ldrsbisPost;
			else 		//If it's a register
				if (u)
					if (pre) return ARM_ldrsbaPre;
					else return ARM_ldrsbaPost;
				else
					if (pre) return ARM_ldrsbsPre;
					else return ARM_ldrsbsPost;
		else if (s)
			if (i) 	//If it's immediate
				if (u)
					if (pre) return ARM_ldrshiaPre;
					else return ARM_ldrshiaPost;
				else
					if (pre) return ARM_ldrshisPre;
					else return ARM_ldrshisPost;
			else 		//If it's a register
				if (u)
					if (pre) return ARM_ldrshaPre;
					else return ARM_ldrshaPost;
				else
					if (pre) return ARM_ldrshsPre;
					else return ARM_ldrshsPost;
		else
			if (i) 	//If it's immediate
				if (u)
					if (pre) return ARM_ldrhiaPre;
					else return ARM_ldrhiaPost;
				else
					if (pre) return ARM_ldrhisPre;
					else return ARM_ldrhisPost;
			else 		//If it's a register
				if (u)
					if (pre) return ARM_ldrhaPre;
					else return ARM_ldrhaPost;
				else
					if (pre) return ARM_ldrhsPre;
					else return ARM_ldrhsPost;
	
	} else {	//if it's a store		
			if (i) 	//If it's immediate
				if (u)
					if (pre) return ARM_strhiaPre;
					else return ARM_strhiaPost;
				else
					if (pre) return ARM_strhisPre;
					else return ARM_strhisPost;
			else 		//If it's a register
				if (u)
					if (pre) return ARM_strhaPre;
					else return ARM_strhaPost;
				else
					if (pre) return ARM_strhsPre;
					else return ARM_strhsPost;
	}
	return ARM_defaultInstruction;
}

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////
//BEGIN HUGE LIST OF FUNCTIONS
////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
//LOADS AND STORES
////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////
//ldrw() functions
///////////////////////////////////////////////////////

//load word, add offset, pre-indexing.
void ARM_ldrwaPre() {
	u32 address = RN(arm.currentInstruction) + IMMSHIFT(arm.currentInstruction);
	
	if ( SUPPORT_GETBIT21(arm.currentInstruction) ) //If only write back if W is set
		RN(arm.currentInstruction) = address;
	RD(arm.currentInstruction) = Memory_readWord (address);				//Read the value
	if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//load word, add offset, post-indexing.
void ARM_ldrwaPost() {
	register u32 tempIndex = SUPPORT_GETRN(arm.currentInstruction);	//Get the rn register number
	u32 address = *(arm.r+tempIndex);	//Read only from the base location
	*(arm.r+tempIndex) = address + IMMSHIFT(arm.currentInstruction);	//Write it back into rn
	RD(arm.currentInstruction) = Memory_readWord (address);				//Read the value
	if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//load word, subtract offset, pre-indexing.
void ARM_ldrwsPre() {
	u32 address = RN(arm.currentInstruction) - IMMSHIFT(arm.currentInstruction);
	
	if ( SUPPORT_GETBIT21(arm.currentInstruction) ) //If only write back if W is set
		RN(arm.currentInstruction) = address;
	RD(arm.currentInstruction) = Memory_readWord (address);				//Read the value
	if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//load word, subtract offset, post-indexing.
void ARM_ldrwsPost() {
	register u32 tempIndex = SUPPORT_GETRN(arm.currentInstruction);	//Get the rn register number
	u32 address = *(arm.r+tempIndex);	//Read only from the base location
	*(arm.r+tempIndex) = address - IMMSHIFT(arm.currentInstruction);	//Write it back into rn
	RD(arm.currentInstruction) = Memory_readWord (address);				//Read the value
	if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}


///////////////////////////////////////////////////////
//ldrwi() functions
///////////////////////////////////////////////////////

//load word immediate, add offset, pre-indexing.
void ARM_ldrwiaPre() {
	u32 address = RN(arm.currentInstruction) + (arm.currentInstruction & 0x00000FFF);
	
	if ( SUPPORT_GETBIT21(arm.currentInstruction) ) //If only write back if W is set
		RN(arm.currentInstruction) = address;

	RD(arm.currentInstruction) = Memory_readWord (address);				//Read the value
	if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
	
}

//load word immediate, add offset, post-indexing.
void ARM_ldrwiaPost() {
	register u32 tempIndex = SUPPORT_GETRN(arm.currentInstruction);	//Get the rn register number
	u32 address = *(arm.r+tempIndex);	//Read only from the base location
	*(arm.r+tempIndex) = address + (arm.currentInstruction & 0x00000FFF);	//Write it back into rn
	RD(arm.currentInstruction) = Memory_readWord (address);				//Read the value
	if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//load word immediate, subtract offset, pre-indexing.
void ARM_ldrwisPre() {
	u32 address = RN(arm.currentInstruction) - (arm.currentInstruction & 0x00000FFF);
	
	if ( SUPPORT_GETBIT21(arm.currentInstruction) ) //If only write back if W is set
		RN(arm.currentInstruction) = address;
	RD(arm.currentInstruction) = Memory_readWord (address);				//Read the value
	if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//load word immediate, subtract offset, post-indexing.
void ARM_ldrwisPost() {
	register u32 tempIndex = SUPPORT_GETRN(arm.currentInstruction);	//Get the rn register number
	u32 address = *(arm.r+tempIndex);	//Read only from the base location
	*(arm.r+tempIndex) = address - (arm.currentInstruction & 0x00000FFF);	//Write it back into rn
	RD(arm.currentInstruction) = Memory_readWord (address);				//Read the value
	if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

///////////////////////////////////////////////////////
//ldrb() functions
///////////////////////////////////////////////////////

//load byte, add offset, pre-indexing.
void ARM_ldrbaPre() {
	u32 address = RN(arm.currentInstruction) + IMMSHIFT(arm.currentInstruction);
	
	if ( SUPPORT_GETBIT21(arm.currentInstruction) ) //If only write back if W is set
		RN(arm.currentInstruction) = address;
	RD(arm.currentInstruction) = Memory_readByte (address);				//Read the value
	if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//load byte, add offset, post-indexing.
void ARM_ldrbaPost() {
	register u32 tempIndex = SUPPORT_GETRN(arm.currentInstruction);	//Get the rn register number
	u32 address = *(arm.r+tempIndex);	//Read only from the base location
	*(arm.r+tempIndex) = address + IMMSHIFT(arm.currentInstruction);	//Write it back into rn
	RD(arm.currentInstruction) = Memory_readByte (address);				//Read the value
	if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//load byte, subtract offset, pre-indexing.
void ARM_ldrbsPre() {
	u32 address = RN(arm.currentInstruction) - IMMSHIFT(arm.currentInstruction);
	
	if ( SUPPORT_GETBIT21(arm.currentInstruction) ) //If only write back if W is set
		RN(arm.currentInstruction) = address;
	RD(arm.currentInstruction) = Memory_readByte(address);				//Read the value
	if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//load byte, subtract offset, post-indexing.
void ARM_ldrbsPost() {
	register u32 tempIndex = SUPPORT_GETRN(arm.currentInstruction);	//Get the rn register number
	u32 address = *(arm.r+tempIndex);	//Read only from the base location
	*(arm.r+tempIndex) = address - IMMSHIFT(arm.currentInstruction);	//Write it back into rn
	RD(arm.currentInstruction) = Memory_readByte (address);				//Read the value
	if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}


///////////////////////////////////////////////////////
//ldrbi() functions
///////////////////////////////////////////////////////

//load byte immediate, add offset, pre-indexing.
void ARM_ldrbiaPre() {
	u32 address = RN(arm.currentInstruction) + (arm.currentInstruction & 0x00000FFF);
	
	if ( SUPPORT_GETBIT21(arm.currentInstruction) ) //If only write back if W is set
		RN(arm.currentInstruction) = address;
	RD(arm.currentInstruction) = Memory_readByte(address);				//Read the value
	if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//load byte immediate, add offset, post-indexing.
void ARM_ldrbiaPost() {
	register u32 tempIndex = SUPPORT_GETRN(arm.currentInstruction);	//Get the rn register number
	u32 address = *(arm.r+tempIndex);	//Read only from the base location
	*(arm.r+tempIndex) = address + (arm.currentInstruction & 0x00000FFF);	//Write it back into rn
	RD(arm.currentInstruction) = Memory_readByte(address);				//Read the value
	if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//load byte immediate, subtract offset, pre-indexing.
void ARM_ldrbisPre() {
	u32 address = RN(arm.currentInstruction) - (arm.currentInstruction & 0x00000FFF);
	
	if ( SUPPORT_GETBIT21(arm.currentInstruction) ) //If only write back if W is set
		RN(arm.currentInstruction) = address;
	RD(arm.currentInstruction) = Memory_readByte(address);				//Read the value
	if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//load byte immediate, subtract offset, post-indexing.
void ARM_ldrbisPost() {
	register u32 tempIndex = SUPPORT_GETRN(arm.currentInstruction);	//Get the rn register number
	u32 address = *(arm.r+tempIndex);	//Read only from the base location
	*(arm.r+tempIndex) = address - (arm.currentInstruction & 0x00000FFF);	//Write it back into rn
	RD(arm.currentInstruction) = Memory_readByte(address);				//Read the value
	if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}


///////////////////////////////////////////////////////
//strw() functions
///////////////////////////////////////////////////////

//store word, add offset, pre-indexing.
void ARM_strwaPre() {
	u32 address = RN(arm.currentInstruction) + IMMSHIFT(arm.currentInstruction);
	
	if ( SUPPORT_GETBIT21(arm.currentInstruction) ) //If only write back if W is set
		RN(arm.currentInstruction) = address;
	Memory_writeWord(address, RD(arm.currentInstruction));				//Write the value
	if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//store word, add offset, post-indexing.
void ARM_strwaPost() {
	register u32 tempIndex = SUPPORT_GETRN(arm.currentInstruction);	//Get the rn register number
	u32 address = *(arm.r+tempIndex);	//Write only from the base location
	*(arm.r+tempIndex) = address + IMMSHIFT(arm.currentInstruction);	//Write it back into rn
	Memory_writeWord(address, RD(arm.currentInstruction));				//Write the value
	if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//store word, subtract offset, pre-indexing.
void ARM_strwsPre() {
	u32 address = RN(arm.currentInstruction) - IMMSHIFT(arm.currentInstruction);
	
	if ( SUPPORT_GETBIT21(arm.currentInstruction) ) //If only write back if W is set
		RN(arm.currentInstruction) = address;
	Memory_writeWord(address, RD(arm.currentInstruction));				//Write the value
	if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//store word, subtract offset, post-indexing.
void ARM_strwsPost() {
	register u32 tempIndex = SUPPORT_GETRN(arm.currentInstruction);	//Get the rn register number
	u32 address = *(arm.r+tempIndex);	//Write only from the base location
	*(arm.r+tempIndex) = address - IMMSHIFT(arm.currentInstruction);	//Write it back into rn
	Memory_writeWord(address, RD(arm.currentInstruction));				//Write the value
	if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}


///////////////////////////////////////////////////////
//strwi() functions
///////////////////////////////////////////////////////

//store word immediate, add offset, pre-indexing.
void ARM_strwiaPre() {
	u32 address = RN(arm.currentInstruction) + (arm.currentInstruction & 0x00000FFF);
	
	if ( SUPPORT_GETBIT21(arm.currentInstruction) ) //If only write back if W is set
		RN(arm.currentInstruction) = address;
	Memory_writeWord(address, RD(arm.currentInstruction));				//Write the value
	if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//store word immediate, add offset, post-indexing.
void ARM_strwiaPost() {
	register u32 tempIndex = SUPPORT_GETRN(arm.currentInstruction);	//Get the rn register number
	u32 address = *(arm.r+tempIndex);	//Write only from the base location
	*(arm.r+tempIndex) = address + (arm.currentInstruction & 0x00000FFF);	//Write it back into rn
	Memory_writeWord(address, RD(arm.currentInstruction));				//Write the value
	if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//store word immediate, subtract offset, pre-indexing.
void ARM_strwisPre() {
	u32 address = RN(arm.currentInstruction) - (arm.currentInstruction & 0x00000FFF);
	
	if ( SUPPORT_GETBIT21(arm.currentInstruction) ) //If only write back if W is set
		RN(arm.currentInstruction) = address;
	Memory_writeWord(address, RD(arm.currentInstruction));				//Write the value
	if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//store word immediate, subtract offset, post-indexing.
void ARM_strwisPost() {
	register u32 tempIndex = SUPPORT_GETRN(arm.currentInstruction);	//Get the rn register number
	u32 address = *(arm.r+tempIndex);	//Write only from the base location
	*(arm.r+tempIndex) = address - (arm.currentInstruction & 0x00000FFF);	//Write it back into rn
	Memory_writeWord(address, RD(arm.currentInstruction));				//Write the value
	if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

///////////////////////////////////////////////////////
//strb() functions
///////////////////////////////////////////////////////

//store byte, add offset, pre-indexing.
void ARM_strbaPre() {
	u32 address = RN(arm.currentInstruction) + IMMSHIFT(arm.currentInstruction);
	
	if ( SUPPORT_GETBIT21(arm.currentInstruction) ) //If only write back if W is set
		RN(arm.currentInstruction) = address;
	Memory_writeByte(address, RD(arm.currentInstruction));				//Write the value
	if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//store byte, add offset, post-indexing.
void ARM_strbaPost() {
	register u32 tempIndex = SUPPORT_GETRN(arm.currentInstruction);	//Get the rn register number
	u32 address = *(arm.r+tempIndex);	//Write only from the base location
	*(arm.r+tempIndex) = address + IMMSHIFT(arm.currentInstruction);	//Write it back into rn
	Memory_writeByte(address, RD(arm.currentInstruction));				//Write the value
	if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//store byte, subtract offset, pre-indexing.
void ARM_strbsPre() {
	u32 address = RN(arm.currentInstruction) - IMMSHIFT(arm.currentInstruction);
	
	if ( SUPPORT_GETBIT21(arm.currentInstruction) ) //If only write back if W is set
		RN(arm.currentInstruction) = address;
	Memory_writeByte(address, RD(arm.currentInstruction));				//Write the value
	if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//store byte, subtract offset, post-indexing.
void ARM_strbsPost() {
	register u32 tempIndex = SUPPORT_GETRN(arm.currentInstruction);	//Get the rn register number
	u32 address = *(arm.r+tempIndex);	//Write only from the base location
	*(arm.r+tempIndex) = address - IMMSHIFT(arm.currentInstruction);	//Write it back into rn
	Memory_writeByte(address, RD(arm.currentInstruction));				//Write the value
	if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}


///////////////////////////////////////////////////////
//strbi() functions
///////////////////////////////////////////////////////

//store byte immediate, add offset, pre-indexing.
void ARM_strbiaPre() {
	u32 address = RN(arm.currentInstruction) + (arm.currentInstruction & 0x00000FFF);
	
	if ( SUPPORT_GETBIT21(arm.currentInstruction) ) //If only write back if W is set
		RN(arm.currentInstruction) = address;
	Memory_writeByte(address, RD(arm.currentInstruction));				//Write the value
	if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//store byte immediate, add offset, post-indexing.
void ARM_strbiaPost() {
	register u32 tempIndex = SUPPORT_GETRN(arm.currentInstruction);	//Get the rn register number
	u32 address = *(arm.r+tempIndex);	//Write only from the base location
	*(arm.r+tempIndex) = address + (arm.currentInstruction & 0x00000FFF);	//Write it back into rn
	Memory_writeByte(address, RD(arm.currentInstruction));				//Write the value
	if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//store byte immediate, subtract offset, pre-indexing.
void ARM_strbisPre() {
	u32 address = RN(arm.currentInstruction) - (arm.currentInstruction & 0x00000FFF);
	
	if ( SUPPORT_GETBIT21(arm.currentInstruction) ) //If only write back if W is set
		RN(arm.currentInstruction) = address;
	Memory_writeByte(address, RD(arm.currentInstruction));				//Write the value
	if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//store byte immediate, subtract offset, post-indexing.
void ARM_strbisPost() {
	register u32 tempIndex = SUPPORT_GETRN(arm.currentInstruction);	//Get the rn register number
	u32 address = *(arm.r+tempIndex);	//Write only from the base location
	*(arm.r+tempIndex) = address - (arm.currentInstruction & 0x00000FFF);	//Write it back into rn
	Memory_writeByte(address, RD(arm.currentInstruction));				// the value
	if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}








////////////////////////////////////////////////////////////
//HALFWORD LOADS AND STORES, PLUS SIGNED BYTE AND HALFWORD LOADS
//NOTE FROM ABOUT THESE:
//Although the ARM docs don't explicitly say that r15 sholdn't be
//the destination register, there aren't many situations where a load
//into this register would be useful, so I'm not going to check on it.
//The only it could ever be useful is if you knew the address was in
//zero memory (i.e. top 16 bits are all zeros), 
////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////
//ldrh() functions
///////////////////////////////////////////////////////

//load HalfWord, add offset, pre-indexing.
void ARM_ldrhaPre() {
	u32 address = RN(arm.currentInstruction) + RM(arm.currentInstruction);
	
	if ( SUPPORT_GETBIT21(arm.currentInstruction) ) //If only write back if W is set
		RN(arm.currentInstruction) = address;
	RD(arm.currentInstruction) = Memory_readHalfWord (address);				//Read the value
	////if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
	ARM_nextInstruction();
}

//load HalfWord, add offset, post-indexing.
void ARM_ldrhaPost() {
	register u32 tempIndex = SUPPORT_GETRN(arm.currentInstruction);	//Get the rn register number
	u32 address = *(arm.r+tempIndex);	//Read only from the base location
	*(arm.r+tempIndex) = address + RM(arm.currentInstruction);	//Write it back into rn
	RD(arm.currentInstruction) = Memory_readHalfWord (address);				//Read the value
	//if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
	ARM_nextInstruction();
}

//load HalfWord, subtract offset, pre-indexing.
void ARM_ldrhsPre() {
	u32 address = RN(arm.currentInstruction) - RM(arm.currentInstruction);
	
	if ( SUPPORT_GETBIT21(arm.currentInstruction) ) //If only write back if W is set
		RN(arm.currentInstruction) = address;
	RD(arm.currentInstruction) = Memory_readHalfWord (address);				//Read the value
	//if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
	ARM_nextInstruction();
}

//load HalfWord, subtract offset, post-indexing.
void ARM_ldrhsPost() {
	register u32 tempIndex = SUPPORT_GETRN(arm.currentInstruction);	//Get the rn register number
	u32 address = *(arm.r+tempIndex);	//Read only from the base location
	*(arm.r+tempIndex) = address - RM(arm.currentInstruction);	//Write it back into rn
	RD(arm.currentInstruction) = Memory_readHalfWord (address);				//Read the value
	//if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
	ARM_nextInstruction();
}


///////////////////////////////////////////////////////
//ldrhi() functions
///////////////////////////////////////////////////////

//load HalfWord immediate, add offset, pre-indexing.
void ARM_ldrhiaPre() {
	u32 address = RN(arm.currentInstruction) 
		+ ((arm.currentInstruction & 0x0000000F) | ((arm.currentInstruction >> 4) & 0x000000F0));
	
	if ( SUPPORT_GETBIT21(arm.currentInstruction) ) //If only write back if W is set
		RN(arm.currentInstruction) = address;

	RD(arm.currentInstruction) = Memory_readHalfWord (address);				//Read the value
	//if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
	ARM_nextInstruction();
}

//load HalfWord immediate, add offset, post-indexing.
void ARM_ldrhiaPost() {
	register u32 tempIndex = SUPPORT_GETRN(arm.currentInstruction);	//Get the rn register number
	u32 address = *(arm.r+tempIndex);	//Read only from the base location
	*(arm.r+tempIndex) = address + ((arm.currentInstruction & 0x0000000F) | ((arm.currentInstruction >> 4) & 0x000000F0));	//Write it back into rn
	RD(arm.currentInstruction) = Memory_readHalfWord (address);				//Read the value
	//if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
	ARM_nextInstruction();
}

//load HalfWord immediate, subtract offset, pre-indexing.
void ARM_ldrhisPre() {
	u32 address = RN(arm.currentInstruction) - ((arm.currentInstruction & 0x0000000F) | ((arm.currentInstruction >> 4) & 0x000000F0));
	
	if ( SUPPORT_GETBIT21(arm.currentInstruction) ) //If only write back if W is set
		RN(arm.currentInstruction) = address;
	RD(arm.currentInstruction) = Memory_readHalfWord (address);				//Read the value
	//if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
	ARM_nextInstruction();
}

//load HalfWord immediate, subtract offset, post-indexing.
void ARM_ldrhisPost() {
	register u32 tempIndex = SUPPORT_GETRN(arm.currentInstruction);	//Get the rn register number
	u32 address = *(arm.r+tempIndex);	//Read only from the base location
	*(arm.r+tempIndex) = address - ((arm.currentInstruction & 0x0000000F) | ((arm.currentInstruction >> 4) & 0x000000F0));	//Write it back into rn
	RD(arm.currentInstruction) = Memory_readHalfWord (address);				//Read the value
	//if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
	ARM_nextInstruction();
}

///////////////////////////////////////////////////////
//ldrsb() functions
///////////////////////////////////////////////////////

//load signed byte, add offset, pre-indexing.
void ARM_ldrsbaPre() {
	u32 address = RN(arm.currentInstruction) + RM(arm.currentInstruction);
	
	if ( SUPPORT_GETBIT21(arm.currentInstruction) ) //If only write back if W is set
		RN(arm.currentInstruction) = address;
	register s32 signedValue = Memory_readWord (address) & 0x000000FF;				//Read the value
	signedValue = signedValue << 24;							//Shift all the way left
	signedValue = signedValue >> 24;							//Shift back, with sign.
	RD(arm.currentInstruction) = signedValue;
	//if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
	ARM_nextInstruction();
}

//load signed byte, add offset, post-indexing.
void ARM_ldrsbaPost() {
	register u32 tempIndex = SUPPORT_GETRN(arm.currentInstruction);	//Get the rn register number
	u32 address = *(arm.r+tempIndex);	//Read only from the base location
	*(arm.r+tempIndex) = address + RM(arm.currentInstruction);	//Write it back into rn
	register s32 signedValue = Memory_readWord (address) & 0x000000FF;				//Read the value
	signedValue = signedValue << 24;							//Shift all the way left
	signedValue = signedValue >> 24;							//Shift back, with sign.
	RD(arm.currentInstruction) = signedValue;
	//if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
	ARM_nextInstruction();
}

//load signed byte, subtract offset, pre-indexing.
void ARM_ldrsbsPre() {
	u32 address = RN(arm.currentInstruction) - RM(arm.currentInstruction);
	
	if ( SUPPORT_GETBIT21(arm.currentInstruction) ) //If only write back if W is set
		RN(arm.currentInstruction) = address;
	register s32 signedValue = Memory_readWord (address) & 0x000000FF;				//Read the value
	signedValue = signedValue << 24;							//Shift all the way left
	signedValue = signedValue >> 24;							//Shift back, with sign.
	RD(arm.currentInstruction) = signedValue;
	//if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
	ARM_nextInstruction();
}

//load signed byte, subtract offset, post-indexing.
void ARM_ldrsbsPost() {
	register u32 tempIndex = SUPPORT_GETRN(arm.currentInstruction);	//Get the rn register number
	u32 address = *(arm.r+tempIndex);	//Read only from the base location
	*(arm.r+tempIndex) = address - RM(arm.currentInstruction);	//Write it back into rn
	register s32 signedValue = Memory_readWord (address) & 0x000000FF;				//Read the value
	signedValue = signedValue << 24;							//Shift all the way left
	signedValue = signedValue >> 24;							//Shift back, with sign.
	RD(arm.currentInstruction) = signedValue;
	//if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
	ARM_nextInstruction();
}


///////////////////////////////////////////////////////
//ldrsbi() functions
///////////////////////////////////////////////////////

//load signed byte immediate, add offset, pre-indexing.
void ARM_ldrsbiaPre() {
	u32 address = RN(arm.currentInstruction) + ((arm.currentInstruction & 0x0000000F) | ((arm.currentInstruction >> 4) & 0x000000F0));
	
	if ( SUPPORT_GETBIT21(arm.currentInstruction) ) //If only write back if W is set
		RN(arm.currentInstruction) = address;
	register s32 signedValue = Memory_readWord (address) & 0x000000FF;				//Read the value
	signedValue = signedValue << 24;							//Shift all the way left
	signedValue = signedValue >> 24;							//Shift back, with sign.
	RD(arm.currentInstruction) = signedValue;
	//if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
	ARM_nextInstruction();
}

//load signed byte immediate, add offset, post-indexing.
void ARM_ldrsbiaPost() {
	register u32 tempIndex = SUPPORT_GETRN(arm.currentInstruction);	//Get the rn register number
	u32 address = *(arm.r+tempIndex);	//Read only from the base location
	*(arm.r+tempIndex) = address + ((arm.currentInstruction & 0x0000000F) | ((arm.currentInstruction >> 4) & 0x000000F0));	//Write it back into rn
	register s32 signedValue = Memory_readWord (address) & 0x000000FF;				//Read the value
	signedValue = signedValue << 24;							//Shift all the way left
	signedValue = signedValue >> 24;							//Shift back, with sign.
	RD(arm.currentInstruction) = signedValue;
	//if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
	ARM_nextInstruction();
}

//load signed byte immediate, subtract offset, pre-indexing.
void ARM_ldrsbisPre() {
	u32 address = RN(arm.currentInstruction) - ((arm.currentInstruction & 0x0000000F) | ((arm.currentInstruction >> 4) & 0x000000F0));
	
	if ( SUPPORT_GETBIT21(arm.currentInstruction) ) //If only write back if W is set
		RN(arm.currentInstruction) = address;
	register s32 signedValue = Memory_readWord (address) & 0x000000FF;				//Read the value
	signedValue = signedValue << 24;							//Shift all the way left
	signedValue = signedValue >> 24;							//Shift back, with sign.
	RD(arm.currentInstruction) = signedValue;
	//if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
	ARM_nextInstruction();
}

//load signed byte immediate, subtract offset, post-indexing.
void ARM_ldrsbisPost() {
	register u32 tempIndex = SUPPORT_GETRN(arm.currentInstruction);	//Get the rn register number
	u32 address = *(arm.r+tempIndex);	//Read only from the base location
	*(arm.r+tempIndex) = address - ((arm.currentInstruction & 0x0000000F) | ((arm.currentInstruction >> 4) & 0x000000F0));	//Write it back into rn
	register s32 signedValue = Memory_readWord (address) & 0x000000FF;				//Read the value
	signedValue = signedValue << 24;							//Shift all the way left
	signedValue = signedValue >> 24;							//Shift back, with sign.
	RD(arm.currentInstruction) = signedValue;
	//if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
	ARM_nextInstruction();
}

///////////////////////////////////////////////////////
//ldrsh() functions
///////////////////////////////////////////////////////

//load signed HalfWord, add offset, pre-indexing.
void ARM_ldrshaPre() {
	u32 address = RN(arm.currentInstruction) + RM(arm.currentInstruction);
	
	if ( SUPPORT_GETBIT21(arm.currentInstruction) ) //If only write back if W is set
		RN(arm.currentInstruction) = address;
	register s32 signedValue = Memory_readHalfWord (address);				//Read the value
	signedValue = signedValue << 16;							//Shift all the way left
	signedValue = signedValue >> 16;							//Shift back, with sign.
	RD(arm.currentInstruction) = signedValue;
	//if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
	ARM_nextInstruction();
}

//load signed HalfWord, add offset, post-indexing.
void ARM_ldrshaPost() {
	register u32 tempIndex = SUPPORT_GETRN(arm.currentInstruction);	//Get the rn register number
	u32 address = *(arm.r+tempIndex);	//Read only from the base location
	*(arm.r+tempIndex) = address + RM(arm.currentInstruction);	//Write it back into rn
	register s32 signedValue = Memory_readHalfWord (address);				//Read the value
	signedValue = signedValue << 16;							//Shift all the way left
	signedValue = signedValue >> 16;							//Shift back, with sign.
	RD(arm.currentInstruction) = signedValue;
	//if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
	ARM_nextInstruction();
}

//load signed HalfWord, subtract offset, pre-indexing.
void ARM_ldrshsPre() {
	u32 address = RN(arm.currentInstruction) - RM(arm.currentInstruction);
	
	if ( SUPPORT_GETBIT21(arm.currentInstruction) ) //If only write back if W is set
		RN(arm.currentInstruction) = address;
	register s32 signedValue = Memory_readHalfWord (address);				//Read the value
	signedValue = signedValue << 16;							//Shift all the way left
	signedValue = signedValue >> 16;							//Shift back, with sign.
	RD(arm.currentInstruction) = signedValue;
	//if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
	ARM_nextInstruction();
}

//load signed HalfWord, subtract offset, post-indexing.
void ARM_ldrshsPost() {
	register u32 tempIndex = SUPPORT_GETRN(arm.currentInstruction);	//Get the rn register number
	u32 address = *(arm.r+tempIndex);	//Read only from the base location
	*(arm.r+tempIndex) = address - RM(arm.currentInstruction);	//Write it back into rn
	register s32 signedValue = Memory_readHalfWord (address);				//Read the value
	signedValue = signedValue << 16;							//Shift all the way left
	signedValue = signedValue >> 16;							//Shift back, with sign.
	RD(arm.currentInstruction) = signedValue;
	//if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
	ARM_nextInstruction();
}


///////////////////////////////////////////////////////
//ldrshi() functions
///////////////////////////////////////////////////////

//load signed HalfWord immediate, add offset, pre-indexing.
void ARM_ldrshiaPre() {
	u32 address = RN(arm.currentInstruction) + ((arm.currentInstruction & 0x0000000F) | ((arm.currentInstruction >> 4) & 0x000000F0));
	
	if ( SUPPORT_GETBIT21(arm.currentInstruction) ) //If only write back if W is set
		RN(arm.currentInstruction) = address;
	register s32 signedValue = Memory_readHalfWord (address);				//Read the value
	signedValue = signedValue << 16;							//Shift all the way left
	signedValue = signedValue >> 16;							//Shift back, with sign.
	RD(arm.currentInstruction) = signedValue;
	//if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
	ARM_nextInstruction();
}

//load signed HalfWord immediate, add offset, post-indexing.
void ARM_ldrshiaPost() {
	register u32 tempIndex = SUPPORT_GETRN(arm.currentInstruction);	//Get the rn register number
	u32 address = *(arm.r+tempIndex);	//Read only from the base location
	*(arm.r+tempIndex) = address + ((arm.currentInstruction & 0x0000000F) | ((arm.currentInstruction >> 4) & 0x000000F0));	//Write it back into rn
	register s32 signedValue = Memory_readHalfWord (address);				//Read the value
	signedValue = signedValue << 16;							//Shift all the way left
	signedValue = signedValue >> 16;							//Shift back, with sign.
	RD(arm.currentInstruction) = signedValue;
	//if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
	ARM_nextInstruction();
}

//load signed HalfWord immediate, subtract offset, pre-indexing.
void ARM_ldrshisPre() {
	u32 address = RN(arm.currentInstruction) - ((arm.currentInstruction & 0x0000000F) | ((arm.currentInstruction >> 4) & 0x000000F0));
	
	if ( SUPPORT_GETBIT21(arm.currentInstruction) ) //If only write back if W is set
		RN(arm.currentInstruction) = address;
	register s32 signedValue = Memory_readHalfWord (address);				//Read the value
	signedValue = signedValue << 16;							//Shift all the way left
	signedValue = signedValue >> 16;							//Shift back, with sign.
	RD(arm.currentInstruction) = signedValue;
	//if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
	ARM_nextInstruction();
}

//load signed HalfWord immediate, subtract offset, post-indexing.
void ARM_ldrshisPost() {
	register u32 tempIndex = SUPPORT_GETRN(arm.currentInstruction);	//Get the rn register number
	u32 address = *(arm.r+tempIndex);	//Read only from the base location
	*(arm.r+tempIndex) = address - ((arm.currentInstruction & 0x0000000F) | ((arm.currentInstruction >> 4) & 0x000000F0));	//Write it back into rn
	register s32 signedValue = Memory_readHalfWord (address);				//Read the value
	signedValue = signedValue << 16;							//Shift all the way left
	signedValue = signedValue >> 16;							//Shift back, with sign.
	RD(arm.currentInstruction) = signedValue;
	//if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
	ARM_nextInstruction();
}


///////////////////////////////////////////////////////
//strh() functions
///////////////////////////////////////////////////////

//store HalfWord, add offset, pre-indexing.
void ARM_strhaPre() {
	u32 address = RN(arm.currentInstruction) + RM(arm.currentInstruction);
	
	if ( SUPPORT_GETBIT21(arm.currentInstruction) ) //If only write back if W is set
		RN(arm.currentInstruction) = address;
	Memory_writeHalfWord (address, RD(arm.currentInstruction));				//Write the value
	//if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
	ARM_nextInstruction();
}

//store HalfWord, add offset, post-indexing.
void ARM_strhaPost() {
	register u32 tempIndex = SUPPORT_GETRN(arm.currentInstruction);	//Get the rn register number
	u32 address = *(arm.r+tempIndex);	//Write only from the base location
	*(arm.r+tempIndex) = address + RM(arm.currentInstruction);	//Write it back into rn
	Memory_writeHalfWord (address, RD(arm.currentInstruction));				//Write the value
	//if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
	ARM_nextInstruction();
}

//store HalfWord, subtract offset, pre-indexing.
void ARM_strhsPre() {
	u32 address = RN(arm.currentInstruction) - RM(arm.currentInstruction);
	
	if ( SUPPORT_GETBIT21(arm.currentInstruction) ) //If only write back if W is set
		RN(arm.currentInstruction) = address;
	Memory_writeHalfWord (address, RD(arm.currentInstruction));				//Write the value
	//if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
	ARM_nextInstruction();
}

//store HalfWord, subtract offset, post-indexing.
void ARM_strhsPost() {
	register u32 tempIndex = SUPPORT_GETRN(arm.currentInstruction);	//Get the rn register number
	u32 address = *(arm.r+tempIndex);	//Write only from the base location
	*(arm.r+tempIndex) = address - RM(arm.currentInstruction);	//Write it back into rn
	Memory_writeHalfWord (address, RD(arm.currentInstruction));				//Write the value
	//if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
	ARM_nextInstruction();
}


///////////////////////////////////////////////////////
//strhi() functions
///////////////////////////////////////////////////////

//store HalfWord immediate, add offset, pre-indexing.
void ARM_strhiaPre() {
	u32 address = RN(arm.currentInstruction) + ((arm.currentInstruction & 0x0000000F) | ((arm.currentInstruction >> 4) & 0x000000F0));
	
	if ( SUPPORT_GETBIT21(arm.currentInstruction) ) //If only write back if W is set
		RN(arm.currentInstruction) = address;
	Memory_writeHalfWord (address, RD(arm.currentInstruction));				//Write the value
	//if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
	ARM_nextInstruction();
}

//store HalfWord immediate, add offset, post-indexing.
void ARM_strhiaPost() {
	register u32 tempIndex = SUPPORT_GETRN(arm.currentInstruction);	//Get the rn register number
	u32 address = *(arm.r+tempIndex);	//Write only from the base location
	*(arm.r+tempIndex) = address + ((arm.currentInstruction & 0x0000000F) | ((arm.currentInstruction >> 4) & 0x000000F0));	//Write it back into rn
	Memory_writeHalfWord (address, RD(arm.currentInstruction));				//Write the value
	//if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
	ARM_nextInstruction();
}

//store HalfWord immediate, subtract offset, pre-indexing.
void ARM_strhisPre() {
	u32 address = RN(arm.currentInstruction) - ((arm.currentInstruction & 0x0000000F) | ((arm.currentInstruction >> 4) & 0x000000F0));
	
	if ( SUPPORT_GETBIT21(arm.currentInstruction) ) //If only write back if W is set
		RN(arm.currentInstruction) = address;
	Memory_writeHalfWord (address, RD(arm.currentInstruction));				//Write the value
	//if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
	ARM_nextInstruction();
}

//store HalfWord immediate, subtract offset, post-indexing.
void ARM_strhisPost() {
	register u32 tempIndex = SUPPORT_GETRN(arm.currentInstruction);	//Get the rn register number
	u32 address = *(arm.r+tempIndex);	//Write only from the base location
	*(arm.r+tempIndex) = address - ((arm.currentInstruction & 0x0000000F) | ((arm.currentInstruction >> 4) & 0x000000F0));	//Write it back into rn
	Memory_writeHalfWord (address, RD(arm.currentInstruction));				//Write the value
	//if ( SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
	ARM_nextInstruction();
}

///////////////////////////////////////////////////////////////////
//Swap two values.  Thank god it's much more simple than regular loads
//and stores.
////////////////////////////////////////////////////////////////////////
void ARM_swap () {
	u32 value;
	
	//Get the opcode and the operands
	arm.rn = SUPPORT_GETRN(arm.currentInstruction);
	arm.rd = SUPPORT_GETRD(arm.currentInstruction);
	arm.rm = SUPPORT_GETRM(arm.currentInstruction);


	u32 address = arm.r[arm.rn];
	
	//clockTick();

	if ( SUPPORT_GETBIT22(arm.currentInstruction) ) {	//If this is a byte swap
		value = Memory_readWord (address) & 0x000000FF;
		Memory_writeByte(address, (u8) arm.r[arm.rm] );
		arm.r[arm.rm] = value;
	} else {									//Otherwise it's a word swap
		value = Memory_readWord (address);
		Memory_writeWord(address,  arm.r[arm.rm] );
		arm.r[arm.rm] = value;
	}	
	
	//Note how the pre-ARM_fetch occurs after evaluation of the operands --
	//otherwise, if one of the ops was r15, we'd have the wrong value
	//(That is, I think this also applies to stores and loads, anyway . . .)
	ARM_nextInstruction();

	//ARM_fetch();	//Pre-ARM_fetch an ARM_instruction into IQ[0]

	//decode();	//(does nothing)

	
	//arm.currentInstruction = ARM_instruction2;
	//ARM_instruction2 = ARM_instruction1;
	//ARM_instruction1 = 0;
	//clockTick();
}

//This covers data operations.
//xxxx011P UBWLnnnn ddddcccc ctt0mmmm  Register foarm.rm
//xxxx010P UBWLnnnn ddddoooo oooooooo  Immediate foarm.rm

//xxxx000a aaaSnnnn ddddcccc ctttmmmm  Register foarm.rm
//xxxx001a aaaSnnnn ddddrrrr bbbbbbbb  Immediate foarm.rm
void ARM_loadStoreMultiple() {
	u32 value;
	u32 regList;
	int offset;
	u32 instruction = arm.currentInstruction;	//We must save the instruction because instruction3 could change
										//if we refill the queue (duh?)
		
	//Get the opcode and the operands
	arm.rn = SUPPORT_GETRN(instruction);//Support::getRn (instruction);
	
	u32 address = *(arm.r+arm.rn); //[arm.rn];

	//if ( SUPPORT_GETBIT20(instruction)  ) 	
	//address &= 0xFFFFFFFC;	

	//If the U bit is set
	if ( SUPPORT_GETBIT23(instruction) ) //Support::getBit (instruction, 23) )		
		offset = 4;
	else
		offset = -4;

	//Get the register list
	regList = instruction & 0x0000FFFF;

	for (u32 j = 0; j < 16; j++) {
		u32 i;
	
		//The lowest numbered register is always stored at the lowest address...
		//makes it awkward to code. :(
		if (offset < 0)
			i = 15 - j;
		else
			i = j;
		
		if ( (regList >> i) & 0x1) {	//If bit "i" is set
			
			//Pre indexing
			if ( SUPPORT_GETBIT24(instruction) ) {//Support::getBit (instruction, 24) ) {		//If P is set (pre-index
				address += offset;
				if ( SUPPORT_GETBIT21(instruction) ) //If only write back if W is set
					arm.r[arm.rn] = address;

				//Yes, I know that this is inneficient!!!
				if ( SUPPORT_GETBIT20(instruction)  ) {	//If this is a load
					
					value = Memory_readWord (address & 0xFFFFFFFC);				//Read the value
					*(arm.r+i) = value;

					//If the destination was r15, refill queue
					if (i == 15)
						ARM_nextInstruction();

					//If this is a store
				} else {
					Memory_writeWord (  address, *(arm.r+i)  );
				}
					

			} else {	//If P is not set, post-index 
	
				//Yes, I know that this is inneficient!!!
				if ( SUPPORT_GETBIT20(instruction)  ) {	//If this is a load
					value = Memory_readWord (address & 0xFFFFFFFC);				//Read the value
					*(arm.r+i) = value;
					
					//If the destination was r15, refill queue
					if (i == 15)
						ARM_nextInstruction();
					
					
					//If this is a store
				} else {
					Memory_writeWord (address,  *(arm.r+i)  );
				}
				address += offset;
				if ( SUPPORT_GETBIT21(instruction) ) //If only write back if W is set
					*(arm.r+arm.rn) = address;
			}
		}//End of if
	}//End of for

	//TODO:  Operation differs slightly when S bit is set.
	
	//Note how the pre-ARM_fetch occurs after evaluation of the operands --
	//otherwise, if one of the ops was r15, we'd have the wrong value
	//(That is, I think this also applies to stores and loads, anyway . . .)
	ARM_nextInstruction();

	//ARM_fetch();	//Pre-ARM_fetch an ARM_instruction into IQ[0]

	//decode();	//(does nothing)
	//arm.currentInstruction = ARM_instruction2;
	//ARM_instruction2 = ARM_instruction1;
	//ARM_instruction1 = 0;
	//clockTick();
}
