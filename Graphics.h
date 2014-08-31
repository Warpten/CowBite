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

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <memory.h>
#include <stdlib.h>
#include "Support.h"

#define SCREEN_SIZE 0x12C00	//240 x 160 x 2 bytes
#define SCREEN_ENTRIES 0x9600	//240 x 160 entries

//Define pointers to some ares in graphics and oam memory.
//#define GRAPHICS_OAM		((OAMInfo*)gbaMem.u8OAM)
//#define GRAPHICS_OAMRS		((OAMRSInfo*)gbaMem.u8OAM)


#define GRAPHICS_OBJBUFFERENTRIES	0x19E00//(240 + 128) * (160 + 128)	= 368 x 288 = 0x19E00;

//The VRAM is termed A (bottom 64K) B (next 16K)
//and C (top 16K). Sprites go into B or C. 
//

#define BG0_INFO       (u16*)(gbaMem.u8IORAM + 0x8) //Background 0 Control register
#define BG0_SCX	     (u16*)(gbaMem.u8IORAM + 0x10) //Background 0 H-Offset
#define BG0_SCY	     (u16*)(gbaMem.u8IORAM + 0x12) //Background 0 V-Offset
#define BG1_INFO       (u16*)(gbaMem.u8IORAM + 0xA) //Background 1 Control register
#define BG1_SCX	     (u16*)(gbaMem.u8IORAM + 0x14) //Background 1 H-Offset
#define BG1_SCY	     (u16*)(gbaMem.u8IORAM + 0x16) //Background 1 V-Offset
#define BG2_INFO       (u16*)(gbaMem.u8IORAM + 0xC) //Background 2 Control register
#define BG2_SCX	     (u16*)(gbaMem.u8IORAM + 0x18) //Background 2 H-Offset
#define BG2_SCY	     (u16*)(gbaMem.u8IORAM + 0x1A) //Background 2 V-Offset
#define BG3_INFO       (u16*)(gbaMem.u8IORAM + 0xE) //Background 3 Control register
#define BG3_SCX	     (u16*)(gbaMem.u8IORAM + 0x1C) //Background 3 H-Offset
#define BG3_SCY	     (u16*)(gbaMem.u8IORAM + 0x1E) //Background 3 V-Offset

#define REG_BLDCNT		(u16*)(gbaMem.u8IORAM + 0x50)
#define REG_BLDALPHA	(u16*)(gbaMem.u8IORAM + 0x52)
#define REG_BLDY		(u16*)(gbaMem.u8IORAM + 0x54)
#define REG_WIN0_H	(u16*)(&gbaMem.u8IORAM[0x40])
#define REG_WIN1_H	(u16*)(&gbaMem.u8IORAM[0x42])
#define REG_WIN0_V	(u16*)(&gbaMem.u8IORAM[0x44])
#define REG_WIN1_V	(u16*)(&gbaMem.u8IORAM[0x46])
#define REG_WIN_IN	(u16*)(&gbaMem.u8IORAM[0x48])
#define REG_WIN_OUT	(u16*)(&gbaMem.u8IORAM[0x4A])
//#define REG_VCOUNT	(u16*)(&gbaMem.u8IORAM[0x6]);



#define TILE_FLIPX 0x0400
#define TILE_FLIPY 0x0800


#define BGTYPE_ROTSCALE 0x1
#define BGTYPE_TEXT 0x2
#define BGTYPE_BITMAP 0x4


//A structure for accessing OAM data
//This is exactly what is used in the sprite demo.
typedef struct tOAMInfo { 
	u16 attribute0;
	u16 attribute1; 
	u16 attribute2; 
	u16 rotateScale; 
} OAMInfo;

//A structure for accessing rotate/scale data
typedef struct tOAMRSInfo {
	u16 filler1[3];
	s16 DX;
	u16 filler2[3];
	s16 DMX;            
	u16 filler3[3];
	s16 DY; 
	u16 filler4[3];
	s16 DMY;
} OAMRSInfo;

