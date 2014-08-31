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

#include <stdio.h>
#include <string.h>
#include "ARM.h"
#include "ARMDebugger.h"
#include "GBADebugger.h"
#include "Support.h"
#include "ALU.h"
#include "Memory.h"
#include "Graphics.h"
#include <windows.h>

///////////////////////////////////////////
//Construction
//////////////////////////////////////////
ARMDebugger::ARMDebugger (GBA* nGBA) {

	conditions[0] = "EQ";
	conditions[1] = "NE";
	conditions[2] = "CS";
	conditions[3] = "CC";
	conditions[4] = "MI";
	conditions[5] = "PL";
	conditions[6] = "VS";
	conditions[7] = "VC";
	conditions[8] = "HI";
	conditions[9] = "LS";
	conditions[10] = "GE";
	conditions[11] = "LT";
	conditions[12] = "GT";
	conditions[13] = "LE";
	conditions[14] = "";
	conditions[15] = "";
	
	aluops[OP_AND] = "AND";
	aluops[OP_EOR] = "EOR";
	aluops[OP_SUB] = "SUB";
	aluops[OP_RSB] = "RSB";
	aluops[OP_ADD] = "ADD";
	aluops[OP_ADC] = "ADC";
	aluops[OP_SBC] = "SBC";
	aluops[OP_RSC] = "RSC";
	aluops[OP_TST] = "TST";
	aluops[OP_TEQ] = "TEQ";
	aluops[OP_CMP] = "CMP";
	aluops[OP_CMN] = "CMN";
	aluops[OP_ORR] = "ORR";
	aluops[OP_MOV] = "MOV";
	aluops[OP_BIC] = "BIC";
	aluops[OP_MVN] = "MVN";
	
	shifts[0] = "LSL";
	shifts[1] = "LSR";
	shifts[2] = "ASR";
	shifts[3] = "ROR";

	//Initialize each level of the UNDO queue to null
	for (int i = 0; i< DEBUG_MAXUNDO; i++)
		undoStates[i] = NULL;

	lastUndo = 0;
	numUndoes = 0;
	browseInstructionAddress = WRAM_EXT_START;	//Since ROM will be empty by default
	
	browseMemAddress = ROM_START;
	sprintf (memVal, "%08X", 0);
	sprintf (locationVal, "%08X", WRAM_EXT_START);	//ROM will be empty by default

	//use auto location to begin with
	useAutoLocation = 1;
	
	//Keep track of the memory size limits so that we don't try to debug
	//areas of memory that don't exist.
	if (gba.loaded) 
		browseMemSize = gbaMem.memorySize[9];
	else
		browseMemSize = 0;

	browseInstructionMemSize = browseMemSize;
	browsingProgram = 0;
}

///////////////
//Destructor
//////////////
ARMDebugger::~ARMDebugger () {
	for (int i = 0; i<DEBUG_MAXUNDO; i++)
		if (undoStates[i] != NULL)
			delete undoStates[i];


}

////////////////////////////////////////
//addBreakpoint()
//This adds a breakpoint the CPU will catch when running
//in debug mode.
////////////////////////////////////////
void ARMDebugger::addBreakpoint(u32 breakpoint) {
	//put them in order from greatest to least
	int done = 0;
	if (arm.numBreakpoints == MAX_BREAKPOINTS)	//If we already have too many...
		return;
	if (ARM_containsBreakpoint(breakpoint))
		return;
	
	//This is clugey, crappy code.
	if (arm.numBreakpoints == 0) {
		arm.breakpoints[0] = breakpoint;
		arm.numBreakpoints = 1;
	} else {
		//Loop until we find where the breakpoint goes.
		for (int i = 0; (breakpoint < arm.breakpoints[i]) && (i < arm.numBreakpoints); i++) {}
		
		//Move the rest of the breakpoints over to make room (loop test will fail before
		//first iteration if we are already at the end).
		for (int j = arm.numBreakpoints; j > i; j--)
			arm.breakpoints[j] = arm.breakpoints[j-1];
		
		arm.breakpoints[i] = breakpoint;
		arm.numBreakpoints++;
	}
	
}

////////////////////////////////////////
//removeBreakpoint()
//This removes a breakpoint from the CPU
////////////////////////////////////////
void ARMDebugger::removeBreakpoint(u32 breakpoint) {
	
	int done = 0;
	if (arm.numBreakpoints == 0)	//If there's no breakpoints, don't worry
		return;
	for (int i = 0; i<arm.numBreakpoints && !done; i++) {
		//If we find it, shift everybody over.
		if (breakpoint == arm.breakpoints[i]) {
			//move all the other breakpoints over.
			for (int j = i; j < arm.numBreakpoints; j++) {
				arm.breakpoints[j] = arm.breakpoints[j+1];
				arm.breakpoints[j+1] = 0;
			}
			arm.numBreakpoints--;
			done = 1;
		} 
	}
}


////////////////////////////////////////
//addConditionalBreakpoint()
//This adds a conditioanl breakpoint the CPU will catch when running
//in debug mode.
////////////////////////////////////////
void ARMDebugger::addConditionalBreakpoint(ConditionalBreakpoint* breakpoint) {
	//Do not bother ordering conditional breakpoints
	int done = 0;
	if (arm.numCondBreakpoints == MAX_CONDBREAKPOINTS)	//If we already have too many...
		return;
	
	for (int i = 0; i < arm.numCondBreakpoints; i++) {
		//If it's identical to something already in the list
		if (!memcmp(&arm.condBreakpoints[i],breakpoint,sizeof(ConditionalBreakpoint)))
			return;
	}
	//it wasn't identical to something alraedy in the list, so we can add it.
	memcpy(&arm.condBreakpoints[arm.numCondBreakpoints],breakpoint,sizeof(ConditionalBreakpoint));
	arm.numCondBreakpoints++;
}

////////////////////////////////////////
//removeConditionalBreakpoint()
//This removes a conditional breakpoint from the CPU
////////////////////////////////////////
void ARMDebugger::removeConditionalBreakpoint(ConditionalBreakpoint* breakpoint) {
	
	int done = 0;
	if (arm.numCondBreakpoints == 0)	//If there's no breakpoints, don't worry
		return;
	for (int i = 0; i<arm.numCondBreakpoints && !done; i++) {
		//If we find it, shift everybody over.
		if (!memcmp(&arm.condBreakpoints[i],breakpoint,sizeof(ConditionalBreakpoint))) {
			//move all the other breakpoints over.
			for (int j = i; j < arm.numCondBreakpoints; j++) {
				memcpy(&arm.condBreakpoints[j], &arm.condBreakpoints[j+1], sizeof(ConditionalBreakpoint));
				memset(&arm.condBreakpoints[j], 0, sizeof(ConditionalBreakpoint));
			}
			arm.numCondBreakpoints--;
			done = 1;
		} 
	}
}


//This runs the CPU up to a specified location
/*void ARMDebugger::runTo (u32 location) {
	u32 index;
	//u32 bogusTime = 0;
	//LARGE_INTEGER largeInt;
	
	saveUndo();

	index = location >> 24;

	if (!gba.loaded)
		return;

	if (index < 0 || index > 0xF)	//Make sure the user doesn't try something dumb
		return;
	
	GBA_stepAndUpdate();	//Go to the instruction after -- otherwise, if you specify the
			//curent instructino as the one to run to, it'll just sit there.

	int done = 0;
	if (arm.thumbMode)	//Check taht we're not already there
		done = (( arm.r[15] - 4) == location);
	else 
		done = (( arm.r[15] - 8) == location);
	
	while ( !done) {
		
		GBA_stepAndUpdate();
		if (arm.thumbMode)
			done = (( arm.r[15] - 4) == location);
		else 
			done = (( arm.r[15] - 8) == location);
			 
	}

	
}*/
/*
//This steps the CPU
void ARMDebugger::step() {
	//LARGE_INTEGER largeInt;
	
	if (!gba.loaded)
		return;

	saveUndo();
	GBA_stepAndUpdate();

}*/


