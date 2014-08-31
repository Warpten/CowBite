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
//#include "MemoryC.h"
#include "Memory.h"
#include "IO.h"
#include "GBA.h"
#include "stdio.h"

GBAMem gbaMem;


/////////////////////////////////////
//Memory_init()
//Initializes all memory for the GBA
////////////////////////////////////////
void Memory_init() {
	
	//Initialize the memory size array to 0
	memset(gbaMem.memorySize,0,sizeof(gbaMem.memorySize));

	//Allocate memory
//	gbaMem.u8BIOS = new u8 [ZERO_PAGE_SIZE];	
	gbaMem.u16BIOS = (u16*)gbaMem.u8BIOS;
	gbaMem.u32BIOS = (u32*)gbaMem.u8BIOS;
	gbaMem.u8Mem[0] = gbaMem.u8BIOS;			//This gives us one place to access
	gbaMem.memorySize[0] = ZERO_PAGE_SIZE;		//all memory via index

	
//	gbaMem.u8WRAMExt = new u8 [WRAM_EXT_SIZE];
	gbaMem.u16WRAMExt = (u16*) gbaMem.u8WRAMExt;
	gbaMem.u32WRAMExt = (u32*) gbaMem.u8WRAMExt;
	gbaMem.u8Mem[2] = gbaMem.u8WRAMExt;
	gbaMem.memorySize[2] = WRAM_EXT_SIZE;

//	gbaMem.u8WRAMInt = new u8 [WRAM_INT_SIZE];
	gbaMem.u16WRAMInt = (u16*) gbaMem.u8WRAMInt;
	gbaMem.u32WRAMInt = (u32*) gbaMem.u8WRAMInt;
	gbaMem.u8Mem[3] = gbaMem.u8WRAMInt;
	gbaMem.memorySize[3] = WRAM_INT_SIZE;

//	gbaMem.u8IORAM = new u8[IORAM_SIZE];
	gbaMem.u16IORAM = (u16*) gbaMem.u8IORAM;
	gbaMem.u32IORAM = (u32*) gbaMem.u8IORAM;
	gbaMem.u8Mem[4] = gbaMem.u8IORAM;
	gbaMem.memorySize[4] = IORAM_SIZE;

//	gbaMem.u8Pal = new u8 [PAL_SIZE];
	gbaMem.u16Pal = (u16*) gbaMem.u8Pal;
	gbaMem.u32Pal = (u32*) gbaMem.u8Pal;
	gbaMem.u8Mem[5] = gbaMem.u8Pal;
	gbaMem.memorySize[5] = PAL_SIZE;

//	winPalette8bit = new u8 [PAL_SIZE];
//	winPalette16bit = (u16*) winPalette8bit;
//	winPalette32bit = (u32*) winPalette8bit;

//	gbaMem.u8VRAM = new u8 [VRAM_SIZE];
	gbaMem.u16VRAM = (u16*) gbaMem.u8VRAM;
	gbaMem.u32VRAM = (u32*) gbaMem.u8VRAM;
	gbaMem.u8Mem[6] = gbaMem.u8VRAM;
	gbaMem.memorySize[6] = VRAM_SIZE;
	
//	gbaMem.u8OAM = new u8[OAM_SIZE];
	gbaMem.u16OAM = (u16*) gbaMem.u8OAM; 
	gbaMem.u32OAM = (u32*) gbaMem.u8OAM;
	gbaMem.u8Mem[7] = gbaMem.u8OAM;
	gbaMem.memorySize[7] = OAM_SIZE;
	
//	gbaMem.u8CartRAM = new u8[CARTRAM_SIZE];
	gbaMem.u16CartRAM = (u16*) gbaMem.u8CartRAM;
	gbaMem.u32CartRAM = (u32*) gbaMem.u8CartRAM;
	gbaMem.u8Mem[0xE] = gbaMem.u8CartRAM;
	gbaMem.memorySize[0xE] = CARTRAM_SIZE;
	
	//Scratch memory for whatever is needed
	//Note that I DO actually use this!! (in sprite flipping, for one)
	gbaMem.scratchMem = new u8 [VRAM_SIZE];	
	gbaMem.u8Mem[1] = gbaMem.scratchMem;


	//Make memory accessible as
	//16 and 32 bit
	gbaMem.u16Mem = (u16**) gbaMem.u8Mem;
	gbaMem.u32Mem = (u32**) gbaMem.u8Mem;

	//Fill up our table of function pointers.
	for (int i = 0; i < 0x100; i++) {
		gbaMem.writeByteAccessTable[i] = Memory_writeByteStandardMemory;
		gbaMem.writeHalfWordAccessTable[i] = Memory_writeHalfWordStandardMemory;
		gbaMem.writeWordAccessTable[i] = Memory_writeWordStandardMemory;
	}
	gbaMem.writeByteAccessTable[0] = Memory_writeByteBIOS;
	gbaMem.writeByteAccessTable[4] = Memory_writeByteIORAM;
	gbaMem.writeByteAccessTable[8] = Memory_writeByteROM0;
	gbaMem.writeByteAccessTable[9] = Memory_writeByteROM0;
	gbaMem.writeByteAccessTable[0xA] = Memory_writeByteROM1;
	gbaMem.writeByteAccessTable[0xB] = Memory_writeByteROM1;
	gbaMem.writeByteAccessTable[0xC] = Memory_writeByteROM2;
	gbaMem.writeByteAccessTable[0xD] = Memory_writeByteROM2;

	gbaMem.writeHalfWordAccessTable[0] = Memory_writeHalfWordBIOS;
	gbaMem.writeHalfWordAccessTable[4] = Memory_writeHalfWordIORAM;
	gbaMem.writeHalfWordAccessTable[8] = Memory_writeHalfWordROM0;
	gbaMem.writeHalfWordAccessTable[9] = Memory_writeHalfWordROM0;
	gbaMem.writeHalfWordAccessTable[0xA] = Memory_writeHalfWordROM1;
	gbaMem.writeHalfWordAccessTable[0xB] = Memory_writeHalfWordROM1;
	gbaMem.writeHalfWordAccessTable[0xC] = Memory_writeHalfWordROM2;
	gbaMem.writeHalfWordAccessTable[0xD] = Memory_writeHalfWordROM2;

	gbaMem.writeWordAccessTable[0] = Memory_writeWordBIOS;
	gbaMem.writeWordAccessTable[4] = Memory_writeWordIORAM;
	gbaMem.writeWordAccessTable[8] = Memory_writeWordROM0;
	gbaMem.writeWordAccessTable[9] = Memory_writeWordROM0;
	gbaMem.writeWordAccessTable[0xA] = Memory_writeWordROM1;
	gbaMem.writeWordAccessTable[0xB] = Memory_writeWordROM1;
	gbaMem.writeWordAccessTable[0xC] = Memory_writeWordROM2;
	gbaMem.writeWordAccessTable[0xD] = Memory_writeWordROM2;
}

