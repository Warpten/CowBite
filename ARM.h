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

#ifndef ARM_H
#define ARM_H

//#include "ALU.h"
//#include "ALU.cpp"

#include <time.h>
#include "Constants.h"
#include "Memory.h"
#include "Graphics.h"
#include "Stabs.h"
//#include "GBA.h"

//#define SCREEN_SIZE 0x1C200	//240 x 160 x 3 bytes
//#define SCREEN_SIZE 0x25800	//240 x 160 x 4 bytes



#define COND_EQ 0x0
#define COND_NE 0x1
#define COND_CS 0x2
#define COND_CC 0x3
#define COND_MI 0x4
#define COND_PL 0x5
#define COND_VS 0x6
#define COND_VC 0x7
#define COND_HI	0x8
#define COND_LS 0x9
#define COND_GE 0xA
#define COND_LT 0xB
#define COND_GT 0xC
#define COND_LE 0xD
#define COND_AL	0xE	//Always
#define COND_NV	0xF	//Never

#define MAX_BREAKPOINTS 1024
#define MAX_CONDBREAKPOINTS 16

#define BPVALUE_U8LOCATION 0x0
#define BPVALUE_U16LOCATION 0x1
#define BPVALUE_U32LOCATION 0x2
#define BPVALUE_HARDWAREREG 0x3
#define BPVALUE_REG 0x4
#define BPVALUE_CONSTANT 0x5
#define BPCOND_EQ	0x1
#define BPCOND_NE	0x2
#define BPCOND_AND	0x4
#define BPCOND_BAND 0x8
#define BPCOND_OR	0x10
#define BPCOND_BOR	0x20
#define BPCOND_NOT	0x40
#define BPCOND_BNOT	0x80
#define BPCOND_ACC	0x100

//Macro to find the 
#define R15CURRENT (arm.thumbMode ? (arm.r[15] - 4) : (arm.r[15] - 8))

typedef struct tBreakpointValue {
	u32 type;	//Type of the value
	u32 mask;	//Mask so we can distinguish bytes, hws, words,
	u32 lValue;	//The "l" value (GBA address or register)
	u32* value;		//The value ("r" value, or contents at the address)
	u32 constant;		//If the value is a constant, point to this
	char* name;	//Pointer to a name, if applicable.
} BreakpointValue;


typedef struct tConditionalBreakpoint {
	BreakpointValue value1;
	BreakpointValue value2;
	u32 condition;	//Conditions under which to break
	u32 breakpoint;	//Address at which to apply the condition
} ConditionalBreakpoint;

//////////////////////////////////////////////////////////////////
//This is a struct containing data relevant to the ARM.
//Intention is to eventually move all ARM data here.
typedef struct tARM {

	//The registers.  Very important.
	u32 r[16];

	//The following are pointers to the registers we need to swap in during a state change
	u32* r_usrPtr[16];
	u32* r_fiqPtr[16];
	u32* r_svcPtr[16];
	u32* r_abtPtr[16];
	u32* r_irqPtr[16];
	u32* r_undPtr[16];

	//These are the registers pointed to by the above
	u32 r_gp[16];	//For keeping the old register values when swapping in new ones
	//u32 r_usr[16];	//r_gp represents r_usr and r_system
	u32 r_fiq[16];
	u32 r_svc[16];
	u32 r_abt[16];
	u32 r_irq[16];
	u32 r_und[16];



	u32 thumbMode;			//Whether we're in thumb mode or not
	int instructionLength;	//Instruction length (in bytes)
	int instructionWidth;	//Instruction width (bits to shift by)

	u32 programStart;		//Address that the program starts at.
	u32 currentInstruction;
	/*u32 instruction1;
	u32 instruction2;
	u32 instruction3;*/

	u32 rn;					//Register values for operations
	u32 rd;
	u32 rm;
	u32 rc;
	u32 rs;
	u32 opcode;				//ALU opcode
	u32 op2;				//Additional ALU operand

	
	u32 location;			//The last accessed location in memory
									//useful for the debugger to see what we're doing.
	/////////////////////////////////////////////////////
	//CPU State variables
	u32 CPSR;		//Program state register
	u32* SPSR_current;	//Pointer to the current spsr...
	u32 SPSR_fiq;	//Registers for saving last state when changing modes
	u32 SPSR_svc;
	u32 SPSR_abt;
	u32 SPSR_irq;
	u32 SPSR_und;
	//extern u32 SPSR_sys;

	u32 N;
	u32 Z;
	u32 C;
	u32 V;
	
	/////////////////
	u32 halted;	//So we can tell if it's been halted...

	/////////////////////////////////////////////////
	//Debugging variables
	u32 paused;			//Flag indigating whether we have paused the CPU
	u32 debugging;		//Flag indicating whether we are in debug mode
	u32 numBreakpoints;	//Total number of breakpoints 
	u32 stepInto;			//Flag to let us know to stop at the next source line
	StabFunction* stepFunction;	//Function we are stepping *from*
	u32 stepAddress;		//When stepping over, the address to break at.
	u32 breakpoints[MAX_BREAKPOINTS];	//array of breakpoints
	u32 numCondBreakpoints;
	ConditionalBreakpoint condBreakpoints[MAX_BREAKPOINTS];
	u32 steps;			//The number of steps to skip by
	Stabs* stabs;		//A pointer to the stabs (unforutnately we need it)

	////////////////////////////////////////////////////
	//Function tables

	//This is a table with a condition for each entry
	u32  (*conditionTable[0x10])(void);

	//This is a table with a function for each possible permutation
	//of the instruction opcodes
	void  (*instructionTable[0x10000])(void);

	//This is a similar table for the thumb instructions
	void (*instructionTableThumb[0x10000])(void);

	/////////////////////////////////////////////////////

	
} ARM;