////////////////////////////////////////////////
//This resets attributes in the debugger.
///////////////////////////////////////////////////
void ARMDebugger::reset() {
	//Initialize each level of the UNDO queue to null
	for (int i = 0; i< DEBUG_MAXUNDO; i++) {
		delete undoStates[i];
		undoStates[i] = NULL;
	}

	lastUndo = 0;
	numUndoes = 0;
}

///////////////////////////////////
//This saves the state of the cpu into a revolvoing queue.
///////////////////////////////////
void ARMDebugger::saveUndo() {
	//If we already used up all our undoes, we need to do some shifting.
	if ( lastUndo == DEBUG_MAXUNDO ) {
		
		if (undoStates[0] != NULL)
			delete undoStates[0];	//Get rid of the first one in the queue

		//Shift everyone down
		for (int i = 0; i < DEBUG_MAXUNDO-1; i++) {
			undoStates[i] = undoStates[i+1];
		}

		//Allocate memory for the new undo state.
		undoStates[lastUndo-1] = new GBAState;
		GBA_getState (undoStates[lastUndo-1]);
	
	} else if (undoStates[lastUndo] == NULL) {
		undoStates[lastUndo] = new GBAState;
		
		GBA_getState (undoStates[lastUndo]);
		
		lastUndo++;
	}
}

/////////////////////////////////////
//This reloads the previous state
/////////////////////////////////////
void ARMDebugger::undo() {
	if (lastUndo != 0) {
		//We want to set the pervious state
		GBA_setState (undoStates[lastUndo-1]);
		delete undoStates[lastUndo-1];
		undoStates[lastUndo-1] = NULL;
		lastUndo--;
	}
}



///////////////////////////////////////////////
//hexToString
//returns a nicely formated 32 bit hex string
///////////////////////////////////////////////
char* ARMDebugger::hexToString (u32 value) {
	static char hexValue [255];
	
	strcpy (hexValue,"");
	sprintf (hexValue, "%08X", value);
	return hexValue;

}

///////////////////////////////////////////////
//hexToString16
//returns a nicely formated 16 bit hex string
///////////////////////////////////////////////
char* ARMDebugger::hexToString16 (u16 value) {
	static char hexValue [255];
	
	strcpy (hexValue,"");
	sprintf (hexValue, "%04X", value);
	return hexValue;

}



////////////////////////////////////////////////////////////////////////
//parseInstruction
//This function parses an instruction and returns a string representation
//////////////////////////////////////////////////////////////////////
char* ARMDebugger::parseInstruction (u32 currentAddress, u32 instruction) {
	u32 conditionI, typeI, rnI, rdI, rmI, rsI;
	int offset;
	char type[255];
	char condition[255];
	char immediate[255];
	char rn[10], rd[10], rm[10], rs[10];
	char* functionName;
	//Attach the hex value for the string
	strcpy (parsedInstruction, "");
	strcat (parsedInstruction, hexToString (instruction));

	conditionI = instruction >> 28;	//Condition is the top 4 bits
	
	//Get the string for the condition code
	sprintf (condition, conditions[conditionI]);
	
	
	typeI = Support_getBits (instruction, 27,25);
	

	rnI = Support_getBits (instruction, 19, 16);
	sprintf  (rn, "r%d", rnI);
	rdI = Support_getBits (instruction, 15, 12);
	sprintf(rd, "r%d", rdI);
	rsI = Support_getBits (instruction, 11, 8);
	sprintf (rs, "r%d", rsI);
	rmI = Support_getBits (instruction, 3, 0);
	sprintf (rm, "r%d", rmI);

	strcpy (type, "");

	//Switch on the type, then divvy up the work accordingly.
		
	switch (typeI) {
		case 0x0:		if (SUPPORT_BITCOMPARE(instruction, 0x0FC000F0, 0x00000090)) {
							sprintf(type, parseMul (currentAddress, instruction, condition));
							break;
						} 
						
						if ( SUPPORT_BITCOMPARE(instruction, 0x0F8000F0, 0x00800090)) {
							if (SUPPORT_GETBIT22(instruction)) {	//if signed
								if (SUPPORT_GETBIT21(instruction))	//if accum
									sprintf(type, "SMLAL %s, %s, %s, %s", rd, rn, rm, rs);
								else
									sprintf(type, "SMULL %s, %s, %s, %s", rd, rn, rm, rs);
							} else {
								if (SUPPORT_GETBIT21(instruction))	//if accum
									sprintf(type, "UMLAL %s, %s, %s, %s", rd, rn, rm, rs);
								else
									sprintf(type, "UMULL %s, %s, %s, %s", rd, rn, rm, rs);
							}
							break;
						}

						else if (SUPPORT_BITCOMPARE(instruction, 0x0FC000F0, 0x00000090)) {
								sprintf(type, parseSwap(currentAddress, instruction, condition));
								break;
						}
						else if (SUPPORT_BITCOMPARE(instruction, 0x0E000090, 0x00000090)) {
								sprintf(type, parseLoadStoreHalfword(currentAddress, instruction, condition));
								break;
						}
						//If it passes these, check to see if it's a BX.  Note that the
						//only thing to distinguish BX from an MSR using our shortened 16 bit
						//codes is the fact that in an MSR the rd can never be r15
						else if (SUPPORT_BITCOMPARE(instruction, 0x0FF00FF0, 0x01200F10)) {
							sprintf(type, "BX%s r%d", condition, instruction & 0xF);
							break;
						}
						
			case 0x1:	//sprintf(type, "Data");
						sprintf (type, parseDataOperation (currentAddress, instruction, condition));
						break;
						

			case 0x5:	//Find if it's a branch and link or just a branch
						if (SUPPORT_GETBIT24(instruction))
							sprintf(type, "BL");
						else
							sprintf(type, "B");
						strcat (type, condition);
						strcat (type, " ");
						offset = instruction & 0x00FFFFFF;
						offset = offset << 8;
						offset = offset  >> 6;
						sprintf(immediate, "0x%08X", (currentAddress + offset+8));
						strcat (type, immediate);
							
						//Try to get a function name from the elf file
						functionName = ELFFile_getFunctionName(&gba.elfFile, (currentAddress + offset+8));
						
						//If it's not null, we got a function!
						if (functionName != NULL) {
							//Spit it into the output.
							strcat (type, " (");
							strcat(type, functionName);
							strcat (type, "())");
						} 	
						
						break;
			case 0x2:
			case 0x3:	sprintf(type, parseLoadStore(currentAddress, instruction, condition));
						break;
			case 0x4:	sprintf(type, parseLoadStoreMultiple(currentAddress, instruction, condition));
						break;
						break;
			case 0x7:	
			case 0x6:	sprintf(type, "CoPro");
						strcat (type, condition);
						break;
			default:	sprintf(type, "UNK");
						strcat (type, condition);
						break;		
			
	}
	strcat (parsedInstruction, " ");
	strcat (parsedInstruction, type);
	//


	return parsedInstruction;
}

////////////////////////////////////////////////////////////////////
//parseInstructionContext
//This parses an instruction and several instructions before and after
//the current instruction.
////////////////////////////////////////////////////////////////////
char** ARMDebugger::parseInstructionContext () {
	//Unless the user is currently browsing with the scroll bar, assume that
	//we want to parse from the PC.
	if (!browsingProgram)
		browseInstructionAddress = arm.r[15];
	return parseInstructionContext(browseInstructionAddress);
}

