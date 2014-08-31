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

#include <string.h>

#include "GBADebugger.h"
#include "Constants.h"
#include "Support.h"
#include "Memory.h"
#include <stdio.h>
#include "GBA.h"
//#define blah 1

GBADebugger gbaDebugger;

////////////////////////////////////////////
//init()
//This function initializes the GBA debugger
////////////////////////////////////////////
void GBADebugger_init() {
	GBADebugger_initRegStrings();

}

////////////////////////////////////////////
//getRegOffset
//Takes an index into the register array and returns the address of the register
////////////////////////////////////////////
/*u32 GBADebugger_getRegOffset(u32 index) {
	//The first element of the register entry uses the mask value as the
	//address.
	return gbaDebugger.regEntries[index].offset;
}*/


///////////////////////////////////////////
//getRegEntry
//////////////////////////////////////////
RegEntry* GBADebugger_getRegEntry (u32 offset) {
	for (int i = 0; i < NUMHARDWAREREGS; i++) {
		if (gbaDebugger.regEntries[i].offset == offset)
			return &gbaDebugger.regEntries[i];
	}
	return NULL;
}

//////////////////////////////
//getRegEntry (by name)
/////////////////////////////
RegEntry* GBADebugger_getRegEntry (char* name) {
	for (int i = 0; i < NUMHARDWAREREGS; i++) {
		if (strcmp (gbaDebugger.regEntries[i].name, name) == 0)
			return &gbaDebugger.regEntries[i];
	}
	return NULL;
}

///////////////////////////////////////////
//Return a string in binary.
//Converts to string starting from bit 0 and going to numBits
///////////////////////////////////////////
char* GBADebugger_hexToBinString(u32 number, u32 numBits) {
	static char returnString[255];
	u32 mask = 1;

	strcpy(returnString,"");	//Set to ""

	//Return a string in binary
	for (int i = 0; i < numBits; i++) {
		if (number & mask)
			strcat(returnString,"1");
		else
			strcat(returnString,"0");
		
		mask = mask << 1;
	}
	strrev(returnString);	//Reverse what we just did.
	return returnString;
}

////////////////////////////////////////////
//regToString
//Takes the given offset and returns information about this register as a string.
////////////////////////////////////////////
char* GBADebugger_regToString(u32 index) {
	static char returnString[255];
	RegEntry* regEntry;
	u32 offset = gbaDebugger.regEntries[index].offset;
	//u32 offset = GBADebugger_getRegOffset(index);
	u32 address = 0x4000000 + offset; 
	u32 value = * ((u16*)(&(gbaMem.u8IORAM[offset])));
	regEntry = GBADebugger_getRegEntry(offset);
	//Return a string listing the address, hex value, binary value, and register name.
	sprintf (returnString, "%08X %04X %s %s",  address, value, GBADebugger_hexToBinString(value,16), 
				regEntry->name);
	return returnString;
}

///////////////////////////////////////////////////////////////////////
//getRegInfo
//Returns info about the register currently in memory
////////////////////////////////////////////////////////////////////////
char* GBADebugger_getRegInfo() {
	u32 offset = gbaDebugger.currentReg.offset;
	u32 value = gbaDebugger.currentReg.value;
	static char text[1024];
	switch(offset) {
		case 0x0:	sprintf(text, "- Value of bit 4 is %d;\n", (value >> 4) & 1);
					strcat(text, "Frame buffer for ");
					if ( (value & 0x7) == 0x5)	//If it's in mode 5..
						if ( value & 0x10)	
							strcat(text, "mode 5 set to 0x06000000");
						else
							strcat(text, "mode 5 set to 0x0600A000");
					else
						if ( value & 0x10)		//In all other cases...
							strcat(text, "mode 4 set to 0x0600A000");
						else
							strcat(text, "mode 4 set to 0x06000000");
					break;
		case 0x3C0:
		case 0x3C2:
		case 0x3C4:
		case 0x3C6:
		case 0x3C8:	sprintf(text, "These registers are currently unimplemented.");
					strcat(text, " The idea is to point EMUIN to a structure");
					strcat(text, " and then use EMUCNT to perform various debugging");
					strcat(text, " functions (such as printing output, setting breakpoints,");
					strcat(text, " dumping memory, etc.) as the program runs.");
					strcat(text, "\n\nOnce I have them implemented, these registers should only");
					strcat(text, " be used for debugging on the emulator and NOT on");
					strcat(text, " actual hardware.");
					break;
		default:	sprintf(text, "No additional info for this register.");
					break;
	}
	return text;
}


///////////////////////////////
//This gets all of the statistics possible.
///////////////////////////////
char* GBADebugger_getStats() {
	static char text[2048];
	strcat(text, GBADebugger_getMiscStats());
	strcat(text, GBADebugger_getMemoryStats());
	strcat(text, GBADebugger_getInterruptStats());
	strcat(text, GBADebugger_getSoundStats());
	return text;
}

//////////////////////////////////////////
//Thsi displays miscellaneous stats I wasn't able to think
//of a category for
///////////////////////////////////////////
char* GBADebugger_getMiscStats() {
	static char text[1024];
	sprintf(text,	"General Statistics\n\
-------------------\n\
Frames rendered: %d (280,896 cycles\/frame at 59.727 hz)\n\
Cycles this frame:	%d*\n\
Cycles this line:	%d*\n\
*CowBite cycle timings are approximated at 4 cycles per instruction\n\
\n\
DMA Transfers\n\
--------------\n\
                \tDMA0\tDMA1\tDMA2\tDMA3 \n\
Total:          \t%d\t%d\t%d\t%d\n\
This frame:     \t%d\t%d\t%d\t%d\n\
\n\
Timer Statistics\n\
----------------\n\
                \tTM0\tTM1\tTM2\tTM3\n\
Start Values:   \t%d\t%d\t%d\t%d\n\
Current Values: \t%d\t%d\t%d\t%d\n\
Tick Tallies:*  \t%d\t%d\t%d\t%d\n\
*Number of cycles since last timer increment\n\n",

gba.stats.frameTally, gba.stats.frameTickTally, gba.stats.frameTickTally - gba.hDrawBeginCycles[graphics.lcy],
gba.stats.dmaTransfers[0], gba.stats.dmaTransfers[1],
gba.stats.dmaTransfers[2], gba.stats.dmaTransfers[3],
gba.stats.frameDMATransfers[0], gba.stats.frameDMATransfers[1],
gba.stats.frameDMATransfers[2], gba.stats.frameDMATransfers[3],
gba.stats.timerStartValues[0],gba.stats.timerStartValues[1],
gba.stats.timerStartValues[2],gba.stats.timerStartValues[3],
*REG_TM0DAT, *REG_TM1DAT,*REG_TM2DAT,*REG_TM3DAT,
gba.stats.timerTickTally[0],gba.stats.timerTickTally[1],
gba.stats.timerTickTally[2],gba.stats.timerTickTally[3]);

	return text;
}

