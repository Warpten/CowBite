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
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "GBA.h"
#include "ARM.h"
#include "Graphics.h"
#include "Memory.h"
#include "IO.h"
#include "Gui.h"
#include "Console.h"

GBA gba;

//////////////////////////////////
//GBA
//Constructor for the GBA.
///////////////////////////////////
void GBA_init(char* path) {
	FILE *fp;
	int start, end, biosSize;
	
	//Set everything to 0.
	memset(&gba, 0, sizeof(GBA));
	
	strcpy (gba.path, path);
	strcpy (gba.biosPath, path);
	strcat (gba.biosPath, "CBBIOS.bin");
	
	Memory_init();
	Graphics_init();
	IO_init();
	Audio_init(appState.hWnd);
	ARM_init(&gba.stabs);	//Initialize the gbaArm
	
	arm.debugging = 0;//Start off in non-debug mode.
	

	gba.loaded = 0;

	//initialize the "NULL" state
	memset (&gba.nullState, 0, sizeof(gba.nullState));
	gba.nullState.instructionLength = 4;
	gba.nullState.instructionWidth = 2;
	gba.nullState.r[15] = gba.nullState.r_svc[15] = 0x08000000;
	gba.nullState.r[13] = gba.nullState.r_svc[13] = 0x03007FE0;			//Stack pointer
	
	//Note to self:  do the irq values really start with an initialized stack pointer,
	//or do they use the current stack?  (i.e. by copying r[13] into r_fiq)
	//MAPPY appears to resolve this by giving them a stack address which is 0x20 (64 bytes,
	//or 16 words) offset from the regular stack.  How does mappy get this value?
	//Or is it just arbitrary?
	gba.nullState.r_irq[13] = 0x03007FA0;
	gba.nullState.r_gp[13] = 0x03007F00;	
	gba.nullState.CPSR = 0x00000013;		//Set it so the saved state is initially svc mode
	gba.nullState.SPSR_fiq = 0x00000013;
	gba.nullState.SPSR_svc = 0x0000001F;	//Guess
	gba.nullState.SPSR_abt = 0x00000013;
	gba.nullState.SPSR_irq = 0x00000013;
	gba.nullState.SPSR_und = 0x00000013;
//	nullState.SPSR_sys = 0x00000010;

	//Clear memory of the nullstate (except for ROM)
/*	memset( gba.nullState.WRAMInt8bit, 0, WRAM_INT_SIZE);
	memset (gba.nullState.WRAMExt8bit, 0, WRAM_EXT_SIZE);
	memset (gba.nullState.pal8bit, 0, PAL_SIZE);
	memset (gba.nullState.VRAM8bit, 0, VRAM_SIZE);
	memset (gba.nullState.OAM8bit, 0, OAM_SIZE);
	memset (gba.nullState.IORAM8bit, 0, IORAM_SIZE);
	memset (gba.nullState.cartRAM8bit, 0, CARTRAM_SIZE);
*/
	//Set the REG_EMU and REG_COWBITE to default values so that
	//program code can determine if they are being emulated.
	*(REG_EMU) = 1;
	*(REG_COWBITE) = 1;
	


	GBA_setState(&gba.nullState);	//Set the current State to the nullState.

	//Clear bios.
	memset(gbaMem.u8BIOS, 0, ZERO_PAGE_SIZE);
	
	//Open file with the BIOS
	//if (!(fp = fopen("c:\\emulators\\cowbite\\debug\\cbbios.bin", "rb"))) {
	if (!(fp = fopen(gba.biosPath, "rb"))) {
		Console_printf("\n\n\nUnable fo find bios: %s\n\n\n", gba.biosPath);
		
		return;
	}
	//
	fseek (fp, 0, SEEK_SET);
	start = ftell (fp);
	fseek (fp, 0, SEEK_END);
	end = ftell(fp);
	biosSize = end - start + 1;
	fseek(fp, 0, SEEK_SET);
	
	fread (gbaMem.u8BIOS, biosSize, 1, fp);		//Load the file into this memory
	fclose(fp);								//Close the file.

	//Debugging
//if (!(gba.logFile = fopen("CowBite.log", "w")))
//		printf("\nUnable to open log file.");

	//Find out the resolution of the high-res timer, use it to sync the GBA to the
	//appropriate hz
	LARGE_INTEGER timerFreq;
	QueryPerformanceFrequency(&timerFreq);
	double temp = 16777216/280896;	//This gets the refresh rate in hertz
	gba.displayInterval = timerFreq.QuadPart / temp;

	//Initialize our cycle count array
	for (int i = 0; i < 228; i++) {
		gba.hDrawBeginCycles[i] = (i) * 1232;
		gba.hDrawEndCycles[i] = gba.hDrawBeginCycles[i] + 1004;
		gba.hBlankBeginCycles[i] = gba.hDrawEndCycles[i];
		gba.hBlankEndCycles[i] = gba.hDrawBeginCycles[i] + 1232;
		
	}
	//Intervals at which to update our emulated clocks and check for interrupts
	//during hdraw
	gba.hDrawSegmentCycles[0] = 335;
	gba.hDrawSegmentCycles[1] = 669;
	gba.hDrawSegmentCycles[2] = 1004;
}

