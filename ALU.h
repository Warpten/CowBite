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

#ifndef ALU_H
#define ALU_H

#include "ARM.h"
#include "Support.h"
#include "Constants.h"



#define SHIFT_LSL 0x0	//Logical shift left by constant
#define SHIFT_LSR 0x1	//Logical shift right
#define SHIFT_ASR 0x2	//Arithmetic shift right
#define SHIFT_ROR 0x3	//Rotate right

#define OP_AND 0x0		//Boolean AND
#define OP_EOR 0x1		//Boolean Eor
#define OP_SUB 0x2		//Subtract
#define OP_RSB 0x3		//Reverse Subtract
#define OP_ADD 0x4		//Addition
#define OP_ADC 0x5		//Addition with carry
#define OP_SBC 0x6		//Subtract with carry
#define OP_RSC 0x7		//Reverse sub with carry
#define OP_TST 0x8		//Test bit
#define OP_TEQ 0x9		//Test equality
#define OP_CMP 0xA		//Compare
#define OP_CMN 0xB		//Compare Negative
#define OP_ORR 0xC		//Boolean Orr
#define OP_MOV 0xD		//Move value
#define OP_BIC 0xE		//Bit clear
#define OP_MVN 0xF		//Move Not

/////////////////////////////////////////////////////
//ALU operations
u32 ALU_barrelShift (u32 value, u32 shiftAmt, u32 shiftType);
//u32 ALU_calculate (u32* destReg, u32 a, u32 b, u32 opcode, int S);
u32 ALU_calculate (int S);

void ALU_setAddFlags (u32 a, u32 b, u32 result);
void ALU_setSubFlags (u32 a, u32 b, u32 result);
void ALU_setNZFlags (register u32 value);
/////////////////////////////////////////////////////


/////////////////////////////////////////////////////////
//I have replicated some of my functions as macros...
/////////////////////////////////////////////////////


//This sets the flags of the arm.CPSR according to the value
__inline void ALU_setNZFlags (register u32 value) {
	if (value)  arm.Z = 0;
	else	arm.Z = 1;

	
	arm.N = value >> 31;	//Get the sign bit.

}


//This calculates the carry and overflow for an add operation
#define ALU_SETADDFLAGS(a,b,result)	\
	ALU_setNZFlags(result);			\
	arm.C = ((a&b)|(a&(~result))|(b&(~result)))>>31;		\
	arm.V = ((a&b&(~result))|((~a)&(~b)&result))>>31;	

//The flag setting code comes from EloGBA's source.  Thanks
//to eloist for this.
#define ALU_SETSUBFLAGS(a,b,result)	\
	ALU_setNZFlags(result);			\
	arm.C = ((a&(~b))|(a&(~result))|((~b)&(~result)))>>31;	\
	arm.V = ((a&~(b|result))|((b&result)&~a))>>31;		


//The flag setting code comes from EloGBA's source.  Thanks
//to eloist for this.
//This calculates the carry and overflow for an add operation
__inline void ALU_setAddFlags (u32 a, u32 b, u32 result) {
	ALU_setNZFlags(result);	
	arm.C = ((a&b)|(a&(~result))|(b&(~result)))>>31;
	arm.V = ((a&b&(~result))|((~a)&(~b)&result))>>31;
}

//The flag setting code comes from EloGBA's source.  Thanks
//to eloist for this.
//This calculates the carry and overflow for a subtract operation
__inline void ALU_setSubFlags (u32 a, u32 b, u32 result) {
	ALU_setNZFlags(result);	
	arm.C = ((a&(~b))|(a&(~result))|((~b)&(~result)))>>31;
	arm.V = ((a&~(b|result))|((b&result)&~a))>>31;


}