char* GBADebugger_getMemoryStats() {
	static char text[1024];
	u32 otherXBounds[6];
	u32 totalXBounds[6];
	GBAStats* s = &gba.stats;

	memset(otherXBounds, 0, sizeof(otherXBounds));
	memset(totalXBounds, 0, sizeof(totalXBounds));

	//Tally all out of bounds acceses not occuring in designated sections
	//of memory.
	for (int i = 0; i <0x100; i++) {
		totalXBounds[0] += s->readWordXBounds[i];
		totalXBounds[1] += s->readHWXBounds[i];
		totalXBounds[2] += s->writeWordXBounds[i];
		totalXBounds[3] += s->writeHWXBounds[i];
		totalXBounds[4] += s->writeByteXBounds[i];

		if ( (i == 0x1) || (i >= 0xF)) {
			otherXBounds[0] += s->readWordXBounds[i];
			otherXBounds[1] += s->readHWXBounds[i];
			otherXBounds[2] += s->writeWordXBounds[i];
			otherXBounds[3] += s->writeHWXBounds[i];
			otherXBounds[4] += s->writeByteXBounds[i];
		}
		
	}
	
	sprintf(text, 
"Memory Statistics\n\
-----------------\n\
\n\
Invalid Writes (to Read-Only Memory)*\n\
         \tBIOS\tIO\tROM0\tROM1\tROM2\n\
         \t%d\t%d\t%d\t%d\t%d\n\
Last:    \t%08X\t%08X\t%08X\t%08X\t%08X\n\
*CowBite does not prevent invalid writes, but it does attempt to log them.\n\
\n\
Out of Bounds Accesses (to Non-Existent Memory)\n\
         \tR. Word\tR. HW\tW. Word\tW. HW\tW. Byte\n\
Total:   \t%d\t%d\t%d\t%d\t%d\n\
BIOS:    \t%d\t%d\t%d\t%d\t%d\n\
EXT:     \t%d\t%d\t%d\t%d\t%d\n\
INT:     \t%d\t%d\t%d\t%d\t%d\n\
IO:      \t%d\t%d\t%d\t%d\t%d\n\
PAL:     \t%d\t%d\t%d\t%d\t%d\n\
VRAM:    \t%d\t%d\t%d\t%d\t%d\n\
OAM:     \t%d\t%d\t%d\t%d\t%d\n\
ROM0:    \t%d\t%d\t%d\t%d\t%d\n\
ROM1:    \t%d\t%d\t%d\t%d\t%d\n\
ROM2:    \t%d\t%d\t%d\t%d\t%d\n\
CARTRAM: \t%d\t%d\t%d\t%d\t%d\n\
Other:   \t%d\t%d\t%d\t%d\t%d\n\
Last:    \t%08X\t%08X\t%08X\t%08X\t%08X\n\n",
s->invalidWrites[0x0],s->invalidWrites[0x4],s->invalidWrites[0x8],s->invalidWrites[0xA],s->invalidWrites[0xC],
s->lastInvalidWrite[0x0], s->lastInvalidWrite[0x4],s->lastInvalidWrite[0x8],s->lastInvalidWrite[0xA],s->lastInvalidWrite[0xC],

totalXBounds[0], totalXBounds[1],totalXBounds[2], totalXBounds[3],totalXBounds[4],
s->readWordXBounds[0],s->readHWXBounds[0],s->writeWordXBounds[0],s->writeHWXBounds[0], s->writeByteXBounds[0],
s->readWordXBounds[2],s->readHWXBounds[2],s->writeWordXBounds[2],s->writeHWXBounds[2], s->writeByteXBounds[2],
s->readWordXBounds[3],s->readHWXBounds[3],s->writeWordXBounds[3],s->writeHWXBounds[3], s->writeByteXBounds[3],
s->readWordXBounds[4],s->readHWXBounds[4],s->writeWordXBounds[4],s->writeHWXBounds[4], s->writeByteXBounds[4],
s->readWordXBounds[5],s->readHWXBounds[5],s->writeWordXBounds[5],s->writeHWXBounds[5], s->writeByteXBounds[5],
s->readWordXBounds[6],s->readHWXBounds[6],s->writeWordXBounds[6],s->writeHWXBounds[6], s->writeByteXBounds[6],
s->readWordXBounds[7],s->readHWXBounds[7],s->writeWordXBounds[7],s->writeHWXBounds[7], s->writeByteXBounds[7],
s->readWordXBounds[8] + s->readWordXBounds[9],s->readHWXBounds[8]+s->readHWXBounds[9],
s->writeWordXBounds[8]+s->writeWordXBounds[9],s->writeHWXBounds[8]+s->writeHWXBounds[9],
s->writeByteXBounds[8]+s->writeByteXBounds[9],
s->readWordXBounds[0xA] + s->readWordXBounds[0xB],s->readHWXBounds[0xA]+s->readHWXBounds[0xB],
s->writeWordXBounds[0xA]+s->writeWordXBounds[0xB],s->writeHWXBounds[0xA]+s->writeHWXBounds[0xB],
s->writeByteXBounds[0xA]+s->writeByteXBounds[0xB],
s->readWordXBounds[0xC] + s->readWordXBounds[0xD],s->readHWXBounds[0xC]+s->readHWXBounds[0xD],
s->writeWordXBounds[0xC]+s->writeWordXBounds[0xD],s->writeHWXBounds[0xC]+s->writeHWXBounds[0xD],
s->writeByteXBounds[0xC]+s->writeByteXBounds[0xD],
s->readWordXBounds[0xE],s->readHWXBounds[0xE],s->writeWordXBounds[0xE],s->writeHWXBounds[0xE], s->writeByteXBounds[0xE],
otherXBounds[0], otherXBounds[1],otherXBounds[2], otherXBounds[3],otherXBounds[4],

s->lastReadWordXBounds,s->lastReadHWXBounds,s->lastWriteWordXBounds,s->lastWriteHWXBounds, s->lastWriteByteXBounds
);
	return text;
}