////////////////////////////////
//~GBA
//Destructor for the GBA
///////////////////////////////////
void GBA_delete() {
//	fclose (gba.logFile);
	ARM_delete();
	Graphics_delete();
	Memory_delete();
	Audio_delete();
	ELFFile_delete(&gba.elfFile);
	Stabs_delete(&gba.stabs);
}


///////////////////////////////////////////////////
//loadROM
//This function loads the game rom into memory
///////////////////////////////////////////////
int GBA_loadROM (char *nFilename) {
	FILE *fp;
	int start, end;
	u32 romSize;
	
	strcpy (gba.filename, nFilename);

	if (gba.loaded) {	//if already loaded, free the rom
		delete [] gbaMem.u8ROM;
		gba.loaded = 0;
	}

	//Call reset to initialize everythign and put the PC at the program start
	GBA_reset();

	//If this is an elf file, let the ELFFile loader do the job.
	if (ELFFile_load(&gba.elfFile, gba.filename) != NULL) {
		gba.fileSize = gba.elfFile.fileSize;
		romSize = gba.elfFile.romSize;
		Stabs_init(&gba.stabs, &gba.elfFile);	//Initialize stabs
		
	} else {	//Otherwise....
		
		if (!(fp = fopen(gba.filename, "rb"))) 
			return 0;

		
		fseek (fp, 0, SEEK_SET);
		start = ftell (fp);
		fseek (fp, 0, SEEK_END);
		end = ftell (fp);
		
		gba.fileSize = (end - start)+1;

		fseek (fp, 0, SEEK_SET);

		//numROMPages = number of pages = filesize/64k, rounded up
		float temp = gba.fileSize;
		temp = temp/0x10000;
		temp = ceil (temp);
		u32 numROMPages = temp;

		//This makes sure that the ROM is allocated within a 64k boundary.
		//I am thinking that the GBA may actually store it's ROMs on some other boundary;
		//anybody care to confirm?
		romSize = numROMPages * 0x10000;
		if (! (gbaMem.u8ROM = new u8[ romSize ] ))	{ //Create enough memory for the file
		//if (! (gbaMem.u8ROM = new u8[gba.fileSize]) ) {
			gba.loaded = 0;
			return 0;
		}		
		gbaMem.u16ROM = (u16*) gbaMem.u8ROM;
		gbaMem.u32ROM = (u32*) gbaMem.u8ROM;

		fread (gbaMem.u8ROM, gba.fileSize, 1, fp);		//Load the file into this memory
		fclose(fp);								//Close the file.
		arm.programStart = 0x08000000;	//Set the arm to start in ROM.
		
		//Delete any elf/stabs that we may have loaded from
		//previous sessions.
		ELFFile_delete(&gba.elfFile);
		Stabs_delete(&gba.stabs);
		
	}

	//Set up our memory indexes.  Also set up mirror ROM.
	gbaMem.u8Mem[8] = gbaMem.u8Mem[0xA] = gbaMem.u8Mem[0xC] = gbaMem.u8ROM;
	//Now because CowBite divides memory up into 15 segments so as to allocate
	//only what it needs, we end up needing to split roms greater than 16 mb
	//accross two segments.
	gbaMem.u8Mem[9] = gbaMem.u8Mem[0xB] = gbaMem.u8Mem[0xD] = gbaMem.u8ROM + 0x1000000;

	//Record the size of each segment of rom.
	//Should I be using "romSize" instead of "fileSize"?  For now I use
	//fileSize (will make keeping tracks of illegal rom accesses more accurate)
	if (romSize <= 0x1000000) {
		gbaMem.memorySize[8] = gbaMem.memorySize[0xA] = gbaMem.memorySize[0xC] = romSize;
		gbaMem.memorySize[9] = gbaMem.memorySize[0xB] = gbaMem.memorySize[0xD] = 0;
	} else {
		//Note that I set the first half of rom to be the filsize.  This is
		//perfectly alright and will even facilitate dumping/loading.
		gbaMem.memorySize[8] = gbaMem.memorySize[0xA] = gbaMem.memorySize[0xC] = romSize;
		gbaMem.memorySize[9] = gbaMem.memorySize[0xB] = gbaMem.memorySize[0xD] = romSize - 0x1000000;
	} 

	
	
	//Find the path of the rom and save it for our use later.
	char* lastSlash = strrchr( gba.filename, '\\' );
	//Ok, if there is a last slash (or any slashes), we know that
	//a full path was specified.
	if (lastSlash != NULL) {
		int i;
		i = lastSlash - gba.filename;
			//For some reason the output varies depending on wehther you start from
			//the console or from windows.  Windows puts a " character in front.
		strcpy (gba.romPath, (gba.filename));
		gba.romPath[i+1] = '\0';
	}

	Console_print("CowBite:  Loaded ROM: ");
	Console_print(gba.filename);
	Console_print("\n");

	gba.loaded = 1;
	arm.numBreakpoints = 0;
	arm.numCondBreakpoints = 0;
	
	//GBA_reset();
	ARM_reset();
	
	return 1;

}