///////////////////////////////////////
//Memory_delete()
//frees up memory.
////////////////////////////////////////
void Memory_delete() {
	delete [] gbaMem.u8ROM;
//	delete [] gbaMem.u8BIOS;
//	delete [] gbaMem.u8WRAMExt;
//	delete [] gbaMem.u8WRAMInt;
//	delete [] gbaMem.u8VRAM;
//	delete [] gbaMem.u8Pal;
//	delete [] winPalette8bit;
//	delete [] gbaMem.u8IORAM;
//	delete [] gbaMem.u8OAM;
//	delete [] gbaMem.u8CartRAM;
	delete [] gbaMem.scratchMem;
}

//This reads from memory
__inline u32 Memory_readWord (u32 address) {
	//Get the first byte of the arm.location (should be
	//0 - F -- if somehow nintendo
	//decides to make ROMs with addrsses higer than that,
	//this will break.
	register u32 value, offset;
	
	//Calculate the destination using an index
	register index = address >> 24;
	offset = address & 0x00FFFFFF;	//Chop off the first byte

	if (offset >= gbaMem.memorySize[index]) {
		//If the reading out of bounds, make note of it
		//for our statistics.
		gba.stats.readWordXBounds[index]++;
		gba.stats.lastReadWordXBounds = arm.location = address;
		return 0;
	}
		

	register u32* dest = *(gbaMem.u32Mem + index);
	register u32 rotAmt;	//Amount to rotate non-aligned reads.

	offset = address & 0x00FFFFFF;	//Chop off the first byte
	rotAmt = offset & 0x00000003;	//Amount to rotate non-alinged values

	//NOTE:  because some demos use "non aligned reads" we have to 
	//slow this up somewhat by checking if it's aligned or not.
	if ( ! rotAmt )	{ //Check if the instruction is aligned
									  //by making sure the last two bits are all zeros
		offset = offset >> 2;	//Convert to viable index

		//Use some pointer arithmetic
		//to retrieve from memory
		value = (u32)*(( u32*)(dest + offset));
		arm.location = address;
	} else {	//Note how evil and slow this is.
		offset = offset >> 2;
		value = (u32)*((u32*)(dest + offset));
		rotAmt = rotAmt << 3; //2 << rotAmt;	//Multiply the rotate amount by 8
		
		//Rotate it so the part of the word they want is in the reg.
		value = (value << (32 - rotAmt)) | (value >> rotAmt);
		//value |= 0xFF000000;	//DEBUG
		//NOTE TO SELF:  If I ever modify this code to have a separate read function
		//for each area of memory, we will have to do our alignment *before* that
		arm.location = address = address & 0xFFFFFFFC;
	}
	if (index == 4)
		IO_updateReg(address);
	return value;
}