////////////////////////////////////////////////////////////////////
//parseInstructionContext
//This parses an instruction and several instructions before and after
////////////////////////////////////////////////////////////////////
char** ARMDebugger::parseInstructionContext (u32 address) {
	//First get the contents of the PC	
//	u32 instruction;
	u32 currentAddress;
	//if (!gba.loaded)
	//	return 0;

	//long address = arm.r[15];
	u32 index = ((address & 0xFF000000) >> 24);

	if ( index > MEMORY_NUMSECTIONS )	{//Make sure the user doesn't try something dumb
		address = browseInstructionAddress;	//(This should always be a "good" address)
		index = (address & 0xFF000000) >> 24;
	} 
	
	browseInstructionMemSize = gbaMem.memorySize[index];
	browseInstructionAddress = address;

	u32 currentOffset = address & 0x00FFFFFF;	//Chop off the first byte

	//Perform different operations depending on if we're in Thumb or ARM mode.
	if (arm.thumbMode) {
		u16* dest16 = gbaMem.u16Mem[index];
		currentOffset = (currentOffset >> 1) - 2;	//Our instruction is 2 behind current pc
		currentOffset = currentOffset - (DEBUG_MAXLINES / 2);
		//Parse this instruction as well as several ahead and behind
		for ( int i = 0; i < DEBUG_MAXLINES; i++) {
			
				currentAddress = (index << 24) | (currentOffset << 1);
				strcpy(instructionArray[i], disassembleThumb(currentAddress));	
			/*
				//If this is the instruction currrently being executed, mark it
				if ( (arm.r[15] - 4) == currentAddress)
					strcpy(instructionArray[i], ">");
				else
					strcpy(instructionArray[i], " ");
				
				//If this is a breakpoint, mark it.
				if (ARM_containsBreakpoint(currentAddress))
					strcat (instructionArray[i], "*");
				else
					strcat (instructionArray[i], " ");

				strcat (instructionArray[i], hexToString (currentAddress));
				strcat (instructionArray[i], ":  ");
				
				//Get the value
				if ( (currentOffset << 1) < gbaMem.memorySize[index]) {	//if it's in bounds
					instruction = dest16[currentOffset];
					strcat (instructionArray[i], hexToString16 (instruction));
					strcat (instructionArray[i], " ");
					strcat (instructionArray[i], parseThumb( currentAddress , instruction ));
					//strcpy (instructionArray[i], "me");
				} else {
					strcat (instructionArray[i], "Nil");
				}
			*/
				currentOffset += 1;
		}
	} else {
		u32* dest = gbaMem.u32Mem[index];		//Calculate where we're coming from
											//(index of 8 indicates ROM)
		
		currentOffset = (currentOffset >> 2) - 2;		//Our instruction is (theoretically) 2
										//behind the current PC
		currentOffset = currentOffset - (DEBUG_MAXLINES / 2);
			
		int j = 0;

		//Parse this instruction as well as several ahead and behind
		for ( int i = 0; i < DEBUG_MAXLINES; i++) {
			
				u32  currentAddress = (currentOffset << 2) | (index << 24);
				strcpy(instructionArray[i], disassembleARM(currentAddress));		

		/*		//If this is the instruction currrently being executed, mark it
				if ( (arm.r[15] - 8) == currentAddress)
					strcpy(instructionArray[i], ">");
				else
					strcpy(instructionArray[i], " ");
				
				//If this is a breakpoint, mark it.
				if (ARM_containsBreakpoint(currentAddress))
					strcat (instructionArray[i], "*");
				else
					strcat (instructionArray[i], " ");

				
				strcat (instructionArray[i], hexToString (currentAddress));
				strcat (instructionArray[i], ":  ");
			
			if ( (currentOffset << 2) < gbaMem.memorySize[index]) {	//if it's in bounds
				//Get the value
				instruction = dest[currentOffset];
				strcat (instructionArray[i], parseInstruction( currentAddress , instruction ));
				//strcpy (instructionArray[i], "me");
			
			} else {
				strcat (instructionArray[i], "Nil");
			}*/
			currentOffset += 1;
		}
	}
	return (char**)instructionArray;
}

//////////////////////////////////////////////////////////////
//Converts a conditional breakpoint to a string.
//////////////////////////////////////////////////////////////
char* ARMDebugger::condBreakpointToString(ConditionalBreakpoint* breakpoint) {
	static char text[255];
	char value1[64];
	char value2[64];
	char bp[16];

	strcpy(value1, breakpointValueToString(&breakpoint->value1));
	strcpy(value2, breakpointValueToString(&breakpoint->value2));
	
	if (breakpoint->breakpoint == 0xFFFFFFFF) {
		strcpy (bp, "at ANY");
	} else {
		sprintf(bp, "at %08X", breakpoint->breakpoint);
	}
	
	switch(breakpoint->condition) {
		case BPCOND_EQ:	sprintf(text, "%s == %s %s", value1, value2, bp);	break;
		case BPCOND_NE:	sprintf(text, "%s != %s %s", value1, value2, bp);	break;
		case BPCOND_AND:	sprintf(text, "%s && %s %s", value1, value2, bp);	break;
		case BPCOND_BAND:	sprintf(text, "%s & %s %s", value1, value2, bp);	break;
		case BPCOND_OR:	sprintf(text, "%s || %s %s", value1, value2, bp);	break;
		case BPCOND_BOR:	sprintf(text, "%s | %s %s", value1, value2, bp);	break;
		case BPCOND_NOT:	sprintf(text, "! %s %s", value2, bp);	break;
		case BPCOND_BNOT:	sprintf(text, "%s == (~%s) %s", value1, value2, bp);	break;
		case BPCOND_ACC:	sprintf(text, "%s accessed %s", value1, bp);	break;
	}
	return text;
}

/////////////////////////////////////
//Converts a breakpoint value to a string.
/////////////////////////////////////
char* ARMDebugger::breakpointValueToString(BreakpointValue* value) {
	static char text[64];
	u32 lValue = value->lValue;
	switch (value->type) {
		case BPVALUE_U8LOCATION:
			sprintf(text, "*(u8*)(%08X)", lValue);
			break;
		case BPVALUE_U16LOCATION:
			sprintf(text, "*(u16*)(%08X)", lValue);
			break;
		case BPVALUE_U32LOCATION:
			sprintf(text, "*(u32*)(%08X)", lValue);
			break;
		case BPVALUE_HARDWAREREG:
			sprintf(text, "%03X %s", (value->lValue - 0x4000000),value->name);
			break;
		case BPVALUE_REG:
			if (lValue < 16) {
				sprintf(text, "r%X", lValue);
			} else if (lValue == 16) {
				sprintf(text, "CPSR");
			} else sprintf(text, "SPSR");
			break;
		case BPVALUE_CONSTANT:
			sprintf(text, "%08X", lValue);
			break;
	}
	return text;
}

/////////////////////////////////////////////////
//This function returns a string of formatted
//dissassmbler.
/////////////////////////////////////////////////
char* ARMDebugger::disassembleThumb(u32 address) {
	static char buffer[128];	//A buffer for our dissassembler.
	u32 offset = address & 0xFFFFFF;
	u32 index = address >> 24;
	u16 instruction;
	char* functionName;

	//If this is the instruction currrently being executed, mark it
	if ( (arm.r[15] - 4) == address)
		strcpy(buffer, ">");
	else
		strcpy(buffer, " ");
				
	//If this is a breakpoint, mark it.
	if (ARM_containsBreakpoint(address))
		strcat (buffer, "*");
	else
		strcat (buffer, " ");

	
	functionName = ELFFile_getFunctionName(&gba.elfFile, address);
	if (functionName != NULL) {
		strcat (buffer, functionName);
		strcat (buffer, "():\t");
	} else {
		strcat (buffer, hexToString (address));
		strcat (buffer, ":\t");
	}
				
	

	//Get the value
	if ( offset  < gbaMem.memorySize[index]) {	//if it's in bounds
		instruction = *(u16*)(&gbaMem.u8Mem[index][offset]);
		strcat (buffer, hexToString16 (instruction));
		strcat (buffer, " ");
		strcat (buffer, parseThumb( address , instruction ));
		//strcpy (buffer, "me");
	} else {
		strcat (buffer, "Nil");
	}
	return buffer;
}

//////////////////////////////////////////////
//This function returns a string of formatted disassembler
////////////////////////////////////////////////
char* ARMDebugger::disassembleARM(u32 address) {
	static char buffer[128];	//A buffer for our dissassembler.
	u32 offset = address & 0xFFFFFF;
	u32 index = address >> 24;
	u32 instruction;
	char* functionName;
	//If this is the instruction currrently being executed, mark it
	//If this is the instruction currrently being executed, mark it
	if ( (arm.r[15] - 8) == address)
		strcpy(buffer, ">");
	else
		strcpy(buffer, " ");
				
	//If this is a breakpoint, mark it.
	if (ARM_containsBreakpoint(address))
		strcat (buffer, "*");
	else
		strcat (buffer, " ");

				
	functionName = ELFFile_getFunctionName(&gba.elfFile, address);
	if (functionName != NULL) {
		strcat (buffer, functionName);
		strcat (buffer, "():\t");
	} else {
		strcat (buffer, hexToString (address));
		strcat (buffer, ":\t");
	}
			
	if ( offset < gbaMem.memorySize[index]) {	//if it's in bounds
		//Get the value
		instruction = *(u32*)(&gbaMem.u8Mem[index][offset]);
		strcat (buffer, parseInstruction( address , instruction ));
		//strcpy (buffer, "me");
			
	} else {
		strcat (buffer, "Nil");
	}
	return buffer;
}