//////////////////////////////////////////////////////////////
//getState
//This gets the current state of the ARM and Memory,
//and can be used in saving and loading states
////////////////////////////////////////////////////////////
int GBA_getState(GBAState* state) {
	
	char* name;
	char delimiter = '\\'; //For UNIX this needs to be changed!!
	if (!gba.loaded)	//don't want retrieve memory that doesn't exist.
		return 0;

	//Cut the filename off of the path
	name = strrchr (gba.filename, delimiter) + 1;
	
	//If there was no delimiter,
	if ( !name )
		name = gba.filename;

	//Save the filename for future reference
	strcpy (state->filename, name);

//	state->frameTickTally = gbaIO.frameTickTally;
//	state->lastTally = gbaIO.lastTally;
//	for (int i = 4; i--;) {
//		state->timerStartValues[i] = gbaIO.timerStartValues[i];
//		state->timerTickTally[i] = gbaIO.timerTickTally[i];
//	}
	
	//All this function does is copy a bunch of variables
	//over.
	state->thumbMode = arm.thumbMode;
	state->instructionLength = arm.instructionLength;
	state->instructionWidth = arm.instructionWidth;
	
	state->currentInstruction = arm.currentInstruction;

	//Note how the state does not need to use pointers --
	//that's because unlike the actual ARM registers,
	//the state does not need to be able to redirect
	//registers.
	for (int i = 0; i<16; i++)	{
		state->r[i] = arm.r[i];
		state->r_gp[i] = arm.r_gp[i];
		state->r_fiq[i] = arm.r_fiq[i];
		state->r_svc[i] = arm.r_svc[i];
		state->r_abt[i] = arm.r_abt[i];
		state->r_irq[i] = arm.r_irq[i];
		state->r_und[i] = arm.r_und[i];
	}
	
	//Calculate the CPSR...
	arm.CPSR = arm.CPSR & 0x0FFFFFFF;	//Wipe out the sign bits
	//Update the CPSR (otherwise we get the incorrect value . . .man what a pain it is
	//having my flags be separate variables . . . but it's (slightly) faster...
	arm.CPSR = arm.CPSR | (arm.N << 31);		//If zero, set N
	arm.CPSR = arm.CPSR | (arm.Z << 30);		//If negative, set Z
	arm.CPSR = arm.CPSR | (arm.C << 29 );		//If carry						
	arm.CPSR = arm.CPSR | (arm.V << 28);		//If overflow

	state->CPSR = arm.CPSR;
	state->SPSR_fiq = arm.SPSR_fiq;
	state->SPSR_svc = arm.SPSR_svc;
	state->SPSR_abt = arm.SPSR_abt;
	state->SPSR_irq = arm.SPSR_irq;
	state->SPSR_und = arm.SPSR_und;

	state->halted = arm.halted;

	for (i = 0; i < 4; i++) {
		state->dmaDest[i] = gba.dmaDest[i];
		state->dmaSource[i] = gba.dmaSource[i];
	}

	//Now copy memory over
	memcpy (&state->gbaMem, &gbaMem, sizeof(GBAMem));
/*	memcpy (state->WRAMInt8bit, gbaMem.u8WRAMInt, WRAM_INT_SIZE);
	memcpy (state->WRAMExt8bit, gbaMem.u8WRAMExt, WRAM_EXT_SIZE);
	memcpy (state->pal8bit, gbaMem.u8Pal, PAL_SIZE);
	memcpy (state->VRAM8bit, gbaMem.u8VRAM, VRAM_SIZE);
	memcpy (state->OAM8bit, gbaMem.u8OAM, OAM_SIZE);
	memcpy (state->IORAM8bit, gbaMem.u8IORAM, IORAM_SIZE);
	memcpy (state->cartRAM8bit, gbaMem.u8CartRAM, CARTRAM_SIZE);
*/	//memcpy (state->screen, screen, SCREEN_SIZE);

	//Copy the stats over
	memcpy (&state->stats, &gba.stats, sizeof(GBAStats));

	return 1;
}



