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

#ifndef IO_H
#define IO_H

#include "Constants.h"

#define INTERRUPT_V 0x1
#define INTERRUPT_H 0x2
#define INTERRUPT_VCOUNT 0x4
#define INTERRUPT_TIMER0 0x8
#define INTERRUPT_TIMER1 0x10
#define INTERRUPT_TIMER2 0x20
#define INTERRUPT_TIMER3 0x40
#define INTERRUPT_SERIAL 0x80
#define INTERRUPT_DMA0	0x100
#define INTERRUPT_DMA1 0x200
#define INTERRUPT_DMA2 0x400
#define INTERRUPT_DMA3 0x800
#define INTERRUPT_KEY	0x1000
#define INTERRUPT_CASSETTE 0x2000

#define IO_LCDREG 0x4000000
#define IO_SOUNDCNT_X 0x4000084
#define IO_DMA0SRC 0x40000B0
#define IO_DMA0DST 0x40000B4
#define IO_DMA0COUNT 0x40000B8
#define IO_DMA0CNT 0x40000BA
#define IO_DMA1SRC 0x40000BC
#define IO_DMA1DST 0x40000C0
#define IO_DMA1COUNT 0x40000C4
#define IO_DMA1CNT 0x40000C6
#define IO_DMA2SRC 0x40000C8
#define IO_DMA2DST 0x40000CC
#define IO_DMA2COUNT 0x40000D0
#define IO_DMA2CNT 0x40000D2
#define IO_DMA3SRC 0x40000D4
#define IO_DMA3DST 0x40000D8
#define IO_DMA3COUNT 0x40000DC
#define IO_DMA3CNT 0x40000DE
#define IO_TM0DAT 0x4000100 
#define IO_TM0CNT 0x4000102 
#define IO_TM1DAT 0x4000104 
#define IO_TM1CNT 0x4000106 
#define IO_TM2DAT 0x4000108 
#define IO_TM2CNT 0x400010A 
#define IO_TM3DAT 0x400010C 
#define IO_TM3CNT 0x400010E 

#define IO_SOUND1CNT   0x4000060	//sound 1
#define IO_SOUND1CNT_L 0x4000060	//
#define IO_SOUND1CNT_H 0x4000062	//
#define IO_SOUND1CNT_X 0x4000064	//

#define IO_SOUND2COUNT 0x4000068		//sound 2 lenght & wave duty
#define IO_SOUND2CNT 0x400006C		//sound 2 frequency+loop+reset

#define IO_SG30       0x4000070		//???
#define IO_SOUND3CNT  0x4000070		//???
#define IO_SG30_L     0x4000070		//???
#define IO_SOUND3CNT_L 0x4000070	//???
#define IO_SG30_H     0x4000072		//???
#define IO_SOUND3CNT_H 0x4000072	//???
#define IO_SG31       0x4000074		//???
#define IO_SOUND3CNT_X 0x4000074	//???

#define IO_SOUND4CNT_L 0x4000078		//???
#define IO_SOUND4CNT_H 0x400007C		//???

#define IO_SGCNT0     0x4000080		
#define IO_SGCNT0_L   0x4000080		
#define IO_SOUNDCNT   0x4000080
#define IO_SOUNDCNT_L 0x4000080		//DMG sound control

#define IO_SGCNT0_H   0x4000082		
#define IO_SOUNDCNT_H 0x4000082		//Direct sound control

#define IO_SGCNT1     0x4000084		
#define IO_SOUNDCNT_X 0x4000084	    //Extended sound control


#define REG_DSPCNT (u16*)(&(gbaMem.u8IORAM[0x00000000]))
#define REG_STAT (u16*)(&(gbaMem.u8IORAM[0x00000004]))

#define REG_FIFO_A ((u8*)(&(gbaMem.u8IORAM[0x0A0])))
#define REG_FIFO_B ((u8*)(&(gbaMem.u8IORAM[0x0A4])))

#define REG_SOUNDCNT_H (u16*)(&(gbaMem.u8IORAM[0x082]))
#define REG_SOUNDCNT_X (u16*)(&(gbaMem.u8IORAM[0x084]))

