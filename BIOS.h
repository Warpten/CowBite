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

#ifndef BIOS_H
#define BIOS_H

#include <math.h>
#include "Graphics.h"
#include "Memory.h"
#include "ARM.h"
#include "Console.h"

//////////////////////////////////////////
//Halt();
//Causes the CPU to do _nothing_ until
//an interrupt occurs.
/////////////////////////////////////////
inline void BIOS_Halt() {
	int blah;
	arm.halted = 1;
	/*if (arm.halted == 2) {
		if (arm.thumbMode)
			ARM_nextInstructionThumb();
		else
			ARM_nextInstruction();
	} else arm.halted = 1;*/
	/*
	if (arm.r[1] != 0)
		blah = 0;
	if (arm.r[3] != 1)
		blah = 0;
	if (arm.r[0] != 0x020032d0)
		blah = 0;
	if ( (arm.CPSR & 0x3) == 0x10)
		int blah = 0;*/
//I really need to learn how this works.

/*	//Test.
	if (graphics.lcy >= 160) {
		if (arm.thumbMode)
			ARM_nextInstructionThumb();
		else
			ARM_nextInstruction();
	}*/
/*
	if (arm.thumbMode)
		ARM_nextInstructionThumb();
	else
		ARM_nextInstruction();*/

}




////////////////////////////////////////////////
//VBlankIntrWait()
//Replicates the bios VBlankIntrWait() call.
//Causes the current instruction (swi) to be
//executed repeatedly until vblank occurs.
/////////////////////////////////////////////
inline void BIOS_VBlankIntrWait() {
	//Not sure if I want to wait until exactly 160 or what...
	if (graphics.lcy == 160) {
		if (arm.thumbMode)
			ARM_nextInstructionThumb();
		else
			ARM_nextInstruction();
	}

}

///////////////////////////////////////////////
//Div()
//Replicates the Div() bios function
//////////////////////////////////////////////
inline void BIOS_Div() {
	s32 result = (s32)arm.r[0]/(s32)arm.r[1];
	s32 remainder = arm.r[0] % arm.r[1];
	arm.r[0] = result;
	arm.r[1] = remainder;
	arm.r[3] = abs(result);
	if (arm.thumbMode)
		ARM_nextInstructionThumb();
	else
		ARM_nextInstruction();
}

//////////////////////////////////////////////////
//Sqrt()
//Replicates the Sqrt() bios function
/////////////////////////////////////////////
inline void BIOS_Sqrt() {
	arm.r[0] = sqrt(arm.r[0]);
	if (arm.thumbMode)
		ARM_nextInstructionThumb();
	else
		ARM_nextInstruction();
}

//////////////////////////////////////////////////
//ArcTan()
//Replicates the ArcTan() bios function
/////////////////////////////////////////////
inline void BIOS_ArcTan() {
	arm.r[0] = atan(arm.r[0]);
	if (arm.thumbMode)
		ARM_nextInstructionThumb();
	else
		ARM_nextInstruction();
}