//////////////////////////////////////////////////////////////
//setState
//This sets the current state of the ARM and can be used in
//saving and loading states
////////////////////////////////////////////////////////////
int GBA_setState (GBAState* newState) {
	
//	gbaIO.frameTickTally = newState->frameTickTally;
//	gbaIO.lastTally = newState->lastTally;
//	for (int i = 4; i--;) {
//		gbaIO.timerStartValues[i] = newState->timerStartValues[i];
//		gbaIO.timerTickTally[i] = newState->timerTickTally[i];
//	}

	//Copy the stats over
	memcpy (&gba.stats, &newState->stats, sizeof(GBAStats));


	//All this function does is copy a bunch of variables
	//over.
	arm.thumbMode = newState->thumbMode;
	arm.instructionLength = newState->instructionLength;
		arm.instructionWidth = newState->instructionWidth;
	
	arm.currentInstruction = newState->currentInstruction;
	
	//Note how the state does not need to use pointers --
	//that's because unlike the actual ARM registers,
	//the state does not need to be able to redirect
	//registers.
	for (int i = 0; i<16; i++)	{
		arm.r_gp[i] = newState->r_gp[i];
		arm.r_fiq[i] = newState->r_fiq[i];
		arm.r_svc[i] = newState->r_svc[i];
		arm.r_abt[i] = newState->r_abt[i];
		arm.r_irq[i] = newState->r_irq[i];
		arm.r_und[i] = newState->r_und[i];

		arm.r[i] = newState->r[i];			
	}

	
	arm.CPSR = newState->CPSR;
	arm.SPSR_fiq = newState->SPSR_fiq;
	arm.SPSR_svc = newState->SPSR_svc;
	arm.SPSR_abt = newState->SPSR_abt;
	arm.SPSR_irq = newState->SPSR_irq;
	arm.SPSR_und = newState->SPSR_und;

	switch (arm.CPSR & 0xF) {
			case 0x01:		//fiq_26  26 bit PC FIQ Mode
							arm.SPSR_current = &arm.SPSR_fiq;
							break;
			case 0x02:		//irq_26  26 bit PC IRQ Mode
							arm.SPSR_current = &arm.SPSR_irq;
							break;
			case 0x03:		//svc_26  26 bit PC SVC Mode
							arm.SPSR_current = &arm.SPSR_svc;
							break;
			case 0x07:		//abt_26  26 bit PC Abt Mode
							arm.SPSR_current = &arm.SPSR_abt;
							break;
			case 0x0D:		//und_25  26 bit PC Und Mode
							arm.SPSR_current = &arm.SPSR_und;
							break;
			default:		break;
	}


	arm.halted = newState->halted;

	//Get the flags
	arm.N = arm.CPSR >> 31;
	arm.Z = (arm.CPSR >> 30) & 0x1;
	arm.C = (arm.CPSR >> 29) & 0x1;
	arm.V = (arm.CPSR >> 28) & 0x1;
	
	for (i = 0; i < 4; i++) {
		gba.dmaDest[i] = newState->dmaDest[i];
		gba.dmaSource[i] = newState->dmaSource[i];
	}


	//Now copy memory over
	memcpy (gbaMem.u8WRAMInt, newState->gbaMem.u8WRAMInt, WRAM_INT_SIZE);
	memcpy (gbaMem.u8WRAMExt, newState->gbaMem.u8WRAMExt, WRAM_EXT_SIZE);
	memcpy (gbaMem.u8Pal, newState->gbaMem.u8Pal, PAL_SIZE);
	memcpy (gbaMem.u8VRAM, newState->gbaMem.u8VRAM, VRAM_SIZE);
	memcpy (gbaMem.u8OAM, newState->gbaMem.u8OAM, OAM_SIZE);
	memcpy (gbaMem.u8IORAM, newState->gbaMem.u8IORAM, IORAM_SIZE);
	memcpy (gbaMem.u8CartRAM, newState->gbaMem.u8CartRAM, CARTRAM_SIZE);
	//memcpy (screen, newState->screen, SCREEN_SIZE);
	//graphics.spritesDirty = 1;
	//Reset the last location to something good.
	//location = *r[15];

	return 1;
}//setstate.