//This reads from memory
inline  u16 Memory_readHalfWord (u32 address) {
	//Get the first byte of the arm.location (should be
	//0, 1, 2, 3, 4, 5, 6, 7, or 8 -- if somehow nintendo
	//decides to make ROMs with addrsses higer than 0x08FFFFFF,
	//this will break.
	u16 value;
	u32 offset;
	u32 deleteme;
	int index = address >> 24;
	u16* dest = gbaMem.u16Mem[index];
	
	offset = address & 0x00FFFFFF;	//Chop off the first byte

	//if (gba.stats.lastReadHWXBounds == 0x1A8C)
	//	int blah = 0;

	if (offset >= gbaMem.memorySize[index]) {
		//If the reading out of bounds, make note of it
		//for our statistics.
		gba.stats.readHWXBounds[index]++;
		gba.stats.lastReadHWXBounds = arm.location = address;
		
		return 0;
	}

	//According to the ARM docs, the behavior of unaligned halfword reads is
	//unpredictable, but gbcft has discovered that it performs a 32 bit rotate.
	//I would have to modify this to return a 32 bit number in order to emulate taht.
	offset = offset >> 1;	//Convert to viable index

	//Check for update of register contents (especially in the case that
	//this is a timer register)
	//NOTE TO SELF:  If we ever modify this code to have a separate read function
	//for each area of memory, we will have to do our alignment *before* that
	arm.location = address = address & 0xFFFFFFFE;
	if (index == 4)
		IO_updateReg(address);

	

	//Use some pointer arithmetic
	//to retrieve from memory
	value = (u16)*(( u16*)(dest + offset));
	//if (deleteme)
	//	value = deleteme;
	

	return value;
}

//Need to write a new readByte function, since I now have it so that readword no longer reads on a boundary
inline u8 Memory_readByte(u32 address) {
	register u32 value, offset;
	
	//Calculate the destination using an index
	register index = address >> 24;
	offset = address & 0x00FFFFFF;	//Chop off the first byte

	if (offset >= gbaMem.memorySize[index]) {
		//If the reading out of bounds, make note of it
		//for our statistics.
		gba.stats.readWordXBounds[index]++;
		gba.stats.lastReadWordXBounds = arm.location = address;
		return 0;
	}
		

	register u8* dest = gbaMem.u8Mem[index];
	arm.location = address;
	if (index == 4)
		IO_updateReg(address);
	value = dest[offset];
	return value;

	//return (Memory_readWord() & 0xFF);
}

//This stores a word to memory
inline void Memory_writeWord (u32 address, u32 data) {
	//Get the first byte of the arm.location 
	int index = address >> 24;
	u32* dest = *(gbaMem.u32Mem+index);
	//u32 offset = arm.location & 0x00FFFFFF;	//Chop off the first byte
	u32 offset = address & 0x00FFFFFC;	//Chop off the first byte and align to word

	if (offset >= gbaMem.memorySize[index]) {
		//If the reading out of bounds, make note of it
		//for our statistics.
		gba.stats.writeWordXBounds[index]++;
		gba.stats.lastWriteWordXBounds = arm.location = address;
		return;
	}
	//offset = offset >> instructionWidth;	//Should I get rid of "instructionWidth" in favor of '2'?

	//Use some pointer arithmetic
	//to retrieve from our array.
	
	//According to ARM docs, "The word presented to the data bus is unaffected
	//if the address is not word aligned."  
	//I used to think this meant we didn't change anything, but now I think it means "write to
	//the word alligned address anyway" (I already clipped it off, above)
	offset = offset >> 2;	//Convert to viable index

	//Use some pointer arithmetic
	//to retrieve from memory
	dest = ( u32*)(dest + offset);

//	if ((arm.location >= 0x03007510) && (arm.location <= 0x03007520))
//		int blah = 0;

	//NOTE:  We must align arm.location before passing it into the memory access
	//functions, or else they can screw up.
	arm.location = address = address & 0xFFFFFFFC;
	gbaMem.writeWordAccessTable[index](address, dest, data);
/*
	//Now switch off and perform special functions
	if ( index == 4 ) {
		//Write to IORAM 
		IO_checkForUpdate(oldValue);

	} else if ( (index >= 8) && (index < 0xE)) {
		//Illegal ROM write
		gba.stats.invalidWrites[index]++;
		gba.stats.lastInvalidWrite[index] = arm.location;
	}*/
	
}	