//////////////////////////////////////////
//CPUSet()
//Replicates the CPUSet() bios function
//Note that I have based this off of the DMA transfer code...
//Probably would be best to re-implement this using
//the Memory module
////////////////////////////////////////
inline void BIOS_CPUSet() {
	u32 sourceAddress = arm.r[0];
	u32 destAddress = arm.r[1];
	arm.location = destAddress;	//For the debugger.
	u32 cnt = arm.r[2];
	u32 destWriteback;
	u32 data32bit;

	//Find out what kind of memory we are dealing with
	
	//Start them off the same
	//use our little indexing scheme to get the appropriate memory
	u32 index = ((sourceAddress & 0xFF000000) >> 24);
	if (index == 4)
		int blah = 0;
	else
		int blah = 1;

/*	u32* source = gbaMem.u32Mem[index];
	u32 sourceOffset = sourceAddress & (0x00FFFFFF);	//Get the last part of the offset

	//Find out what kind of memory the dest is
	index = ((destAddress & 0xFF000000) >> 24);
	u32* dest = gbaMem.u32Mem[index];
	u32 destOffset = destAddress & (0x00FFFFFF);
*/
	if (index == 4)
		int blah = 0;

	u32 numTransfers = (cnt & 0x0000FFFF);//Could easily be WRONG!!!
	s32 sourceIncrement, destIncrement;
	sourceIncrement = destIncrement = 1;
	destWriteback = 1;



	//Find out the nature of the source copy
	switch ( (cnt >> 23) & 0x3) {
		case 0x0:	//Increment the source address each copy
					break;

		case 0x1:	//Decrement the source address each copy
					sourceIncrement = -sourceIncrement;
					break;

		default:	//In any other case, no increment.
					sourceIncrement = 0;
	}

	//Find out the nature of the destination copy
	switch ( (cnt >> 21) & 0x3) {
		case 0x0:	destWriteback = 1;
					break;
		case 0x3:	destWriteback = 0;//Increment the dest address each copy
					break;

		case 0x1:	//Decrement the dest address each copy
					destIncrement = -destIncrement;
					destWriteback = 1;
					break;

		default:	//In any other case, no increment.
					destWriteback = 0;
					destIncrement = 0;
	}

	//NOTE:  As far as I can tell, there is no write back for this function.

//	if ( cnt & 0x04000000) {	//If the S bit is set, 32 bit
	
	
	//I guess all CPU set transfers are 32 bit???
/*		sourceOffset = sourceOffset >> 2;
		source += sourceOffset;					//Add onto the source
		destOffset = destOffset >> 2;
		dest += destOffset;
*/
		
		for (int i = numTransfers; i > 0; i--) {
			//u32 address = sourceAddress;
			data32bit = Memory_readWord(sourceAddress);
			
			//This is necessary in case the user reaches a breakpoint during transfer...
			if (ARM_reachedConditionalBreakpoint(0xFFFFFFFF)) {
				arm.paused = 1;
				arm.steps = arm.stepAddress = arm.stepInto = 0;
				arm.stepFunction = NULL;
			} 
			
			//address = destAddress;
			Memory_writeWord (destAddress, data32bit);

			//This is necessary in case the user reaches a breakpoint during transfer...
			if (ARM_reachedConditionalBreakpoint(0xFFFFFFFF)) {
				arm.paused = 1;
				arm.steps = arm.stepAddress = arm.stepInto = 0;
				arm.stepFunction = NULL;
			} 

			sourceAddress+= (sourceIncrement << 2);
			//*srcPtr += (sourceIncrement << 2);
			destAddress += (destIncrement << 2);
			//if (destWriteback)
			//	*dstPtr += (destIncrement << 2);

	/*		*dest = *source;
			source+= sourceIncrement;
			dest += destIncrement;			*/

		}
		if (destWriteback)
			arm.r[1] = destAddress;	//not sure about this
		arm.r[2] = sourceAddress;
		
/*	} else {	//Otherwise it's 16 bit, and we have to do stuff.
		//u16* scratch16bit = (u16*) scratchMem;
		u16* source16bit = (u16*) source;
		u16* dest16bit = (u16*) dest;
		
		sourceOffset = sourceOffset >> 1;
		source16bit += sourceOffset;					//Add onto the source
		destOffset = destOffset >> 1;
		dest16bit += destOffset;

		for (int i = numTransfers; i > 0; i--) {
						//*scratch16bit = *source16bit;
			*dest16bit = *source16bit;
			source16bit+= sourceIncrement;
			//scratch16bit += destIncrement;
			dest16bit += destIncrement;
		} 
		
		
	}//End of else	
*/
	if (arm.thumbMode)
		ARM_nextInstructionThumb();
	else
		ARM_nextInstruction();

}

//////////////////////////////////////////////
//LZ77UnComp()
//This decompresses memory using LZSS.
//NOTE:  Have not implemented the ability to break on access during this function.
//////////////////////////////////////////////
inline void LZ77UnComp() {
	u32 sourceAddress = arm.r[0];
	u32 destAddress = arm.r[1];
	u32 outSize;
	u32 bytesDecompressed;
	u32 windowPtr;
	u32 codePtr;
	u32 mask;
	u32 data;
	u32 key;
	u32 length;
	u32 offset;
//	char text[256];
	
	//u32 address = sourceAddress;
	outSize = Memory_readWord(sourceAddress);
	//I don't know what the first byte does, so I'm clipping it off for now.
	outSize = outSize >> 8;

	windowPtr = destAddress;
	codePtr = sourceAddress+4;
	bytesDecompressed = 0;	

	//Until done...
	while (bytesDecompressed < outSize) {
		//The in buffer is divided up into sections starting with an 8 bit key
		//describing what each unit does, followed by 8 units.
		//arm.location = codePtr;
		key = Memory_readByte(codePtr);
		codePtr++;	//Move to the next byte
		
		mask = 0x80;
		for (int j = 0; (j < 8) && (bytesDecompressed < outSize); j++) {
			//If the bit is set, that means it's encoded.
			if (key & mask) {
				//arm.location = codePtr;
				data = Memory_readByte(codePtr);
				length = (data >> 4)+3;	//The length is the top four bits
				offset = (data & 0xF) << 8;	//The top 4 bits of offset are in the bottom four bits of the key
				codePtr++;
			//	arm.location = codePtr;
				offset |= Memory_readByte(codePtr);			//'OR' in the bottom 8 bits
				offset++;									//The offset is greater
				codePtr++;
				
				//Copy the bytes into the outbuffer
				for (int k = 0; k < length; k++) {
					if ((windowPtr-offset) < destAddress) {
						//sprintf(text, "\nRead out of bounds: %d, %08X, %08X", offset, windowPtr-offset, destAddress);
						//Console_print(text);
						//arm.location = windowPtr;
						Memory_writeByte(windowPtr, ' ');
						
					} else {
						//arm.location = windowPtr - offset;
						data = Memory_readByte(windowPtr - offset);
						arm.location = windowPtr;
						Memory_writeByte(windowPtr, data);
					}
					windowPtr++;
					bytesDecompressed++;
				}
			} else {	//Otherwise, it's uncompresed.
				//arm.location = codePtr;
				data = Memory_readByte(codePtr);
				//arm.location = windowPtr;
				Memory_writeByte(windowPtr, data);
				windowPtr++;
				codePtr++;
				bytesDecompressed++;
			}
			//if (bytesDecompressed >= outSize)	//If we exceeded our encoded bytes....
			//	break;
			mask = mask >> 1;
		}
		//printf("\nDecompressed: %d", bytesDecompressed);
		
	}
	//sprintf(text, "\nBytes Decompressed: %X; outSize: %X.", bytesDecompressed, outSize);
	//Console_print(text);
	
	if (arm.thumbMode)
		ARM_nextInstructionThumb();
	else
		ARM_nextInstruction();
	
}