////////////////////////////////////
//saveState
//Saves the current state to a numbered filename.
//////////////////////////////////////
void GBA_saveState (int state) {
	char filename[256];
	char *temp;
	char delimiter = '\\';	//Must change for UNIX

	//Cut the filename off of the path
	temp = strrchr (gba.filename, delimiter) + 1;

	//If there is no path delimiter
	if (temp == NULL)
		temp = gba.filename;

	//Copy it to our string buffer
	strcpy(filename, temp);

	//Cut off everything after the first '.'
	temp = strrchr (filename, '.');
		
	if (temp != NULL)
		*temp = '\0';	//Set this to be the null terminator
		
	//Save a file of the name "<name>State
	sprintf (filename, "%s.ga%d", filename, state);
	GBA_saveState(filename);
}


///////////////////////////////////
//saveState()
//This saves the current state to a specific
//filename
//////////////////////////////////
void GBA_saveState (char* filename) {
	FILE *file;
	GBAState* gbaState;

	if (!gba.loaded)
		return;
	
	if (filename == NULL)
		return;

	gbaState = new GBAState;
	
	//fill the state variable
	GBA_getState (gbaState);


	//This opens the file, creating it if it
	//doesn't exist.
	if (!(file = fopen(filename, "wb")))
		return;

	
	fwrite (gbaState, 1, sizeof(GBAState), file);		//Save the file
	fclose(file);										//Close the file.
	
	//Free memory.
	delete gbaState;
}//saveState

////////////////////////////////////
//loadState
//Loads the current state to a numbered filename.
//////////////////////////////////////
void GBA_loadState (int state) {
	char filename[256];
	char* temp;
	char delimiter = '\\';	//Must change for UNIX
	
	//Cut the filename off of the path
	temp = strrchr (gba.filename, delimiter) + 1;

	//If there is no path delimiter
	if (temp == NULL)
		temp = gba.filename;

	//Copy it to our string buffer
	strcpy(filename, temp);

	//Cut off everything after the first '.'
	temp = strrchr (filename, '.');
		
	if (temp != NULL)
		*temp = '\0';	//Set this to be the null terminator
		
	//Save a file of the name "<name>State
	sprintf (filename, "%s.ga%d", filename, state);
	GBA_loadState(filename);
}

///////////////////////////////////////
//loadState(int state)
//This loads the current state from a numbered file or to specific
//filename, if provided.
//////////////////////////////////
void GBA_loadState (char* filename) {
	FILE *file;
	GBAState* gbaState;
	char delimiter = '\\'; //For UNIX this needs to be changed!!

	if (!gba.loaded)
		return;

	if (filename == NULL)
		return;

	gbaState = new GBAState;
	
	//This opens the file, exiting if it doesn't exist.
	if (!(file = fopen(filename, "rb"))) 
		return;


	fseek (file, 0, SEEK_SET);
	
	fread (gbaState, 1, sizeof (GBAState), file);		//Load the file into this memory
	fclose(file);								//Close the file.
	
	GBA_setState (gbaState);
	delete gbaState;
}//loadState

///////////////////////////////////////////////
//reset()
//Resets the GBA to the null state
///////////////////////////////////////////////
void GBA_reset() {
	GBA_setState(&gba.nullState);
	if (gba.loaded)
		ARM_reset();

}