//////////////////////////////////////////////////////////////////////
//outputDisassmbler
//This function outputs disassmbler from the given memory into a file,
//labelling the start and end with the given start and end address.
//Note that this function takes a pointer to the source memory,
//not an ARM address.
//////////////////////////////////////////////////////////////////////
void ARMDebugger::outputDisassembler(char* filename, u8* source, u32 startAddress, 
						u32 endAddress, u32 thumbMode) {

	FILE* file;
	s32 size;
	u16* source16 = (u16*)source;
	u32* source32 = (u32*) source;
	char*text;
	u32 instruction;

	//This opens the file, exiting if it doesn't exist.
	if (!(file = fopen(filename, "wt"))) 
		return;

	//Go to beginning of file
	fseek (file, 0, SEEK_SET);

	if (thumbMode) {
		size = endAddress - startAddress + 2;
		size = size & 0xFFFFFFFC;	//Wipe off the lowest two bits to align to words
		while (size > 0) {

			instruction = *source16;
			text = hexToString (startAddress);
			fwrite (text, 1, strlen(text), file);
			fwrite (":  ", 1, 3, file);
			text = hexToString16 (instruction);
			fwrite (text, 1, strlen(text), file);
			fwrite (" ", 1, 1, file);
			text = parseThumb( startAddress , instruction );
			fwrite (text, 1, strlen(text), file);
			fwrite ("\n", 1, 1, file);

			source16++;
			size-=2;
			startAddress+=2;
		}
	} else {
		size = endAddress - startAddress + 4;
		size = size & 0xFFFFFFFC;	//Wipe off the lowest two bits to align to words
		while (size > 0) {
			instruction = *source32;
			text = hexToString (startAddress);
			fwrite (text, 1, strlen(text), file);
			fwrite (":  ", 1, 3, file);
			text = parseInstruction( startAddress , instruction );
			fwrite (text, 1, strlen(text), file);
			fwrite ("\n", 1, 1, file);
			
			source32++;
			size-=4;
			startAddress+=4;
		}
	}
	
	fclose(file);		
}

//////////////////////////////////////////////////////////////
//parseMemory()
//This parses the last location stored
///////////////////////////////////////////////////////////
char** ARMDebugger::parseMemory() {
	if (useAutoLocation)			//Check and see whether we're auto parsing.
		//parseMemory(debugArm->location);
          parseMemory(arm.location);
	else
		parseMemory(browseMemAddress);
	return (char**)memArray;
}

////////////////////////////////////////////////////////////////////
//parseMemory(u32 location)
//This takes a location in memory and converts it to string format
///////////////////////////////////////////////////////////////////
char** ARMDebugger::parseMemory(u32 address) {
	s32 memSize, offset;
	u32 value, index;
	char temp[255];
	u8* memory;

	//if (!gba.loaded)
	//	return 0;

	index = address >> 24;	//Wipe off all but the upper byte
		
	//if (address == 0xFFFFFFFF)	//This was going to be a special "AUTO" address
	//	address = 0x08000000;	

	//browseMemAddress keeps track of the current memory address being browsed
	if ( (index < 0) || (index > MEMORY_NUMSECTIONS) )	{//Make sure the user doesn't try something dumb
		address = browseMemAddress;	//(This should always be a "good" address)
		index = address >> 24;
	} 

	memSize = gbaMem.memorySize[index];
	memory = gbaMem.u8Mem[index];
	
	offset = address & 0x00FFFFFF;	//Get the offset

	//More preventative measures.
	if (offset > memSize - 4) {
		address = browseMemAddress;
		offset = browseMemAddress & 0x00FFFFFF;
		memSize = browseMemSize;
		memory = gbaMem.u8Mem[address >> 24];
	}

	//If we passed all the trials, save it for good
	browseMemAddress = address;
	browseMemSize = memSize;

	strcpy (locationVal, hexToString(browseMemAddress));
	
	//Get all 32 bits (yeah I know this is roundabout, bite me.)
	if (offset < memSize) {
		value = memory[offset+3];
		value = memory[offset+2] | value << 8;
		value = memory[offset+1] | value << 8;
		value = memory[offset] | value << 8;
	} else {
		value = 0;
	}
	
	//Convert to string
	strcpy (memVal, hexToString(value));

	//That's just the first part.
	//Now we want to display the memory in a nice line by line format with each
	//line offset by 16.  Thus we have 
	offset = offset & 0xFFFFFFF0;	//Wipe off the lower 8 bits


		
	//Display 16 lines of 16 bytes each
	for (u32 i = 0; i < DEBUG_MAXMEMLINES; i++) {
		strcpy (memArray[i], "");	//Initialize the string
		strcat (memArray[i], hexToString(offset + (index << 24) ));
		strcat (memArray[i], ": ");

		for (u32 j = offset; j < offset + 16; j++) {
			
			if (j >= memSize) {		//Let the user know they're at the end
				strcat (memArray[i], "NN");
			} else {
				//Put it into byte format (we must do this in bytes
				//because we want to show the byte reversal).
				sprintf (temp, "%02x", memory[j]);
				strcat (memArray[i], temp);
			}

			if ( j%2  )	//Put spacers ever 2 bytes
				strcat (memArray[i], " ");
		}
		offset+=16;


	}
		
	

	return (char**)memArray;;

	
}

///////////////////////////////////////////////////////////////////////
//parseRegisters
//This gets the registers, converts them to string format, print them.
///////////////////////////////////////////////////////////////////
char** ARMDebugger::parseRegisters () {
	
	for (int i = 0; i < 16; i++) {
		strcpy (regArray[i], hexToString ( arm.r[i] ) );
	}
	//The last thing in our array is the CPSR
	//strcpy (regArray[16], hexToString (debugArm->CPSR) );
     strcpy (regArray[16], hexToString (arm.CPSR) );
	return (char**)regArray;
}


//This just runs the CPU
/*
void ARMDebugger::run () {
	LARGE_INTEGER largeInt;
	
	if (!gba.loaded)
		return;

	while ( 1 ) {
		debugArm->runNext();
		//WE have to tell the graphics to update.
		//This bogus timer value should make them update as fast as they can.
		QueryPerformanceCounter(&largeInt);
		//I hate having to use the actual timer, but otherwise it just doesn't
		//seem to work.:(
		debugArm->Graphics_updateScreen(0,largeInt.LowPart);//	
		//bogusTime = bogusTime + debugArm->lineRefreshTime * 2;
	}
}*/