/////////////////////////////////////////////
//BIOS_exec
//Calls the appropriate BIOS function based on the value
//passed in.
////////////////////////////////////////////////
inline void BIOS_exec( u32 value) {
	int blah;
	//NOTE:  This is a kind of cluge.  I have noticed that, for whatever
	//reason, swi instructions seem to be shifted by 16 depending on the mode.
	if (arm.thumbMode)
		value = value << 16;
	switch (value) {
		case 0x020000:	BIOS_Halt();
						break;
		case 0x030000: BIOS_Halt();	//Stop()
						break;

		case 0x050000:	//IntrWait
		case 0x040000:	BIOS_VBlankIntrWait();
						break;
		case 0x060000:	BIOS_Div();
					break;
		case 0x080000:	BIOS_Sqrt();
					break;
		
		case 0x090000:	BIOS_ArcTan();
					break;
		case 0x0C0000:	//CPUFastSet
		case 0x0b0000:	BIOS_CPUSet();	//CPUSet
					break;
		case 0x110000: //LZ77UncompWRAM
		case 0x120000: LZ77UnComp(); //LZ77UncompVRAM
					break;

		//Cases I haven't implemented yet:
		case 0x0A0000:	blah = blah;//ArcTan2
		case 0x0E0000:	blah = blah;//Bg affine set
		case 0x0F0000:	blah = blah;//Obj affine set
		case 0x100000:	blah = blah;//bit unpack
		case 0x010000:	blah = blah;//Register ram reset
		case 0x000000:	blah = blah;//Soft reset
		case 0x130000: blah = blah;//HuffUnComp
		case 0x140000: blah = blah;//RLUnCompWRAM
		case 0x150000:	blah = blah;//RLUnCompVRAM
		case 0x160000:	blah = blah;//Diff8bitUnFilterWRAM
		case 0x170000:	blah = blah;//0x17: Diff8bitUnFilterVRAM
		case 0x180000:	blah = blah;//0x18: Diff16bitUnFilter
		case 0x190000:	blah = blah;//0x19: SoundBiasChange
		case 0x1A0000:	blah = blah;//0x1A: SoundDriverInit
		case 0x1B0000:	blah = blah;//0x1B: SoundDriverMode
		case 0x1C0000:	blah = blah;//0x1C: SoundDriverMain
		case 0x1D0000:	blah = blah;//0x1D: SoundDriverVSync
		case 0x1E0000:	blah = blah;//0x1E: SoundChannelClear
		case 0x1F0000:	blah = blah;//0x1F: MIDIKey2Freq
		case 0x200000:	blah = blah;//Ox20: MusicPlayerOpen
		case 0x210000:	blah = blah;//0x21: MusicPlayerStart
		case 0x220000:	blah = blah;//0x22: MusicPlayerStop
		case 0x230000:	blah = blah;//0x23: MusicPlayerContinue
		case 0x240000:	blah = blah;//0x24: MusicPlayerFadeOut
		case 0x250000:	blah = blah;//0x25: MultiBoot
		case 0x260000:	blah = blah;//0x26: ??
		case 0x270000: blah = blah;//0x27: ??
		case 0x280000:	blah = blah;//0x28: SoundDriverVSyncOff
		case 0x290000:	blah = blah;//0x29: SoundDriverVSyncOn
		default:	if (arm.thumbMode)
						ARM_nextInstructionThumb();
					else
						ARM_nextInstruction();
					break;
	}
}


#endif 