///////////////////////////////////////////////////////
//hardwareInterrupt
//This function emulates a hardware interrupt.  Although a true-to-life
//emulation would 
//  1.  Switches state to IRQ mode, bank-swapping the current stack register and link
//		register (thus preserving their old values)
//	2.  Saves the address of the next instruction in LR_fiq
//		compensating for Thumb/ARM depending on the mode you are in.
//	3.  Switches to ARM mode, executes code in BIOS at a hardware interrupt vector 
//		(which you never see)
//And then pick up execution in the BIOS, CowBite has a very limited BIOS,
//just enough to be able to save the registers and branch to the address 
//at 0x03007FFC.
//////////////////////////////////////////////////////////////////////
void GBA_hardwareInterrupt(u32 interruptFlag) {
	//Console_print("Entered interrupt function.\n");
	if ( (*(REG_IME) == 0) || (arm.CPSR & 0x80))	//If interrupts are disabled, ignore.
		return;
	//Console_print("Passed Interupt Enable.\n");
	//If the specified interrupt was enabled...
	if (*(REG_IE) & interruptFlag) {
		*(REG_IF) = *(REG_IF) | interruptFlag;	//Set the flag
	
	//Log this interrupt in our interrupt log.
	//No bounds checking since this function wouldn't get called if there
	//were no flags set.
	for (int i = 0; !(interruptFlag & 0x1); i++) {
		interruptFlag = interruptFlag >> 1;
	}
	gba.stats.interrupts[i]++;
	gba.stats.frameInterrupts[i]++;
	gba.stats.lastInterrupt[i] = arm.r[15];

	//	sprintf(console.tempBuf, "Interrupt at line: %d, %d\n", graphics.lcy, gbaMem.u8IORAM[0x6]);
	//	Console_print(console.tempBuf);

		//Switch from the current mode to a new mode
		u32 oldCPSR = arm.CPSR;
		u32 currentMode = arm.CPSR & 0x1F;
		ARM_changeCPSRState (currentMode, 0x12);	//Change to IRQ mode
		*arm.SPSR_current = oldCPSR;
		
		//Set the i-bit.
		arm.CPSR = arm.CPSR | 0x80;

		//Debug
		//if (arm.r[15] & 0x1)
		//	int blah = 0;

		//if (graphics.lcy == 0x1C)
		//	int blah = 0;
	
		if (arm.halted) {	//If we were halted at the time this interupt occurs
			//arm.halted = 2;
			
			arm.halted = 0;
			if (arm.thumbMode)	//Move on to the next instruction
				ARM_nextInstructionThumb();
			else
				ARM_nextInstruction();
		}

		//Set LR such that subs pc, lr, #4 can return us to our previous state.
		if (arm.thumbMode) {
			//temp2 = arm.r[15];
			arm.r[14] = arm.r[15];	//Thumb mode
		} else {
			arm.r[14] = arm.r[15]-4;	//Set to the next instruction
		}
		
		//DEBUG
		//temp3 = arm.r[0xE];

		arm.thumbMode = 0;	//Go into ARM mode.
		arm.CPSR = arm.CPSR & 0xFFFFFFDF;

		//Finally branch to IRQ vector 0x00000018
		arm.r[15] = 0x00000018;
		ARM_skipInstruction();
	}
}