////////////////////////////////////////////////////////////
//This stores a hlf word to memory
//////////////////////////////////////////////////////////////
inline void Memory_writeHalfWord (u32 address, u16 data) {
	//Get the first byte of the arm.location (should be
	//0, 1, 2, 3, 4, 5, 6, 7, or 8 -- if somehow nintendo
	//decides to make ROMs with addresses higer than 0x08FFFFFF,
	//this will break.
	int index = address >> 24;
	
	u32 offset = address & 0x00FFFFFF;	//Chop off the first byte
	if (offset >= gbaMem.memorySize[index]) {
		//If the writing out of bounds, make note of it
		//for our statistics.
		gba.stats.writeHWXBounds[index]++;
		gba.stats.lastWriteHWXBounds = arm.location = address;
		return;
	}

	u16* dest = gbaMem.u16Mem[index];
	
	

	//According to ARM docs, halfwords can only be written to aligned addresses.
	//if ( ! (arm.location & 0x1 ) )	{ //Check if the instruction is halford aligned
	
	offset = offset >> 1;	//Convert to viable index

	//Use some pointer arithmetic
	//to retrieve from memory
	dest = ( u16*)(dest + offset);

	//NOTE:  We must align arm.location before passing it into the memory access
	//functions, or else they can screw up.
	arm.location = address = address & 0xFFFFFFFE;
	gbaMem.writeHalfWordAccessTable[index](address, dest, data);
	/*//Now switch off and perform special functions
	if ( index == 4 ) {
		IO_checkForUpdate(oldValue);

	} else if ( (index >= 8) && (index < 0xE)) {
		//Illegal ROM write
		gba.stats.invalidWrites[index]++;
		gba.stats.lastInvalidWrite[index] = arm.location;
	}*/
	
}

////////////////////////////////////////////////////////////
//This stores a bite to memory
//////////////////////////////////////////////////////////////
__inline void Memory_writeByte (u32 address, register u8 data) {
    //Get the first byte of the arm.location (should be
	//0, 1, 2, 3, 4, 5, 6, 7, or 8 -- if somehow nintendo
	//decides to make ROMs with addresses higer than 0x08FFFFFF,
	//this will break.
	//register u32 oldValue;
	register u32 index = (address >> 24);
	register u32 offset = address & 0x00FFFFFF;	//Chop off the first byte
	
	//This is a "fix" to get the star demo working.  Intention is to get rid of this
	//at a later date, when I can run more than just demos.;)
	if (offset >= gbaMem.memorySize[index]) {
		//If the writing out of bounds, make note of it
		//for our statistics.
		gba.stats.writeByteXBounds[index]++;
		gba.stats.lastWriteByteXBounds = arm.location = address;
		return;
	}

	register u8* dest = gbaMem.u8Mem[index];
	
	//Use some pointer arithmetic
	//to retrieve from our array.
	//Is storing a byte supposed to clobber the top 24 bits??  Here it does...
	dest = ( u8*)(dest + offset);
	arm.location = address;
	gbaMem.writeByteAccessTable[index](address, dest, data);

}

void Memory_writeByteStandardMemory(u32 address, u8* dest, u8 data) {
	*dest = data;
	//Do nothing.
}

void Memory_writeByteIORAM(u32 address, u8* dest, u8 data) {
	//This is highly unusual . . . keep a lookout for any roms that do this...
	//Because our update function expects a halfword, we need to do a read/modify/write.
	u32 offset = address & 0x00FFFFFF;
	u16 data16 = gbaMem.u16IORAM[offset>>1];	//This aligns it to 16 bit
	if (offset & 0x1)		//If it wasn't halfword aligned, it goes in the high byte
		data16 = (data16 & 0x00FF) | (data << 8);
	else						//Otherwise, it was aligned to a halfword, and goes in the low byte
		data16 = (data16 & 0xFF00) | data;
	
	//Check for update expects halfword-aligned, 16 bit data.  Thus chop of the lowest bit
	//of the address.
	IO_checkForUpdate(address & 0xFFFFFFFE, data16);	
}