//////////////////////////////////////////////////////////
//parseDataOperation
//This parses a data operation.  Doesn't do anything else.
//////////////////////////////////////////////////////////
char* ARMDebugger::parseDataOperation(u32 currentAddress, u32 instruction, char* condition) {
	static char returnChar[256];

	char rm[10], rd[10], rn[10], rc[10], op2[256];
	u32 opcode, rdI, rnI, rmI, rcI, op2I;
    int shiftType, shift;
	u32 S;
	//fetch(IQ[0]);	//Pre-fetch an instruction into IQ[0]
	//*r[15] += instructionLength;	//Increase the program counter

	
	//Get the opcode and the operands
	rnI = Support_getBits (instruction, 19, 16);
	sprintf  (rn, "r%d", rnI);
	rdI = Support_getBits (instruction, 15, 12);
	sprintf(rd, "r%d", rdI);
	rmI = Support_getBits (instruction, 3, 0);
	sprintf (rm, "r%d", rmI);
	rcI = Support_getBits (instruction, 11, 8);
	sprintf (rc, "r%d", rcI);

	//Find if we're supposed to carry
	S = SUPPORT_GETBIT20(instruction);

	opcode = Support_getBits (instruction, 24, 21);

	strcpy (returnChar, "");
	//If the s bit isn't set and it's a TEQ or CMN,
	if (!S) {
		//////////////////////////////////
		//MSR
		///////////////////////////////////
		if ((opcode == OP_TEQ) || (opcode == OP_CMN)) {
			sprintf(returnChar, "MSR");	
			strcat (returnChar, " ");
			if (SUPPORT_GETBIT22(instruction)) {	//If it's an SPSR transfer
				strcat (returnChar, "SPSR");
			} else {
				strcat (returnChar, "CPSR");
			}
			//Find otu what part of the CPSR gets updated.
			switch ( (instruction >> 16) & 0xF) {
				case 0x8:	strcat(returnChar, "_flg");
					break;
				case 0x9:	strcat(returnChar, "_all");
					break;
				case 0x1:	strcat(returnChar, "_ctl");
					break;
			}

			
			//Find if the operand is immediate or a register.
			if ( SUPPORT_GETBIT25 (instruction) ) {	//If it's immediate
				op2I = Support_getBits (instruction, 7, 0);
				//Get the shift
				shift = Support_getBits (instruction, 11, 8) << 1;
				//Calculate the shift here
				op2I = (op2I << (32 - shift)) | (op2I >> shift);
				sprintf (op2, "0x%X", op2I);
			} else {	//if it's a register...
				sprintf(op2, "r%d", (instruction & 0xF));
			}
			strcat(returnChar, ", ");
			strcat(returnChar, op2);
			return returnChar;

		////////////////////////////////////////
		//MRS
		////////////////////////////////////////
		} else if ((opcode == OP_TST) || (opcode == OP_CMP)) {
			sprintf (returnChar, "MRS");
			strcat (returnChar, " ");
			sprintf(op2, "r%d", ((instruction >> 12) & 0xF));
			strcat (returnChar, op2);
			if (SUPPORT_GETBIT22(instruction)) {	//If it's an SPSR transfer
				strcat (returnChar, ", SPSR");
			} else {
				strcat (returnChar, ", CPSR");
			}

			return returnChar;
		} else
			sprintf (returnChar, aluops[opcode]);	//Get the opcode
	} else {
		sprintf (returnChar, aluops[opcode]);	//Get the opcode
		strcat (returnChar, "S");
	}
	
	strcat (returnChar, condition);			//concat the condition

	//Calculate op2
	//S = Support::getBit (instruction, 20);	//Find if we're supposed to carry

	//Find if the operand is immediate or a register.
	if ( SUPPORT_GETBIT25 (instruction) ) {	//If it's immediate
		strcat (returnChar, " ");

		op2I = Support_getBits (instruction, 7, 0);
		
		//Get the shift
		shift = Support_getBits (instruction, 11, 8) << 1;

		//Calculate the shift here
		op2I = (op2I << (32 - shift)) | (op2I >> shift);

		sprintf (op2, "0x%X", op2I);

		strcat(returnChar, parseALUOpcodes(opcode, rd, rn, op2));//rd);
		//strcat(returnChar, ", ");
		//strcat(returnChar, rn);
		//strcat (returnChar, ", ");
		//strcat (returnChar, op2);
		return returnChar;


	} else {	//If the operand is a register
		shiftType = Support_getBits (instruction, 6, 5);
		
		strcat(returnChar, " ");
		strcat(returnChar, parseALUOpcodes(opcode, rd, rn, rm));

		//Find out if it's an immediate shift or a reg shift
		if (SUPPORT_GETBIT4(instruction)) {	//It's a reg shift
			
			//Concat the shift type
			strcat (returnChar, ", ");
			strcat (returnChar, shifts[shiftType]);
			strcat (returnChar, " ");
			strcat (returnChar, rc);
		
		} else {					//It's an immediate shift
			shift = Support_getBits (instruction, 11, 7);
			
			if (shift) {	//If there was a shift specified . . .
				sprintf (op2, "#%d", shift);
				
				strcat (returnChar, ", ");
				strcat (returnChar, shifts[shiftType]);
				strcat (returnChar, " ");
				strcat (returnChar, op2);
			}
			
		}
		return returnChar;
	}
	return returnChar;
}

/////////////////////////////////////////////////
//paseALUOpcodes()
//Prints appropriate information for opcode to string
////////////////////////////////////////////////
char* ARMDebugger::parseALUOpcodes(u32 opcode, char* rd, char* rn, char* op2) {
	static char returnChar[255];

	switch (opcode) {
		case OP_AND:	//Boolean And
						sprintf (returnChar, "%s, %s, %s", rd, rn, op2);
						break;

		case OP_EOR:	sprintf (returnChar, "%s, %s, %s", rd, rn, op2);
						break;

		case OP_SUB:	sprintf (returnChar, "%s, %s, %s", rd, rn, op2);
						break;

		case OP_RSB:	sprintf (returnChar, "%s, %s, %s", rd, rn, op2);
						break;

		case OP_ADD:	sprintf (returnChar, "%s, %s, %s", rd, rn, op2);
						break;

		//For these we want to get the carry bit of the arm.CPSR
		case OP_ADC:	sprintf (returnChar, "%s, %s, %s", rd, rn, op2);
						break;
		
		case OP_SBC:	sprintf (returnChar, "%s, %s, %s", rd, rn, op2);
						break;

		case OP_RSC:	sprintf (returnChar, "%s, %s, %s", rd, rn, op2);
						break;
		//ToDO:  Only execute these if S is set, otherwise execute MSR.
		//For these we do not want to modify the result register
		case OP_TST:	sprintf (returnChar, "%s, %s", rn, op2);
						break;

		case OP_TEQ:	sprintf (returnChar, "%s, %s", rn, op2);
						break;

		case OP_CMP:	sprintf (returnChar, "%s, %s", rn, op2);
						break;

		case OP_CMN:	sprintf (returnChar, "%s, %s", rn, op2);
						break;
			
		case OP_ORR:	sprintf (returnChar, "%s, %s, %s", rd, rn, op2);	//could be wrong...
						break;

		case OP_MOV:	//sprintf (returnChar, "%s, %s, %s", rd, rn, op2);
						sprintf (returnChar, "%s, %s", rd, op2);
						break;

		case OP_BIC:	sprintf (returnChar, "%s, %s, %s", rd, rn, op2);
						break;

		case OP_MVN:	sprintf (returnChar, "%s, %s", rd, op2);
						break;

	}
	return returnChar;
}

//////////////////////////////////////////////////////////////////////
//Parse a load/store operation
////////////////////////////////////////////////////////////////////
char* ARMDebugger::parseLoadStore(u32 currentAddress, u32 instruction, char* condition) {
	static char returnChar[255];
	char offset[255];
	char shiftAmt[255];
	char temp[255];
	u32 rdI, rnI, rmI, offsetI;
	char rd[5], rn[5], rm[5];
	int shiftType, shift;

	//fetch(IQ[0]);	//Pre-fetch an instruction into IQ[0]
	//*r[15] += instructionLength;	//Increase the program counter

	
	//Get the opcode and the operands
	rnI = Support_getBits (instruction, 19, 16);
	sprintf  (rn, "r%d", rnI);
	rdI = Support_getBits (instruction, 15, 12);
	sprintf(rd, "r%d", rdI);
	rmI = Support_getBits (instruction, 3, 0);
	sprintf (rm, "r%d", rmI);
	
	if (SUPPORT_GETBIT20(instruction)) //If this is a load
		sprintf (returnChar, "LDR");
	else
		sprintf (returnChar, "STR");

	if (SUPPORT_GETBIT22 (instruction) ) 		//If the B bit is set
		strcat (returnChar, "B");
	else
		strcat (returnChar, "W");
	
	
	//Stick the condition in there
	strcat (returnChar, condition);
	//strcat (returnChar, " ");
	
	//Initialize offset to ""
	sprintf (offset, "");

	//Find if the operand is immediate or a register.
	if ( !SUPPORT_GETBIT25 (instruction) ) {	//If it's immediate
		
		//Get the operand
		offsetI = Support_getBits (instruction, 11, 0);
		
		sprintf (offset, "0x%X", offsetI);
		if (rnI == 15) {	//We can only do this fo r15 because we can't tell
							//the contents of the other registers when debugging		
			offsetI += currentAddress + 8;
			sprintf (temp, " (0x%08X)", offsetI);
			strcat (offset, temp);
		}
		//Print the registers
		strcat (returnChar, " ");
		strcat (returnChar, rd);
		
		
	} else {	//If the operand is a register
		//Print the registers
		strcat (returnChar, " ");
		strcat (returnChar, rd);
	
		shiftType = Support_getBits (instruction, 6, 5);
		
		strcat (offset, rm);
		strcat (offset, " ");
		strcat (offset, shifts[shiftType]);

		shift = Support_getBits (instruction, 11, 7);

		sprintf (shiftAmt, "#%d", shift);
		strcat (offset, " ");
		strcat (offset, shiftAmt);
		
	}
		
	strcat (returnChar, ", [");
	strcat (returnChar, rn);

	if ( SUPPORT_GETBIT23(instruction) )		//If the U bit is set
		strcat (returnChar, " +");
	else
		strcat (returnChar, " -");



	//Add the offset
	strcat(returnChar, offset);
	strcat (returnChar, "]");


	//Pre or post indexing
	if (SUPPORT_GETBIT24(instruction) ) {		//If P is set (pre-index
		strcat (returnChar, " (PRE)");
	} else {	//If P is not set, post-index 
		strcat (returnChar, " (POST)");
	}

	if (SUPPORT_GETBIT21(instruction) )			//If W is set
		strcat (returnChar, "(W)");

	return returnChar;
}