//A structure for the sprite info.
//Used to store data that is not likely to change between scanlines--
//such as the base (non scaled) width and height.
typedef struct tSpriteInfo {
	OAMInfo* oamInfo;
	OAMRSInfo* oamRSInfo;
	int x;
	int y;
	int width;
	int height;
	int effectiveWidth;
	int effectiveHeight;
	u32 xTiles;	//How many tiles across
	u32 yTiles;	//How many tiles down
	u32 index;	//
	u32 pal256;	//If it uses a 256 color palette.
	u32 palNum;	//Which of the 16 palettes to use (16 color sprites only)
	u32 rotScale;	//If we're using rotate and scale
	u32 visible;
	u8 bitmap[4096];	//64 * 64 - maximum sprite size.
} SpriteInfo;

//A structure for the background info.
typedef struct tBackgroundInfo {
	u16* info;	//location of background's info
	u16* scx;
	u16* scy;
	u16* DY;
	u16* DMX;
	u16* DX;
	u16* DMY;
	u32* X;
	u32* Y;
	u32 bgNum;		//Self explanatory?
	int width;
	int height;
	u32 xTiles;
	u32 yTiles;
	u32 sizeCode;
	u32 xBitWidth;	//The number of x tiles as a power of 2
	u32 pal256;		//If it uses a 256 color palette
	u32 rotScale;	//If we're using rotate and scale...
	u32 type;
	u16* tileMap;
	u8* tileData;
} BackgroundInfo;

//A structure for maintaining data
//at each of the different priority levels
typedef struct tPriority {
	int visible;			//A flag to let the priority level be turned off in the UI
	int numSprites;			//The number of sprites at this priority level.
	int numBGs;				//The number of backgrounds at this priority
	SpriteInfo sprites[128] ;		//An array of sprites, each of which has a pointer to OAM
	BackgroundInfo bgInfo[4];
	//u16* backgroundInfo;	//An pointer to the background graphics at this priority level
	//u16* scx;			//A pointer to the scroll value for this background.
	//u16* scy;
	//u16 spriteBuffer[GRAPHICS_OBJBUFFERENTRIES];		//A buffer on which to draw sprites when not using hblank obj processing
	//u8 dirtyPixels[GRAPHICS_OBJBUFFERENTRIES];		//This lets us know which pixels are set, which are not.
							//0 = not set
							//1 = set
							//2 = alpha blended.
	
	//u8 dirtyLines[160];		//This lets us know which scanlines are dirty, which are not.
} Priority;

typedef struct tBlendTarget {
	u32 dirty;
	u16 buffer[240];
} BlendTarget;

typedef struct tBlend {
	u16 bldCnt;
	u16 bldAlpha;
	u16 bldY;
	u16 sourceMul;
	u16 destMul;
	u32 mulOffset;	//Index into LUTs.
	u32 bldType;
	u32 enabled;
	u32* blendArray;//[240];
} Blend;

typedef struct tWindow {
	u32 winLeft;
	u32 winRight;
	u32 winTop;
	u32 winBottom;
	u32 winWidth;
	u32 winFlags;
	Blend* blend;
} Window;

typedef struct tGraphics {
	u32 videoMode;
	u32 screenMode;

	u8* screen;
	u16* screen16bit;
	u16* spritePalette;
	u8* spriteData;
	
	u16* screenPointer;
	u8* VRAMPointer;	//Pointer the line in vram we're currently writing to


	u32 spritesDirty;	//Flags to tell whether we shoudl update the sprites/backgrounds.
	u32 backgroundsDirty;

	int lcy;	//Store the current lcd y variable
	u32 lineRefreshTime;
	u32 screenRefreshTime;

	u32 hBlankObjProcessing;	//Flag to tell whether or not to process objs during hblank
											//it's faster if only vblank processing is allowed.
	u32 use2DTiles;				//Flag to tell whether tiles are stored in a "2d" or "1d" format.
	Priority *priorityArray;

	s32 masterVisibility;		//Flag for turning backgrounds, sprite, and blends visible

	//u32 layerVisible[6];			//Flags for turning background, sprite, and framebuffer
										//visibliity
	
	int framesPerRefresh;		//Number of frames to skip
	int framesLeft;			//Number of frames left

	BackgroundInfo* bgInfo[4];
	OAMInfo* OAM;		
	OAMRSInfo* OAMRS;
	BlendTarget blendTargets[10];
	u16 blendMulLUT[1024];
	u8 blendMinLUT[128];
	u8 blendLUT[32*32*32*32];
	u16 darkenLUT[0x8000 * 17];	//Or should it be 16??
	u16 lightenLUT[0x8000 * 17];	//Or should it be 16??

	//rotatioion/scaling registers
	u16* BG2DY;
	u16* BG2DMX;
	u16* BG2DX;
	u16* BG2DMY;
	u16* BG2X_L;
	u16* BG2Y_L;
	u16* BG3DY;
	u16* BG3DMX;
	u16* BG3DX;
	u16* BG3DMY;
	u16* BG3X_L;
	u16* BG3Y_L;
} Graphics;