#define REG_VCOUNT (u16*)(&(gbaMem.u8IORAM[0x00000006]))
#define REG_DMA0SRC (&(gbaMem.u32IORAM[0x2C]))
#define REG_DMA0DST (&(gbaMem.u32IORAM[0x2D]))
#define REG_DMA0COUNT (u16*)(&(gbaMem.u8IORAM[0xB8]))
#define REG_DMA0CNT (u16*)(&(gbaMem.u8IORAM[0xBA]))
#define REG_DMA1SRC (&(gbaMem.u32IORAM[0x2F]))
#define REG_DMA1DST (&(gbaMem.u32IORAM[0x30]))
#define REG_DMA1COUNT (u16*)(&(gbaMem.u8IORAM[0xC4]))
#define REG_DMA1CNT (u16*)(&(gbaMem.u8IORAM[0xC6]))
#define REG_DMA2SRC (&(gbaMem.u32IORAM[0x32]))
#define REG_DMA2DST (&(gbaMem.u32IORAM[0x33]))
#define REG_DMA2COUNT (u16*)(&(gbaMem.u8IORAM[0xD0]))
#define REG_DMA2CNT (u16*)(&(gbaMem.u8IORAM[0xD2]))
#define REG_DMA3SRC (&(gbaMem.u32IORAM[0x35]))
#define REG_DMA3DST (&(gbaMem.u32IORAM[0x36]))
#define REG_DMA3COUNT (u16*)(&(gbaMem.u8IORAM[0xDC]))
#define REG_DMA3CNT (u16*)(&(gbaMem.u8IORAM[0xDE]))
#define REG_TM0DAT (u16*)(&(gbaMem.u8IORAM[0x100]))
#define REG_TM1DAT (u16*)(&(gbaMem.u8IORAM[0x104]))
#define REG_TM2DAT (u16*)(&(gbaMem.u8IORAM[0x108]))
#define REG_TM3DAT (u16*)(&(gbaMem.u8IORAM[0x10C]))
#define REG_TM0CNT (u16*)(&(gbaMem.u8IORAM[0x102]))
#define REG_TM1CNT (u16*)(&(gbaMem.u8IORAM[0x106]))
#define REG_TM2CNT (u16*)(&(gbaMem.u8IORAM[0x10A]))
#define REG_TM3CNT (u16*)(&(gbaMem.u8IORAM[0x10E]))

#define REG_IE (u16*)(&(gbaMem.u8IORAM[0x00000200]))
#define REG_IF (u16*)(&(gbaMem.u8IORAM[0x00000202]))
#define REG_IME (u16*)(&(gbaMem.u8IORAM[0x00000208]))
#define REG_EMU (u16*)(&(gbaMem.u8IORAM[0x00000302]))
#define REG_COWBITE (u16*)(&(gbaMem.u8IORAM[0x00000304]))


//////////////////////////////////////////////////////
//IO Operations
///////////////////////////////////
void IO_init();
void IO_updateReg(u32 address);
int IO_checkForUpdate (u32 address, u16 newValue);
void IO_dmaTransfer (u32 dmaNum);
void IO_updateTimers();
void IO_setUp (u32 setVal);
void IO_setDown (u32 setVal);
void IO_setLeft (u32 setVal);
void IO_setRight (u32 setVal);
void IO_setA (u32 setVal);
void IO_setB (u32 setVal);
void IO_setL (u32 setVal);
void IO_setR (u32 setVal);
void IO_setStart (u32 setVal);
void IO_setSelect(u32 setVal);

//IO data
extern u16* IO_keyInput;

//Struct
typedef struct tGBAIO {
/*	u32 frameTickTally;		//Number of ticks from the start of this frame
	u32 lastTally;
	u32 timerStartValues[4];	//Holds the start value of all the timers.
	u32 timerTickTally[4];	//A tally of ticks for lower-frequency timers
	u32 lineTickTally;		//Number of ticks from the start of this line
*/	
} GBAIO;

extern GBAIO gbaIO;



#endif