//////////////////////////////////////////////////////////////////////
//Parse a load/store multiple operation
////////////////////////////////////////////////////////////////////
char* ARMDebugger::parseLoadStoreMultiple(u32 currentAddress, u32 instruction, char* condition) {
	static char returnChar[255];
	u32 rnI, regList;
	char rn[5], reg[5];
	
	
	//Get the opcode and the operands
	rnI = Support_getBits (instruction, 19, 16);
	regList = instruction & 0x0000FFFF;
	sprintf  (rn, "r%d", rnI);
		
	if (SUPPORT_GETBIT20(instruction)) //If this is a load
		sprintf (returnChar, "LDM");
	else
		sprintf (returnChar, "STM");

	//Stick the condition in there
	strcat (returnChar, condition);
	strcat (returnChar, " ");
	strcat (returnChar, rn);
	strcat (returnChar, ", {");
	

	for (u32 i = 0; i < 16; i++) {
		if ( (regList >> i) & 0x1) {	//If bit "i" is set
			sprintf  (reg, "r%d,", i);
			strcat (returnChar, reg);
		}
	}
	if ( SUPPORT_GETBIT23 (instruction) )		//If the U bit is set
		strcat (returnChar, "} +4");
	else
		strcat (returnChar, "} -4");



	//Pre or post indexing
	if (SUPPORT_GETBIT24(instruction) ) {		//If P is set (pre-index
		strcat (returnChar, " (PRE)");
	} else {	//If P is not set, post-index 
		strcat (returnChar, " (POST)");
	}

	if (SUPPORT_GETBIT21 (instruction) )			//If W is set
		strcat (returnChar, "(W)");

	return returnChar;
}

//////////////////////////////////////////
//Parse load/store of a halfword
char* ARMDebugger::parseLoadStoreHalfword(u32 currentAddress, u32 instruction, char* condition) {
	static char returnChar[255];
	char offset[255];
	u32 rdI, rnI, rmI, offsetI;
	char rd[10], rn[10], rm[10], temp[255];

	//fetch(IQ[0]);	//Pre-fetch an instruction into IQ[0]
	//*r[15] += instructionLength;	//Increase the program counter

	
	//Get the opcode and the operands
	rnI = Support_getBits (instruction, 19, 16);
	sprintf  (rn, "r%d", rnI);
	rdI = Support_getBits (instruction, 15, 12);
	sprintf(rd, "r%d", rdI);
	rmI = Support_getBits (instruction, 3, 0);
	sprintf (rm, "r%d", rmI);
	
	if (SUPPORT_GETBIT20(instruction)) //If this is a load
		sprintf (returnChar, "LDR");
	else
		sprintf (returnChar, "STR");

	switch (Support_getBits (instruction, 6, 5)) {
		case 1:	strcat (returnChar, "H");	//It's a halfword
			break;
		case 2: strcat (returnChar, "SB");	//signed byte
			break;
		case 3: strcat (returnChar, "SH");	//signed halfword
			break;
		default:
				strcat (returnChar, "?");	//Problem
			break;

	}

	
	//Stick the condition in there
	strcat (returnChar, condition);
	//strcat (returnChar, " ");
	
	//Initialize offset to ""
	sprintf (offset, "");

	//Find if the operand is immediate or a register.
	if ( SUPPORT_GETBIT22(instruction) ) {	//If it's immediate
		
		//Get the operand
		offsetI = Support_getBits (instruction, 11, 8);
		offsetI = offsetI << 4;
		offsetI = offsetI | Support_getBits (instruction, 3, 0);
		
		sprintf (offset, "0x%X", offsetI);
		if (rnI == 15) {	//We can only do this fo r15 because we can't tell
							//the contents of the other registers when debugging		
			offsetI += currentAddress + 8;
			sprintf (temp, " (0x%08X)", offsetI);
			strcat (offset, temp);
		}
	
		
	} else {	//If the operand is a register
		strcat (offset, rm);
	}
	
	//Print the registers
	strcat (returnChar, " ");
	strcat (returnChar, rd);
	strcat (returnChar, ", [");
	strcat (returnChar, rn);

	if ( SUPPORT_GETBIT23(instruction) )		//If the U bit is set
		strcat (returnChar, " +");
	else
		strcat (returnChar, " -");

	//Add the offset
	strcat(returnChar, offset);
	strcat(returnChar, "]");

	//Pre or post indexing
	if (SUPPORT_GETBIT24(instruction) ) {		//If P is set (pre-index
		strcat (returnChar, " (PRE)");
	} else {	//If P is not set, post-index 
		strcat (returnChar, " (POST)");
	}

	if (SUPPORT_GETBIT21(instruction) )			//If W is set
		strcat (returnChar, "(W)");

	return returnChar;
}



///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
char* ARMDebugger::parseSwap (u32 currentAddress, u32 instruction, char* condition) {
	static char returnChar[255];
	u32 rdI, rnI, rmI;
	char rd[10], rn[10], rm[10];

	//fetch(IQ[0]);	//Pre-fetch an instruction into IQ[0]
	//*r[15] += instructionLength;	//Increase the program counter

	
	//Get the opcode and the operands
	rnI = Support_getBits (instruction, 19, 16);
	sprintf  (rn, "r%d", rnI);
	rdI = Support_getBits (instruction, 15, 12);
	sprintf(rd, "r%d", rdI);
	rmI = Support_getBits (instruction, 3, 0);
	sprintf (rm, "r%d", rmI);
	
	sprintf (returnChar, "SWP");

	if (SUPPORT_GETBIT22(instruction)) //If it's a byte
		strcat (returnChar, "B");
	
	strcat (returnChar, condition);
	
	//Print the registers
	strcat (returnChar, " ");
	strcat (returnChar, rd);
	strcat (returnChar, ", [");
	strcat (returnChar, rn);
	strcat (returnChar, "]");

	
	return returnChar;

}

///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
char* ARMDebugger::parseMul (u32 currentAddress, u32 instruction, char* condition) {
	static char returnChar[255];
	u32 rdI, rnI, rmI, rsI;
	char rd[10], rn[10], rm[10], rs[10];
	int MLA;

	//fetch(IQ[0]);	//Pre-fetch an instruction into IQ[0]
	//*r[15] += instructionLength;	//Increase the program counter
	//NOTE That rn and rd are backwards for a mul...
	
	//Get the opcode and the operands
	rdI = Support_getBits (instruction, 19, 16);
	sprintf  (rd, "r%d", rdI);
	rnI = Support_getBits (instruction, 15, 12);
	sprintf(rn, "r%d", rnI);
	rsI = Support_getBits (instruction, 11, 8);
	sprintf(rs, "r%d", rsI);
	rmI = Support_getBits (instruction, 3, 0);
	sprintf (rm, "r%d", rmI);
	
	MLA = SUPPORT_GETBIT21(instruction);

	//Is it a MUL or an MLA?
	if (MLA)		//If it's a MLA
		sprintf (returnChar, "MLA");
	else
		sprintf (returnChar, "MUL");
	
	strcat(returnChar, condition);

	//Print the registers
	strcat (returnChar, " ");
	strcat (returnChar, rd);	//
	strcat (returnChar, ", ");
	strcat (returnChar, rm);
	strcat (returnChar, ", ");
	strcat (returnChar, rs);

	if (MLA) {
		strcat (returnChar, ", +");
		strcat (returnChar, rn);
	}

	
	return returnChar;

}