//////////////////////////////////////////////////////////
//Graphics operations
///////////////////////////////////////////////////////
void Graphics_render();
void Graphics_renderLine ();
void Graphics_renderWindow(u16* dest, u32 y, const Window* window);
void Graphics_renderBlendWindow(u16* dest, u32 y, const Window* window);
extern void Graphics_renderRSBgPixel(const BackgroundInfo* bgInfo, u16* dest, u32 bgX, u32 bgY,
													u32* blendArray, s32 blendFlags);
void Graphics_renderTileBG(const BackgroundInfo *bgInfo, u16* dest, u32 y, const Window* window);
void Graphics_renderTileBGBlend(const BackgroundInfo* bgInfo, u16* dest, u32 y, const Window* window,
											u32* blendArray, u32 blendFlags);
void Graphics_renderRSTileBG(const BackgroundInfo* bgInfo, u16* dest, u32 y, const Window* window, u32* blendArray, u32 blendFlags);
inline int Graphics_renderSprite(const SpriteInfo* spriteInfo, u16* dest, u32 scanY, const Window* window, u32* blendArray, u32 blendFlags);
inline void Graphics_renderSpriteBits(const SpriteInfo* spriteInfo, u8* source, u32 startX, u16* dest, u32 numBytes, u32* blendArray, u32 blendFlags);
inline u32 Graphics_alphaBlend(u32 sourceColor, u32 destColor, u32 mulOffset);
void Graphics_renderRSSprite(const SpriteInfo* spriteInfo, u16* dest, s32 y, const Window* window, 
											u32* blendArray, s32 blendFlags);
//void Graphics_mode3RenderBG();
//void Graphics_mode4RenderBG();
//void Graphics_mode5RenderBG();
void Graphics_init();
void Graphics_initBackgrounds();
void Graphics_initBackground(BackgroundInfo* bgInfo, u32 bg);
void Graphics_initSprites (u32 maxSprites, u32 force);
int Graphics_loadSprite (SpriteInfo* spriteInfo, OAMInfo* oamInfo, u32 force);
void Graphics_delete();

extern Graphics graphics;







////////////////////////////////////////////////////////
//initSprites()
//Initializes sprite data.  Because sprites are essentially the same in every mode
//-- except for the number of sprites allowed -- I have consolodated initialization
//into one routine
////////////////////////////////////////////////////////
inline void Graphics_initSprites (u32 maxSprites, u32 force) {
	OAMInfo* oamInfo;
	SpriteInfo* currentSprite;
	Priority* currentPriority;
	
	//Initialize the number of sprites to be 0 by default.
	for (int p = 4; p--;) {
		graphics.priorityArray[p].numSprites = 0;
	}

	//If sprites are enabled...
	if (graphics.screenMode & 0x1000) {
		//As far as I know, this only applies to sprites, not to backgrounds.
		graphics.use2DTiles = !SUPPORT_GETBIT6(graphics.screenMode);
		
		
		
		//Loop through each entry in OAM.
		//For some reason it seems that the GBA draws them 
		//in reverse order -- sprites appearing first in OAM get drawn
		//last.  Thus, I am loading them in reverse order.
		for (int i = 127; i>=0; i--) {//(int i = 0; i<128; i++) {	
			register u32 priority = (graphics.OAM[i].attribute2 >> 10) & 0x3;
			currentPriority = &graphics.priorityArray[priority];	
			 
			//if (priority == p)	{//The sprite is at this priority level
			currentSprite = &(currentPriority->sprites[currentPriority->numSprites]);	//Get the sprite info ptr

			//Save a pointer to this sprite's OAM location in our spriteInfo structure
			oamInfo = &(graphics.OAM[i]);
			
			//Only keep this sprite if it successfully loaded.
			//Do not force loading in order to save time.
			if (Graphics_loadSprite (currentSprite, oamInfo, force))
				currentPriority->numSprites++;
		}				
	
		
	}
	graphics.spritesDirty = 0;

}




#endif