/////////////////////////////////////////////////////
//debugHDraw()
//Carries out processing that occurs during hdraw
/////////////////////////////////////////////////////
void GBA_debugHDraw() {
		
	//Run through hDraw.  The algorithm here is to run through a few
	//clock cycles, update the clock, run through a few more cycles,
	//update the clock, etc.  In certain cases user code will access
	//the clocks; in these instances we update the clock earlier to
	//reflect the new value.  In order to keep this from screwing us up,
	//we need to tally how many ticks passed since the last update.
	
	//If we are in hblank (not hdraw), return.
	if ( *(REG_STAT) & 0x2 )	
		return;
	
	//Note that an update of 44.1 khz would occur every 380 ticks.
	//I want FIFO interrupts to be able to update at at least this rate.
	u32 beginCycles = gba.hDrawBeginCycles[graphics.lcy];
			
	for (int segment = 0; segment < 3; segment++) {
		//This loop will repeat until we return to the place we left off
		//at.  We use a lookup table to see how many cycles we have at each interval
		u32 endTally = gba.hDrawSegmentCycles[segment] + beginCycles;
		while (gba.stats.frameTickTally < endTally) {
			gba.stats.frameTickTally += ARM_execute();
			if (arm.paused) { 	//If we reached a breakpoint, cease execution.
				IO_updateTimers();
				return;
			}
		}
		IO_updateTimers();
	}

	/*	//1004 - gba.stats.lineTickTally;
		//u32 numIncrements = ticksLeft/251
		while (ticksLeft > 0) {
			//Run in 251 tick increments
			u32 endTally = gba.stats.frameTickTally + __min(ticksLeft, 251);
			ticksLeft = __max (ticksLeft-251,0);
			
			while ( gba.stats.frameTickTally < endTally) {
				 
				if (arm.paused) { 	//If we reached a breakpoint, cease execution.
					IO_updateTimers();
					return;
				}
			}
			IO_updateTimers();
		}
		gba.stats.lineTickTally = 0;*/
	/*} else {						//If we are starting from the beginning of the line
		for (int i = 4; i--;) {	//4 * 251 = 1004
			//Note that an update of 44.1 khz would occur every 380 ticks.
			u32 endTally = gba.stats.frameTickTally + 251;
			while ( gba.stats.frameTickTally < endTally) {
				//ARM_execute();              //execute instruction
				gba.stats.frameTickTally += ARM_execute(); 
				if (arm.paused) {
					IO_updateTimers();
					return;
				}
			}
			IO_updateTimers();	//Update the timers every 251 ticks
		}
		
	}*/
	
	
	*(REG_STAT) |= 0x2;	//Tell Reg stat that we're in hblank now

	//Execute hdma 0...
	//if ((*REG_STAT & 0x1) == 0) {
		
	//}
	
	//Hardware testing by gbcft shows that hblank interrupts occur during vblank
	if (*(REG_STAT) & 0x10) {
			GBA_hardwareInterrupt(INTERRUPT_H);
	}
}//debugHDraw

/////////////////////////////////////////////////////
//debugHBlank()
//Carries out processing that occurs during hblank.
/////////////////////////////////////////////////////
void GBA_debugHBlank() {
	u32 endTally = gba.hBlankEndCycles[graphics.lcy];

	/*if (gba.stats.lineTickTally) {	//If we reached breakpoint in the middle of a line
		endTally = gba.stats.frameTickTally + 228 - (gba.stats.lineTickTally - 1008);
		gba.stats.lineTickTally = 0;
	}
	else
		endTally = gba.stats.frameTickTally + 228;*/

	//Run through hblank
	while (gba.stats.frameTickTally < endTally) {
		gba.stats.frameTickTally += ARM_execute(); 
		if (arm.paused)	{//If we reached a breakpoint, get out.
			IO_updateTimers();
			return;
		}
	}
	IO_updateTimers();
	

	*(REG_STAT) &= 0xFFFFFFFFD;
	
	//Check if there's a VCOUNT interrupt.
	if (*REG_STAT & 0x20) {
		if ((*REG_STAT >> 8) == graphics.lcy) {
			GBA_hardwareInterrupt(INTERRUPT_VCOUNT);
		}
	}
}//debugHBlank()

