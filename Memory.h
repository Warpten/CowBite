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

#ifndef MEMORY_H
#define MEMORY_H

#define MEMORY_NUMSECTIONS 0xF

#define ZERO_PAGE_SIZE 255		
#define ZERO_PAGE_START 0x0

#define WRAM_EXT_SIZE 0x40000	//256 k
#define WRAM_EXT_START 0x02000000

#define WRAM_INT_SIZE 0x8000	//32 k
#define WRAM_INT_START 0x03000000

#define IORAM_SIZE 0x400		//1 k
#define IORAM_START 0x04000000

#define PAL_SIZE 0x400			//1k
#define PAL_START 0x05000000

#define VRAM_SIZE 0x20000		//128k
#define VRAM_START 0x06000000

#define OAM_SIZE 0x400			//1k
#define OAM_START 0x07000000

#define ROM_START 0x08000000
#define ROM1_START 0x0A000000
#define ROM2_START 0x0C000000

#define CARTRAM_START 0x0E000000
#define CARTRAM_SIZE 0x10000	//64 k

typedef struct tGBAMem {
	//This is a funny little pointer for accessing different
	//portions of memory based on the address
	u8* u8Mem[MEMORY_NUMSECTIONS];		u16** u16Mem;	u32** u32Mem;
	u32 memorySize[0x100];

	//This contains the rom memory as a contiguous array
	u8 *u8ROM;			u16 *u16ROM;		u32 *u32ROM;

	u8 u8WRAMInt[WRAM_INT_SIZE];		u16 *u16WRAMInt;	u32 *u32WRAMInt;
	u8 u8WRAMExt[WRAM_EXT_SIZE];		u16 *u16WRAMExt;	u32 *u32WRAMExt;
	u8 u8Pal[PAL_SIZE];					u16 *u16Pal;		u32 *u32Pal;
	
	u8 u8VRAM[VRAM_SIZE];				u16 *u16VRAM;		u32 *u32VRAM;
	u8 u8OAM[OAM_SIZE];					u16 *u16OAM;		u32 *u32OAM;
	u8 u8IORAM[IORAM_SIZE];				u16 *u16IORAM;		u32 *u32IORAM;
	u8 u8BIOS[ZERO_PAGE_SIZE];			u16 *u16BIOS;		u32 *u32BIOS;
	u8 u8CartRAM[CARTRAM_SIZE];			u16 *u16CartRAM;	u32 *u32CartRAM;
	u8 *scratchMem;


	//This is a table of function pointers to switch off depending on what
	//area of memory was written to.and what the previous value of this memory was.
	void  (*writeByteAccessTable[0x100])(u32 address, u8* dest, u8 data);
	void  (*writeHalfWordAccessTable[0x100])(u32 address, u16* dest, u16 data);
	void  (*writeWordAccessTable[0x100])(u32 address, u32* dest, u32 data);
} GBAMem;

extern GBAMem gbaMem;



///////////////////////////////////////////////////////
//Memory Operations
extern void Memory_init();
extern void Memory_delete();
extern u8 Memory_readByte(u32 address);
extern u32 Memory_readWord (u32 address);
extern u16 Memory_readHalfWord(u32 address);
extern void Memory_writeWord (u32 address, u32 data);
extern void Memory_writeByte (u32 address, u8 data);
extern void Memory_writeHalfWord (u32 address, u16 data);

///////////////////////////////////////
//functions for writing to various places in memory.
///////////////////////////////////////
void Memory_writeByteStandardMemory(u32 address, u8* dest, u8 data);
void Memory_writeByteIORAM(u32 address, u8* dest, u8 data);
void Memory_writeByteBIOS(u32 address, u8* dest, u8 data);
void Memory_writeByteROM0(u32 address, u8* dest, u8 data);
void Memory_writeByteROM1(u32 address, u8* dest, u8 data);
void Memory_writeByteROM2(u32 address, u8* dest, u8 data);

void Memory_writeHalfWordStandardMemory(u32 address, u16* dest, u16 data);
void Memory_writeHalfWordIORAM(u32 address, u16* dest, u16 data);
void Memory_writeHalfWordBIOS(u32 address, u16* dest, u16 data);
void Memory_writeHalfWordROM0(u32 address, u16* dest, u16 data);
void Memory_writeHalfWordROM1(u32 address, u16* dest, u16 data);
void Memory_writeHalfWordROM2(u32 address, u16* dest, u16 data);

void Memory_writeWordStandardMemory(u32 address, u32* dest, u32 data);
void Memory_writeWordIORAM(u32 address, u32* dest, u32 data);
void Memory_writeWordBIOS(u32 address, u32* dest, u32 data);
void Memory_writeWordROM0(u32 address, u32* dest, u32 data);
void Memory_writeWordROM1(u32 address, u32* dest, u32 data);
void Memory_writeWordROM2(u32 address, u32* dest, u32 data);


#endif