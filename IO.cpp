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

#include <memory.h>
#include "ARM.h"
#include "Support.h"
#include "Memory.h"
#include "Graphics.h"
//#include "ARMExtern.h"
#include "IO.h"
#include "GBA.h"
#include "Audio.h"



u16* IO_keyInput;
//gba.stats gba.stats;

///////////////////////////
//IO_init()
//Initializes any variables related to IO
////////////////////////////
void IO_init() {
	IO_keyInput = (u16*)(gbaMem.u8IORAM + 0x130);
	//FFFF;
	//IO_keyInput = (u16*)(gbaMem.u8IORAM + 0x98);
}

////////////////////////////
//sets the flag for the joypad up.
//upVal is either 1 or 0.
////////////////////////////////
void IO_setUp (u32 setVal) {
	//Zero out bit 6, set it to setVal.
	*IO_keyInput  = (*IO_keyInput & 0xFFBF) | (setVal << 6);
}


void IO_setDown (u32 setVal) {
	//Zero out bit 7, set it to setVal.
	*IO_keyInput  = (*IO_keyInput & 0xFF7F) | (setVal << 7);
}

void IO_setLeft (u32 setVal) {
	//Zero out bit 5, set it to setVal.
	*IO_keyInput  = (*IO_keyInput & 0xFFDF) | (setVal << 5);
}

void IO_setRight (u32 setVal) {
	//Zero out bit 4, set it to setVal.
	*IO_keyInput  = (*IO_keyInput & 0xFFEF) | (setVal << 4);
}

void IO_setA (u32 setVal) {
	//Zero out bit 0, set it to setVal.
	*IO_keyInput  = (*IO_keyInput & 0xFFFE) | setVal ;
}

void IO_setB (u32 setVal) {
	//Zero out bit 1, set it to setVal.
	*IO_keyInput  = (*IO_keyInput & 0xFFFD) | (setVal << 1);
}

void IO_setL (u32 setVal) {
	//Zero out bit 9, set it to setVal.
	*IO_keyInput  = (*IO_keyInput & 0xFDFF) | (setVal << 9);
}

void IO_setR (u32 setVal) {
	//Zero out bit 8, set it to setVal.
	*IO_keyInput  = (*IO_keyInput & 0xFEFF) | (setVal << 8);
}

void IO_setStart (u32 setVal) {
	//Zero out bit 3, set it to setVal.
	*IO_keyInput  = (*IO_keyInput & 0xFFF7) | (setVal << 3);
}

void IO_setSelect(u32 setVal) {
	//Zero out bit 2, set it to setVal.
	*IO_keyInput  = (*IO_keyInput & 0xFFFB) | (setVal << 2);
}


//////////////////////////////////////////////////////////
//IO_updateReg
//This function gets called whenever a reg is read and may
//need to be updated with the most recent value (such as a timer reg)
//////////////////////////////////////////////////////////
void IO_updateReg(u32 address) {
	if ((address >= IO_TM0DAT) && (address <= IO_TM3DAT+2)) {
		IO_updateTimers();
	}
}

