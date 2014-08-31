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
#include <stdio.h>
#include "GBA.h"
#include "ARM.h"
#include "ALU.h"
#include "Support.h"
#include "Console.h"
//#include "ARMExtearm.rn.h"
#include "ARMDataOp.h"	//Must include because of inlines

//#include "ARMC.h"


/////////////////////////////////////////
//dataOperationFucntion(u32 instruction)
//This returns a function based on the instruction passed in.
//The function can then be placed in a table and used again later.
////////////////////////////////////////////////
void* ARM_dataOperationFunction(u32 instruction) {
	u32 opcode;

	opcode = SUPPORT_GETOP(instruction); 
	//Get the opcode and the operands
	switch (opcode) {
		case OP_AND:
			return ARM_aluOperationFunction(instruction, ARM_and, ARM_andi, ARM_ands, ARM_andsi,
										ARM_andRegShift, ARM_andImmShift, ARM_andiShift,
										ARM_andsRegShift, ARM_andsImmShift, ARM_andsiShift);
		case OP_EOR:
			return ARM_aluOperationFunction(instruction, ARM_eor, ARM_eori, ARM_eors, ARM_eorsi,
										ARM_eorRegShift, ARM_eorImmShift, ARM_eoriShift,
										ARM_eorsRegShift, ARM_eorsImmShift, ARM_eorsiShift);
		case OP_SUB:
			return ARM_aluOperationFunction(instruction, ARM_sub, ARM_subi, ARM_subs, ARM_subsi,
										ARM_subRegShift, ARM_subImmShift, ARM_subiShift,
										ARM_subsRegShift, ARM_subsImmShift, ARM_subsiShift);
		case OP_RSB:
			return ARM_aluOperationFunction(instruction, ARM_rsb, ARM_rsbi, ARM_rsbs, ARM_rsbsi,
										ARM_rsbRegShift, ARM_rsbImmShift, ARM_rsbiShift,
										ARM_rsbsRegShift, ARM_rsbsImmShift, ARM_rsbsiShift);
		case OP_ADD:
			return ARM_aluOperationFunction(instruction, ARM_add, ARM_addi, ARM_adds, ARM_addsi,
										ARM_addRegShift, ARM_addImmShift, ARM_addiShift,
										ARM_addsRegShift, ARM_addsImmShift, ARM_addsiShift);
		case OP_ADC:
			return ARM_aluOperationFunction(instruction, ARM_adc, ARM_adci, ARM_adcs, ARM_adcsi,
										ARM_adcRegShift, ARM_adcImmShift, ARM_adciShift,
										ARM_adcsRegShift, ARM_adcsImmShift, ARM_adcsiShift);
		case OP_SBC:
			return ARM_aluOperationFunction(instruction, ARM_sbc, ARM_sbci, ARM_sbcs, ARM_sbcsi,
										ARM_sbcRegShift, ARM_sbcImmShift, ARM_sbciShift,
										ARM_sbcsRegShift, ARM_sbcsImmShift, ARM_sbcsiShift);
		case OP_RSC:
			return ARM_aluOperationFunction(instruction, ARM_rsc, ARM_rsci, ARM_rscs, ARM_rscsi,
										ARM_rscRegShift, ARM_rscImmShift, ARM_rsciShift,
										ARM_rscsRegShift, ARM_rscsImmShift, ARM_rscsiShift);
		case OP_TST:
			return ARM_aluOperationFunction(instruction, ARM_mrs, ARM_mrs, ARM_tsts, ARM_tstsi,
										ARM_mrs, ARM_mrs, ARM_mrs,
										ARM_tstsRegShift, ARM_tstsImmShift, ARM_tstsiShift);
		case OP_TEQ:
			return ARM_aluOperationFunction(instruction, ARM_msr, ARM_msri, ARM_teqs, ARM_teqsi,
										ARM_msr, ARM_msr, ARM_msriShift,
										ARM_teqsRegShift, ARM_teqsImmShift, ARM_teqsiShift);
		case OP_CMP:
			return ARM_aluOperationFunction(instruction, ARM_mrs, ARM_mrs, ARM_cmps, ARM_cmpsi,
										ARM_mrs, ARM_mrs, ARM_mrs,
										ARM_cmpsRegShift, ARM_cmpsImmShift, ARM_cmpsiShift);
		case OP_CMN:
			return ARM_aluOperationFunction(instruction, ARM_msr, ARM_msri, ARM_cmns, ARM_cmnsi,
										ARM_msr, ARM_msr, ARM_msriShift,
										ARM_cmnsRegShift, ARM_cmnsImmShift, ARM_cmnsiShift);
		case OP_ORR:
			return ARM_aluOperationFunction(instruction, ARM_orr, ARM_orri, ARM_orrs, ARM_orrsi,
										ARM_orrRegShift, ARM_orrImmShift, ARM_orriShift,
										ARM_orrsRegShift, ARM_orrsImmShift, ARM_orrsiShift);
		case OP_MOV:
			return ARM_aluOperationFunction(instruction, ARM_mov, ARM_movi, ARM_movs, ARM_movsi,
										ARM_movRegShift, ARM_movImmShift, ARM_moviShift,
										ARM_movsRegShift, ARM_movsImmShift, ARM_movsiShift);
		case OP_BIC:
			return ARM_aluOperationFunction(instruction, ARM_bic, ARM_bici, ARM_bics, ARM_bicsi,
										ARM_bicRegShift, ARM_bicImmShift, ARM_biciShift,
										ARM_bicsRegShift, ARM_bicsImmShift, ARM_bicsiShift);
		case OP_MVN:
			return ARM_aluOperationFunction(instruction, ARM_mvn, ARM_mvni, ARM_mvns, ARM_mvnsi,
										ARM_mvnRegShift, ARM_mvnImmShift, ARM_mvniShift,
										ARM_mvnsRegShift, ARM_mvnsImmShift, ARM_mvnsiShift);
		break;
	}
	
	//If somehow it makes it past this (some kind of error?)
	//return a default value.
	return NULL;	
}

///////////////////////////////////////////
//void* aluOperationFunction()
//Determines which version of an ALU operation to use based on flags 
//in the instruction. Note that for each ALU operation, I have TEN
//different permutations, all varations on the basic operation,
//an immediate operation, an operation with the S bit set, an operation
//with an immediate shift, and an operation with a register shift.
/////////////////////////////////////////////
void* ARM_aluOperationFunction(u32 instruction, 
							void* func, void* funci, void* funcs, void* funcsi,
							void * funcRegShift, void* funcImmShift, void* funciShift,
							void * funcsRegShift, void* funcsImmShift, void* funcsiShift) {
	
	u32 s, i, regShift, shiftAmt, shiftType;
	
	s = SUPPORT_GETBIT20(instruction);	//Status bit set?
	i = SUPPORT_GETBIT25(instruction);	//Is it an immediate value?
	regShift = SUPPORT_GETBIT4(instruction);	//Is it shifted by a register value?
	
	if ( i ) { //If it's immediate
		//Get the shift
		shiftAmt = (instruction & 0x00000F00 ) >> 7;
	
		if (s)
			if (shiftAmt)
				return funcsiShift;
			else
				return funcsi;
		else
			if (shiftAmt)
				return funciShift;
			else
				return funci;
		
	} else {	//If the operand is a register
		
		//Find out if it's an immediate shift or a reg shift
		if ( regShift ) { 	//It's a reg shift
			
			if (s)
				return funcsRegShift;
			else
				return funcRegShift;

		} else {					//It's an immediate shift
			shiftAmt = (instruction >> 7) & 0x1F; //Support::getBits (arm.currentInstruction, 11, 7);
			shiftType = (instruction >> 5) & 0x3;
			if (s)
				if (shiftAmt || (shiftType != SHIFT_LSL))	//Always perform shift, unless lsl 0 specified
					return funcsImmShift;
				else
					return funcs;
			else
				if (shiftAmt || (shiftType != SHIFT_LSL))	//Always peform shift, unless lsl 0
					return funcImmShift;
				else
					return func;
		}//end of if/else(regShift)
		
	}//end of if/else(i)

	//It would never get through this logic, but if somehow it did,
	//return a noop.
	return ARM_defaultInstruction;
}

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////
//BEGIN HUGE LIST OF FUNCTIONS
//In the case
//of data operations, when the destination register is 15, the
//instruction queue is supposed to be invalidated and then filled up.
//TODO:  Implement the case where r15 is used as an operand with a register
//shift.
////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////
//and() functions
///////////////////////////////////////////////////////