///////////////////////////////////////////////////////
//debugVDraw()
//Runs the cpu in debug mode, drawing the screen...the tricky part is
//that the debugger should be able to resume execution of the gba
//at any point, including right in the middle of vdraw.  The code
//must be written in a way that lets us determine where we last left off.
/////////////////////////////////////////////////////
void GBA_debugVDraw() {
	
//	QueryPerformanceCounter (&gba.lastTime64);

	//Make both sprites and backgrounds dirty to force at least one update
	//per refresh.
	graphics.spritesDirty = graphics.backgroundsDirty = 1;
				
	//If we are in vblank (not vdraw), return
	if ( *(REG_STAT) & 0x1 )	
		return;

	for (graphics.lcy; graphics.lcy<160; graphics.lcy++) {
		
		//Must update the register for programs that want to check lcy...
		gbaMem.u8IORAM[0x6]=graphics.lcy;		

		//We don't want there be two vcount interrupts if the debugger
		//reaches two breakpoints on a line.

		GBA_debugHDraw();
		Graphics_renderLine();

		//Note that hdma ONLY occurs during vdraw.
		if ( (*(REG_DMA0CNT) & 0x3000) == 0x2000) 	//If set to transfer on hblank
				IO_dmaTransfer (0);
		if ((*(REG_DMA1CNT) & 0x3000) == 0x2000)
				IO_dmaTransfer (1);
		if ((*(REG_DMA2CNT) & 0x3000) == 0x2000)			
				IO_dmaTransfer (2);
		if ((*(REG_DMA3CNT) & 0x3000) == 0x2000)
				IO_dmaTransfer (3);

		if (arm.paused)	//If a breakpoint was reached during hdraw, stop.
			return;

		GBA_debugHBlank();

		if (arm.paused)	//If a breakpoint was reached during hlank, stop.
			return;
	}

	
	//Now that we are at the end of VDraw, set REG_STAT to say that we're in vblank,
	//perform DMA transfers, and start any vlank interrupts.  THe reason I put this
	//at the end of the vdraw function instead of at the beginning of the vblank function
	//is that after the last instruction of vdraw is executed, the person debugging
	//would probably expect to see the cursor move to the first instruction
	//of the interrupt routine, and for the flag indicate vblank.
	//In this implementation it will appear that the DMA transfers have already
	//ocurred before the interrupt was reached (as opposed to happening when
	//the programmer hits "step" after the first instruction of vblank)
	*(REG_STAT) |= 0x1;	//Reg stat

	if ((*(REG_DMA0CNT) & 0x3000) == 0x1000) 	//If set to transfer on vblank
		IO_dmaTransfer (0);
	if ((*(REG_DMA1CNT) & 0x3000) == 0x1000)
		IO_dmaTransfer (1);
	if ((*(REG_DMA2CNT) & 0x3000) == 0x1000)			
		IO_dmaTransfer (2);
	if ((*(REG_DMA3CNT) & 0x3000) == 0x1000)
		IO_dmaTransfer (3);

		//Vblank interrupt
	if (*(REG_STAT) & 0x8) {
			GBA_hardwareInterrupt(INTERRUPT_V);
	}
}


///////////////////////////////////////////////////////////////////
//debugVBlank
//Debugs the CPU through the VBlank period
//////////////////////////////////////////////////////////////////
void GBA_debugVBlank() {
	if (arm.paused)	//If we paused execution, don't execute any of this.
		return;			
	
	
	for ( graphics.lcy; graphics.lcy < 228; graphics.lcy++ ) {
		gbaMem.u8IORAM[0x6]=graphics.lcy;
		
		GBA_debugHDraw();
		if (arm.paused)	//If a breakpoint was reached during hdraw, stop.
			return;
		
		GBA_debugHBlank();
		if (arm.paused)	//If a breakpoint was reached during hdraw, stop.
			return;
	}

	GBA_sync();
	//Note that we should always play the audio immediately after syncing
	if ( audio.frameCount < (audio.framesPerUpdate-1)) {
		audio.frameCount++;
	} else {
		Audio_playSound();
		audio.frameCount = 0;
		//char text[64];
		//sprintf(text, "\nTally: %d", gba.stats.frameTally);
		//Console_print(text);
	}
	//(gba.stats.frameTally & audio.updateMask) == audio.updateMask)
		

	
	gbaMem.u8IORAM[0x6] = 0;	//Reg Vcount

	//Reset our tally of ticks.  Note that I do this at the *end* of the frame
	//rather than the beginning.  This is mostly a matter of preference, but I think
	//it will also make maintaining the debugger easier.
	gba.stats.frameTickTally -= 280896;
	gba.stats.frameTally++;
	gba.stats.lastTally = 0;
	gba.stats.frameDMATransfers[0] = gba.stats.frameDMATransfers[1] = gba.stats.frameDMATransfers[2] = gba.stats.frameDMATransfers[3] = 0;
	for (int i = 16; i--;)
		gba.stats.frameInterrupts[i] = 0;

	*(REG_STAT) &= 0xFFFFFFFFE;
	
	graphics.lcy = 0;
}



//////////////////////////////////
//This syncs cowbite either using a high resolution clock
//or, if sound is running, by syncing to the play cursor
void GBA_sync() {
	if (graphics.framesLeft <=0) {
		graphics.framesLeft = graphics.framesPerRefresh;
		if (!Audio_sync()) {
			QueryPerformanceCounter(&gba.currentTime64);
			//gba.currentTime = clock();
				
			//Sync to the system clock
			while ( (gba.currentTime64.QuadPart - gba.lastTime64.QuadPart) < (gba.displayInterval*graphics.framesPerRefresh)) {
				QueryPerformanceCounter(&gba.currentTime64);
			}
				
		}
		QueryPerformanceCounter(&gba.lastTime64);	//Start timer count
	} 
	graphics.framesLeft--;
	
	
}