//////////////////////////////////////////////////////////
//IO_checkForUpdate
//This function should be called whenever io is written,
// in order to check if dma or other registers were accessed
//It checks based on 16 bit words.
///////////////////////////////////////////////////////////
int IO_checkForUpdate (u32 address, u16 newValue) {	//
	u16* dest;
	u16 oldValue;
	//static u32 dspcntLast = 0;
	u32 offset;
	u32 audioDirty = 0;

	offset = address & 0x00FFFFFF;

	//Calculate the destination to write to
	dest = ((u16*)&gbaMem.u8IORAM[offset]);
	
	//Save the old value
	oldValue = *dest;

	//Write the new value.
	*dest = newValue;
	

	//If the user is writing to the display control register...
	if (address == 0x04000000) {
		graphics.screenMode = newValue;
		if (oldValue != newValue) {	//If something has been modified...
			//If windows, hblank obj, or sprites have been enabled/disabled...
			if ( (oldValue & 0xF060) != (newValue & 0xF060) )
				graphics.spritesDirty = 1;	//Let us know they need to be initialized
				//graphics.initSprites(128);	//Re initialize sprites.

			//If the background flags have been modified...
			if ( (oldValue & 0xEF17) != (newValue & 0xEF17) )
				graphics.backgroundsDirty = 1;	//Let us know they need to be initialized
				//graphics.initBackgrounds();
		}
	
	//Check to see if the user wrote a word, halfword, or byte to IO_DMA0CNT
	} else if (address == IO_DMA0CNT) {	//((address >= IO_DMA0COUNT) && (address < IO_DMA0COUNT+4) ) {			
	gba.dmaCnt[0] = *REG_DMA0CNT;	//Needed?
		if (newValue & 0x8000) {	//If the user enabled DMA
			gba.dmaDest[0] = *REG_DMA0DST;
			gba.dmaSource[0] = *REG_DMA0SRC;
			gba.dmaCount[0] = *REG_DMA0COUNT;
			gba.dmaCnt[0] = *REG_DMA0CNT;

			//If the transfer mode is immediate...
			if ( !(newValue & 0x3000)) {
				IO_dmaTransfer (0);
			}
		} else if ( oldValue & 0x8000) {	//If the user disabled DMA
				gba.dmaCnt[0] = *REG_DMA0CNT;	//Needed?
		}
	
	//Check to see if the user wrote a word, halfword, or byte to IO_DMA1CNT
	} else if (address == IO_DMA1CNT) {
		gba.dmaCnt[1] = *REG_DMA1CNT;	//Needed?
		if ( newValue & 0x8000) {	//If the user enabled dma...
			gba.dmaDest[1] = *REG_DMA1DST;
			gba.dmaSource[1] = *REG_DMA1SRC;
			gba.dmaCount[1] = *REG_DMA1COUNT;
			gba.dmaCnt[1] = *REG_DMA1CNT;
		
			//If the transfer mode is immediate...
			if ( !(newValue & 0x3000)) {
				IO_dmaTransfer (1);
			}

		//If the user disabled DMA
		} else if ( oldValue & 0x8000) {
				gba.dmaCnt[1] = *REG_DMA1CNT;	//Needed?
				//if it was previously in fifo mode...
				//if ( ((oldValue >> 12) & 0x3) == 0x3 ) {
				//*REG_DMA1SRC = gba.stats.dmaSourceStartVals[1];
				//}
		}
		
	
		
		
	//Check to see if the user wrote a word, halfword, or byte to IO_DMA2CNT
	} else if (address == IO_DMA2CNT) {
		gba.dmaCnt[2] = *REG_DMA2CNT;	//Needed?
		if ( newValue & 0x8000) {	//If the user enabled dma...
			gba.dmaDest[2] = *REG_DMA2DST;
			gba.dmaSource[2] = *REG_DMA2SRC;
			gba.dmaCount[2] = *REG_DMA2COUNT;
			gba.dmaCnt[2] = *REG_DMA2CNT;

			//If the transfer mode is immediate...
			if ( !(newValue & 0x3000)) {
				IO_dmaTransfer (2);
			}
		} else if ( oldValue & 0x8000) {
				gba.dmaCnt[2] = *REG_DMA2CNT;	//Needed?
				//if it was previously in fifo mode...
				//if ( ((oldValue >> 12) & 0x3) == 0x3 ) {
				//*REG_DMA1SRC = gba.stats.dmaSourceStartVals[1];
				//}
		}
		
		
	//Check to see if the user wrote a word, halfword, or byte to IO_DMA3CNT
	} else if (address == IO_DMA3CNT) {
			gba.dmaCnt[3] = *REG_DMA3CNT;	//Needed?
		if (newValue & 0x8000) {	//If the user enabled DMA
			gba.dmaDest[3] = *REG_DMA3DST;
			gba.dmaSource[3] = *REG_DMA3SRC;
			gba.dmaCount[3] = *REG_DMA3COUNT;
			gba.dmaCnt[3] = *REG_DMA3CNT;
			//If the transfer mode is immediate...
			if ( !(newValue & 0x3000)) {
				IO_dmaTransfer (3);
			}

		} else if ( oldValue & 0x8000) {
				gba.dmaCnt[3] = *REG_DMA3CNT;	//Needed?
				//if it was previously in fifo mode...
				//if ( ((oldValue >> 12) & 0x3) == 0x3 ) {
				//*REG_DMA1SRC = gba.stats.dmaSourceStartVals[1];
				//}
		}
		
		
	} else if (address == 0x04000202) {	//If the flags are being written to...
		*(REG_IF) = oldValue & ~(newValue);	//Unset every value being set.
	
	//Check to see if the clocks were written to.
	} else if (address == IO_TM0DAT) {
		gba.stats.timerStartValues[0] = newValue;
	} else if (address == IO_TM1DAT) {
		gba.stats.timerStartValues[1] = newValue;
	} else if (address == IO_TM2DAT) {
		gba.stats.timerStartValues[2] = newValue;
	} else if (address == IO_TM3DAT) {
		gba.stats.timerStartValues[3] = newValue;
	}
	//Check to see if the timer controllers were written to.
	else if (address == IO_TM0CNT) {
		
		//If the sound controller is using timer 0 to control fifos
		if (oldValue != newValue)
			if (!(*REG_SOUNDCNT_H & 0x0400) || !(*REG_SOUNDCNT_H & 0x4000)) {
				audioDirty = 1;
			}
	} else if (address == IO_TM1CNT) {
		//If the sound controller is using timer 1 to control fifos
		if (oldValue != newValue)
		if ((*REG_SOUNDCNT_H & 0x0400) || (*REG_SOUNDCNT_H & 0x4000)) {
			audioDirty = 1;
		}

	} else if (address == IO_TM2CNT) {
		//gba.stats.timerStartValues[2] = *REG_TM2DAT;
	} else if (address == IO_TM3CNT) {
		//gba.stats.timerStartValues[3] = *REG_TM3DAT;
	}
	else if (address == IO_SOUNDCNT_H) {
		//If the user wrote to this register...
		if (newValue & 0x0800) {	//If fifo A reset...
			audio.fifoAPos = 0;	//For some reason this screws the sound up.
			REG_FIFO_A[0] = REG_FIFO_A[1] = REG_FIFO_A[2] = REG_FIFO_A[3] = 0;
			*REG_SOUNDCNT_H = newValue & 0xF7FF;	//reset the reset bit.
		}
		
		if (newValue & 0x8000) {
			audio.fifoBPos = 0;	//This screws the sound up.
			REG_FIFO_B[0] = REG_FIFO_B[1] = REG_FIFO_B[2] = REG_FIFO_B[3] = 0;
			*REG_SOUNDCNT_H = newValue & 0x7FFF;	//reset the reset bit.
		}
		//If anything other than the fifo reset has changed, audio dirty.
		if ( (newValue & 0x77FF) != (oldValue & 0x77FF)) {
			audioDirty = 1;
		}
	
	} 
	//If sound was enabled or disabled
	else if (address == IO_SOUNDCNT_X) {	
		if ( (oldValue & 0x0080) != (newValue & 0x0080)) {
			audioDirty = 1;
		}
	}
	//if audio is dirty, we must mix on the fly (to keep it up to date with
	//the changes, and prevent artifacts)
	if (audioDirty) {
		Audio_mixUpdate();

	}

	return 0;
}