//////////////////////////////////////////////////////////////
//barrelShift -- used to shift operands of the ALU based on immediate shift values
//NOTE That this version of ALU barrelShift does not set the carry bit!
//IF you need to set the carry bit, use barrelShiftS
//////////////////////////////////////////////////////////////////
__inline u32 ALU_barrelShift (u32 value, u32 shiftAmt, u32 shiftType) {
	int valueSigned;
		
	//We will never get an immediate value shift greater than 31, because it's
	//only specified in 5 bits.  
	//Note:  apparently, in windows, if your shift amount is >= 32, it
	//does something like shiftAmt = shiftAmt %32; not what we want.
	if (shiftAmt >= 32)	//Should never occur . . .but who knows
		shiftAmt = 0;
	
	switch (shiftType) {
			case SHIFT_LSL:  //LSL                   Logical Shift Left
				 

				value = value << shiftAmt;
				break;
			case SHIFT_LSR:	//LSR #c  for c != 0      Logical Shift Right
							//LSR #32 for c  = 0
				//if (shiftAmt) {							//Get last bit shifted
					value = value >> shiftAmt;
						
				//} else {
			
				//	value = 0;
				//}
				break;

			case SHIFT_ASR:	//ASR #c  for c != 0      Arithmetic Shift Right
				if (shiftAmt) {
				//ASR #32 for c  = 0					//Get last bit shifted
				//		arm.C = (value >> (shiftAmt -1)) & 0x00000001;
						valueSigned = value;			//Convert value to signed
						value = valueSigned >> shiftAmt;
				} else {
					//arm.C = value >> 31;
					if (arm.C) value = 0xFFFFFFFF; else value = 0;
				}
				break;
			case SHIFT_ROR:	//ROR #c  for c != 0      Rotate Right.
					//RRX     for c  = 0      Rotate Right one bit with extend.
				
				if (shiftAmt) {	//ROR
					//arm.C = (value >> (shiftAmt -1)) & 0x00000001;
					//value = _rotr (value, shiftAmt);
					value = (value << (32 - shiftAmt)) | (value >> shiftAmt);
				
				} else {		//RRX (tricky)
					//arm.C = value & 0x00000001;
					
					if ( arm.C) { //arm.CPSR & 0x20000000) {	//If the carry bit set
						value = ((value>>1)|0x80000000);
					} else {
						value = value >> 1;
					}
				}
				break;		
			
	}//End of switch

	return value;

}

//////////////////////////////////////////////////////////////////////
//barrelShiftS -- used to shift operands of the ALU, setting the last
//bit shifted out to the carry bit
//////////////////////////////////////////////////////////////////////
__inline u32 ALU_barrelShiftS (u32 value, u32 shiftAmt, u32 shiftType) {
	int valueSigned;
	//arm.C = 0;	//Why did I put this here?? it caused me more pain . . .
	
	//Note:  apparently, if your shift amount is greater than 32, it
	//does something like shiftAmt = shiftAmt %32; not what we want.
	if (shiftAmt >= 32)
		value = 0;

	switch (shiftType) {
			case SHIFT_LSL:  //LSL                   Logical Shift Left
				arm.C = ((value << (shiftAmt - 1)) & 0x80000000) >> 31;
				value = value << shiftAmt;
				break;
			case SHIFT_LSR:	//LSR #c  for c != 0      Logical Shift Right
							//LSR #32 for c  = 0
				if (shiftAmt) {							//Get last bit shifted
					arm.C = (value >> (shiftAmt -1)) & 0x00000001;
					value = value >> shiftAmt;
						
				} else {
					arm.C = value >> 31;
					value = 0;
				}
				break;

			case SHIFT_ASR:	//ASR #c  for c != 0      Arithmetic Shift Right
				if (shiftAmt) {
									//Get last bit shifted
						arm.C = (value >> (shiftAmt -1)) & 0x00000001;
						valueSigned = value;			//Convert value to signed
						value = valueSigned >> shiftAmt;
				} else {	//ASR #32 for c  = 0
					arm.C = value >> 31;
					if (arm.C) value = 0xFFFFFFFF; else value = 0;
				}
				break;
			case SHIFT_ROR:	//ROR #c  for c != 0      Rotate Right.
					//RRX     for c  = 0      Rotate Right one bit with extend.
				
				if (shiftAmt) {	//ROR
					arm.C = (value >> (shiftAmt -1)) & 0x00000001;
					//value = _rotr (value, shiftAmt);
					value = (value << (32 - shiftAmt)) | (value >> shiftAmt);
				
				} else {		//RRX (tricky)
					
					if ( arm.C) { //arm.CPSR & 0x20000000) {	//If the carry bit set
						arm.C = value & 0x00000001;	//Set to last bit shifted out
						value = ((value>>1)|0x80000000);
					} else {
						arm.C = value & 0x00000001;	//Set to last bit shifted out
						value = value >> 1;
					}
					
					
				}
				break;		
			
	}//End of switch

	//Now update the arm.CPSR with any carry flag changes
	//NOTE that in the case of arithmetic operations (such as add), this
	//value will be overwritten by the value resulting
	//from the operation.
	//arm.CPSR = arm.CPSR | (arm.C << 29);
	return value;

}