char* GBADebugger_getInterruptStats(){
	static char text[1024];
	sprintf(text,
"Interrupts\n\
----------\n\
\n\
          \tTotal\tFrame\tLast PC Value\n\
V:        \t%d\t%d\t%08X\n\
H:        \t%d\t%d\t%08X\n\
VCount:   \t%d\t%d\t%08X\n\
TM0:      \t%d\t%d\t%08X\n\
TM1:      \t%d\t%d\t%08X\n\
TM2:      \t%d\t%d\t%08X\n\
TM3:      \t%d\t%d\t%08X\n\
SERIAL:   \t%d\t%d\t%08X\n\
DMA0:     \t%d\t%d\t%08X\n\
DMA1:     \t%d\t%d\t%08X\n\
DMA2:     \t%d\t%d\t%08X\n\
DMA3:     \t%d\t%d\t%08X\n\
KEY:      \t%d\t%d\t%08X\n\
CASSETTE: \t%d\t%d\t%08X\n\n",
gba.stats.interrupts[0],gba.stats.frameInterrupts[0],gba.stats.lastInterrupt[0],
gba.stats.interrupts[1],gba.stats.frameInterrupts[1],gba.stats.lastInterrupt[1],
gba.stats.interrupts[2],gba.stats.frameInterrupts[2],gba.stats.lastInterrupt[2],
gba.stats.interrupts[3],gba.stats.frameInterrupts[3],gba.stats.lastInterrupt[3],
gba.stats.interrupts[4],gba.stats.frameInterrupts[4],gba.stats.lastInterrupt[4],
gba.stats.interrupts[5],gba.stats.frameInterrupts[5],gba.stats.lastInterrupt[5],
gba.stats.interrupts[6],gba.stats.frameInterrupts[6],gba.stats.lastInterrupt[6],
gba.stats.interrupts[7],gba.stats.frameInterrupts[7],gba.stats.lastInterrupt[7],
gba.stats.interrupts[8],gba.stats.frameInterrupts[8],gba.stats.lastInterrupt[8],
gba.stats.interrupts[9],gba.stats.frameInterrupts[9],gba.stats.lastInterrupt[9],
gba.stats.interrupts[10],gba.stats.frameInterrupts[10],gba.stats.lastInterrupt[10],
gba.stats.interrupts[11],gba.stats.frameInterrupts[11],gba.stats.lastInterrupt[11],
gba.stats.interrupts[12],gba.stats.frameInterrupts[12],gba.stats.lastInterrupt[12],
gba.stats.interrupts[13],gba.stats.frameInterrupts[13],gba.stats.lastInterrupt[13],
gba.stats.interrupts[14],gba.stats.frameInterrupts[14],gba.stats.lastInterrupt[14],
gba.stats.interrupts[15],gba.stats.frameInterrupts[15],gba.stats.lastInterrupt[15]);

	return text;
}

inline u32 GBADebugger_getFrequency(u32 value) {
	switch (value & 0x3) {
		case 0:	return 1;
		case 1: return 64;
		case 2: return 256;
		case 3: return 1024;
	}
	return 0;
}

char* GBADebugger_getSoundStats() {
	static char text[1024];
	
	u32 soundCntH = *REG_SOUNDCNT_H;
	u32 sampleTimerA = (soundCntH >> 10) & 1;	//Timer 0 or 1?
	u32 sampleTimerB = (soundCntH >> 14) & 1;	//Timer 0 or 1?
	u32 tmDivider[2];
	u8 useDma = 0;
	tmDivider[0] = 	GBADebugger_getFrequency(*REG_TM0CNT);					
	tmDivider[1] = 	GBADebugger_getFrequency(*REG_TM1CNT);					
	double sampleRateA = 16777216 / 
			((0x10000 - gba.stats.timerStartValues[sampleTimerA]) * tmDivider[sampleTimerA]);
	double sampleRateB = 16777216 / 
			((0x10000 - gba.stats.timerStartValues[sampleTimerB]) * tmDivider[sampleTimerB]);
	

	sprintf(text, 
"Sample Timer A Set to TM%d\n\
Sample Timer B Set to TM%d\n\
TM0 Start Value:       %d\n\
TM0 Divider:	       %d\n\
TM1 Start Value:       %d\n\
TM1 Divider:           %d\n\
Sample Rate A:         %.2f hz\n\
Sample Rate B:         %.2f hz\n\
",
sampleTimerA, sampleTimerB, gba.stats.timerStartValues[0], tmDivider[0],
gba.stats.timerStartValues[1],tmDivider[1],sampleRateA, sampleRateB);

strcat(text, "Sound A Output: ");
if (*REG_SOUNDCNT_H & 0x0200)
	strcat (text, "Left ");
if (*REG_SOUNDCNT_H & 0x0100)
	strcat (text, "Right ");
if (*REG_SOUNDCNT_H & 0x0004)
	strcat (text, "100%\n");
else
	strcat (text, "50%\n");

strcat(text, "Sound B Output: ");
if (*REG_SOUNDCNT_H & 0x2000)
	strcat (text, "Left ");
if (*REG_SOUNDCNT_H & 0x0200)
	strcat (text, "Right ");
if (*REG_SOUNDCNT_H & 0x0008)
	strcat (text, "100%\n");
else
	strcat (text, "50%\n");



	strcat(text, 
"Fifo Refill:\n");
if (((*REG_DMA1CNT >> 12) & 0x3) == 0x3) {
	switch (*REG_DMA1DST) {
		case 0x40000A0:	strcat(text, "DMA1, FIFOA\n");
		break;
		case 0x40000A2:	strcat(text, "DMA1, FIFOA_H\n");
		break;
		case 0x40000A4:	strcat(text, "DMA1, FIFOB\n");
		break;
		case 0x40000A6:	strcat(text, "DMA1, FIFOB_H\n");
		break;
		default:	strcat(text, "DMA1");
		break;
	}
	useDma = 1;
}
if (((*REG_DMA2CNT >> 12) & 0x3) == 0x3) {
	switch (*REG_DMA2DST) {
		case 0x40000A0:	strcat(text, "DMA2, FIFOA\n");
		break;
		case 0x40000A2:	strcat(text, "DMA2, FIFOA_H\n");
		break;
		case 0x40000A4:	strcat(text, "DMA2, FIFOB\n");
		break;
		case 0x40000A6:	strcat(text, "DMA2, FIFOB_H\n");
		break;
		default:	strcat(text, "DMA2\n");
		break;
	}
	useDma = 1;
}
if (! useDma) {
	strcat(text, "User (Interrupt)");
}
	strcat(text, "\n\n");
	return text;
}

