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

#ifndef GBADEBUGGER_H
#define GBADEBUGGER_H

#include "Constants.h"

#define NUMHARDWAREREGS 122

typedef struct tBitEntry {
	u32 mask;
	char* name;
} BitEntry;

typedef struct tRegEntry {
	u32 index;	//Index of this register in the list
	u32 offset;	//Offset of this register in IO_RAM
	char* name;	//Name of the register
	u32 numEntries;	//Number of bit fields in this register
	u32 value;		//Current value of this register
	BitEntry* bitEntries;
	struct tRegEntry* next;
} RegEntry;

//typedef struct tRegInfo {
//	bit

//}

typedef struct tGBADebugger {
	RegEntry regEntries[NUMHARDWAREREGS];
	RegEntry currentReg;	//The current register being browsed.
	//BitEntry* bitEntries[NUMHARDWAREREGS];	//An array of arrays of bitEntries
} GBADebugger;

extern GBADebugger gbaDebugger;

void GBADebugger_init();
void GBADebugger_initRegStrings();
RegEntry* GBADebugger_getRegEntry (u32 offset);
RegEntry* GBADebugger_getRegEntry (char* name);

//u32 GBADebugger_getRegOffset(u32 index);
char* GBADebugger_hexToBinString(u32 number, u32 numBits);
char* GBADebugger_regToString(u32 offset);
char* GBADebugger_getRegInfo();
char* GBADebugger_getStats();
char* GBADebugger_getMiscStats();
char* GBADebugger_getMemoryStats();
char* GBADebugger_getInterruptStats();
char* GBADebugger_getSoundStats();


#endif