//////////////////////////////////////////////////////////////
//barrelRegShift -- used to shift operands of the ALU based on register shift values
//NOTE That this version of ALU barrelRegShift does not set the carry bit!
//IF you need to set the carry bit, use barrelRegShiftS
//////////////////////////////////////////////////////////////////
__inline u32 ALU_barrelRegShift (u32 value, u32 shiftAmt, u32 shiftType) {
	int valueSigned;
		
	//We will never get an immediate value shift greater than 31, because it's
	//only specified in 5 bits.  But since this is a reg shift, we need to do some
	//extra switching (see ARM instruction code manual, 3-15) for cases where
	//the shiftAMt is greater or equal to 32.
	if (shiftAmt == 0)	//According to ARM specs, a reg shift of 0 results in
		return value;			//no change.
	
	switch (shiftType) {
			case SHIFT_LSL:  //LSL                   Logical Shift Left
				if (shiftAmt >= 32)
					value = 0;
				else
					value = value << shiftAmt;
				break;
		
			case SHIFT_LSR:	//LSR #c  for c != 0      Logical Shift Right
							//LSR #32 for c  = 0
				if (shiftAmt >= 32) 
					value = 0;
				else							//Get last bit shifted
					value = value >> shiftAmt;
						
				//} else {
			
				//	value = 0;
				//}
				break;

			case SHIFT_ASR:	//ASR #c  for c != 0      Arithmetic Shift Right
				//if (shiftAmt) {	//ASR #32 for c  = 0					//Get last bit shifted
					if (shiftAmt >= 32) {
						
						valueSigned = value;			//Convert value to signed
						value = valueSigned >> 31;	//Fill upper bits with bit 31
					}
				//} else {
					//arm.C = value >> 31;
				//	if (arm.C) value = 0xFFFFFFFF; else value = 0;
				//}
				break;
			case SHIFT_ROR:	//ROR #c  for c != 0      Rotate Right.
					//RRX     for c  = 0      Rotate Right one bit with extend.
				shiftAmt = shiftAmt &0x1F;	//"mod" shift amount

				if (shiftAmt) {	//ROR
					//arm.C = (value >> (shiftAmt -1)) & 0x00000001;
					//value = _rotr (value, shiftAmt);
					value = (value << (32 - shiftAmt)) | (value >> shiftAmt);
				
				} else {		//RRX (tricky)
					//arm.C = value & 0x00000001;
					
					if ( arm.C) { //arm.CPSR & 0x20000000) {	//If the carry bit set
						value = ((value>>1)|0x80000000);
					} else {
						value = value >> 1;
					}
				}
				break;		
			
	}//End of switch

	return value;

}