//////////////////////////////////////////////////////////
//parseThumb()
//A catch all function for parsing thumb instructions.
//////////////////////////////////////////////////////////////
char* ARMDebugger::parseThumb (u32 currentAddress, u32 instruction) {
	static char returnChar[256];
	char temp64[64];
	char temp16[16];
	u32 rdI, rnI, rsI, roI, rbI, condI;
	s32 offsetI;
	u32 tempAddress, tempData, index;
	s32 offset;
	char* functionName = NULL;
	char* symbolName = NULL;
	tempData = 0xFFFFFFFF;
	StabFunction* stabFunction;
	StabData* stabData;
	
	instruction = instruction & 0xFFFF;	//Chop off any top bits.
	
	
	
	u32 type = instruction >> 13;	//Get the top 3 bits

	if (type == 0x0 ) {	//Move shifted/add/subtract
		rdI = SUPPORT_GETRDTHUMB(instruction);
		rsI = SUPPORT_GETRSTHUMB(instruction);
		rnI = (instruction >> 6) & 0x7;
		offsetI = (instruction >> 6) & 0x1F;
		//Switch on the opcode
		switch ( (instruction >> 11) & 0x3) {
			//Move shifted
			case 0x0:		sprintf(returnChar, "LSL r%d, r%d, 0x%X", rdI, rsI, offsetI);	break;
			case 0x1:		sprintf(returnChar, "LSR r%d, r%d, 0x%X", rdI, rsI, offsetI); break;
			case 0x2:		sprintf(returnChar, "ASR r%d, r%d, 0x%X", rdI, rsI, offsetI); break;
			//Add/subtract
			case 0x3:		switch ( (instruction >> 9) & 0x3) {
								case 0x0:	sprintf(returnChar, "ADD r%d, r%d, r%d", rdI, rsI, rnI); break;
								case 0x1:	sprintf(returnChar, "SUB r%d, r%d, r%d", rdI, rsI, rnI); break;
								case 0x2:	sprintf(returnChar, "ADDI r%d, r%d, 0x%X", rdI, rsI, rnI); break;
								case 0x3:	sprintf(returnChar, "SUBI r%d, r%d, 0x%X", rdI, rsI, rnI); break;
							}//switch
			
		}
	} else if (type == 0x1) {	//Move/compare/add/subtract immediate
		rdI = (instruction >> 8) & 0x7;
		offsetI = instruction & 0xFF;
		switch ( (instruction >> 11) & 0x3) {
			case 0x0:	sprintf(returnChar, "MOVI r%d, 0x%X", rdI, offsetI); break;
			case 0x1:	sprintf(returnChar, "CMPI r%d, 0x%X", rdI, offsetI); break;
			case 0x2:	sprintf(returnChar, "ADDI r%d, 0x%X", rdI, offsetI); break;
			case 0x3:	sprintf(returnChar, "SUBI r%d, 0x%X", rdI, offsetI); break;
		}//switch
	} else {
		type = instruction >> 12;	//Get the top 4 bits
		rdI = SUPPORT_GETRDTHUMB(instruction);
		rsI = SUPPORT_GETRSTHUMB(instruction);
		switch (type) {
			//ALU operations / Hi register operations/branch exchange / PC relative load
			case 0x4:	
				if ( ((instruction >> 10) & 0x3) == 0) {	//ALU operation
					
					switch ( (instruction >> 6) & 0xF) {
						case 0x0:	sprintf(returnChar, "AND r%d, r%d", rdI, rsI); break;
						case 0x1:	sprintf(returnChar, "EOR r%d, r%d", rdI, rsI); break;
						case 0x2:	sprintf(returnChar, "LSL r%d, r%d", rdI, rsI); break;
						case 0x3:	sprintf(returnChar, "LSR r%d, r%d", rdI, rsI); break;
						case 0x4:	sprintf(returnChar, "ASR r%d, r%d", rdI, rsI); break;
						case 0x5:	sprintf(returnChar, "ADC r%d, r%d", rdI, rsI); break;
						case 0x6:	sprintf(returnChar, "SBC r%d, r%d", rdI, rsI); break;
						case 0x7:	sprintf(returnChar, "ROR r%d, r%d", rdI, rsI); break;
						case 0x8:	sprintf(returnChar, "TST r%d, r%d", rdI, rsI); break;
						case 0x9:	sprintf(returnChar, "NEG r%d, r%d", rdI, rsI); break;
						case 0xA:	sprintf(returnChar, "CMP r%d, r%d", rdI, rsI); break;
						case 0xB:	sprintf(returnChar, "CMN r%d, r%d", rdI, rsI); break;
						case 0xC:	sprintf(returnChar, "ORR r%d, r%d", rdI, rsI); break;
						case 0xD:	sprintf(returnChar, "MUL r%d, r%d", rdI, rsI); break;
						case 0xE:	sprintf(returnChar, "BIC r%d, r%d", rdI, rsI); break;
						case 0xF:	sprintf(returnChar, "MVN r%d, r%d", rdI, rsI); break;
					}//switch
				} else if ( ((instruction >> 10) & 0x3) == 0x1) {	//Hi register operations
					if (SUPPORT_GETBIT7(instruction))
						rdI = rdI + 0x8;
					if (SUPPORT_GETBIT6(instruction))
						rsI = rsI + 0x8;

					switch ( (instruction >>8 ) & 0x3) {
						case 0x0:	sprintf(returnChar, "ADDHI r%d, r%d", rdI, rsI); break;
						case 0x1:	sprintf(returnChar, "CMPHI r%d, r%d", rdI, rsI); break;
						case 0x2:	sprintf(returnChar, "MOVHI r%d, r%d", rdI, rsI); break;
						case 0x3:	sprintf(returnChar, "BXHI r%d", rsI);  break;
					}//switch
				} else {	
					//PC - relative load
					rdI = (instruction >> 8) & 0x7;
					offsetI = (instruction & 0xFF) << 2;
					//offsetI = offsetI >> 22;
					
					tempAddress = (currentAddress & 0xFFFFFFFC) + offsetI + 4;
					index = tempAddress >> 24;
					offset = tempAddress & 0x00FFFFFF;
					//See if we have a symbol for this in the symboltable
					if (gbaMem.memorySize[index] > offset) {	//First check this location in memory
						tempData = *(u32*)(&gbaMem.u8Mem[index][offset]);
					}
					symbolName = ELFFile_getSymbolName(&gba.elfFile, tempData);
					if (symbolName != NULL) 
						sprintf(returnChar, "LDRPC, r%d, 0x%08X (=%s) ", rdI, tempAddress, symbolName);
					else
						sprintf(returnChar, "LDRPC, r%d, 0x%08X (=0x%08X)", rdI, tempAddress, tempData);
					break;
				}
				break;
			//load store
			case 0x5:	
					rdI = SUPPORT_GETRDTHUMB(instruction);
					rbI = SUPPORT_GETRBTHUMB(instruction);
					roI = (instruction >> 6) & 0x7;//SUPPORT_GETRDTHUMB(instruction);
					switch ( (instruction >> 9) & 0x7) {
						case 0x0:	sprintf(returnChar, "STR r%d, [r%d, r%d]", rdI, rbI, roI);		break;
						case 0x1:	sprintf(returnChar, "STRH r%d, [r%d, r%d]", rdI, rbI, roI);	break;
						case 0x2:	sprintf(returnChar, "STRB r%d, [r%d, r%d]", rdI, rbI, roI);	break;
						case 0x3:	sprintf(returnChar, "LDRSB r%d, [r%d, r%d]", rdI, rbI, roI);	break;
						case 0x4:	sprintf(returnChar, "LDR r%d, [r%d, r%d]", rdI, rbI, roI);		break;
						case 0x5:	sprintf(returnChar, "LDRH r%d, [r%d, r%d]", rdI, rbI, roI);	break;
						case 0x6:	sprintf(returnChar, "LDRB r%d, [r%d, r%d]", rdI, rbI, roI);	break;
						case 0x7:	sprintf(returnChar, "LDSH r%d, [r%d, r%d]", rdI, rbI, roI);	break;							
					}
					break;
			//////////////////////////////
			//load/store immediate word
			////////////////////////////
			case 0x6:	
						rdI = SUPPORT_GETRDTHUMB(instruction);
						rbI = SUPPORT_GETRBTHUMB(instruction);
						offsetI = ((instruction >> 6) & 0x1F) << 2;
						
						//If rd is 7, chances are good that it's the stack pointer offset
						//for a function.
						if (rbI == 0x7) {
							stabFunction = Stabs_findFunctionByAddress(&gba.stabs, currentAddress);
							if (stabFunction != NULL) {	//If this is happening in a function...
								//See if it's a local variable.
								stabData = (StabData*)Stabs_findByValue((StabListEntry*)&stabFunction->localVarList, offsetI);
								if (stabData != NULL)
									symbolName = stabData->name;
								else {	//Otherwise maybe it's a parameter.
									stabData = (StabData*)Stabs_findByValue((StabListEntry*)&stabFunction->paramList, offsetI);
									if (stabData != NULL)
										symbolName = stabData->name;
								}
							}
						} 
						if (symbolName != NULL) {
							if (SUPPORT_GETBIT11(instruction))	//load
								sprintf(returnChar, "LDRI r%d, [r%d, 0x%X] (%s)", rdI, rbI, offsetI, symbolName);
							else								//store
								sprintf(returnChar, "STRI r%d, [r%d, 0x%X] (%s)", rdI, rbI, offsetI, symbolName);
						} else {
							if (SUPPORT_GETBIT11(instruction))	//load
								sprintf(returnChar, "LDRI r%d, [r%d, 0x%X]", rdI, rbI, offsetI);
							else								//store
								sprintf(returnChar, "STRI r%d, [r%d, 0x%X]", rdI, rbI, offsetI);
						}
						break;
			//load/store imediate byte
			case 0x7:	rdI = SUPPORT_GETRDTHUMB(instruction);
						rbI = SUPPORT_GETRBTHUMB(instruction);
						offsetI = ((instruction >> 6) & 0x1F);
						if (SUPPORT_GETBIT11(instruction))	//load
							sprintf(returnChar, "LDRB r%d, [r%d, 0x%X]", rdI, rbI, offsetI);
						else
							sprintf(returnChar, "STRB r%d, [r%d, 0x%X]", rdI, rbI, offsetI);
						break;
			//load store halfword
			case 0x8:	rdI = SUPPORT_GETRDTHUMB(instruction);
						rbI = SUPPORT_GETRBTHUMB(instruction);
						offsetI = ((instruction >> 6) & 0x1F) << 1;
						if (SUPPORT_GETBIT11(instruction))	//load
							sprintf(returnChar, "LDRH r%d, [r%d, 0x%X]", rdI, rbI, offsetI);
						else
							sprintf(returnChar, "STRH r%d, [r%d, 0x%X]", rdI, rbI, offsetI);
						break;
			//SP relative load/store
			case 0x9:	rdI = (instruction >> 8) & 0x7;
						offsetI = (instruction & 0xFF) << 2;
						if (SUPPORT_GETBIT11(instruction))	//load
							sprintf(returnChar, "LDRSP r%d, [SP, 0x%X]", rdI, offsetI);
						else
							sprintf(returnChar, "STRSP r%d, [SP, 0x%X]", rdI, offsetI);
						break;
			//load address
			case 0xA:	rdI = (instruction >> 8) & 0x7;
						offsetI = (instruction & 0xFF) << 2;
						if (SUPPORT_GETBIT11(instruction))	//sp
							sprintf(returnChar, "ADRLSP r%d, SP, 0x%X", rdI, offsetI);
						else								//pc
							sprintf(returnChar, "ADRLPC r%d, PC, 0x%X", rdI, offsetI);
						break;
			///////////////////////////////////////////
			//Add offset to stack pointer/push/pop
			///////////////////////////////////////////
			case 0xB:	{
					offsetI = (instruction & 0x7F) << 2;
					//Put together a string for the register list.
					u32 regList = instruction & 0xFF;
					strcpy (temp64, "");
					for (int i = 0; i < 8; i++) {
						if ((regList >> i) & 0x1) {
							sprintf(temp16, "r%d", i);
							strcat(temp64, temp16);
							strcat(temp64, " ");
						}
					}
					

					switch ((instruction >> 7) & 0x1F) {
						case 0x0:	sprintf(returnChar, "ADD SP, 0x%X", offsetI); 	break;
						case 0x1:	sprintf(returnChar, "ADD SP, -0x%X", offsetI);	break;
						case 0x8:
						case 0x9:
						case 0xA:
						case 0xB:	if (SUPPORT_GETBIT8(instruction))
										strcat(temp64, "r13");
									sprintf(returnChar, "PUSH (%s)", temp64);	break;
						case 0x18:	
						case 0x19: 
						case 0x1A:
						case 0x1B:  if (SUPPORT_GETBIT8(instruction))
										strcat(temp64, "r15");
									sprintf(returnChar, "POP (%s)", temp64);		break;
									
					}
					break;
			}
			//Multiple load/store
			case 0xC:	if (SUPPORT_GETBIT11(instruction))
							sprintf(returnChar, "LDMIA");
						else
							sprintf(returnChar, "STMIA");
						break;
			//Conditional branch/software interrupt
			case 0xD:	condI = (instruction >> 8) & 0xF;
						offsetI = (instruction & 0xFF) << 24;
						offsetI = offsetI >> 23;	//Get sign bit.
						tempAddress = currentAddress+ offsetI+4;
						if ( ((instruction >> 8) & 0xF) <= 0xD)
							sprintf(returnChar, "B%s 0x%X", conditions[condI], tempAddress);
						else if ( ((instruction >> 8) & 0xF) == 0xF)
							sprintf(returnChar, "SWI %X", instruction & 0xFF);
						break;
			//Unconditional branch		
			case 0xE:	offsetI = (instruction & 0x7FF) << 21;
						offsetI = offsetI >> 20;
						tempAddress = currentAddress + offsetI+4;
						sprintf(returnChar, "B 0x%08X", tempAddress);
						break;
			/////////////////////////////////////////////
			//Long branch with link
			//Does some additional math to calculate the absolute address
			//and get a function name (if supported by stabs)
			/////////////////////////////////////////////
			case 0xF:	offsetI = instruction & 0x7FF;
						if (SUPPORT_GETBIT11(instruction)) {
							//In the case of a bll, calculate the actual address.
							u32 branchAddress;
							u32 previousInstruction;
							
							
							
							branchAddress = (currentAddress -2);
							index = branchAddress >> 24;
							offset = branchAddress & 0x00FFFFFF;
							if (gbaMem.memorySize[index] > offset) {
								//Get the previous instruction (presumably BLH)
								previousInstruction = *(u16*)(&gbaMem.u8Mem[index][offset]);
								offset = ((previousInstruction & 0x7FF) << 21);
								offset = offset >> 9;		//Get the sign bit.
								branchAddress += offset + (offsetI << 1) + 4;	//I think you need to add four
								//Try to get a function name from the elf file
								functionName = ELFFile_getFunctionName(&gba.elfFile, branchAddress);
							}
							//If it's not null, we got a function!
							if (functionName != NULL) {
								//Spit it into the output.
								sprintf(returnChar, "BLL 0x%08X (%s())", branchAddress, functionName);
							} else 
								sprintf(returnChar, "BLL 0x%X (0x%08X)", offsetI, branchAddress);
						} else {
							sprintf(returnChar, "BLH 0x%X", offsetI);

						}
						break;
		}//switch(type)
	}//if/else
	return returnChar;
}//parseThumb()