extern ARM arm;



//Functions needed for initialization
void ARM_init(Stabs* stabs);
void ARM_delete();


int ARM_reset();

//////////////////////////////////////////////////////
//Functions needed for regular operation fo the CPU
void ARM_fetch();
#define ARM_decode() 		//"decode" is just a placeholder
int ARM_execute();
//int ARM_debugExecute();
void ARM_nextInstruction();
void ARM_runNext ();
int ARM_changeCPSRState (u32 oldCPSR, u32 newCPSR);
int ARM_runToNextRefresh();	//What it sounds like.
void ARM_handleDebug();	
	
////////////////////////////////////////////////////
//Functions for initializing our tables of functions
void ARM_initTables();
void* ARM_branchFunction (u32 instruction);
void* ARM_dataOperationFunction(u32 instruction);
void* ARM_aluOperationFunction (u32 instruction, 
					void* func, void* funci, void* funcs, void* funcsi,
					void * funcRegShift, void* funcImmShift, void* funciShift,
					void * funcsRegShift, void* funcsImmShift, void* funcsiShift);
void* ARM_loadStoreFunction (u32 instruction);
void* ARM_loadStoreHalfwordFunction (u32 instruction);
	


////////////////////////////////////////////////////
//Instructions

//This is the default behavior for an instrution
void ARM_defaultInstruction();
void ARM_defaultInstructionThumb();


//Found in LoadStore.cpp
void ARM_loadStore ();
void ARM_loadStoreHalfword();
void ARM_loadStoreMultiple();
void ARM_swap();

//The following are found in ARMBranches.cpp
void ARM_branch ();
void ARM_swi();	//Not really a branch but close enough.

u32 ARM_isEQ (); 
u32 ARM_isNE ();
u32 ARM_isCS ();
u32 ARM_isCC ();
u32 ARM_isMI ();
u32 ARM_isPL ();
u32 ARM_isVS ();
u32 ARM_isVC ();
u32 ARM_isHI ();
u32 ARM_isLS ();
u32 ARM_isGE ();
u32 ARM_isLT ();
u32 ARM_isGT ();
u32 ARM_isLE ();
u32 ARM_always ();
u32 ARM_never ();

/*
extern u32 temp1;
extern u32 temp2;
extern u32 temp3;
extern u32* TEMP_dest;
extern u32* TEMP_source;
extern u16* TEMP_dest16;
extern u16* TEMP_source16;
extern u8* TEMP_dest8;
extern u8* TEMP_source8;
extern u32 TEMP_offset;
extern u32 TEMP_location;
extern u32 TEMP_offset;
extern u32 TEMP_location;
*/
//#define ARM_GETN() arm.N//(SUPPORT_GETBIT31(arm.CPSR))
//#define ARM_GETZ() arm.Z//(SUPPORT_GETBIT30(arm.CPSR))
//#define ARM_GETC() arm.C//(SUPPORT_GETBIT29(arm.CPSR))
//#define ARM_GETV() arm.V//(SUPPORT_GETBIT28(arm.CPSR))