///////////////////////////////////////////////////////////////////
//IO_dmaTransfer
//Function replicating a DMA transfer.
///////////////////////////////////////////////////////////////////
void IO_dmaTransfer (u32 dmaNum) {
	//Initialize them all to DMA 0, then add an offset					 
	/*
	u32* srcPtr = REG_DMA0SRC;
	u32* dstPtr = REG_DMA0DST;

	u16* countPtr = REG_DMA0COUNT;
*/
	u16* cntPtr = REG_DMA0CNT;
	//u16* cntPtr = &gba.dmaCnt[dmaNum];	//This may break it...

	//Add an offset of 12 * dmaNum (multiply the dma num by 3)
	u32 i = dmaNum * 3;
/*	srcPtr += i;
	dstPtr += i;
	countPtr += (i << 1);
*/	
	cntPtr += (i << 1);


	u32 numTransfers;
	s32 sourceIncrement, destIncrement;

	u32 sourceOrig, destOrig, sourceAddress, destAddress, savedLocation;
	u16 cnt;		//cnt stands for "Control", not "count"
	u32 destWriteback;
	u32 data32bit;
	u16 data16bit;
	cnt = *cntPtr;	//Possible to eliminate this??
		
		
	

	//If transfer is not enabled, return.
	if (! (cnt & 0x8000))
		return;

	//NOTE:  I think i remedied the below comment.  Too tired to verify.
	//A note about this:  I use arm.location to be able to tell where the last memory was
	//(it's kind of like a Memory Address Register).  However, since I use this variable
	//to write to memory in the DMA function (and also, to check for conditional braekpoints)
	//I work myself into the cluge of having to save the old arm.location every time.
	//in order not to cause errors with instructions that also depend on arm.location (such as ldmia
	//and stmia)
	//Solution (unimplemented):  Make it so that memory accesses use a parameter rather than a global variable,
	//and make it so that breakpoints use some other means -- perhaps having an
	//gba.dmaLocation variable.
	//Possible errors if I never implement this:  Situation where user's DMA triggers
	//yet another DMA?  Not sure...
	savedLocation = arm.location;

	//Commented out the below becuase I think the DMA gets its source and dest
	//by copying them from temporary registers when it is fist enabled.

	//sourceOrig = sourceAddress = *srcPtr;
	//destOrig = destAddress = *dstPtr;
	sourceOrig = sourceAddress = gba.dmaSource[dmaNum];
	destOrig = destAddress = gba.dmaDest[dmaNum];
	numTransfers = gba.dmaCount[dmaNum];//*countPtr;
	destWriteback = 0;

	//u32 index = destAddress >> 24;
	//if (index == 5)
	//	int blah = 0;

/*
	//if (cnt & 0x80000000) {	//If the first bit is set, immediate transfer
	//	sourceAddress = *(u32*) (IORAM32bit + 0x35);
	//	destAddress = *(u32*)(IORAM32bit + 0x36);
	arm.location = destAddress;	//For the debugger.
	//Find out what kind of memory we are dealing with

			
	//Start them off the same
	//use our little indexing scheme to get the appropriate memory
	sourceIndex = ((sourceAddress & 0xFF000000) >> 24);
	source = gbaMem.u32Mem[sourceIndex];
	sourceOffset = sourceAddress & (0x00FFFFFF);	//Get the last part of the offset

	//Find out what kind of memory the dest is
	destIndex = ((destAddress & 0xFF000000) >> 24);
	dest = gbaMem.u32Mem[destIndex];
	destOffset = destAddress & (0x00FFFFFF);
*/
	
	//numTransfers = count;//Or, according to mappy docs,  count & 0x3FFF;
	sourceIncrement = destIncrement = 1;
	
				
	//If the mode is 11 (transfer on FIFO empty), set the count to 1
	//and dest writeback to 0.
	//Note that I'm not sure about this!!  Will have to ask Uze about it...
	if ( ((dmaNum == 1) || (dmaNum == 2)) && 	(((cnt >> 12) & 0x3) == 0x3) ) {
		numTransfers = 1;
		destWriteback = 0;
		destIncrement = 0;
	
	} else {
		//Otherwise it's a normal copy.
		//Find out the nature of the source copy
		switch ( (cnt >> 7) & 0x3) {
			case 0x0:
			//case 0x3:	//Increment the source address each copy
				//Correct?
						break;

			case 0x1:	//Decrement the source address each copy
						sourceIncrement = -sourceIncrement;
						break;

			default:	//In any other case, no increment.
						sourceIncrement = 0;
		}

		//Find out the nature of the destination copy
		switch ( (cnt >> 5) & 0x3) {
			case 0x0:	destWriteback = 1;
						break;

			case 0x3:	//Incrmenet the dest after each copy, but NO writeback.
						destWriteback = 0;
						break;

			case 0x1:	//Decrement the dest address each copy
						destIncrement = -destIncrement;
						destWriteback = 1;
						break;

			default:	//In any other case, no increment.
						destWriteback = 0;
						destIncrement = 0;
		}
	}

	

	if ( cnt & 0x0400) {	//If the S bit is set, 32 bit
		for (i = numTransfers; i > 0; i--) {
			//arm.location = sourceAddress;
			data32bit = Memory_readWord(sourceAddress);
		
			//In case a user reaches breakpoint during DMA
			if (ARM_reachedConditionalBreakpoint(0xFFFFFFFF)) {
				arm.paused = 1;
				arm.steps = arm.stepAddress = arm.stepInto = 0;
				arm.stepFunction = NULL;
			} 


			//arm.location = destAddress;
			Memory_writeWord (destAddress, data32bit);

			//This is necessary in case the user reaches a breakpoint during DMA...
			if (ARM_reachedConditionalBreakpoint(0xFFFFFFFF)) {
				arm.paused = 1;
				arm.steps = arm.stepAddress = arm.stepInto = 0;
				arm.stepFunction = NULL;
			} 
						
			sourceAddress+= (sourceIncrement << 2);
			
		//	*srcPtr += (sourceIncrement << 2);
			gba.dmaSource[dmaNum] += (sourceIncrement << 2);
			destAddress += (destIncrement << 2);
			if (destWriteback)
			//	*dstPtr += (destIncrement << 2);
				//gba.dmaDest[dmaNum] += (destIncrement << 2);
				gba.dmaDest[dmaNum] = destAddress;

		}


	} else {	//Otherwise it's 16 bit, and we have to do stuff.
		for (i = numTransfers; i > 0; i--) {
			//arm.location = sourceAddress;
			data16bit = Memory_readHalfWord(sourceAddress);

			//This is necessary in case the user reaches a breakpoint during DMA...
			if (ARM_reachedConditionalBreakpoint(0xFFFFFFFF)) {
				arm.paused = 1;
				arm.steps = arm.stepAddress = arm.stepInto = 0;
				arm.stepFunction = NULL;
			} 
			
			//arm.location = destAddress;
			Memory_writeHalfWord (destAddress, data16bit);

			//This is necessary in case the user reaches a breakpoint during DMA...
			if (ARM_reachedConditionalBreakpoint(0xFFFFFFFF)) {
				arm.paused = 1;
				arm.steps = arm.stepAddress = arm.stepInto = 0;
				arm.stepFunction = NULL;
			} 
						
			sourceAddress+= (sourceIncrement << 1);
		//	*srcPtr += (sourceIncrement << 1);
			gba.dmaSource[dmaNum] += (sourceIncrement << 1);
			destAddress += (destIncrement << 1);
			if (destWriteback)
				//*dstPtr += (destIncrement << 1);
				//gba.dmaDest[dmaNum] += (destIncrement << 1);
				gba.dmaDest[dmaNum] = destAddress;
		}

	}

	
	//The first bit should get reset (I think) if the repeat bit is not set.
	if ( ! (cnt & 0x0200) ) {
		cnt = cnt & 0x7FFF;
		*cntPtr = cnt;	//Write back to reg
	}
	
	//Restore the global location variable (yeah I suck for having to do this)
	//Might not be necessary now that I pass addresses as a parameter;)
	arm.location = savedLocation;

	//Record the transfer in our statistics
	gba.stats.dmaTransfers[dmaNum]++;
	gba.stats.frameDMATransfers[dmaNum]++;

	//If set to interrupt after a transfer...
	if (SUPPORT_GETBIT14(cnt)) {
		//Perform an interrupt if necessary.
		switch (dmaNum) {
			case 0:
				GBA_hardwareInterrupt(INTERRUPT_DMA0);
				break;
			case 1:
				GBA_hardwareInterrupt(INTERRUPT_DMA1);
				break;
			case 2:
				GBA_hardwareInterrupt(INTERRUPT_DMA2);
				break;
			case 3:		
				GBA_hardwareInterrupt(INTERRUPT_DMA3);
				break;
		}
	}
}

