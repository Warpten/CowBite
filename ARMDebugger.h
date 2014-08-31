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

#ifndef ARMDEBUGGER_H
#define ARMDEBUGGER_H

#include "GBA.h"
#include "ARM.h"

#define DEBUG_MAXLINES 20	//was 26
#define DEBUG_MAXMEMLINES 16
#define DEBUG_MAXUNDO 10	

//This class works in conjunction with the ARM class
//in order to provide debugging information.  Although the ARM class does
//all the work, the debugger class provides an interface to the calling program.
class ARMDebugger {
	public:
		ARMDebugger (GBA* nGBA);
		~ARMDebugger();
		
		void addBreakpoint(u32 breakPoint);
		void removeBreakpoint (u32 breakPoint);
		void addConditionalBreakpoint(ConditionalBreakpoint *breakPoint);
		void removeConditionalBreakpoint (ConditionalBreakpoint *breakPoint);
		//void run();
		//void step();
		void reset();
		//void runTo (u32 address);
		void saveUndo();	//Saves the current state
		void undo();		//Retrieves the previous saved state.


		char *hexToString(u32 value);
		char *hexToString16(u16 value);
		char* parseInstruction(u32 currentAddress, u32 instruction);
		char** parseInstructionContext();
		char** parseInstructionContext(u32 instruction);
		char** parseRegisters();
		char** parseMemory();
		char** parseMemory(u32 location);
		char* disassembleThumb(u32 address);
		char* disassembleARM(u32 address);
		void outputDisassembler(char* filename, u8* source, u32 startAddress, u32 endAddress, u32 thumbMode);
		char* condBreakpointToString(ConditionalBreakpoint* breakpoint);
		char* breakpointValueToString(BreakpointValue* value);

		char *parseALUOpcodes(u32 opcode, char* rd, char* rn, char* op2);
		char* parseDataOperation(u32 currentAddress, u32 instruction, char* condition);
		char* parseLoadStore(u32 currentAddress, u32 instruction, char* condition);
		char* parseLoadStoreHalfword(u32 currentAddress, u32 instruction, char* condition);
		char* parseLoadStoreMultiple(u32 currentAddress, u32 instruction, char* condition);
		char* parseSwap(u32 currentAddress, u32 instruction, char* condition);
		char* parseMul(u32 currentAddress, u32 instruction, char* condition);
		char* parseThumb(u32 currentAddress, u32 instruction);
	
		//GBA* gba;
		char parsedInstruction[255];

		//An array of registers
		char regArray[17][255];
		
		//An array of memory addresses
		char memArray[DEBUG_MAXMEMLINES][255];	
		char memVal[255];	//A place to store the specific value at that address
		char locationVal[255];	//A place to store the location
		
		
		u32 browseInstructionAddress;	//Address of current instruction being browsed
		u32 browseInstructionMemSize;
		u32 browseMemAddress;			//Address of current memory address being browsed
		u32 browseMemSize;				//Size of the current part of memory being browsed
		u32 browsingProgram;			//Flag to let is know if the user is browsing CPU instructions
		u32 useAutoLocation;	//Tells it to auomatically pick the memory location
								//to parse.
		//u32 lastAccessedAddress;	//For the auto update feature

		//An array of instructions
		char instructionArray[DEBUG_MAXLINES][255];

		char* conditions[16];
		char* aluops[16];
		char* shifts[4];
		
		GBAState* undoStates[DEBUG_MAXUNDO];
		int numUndoes;
		int lastUndo;
};

#endif