///////////////////////////////////////////////////////
//fetch()
//Grab the next instruction from memory
///////////////////////////////////////////////////////
#define ARM_FETCH()	\
	register u32 instLocation = *(arm.r + 15) -4;									\
	register u32 index = ((instLocation & 0xFF000000) >> 24);				\
	register u32* instDest = *(gbaMem.u32Mem+index);								\
	register u32 instOffset = (instLocation & 0x00FFFFFF) >> 2;						\
	arm.currentInstruction = (u32)*(( u32*)(instDest + instOffset));	\
	*(arm.r + 15) += 4;	


//This version of fetch *does not* realistically implement the 
//instruction queue.  However, this *should* be invisible to the user.
__inline void ARM_fetch()	{
	register u32 instOffset = arm.r[15] -4;									
	//register u32 index = ((instOffset & 0xFF000000) >> 24);				
	register u32* instDest = *(gbaMem.u32Mem+  ((instOffset & 0xFF000000) >> 24)  );								
	instOffset = (instOffset & 0x00FFFFFF) >> 2;						
	arm.currentInstruction = (u32)*(( u32*)(instDest + instOffset));	
	arm.r[15] += 4;	
}

//Fetch a thumb instruction
__inline void ARM_fetchThumb()	{
	register u32 instOffset = arm.r[15] -2;									
	//register u32 index = ((instOffset & 0xFF000000) >> 24);				
	register u16* instDest = gbaMem.u16Mem[ ((instOffset & 0xFF000000) >> 24)];	
	instOffset = (instOffset & 0x00FFFFFF) >> 1;	
	arm.currentInstruction = instDest[instOffset];
	arm.currentInstruction = arm.currentInstruction & 0x0000FFFF;	
	arm.r[15] += 2;	
}

//This a the non-realistic instruction queue setup.  Instead of having
//three instructions, we only have one. Hopefully it's invisible to the user...
#define ARM_nextInstruction()	ARM_fetch()				
#define ARM_skipInstruction()	\
		*(arm.r + 15) += 4;		\
		ARM_fetch();

//For fetching thumb instructions.
#define ARM_nextInstructionThumb()	ARM_fetchThumb()				
#define ARM_skipInstructionThumb()	\
		*(arm.r + 15) += 2;		\
		ARM_fetchThumb();


//////////////////////////////////////////////////
//containsBreakpoint
//This function determines whether the given breakpoint 
//is in the breakpoint list.
inline int ARM_containsBreakpoint(u32 breakpoint) {
	for (int i = arm.numBreakpoints; i--;) {	//May want to optimize loop
		if (arm.breakpoints[i] == breakpoint) {
			return 1;
		}
	}
	return 0;
}

//////////////////////////////////////////////////
//reachedConditionalBreakpoint
//This function determines whether we reached a conditionalbraekpoint
///////////////////////////////////////////////////
inline int ARM_reachedConditionalBreakpoint(u32 currentAddress) {
	u32 temp, value1, value2;
	u32 checkCondBreakpoint;
	for (int i = arm.numCondBreakpoints; i--;) {
		temp = arm.condBreakpoints[i].breakpoint;
		//See if the user specified a berakpoint
		if (temp != 0xFFFFFFFF) {
			if (temp == currentAddress)
				checkCondBreakpoint = 1;
			else 
				checkCondBreakpoint = 0;
		} else {
			checkCondBreakpoint = 1;
		}
		if (checkCondBreakpoint) {
			value1 = (*arm.condBreakpoints[i].value1.value) & arm.condBreakpoints[i].value1.mask;
			value2 = (*arm.condBreakpoints[i].value2.value) & arm.condBreakpoints[i].value2.mask;
			switch (arm.condBreakpoints[i].condition) {
				
				case BPCOND_EQ:		return (value1 == value2);
				case BPCOND_NE:		return ( value1 != value2);
				case BPCOND_AND:	return ( value1 && value2);
				case BPCOND_BAND:	return ( value1 & value2);
				case BPCOND_OR:		return ( value1 || value2);
				case BPCOND_BOR:	return ( value1 | value2);
				case BPCOND_NOT:	return ( !value2);
				case BPCOND_BNOT:	return ( value1 == (~value2));
				case BPCOND_ACC:	if ( arm.condBreakpoints[i].value1.lValue == arm.location) {
										//We need to set arm.location to something else.
										//(otherwise it reaches the breakpoint repeatedly)
										arm.location = arm.location +4;
										return 1;
									}
			}
		}//if (good)
		
		
	}
	//If we get to this point, we must not have reached our condition.
	return 0;
}

