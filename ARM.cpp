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
#include <stdlib.h>
#include <math.h>
#include <memory.h>
#include <time.h>
#include <string.h>
//#include <windows.h>

#include "ARM.h"
#include "Memory.h"	
#include "ALU.h"
#include "Support.h"
#include "Graphics.h"
#include "ARMBranches.h"	//Must include becuase of inline function
#include "ARMDataop.h"		//""
		//""
#include "ARMLoadStore.h"	//""
#include "ARMThumb.h"
#include "Gui.h"	//Need this to be able to give focus to asm window :(
/*
u32 temp1, temp2, temp3;*/
ARM arm;

//////////////////////////////////////////////
//Constructor
//The parameter "stabs" is for debugging.
//I couldn't find a better way around it. sorry.
/////////////////////////////////////////////////////
void ARM_init(Stabs* stabs) {
	memset(&arm, 0, sizeof(ARM));
	arm.stabs = stabs;
	ARM_initTables();	//Initialize instruction tables first.
	
	//Initialize register pointers.
	for (int i = 0; i < 16; i++) {
		arm.r[i] = arm.r_gp[i];	//'r' is what the user sees.
		
		//Set them al to point to r_gp...
		arm.r_usrPtr[i] = &(arm.r_gp[i]);
		arm.r_fiqPtr[i] = &(arm.r_gp[i]);
		arm.r_svcPtr[i] = &(arm.r_gp[i]);
		arm.r_abtPtr[i] = &(arm.r_gp[i]);
		arm.r_irqPtr[i] = &(arm.r_gp[i]);
		arm.r_undPtr[i] = &(arm.r_gp[i]);

	}	
	
	//Now set the pointers to registers we want to be able to swap in.
	arm.r_fiqPtr[8] = &(arm.r_fiq[8]);
	arm.r_fiqPtr[9] = &(arm.r_fiq[9]);
	arm.r_fiqPtr[10] = &(arm.r_fiq[10]);
	arm.r_fiqPtr[11] = &(arm.r_fiq[11]);
	arm.r_fiqPtr[12] = &(arm.r_fiq[12]);
	arm.r_fiqPtr[13] = &(arm.r_fiq[13]);
	arm.r_fiqPtr[14] = &(arm.r_fiq[14]);

	arm.r_svcPtr[13] = &(arm.r_svc[13]);
	arm.r_svcPtr[14] = &(arm.r_svc[14]);

	arm.r_abtPtr[13] = &(arm.r_abt[13]);
	arm.r_abtPtr[14] = &(arm.r_abt[14]);

	arm.r_irqPtr[13] = &(arm.r_irq[13]);
	arm.r_irqPtr[14] = &(arm.r_irq[14]);

	arm.programStart = 0x08000000;	//Set the program start.
	arm.location = 0x08000000;
	arm.thumbMode = 0;
	
	arm.SPSR_current = &arm.SPSR_svc;

//	temp1 = temp2 = temp3 = 0;
	arm.paused = arm.debugging = 0;
	arm.numBreakpoints = 0;
	arm.numCondBreakpoints = 0;
	
}//ARM()

////////////////////////////////////////
//Destructor
///////////////////////////////////////
void ARM_delete() {
	
}

/////////////////////////////////////////////



//This happily resets the ARM to default values
int ARM_reset() {
//	setState(&nullState);

	//I'm not sure whether a *real* ARM attempts to load up the queue on
	//a reset, but why not?
	//Get the first arm.instruction into the queue
	arm.r[15] = arm.programStart;
	arm.r[15]+=4;	//Start off 4 ahead of where we were
	ARM_nextInstruction();
		//ARM_fetch ();
		//arm.instruction2 = arm.instruction1;
		//ARM_clockTick();	

		//To be honest, this is real guesswork, because the first arm.instruction
		//of the first demo I chose to test was a branch.
		//It may be necessary to remove this second fetch in order to get PC value
		//correctly
		//ARM_fetch ();
		//ARM_decode();
		//arm.instruction3 = arm.instruction2;
		//arm.instruction2 = arm.instruction1;
	return 0;
}