////////////////////////////////////////
//updateTimers()
//This method attempts to update all the timers in a somewhat realistic manner.
//Because we know there are 280896 cpu ticks each frame (at 59.727 hz),
//we keep a tally that goes from 0 to 280896.  We cannot update all clocks
//once per tick, but we can update them at intervals.  Thus at each interval
//we keep track of how many ticks elapsed since the time we updated the timers.
//
////////////////////////////////////////
void IO_updateTimers() {
	u32 cnt;
	u32 dat;
	u32 freq;
	u32 numTicks;
	u16 *currentTimerCnt = REG_TM0CNT;
	u16 *currentTimerDat = REG_TM0DAT;
	u32 *currentTimerTally;
	u32 timerOverflow = 0;
	u32 interruptPending = 0;
	
	//Find out how many ticks have elapsed since the last update.
	numTicks = gba.stats.frameTickTally - gba.stats.lastTally;
	if (numTicks == 0)
		return;

	gba.stats.lastTally = gba.stats.frameTickTally;

	//Go through each timer, see if they are enabled,
	//update, or cascade.  Uses pointer arithmatic.
	for (int i = 0; i < 4; i++, currentTimerCnt+=2, currentTimerDat+=2) {
		cnt = *currentTimerCnt;
		dat = *currentTimerDat;
		currentTimerTally=&(gba.stats.timerTickTally[i]);
		if (SUPPORT_GETBIT7(cnt)) {	//If the timer is enabled
			if (SUPPORT_GETBIT2(cnt)) {	//If it's set to cascade
				dat+=timerOverflow;	//Increment this timer's count by the number
									//of times the previous clock overflowed.
									//Timer frequency is ignored for cascaded timers.
			} else {				//Otherwise, we count.
				freq = cnt & 0x3;	//Switch on the frequency.
				switch (freq) {
					case 0x0:	dat+=numTicks;
								*currentTimerTally = 0;
								break;
					case 0x1:	*currentTimerTally+=numTicks;
								while (*currentTimerTally >= 64) {
									*currentTimerTally-=64;
									dat++;
								}
								break;
					case 0x2:	*currentTimerTally+=numTicks;
								while (*currentTimerTally >= 256) {
									*currentTimerTally-=256;
									dat++;
								}
								break;
					case 0x3:	*currentTimerTally+=numTicks;
								while (*currentTimerTally >= 1024) {
									*currentTimerTally-=1024;
									dat++;
								}
								break;

				}//switch(freq)
			}//else
			timerOverflow = 0;
			if (dat > 0xFFFF) {	//Find out how much overflow there was
				int fifoAEmpty, fifoBEmpty;
				
				//Note:  Do DMA 1 and 2 really react individually to fifoa A and fifoB empty,
				//or do they both respond if either is empty?
				fifoAEmpty = fifoBEmpty = 0;
				u32 intervalSize = dat - gba.stats.timerStartValues[i];
				u32 intervalMax = 0x10000 - gba.stats.timerStartValues[i];
				while (intervalSize >= intervalMax) {
					intervalSize -= intervalMax;
					timerOverflow++;
					
					//If sound is enabled....
					if ( SUPPORT_GETBIT7 (*REG_SOUNDCNT_X) ) {

						//If this is timer 0 or 1, see if the sound hardware is using it
						//for the sample rate, and process the fifo
						int soundCntH = *REG_SOUNDCNT_H;
						int sampleTimerA = (soundCntH >> 10) & 1;	//Timer 0 or 1?
						if ( i == sampleTimerA ) {	//If this is the sample rate timer
							fifoAEmpty = Audio_processFifoA();
						}
				
						int sampleTimerB = (soundCntH >> 14) & 1;
						if ( i == sampleTimerB ) {	//If this is the sample rate timer
							//Note that it sees fifo as empty if EITHER A or B is empty
							fifoBEmpty = Audio_processFifoB();
						}

						//Check DMA1 and DMA2 to see if they are set to activate on fifo
						if (fifoAEmpty) {
							if ( ((gba.dmaCnt[1] >> 12) & 0x3) == 0x3) {
								IO_dmaTransfer (1);
							}
						}
						if (fifoBEmpty) {
							//Check DMA1 and DMA2 to see if they are set to activate on fifo
							if ( ((gba.dmaCnt[2] >> 12) & 0x3) == 0x3) {
								IO_dmaTransfer (2);
							}
			
						}//if (fifoEmpty)
				
					//If sound is disabled . ..
					} else {
						Audio_stopSound();
					}

				}//while
				if (SUPPORT_GETBIT6(cnt)) {	//If set to generate interrupts on overflow
					switch(i) {				//Find out interrupt flag to use.
						case 0:	interruptPending = INTERRUPT_TIMER0;
								break;
						case 1: interruptPending = INTERRUPT_TIMER1;
								break;
						case 2: interruptPending = INTERRUPT_TIMER2;
								break;
						case 3: interruptPending = INTERRUPT_TIMER3;
								break;
					}
				}
				//Finally, write back to the timer with the final calculated value
				*currentTimerDat = gba.stats.timerStartValues[i] + intervalSize;
			//	if (intervalSize)
			//		int blah = 0;
			} else {	//If there was no overflow...
				*currentTimerDat = dat;
			}

			
		}//end of if(enabled)
	}//for
	//If we generated an interrupt (note that this will only generate one,
	//even if multiple timers overflowed at once)...
	if (interruptPending) {
		GBA_hardwareInterrupt(interruptPending);
	}
}