/////////////////////////////////////////////////////////////////////////
//execute()
//This function executes arm instructions while checking to see if a
//breakpoint has been reached.
/////////////////////////////////////////////////////////////////////////
inline int ARM_execute() { 
	

	if (arm.thumbMode) {	//If we're in thumb mode, do things the thumb way.
		
		arm.instructionTableThumb[arm.currentInstruction]();

		if (arm.debugging)
			ARM_handleDebug();
//if ( *((u16*)(&gbaMem.u8IORAM[0xC6])) == 0x0440)
//		arm.paused = 1;

		return 4;
	} else {

		

		register u32 condition = arm.currentInstruction >> 28;	//The condition occupies the top four bits
		register u32 instructionCode = ((arm.currentInstruction & 0x0FF00000) >> 12) | ((arm.currentInstruction & 0x00000FF0) >> 4);

		//Begin instruction execution
		
		if ( (*(arm.conditionTable + condition))() ) {
		(*(arm.instructionTable+instructionCode))();
		
		}
		else {	//If it fails, stick another ARM_instruction in the queue
			ARM_nextInstruction();
		}

		if (arm.debugging) 
			ARM_handleDebug();

//		if ( *((u16*)(&gbaMem.u8IORAM[0xC6])) == 0x0440)
//		arm.paused = 1;
			/*
			{
			//See if we matched a breakpoint
			if (ARM_containsBreakpoint(arm.r[15] - 8))
				arm.paused = 1;
			
			if (ARM_reachedConditionalBreakpoint(arm.r[15] - 8))
				arm.paused = 1;

			//See if we are stepping
			if (arm.steps) {
				if ( ! (--arm.steps)) {
					arm.paused = 1;
				}
			}
		}*/
		return 4;
	}
	return 4;
}