/////////////////////////////////////////////
//initRegStrings
//This function creates an array of arrays of structs.
//Each element in bitEntries array is another array 
/////////////////////////////////////////////
void GBADebugger_initRegStrings() {

	BitEntry *bitEntries[NUMHARDWAREREGS];

	memset (bitEntries, NULL, NUMHARDWAREREGS * sizeof (BitEntry*));

	/////////////////////////////////////////////////////////
	//WARNING:  BRUTISH, INNEFFICIENT, INELEGANT CODE FOLLOWS
	/////////////////////////////////////////////////////////

	//Display regs
	static BitEntry reg0[] = {0x0, "REG_DISPCNT", B0|B1|B2, "Video mode", B3, "Game Boy Color mode",
						B4, "Frame buffer start", B5, "Process OBJ in hblank",
						B6, "Use 1D sprites", B7, "Force blank", B8, "BG0 enabled",
						B9, "BG1 enabled", B10, "BG2 enabled", B11, "BG3 enabled",
						B12, "Sprites enabled", B13, "Window 0 Enabled", B14, "Window 1 Enabled",
						B15, "Sprite windows enabled"};
	bitEntries[0] = reg0;
	static BitEntry reg1[] = {0x2, "?", 0xFFFF, "Unknown"};
	bitEntries[1] = reg1;


	static BitEntry reg2[] = {0x4, "REG_STAT", B0, "In vblank", B1, "In hblank", B2, "Vcount triggered",
						B3, "Vblank IRQ enabled", B4, "Hblank IRQ enabled", B5, "Vcount IRQ enabled",
						0xFF00, "Vcount trigger value"};
	bitEntries[2] = reg2;
	static BitEntry reg3[] = {0x6, "REG_VCOUNT", 0xFFFF, "Vcount value"};
	bitEntries[3] = reg3;
	
	//Background regs
	static BitEntry reg4[] = {0x8, "REG_BG0", B0|B1, "Priority",B2|B3, "Tile Data", B6, "Mosaic", 
						B7, "256 Color Palette", B8|B9|B10|B11|B12, "Tile Map", B13, "Area Overflow",
						B14|B15, "Size" };
	bitEntries[4] = reg4;
	static BitEntry reg5[] = {0xA, "REG_BG1", B0|B1, "Priority",B2|B3, "Tile Data", B6, "Mosaic", 
						B7, "256 Color Palette", B8|B9|B10|B11|B12, "Tile Map", B13, "Area Overflow",
						B14|B15, "Size" };
	bitEntries[5] = reg5;
	static BitEntry reg6[] = {0xC, "REG_BG2", B0|B1, "Priority",B2|B3, "Tile Data", B6, "Mosaic", 
						B7, "256 Color Palette", B8|B9|B10|B11|B12, "Tile Map", B13, "Area Overflow",
						B14|B15, "Size" };
	bitEntries[6] = reg6;
	static BitEntry reg7[] = {0xE, "REG_BG3", B0|B1, "Priority",B2|B3, "Tile Data", B6, "Mosaic", 
						B7, "256 Color Palette", B8|B9|B10|B11|B12, "Tile Map", B13, "Area Overflow",
						B14|B15, "Size" };
	bitEntries[7] = reg7;

	//background scroll regs
	static BitEntry reg8[] = {0x10, "REG_BG0SCX", 0x3FF, "Scoll Value"};
	bitEntries[8] = reg8;
	static BitEntry reg9[] = {0x12, "REG_BG0SCY", 0x3FF, "Scoll Value"};
	bitEntries[9] = reg9;
	static BitEntry reg10[] = {0x14, "REG_BG1SCX", 0x3FF, "Scoll Value"};
	bitEntries[10] = reg10;
	static BitEntry reg11[] = {0x16, "REG_BG1SCY", 0x3FF, "Scoll Value"};
	bitEntries[11] = reg11;
	static BitEntry reg12[] = {0x18, "REG_BG2SCX", 0x3FF, "Scoll Value"};
	bitEntries[12] = reg12;
	static BitEntry reg13[] = {0x1A, "REG_BG2SCY", 0x3FF, "Scoll Value"};
	bitEntries[13] = reg13;
	static BitEntry reg14[] = {0x1C, "REG_BG3SCX", 0x3FF, "Scoll Value"};
	bitEntries[14] = reg14;
	static BitEntry reg15[] = {0x1E, "REG_BG3SCY", 0x3FF, "Scoll Value"};
	bitEntries[15] = reg15;

	//Background scaling/rotation regs
	static BitEntry reg16[] = {0x20, "REG_BG2DX", 0xFF, "X scale fraction", 0x7F00, "X scale integer", 0x8000, "Sign bit"};
	bitEntries[16] = reg16;
	static BitEntry reg17[] = {0x22, "REG_BG2DMX", 0xFF, "X shear fraction", 0x7F00, "X shear integer", 0x8000, "Sign bit"};
	bitEntries[17] = reg17;
	static BitEntry reg18[] = {0x24, "REG_BG2DY", 0xFF, "Y shear fraction", 0x7F00, "Y shear integer", 0x8000, "Sign bit"};
	bitEntries[18] = reg18;
	static BitEntry reg19[] = {0x26, "REG_BG2DMY", 0xFF, "Y scale fraction", 0x7F00, "Y scale integer", 0x8000, "Sign bit"};
	bitEntries[19] = reg19;
	static BitEntry reg20[] = {0x28, "REG_BG2X", 0xFF, "X coord fraction", 0x07FFFF00, "X coord integer", 0x80000000, "Sign bit"};
	bitEntries[20] = reg20;
	static BitEntry reg21[] = {0x2C, "REG_BG2Y", 0xFF, "Y coord fraction", 0x07FFFF00, "Y coord integer", 0x80000000, "Sign bit"};
	bitEntries[21] = reg21;
	
	//Background scaling/rotation regs
	static BitEntry reg22[] = {0x30, "REG_BG3DX", 0xFF, "X scale fraction", 0x7F00, "X scale integer", 0x8000, "Sign bit"};
	bitEntries[22] = reg22;
	static BitEntry reg23[] = {0x32, "REG_BG3DMX", 0xFF, "X shear fraction", 0x7F00, "X shear integer", 0x8000, "Sign bit"};
	bitEntries[23] = reg23;
	static BitEntry reg24[] = {0x34, "REG_BG3DY", 0xFF, "Y shear fraction", 0x7F00, "Y shear integer", 0x8000, "Sign bit"};
	bitEntries[24] = reg24;
	static BitEntry reg25[] = {0x36, "REG_BG3DMY", 0xFF, "Y scale fraction", 0x7F00, "Y scale integer", 0x8000, "Sign bit"};
	bitEntries[25] = reg25;
	static BitEntry reg26[] = {0x38, "REG_BG3X", 0xFF, "X coord fraction", 0x07FFFF00, "X coord integer", 0x80000000, "Sign bit"};
	bitEntries[26] = reg26;
	static BitEntry reg27[] = {0x3C, "REG_BG3Y", 0xFF, "Y coord fraction", 0x07FFFF00, "Y coord integer", 0x80000000, "Sign bit"};
	bitEntries[27] = reg27;

	//Windows
	static BitEntry reg28[] = {0x40, "REG_WIN0_H", 0xFF, "Right X coord", 0xFF00, "Left X coord"};
	bitEntries[28] = reg28;
	static BitEntry reg29[] = {0x42, "REG_WIN1_H", 0xFF, "Right X coord", 0xFF00, "Left X coord"};
	bitEntries[29] = reg29;
	static BitEntry reg30[] = {0x44, "REG_WIN0_V", 0xFF, "Bottom Y coord", 0xFF00, "Top Y coord"};
	bitEntries[30] = reg30;
	static BitEntry reg31[] = {0x46, "REG_WIN1_V", 0xFF, "Bottom Y coord", 0xFF00, "Top Y coord"};
	bitEntries[31] = reg31;
	static BitEntry reg32[] = {0x48, "REG_WIN_IN", B0, "BG0 in win 0", B1, "BG1 in win0",
						B2, "BG2 in win0", B3, "BG3 in win0", B4, "Sprites in win0",
						B5, "Blends in win0", B8, "BG0 in win1", B9, "BG1 in win1",
						B10, "BG2 in win1", B11, "BG3 in win1", B12, "Sprites in win1",
						B13, "Blends in win1"};
	bitEntries[32] = reg32;
	static BitEntry reg33[] = {0x4A, "REG_WIN_IN", B0, "BG0 outside", B1, "BG1 outside",
						B2, "BG2 outside", B3, "BG3 outside", B4, "Sprites outside",
						B5, "Blends outside", B8, "BG0 in sprite win", B9, "BG1 in sprite win",
						B10, "BG2 in sprite win", B11, "BG3 in sprite win", B12, "Sprites in sprite win",
						B13, "Blends in sprite win"};
	bitEntries[33] = reg33;

	//Special effects
	static BitEntry reg34[] = {0x4C, "REG_MOSAIC", 0xF, "BG X size", 0xF0, "BG Y size",
						0x0F00, "Sprite X size", 0xF000, "Sprite Y size"};
	bitEntries[34] = reg34;
	static BitEntry reg35[] = {0x50, "REG_BLDCNT", B0, "BG0 1st source", B1, "BG1 1st source",
						B2, "BG2 1st source", B3, "BG3 1st source", B4, "Sprites 1st source",
						B5, "Backdrop 1st source", B6 | B7, "Blend mode",
						B8, "BG0 2nd source", B9, "BG1 2nd source",
						B10, "BG2 2nd source", B11, "BG3 2nd source", B12, "Sprites 2nd source",
						B13, "Backdrop 2nd source"};
	bitEntries[35] = reg35;
	static BitEntry reg36[] = {0x52, "REG_BLDALPHA", 0x1F, "1st source pixel", 0x01F00, "Second source pixel"};
	bitEntries[36] = reg36;
	static BitEntry reg37[] = {0x54, "REG_BLDY", 0x1F, "Fade value"};
	bitEntries[37] = reg37;
	
	//Sound controls
	static BitEntry reg38[] = {0x60, "REG_SOUND1CNT_L", B0|B1|B2, "Num. Sweep Shifts", B3, "Sweep decrease",
						B4|B5|B6, "Sweep time"};
	bitEntries[38] = reg38;
	static BitEntry reg39[] = {0x62, "REG_SOUND1CNT_H", 0x3F, "Sound length", B6|B7, "Wave duty cycle",
						B8|B9|B10, "Envelope step time", B11, "Envelope increases",
						0xF000, "Initial envelope value"};
	bitEntries[39] = reg39;
	static BitEntry reg40[] = {0x64, "REG_SOUND1CNT_X", 0x7FF, "Sound frequency", B14, "Timed sound", B15, "Sound reset"};
	bitEntries[40] = reg40;
	static BitEntry reg41[] = {0x68, "REG_SOUND2CNT_L", 0x3F, "Sound length", B6|B7, "Wave duty cycle",
						B8|B9|B10, "Envelope step time", B11, "Envelope increases",
						0xF000, "Initial envelope value"};
	bitEntries[41] = reg41;
	static BitEntry reg42[] = {0x6C, "REG_SOUND2CNT_H", 0x7FF, "Sound frequency", B14, "Timed sound", B15, "Sound reset"};
	bitEntries[42] = reg42;
	static BitEntry reg43[] = {0x70, "REG_SOUND3CNT_L", B5, "Single bank mode", B6, "Bank select",
							B7, "Channel 3 ouput enabled"};
	bitEntries[43] = reg43;
	static BitEntry reg44[] = {0x72, "REG_SOUND3CNT_H", 0xFF, "Sound length", B15|B14|B13, "Output volume ratio"};
	bitEntries[44] = reg44;
	static BitEntry reg45[] = {0x74, "REG_SOUND3CNT_X", 0x7FF, "Sound frequency", B14, "Timed sound", B15, "Sound reset"};
	bitEntries[45] = reg45;
	static BitEntry reg46[] = {0x78, "REG_SOUND4CNT_L", 0x3F, "Sound length",
						B8|B9|B10, "Envelope step time", B11, "Envelope increases",
						0xF000, "Initial envelope value"};
	bitEntries[46] = reg46;
	static BitEntry reg47[] = {0x7C, "REG_SOUND4CNT_H", B0|B1|B2, "Clock divider frequency",
						B3, "Counter stages", 0xF0, "Counter pre-stepper freq.",
						B14, "Timed sound", B15, "Sound reset"};
	bitEntries[47] = reg47;
	static BitEntry reg48[] = {0x80, "REG_SOUNDCNT_L", B0|B1|B2, "DMG left volume", B3, "Vin left enabled",
						B4|B5|B6, "DMG right volume", B7, "Vin right enable", B8, "DMG Sound1 to left",
						B9, "DMG Sound2 to left", B10, "DMG Sound3 to left", B11, "DMG Sound 4 to left",
						B12, "DMG Sound1 to right", B13, "DMG Sound2 to right", B14, "DMG Sound 3 to right",
						B15, "DMG Sound 4 to right"};
	bitEntries[48] = reg48;
	static BitEntry reg49[] = {0x82, "REG_SOUNDCNT_H", B0|B1, "Channels 1-4 output ratio",
							B2, "Direct Sound A output ratio", B3, "Direct Sound B output ratio",
							B8, "Direct Sound A to right", B9, "Direct Sound A to left",
							B10, "Direct Sound A sampling timer", B11, "FIFO A reset",
							B12, "Direct Sound B to right", B13, "Direct Sound B to left",
							B14, "Direct Sound B sampling timer", B15, "FIFO B reset"};
	bitEntries[49] = reg49;
	static BitEntry reg50[] = {0x84, "REG_SOUNDCNT_X", B0, "DMG Sound 1 playing", B1, "DMG Sound 2 playing",
						B2, "DMG Sound 3 playing", B3, "DMG Sound 4 playing",
						B7, "All sound circuits enabled"};
	bitEntries[50] = reg50;
	static BitEntry reg51[] = {0x88, "REG_SOUNDBIAS", 0x1FF, "PWM bias value", B14|B15, "Amplitude resolutions"};
	bitEntries[51] = reg51;
	
	//Sound buffers
	static BitEntry reg52[] = {0x90, "REG_WAVE_RAM0_L", 0xF, "Sound 3 sample 0", 0xF0, "Sound3 sample 1",
						0xF00, "Sound 3 sample 2", 0xF000, "Sound 3 sample 3"};
	bitEntries[52] = reg52;
	static BitEntry reg53[] = {0x92, "REG_WAVE_RAM0_H", 0xF, "Sound 3 sample 4", 0xF0, "Sound3 sample 5",
						0xF00, "Sound 3 sample 6", 0xF000, "Sound 3 sample 7"};
	bitEntries[53] = reg53;
	static BitEntry reg54[] = {0x94, "REG_WAVE_RAM1_L", 0xF, "Sound 3 sample 8", 0xF0, "Sound3 sample 9",
						0xF00, "Sound 3 sample 10", 0xF000, "Sound 3 sample 11"};
	bitEntries[54] = reg54;
	static BitEntry reg55[] = {0x96, "REG_WAVE_RAM1_H", 0xF, "Sound 3 sample 12", 0xF0, "Sound3 sample 13",
						0x0F00, "Sound 3 sample 14", 0xF000, "Sound 3 sample 15"};
	bitEntries[55] = reg55;
	static BitEntry reg56[] = {0x98, "REG_WAVE_RAM2_L", 0xF, "Sound 3 sample 16", 0xF0, "Sound3 sample 17",
						0x0F00, "Sound 3 sample 18", 0xF000, "Sound 3 sample 19"};
	bitEntries[56] = reg56;
	static BitEntry reg57[] = {0x9A, "REG_WAVE_RAM2_H", 0xF, "Sound 3 sample 20", 0xF0, "Sound3 sample 21",
						0x0F00, "Sound 3 sample 22", 0xF000, "Sound 3 sample 23"};
	bitEntries[57] = reg57;
	static BitEntry reg58[] = {0x9C, "REG_WAVE_RAM3_L", 0xF, "Sound 3 sample 24", 0xF0, "Sound3 sample 25",
						0x0F00, "Sound 3 sample 26", 0xF000, "Sound 3 sample 27"};
	bitEntries[58] = reg58;
	static BitEntry reg59[] = {0x9E, "REG_WAVE_RAM3_H", 0xF, "Sound 3 sample 28", 0xF0, "Sound3 sample 29",
						0x0F00, "Sound 3 sample 30", 0xF000, "Sound 3 sample 31"};
	bitEntries[59] = reg59;
	static BitEntry reg60[] = {0xA0, "REG_FIFO_A_L", 0xFF, "Channel A sample 0", 0xFF00, "Channel A sample 1"};
	bitEntries[60] = reg60;						
	static BitEntry reg61[] = {0xA2, "REG_FIFO_A_H", 0xFF, "Channel A sample 2", 0xFF00, "Channel A sample 3"};
	bitEntries[61] = reg61;	
	static BitEntry reg62[] = {0xA4, "REG_FIFO_B_L", 0xFF, "Channel B sample 0", 0xFF00, "Channel B sample 1"};
	bitEntries[62] = reg62;						
	static BitEntry reg63[] = {0xA6, "REG_FIFO_B_H", 0xFF, "Channel B sample 2", 0xFF00, "Channel B sample 3"};
	bitEntries[63] = reg63;						
	
	//DMA registers
	static BitEntry reg64[] = {0xB0, "REG_DMA0SAD_L", 0xFFFF, "Source address low"};
	bitEntries[64] = reg64;
	static BitEntry reg100[] = {0xB2, "REG_DMA0SAD_H", 0x07FF, "Source address high"};
	bitEntries[100] = reg100;
	
	
	static BitEntry reg65[] = {0xB4, "REG_DMA0DST_L", 0xFFFF, "Destination address low"};
	bitEntries[65] = reg65;
	static BitEntry reg101[] = {0xB6, "REG_DMA0DST_H", 0x07FF, "Destination address high"};
	bitEntries[101] = reg101;

	static BitEntry reg66[] = {0xB8, "REG_DMA0SIZE", 0x3FFF, "Word/halfword count"};
	bitEntries[66] = reg66;
	static BitEntry reg67[] = {0xBA, "REG_DMA0CNT", B5|B6, "Destination increment/decrement",
						B7|B8, "Source increment/decrement", B9, "Repeat",
						B10, "0 = 16 bit, 1 = 32 bit transfers", B12|B13, "Start mode",
						B14, "Generate IRQ on completion", B15, "Enable DMA operation"};
	bitEntries[67] = reg67;
	
	static BitEntry reg68[] = {0xBC, "REG_DMA1SAD_L", 0xFFFF, "Source address low"};
	bitEntries[68] = reg68;
	static BitEntry reg102[] = {0xBE, "REG_DMA1SAD_H", 0x0FFF, "Source address high"};
	bitEntries[102] = reg102;

	static BitEntry reg69[] = {0xC0, "REG_DMA1DST_L", 0xFFFF, "Destination address low"};
	bitEntries[69] = reg69;
	static BitEntry reg103[] = {0xC2, "REG_DMA1DST_H", 0x07FF, "Destination address high"};
	bitEntries[103] = reg103;

	static BitEntry reg70[] = {0xC4, "REG_DMA1SIZE", 0x3FFF, "Word/halfword count"};
	bitEntries[70] = reg70;
	static BitEntry reg71[] = {0xC6, "REG_DMA1CNT", B5|B6, "Destination increment/decrement",
						B7|B8, "Source increment/decrement", B9, "Repeat",
						B10, "0 = 16 bit, 1 = 32 bit transfers", B12|B13, "Start mode",
						B14, "Generate IRQ on completion", B15, "Enable DMA operation"};
	bitEntries[71] = reg71;

	static BitEntry reg72[] = {0xC8, "REG_DMA2SAD_L", 0xFFFF, "Source address low"};
	bitEntries[72] = reg72;
	static BitEntry reg104[] = {0xCA, "REG_DMA2SAD_H", 0x0FFF, "Source address high"};
	bitEntries[104] = reg104;
	static BitEntry reg73[] = {0xCC, "REG_DMA2DST_L", 0xFFFF, "Destination address low"};
	bitEntries[73] = reg73;
	static BitEntry reg105[] = {0xCE, "REG_DMA2DST_H", 0x07FF, "Destination address high"};
	bitEntries[105] = reg105;
	static BitEntry reg74[] = {0xD0, "REG_DMA2SIZE", 0x3FFF, "Word/halfword count"};
	bitEntries[74] = reg74;
	static BitEntry reg75[] = {0xD2, "REG_DMA2CNT", B5|B6, "Destination increment/decrement",
						B7|B8, "Source increment/decrement", B9, "Repeat",
						B10, "0 = 16 bit, 1 = 32 bit transfers", B12|B13, "Start mode",
						B14, "Generate IRQ on completion", B15, "Enable DMA operation"};
	bitEntries[75] = reg75;

	static BitEntry reg76[] = {0xD4, "REG_DMA3SAD_L", 0xFFFF, "Source address low"};
	bitEntries[76] = reg76;
	static BitEntry reg106[] = {0xD6, "REG_DMA3SAD_H", 0x0FFF, "Source address high"};
	bitEntries[106] = reg106;

	static BitEntry reg77[] = {0xD8, "REG_DMA3DST_L", 0xFFFF, "Destination address low"};
	bitEntries[77] = reg77;
	static BitEntry reg107[] = {0xDA, "REG_DMA3DST_H", 0x0FFF, "Destination address high"};
	bitEntries[107] = reg107;

	static BitEntry reg78[] = {0xDC, "REG_DMA3SIZE", 0x3FFF, "Word/halfword count"};
	bitEntries[78] = reg78;
	static BitEntry reg79[] = {0xDE, "REG_DMA3CNT", B5|B6, "Destination increment/decrement",
						B7|B8, "Source increment/decrement", B9, "Repeat",
						B10, "0 = 16 bit, 1 = 32 bit transfers", B12|B13, "Start mode",
						B14, "Generate IRQ on completion", B15, "Enable DMA operation"};
	bitEntries[79] = reg79;

	//Timer registers
	static BitEntry reg80[] = {0x100, "REG_TM0DAT", 0xFFFF, "Count value"};
	bitEntries[80] = reg80;
	static BitEntry reg81[] = {0x102, "REG_TM0CNT", B0|B1, "Frequency", B2, "Cascade",
						B6, "Generate IRQ on overflow", B7, "Timer enabled"};
	bitEntries[81] = reg81;
	static BitEntry reg82[] = {0x104, "REG_TM1DAT", 0xFFFF, "Count value"};
	bitEntries[82] = reg82;
	static BitEntry reg83[] = {0x106, "REG_TM1CNT", B0|B1, "Frequency", B2, "Cascade",
						B6, "Generate IRQ on overflow", B7, "Timer enabled"};
	bitEntries[83] = reg83;
	static BitEntry reg84[] = {0x108, "REG_TM2DAT", 0xFFFF, "Count value"};
	bitEntries[84] = reg84;
	static BitEntry reg85[] = {0x10A, "REG_TM2CNT", B0|B1, "Frequency", B2, "Cascade",
						B6, "Generate IRQ on overflow", B7, "Timer enabled"};
	bitEntries[85] = reg85;
	static BitEntry reg86[] = {0x10C, "REG_TM3DAT", 0xFFFF, "Count value"};
	bitEntries[86] = reg86;
	static BitEntry reg87[] = {0x10E, "REG_TM3CNT", B0|B1, "Frequency", B2, "Cascade",
						B6, "Generate IRQ on overflow", B7, "Timer enabled"};
	bitEntries[87] = reg87;

	//Data link registers.
	static BitEntry reg108[] = {0x120, "REG_SCD0", 0xFFFF, "Unknown"};
	bitEntries[108] = reg108;
	static BitEntry reg109[] = {0x122, "REG_SCD1", 0xFFFF, "Unknown"};
	bitEntries[109] = reg109;
	static BitEntry reg110[] = {0x124, "REG_SCD2", 0xFFFF, "Unknown"};
	bitEntries[110] = reg110;
	static BitEntry reg111[] = {0x126, "REG_SCD2", 0xFFFF, "Unknown"};
	bitEntries[111] = reg111;
	static BitEntry reg112[] = {0x128, "REG_SCCNT_L", 0xFFFF, "Unknown"};
	bitEntries[112] = reg112;
	static BitEntry reg113[] = {0x12A, "REG_SCCNT_H", 0xFFFF, "Unknown"};
	bitEntries[113] = reg113;


	//Key registers
	static BitEntry reg88[] = {0x130, "REG_KEY", B0, "A released", B1, "B released",
						B2, "Select released", B3, "Start released",
						B4, "D-pad right released", B5, "D-pad left released",
						B6, "D-pad up released", B7, "D-pad down released",
						B8, "Right shoulder released", B9, "Left Shoulder released"};
	bitEntries[88] = reg88;
	static BitEntry reg89[] = {0x132, "REG_KEYCNT", B0, "A", B1, "B",
						B2, "Select", B3, "Start",
						B4, "D-pad right", B5, "D-pad left",
						B6, "D-pad up", B7, "D-pad down",
						B8, "Right shoulder", B9, "Left Shoulder",
						B14, "Generate interrupt on kepress", B15, "Interrupt generation type"};
	bitEntries[89] = reg89;

	//Unknown
	static BitEntry reg114[] = {0x134, "REG_R", 0xFFFF, "Unknown"};
	bitEntries[114] = reg114;
	static BitEntry reg115[] = {0x140, "REG_HS_CTRL", 0xFFFF, "Unknown"};
	bitEntries[115] = reg115;
	static BitEntry reg116[] = {0x150, "REG_JOYRE_L", 0xFFFF, "Unknown"};
	bitEntries[116] = reg116;
	static BitEntry reg117[] = {0x152, "REG_JOYRE_H", 0xFFFF, "Unknown"};
	bitEntries[117] = reg117;
	static BitEntry reg118[] = {0x154, "REG_JOYTR_L", 0xFFFF, "Unknown"};
	bitEntries[118] = reg118;
	static BitEntry reg119[] = {0x156, "REG_JOYTR_H", 0xFFFF, "Unknown"};
	bitEntries[119] = reg119;
	static BitEntry reg120[] = {0x158, "REG_JSTAT_L", 0xFFFF, "Unknown"};
	bitEntries[120] = reg120;
	static BitEntry reg121[] = {0x15A, "REG_JSTAT_H", 0xFFFF, "Unknown"};
	bitEntries[121] = reg121;



	//Interrupt registers
	static BitEntry reg90[] = {0x200, "REG_IE", B0, "VBlank IRQ", B1, "HBlank IRQ",
						B2, "Vcount IRQ", B3, "Timer0 IRQ", B4, "Timer1 IRQ",
						B5, "Timer 2 IRQ", B6, "Timer 3 IRQ", B7, "Serial comm. IRQ",
						B8, "DMA0 IRQ", B9, "DMA1 IRQ", B10, "DMA2 IRQ", B11, "DMA3 IRQ",
						B12, "Key IRQ", B13, "Cassette IRQ"};
	bitEntries[90] = reg90;
	static BitEntry reg91[] = {0x202, "REG_IF", B0, "VBlank IRQ", B1, "HBlank IRQ",
						B2, "Vcount IRQ", B3, "Timer0 IRQ", B4, "Timer1 IRQ",
						B5, "Timer 2 IRQ", B6, "Timer 3 IRQ", B7, "Serial comm. IRQ",
						B8, "DMA0 IRQ", B9, "DMA1 IRQ", B10, "DMA2 IRQ", B11, "DMA3 IRQ",
						B12, "Key IRQ", B13, "Cassette IRQ"};
	bitEntries[91] = reg91;
	
	static BitEntry reg92[] = {0x204, "REG_WSCNT", B0|B1, "SRAM mode", B2|B3|B4, "Bank 0",
						B5|B6|B7, "Bank 1", B8|B9|B10, "Bank2", B11|B12, "Cart Clock",
						B14, "Prefetch", B15, "Game Pack Type"};
	bitEntries[92] = reg92;
	
	static BitEntry reg93[] = {0x208, "REG_IME", B0, "Master interrupt enable"};
	bitEntries[93] = reg93;

	static BitEntry reg94[] = {0x300, "REG_PAUSE", B14, "Mode", B15, "Power Down"};
	bitEntries[94] = reg94;

	static BitEntry reg95[] = {0x3C0, "REG_EMUIN_L", 0xFFFF, "(Cowbite)EMU Input Addr.L"};
	bitEntries[95] = reg95;

	static BitEntry reg96[] = {0x3C2, "REG_EMUIN_H", 0xFFFF, "(Cowbite)EMU Input Addr.H"};
	bitEntries[96] = reg96;

	static BitEntry reg97[] = {0x3C4, "REG_EMUOUT_L", 0xFFFF, "(Cowbite)EMU Output Addr.L"};
	bitEntries[97] = reg97;

	static BitEntry reg98[] = {0x3C6, "REG_EMUOUT_H", 0xFFFF, "(Cowbite)EMU Output Addr.H"};
	bitEntries[98] = reg98;
	
	static BitEntry reg99[] = {0x3C8, "REG_EMUCNT", 0xFFFF, "(Cowbite)EMU Control"};
	bitEntries[99] = reg99;


	//I allocated all of these arrays statically in order to save myself the effort of
	//creating a custom-formatted file as found in Mappy.  However, I pay the price
	//in that there is no easy way to determine the size of each entry.
	u32 size[NUMHARDWAREREGS];
	size[0] = sizeof(reg0);	size[1] = sizeof(reg1);	size[2] = sizeof(reg2);
	size[3] = sizeof(reg3);	size[4] = sizeof(reg4);	size[5] = sizeof(reg5);
	size[6] = sizeof(reg6);	size[7] = sizeof(reg7);	size[8] = sizeof(reg8);
	size[9] = sizeof(reg9);	size[10] = sizeof(reg10);	size[11] = sizeof(reg11);
	size[12] = sizeof(reg12);	size[13] = sizeof(reg13);	size[14] = sizeof(reg14);
	size[15] = sizeof(reg15);	size[16] = sizeof(reg16);	size[17] = sizeof(reg17);
	size[18] = sizeof(reg18);	size[19] = sizeof(reg19);	size[20] = sizeof(reg20);
	size[21] = sizeof(reg21);	size[22] = sizeof(reg22);	size[21] = sizeof(reg21);
	size[24] = sizeof(reg24);	size[25] = sizeof(reg25);	size[26] = sizeof(reg26);
	size[25] = sizeof(reg25);	size[26] = sizeof(reg26);	size[27] = sizeof(reg27);
	size[28] = sizeof(reg28);	size[29] = sizeof(reg29);	size[30] = sizeof(reg30);
	size[31] = sizeof(reg31);	size[32] = sizeof(reg32);	size[33] = sizeof(reg33);
	size[34] = sizeof(reg34);	size[35] = sizeof(reg35);	size[36] = sizeof(reg36);
	size[37] = sizeof(reg37);	size[38] = sizeof(reg38);	size[39] = sizeof(reg39);
	size[40] = sizeof(reg40);	size[41] = sizeof(reg41);	size[42] = sizeof(reg42);
	size[43] = sizeof(reg43);	size[44] = sizeof(reg44);	size[45] = sizeof(reg45);
	size[46] = sizeof(reg46);	size[47] = sizeof(reg47);	size[48] = sizeof(reg48);
	size[49] = sizeof(reg49);	size[50] = sizeof(reg50);	size[51] = sizeof(reg51);
	size[52] = sizeof(reg52);	size[53] = sizeof(reg53);	size[54] = sizeof(reg54);
	size[55] = sizeof(reg55);	
	size[56] = sizeof(reg56);	size[57] = sizeof(reg57);	size[58] = sizeof(reg58);
	size[59] = sizeof(reg59);	size[60] = sizeof(reg60);	size[61] = sizeof(reg61);
	size[62] = sizeof(reg62);	size[63] = sizeof(reg63);	size[64] = sizeof(reg64);
	size[65] = sizeof(reg65);	size[66] = sizeof(reg66);	size[67] = sizeof(reg67);
	size[68] = sizeof(reg68);	size[69] = sizeof(reg69);	size[70] = sizeof(reg70);
	size[71] = sizeof(reg71);	size[72] = sizeof(reg72);	size[73] = sizeof(reg73);
	size[74] = sizeof(reg74);	size[75] = sizeof(reg75);	size[76] = sizeof(reg76);
	size[77] = sizeof(reg77);	size[78] = sizeof(reg78);	size[79] = sizeof(reg79);
	size[80] = sizeof(reg80);	size[81] = sizeof(reg81);	size[82] = sizeof(reg82);
	size[83] = sizeof(reg83);	size[84] = sizeof(reg84);	size[85] = sizeof(reg85);
	size[86] = sizeof(reg86);	size[87] = sizeof(reg87);	size[88] = sizeof(reg88);
	size[89] = sizeof(reg89);	size[90] = sizeof(reg90);	size[91] = sizeof(reg91);
	size[92] = sizeof(reg92);	size[93] = sizeof(reg93);	size[94] = sizeof(reg94);
	size[95] = sizeof(reg95);	size[96] = sizeof(reg96);	size[97] = sizeof(reg97);
	size[98] = sizeof(reg98);	size[99] = sizeof(reg99);	size[100] = sizeof(reg100);
	size[101] = sizeof(reg101);	size[102] = sizeof(reg102);	size[103] = sizeof(reg103);
	size[104] = sizeof(reg104);	size[105] = sizeof(reg105);	size[106] = sizeof(reg106);
	size[107] = sizeof(reg107);	size[108] = sizeof(reg108);	size[109] = sizeof(reg109);
	size[110] = sizeof(reg110);	size[111] = sizeof(reg111);	size[112] = sizeof(reg112);
	size[113] = sizeof(reg113);	size[114] = sizeof(reg114);	size[115] = sizeof(reg115);
	size[116] = sizeof(reg116);	size[117] = sizeof(reg117);	size[118] = sizeof(reg118);
	size[119] = sizeof(reg119);	size[120] = sizeof(reg120);	size[121] = sizeof(reg121);
	//size[122] = sizeof(reg122);
	
	
	memset(gbaDebugger.regEntries, 0, sizeof(RegEntry));	//Initialize them to zero.
/*	RegEntry* regEntry = &gbaDebugger.regEntries[0];
	gbaDebugger.regEntries[0].index = 0;
	gbaDebugger.regEntries[0].numEntries = size[0]/sizeof(BitEntry) - 1;
	gbaDebugger.regEntries[0].offset = bitEntries[0][0].mask;
	gbaDebugger.regEntries[0].name = bitEntries[0][0].name;
	gbaDebugger.regEntries[0].bitEntries = & (bitEntries[0][1]);*/
	
	//Now put all of this CRAP into a coherent form.
	for (int i = 0; i < NUMHARDWAREREGS; i++) {
		//Order is not important; they eventually get ordered by a list box.

		gbaDebugger.regEntries[i].index = i;
		gbaDebugger.regEntries[i].numEntries = size[i]/sizeof(BitEntry) - 1;
		gbaDebugger.regEntries[i].offset = bitEntries[i][0].mask;
		gbaDebugger.regEntries[i].name = bitEntries[i][0].name;
		//Start our list at the first entry.
		gbaDebugger.regEntries[i].bitEntries = & (bitEntries[i][1]);
		
	}
		
}