//This is a special version of a noop that prints to the console.
//Invented by joat.  Slick!
void ARM_dprint() {
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
}

//and with no shift
void ARM_and() {
	if (arm.currentInstruction == 0xE0000000) {
		ARM_dprint();
	}
	RD(arm.currentInstruction) = RN(arm.currentInstruction) & RM(arm.currentInstruction);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//and immediate with no shift
void ARM_andi() {
	RD(arm.currentInstruction) = RN(arm.currentInstruction) & IMM(arm.currentInstruction);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//and, no shift, S bit set
void ARM_ands() {
	register u32 tempC = RD(arm.currentInstruction) = RN(arm.currentInstruction) & RM(arm.currentInstruction);
	ALU_setNZFlags(tempC);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
	
}

//and immediate, no shift, S bit set
void ARM_andsi() {
	register u32 tempC = RD(arm.currentInstruction) = RN(arm.currentInstruction) & IMM(arm.currentInstruction);
	ALU_setNZFlags(tempC);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
}

//and with a reg shift
void ARM_andRegShift() {
	RD(arm.currentInstruction) = RN(arm.currentInstruction) & REGSHIFT(arm.currentInstruction);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//and with an immediate shift
void ARM_andImmShift() {
	RD(arm.currentInstruction) = RN(arm.currentInstruction) & IMMSHIFT(arm.currentInstruction);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}


//andi with shift
void ARM_andiShift() {
	register u32 tempC = ((arm.currentInstruction & 0xF00 ) >> 7);	//Get the shift amount
	register u32 op2 = IMM(arm.currentInstruction);
	RD(arm.currentInstruction) = RN(arm.currentInstruction) & IMMROT(op2, tempC);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//ands with a reg shift
void ARM_andsRegShift() {
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = REGSHIFTS(arm.currentInstruction);
	register u32 tempC = RD(arm.currentInstruction) = tempA & tempB;
	ALU_setNZFlags(tempC);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
}

//ands with an immediate shift 
void ARM_andsImmShift() {
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = IMMSHIFTS(arm.currentInstruction);
	register u32 tempC = RD(arm.currentInstruction) = tempA & tempB;
	ALU_setNZFlags(tempC);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
}

//ands immediate with shift
void ARM_andsiShift() {
	u32 shift = ((arm.currentInstruction & 0xF00 ) >> 7);	//Get the shift
	register u32 op2 = IMM(arm.currentInstruction);
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = IMMROT(op2, shift);
	register u32 tempC = RD(arm.currentInstruction) = tempA & tempB;
	ALU_setNZFlags(tempC);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
}

///////////////////////////////////////////////////////
//eor() functions
///////////////////////////////////////////////////////
//eor with no shift
void ARM_eor() {
	RD(arm.currentInstruction) = RN(arm.currentInstruction) ^ RM(arm.currentInstruction);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//eor immediate with no shift
void ARM_eori() {
	RD(arm.currentInstruction) = RN(arm.currentInstruction) ^ IMM(arm.currentInstruction);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//eor, no shift, S bit set
void ARM_eors() {
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = RM(arm.currentInstruction);
	register u32 tempC = RD(arm.currentInstruction) = tempA ^ tempB;
	ALU_setNZFlags(tempC);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
	
}

//eor immediate, no shift, S bit set
void ARM_eorsi() {
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = IMM(arm.currentInstruction);
	register u32 tempC = RD(arm.currentInstruction) = tempA ^ tempB;
	ALU_setNZFlags(tempC);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
}

//eor with a reg shift
void ARM_eorRegShift() {
	RD(arm.currentInstruction) = RN(arm.currentInstruction) ^ REGSHIFT(arm.currentInstruction);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//eor with an immediate shift
void ARM_eorImmShift() {
	RD(arm.currentInstruction) = RN(arm.currentInstruction) ^ IMMSHIFT(arm.currentInstruction);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}


//eori with shift
void ARM_eoriShift() {
	register u32 tempC = ((arm.currentInstruction & 0xF00 ) >> 7);	//Get the shift amount
	register u32 op2 = IMM(arm.currentInstruction);
	RD(arm.currentInstruction) = RN(arm.currentInstruction) ^ IMMROT(op2, tempC);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//eors with a reg shift
void ARM_eorsRegShift() {
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = REGSHIFTS(arm.currentInstruction);
	register u32 tempC = RD(arm.currentInstruction) = tempA ^ tempB;
	ALU_setNZFlags(tempC);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
}

//eors with an immediate shift 
void ARM_eorsImmShift() {
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = IMMSHIFTS(arm.currentInstruction);
	register u32 tempC = RD(arm.currentInstruction) = tempA ^ tempB;
	ALU_setNZFlags(tempC);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
}

//eors immediate with shift
void ARM_eorsiShift() {
	register u32 shift = ((arm.currentInstruction & 0xF00 ) >> 7);	//Get the shift
	register u32 op2 = IMM(arm.currentInstruction);
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = IMMROT(op2, shift);
	register u32 tempC = RD(arm.currentInstruction) = tempA ^ tempB;
	ALU_setNZFlags(tempC);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
}

////////////////////////////////////////////////////////
//sub() functions
///////////////////////////////////////////////////////

//sub with no shift
void ARM_sub() {
	RD(arm.currentInstruction) = RN(arm.currentInstruction) - RM(arm.currentInstruction);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//sub immediate with no shift
void ARM_subi() {
	RD(arm.currentInstruction) = RN(arm.currentInstruction) - IMM(arm.currentInstruction);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//sub, no shift, S bit set
void ARM_subs() {
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = RM(arm.currentInstruction);
	register u32 tempC = RD(arm.currentInstruction) = tempA - tempB;
	ALU_SETSUBFLAGS(tempA,tempB,tempC)
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
}

//sub immediate, no shift, S bit set
void ARM_subsi() {
	//register u32 oldMode, newMode;
	//register u32 CPSR;
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = IMM(arm.currentInstruction);
	register u32 tempC = RD(arm.currentInstruction) = tempA - tempB;
	ALU_SETSUBFLAGS(tempA,tempB,tempC)

	

	
	//This instruction has some additional overhead in that it is supposed
	//to restore CPU state if entered from an exception handler
	if ( (SUPPORT_GETRD(arm.currentInstruction) == 15)) {	//If the instruction is subs pc, lr #4
		//If it's a mode change
		if ( (SUPPORT_GETRN(arm.currentInstruction) == 14) && (tempB == 0x4) ) {
	
			//DEBUG
			//if (temp3 != 0x3C)
			//	int blah = 0;

			//DEBUG
			//temp3 = arm.r[14];	//Save the LR before we blow it away, below.

			//CPSR = *arm.SPSR_current;	//Get the saved program state register
		/*	oldMode = arm.CPSR & 0x1F;
			newMode = *arm.SPSR_current & 0x1F;
			ARM_changeCPSRState(oldMode, newMode);	//Swap registers.
			arm.CPSR = *arm.SPSR_current;		//Get the CPSR back.
			
			ARM_N = arm.CPSR >> 31;
			ARM_Z = (arm.CPSR >> 30) & 0x1;
			arm.C = (arm.CPSR >> 29) & 0x1;
			ARM_V = (arm.CPSR >> 28) & 0x1;
		
			ARM_swapSPSR();
			//Check to see if we're in thumb mode.
			//I am pretty sure it does it by the T bit of the CPSR.
			if (arm.CPSR & 0x00000020) {
				ARM_thumbMode = 1;
				ARM_skipInstructionThumb();
			} else {
				ARM_thumbMode = 0;
				ARM_skipInstruction();
			}
			*/
			//Get the last four bits of CPSR
			u32 oldMode = arm.CPSR & 0xF;
			//Why did I comment the part about not doing this if in user or system mode?
			//I am putting it back in, until I remember a good reason for doing this.
			if ( (oldMode != 0x0) && (oldMode != 0xF) )	//If not user or system mode . . 
				ARM_swapSPSR();
			//fprintf(GBA_logFile, "\nB Exited interrupt. lcy: %X, r15: %X, IE: %X, IF: %X", graphics.lcy, arm.r[15], *(REG_IE), *(REG_IF));
			//fflush(GBA_logFile);

			//Debugging
		//	if (arm.r[0xF] & 0x1)
		//		int blah = 0;
		
		//Otherwise flush the queue since r15 changed, but no mode change
		} else {
			ARM_skipInstruction();
		}

	//Or just get the next instruction
	} else {
		ARM_nextInstruction();
	}
			//More debugging
		//	if (arm.r[0xF] & 0x1)
		//		int blah = 0;
	//} else 
	//	ARM_nextInstruction(); //if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}

	//More debugging
//	if (arm.r[0xF] & 0x1)
//		int blah = 0;
}



//sub with a reg shift
void ARM_subRegShift() {
	RD(arm.currentInstruction) = RN(arm.currentInstruction) - REGSHIFT(arm.currentInstruction);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//sub with an immediate shift
void ARM_subImmShift() {
	RD(arm.currentInstruction) = RN(arm.currentInstruction) - IMMSHIFT(arm.currentInstruction);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}


//subi with shift
void ARM_subiShift() {
	register u32 tempC = ((arm.currentInstruction & 0xF00 ) >> 7);	//Get the shift amount
	register u32 op2 = IMM(arm.currentInstruction);
	RD(arm.currentInstruction) = RN(arm.currentInstruction) - IMMROT(op2, tempC);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//subs with a reg shift
void ARM_subsRegShift() {
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = REGSHIFTS(arm.currentInstruction);
	register u32 tempC = RD(arm.currentInstruction) = tempA - tempB;
	ALU_SETSUBFLAGS(tempA,tempB,tempC)
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
}

//subs with an immediate shift 
void ARM_subsImmShift() {
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = IMMSHIFTS(arm.currentInstruction);
	register u32 tempC = RD(arm.currentInstruction) = tempA - tempB;
	ALU_SETSUBFLAGS(tempA,tempB,tempC)
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
}

//subs immediate with shift
void ARM_subsiShift() {
	register u32 shift = ((arm.currentInstruction & 0xF00 ) >> 7);	//Get the shift
	register u32 op2 = IMM(arm.currentInstruction);
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = IMMROT(op2, shift);
	register u32 tempC = RD(arm.currentInstruction) = tempA - tempB;
	ALU_SETSUBFLAGS(tempA,tempB,tempC)
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
}

///////////////////////////////////////////////////////
//rsb() functions
///////////////////////////////////////////////////////
//rsb with no shift
void ARM_rsb() {
	RD(arm.currentInstruction) = RM(arm.currentInstruction) - RN(arm.currentInstruction);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//rsb immediate with no shift
void ARM_rsbi() {
	RD(arm.currentInstruction) = IMM(arm.currentInstruction) - RN(arm.currentInstruction);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//rsb, no shift, S bit set
void ARM_rsbs() {
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = RM(arm.currentInstruction);
	register u32 tempC = RD(arm.currentInstruction) = tempB - tempA;
	ALU_SETSUBFLAGS(tempB,tempA,tempC)
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
	
}

//rsb immediate, no shift, S bit set
void ARM_rsbsi() {
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = IMM(arm.currentInstruction);
	register u32 tempC = RD(arm.currentInstruction) = tempB - tempA;
	ALU_SETSUBFLAGS(tempB,tempA,tempC)
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
}

//rsb with a reg shift
void ARM_rsbRegShift() {
	RD(arm.currentInstruction) = REGSHIFT(arm.currentInstruction)  - RN(arm.currentInstruction);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//rsb with an immediate shift
void ARM_rsbImmShift() {
	RD(arm.currentInstruction) = IMMSHIFT(arm.currentInstruction)  - RN(arm.currentInstruction);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}


//rsbi with shift
void ARM_rsbiShift() {
	register u32 tempC = ((arm.currentInstruction & 0xF00 ) >> 7);	//Get the shift amount
	register u32 op2 = IMM(arm.currentInstruction);
	RD(arm.currentInstruction) = IMMROT(op2, tempC) - RN(arm.currentInstruction);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//rsbs with a reg shift
void ARM_rsbsRegShift() {
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = REGSHIFTS(arm.currentInstruction);
	register u32 tempC = RD(arm.currentInstruction) = tempB - tempA;
	ALU_SETSUBFLAGS(tempB,tempA,tempC)
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
}

//rsbs with an immediate shift 
void ARM_rsbsImmShift() {
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = IMMSHIFTS(arm.currentInstruction);
	register u32 tempC = RD(arm.currentInstruction) = tempB - tempA;
	ALU_SETSUBFLAGS(tempB,tempA,tempC)
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
}

//rsbs immediate with shift
void ARM_rsbsiShift() {
	register u32 shift = ((arm.currentInstruction & 0xF00 ) >> 7);	//Get the shift
	register u32 op2 = IMM(arm.currentInstruction);
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = IMMROT(op2, shift);
	register u32 tempC = RD(arm.currentInstruction) = tempB - tempA;
	ALU_SETSUBFLAGS(tempB,tempA,tempC)
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
}


///////////////////////////////////////////////////////
//add() functions
///////////////////////////////////////////////////////
//Add with no shift
void ARM_add() {
	RD(arm.currentInstruction) = RN(arm.currentInstruction) + RM(arm.currentInstruction);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//Add immediate with no shift
void ARM_addi() {
	RD(arm.currentInstruction) = RN(arm.currentInstruction) + IMM(arm.currentInstruction);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//Add, no shift, S bit set
void ARM_adds() {
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = RM(arm.currentInstruction);
	register u32 tempC = RD(arm.currentInstruction) = tempA + tempB;
	ALU_SETADDFLAGS(tempA,tempB,tempC)
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
	
}

//Add immediate, no shift, S bit set
void ARM_addsi() {
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = IMM(arm.currentInstruction);
	register u32 tempC = RD(arm.currentInstruction) = tempA + tempB;
	ALU_SETADDFLAGS(tempA,tempB,tempC)
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
}

//Add with a reg shift
void ARM_addRegShift() {
	RD(arm.currentInstruction) = RN(arm.currentInstruction) + REGSHIFT(arm.currentInstruction);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//Add with an immediate shift
void ARM_addImmShift() {
	RD(arm.currentInstruction) = RN(arm.currentInstruction) + IMMSHIFT(arm.currentInstruction);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}


//Addi with shift
void ARM_addiShift() {
	register u32 tempC = ((arm.currentInstruction & 0xF00 ) >> 7);	//Get the shift amount
	register u32 op2 = IMM(arm.currentInstruction);
	RD(arm.currentInstruction) = RN(arm.currentInstruction) + IMMROT(op2, tempC);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//adds with a reg shift
void ARM_addsRegShift() {
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = REGSHIFTS(arm.currentInstruction);
	register u32 tempC = RD(arm.currentInstruction) = tempA + tempB;
	ALU_SETADDFLAGS(tempA,tempB,tempC)
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
}

//Adds with an immediate shift 
void ARM_addsImmShift() {
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = IMMSHIFTS(arm.currentInstruction);
	register u32 tempC = RD(arm.currentInstruction) = tempA + tempB;
	ALU_SETADDFLAGS(tempA,tempB,tempC)
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
}

//Adds immediate with shift
void ARM_addsiShift() {
	register u32 shift = ((arm.currentInstruction & 0xF00 ) >> 7);	//Get the shift
	register u32 op2 = IMM(arm.currentInstruction);
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = IMMROT(op2, shift);
	register u32 tempC = RD(arm.currentInstruction) = tempA + tempB;
	ALU_SETADDFLAGS(tempA,tempB,tempC)
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
}

///////////////////////////////////////////////////////
//adc() functions
///////////////////////////////////////////////////////
//Adc with no shift
void ARM_adc() {
	RD(arm.currentInstruction) = RN(arm.currentInstruction) + RM(arm.currentInstruction) + arm.C;
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//Adc immediate with no shift
void ARM_adci() {
	RD(arm.currentInstruction) = RN(arm.currentInstruction) + IMM(arm.currentInstruction) + arm.C;
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//Adc, no shift, S bit set
void ARM_adcs() {
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = RM(arm.currentInstruction);
	register u32 tempC = RD(arm.currentInstruction) = tempA + tempB + arm.C;
	ALU_SETADDFLAGS(tempA,tempB,tempC)
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
	
}

//Adc immediate, no shift, S bit set
void ARM_adcsi() {
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = IMM(arm.currentInstruction);
	register u32 tempC = RD(arm.currentInstruction) = tempA + tempB + arm.C;
	ALU_SETADDFLAGS(tempA,tempB,tempC)
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
}

//Adc with a reg shift
void ARM_adcRegShift() {
	RD(arm.currentInstruction) = RN(arm.currentInstruction) + REGSHIFT(arm.currentInstruction) + arm.C;
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//Adc with an immediate shift
void ARM_adcImmShift() {
	RD(arm.currentInstruction) = RN(arm.currentInstruction) + IMMSHIFT(arm.currentInstruction) + arm.C;
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}


//Adci with shift
void ARM_adciShift() {
	register u32 tempC = ((arm.currentInstruction & 0xF00 ) >> 7);	//Get the shift amount
	register u32 op2 = IMM(arm.currentInstruction);
	RD(arm.currentInstruction) = RN(arm.currentInstruction) + IMMROT(op2, tempC) + arm.C;
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//adcs with a reg shift
void ARM_adcsRegShift() {
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = REGSHIFTS(arm.currentInstruction);
	register u32 tempC = RD(arm.currentInstruction) = tempA + tempB + arm.C;
	ALU_SETADDFLAGS(tempA,tempB,tempC)
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
}

//Adcs with an immediate shift 
void ARM_adcsImmShift() {
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = IMMSHIFTS(arm.currentInstruction);
	register u32 tempC = RD(arm.currentInstruction) = tempA + tempB + arm.C;
	ALU_SETADDFLAGS(tempA,tempB,tempC)
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
}

//Adcs immediate with shift
void ARM_adcsiShift() {
	register u32 shift = ((arm.currentInstruction & 0xF00 ) >> 7);	//Get the shift
	register u32 op2 = IMM(arm.currentInstruction);
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = IMMROT(op2, shift);
	register u32 tempC = RD(arm.currentInstruction) = tempA + tempB + arm.C;
	ALU_SETADDFLAGS(tempA,tempB,tempC)
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
}


///////////////////////////////////////////////////////
//sbc() functions
///////////////////////////////////////////////////////
//Subc with no shift
void ARM_sbc() {
	RD(arm.currentInstruction) = RN(arm.currentInstruction) - RM(arm.currentInstruction) - !arm.C;
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//Subc immediate with no shift
void ARM_sbci() {
	RD(arm.currentInstruction) = RN(arm.currentInstruction) - IMM(arm.currentInstruction) - !arm.C;
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//Subc, no shift, S bit set
void ARM_sbcs() {
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = RM(arm.currentInstruction);
	register u32 tempC = RD(arm.currentInstruction) = tempA - tempB - !arm.C;
	ALU_SETSUBFLAGS(tempA,tempB,tempC)
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
	
}

//Subc immediate, no shift, S bit set
void ARM_sbcsi() {
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = IMM(arm.currentInstruction);
	register u32 tempC = RD(arm.currentInstruction) = tempA - tempB - !arm.C;
	ALU_SETSUBFLAGS(tempA,tempB,tempC)
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
}

//Subc with a reg shift
void ARM_sbcRegShift() {
	RD(arm.currentInstruction) = RN(arm.currentInstruction) - REGSHIFT(arm.currentInstruction) - !arm.C;
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//Subc with an immediate shift
void ARM_sbcImmShift() {
	RD(arm.currentInstruction) = RN(arm.currentInstruction) - IMMSHIFT(arm.currentInstruction) - !arm.C;
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}


//Subci with shift
void ARM_sbciShift() {
	register u32 tempC = ((arm.currentInstruction & 0xF00 ) >> 7);	//Get the shift amount
	register u32 op2 = IMM(arm.currentInstruction);
	RD(arm.currentInstruction) = RN(arm.currentInstruction) - IMMROT(op2, tempC) - !arm.C;
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//sbcs with a reg shift
void ARM_sbcsRegShift() {
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = REGSHIFTS(arm.currentInstruction);
	register u32 tempC = RD(arm.currentInstruction) = tempA - tempB - !arm.C;
	ALU_SETSUBFLAGS(tempA,tempB,tempC)
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
}

//Subcs with an immediate shift 
void ARM_sbcsImmShift() {
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = IMMSHIFTS(arm.currentInstruction);
	register u32 tempC = RD(arm.currentInstruction) = tempA - tempB - !arm.C;
	ALU_SETSUBFLAGS(tempA,tempB,tempC)
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
}

//Subcs immediate with shift
void ARM_sbcsiShift() {
	u32 shift = ((arm.currentInstruction & 0xF00 ) >> 7);	//Get the shift
	register u32 op2 = IMM(arm.currentInstruction);
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = IMMROT(op2, shift);
	register u32 tempC = RD(arm.currentInstruction) = tempA - tempB - !arm.C;
	ALU_SETSUBFLAGS(tempA,tempB,tempC)
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
}


///////////////////////////////////////////////////////
//rsc() functions
///////////////////////////////////////////////////////
//rsc with no shift
void ARM_rsc() {
	RD(arm.currentInstruction) = RM(arm.currentInstruction) - RN(arm.currentInstruction) - !arm.C;
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//rsc immediate with no shift
void ARM_rsci() {
	RD(arm.currentInstruction) = IMM(arm.currentInstruction) - RN(arm.currentInstruction) - !arm.C;
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//rsc, no shift, S bit set
void ARM_rscs() {
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = RM(arm.currentInstruction);
	register u32 tempC = RD(arm.currentInstruction) = tempB - tempA - !arm.C;
	ALU_SETSUBFLAGS(tempB,tempA,tempC)
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
	
}

//rsc immediate, no shift, S bit set
void ARM_rscsi() {
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = IMM(arm.currentInstruction);
	register u32 tempC = RD(arm.currentInstruction) = tempB - tempA - !arm.C;
	ALU_SETSUBFLAGS(tempB,tempA,tempC)
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
}

//rsc with a reg shift
void ARM_rscRegShift() {
	RD(arm.currentInstruction) = REGSHIFT(arm.currentInstruction)  - RN(arm.currentInstruction) - !arm.C;
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//rsc with an immediate shift
void ARM_rscImmShift() {
	RD(arm.currentInstruction) = IMMSHIFT(arm.currentInstruction)  - RN(arm.currentInstruction) - !arm.C;
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}


//rsci with shift
void ARM_rsciShift() {
	register u32 tempC = ((arm.currentInstruction & 0xF00 ) >> 7);	//Get the shift amount
	register u32 op2 = IMM(arm.currentInstruction);
	RD(arm.currentInstruction) = IMMROT(op2, tempC) - RN(arm.currentInstruction) - !arm.C;
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//rscs with a reg shift
void ARM_rscsRegShift() {
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = REGSHIFTS(arm.currentInstruction);
	register u32 tempC = RD(arm.currentInstruction) = tempB - tempA - !arm.C;
	ALU_SETSUBFLAGS(tempB,tempA,tempC)
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
}

//rscs with an immediate shift 
void ARM_rscsImmShift() {
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = IMMSHIFTS(arm.currentInstruction);
	register u32 tempC = RD(arm.currentInstruction) = tempB - tempA - !arm.C;
	ALU_SETSUBFLAGS(tempB,tempA,tempC)
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
}

//rscs immediate with shift
void ARM_rscsiShift() {
	register u32 shift = ((arm.currentInstruction & 0xF00 ) >> 7);	//Get the shift
	register u32 op2 = IMM(arm.currentInstruction);
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = IMMROT(op2, shift);
	register u32 tempC = RD(arm.currentInstruction) = tempB - tempA - !arm.C;
	ALU_SETSUBFLAGS(tempB,tempA,tempC)
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
}

///////////////////////////////////////////////////////
//tsts() functions
//Note that there are no "tst" functions -- the s
//is always set for these; in the case with s bit not set,
//we call msr or mrs
///////////////////////////////////////////////////////
void ARM_tsts() {
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = RM(arm.currentInstruction);
	register u32 tempC = tempA & tempB;
	ALU_setNZFlags(tempC);
	ARM_nextInstruction();
	
}

//tst immediate, no shift, S bit set
void ARM_tstsi() {
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = IMM(arm.currentInstruction);
	register u32 tempC = tempA & tempB;
	ALU_setNZFlags(tempC);
	ARM_nextInstruction();
}

//tsts with a reg shift
void ARM_tstsRegShift() {
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = REGSHIFTS(arm.currentInstruction);
	register u32 tempC = tempA & tempB;
	ALU_setNZFlags(tempC);
	ARM_nextInstruction();
}

//tsts with an immediate shift 
void ARM_tstsImmShift() {
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = IMMSHIFTS(arm.currentInstruction);
	register u32 tempC = tempA & tempB;
	ALU_setNZFlags(tempC);
	ARM_nextInstruction();
}

//tsts immediate with shift
void ARM_tstsiShift() {
	register u32 shift = ((arm.currentInstruction & 0xF00 ) >> 7);	//Get the shift
	register u32 op2 = IMM(arm.currentInstruction);
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = IMMROT(op2, shift);
	register u32 tempC = tempA & tempB;
	ALU_setNZFlags(tempC);
	ARM_nextInstruction();
}

///////////////////////////////////////////////////////
//teqs() functions
//Note that there are no "teq" functions -- the s
//is always set for these; in the case with s bit not set,
//we call msr or mrs
///////////////////////////////////////////////////////
void ARM_teqs() {
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = RM(arm.currentInstruction);
	register u32 tempC = tempA ^ tempB;
	ALU_setNZFlags(tempC);
	ARM_nextInstruction();
}

//teq immediate, no shift, S bit set
void ARM_teqsi() {
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = IMM(arm.currentInstruction);
	register u32 tempC = tempA ^ tempB;
	ALU_setNZFlags(tempC);
	ARM_nextInstruction();
}

//teqs with a reg shift
void ARM_teqsRegShift() {
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = REGSHIFTS(arm.currentInstruction);
	register u32 tempC = tempA ^ tempB;
	ALU_setNZFlags(tempC);
	ARM_nextInstruction();
}

//teqs with an immediate shift 
void ARM_teqsImmShift() {
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = IMMSHIFTS(arm.currentInstruction);
	register u32 tempC = tempA ^ tempB;
	ALU_setNZFlags(tempC);
	ARM_nextInstruction();
}

//teqs immediate with shift
void ARM_teqsiShift() {
	u32 shift = ((arm.currentInstruction & 0xF00 ) >> 7);	//Get the shift
	register u32 op2 = IMM(arm.currentInstruction);
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = IMMROT(op2, shift);
	register u32 tempC = tempA ^ tempB;
	ALU_setNZFlags(tempC);
	ARM_nextInstruction();
}


///////////////////////////////////////////////////////
//cmps() functions
//Note that there are no "cmp" functions -- the s
//is always set for these; in the case with s bit not set,
//we call msr or mrs
///////////////////////////////////////////////////////
void ARM_cmps() {
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = RM(arm.currentInstruction);
	register u32 tempC = tempA - tempB;
	ALU_SETSUBFLAGS(tempA,tempB,tempC);
	ARM_nextInstruction();
	
}

//cmp immediate, no shift, S bit set
void ARM_cmpsi() {
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = IMM(arm.currentInstruction);
	register u32 tempC = tempA - tempB;
	ALU_SETSUBFLAGS(tempA,tempB,tempC);
	ARM_nextInstruction();
}

//cmps with a reg shift
void ARM_cmpsRegShift() {
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = REGSHIFTS(arm.currentInstruction);
	register u32 tempC = tempA - tempB;
	ALU_SETSUBFLAGS(tempA,tempB,tempC);
	ARM_nextInstruction();
}

//cmps with an immediate shift 
void ARM_cmpsImmShift() {
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = IMMSHIFTS(arm.currentInstruction);
	register u32 tempC = tempA - tempB;
	ALU_SETSUBFLAGS(tempA,tempB,tempC);
	ARM_nextInstruction();
}

//cmps immediate with shift
void ARM_cmpsiShift() {
	register u32 shift = ((arm.currentInstruction & 0xF00 ) >> 7);	//Get the shift
	register u32 op2 = IMM(arm.currentInstruction);
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = IMMROT(op2, shift);
	register u32 tempC = tempA - tempB;
	ALU_SETSUBFLAGS(tempA,tempB,tempC);
	ARM_nextInstruction();
}

///////////////////////////////////////////////////////
//cmns() functions
//Note that there are no "cmn" functions ++ the s
//is always set for these; in the case with s bit not set,
//we call msr or mrs
///////////////////////////////////////////////////////
void ARM_cmns() {
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = RM(arm.currentInstruction);
	register u32 tempC = tempA + tempB;
	ALU_SETADDFLAGS(tempA,tempB,tempC);
	ARM_nextInstruction();
	
}

//cmn immediate, no shift, S bit set
void ARM_cmnsi() {
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = IMM(arm.currentInstruction);
	register u32 tempC = tempA + tempB;
	ALU_SETADDFLAGS(tempA,tempB,tempC);
	ARM_nextInstruction();
}

//cmns with a reg shift
void ARM_cmnsRegShift() {
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = REGSHIFTS(arm.currentInstruction);
	register u32 tempC = tempA + tempB;
	ALU_SETADDFLAGS(tempA,tempB,tempC);
	ARM_nextInstruction();
}

//cmns with an immediate shift 
void ARM_cmnsImmShift() {
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = IMMSHIFTS(arm.currentInstruction);
	register u32 tempC = tempA + tempB;
	ALU_SETADDFLAGS(tempA,tempB,tempC);
	ARM_nextInstruction();
}

//cmns immediate with shift
void ARM_cmnsiShift() {
	u32 shift = ((arm.currentInstruction & 0xF00 ) >> 7);	//Get the shift
	register u32 op2 = IMM(arm.currentInstruction);
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = IMMROT(op2, shift);
	register u32 tempC = tempA + tempB;
	ALU_SETADDFLAGS(tempA,tempB,tempC);
	ARM_nextInstruction();
}

///////////////////////////////////////////////////////
//orr() functions
///////////////////////////////////////////////////////
//orr with no shift
void ARM_orr() {
	RD(arm.currentInstruction) = RN(arm.currentInstruction) | RM(arm.currentInstruction);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//orr immediate with no shift
void ARM_orri() {
	RD(arm.currentInstruction) = RN(arm.currentInstruction) | IMM(arm.currentInstruction);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//orr, no shift, S bit set
void ARM_orrs() {
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = RM(arm.currentInstruction);
	register u32 tempC = RD(arm.currentInstruction) = tempA | tempB;
	ALU_setNZFlags(tempC);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
	
}

//orr immediate, no shift, S bit set
void ARM_orrsi() {
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = IMM(arm.currentInstruction);
	register u32 tempC = RD(arm.currentInstruction) = tempA | tempB;
	ALU_setNZFlags(tempC);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
}

//orr with a reg shift
void ARM_orrRegShift() {
	RD(arm.currentInstruction) = RN(arm.currentInstruction) | REGSHIFT(arm.currentInstruction);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//orr with an immediate shift
void ARM_orrImmShift() {
	RD(arm.currentInstruction) = RN(arm.currentInstruction) | IMMSHIFT(arm.currentInstruction);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}


//orri with shift
void ARM_orriShift() {
	register u32 tempC = ((arm.currentInstruction & 0xF00 ) >> 7);	//Get the shift amount
	register u32 op2 = IMM(arm.currentInstruction);
	RD(arm.currentInstruction) = RN(arm.currentInstruction) | IMMROT(op2, tempC);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//orrs with a reg shift
void ARM_orrsRegShift() {
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = REGSHIFTS(arm.currentInstruction);
	register u32 tempC = RD(arm.currentInstruction) = tempA | tempB;
	ALU_setNZFlags(tempC);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
}

//orrs with an immediate shift 
void ARM_orrsImmShift() {
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = IMMSHIFTS(arm.currentInstruction);
	register u32 tempC = RD(arm.currentInstruction) = tempA | tempB;
	ALU_setNZFlags(tempC);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
}

//orrs immediate with shift
void ARM_orrsiShift() {
	register u32 shift = ((arm.currentInstruction & 0xF00 ) >> 7);	//Get the shift
	register u32 op2 = IMM(arm.currentInstruction);
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = IMMROT(op2, shift);
	register u32 tempC = RD(arm.currentInstruction) = tempA | tempB;
	ALU_setNZFlags(tempC);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
}

///////////////////////////////////////////////////////
//mov() functions
///////////////////////////////////////////////////////
//mov with no shift
void ARM_mov() {
	RD(arm.currentInstruction) = RM(arm.currentInstruction);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//mov immediate with no shift
void ARM_movi() {
	RD(arm.currentInstruction) = IMM(arm.currentInstruction);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//mov, no shift, S bit set
void ARM_movs() {
	register u32 tempB = RM(arm.currentInstruction);
	RD(arm.currentInstruction) = tempB;
	ALU_setNZFlags(tempB);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
	
}

//mov immediate, no shift, S bit set
void ARM_movsi() {
	register u32 tempB = IMM(arm.currentInstruction);
	RD(arm.currentInstruction) = tempB;
	ALU_setNZFlags(tempB);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
}

//mov with a reg shift
void ARM_movRegShift() {
	RD(arm.currentInstruction) = REGSHIFT(arm.currentInstruction);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//mov with an immediate shift
void ARM_movImmShift() {
	RD(arm.currentInstruction) = IMMSHIFT(arm.currentInstruction);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}


//movi with shift
void ARM_moviShift() {
	register u32 tempC = ((arm.currentInstruction & 0xF00 ) >> 7);	//Get the shift amount
	register u32 op2 = IMM(arm.currentInstruction);
	RD(arm.currentInstruction) = IMMROT(op2, tempC);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//movs with a reg shift
void ARM_movsRegShift() {
	register u32 tempB = REGSHIFTS(arm.currentInstruction);
	RD(arm.currentInstruction) = tempB;
	ALU_setNZFlags(tempB);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
}

//movs with an immediate shift 
void ARM_movsImmShift() {
	register u32 tempB = IMMSHIFTS(arm.currentInstruction);
	RD(arm.currentInstruction) = tempB;
	ALU_setNZFlags(tempB);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
}

//movs immediate with shift
void ARM_movsiShift() {
	register u32 shift = ((arm.currentInstruction & 0xF00 ) >> 7);	//Get the shift
	register u32 op2 = IMM(arm.currentInstruction);
	register u32 tempB = IMMROT(op2, shift);
	RD(arm.currentInstruction) = tempB;
	ALU_setNZFlags(tempB);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
}

///////////////////////////////////////////////////////
//bic() functions
///////////////////////////////////////////////////////
//bic with no shift
void ARM_bic() {
	RD(arm.currentInstruction) = RN(arm.currentInstruction) & (~RM(arm.currentInstruction));
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//bic immediate with no shift
void ARM_bici() {
	RD(arm.currentInstruction) = RN(arm.currentInstruction) & (~IMM(arm.currentInstruction));
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//bic, no shift, S bit set
void ARM_bics() {
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = RM(arm.currentInstruction);
	register u32 tempC = RD(arm.currentInstruction) = tempA & (~tempB);
	ALU_setNZFlags(tempC);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
	
}

//bic immediate, no shift, S bit set
void ARM_bicsi() {
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = IMM(arm.currentInstruction);
	register u32 tempC = RD(arm.currentInstruction) = tempA & (~tempB);
	ALU_setNZFlags(tempC);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
}

//bic with a reg shift
void ARM_bicRegShift() {
	RD(arm.currentInstruction) = RN(arm.currentInstruction) & (~REGSHIFT(arm.currentInstruction));
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//bic with an immediate shift
void ARM_bicImmShift() {
	RD(arm.currentInstruction) = RN(arm.currentInstruction) & (~IMMSHIFT(arm.currentInstruction));
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}


//bici with shift
void ARM_biciShift() {
	register u32 tempC = ((arm.currentInstruction & 0xF00 ) >> 7);	//Get the shift amount
	register u32 op2 = IMM(arm.currentInstruction);
	RD(arm.currentInstruction) = RN(arm.currentInstruction) & (~IMMROT(op2, tempC));
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//bics with a reg shift
void ARM_bicsRegShift() {
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = REGSHIFTS(arm.currentInstruction);
	register u32 tempC = RD(arm.currentInstruction) = tempA & (~tempB);
	ALU_setNZFlags(tempC);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
}

//bics with an immediate shift 
void ARM_bicsImmShift() {
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = IMMSHIFTS(arm.currentInstruction);
	register u32 tempC = RD(arm.currentInstruction) = tempA & (~tempB);
	ALU_setNZFlags(tempC);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
}

//bics immediate with shift
void ARM_bicsiShift() {
	register u32 shift = ((arm.currentInstruction & 0xF00 ) >> 7);	//Get the shift
	register u32 op2 = IMM(arm.currentInstruction);
	register u32 tempA = RN(arm.currentInstruction);
	register u32 tempB = IMMROT(op2, shift);
	register u32 tempC = RD(arm.currentInstruction) = tempA & (~tempB);
	ALU_setNZFlags(tempC);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
}

///////////////////////////////////////////////////////
//mvn() functions
///////////////////////////////////////////////////////
//mvn with no shift
void ARM_mvn() {
	RD(arm.currentInstruction) = ~RM(arm.currentInstruction);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//mvn immediate with no shift
void ARM_mvni() {
	RD(arm.currentInstruction) = ~IMM(arm.currentInstruction);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//mvn, no shift, S bit set
void ARM_mvns() {
	register u32 tempB = RM(arm.currentInstruction);
	RD(arm.currentInstruction) = ~tempB;
	ALU_setNZFlags(tempB);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
	
}

//mvn immediate, no shift, S bit set
void ARM_mvnsi() {
	register u32 tempB = IMM(arm.currentInstruction);
	RD(arm.currentInstruction) = ~tempB;
	ALU_setNZFlags(tempB);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
}

//mvn with a reg shift
void ARM_mvnRegShift() {
	RD(arm.currentInstruction) = ~REGSHIFT(arm.currentInstruction);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//mvn with an immediate shift
void ARM_mvnImmShift() {
	RD(arm.currentInstruction) = ~IMMSHIFT(arm.currentInstruction);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}


//mvni with shift
void ARM_mvniShift() {
	register u32 tempC = ((arm.currentInstruction & 0xF00 ) >> 7);	//Get the shift amount
	register u32 op2 = ~IMM(arm.currentInstruction);
	RD(arm.currentInstruction) = IMMROT(op2, tempC);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//mvns with a reg shift
void ARM_mvnsRegShift() {
	register u32 tempB = REGSHIFTS(arm.currentInstruction);
	RD(arm.currentInstruction) = ~tempB;
	ALU_setNZFlags(tempB);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
}

//mvns with an immediate shift 
void ARM_mvnsImmShift() {
	register u32 tempB = IMMSHIFTS(arm.currentInstruction);
	RD(arm.currentInstruction) = ~tempB;
	ALU_setNZFlags(tempB);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
}

//mvns immediate with shift
void ARM_mvnsiShift() {
	register u32 shift = ((arm.currentInstruction & 0xF00 ) >> 7);	//Get the shift
	register u32 op2 = IMM(arm.currentInstruction);
	register u32 tempB = IMMROT(op2, shift);
	RD(arm.currentInstruction) = ~tempB;
	ALU_setNZFlags(tempB);
	if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_swapSPSR(); } else {ARM_nextInstruction();}
}


///////////////////////////////////////////////////////
//MSR/MRS() functions
//Not as optimized as they could be so that I have less of them
//to manage (would have to remove control statement for SPSR,
//then create more functions and map them to CMP, CMN...)
///////////////////////////////////////////////////////
//msr with no shift
//Copies flag, control, or all bits from the value in register rm to the
//CPSR or the SPSR
void ARM_msr() {
	u32* dest;
	u32 temp;
	u32 value, oldMode, newMode;
	if (SUPPORT_GETBIT22(arm.currentInstruction))	{//If it's an SPSR transfer
		//SPSR transfers are not allowed (I think) for user and system mode
		if ( ((arm.CPSR & 0xF) == 0x0) || ((arm.CPSR & 0xF) == 0xF) ) {
			ARM_nextInstruction();
			return;
		}

		dest = arm.SPSR_current;
	//If it's a transfer to the current CPSR, we want to perform the mode change first,
	//THEN update the contents (because otherwise the mode change will erroneously copy
	//the *new* CPSR to the SPSR.
	} else {
		//dest = &arm.CPSR;
		temp = arm.CPSR;	//copy over to temp
		dest = &temp;	//Store to temporary variable
		oldMode = arm.CPSR & 0x1F;
	}
	value = RM(arm.currentInstruction);
	switch ( (arm.currentInstruction >> 16) & 0xF) {
	
		
		case 0x8:	*dest = (*dest & 0x0FFFFFFF) | (value & 0xF0000000); //Set the flag bits only
			break;
		case 0x9:	*dest = value;	//Set all of the bits
			break;
		case 0x1:	*dest = (*dest & 0xF0000000) | (value & 0x0FFFFFFF);	//Set the control bits only
			break;
	}

	if (!SUPPORT_GETBIT22(arm.currentInstruction)) {	//Now copy back to our variables...
		if ((arm.CPSR & 0xF) == 0x0) {	//If it's user mode, the control bits are protected.
										//This code sets TEMP to be how it should be.
			temp = (temp & 0xF0000000) | (arm.CPSR & 0x0FFFFFFFF);

		} else {
			newMode = temp & 0x1F;	//Otherwise we can change mode.
			ARM_changeCPSRState(oldMode, newMode);
		}
		arm.CPSR = temp;	//Set any remaining bits
		arm.N = arm.CPSR >> 31;	//Yuck.
		arm.Z = (arm.CPSR >> 30) & 0x1;
		arm.C = (arm.CPSR >> 29) & 0x1;
		arm.V = (arm.CPSR >> 28 ) & 0x1;
		/*
		arm.N = arm.CPSR >> 31;
		arm.Z = (arm.CPSR >> 30) & 0x1;
		arm.C = (arm.CPSR >> 29) & 0x1;
		arm.V = (arm.CPSR >> 28 ) & 0x1;
		newMode = arm.CPSR & 0x1F;
		ARM_changeCPSRState(oldMode, newMode);*/
	}

	ARM_nextInstruction(); //if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}

//msr immediate with no shift
//Copies flag, control, or all bits from an immediate to the
//CPSR or the SPSR
void ARM_msri() {
	u32* dest;
	u32 temp;
	u32 value, oldMode, newMode;
	if (SUPPORT_GETBIT22(arm.currentInstruction)) {	//If it's an SPSR transfer
		//SPSR transfers are not allowed (I think) for user and system mode
		if ( ((arm.CPSR & 0xF) == 0x0) || ((arm.CPSR & 0xF) == 0xF) ) {
			ARM_nextInstruction();
			return;
		}
		dest = arm.SPSR_current;
	} else {
		temp = arm.CPSR;
		dest = &temp;
		//dest = &arm.CPSR;
		oldMode = arm.CPSR & 0x1F;
	}
	value = IMM(arm.currentInstruction);
	
	switch ( (arm.currentInstruction >> 16) & 0xF) {
	
		
		case 0x8:	*dest = (*dest & 0x0FFFFFFF) | (value & 0xF0000000); //Set the flag bits only
			break;
		case 0x9:	*dest = value;	//Set all of the bits
			break;
		case 0x1:	*dest = (*dest & 0xF0000000) | (value & 0x0FFFFFFF);	//Set the control bits only
			break;
	}

	if (!SUPPORT_GETBIT22(arm.currentInstruction)) {	//Now copy back to our variables...
		if ((arm.CPSR & 0xF) == 0x0) {	//If it's user mode, the control bits are protected.
										//This code sets TEMP to be how it should be.
			temp = (temp & 0xF0000000) | (arm.CPSR & 0x0FFFFFFFF);

		} else {
			newMode = temp & 0x1F;	//Otherwise we can change mode.
			ARM_changeCPSRState(oldMode, newMode);
		}
		arm.CPSR = temp;	//Set any remaining bits
		arm.N = arm.CPSR >> 31;
		arm.Z = (arm.CPSR >> 30) & 0x1;
		arm.C = (arm.CPSR >> 29) & 0x1;
		arm.V = (arm.CPSR >> 28 ) & 0x1;
		//newMode = arm.CPSR & 0x1F;
		//ARM_changeCPSRState(oldMode, newMode);
	}

	ARM_nextInstruction(); //if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}


//msri with shift
//Copies an immediate value onto the CPSR or SPSR, with a shift
void ARM_msriShift() {
	u32* dest;
	u32 temp;
	u32 oldMode, newMode;
	register u32 tempC = ((arm.currentInstruction & 0xF00 ) >> 7);	//Get the shift amount
	register u32 value = IMM(arm.currentInstruction);
	
	if (SUPPORT_GETBIT22(arm.currentInstruction)) {	//If it's an SPSR transfer
		//SPSR transfers are not allowed (I think) for user and system mode
		if ( ((arm.CPSR & 0xF) == 0x0) || ((arm.CPSR & 0xF) == 0xF) ) {
			ARM_nextInstruction();
			return;
		}


		
		dest = arm.SPSR_current;

	} else {
		temp = arm.CPSR;
		dest = &temp;
		//dest = &arm.CPSR;
		oldMode = arm.CPSR & 0x1F;
	}
	value = IMMROT(value, tempC);
	switch ( (arm.currentInstruction >> 16) & 0xF) {
	
		
		case 0x8:	*dest = (*dest & 0x0FFFFFFF) | (value & 0xF0000000); //Set the flag bits only
			break;
		case 0x9:	*dest = value;	//Set all of the bits
			break;
		case 0x1:	*dest = (*dest & 0xF0000000) | (value & 0x0FFFFFFF);	//Set the control bits only
			break;
	}

	//And if it was aa CPSR transfer, see if we need to do a mode change.
	if (!SUPPORT_GETBIT22(arm.currentInstruction)) {	//Now copy back to our variables...
		if ((arm.CPSR & 0xF) == 0x0) {	//If it's user mode, the control bits are protected.
										//This code sets TEMP to be how it should be.
			temp = (temp & 0xF0000000) | (arm.CPSR & 0x0FFFFFFFF);

		} else {
			newMode = temp & 0x1F;	//Otherwise we can change mode.
			ARM_changeCPSRState(oldMode, newMode);
		}
		arm.CPSR = temp;	//Set any remaining bits
		arm.N = arm.CPSR >> 31;
		arm.Z = (arm.CPSR >> 30) & 0x1;
		arm.C = (arm.CPSR >> 29) & 0x1;
		arm.V = (arm.CPSR >> 28 ) & 0x1;
		//newMode = arm.CPSR & 0x1F;
		//ARM_changeCPSRState(oldMode, newMode);

	}

	ARM_nextInstruction(); //if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();}
}


//mrs 
void ARM_mrs() {
	arm.CPSR = arm.CPSR& 0x0FFFFFFF;
	arm.CPSR = arm.CPSR | (arm.N << 31) | (arm.Z << 30) | (arm.C << 29) | (arm.V << 28); 

	if (SUPPORT_GETBIT22(arm.currentInstruction)) {	//If it's an SPSR transfer...
		RD(arm.currentInstruction) = *arm.SPSR_current;
	} else {
		RD(arm.currentInstruction) = arm.CPSR;
	}
	
	ARM_nextInstruction();
}

//This covearm.rs multiplies
//xxxx0000 00ASdddd nnnnssss 1001mmmm
void ARM_mul() {
	u32 result;

	//Get the ARM_opcode and the operands
	//arm.rn and arm.rd are backwaarm.rds for muls.
	arm.rd = SUPPORT_GETRN(arm.currentInstruction);//Support::getBits (arm.currentInstruction, 19, 16);
	arm.rs = SUPPORT_GETRS(arm.currentInstruction);//Support::getBits (arm.currentInstruction, 11, 8);
	arm.rm = SUPPORT_GETRM(arm.currentInstruction);//Support::getBits (arm.currentInstruction, 3, 0);

	//Find if the operation is a MUL or a MLA
	if ( SUPPORT_GETBIT21(arm.currentInstruction) ) {	//If it's MLA
		arm.rn = SUPPORT_GETRD(arm.currentInstruction);	//arm.rn and arm.rd are switched for muls
		result = arm.r[arm.rm] * arm.r[arm.rs] + arm.r[arm.rn];
	
	//If it's a regular MUL
	} else {
		result = arm.r[arm.rm] * arm.r[arm.rs];

	}
	
	if ( SUPPORT_GETBIT20(arm.currentInstruction) )	//Find if we're supposed to carry
		ALU_setNZFlags ( result);

	arm.r[arm.rd] = result;

	//Note how the pre-ARM_fetch occuarm.rs after evaluation of the operands --
	//otherwise, if one of the ops was r15, we'd have the wrong value
	ARM_nextInstruction(); //if (SUPPORT_GETRD(arm.currentInstruction) == 15) { ARM_skipInstruction();} else {ARM_nextInstruction();};
}

//Finally implemented.  May be a source of errors though....
void ARM_mull() {
	register u32 rl;
	register u32 rh;
	register u32 rs;
	register u32 rm;
	DWORDLONG uResult;
	DWORDLONG uAccum;
	LONGLONG sResult;
	LONGLONG sAccum;
	rl  = (arm.currentInstruction >> 12) & 0xF;
	rh  = (arm.currentInstruction >> 16) & 0xF;
	rs  = (arm.currentInstruction >> 8) & 0xF;
	rm  = arm.currentInstruction & 0xF;
	
	if (SUPPORT_GETBIT22(arm.currentInstruction)) {	//If it's signed...
		sResult = Int32x32To64(arm.r[rm], arm.r[rs]);
		if (SUPPORT_GETBIT21(arm.currentInstruction)) {	//If it's an accumulate...
			sAccum = arm.r[rl] | (arm.r[rh] << 32);		//get the current value
			sResult += sAccum;							//add it as signed
		}
		arm.r[rl] = sResult & 0xFFFFFFFF;
		arm.r[rh] = sResult >> 32;		
	} else {
		uResult = UInt32x32To64(arm.r[rm], arm.r[rs]);
		if (SUPPORT_GETBIT21(arm.currentInstruction)) {	//If it's an accumulate...
			uAccum = arm.r[rl] | (arm.r[rh] << 32);		//get the current value
			uResult += uAccum;							//add it as unsigned
		}
		arm.r[rl] = uResult & 0xFFFFFFFF;
		arm.r[rh] = uResult >> 32;	
	}
	sResult = 0xFFFFFFFFFFFFFFF1;
	uResult = (DWORDLONG)sResult;	//Get an absolute value
	if (SUPPORT_GETBIT20(arm.currentInstruction)) {	//If it the S bit is set
		arm.N = uResult >> 63;
		arm.Z = (uResult == 0);
	}
	ARM_nextInstruction();

}