///////////////////////////////////////////////////////
//changeCPSRState
//Perform necessary changes for changing of the CPSR state,
//such as switching register banks.
/////////////////////////////////////////////////
inline int ARM_changeCPSRState (u32 oldCPSRMode, u32 newCPSRMode) {
	u32** oldRegs;
	u32** newRegs;
	//int oldState = oldCPSR & 0x0000001F;
	
	//DEBUG
//	if ( temp3 != 0x3C)
//		int blah = 0;

	//if (graphics.lcy == 0x1c)
	//	int blah = 0;

	//This is the part where I copy all the registers over.
	//NOTE:  We save our state in the SPSR registers, but it is the responsiblitiy
	//of the BIOS to execute a SUBS PC, LR #4, which also restores the CPSR.
	if (oldCPSRMode != newCPSRMode) {
		//Because I decided to use variables instead of the register,
		//I must write those variables back to arm.CPSR whenever I wish
		//to use this register.
		arm.CPSR = arm.CPSR & 0x0FFFFFFF;	//Wipe out the sign bits
		arm.CPSR = arm.CPSR | (arm.N << 31);		//If zero, set N
		arm.CPSR = arm.CPSR | (arm.Z << 30);		//If negative, set Z
		arm.CPSR = arm.CPSR | (arm.C << 29 );		//If carry						
		arm.CPSR = arm.CPSR | (arm.V << 28);		//If overflow
		
		switch (oldCPSRMode) {
			//Could probably just switch on the lowest 4 bits...
			case 0x00:		//usr_26	26 bit PC User mode
			case 0x10:		//usarm.r_32  32 bit PC User Mode
			case 0x0F:		//Also for sys mode...
			case 0x1F:
							oldRegs = arm.r_usrPtr;
			/*				arm.r_gp[8] = arm.r[8];	
							arm.r_gp[9] = arm.r[9];
							arm.r_gp[10] = arm.r[10];
							arm.r_gp[11] = arm.r[11];
							arm.r_gp[12] = arm.r[12];
							arm.r_gp[13] = arm.r[13];
							arm.r_gp[14] = arm.r[14];
			*/				
							break;
		
			case 0x01:		//fiq_26  26 bit PC FIQ Mode
			case 0x11:		//fiq_32  32 bit PC FIQ Mode
							oldRegs = arm.r_fiqPtr;
							/*
							arm.r_fiq[8] = arm.r[8];	
							arm.r_fiq[9] = arm.r[9];
							arm.r_fiq[10] = arm.r[10];
							arm.r_fiq[11] = arm.r[11];
							arm.r_fiq[12] = arm.r[12];
							arm.r_fiq[13] = arm.r[13];
							arm.r_fiq[14] = arm.r[14];
							*/
							break;
			case 0x02:		//irq_26  26 bit PC IRQ Mode
			case 0x12:		//irq_32  32 bit PC IRQ Mode
							oldRegs = arm.r_irqPtr;
							/*
							arm.r_irq[13] = arm.r[13];
							arm.r_irq[14] = arm.r[14];
							*/
							break;
			case 0x03:		//svc_26  26 bit PC SVC Mode
			case 0x13:		//svc_32  32 bit PC SVC Mode
							oldRegs = arm.r_svcPtr;
							/*
							arm.r_svc[13] = arm.r[13];
							arm.r_svc[14] = arm.r[14];
							*/
							break;
			case 0x07:		//abt_26  26 bit PC Abt Mode
			case 0x17:		//abt_32  32 bit PC Abt Mode
							oldRegs = arm.r_abtPtr;
							/*
							arm.r_abt[13] = arm.r[13];
							arm.r_abt[14] = arm.r[13];
							*/
							break;
			case 0x0B:		//und_25  26 bit PC Und Mode
			case 0x1B:		//und_32  32 bit PC Und Mode
							oldRegs = arm.r_undPtr;
							/*
							arm.r_und[13] = arm.r[13];
							arm.r_und[14] = arm.r[14];
*/
			//case 0x0F:		//und_25  26 bit PC Sys Mode
			//case 0x1F:		//und_32  32 bit PC Sys Mode
							
							//arm.r_sys[13] = arm.r[13];
							//arm.r_sys[14] = arm.r[14];
							
							break;
			default:		break;

		}

		switch (newCPSRMode) {
			//If we're switching to user mode from some other mode...
			case 0x00:		//usr_26	26 bit PC User mode
			case 0x10:		//usr_32  32 bit PC User Mode
			case 0x0F:
			case 0x1F:		newRegs = arm.r_usrPtr;
							/*
							//Also for system mode...
							if ( (oldCPSRMode == 0x01) || (oldCPSRMode == 0x11)) {
								arm.r[8] = arm.r_gp[8];	
								arm.r[9] = arm.r_gp[9];
								arm.r[10] = arm.r_gp[10];
								arm.r[11] = arm.r_gp[11];
								arm.r[12] = arm.r_gp[12];
							}
							arm.r[13] = arm.r_gp[13];
							arm.r[14] = arm.r_gp[14];
							*/
							
							//Should this go here or in subsi()?
							//I have decided on subsi, because then we are assured
							//that the code gets executed whenever *returning*
							//from an exception, as opposed to just any old time.
							//arm.CPSR = *arm.SPSR_current;
							break;
		
			case 0x01:		//fiq_26  26 bit PC FIQ Mode
			case 0x11:		//fiq_32  32 bit PC FIQ Mode
							newRegs = arm.r_fiqPtr;
							/*
							arm.r[8] = arm.r_fiq[8];
							arm.r[9] = arm.r_fiq[9];
							arm.r[10] = arm.r_fiq[10];
							arm.r[11] = arm.r_fiq[11];
							arm.r[12] = arm.r_fiq[12];
							arm.r[13] = arm.r_fiq[13];
							arm.r[14] = arm.r_fiq[14];
							*/
							//arm.SPSR_fiq = arm.CPSR;  //Don't want to swap for ALL mode changes, only for exceptions
							arm.SPSR_current = &arm.SPSR_fiq;
							break;
			case 0x02:		//irq_26  26 bit PC IRQ Mode
			case 0x12:		//irq_32  32 bit PC IRQ Mode
							newRegs = arm.r_irqPtr;
							/*
							arm.r[13] = arm.r_irq[13];
							arm.r[14] = arm.r_irq[14];
							*/
							//arm.SPSR_irq = arm.CPSR;//Don't want to swap for ALL mode changes, only for exceptions
							arm.SPSR_current = &arm.SPSR_irq;
							break;
			case 0x03:		//svc_26  26 bit PC SVC Mode
			case 0x13:		//svc_32  32 bit PC SVC Mode
							newRegs = arm.r_svcPtr;
							/*							
							arm.r[13] = arm.r_svc[13];
							arm.r[14] = arm.r_svc[14];*/
							//arm.SPSR_svc = arm.CPSR;//Don't want to swap for ALL mode changes, only for exceptions
							arm.SPSR_current = &arm.SPSR_svc;
							break;
			case 0x07:		//abt_26  26 bit PC Abt Mode
			case 0x17:		//abt_32  32 bit PC Abt Mode
							newRegs = arm.r_abtPtr;
							/*
							arm.r[13] = arm.r_abt[13];
							arm.r[14] = arm.r_abt[13];*/
							//arm.SPSR_abt = arm.CPSR;	//Don't want to swap for ALL mode changes, only for exceptions
							arm.SPSR_current = &arm.SPSR_abt;
							break;
			case 0x0D:		//und_25  26 bit PC Und Mode
			case 0x1D:		//und_32  32 bit PC Und Mode
							newRegs = arm.r_undPtr;
							/*
							arm.r[13] = arm.r_und[13];
							arm.r[14] = arm.r_und[14];*/
							//arm.SPSR_und = arm.CPSR;//Don't want to swap for ALL mode changes, only for exceptions
							arm.SPSR_current = &arm.SPSR_und;
							break;
			default:		break;
		}


		

		//Save the data from the current registers into these pointers.
		*(oldRegs[8]) = arm.r[8];
		*(oldRegs[9]) = arm.r[9];
		*(oldRegs[10]) = arm.r[10];
		*(oldRegs[11]) = arm.r[11];
		*(oldRegs[12]) = arm.r[12];
		*(oldRegs[13]) = arm.r[13];
		*(oldRegs[14]) = arm.r[14];

		//Copy data back into them from the new registers.
		arm.r[8] = *(newRegs[8]);
		arm.r[9] = *(newRegs[9]);
		arm.r[10] = *(newRegs[10]);
		arm.r[11] = *(newRegs[11]);
		arm.r[12] = *(newRegs[12]);
		arm.r[13] = *(newRegs[13]);
		arm.r[14] = *(newRegs[14]);

		
		arm.N = arm.CPSR >> 31;
		arm.Z = (arm.CPSR >> 30) & 0x1;
		arm.C = (arm.CPSR >> 29) & 0x1;
		arm.V = (arm.CPSR >> 28) & 0x1;
		
		//Finally, update the mode bits
		arm.CPSR = arm.CPSR & 0xFFFFFFE0;
		arm.CPSR = arm.CPSR | newCPSRMode;
		
	}//if
	return 0;
}//changeState

//////////////////////////////////////////////
//swapSPSR
//Swaps in the SPSR corresponding to the current mode,
//then forwards the instruction queue depending on
//whether we are in ARM or thumb mode
///////////////////////////////////////////////////
inline void ARM_swapSPSR () {
	u32 oldMode = arm.CPSR & 0x1F;
	u32 newMode = *arm.SPSR_current & 0x1F;
	ARM_changeCPSRState(oldMode, newMode);	//Swap registers.
	oldMode &= 0xF;	//Get the lowest 4 bits of the old mode
	if ( ( oldMode != 0x0) && (oldMode != 0xF) )
		arm.CPSR = *arm.SPSR_current;		//Get the CPSR back (only in exception modes)
		
	arm.N = arm.CPSR >> 31;
	arm.Z = (arm.CPSR >> 30) & 0x1;
	arm.C = (arm.CPSR >> 29) & 0x1;
	arm.V = (arm.CPSR >> 28) & 0x1;
	
	if (arm.CPSR & 0x00000020) {
		arm.thumbMode = 1;
		ARM_skipInstructionThumb();
	} else {
		arm.thumbMode = 0;
		ARM_skipInstruction();
	}
}
 

#endif