//////////////////////////////////////////////
//handleDebug
//This function handles any debugging (check for breakpoints, etc.)
//needed at the current instruction.  Usually with the end result
//of either pausing or not pausing the CPU.
//////////////////////////////////////////////
void ARM_handleDebug() {
	//This is slow.  Would it be good to conver the code over to setting arm.r to the 
	//current address rather than two instructions ahead???
	u32 breakAddress = R15CURRENT;
		
	if (ARM_containsBreakpoint(breakAddress))	{//See if we matched a breakpoint
		arm.paused = 1;
		arm.steps = arm.stepAddress = arm.stepInto = 0;
	} else if (ARM_reachedConditionalBreakpoint(breakAddress)) {
		arm.paused = 1;
		arm.steps = arm.stepAddress = arm.stepInto = 0;
		arm.stepFunction = NULL;
	} 
	else if (arm.stepInto) {
		u32 address = R15CURRENT;
		StabSourceFile* sourceFile = Stabs_findSourceByAddress(arm.stabs, address);
		if (sourceFile != NULL ) {
			if (Stabs_findByValue((StabListEntry*)&sourceFile->lineList, address) != NULL) {
				arm.paused = 1;
				arm.steps = arm.stepAddress = arm.stepInto = 0;
				arm.stepFunction = NULL;
			}
		} else {
			//This means we stepped right out of a file.  
			arm.steps = arm.stepAddress = arm.stepInto = 0;
			arm.stepFunction = NULL;
			SetFocus(appState.hDlg);
			arm.paused = 1;
		}
	
	
	//In this implementation, we go to the next sourceline we reach *within this function*
	} else if (arm.stepFunction) {
		u32 address = R15CURRENT;
		//If we're in the current function...
		if ( (address >= arm.stepFunction->functionBegin) && (address <= arm.stepFunction->blockEnd)) {
			//Get the current source file
			StabSourceFile* sourceFile = Stabs_findSourceByAddress(arm.stabs, address);
			//See if we match a line number.
			if (sourceFile != NULL ) {
				if (Stabs_findByValue((StabListEntry*)&sourceFile->lineList, address) != NULL) {
					arm.paused = 1;
					arm.steps = arm.stepAddress = arm.stepInto = 0;
					arm.stepFunction = NULL;
				}
			} 
		}
		
	//arm.steps indicates the number of steps *including* the starting
	//instruction.  So an arm.steps of 1 is equivalent to not stepping at all.
	//An arm.steps of 2 is a single step.
	}else if (arm.steps) {	
		if ( ! (--arm.steps)) {
			arm.paused = 1;
			arm.steps = arm.stepAddress = arm.stepInto = 0;
			arm.stepFunction = NULL;
		}
	} 
}

//////////////////////////////////////////////////////
//defaultInstruction
//This is mostly dummy code to serve as a model for
//other arm.instructions
//////////////////////////////////////////////////////
void ARM_defaultInstruction() {  //u32 arm.instruction) {
	ARM_nextInstruction();
}

////////////////////////////////////////////////////////
//defaultInstructionThumb
//This is dumby code to serve in place of thumb instructions.
////////////////////////////////////////////////////////
void ARM_defaultInstructionThumb() {
	ARM_nextInstruction();
}