//////////////////////////////////////////////////////////////////////
//barrelRegShiftS -- used to shift operands of the ALU, setting the last
//bit shifted out to the carry bit.  See the ARM instruction code manual,
//3 - 15 for details on shifts greater than or equal to 32
//////////////////////////////////////////////////////////////////////
__inline u32 ALU_barrelRegShiftS (u32 value, u32 shiftAmt, u32 shiftType) {
	int valueSigned;
	//arm.C = 0;	//Why did I put this here?? it caused me more pain . . .
	
	//According to ARM docs, a register shift amount of 0 results in no change.
	if (shiftAmt == 0)
		return value;

	switch (shiftType) {
			case SHIFT_LSL:  //LSL                   Logical Shift Left
				if (shiftAmt >= 32) {
					if (shiftAmt == 32) 	//equal to 32
						arm.C = value & 0x1;
					else					//greater than 32...
						arm.C = 0;

					value = 0;
				} else {
					arm.C = ((value << (shiftAmt - 1)) & 0x80000000) >> 31;
					value = value << shiftAmt;
				}
				break;
			case SHIFT_LSR:	//LSR #c  for c != 0      Logical Shift Right
							//LSR #32 for c  = 0
				//if (shiftAmt) {							//Get last bit shifted
					
					if (shiftAmt >= 32) {
						if (shiftAmt == 32)	//equal to 32
							arm.C = value >> 31;
						else				//greater than 32
							arm.C = 0;
						value = 0;
					} else {
					
						arm.C = (value >> (shiftAmt -1)) & 0x00000001;
						value = value >> shiftAmt;
					}
						
				//} else {
				//	arm.C = value >> 31;
				//	value = 0;
				//}
				break;

			case SHIFT_ASR:	//ASR #c  for c != 0      Arithmetic Shift Right
				if (shiftAmt > 0 ) {//ASR #32 for c  = 0					//Get last bit shifted
					if (shiftAmt >= 32) {	//This case could be consolidated with shiftAmt == 0 case
						arm.C = value >> 31;
						valueSigned = value;			//Convert value to signed
						value = valueSigned >> 31;	//Fill upper bits with bit 31

					} else {
						
						arm.C = (value >> (shiftAmt -1)) & 0x00000001;
						valueSigned = value;			//Convert value to signed
						value = valueSigned >> shiftAmt;
					}
				} else {
					arm.C = value >> 31;
					if (arm.C) value = 0xFFFFFFFF; else value = 0;
				}
				break;
			case SHIFT_ROR:	//ROR #c  for c != 0      Rotate Right.
					//RRX     for c  = 0      Rotate Right one bit with extend.
				if (shiftAmt == 32) {
					arm.C = value >> 31;
					return value;
				}
				else 
					shiftAmt = shiftAmt & 0x1F;	//"mod" shift amounts greater than 31
				
				//I am thinking that RRX is impossible for register shifts, since
				//shiftAmt of 0 is supposed to result in no change.  If this is the case,
				//perhaps we can remove this code??
				if (shiftAmt) {	//ROR
					arm.C = (value >> (shiftAmt -1)) & 0x00000001;
					value = (value << (32 - shiftAmt)) | (value >> shiftAmt);
				} else {		//RRX (tricky)
										
					if ( arm.C) { //arm.CPSR & 0x20000000) {	//If the carry bit set
						arm.C = value & 0x00000001;	//Set to last bit shifted out
						value = ((value>>1)|0x80000000);
					} else {
						arm.C = value & 0x00000001;	//Set to last bit shifted out
						value = value >> 1;
					}
				}
				break;		
			
	}//End of switch

	//Now update the arm.CPSR with any carry flag changes
	//NOTE that in the case of arithmetic operations (such as add), this
	//value will be overwritten by the value resulting
	//from the operation.
	//arm.CPSR = arm.CPSR | (arm.C << 29);
	return value;

}


#endif


