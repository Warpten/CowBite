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

#ifndef GBA_H
#define GBA_H

#include <time.h>
#include <stdio.h>
#include <windows.h>
#include "ARM.h"
#include "Graphics.h"
#include "IO.h"
#include "Audio.h"
#include "ELFFile.h"
#include "Stabs.h"

//A structure for holding statistics about the GBA
typedef struct tGBAStats {
	//Graphics
	u32 frameTally;
		
	//Clock cycles
	u32 frameTickTally;		//Number of ticks from the start of this frame
	u32 lastTally;
	u32 timerStartValues[4];	//Holds the start value of all the timers.
	u32 timerTickTally[4];	//A tally of ticks for lower-frequency timers
//	u32 lineTickTally;		//Number of ticks from the start of this line
	
	//DMA
	u32 dmaTransfers[4];		//Log of DMA transfers
	u32 frameDMATransfers[4];	//Number of DMA Transfers this frame
	u32 dmaDestStartVals[4];		//Start values of dma (used for FIFO)
	u32 dmaSourceStartVals[4];		//Start values of dma (used for FIFO)

	//Memory errors
	u32 readWordXBounds[0x100];
	u32 lastReadWordXBounds;
	u32 readHWXBounds[0x100];
	u32 lastReadHWXBounds;
	u32 writeWordXBounds[0x100];
	u32 lastWriteWordXBounds;
	u32 writeHWXBounds[0x100];
	u32 lastWriteHWXBounds;
	u32 writeByteXBounds[0x100];
	u32 lastWriteByteXBounds;
	
	u32 invalidWrites[MEMORY_NUMSECTIONS];
	u32 lastInvalidWrite[MEMORY_NUMSECTIONS];
	u32 invalidReads[MEMORY_NUMSECTIONS];
	u32 lastInvalidRead[MEMORY_NUMSECTIONS];
	
	//Interrupts
	u32 interrupts[16];
	u32 frameInterrupts[16];
	u32 lastInterrupt[16];	//PC address at last interrupt
	
	//Sound
	u32 samplesIntoFifoA;
	u32 samplesIntoFifoB;
	u32 samplesPlayedA;
	u32 samplesPlayedB;

} GBAStats;

//A structure to save or load a new GBA state.
typedef struct tGBAState {
	char filename[256];		//The name and path of the ROM
//	u32 frameTickTally;			//Ticks counted on this frame
//	u32 lastTally;			//Number of ticks the last time we checked
//	u32 timerStartValues[4];
//	u32 timerTickTally[4];
	
	int numROMPages;
	u32 thumbMode;
	int instructionLength;	//Instruction length (in bytes)
	int instructionWidth;	//Instruction width (bits to shift by)

	u32 currentInstruction;	//The current instruction
	/*u32 instruction3;		//The current instruction
	u32 instruction2;		//The instruction currently being decoded
	u32 instruction1;		//The instruction just fetched*/

	u32 r_gp[16];	//For keeping the old register values when swapping in new ones
	u32 r[16];		//Registers as seen by user.  
	u32 r_fiq[16];
	u32 r_svc[16];
	u32 r_abt[16];
	u32 r_irq[16];
	u32 r_und[16];
	//u32 r_sys[16];
		
	/////////////////////////////////////////////////////
	//CPU State variables
	u32 CPSR;		//Program state register
	u32 SPSR_fiq;	//Registers for saving last state when changing modes
	u32 SPSR_svc;
	u32 SPSR_abt;
	u32 SPSR_irq;
	u32 SPSR_und;
	//u32 SPSR_sys;
	//////////////////////////////////////////////////////

	//////////////////////////////////////////
	//DMA stuff
	u32 dmaSource[4];	//The current dma source counter
	u32 dmaDest[4];	//The current dma destination counter
	u16 dmaCount[4];	//The current dma count
	u16 dmaCnt[4];		//The current dma controller
	////////////////////////////////////////////////////

	///////////////
	u32 halted;

	///////////////////////////////////////////////////////
	//Memory.  Note that when saving and loading states, we do not save
	//or load the ROM!!
	GBAMem gbaMem;
/*	u8  WRAMInt8bit[WRAM_INT_SIZE];
	u8  WRAMExt8bit[WRAM_EXT_SIZE];
	u8  pal8bit[PAL_SIZE];
	u8  VRAM8bit[VRAM_SIZE];
	u8  OAM8bit[OAM_SIZE];	
	u8  IORAM8bit[IORAM_SIZE];
	u8	cartRAM8bit[CARTRAM_SIZE];
*/	//u8 screen[SCREEN_SIZE];
	GBAStats stats;				//Statistics
} GBAState;

/////////////////////////////////
//GBA
//Class encapsulating the functionality of the game boy advance
//hardware.
//////////////////////////////////
//class GBA {
//	public:
void GBA_init(char* path);
void GBA_delete();

void GBA_reset();
int GBA_loadROM (char *nFilename);
int GBA_setState (GBAState* newState);
int GBA_getState(GBAState* state);
void GBA_saveState (int state);
void GBA_saveState (char* filename);
void GBA_loadState (int state);
void GBA_loadState (char* filename);
void GBA_debugHDraw();
void GBA_debugHBlank();
void GBA_debugVDraw();
void GBA_debugVBlank();
void GBA_sync();
void GBA_hardwareInterrupt(u32 interruptFlag);

typedef struct tGBA {
		ELFFile elfFile;
		Stabs stabs;
		GBAState nullState;	//For resetting the GBA to a default state
		char path[_MAX_PATH];	//Path to cowbite.
		char filename[_MAX_PATH];	//Filename of the rom.
		char romPath[_MAX_PATH];	//Path to the rom
		char biosPath[_MAX_PATH];
		u32 fileSize;
		int loaded;
		FILE* logFile;
		time_t currentTime;
		time_t lastTime;
		LARGE_INTEGER currentTime64;
		LARGE_INTEGER lastTime64;
		//LARGE_INTEGER frameStartTime;	//Use to determine frame skipping
		__int64 displayInterval;	//Delay needed for a 60hz-ish sync

		u32 hDrawBeginCycles[228];	//Array listing how many cycles at the begin of
									//each hDraw
		u32 hDrawEndCycles[228];	//Array listing how many cycles at the end of
									//each hDraw
		u32 hBlankBeginCycles[228];	//Array listing how many cycles at the begin of each
									//hblank
		u32 hBlankEndCycles[228];	//Array listing how many cycles at the begin of each
									//hblank
		u32 hDrawSegmentCycles[3];	//Array listing how many clock ticks between each
									//time the emu services the timer
		
		u32 dmaSource[4];	//The current dma source counter
		u32 dmaDest[4];	//The current dma destination counter
		u16 dmaCount[4];	//The current dma count
		u16 dmaCnt[4];		//The current dma controller
	
		
		GBAStats stats;
		
} GBA;
//};

extern GBA gba;




#endif