///////////////////////////////////////////////////////
//initTables()
//This function loads a function corresponding to each instruction
//into a table.  Though I normally dislike such methodology,
//it is necessary in order to achieve acceptable emulation
//speed.
////////////////////////////////////////////////////////
void ARM_initTables() {
	u32 instruction;
	u32 type;

	//First initialize the condition table with one entry for
	//each possible condition value
	arm.conditionTable[COND_EQ] = ARM_isEQ;
	arm.conditionTable[COND_NE] = ARM_isNE;
	arm.conditionTable[COND_CS] = ARM_isCS;
	arm.conditionTable[COND_CC] = ARM_isCC;
	arm.conditionTable[COND_MI] = ARM_isMI;
	arm.conditionTable[COND_PL] = ARM_isPL;
	arm.conditionTable[COND_VS] = ARM_isVS;
	arm.conditionTable[COND_VC] = ARM_isVC;
	arm.conditionTable[COND_HI] = ARM_isHI;
	arm.conditionTable[COND_LS] = ARM_isLS;
	arm.conditionTable[COND_GE] = ARM_isGE;
	arm.conditionTable[COND_LT] = ARM_isLT;
	arm.conditionTable[COND_GT] = ARM_isGT;
	arm.conditionTable[COND_LE] = ARM_isLE;
	arm.conditionTable[COND_AL] = ARM_always;
	arm.conditionTable[COND_NV] = ARM_never;

	//This is how we fill up the instruction table
	for (u32 i = 0; i <= 0xFFFF; i++) {
		//First convert from "condensed" format into something resembling an
		//actual instruction
		instruction = (i & 0xFF00) << 12;	//Get bits 20 - 27
		instruction = instruction | ((i & 0x00FF) << 4);	//ANd bits 4 - 11

		//Get the type of instruction
		type = (instruction >> 25) & 0x7;

		//Switch on the type, then divvy up the work accordingly.
		switch (type) {
			case 0x0:	//For a mul, bits 27 - 22 are always 0, bits 4 - 7 always 1001
						if ( SUPPORT_BITCOMPARE(instruction, 0x0FC000F0, 0x00000090)) {
							arm.instructionTable[i] = ARM_mul;
							
							break;
						}
						//For a mull, bits 27 - 24 are always 0, 
						//bit 23 is a 1, and bits 4 - 7 always 1001
						if ( SUPPORT_BITCOMPARE(instruction, 0x0F8000F0, 0x00800090)) {
							arm.instructionTable[i] = ARM_mull;
							
							break;
						}

						//For a swap, bits 27 -24 are always 0001, bits 4- 7 are always 1001
						else if (SUPPORT_BITCOMPARE(instruction, 0x0F0000F0, 0x01000090)) {
							arm.instructionTable[i] = ARM_swap;
							break;
						}
						//For a halfword load/store, bits 27 - 25 are always 0, bits 4 - 7 are
						//1xx1, where xx is not 00 (would be a swap or mul, above)
						else if (SUPPORT_BITCOMPARE(instruction, 0x0E000090, 0x00000090)) {
							arm.instructionTable[i] = (void (__fastcall *)(void))ARM_loadStoreHalfwordFunction(instruction);
							break;
						}
						//If it passes these, check to see if it's a BX.  Note that the
						//only thing to distinguish BX from an MSR using our shortened 16 bit
						//codes is the fact that in an MSR the rd can never be r15
						else if (SUPPORT_BITCOMPARE(instruction, 0x0FF00FF0, 0x01200F10)) {
							arm.instructionTable[i] = ARM_bx;
							break;
						}

						//Note there is no break here
			case 0x1:	arm.instructionTable[i] = (void (__fastcall *)(void))ARM_dataOperationFunction(instruction);
						break;

			case 0x5:	arm.instructionTable[i] = (void (__fastcall *)(void))ARM_branchFunction(instruction);
						break;
			case 0x2:
			case 0x3:	arm.instructionTable[i] = (void (__fastcall *)(void))ARM_loadStoreFunction(instruction);
						break;

			case 0x4:	arm.instructionTable[i] = ARM_loadStoreMultiple;
						break;
			case 0x7:	if ((instruction >> 24) & 0x1) {	//If bit 24 is also set..
							arm.instructionTable[i] = (void (__fastcall *)(void))ARM_swi;
							break;
						}

			case 0x6:	arm.instructionTable[i] = ARM_defaultInstruction;
						break;
			default:	arm.instructionTable[i] = ARM_defaultInstruction;
						break;		
			

		}//End of switch
	}//End of for

	//This is how we fill up the thumb table
	for (instruction = 0; instruction<=0xFFFF; instruction++) {
		type = instruction >> 13;	//Get the top 3 bits
		if (type == 0x0 ) {	//Move shifted/add/subtract
			//Switch on the opcode
			switch ( (instruction >> 11) & 0x3) {
				//Move shifted
				case 0x0:		arm.instructionTableThumb[instruction] = ARM_lslThumb;
								break;
				case 0x1:		arm.instructionTableThumb[instruction] = ARM_lsrThumb;
								break;
				case 0x2:		arm.instructionTableThumb[instruction] = ARM_asrThumb;
								break;
				//Add/subtract
				case 0x3:		switch ( (instruction >> 9) & 0x3) {
									case 0x0:	arm.instructionTableThumb[instruction] = ARM_addThumb;
												break;
									case 0x1:	arm.instructionTableThumb[instruction] = ARM_subThumb;
												break;
									case 0x2:	arm.instructionTableThumb[instruction] = ARM_addiThumb;
												break;
									case 0x3:	arm.instructionTableThumb[instruction] = ARM_subiThumb;
												break;
								}//switch
				
								break;
			}
		} else if (type == 0x1) {	//Move/compare/add/subtract immediate
			switch ( (instruction >> 11) & 0x3) {
				case 0x0:	arm.instructionTableThumb[instruction] = ARM_movImmediateThumb;
							break;
				case 0x1:	arm.instructionTableThumb[instruction] = ARM_cmpImmediateThumb;
							break;
				case 0x2:	arm.instructionTableThumb[instruction] = ARM_addImmediateThumb;
							break;
				case 0x3:	arm.instructionTableThumb[instruction] = ARM_subImmediateThumb;
							break;
			}//switch
		} else {
			type = instruction >> 12;	//Get the top 4 bits
			switch (type) {
				//ALU operations / Hi register operations/branch exchange / PC relative load
				case 0x4:	
					if ( ((instruction >> 10) & 0x3) == 0) {	//ALU operation
						switch ( (instruction >> 6) & 0xF) {
							case 0x0:	arm.instructionTableThumb[instruction] = ARM_andThumb;
										break;
							case 0x1:	arm.instructionTableThumb[instruction] = ARM_eorThumb;
										break;
							case 0x2:	arm.instructionTableThumb[instruction] = ARM_lslRegThumb;
										break;
							case 0x3:	arm.instructionTableThumb[instruction] = ARM_lsrRegThumb;
										break;
							case 0x4:	arm.instructionTableThumb[instruction] = ARM_asrRegThumb;
										break;
							case 0x5:	arm.instructionTableThumb[instruction] = ARM_adcThumb;
										break;
							case 0x6:	arm.instructionTableThumb[instruction] = ARM_sbcThumb;
										break;
							case 0x7:	arm.instructionTableThumb[instruction] = ARM_rorThumb;
										break;
							case 0x8:	arm.instructionTableThumb[instruction] = ARM_tstThumb;
										break;
							case 0x9:	arm.instructionTableThumb[instruction] = ARM_negThumb;
										break;
							case 0xA:	arm.instructionTableThumb[instruction] = ARM_cmpThumb;
										break;
							case 0xB:	arm.instructionTableThumb[instruction] = ARM_cmnThumb;
										break;
							case 0xC:	arm.instructionTableThumb[instruction] = ARM_orrThumb;
										break;
							case 0xD:	arm.instructionTableThumb[instruction] = ARM_mulThumb;
										break;
							case 0xE:	arm.instructionTableThumb[instruction] = ARM_bicThumb;
										break;
							case 0xF:	arm.instructionTableThumb[instruction] = ARM_mvnThumb;
										break;
						}//switch
					} else if ( ((instruction >> 10) & 0x3) == 0x1) {	//Hi register operations
						switch ( (instruction >>8 ) & 0x3) {
							case 0x0:	arm.instructionTableThumb[instruction] = ARM_addhiThumb;
										break;
							case 0x1:	arm.instructionTableThumb[instruction] = ARM_cmphiThumb;
										break;
							case 0x2:	arm.instructionTableThumb[instruction] = ARM_movhiThumb;
										break;
							case 0x3:	arm.instructionTableThumb[instruction] = ARM_bxhiThumb;
										break;

						}//switch
					} else {	//PC - relative load
						arm.instructionTableThumb[instruction] = ARM_ldrpcThumb;
						break;
					}
					break;
				//load store.  note that the ARM docs are completely wrong about the opcodes
				//for this, as far as I can tell
				case 0x5:	
						switch ( (instruction >> 9) & 0x7) {
							case 0x0:	arm.instructionTableThumb[instruction] = ARM_strThumb;
										break;
							case 0x1:	arm.instructionTableThumb[instruction] = ARM_strhThumb;
										break;
							case 0x2:	arm.instructionTableThumb[instruction] = ARM_strbThumb;
										break;
							case 0x3:	arm.instructionTableThumb[instruction] = ARM_ldsbThumb;
										break;
							case 0x4:	arm.instructionTableThumb[instruction] = ARM_ldrThumb;
										break;
							case 0x5:	arm.instructionTableThumb[instruction] = ARM_ldrhThumb;
										break;
							case 0x6:	arm.instructionTableThumb[instruction] = ARM_ldrbThumb;
										break;
							case 0x7:	arm.instructionTableThumb[instruction] = ARM_ldshThumb;
										break;							
						}
						break;
				//load/store immediate word
				case 0x6:	if (SUPPORT_GETBIT11(instruction))	//load
								arm.instructionTableThumb[instruction] = ARM_ldriThumb;
							else								//store
								arm.instructionTableThumb[instruction] = ARM_striThumb;
							break;
				//load/store imediate byte
				case 0x7:	if (SUPPORT_GETBIT11(instruction))	//load
								arm.instructionTableThumb[instruction] = ARM_ldrbiThumb;
							else
								arm.instructionTableThumb[instruction] = ARM_strbiThumb;
							break;
				//load store halfword
				case 0x8:	if (SUPPORT_GETBIT11(instruction))	//load
								arm.instructionTableThumb[instruction] = ARM_ldrhiThumb;
							else
								arm.instructionTableThumb[instruction] = ARM_strhiThumb;
							break;
				//SP relative load/store
				case 0x9:	if (SUPPORT_GETBIT11(instruction))	//load
								arm.instructionTableThumb[instruction] = ARM_ldrspThumb;
							else
								arm.instructionTableThumb[instruction] = ARM_strspThumb;
							break;
				//load address
				case 0xA:	if (SUPPORT_GETBIT11(instruction))	//sp
								arm.instructionTableThumb[instruction] = ARM_adrlspThumb;
							else								//pc
								arm.instructionTableThumb[instruction] = ARM_adrlpcThumb;
							break;
				//Add offset to stack pointer/push/pop
				case 0xB:	
						switch ((instruction >> 7) & 0x1F) {
							case 0x0:	arm.instructionTableThumb[instruction] = ARM_addspThumb;
										break;
							case 0x1:	arm.instructionTableThumb[instruction] = ARM_subspThumb;
										break;
							case 0x8:
							case 0x9:
							case 0xA:
							case 0xB:	arm.instructionTableThumb[instruction] = ARM_pushThumb;
										break;
							case 0x18:	
							case 0x19: 
							case 0x1A:
							case 0x1B:  arm.instructionTableThumb[instruction] = ARM_popThumb;
										break;
							}
							break;
				//Multiple load/store
				case 0xC:	if (SUPPORT_GETBIT11(instruction))
								arm.instructionTableThumb[instruction] = ARM_ldmiaThumb;
							else
								arm.instructionTableThumb[instruction] = ARM_stmiaThumb;
							break;
				//Conditional branch/software interrupt
				case 0xD:	
							if ( ((instruction >> 8) & 0xF) <= 0xD)
								arm.instructionTableThumb[instruction] = ARM_bcondThumb;
							else if ( ((instruction >> 8) & 0xF) == 0xF)
								arm.instructionTableThumb[instruction] = ARM_swiThumb;
							break;
				//Unconditional branch								
				case 0xE:	arm.instructionTableThumb[instruction] = ARM_bThumb;
							break;
				//Long branch with link
				case 0xF:	arm.instructionTableThumb[instruction] = ARM_blThumb;
							break;
			}//switch(type)
		}//if/else
	}
	//Fill in the thumb noop with the dprint statement.
	arm.instructionTableThumb[0x4000] = ARM_dprintThumb;
}//initTables()


//////////////////////////////////////////
//Conditional functions
//These return a true or false value based on the
//flags in the CPSR
//////////////////////////////////////////
inline u32 ARM_isEQ () {return arm.Z;}
inline u32 ARM_isNE () {return !arm.Z;}
inline u32 ARM_isCS () {return arm.C;}
inline u32 ARM_isCC () {return !arm.C;}
inline u32 ARM_isMI () {return arm.N;}
inline u32 ARM_isPL () {return !arm.N;}
inline u32 ARM_isVS () {return arm.V;}
inline u32 ARM_isVC () {return !arm.V;}
inline u32 ARM_isHI () {return arm.C && !arm.Z;}
inline u32 ARM_isLS () {return !arm.C || arm.Z;}
inline u32 ARM_isGE () {return arm.N == arm.V;}
inline u32 ARM_isLT () {return arm.N != arm.V;}
inline u32 ARM_isGT () {return (arm.N == arm.V) && !arm.Z;}
inline u32 ARM_isLE () {return (arm.N != arm.V) || arm.Z;}
inline u32 ARM_always () {return 1;}
inline u32 ARM_never () { return 0;}