void Memory_writeByteBIOS(u32 address, u8* dest, u8 data) {
	//Illegal BIOS write
	gba.stats.invalidWrites[0]++;
	gba.stats.lastInvalidWrite[0] = address;
	*dest = data;
}

void Memory_writeByteROM0(u32 address, u8* dest, u8 data) {
	//Illegal ROM write
	gba.stats.invalidWrites[8]++;
	gba.stats.lastInvalidWrite[8] = address;
	*dest = data;
}

void Memory_writeByteROM1(u32 address, u8* dest, u8 data) {
	//Illegal ROM write
	gba.stats.invalidWrites[0xA]++;
	gba.stats.lastInvalidWrite[0xA] = address;
	*dest = data;
}

void Memory_writeByteROM2(u32 address, u8* dest, u8 data) {
	//Illegal ROM write
	gba.stats.invalidWrites[0xC]++;
	gba.stats.lastInvalidWrite[0xC] = address;
	*dest = data;
}

void Memory_writeHalfWordStandardMemory(u32 address, u16* dest, u16 data) {
	*dest = data;
	//Do nothing.
}

void Memory_writeHalfWordIORAM(u32 address, u16* dest, u16 data) {
	IO_checkForUpdate(address, data);	//This performs the actual memory write.
	
}

void Memory_writeHalfWordBIOS(u32 address, u16* dest, u16 data) {
	//Illegal BIOS write
	gba.stats.invalidWrites[0]++;
	gba.stats.lastInvalidWrite[0] = address;
	*dest = data;
}

void Memory_writeHalfWordROM0(u32 address, u16* dest, u16 data) {
	//Illegal ROM write
	gba.stats.invalidWrites[8]++;
	gba.stats.lastInvalidWrite[8] = address;
	*dest = data;
}

void Memory_writeHalfWordROM1(u32 address, u16* dest, u16 data) {
	//Illegal ROM write
	gba.stats.invalidWrites[0xA]++;
	gba.stats.lastInvalidWrite[0xA] = address;
	*dest = data;
}

void Memory_writeHalfWordROM2(u32 address, u16* dest, u16 data) {
	//Illegal ROM write
	gba.stats.invalidWrites[0xC]++;
	gba.stats.lastInvalidWrite[0xC] = address;
	*dest = data;
}


void Memory_writeWordStandardMemory(u32 address, u32* dest, u32 data) {
	*dest = data;
	//Do nothing.
}

void Memory_writeWordIORAM(u32 address, u32* dest, u32 data) {
	//if ( (data >> 16) == 0x8440)
	//	arm.paused = arm.paused;//1;
	
	//In the case of writing a word to IO ram, we need to check both the top and bottom
	//halfwords to see if an update is necessary.
	IO_checkForUpdate(address, (u16)data);	//This performs the actual memory write as well
	IO_checkForUpdate(address+2, (u16)(data>>16));	//This performs the actual memory write as well
	
	//if ( *((u16*)(&gbaMem.u8IORAM[0xC6])) == 0x0440)
	//	arm.paused = arm.paused;//1;
}

void Memory_writeWordBIOS(u32 address, u32* dest, u32 data) {
	//Illegal BIOS write
	gba.stats.invalidWrites[0]++;
	gba.stats.lastInvalidWrite[0] = address;
	*dest = data;
}

void Memory_writeWordROM0(u32 address, u32* dest, u32 data) {
	//Illegal ROM write
	gba.stats.invalidWrites[8]++;
	gba.stats.lastInvalidWrite[8] = address;
	*dest = data;
}

void Memory_writeWordROM1(u32 address, u32* dest, u32 data) {
	//Illegal ROM write
	gba.stats.invalidWrites[0xA]++;
	gba.stats.lastInvalidWrite[0xA] = arm.location;
	*dest = data;
}

void Memory_writeWordROM2(u32 address, u32* dest, u32 data) {
	//Illegal ROM write
	gba.stats.invalidWrites[0xC]++;
	gba.stats.lastInvalidWrite[0xC] = address;
	*dest = data;
}