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
#include <math.h>
#include "ARM.h"
#include "Memory.h"
#include "Support.h"
//#include "GraphicsC.h"
#include "Graphics.h"
#include "IO.h"

Graphics graphics;



/////////////////////////////////////////
//Graphics_init
//initializes the GBA graphics functions
////////////////////////////////////////
void Graphics_init() { 
	memset(&graphics, 0, sizeof(Graphics));
	graphics.screen = new u8 [SCREEN_SIZE];;
	graphics.screen16bit = (u16*)graphics.screen;
	graphics.hBlankObjProcessing = 0;	//Whether we process sprites during vblank or hblank
	graphics.spritesDirty = graphics.backgroundsDirty = 1;	//Set them both to "dirty"
	graphics.framesPerRefresh = 1;	//i.e. default = no frame skip

	//Refresh rate = 59.7275Hz (* 229 = 13677.5975 hz)
	//CPU clock = 16,777,216Hz
	graphics.videoMode = 0x4;



	graphics.spriteData = (u8*)(gbaMem.u8VRAM + 0x10000);
	graphics.spritePalette = (u16*)(gbaMem.u8Pal + 0x200);

	

	graphics.OAM = (OAMInfo*)gbaMem.u8OAM;
	graphics.OAMRS = (OAMRSInfo*)gbaMem.u8OAM;

	//This initializes the rotation registers...
	graphics.BG2DY = ((u16*)(gbaMem.u8IORAM + 0x24));
	graphics.BG2DMX = ((u16*)(gbaMem.u8IORAM + 0x22));
	
	//Scaling registers
	graphics.BG2DX = ((u16*)(gbaMem.u8IORAM + 0x20));
	graphics.BG2DMY = ((u16*)(gbaMem.u8IORAM + 0x26));
	
	//Positioning(?) registers.  I don't know what the _H regs
	//are for so I have left them out for now.
	graphics.BG2X_L = ((u16*)(gbaMem.u8IORAM + 0x28));
	graphics.BG2Y_L = ((u16*)(gbaMem.u8IORAM + 0x2C));
	//graphics.BG2X_H = ((u16*)(gbaMem.u8IORAM + 0x2A));
	//graphics.BG2Y_H = ((u16*)(gbaMem.u8IORAM + 0x2E));
	
	
	graphics.BG3DY = ((u16*)(gbaMem.u8IORAM + 0x34));
	graphics.BG3DMX = ((u16*)(gbaMem.u8IORAM + 0x32));
	graphics.BG3DX = ((u16*)(gbaMem.u8IORAM + 0x30));
	graphics.BG3DMY = ((u16*)(gbaMem.u8IORAM + 0x36));
	graphics.BG3X_L = ((u16*)(gbaMem.u8IORAM + 0x38));
	graphics.BG3Y_L = ((u16*)(gbaMem.u8IORAM + 0x3C));
	//graphics.BG3X_H = ((u16*)(gbaMem.u8IORAM + 0x3A));
	//graphics.BG3Y_H = ((u16*)(gbaMem.u8IORAM + 0x3E));

    	
	//The following two variables will require watching,
	//especially when saving and loading states...
	graphics.VRAMPointer = gbaMem.u8VRAM;			//by toggling the A bit


			
	//Reset the screen pointer
	graphics.screenPointer = graphics.screen16bit;
	graphics.priorityArray = new Priority[4];
	
	//Zero out our priority data and set backgrounds to visible.
	for (int i = 0; i < 4; i++) {
		memset (&graphics.priorityArray[i], 0, sizeof (Priority));
		graphics.priorityArray[i].visible = 1;	//Make every priority visible.
		
	}
	graphics.masterVisibility = 0xFFFFFFFF;
	

	//Fill up the blend lookup table with values.
	//Color is the "y" axis, mul is the "x" axis.
	u32 color1, color2, mul1, mul2;
	u32 result;
	//This stores the result of the mul1tiply
	for (color1 = 0; color1 <= 0x1F; color1++) {
		for (mul1 = 0; mul1 < 0x1F; mul1++) {
			result = color1 * mul1;	//Result is a value from 0 - 961 (31 * 31).
			graphics.blendMulLUT[(color1 << 5) + mul1] = result;
		}
	}
	//This takes the (result1 + result2) >> 4, which is a number from 0 - 120.125, returns the min of this and 0x1F
	for (color1 = 0; color1 < 128; color1++) 
		graphics.blendMinLUT[color1] = __min (color1, 0x1F);
	
	for (mul1 = 0; mul1 <= 0x1F; mul1++) {
		for (mul2 = 0; mul2 <= 0x1F; mul2++)
			for (color1 = 0; color1 <= 0x1F; color1++) 
				for (color2 = 0; color2 <= 0x1F; color2++) {
					result = color1 * mul1;
					result += color2 * mul2;
					result >>= 4;
					result = __min(result, 0x1F);
					graphics.blendLUT[(mul1 << 15) + (mul2 << 10) + (color1 <<5) + color2] = result;
				}
	}//for

	

	//Now do the fade LUTs.
	u32 pixel, fade;
	for (fade = 0; fade < 17; fade++) {
		u32 mulOffset = ((16-fade) << 15) + (fade << 10);
		for (pixel = 0; pixel < 0x8000; pixel++) {
			//A fade is essentially a kind of alpha blend.  But let's use a separate lookup just to
			//say we were efficient.
			graphics.darkenLUT[ (fade << 15) + pixel ]  = Graphics_alphaBlend(pixel, 0,mulOffset);
			graphics.lightenLUT[ (fade << 15) + pixel ]  = Graphics_alphaBlend(pixel,0x7FFF,mulOffset);
		}
	}


}

//////////////////////////////////
//graphics.delete
//Deaollocates everything releated to graphics
//////////////////////////////////////////////
void Graphics_delete() {
	delete [] graphics.screen;
	delete [] graphics.priorityArray;
}

/////////////////////////////////////////////////
//graphics.initBackgrounds
//Initializes the backgrounds for all modes.
///////////////////////////////////////////////////////////////////
void Graphics_initBackgrounds () {
	graphics.videoMode = graphics.screenMode & 0x00000007;	//Mode is the last 3 bits of this reg
	

	//Loop through each level of priority.
	for (u32 p = 4; p--;) {
		
		//Now loop through the backgrounds and see if any of them match
		//with this priority.
		//Don't question my variable names.
		BackgroundInfo* bgInfo;
		int offset = 0x6;
		u16* info;
		u32 infoVal;
		int numBGs = 0;
		
		//Start with lowest priority background, work our way upward, as we would with
		//rendering.
		//If this is a tiled mode...
		//if (graphics.videoMode <=2) {
			for (int bg = 3; bg >= 0; bg--) {
				

				//If the bit which enables this background is set...
				if ( graphics.screenMode & (0x1 << (bg + 8)) ) {
				
					info = BG0_INFO + bg;
					infoVal = *info;
					//If the priority value matches...
					if ( p == (infoVal & 0x3)) {
						if ( (graphics.videoMode <=2 ) || ((graphics.videoMode > 2) && (bg == 2))) {
							bgInfo = &graphics.priorityArray[p].bgInfo[numBGs];
							numBGs++;
							Graphics_initBackground(bgInfo, bg);
						}
					}//if
				}//if
			}//for
			graphics.priorityArray[p].numBGs = numBGs;
		//if
	}//For

						/*
						bgInfo->info = info;
						bgInfo->scx = (BG0_SCX + offset);
						bgInfo->scy = (BG0_SCY + offset);
				
						//s and m are indexes into tile data and tile maps
						//for the backgrounds.
						s = (infoVal >> 2) & 0x3;
						bgInfo->tileData = (u8*)(gbaMem.u8VRAM + s*0x4000);
						
						m = (infoVal >> 8) & 0x1F;
						bgInfo->tileMap = (u16*)(gbaMem.u8VRAM + m*0x800);

						sizeCode = (infoVal >> 14) & 0x3;	//z is the size code.
						if ((bg < 2) || (graphics.videoMode == 0)) {	//If it's a non-rotational background . . .
							bgInfo->rotScale = 0;
							switch (sizeCode) {
								case 0x0:	width = height = 256;
											//bgInfo->xBitWidth = 5;
											
											break;
								case 0x1:	width = 512;
											height = 256;
											//bgInfo->xBitWidth = 6;
											break;
								case 0x2:	width = 256;
											height = 512;
											//bgInfo->xBitWidth = 5;
											break;
								case 0x3:	width = 512;
											height = 512;
											//bgInfo->xBitWidth = 6;
											break;
							}
						}
						else {
							bgInfo->rotScale = 1;
							//For bgs 2 and 3 in modes 1 and 2, we have different values.
							switch (sizeCode) {
								case 0x0:	width = height = 128;
											//width = height = 128;	//Uncomment for mode 2
											bgInfo->xBitWidth = 4;
											//bgInfo->xBitWidth = 4;	//Uncoment for mode 2...
											break;
								case 0x1:	width = 256;
											height = 256;
											bgInfo->xBitWidth = 5;
											break;
								case 0x2:	width = 512;
											height = 512;
											bgInfo->xBitWidth = 6;
											break;
								case 0x3:	width = 1024;
											height = 1024;
											bgInfo->xBitWidth = 7;
											break;
							}
					
							u32 bgOffset = bg - 2;	//Calculate the register offsets from bg2.
							//multiply it by 8 (8 x 2 bytes = 16 byte offset)
							bgOffset = bgOffset << 3;		//not to sure if this will work...
								
							//Store the scaling and rotation regs for later use
							//(so we can call the same functions on both backgrounds)
							bgInfo->DY = graphics.BG2DY + bgOffset;
							bgInfo->DMX = graphics.BG2DMX + bgOffset;
							bgInfo->DX = graphics.BG2DX + bgOffset;
							bgInfo->DMY = graphics.BG2DMY + bgOffset;
							bgInfo->X = (u32*)(graphics.BG2X_L + bgOffset);
							bgInfo->Y = (u32*)(graphics.BG2Y_L + bgOffset);
						}
						bgInfo->sizeCode = sizeCode;
						
						
						//Find out whether the bg uses 8 or 4 bit palette entries
						bgInfo->pal256 = (infoVal >> 7) & 0x1;
						//bgInfo->palNum = (infoVal >> 12) & 0xF;	//Palette number if 16 color
						//I am a dumbass.  the palette number goes with the tiles map, not the bg register.
						
						bgInfo->bgNum = bg;
						bgInfo->width = width;
						bgInfo->height = height;
						bgInfo->xTiles = width >> 3;
						bgInfo->yTiles = height >> 3;
	
					

					}//if
				
				}//if
			}//for
		//If this is a bitmap mode...
		} else {
			info = BG0_INFO + 2;	//Bitmapped backgrounds count as bg2
			infoVal = *info;

			//If the priority value matches...
			if ( p == (infoVal & 0x3)) {
				numBGs = 1;	
				bgInfo = &graphics.priorityArray[p].bgInfo[0];	//Get the info for the first bg at this priority.
				bgInfo->info = info;
				bgInfo->rotScale = 1;	//all bitmap bgs can rotate and scale.
								
				//Store the scaling and rotation regs for later use
				bgInfo->DY = graphics.BG2DY;
				bgInfo->DMX = graphics.BG2DMX;
				bgInfo->DX = graphics.BG2DX;
				bgInfo->DMY = graphics.BG2DMY;
				bgInfo->X = (u32*)(graphics.BG2X_L);
				bgInfo->Y = (u32*)(graphics.BG2Y_L);
				bgInfo->bgNum = 2;
				
				if (graphics.videoMode ==  3) {
					graphics.VRAMPointer = gbaMem.u8VRAM;
							
				} else if (graphics.videoMode == 4)	{
					
					if ( graphics.screenMode & 0x000000010 ) 	//If the 'A' bit is set.
						graphics.VRAMPointer = gbaMem.u8VRAM + 0xA000;	//These two addesses are used
					else														//by programs for flipping
						graphics.VRAMPointer = gbaMem.u8VRAM;			//by toggling the A bit
						//Reset the screen pointer
								
				} else if (graphics.videoMode == 5) {
					//Note, I switched the flipping order of these ONLY because it SEEMS to work
					//better.  Since I have no reference material on these, not much
					//else I can do.
					if ( graphics.screenMode & 0x000000010 ) 	//If the 'A' bit is set.
						graphics.VRAMPointer = gbaMem.u8VRAM;// + 0xA000;	//These two addesses are used
					else											//by programs for flipping
						graphics.VRAMPointer = gbaMem.u8VRAM + 0xA000;			//by toggling the A bit
						//Clear the screen so that we have only black around the edges
						//May not be necessary if color 0 of palette 0 is black . . .
						//memset (graphics.screen16bit, 0, SCREEN_SIZE);
				}//if/else
			}//if
		}//if
		
		
	}//for*/
	graphics.backgroundsDirty = 0;
	
}

///////////////////////////////////////////////////////////////////
//initBackground
//This initializes the given background structure according to the
//current video mode and bg type
///////////////////////////////////////////////////////////////////
void Graphics_initBackground(BackgroundInfo* bgInfo, u32 bg) {
	int offset = 0x6;
	u32 width, height;//, xTiles, yTiles;
	u32 infoVal;
	int numBGs = 0;
	u32 s, m, sizeCode;
	
	
	//if (graphics.videoMode <= 2) {	//If it's a tile-based mode...
		offset = bg << 1;
		bgInfo->info = BG0_INFO + bg;
		bgInfo->scx = (BG0_SCX + offset);
		bgInfo->scy = (BG0_SCY + offset);
		infoVal = *bgInfo->info;
		//s and m are indexes into tile data and tile maps
		//for the backgrounds.
		s = (infoVal >> 2) & 0x3;
		bgInfo->tileData = (u8*)(gbaMem.u8VRAM + s*0x4000);
		
		m = (infoVal >> 8) & 0x1F;
		bgInfo->tileMap = (u16*)(gbaMem.u8VRAM + m*0x800);

		sizeCode = (infoVal >> 14) & 0x3;	//z is the size code.
		if ((bg < 2) || (graphics.videoMode == 0)) {	//If it's a non-rotational background . . .
			bgInfo->rotScale = 0;
			bgInfo->type = BGTYPE_TEXT;
			switch (sizeCode) {
				case 0x0:	width = height = 256;
							//bgInfo->xBitWidth = 5;
							
							break;
				case 0x1:	width = 512;
							height = 256;
							//bgInfo->xBitWidth = 6;
							break;
				case 0x2:	width = 256;
							height = 512;
							//bgInfo->xBitWidth = 5;
							break;
				case 0x3:	width = 512;
							height = 512;
							//bgInfo->xBitWidth = 6;
							break;
			}
			//Set the rotate/scale parameters to point to *something*
			static u32 zero = 0;
			bgInfo->DY = (u16*)&zero;
			bgInfo->DMX = (u16*)&zero;
			bgInfo->DX = (u16*)&zero;
			bgInfo->DMY = (u16*)&zero;
			bgInfo->X = &zero;
			bgInfo->Y = &zero;
		}
		else {
			bgInfo->info = BG0_INFO + bg;	
			bgInfo->rotScale = 1;
			bgInfo->type = BGTYPE_ROTSCALE;
			//For bgs 2 and 3 in modes 1 and 2, we have different values.
			switch (sizeCode) {
				case 0x0:	width = height = 128;
							//width = height = 128;	//Uncomment for mode 2
							bgInfo->xBitWidth = 4;
							//bgInfo->xBitWidth = 4;	//Uncoment for mode 2...
							break;
				case 0x1:	width = 256;
							height = 256;
							bgInfo->xBitWidth = 5;
							break;
				case 0x2:	width = 512;
							height = 512;
							bgInfo->xBitWidth = 6;
							break;
				case 0x3:	width = 1024;
							height = 1024;
							bgInfo->xBitWidth = 7;
							break;
			}
		
			u32 bgOffset = bg - 2;	//Calculate the register offsets from bg2.
			//multiply it by 8 (8 x 2 bytes = 16 byte offset)
			bgOffset = bgOffset << 3;		//not to sure if this will work...
				
			//Store the scaling and rotation regs for later use
			//(so we can call the same functions on both backgrounds)
			bgInfo->DY = graphics.BG2DY + bgOffset;
			bgInfo->DMX = graphics.BG2DMX + bgOffset;
			bgInfo->DX = graphics.BG2DX + bgOffset;
			bgInfo->DMY = graphics.BG2DMY + bgOffset;
			bgInfo->X = (u32*)(graphics.BG2X_L + bgOffset);
			bgInfo->Y = (u32*)(graphics.BG2Y_L + bgOffset);
		}
		bgInfo->sizeCode = sizeCode;
	
		if (graphics.videoMode ==  3) {
			bgInfo->type = BGTYPE_BITMAP;
			bgInfo->pal256 = 0;
			graphics.VRAMPointer = gbaMem.u8VRAM;
					
		} else if (graphics.videoMode == 4)	{
			bgInfo->type = BGTYPE_BITMAP;
			bgInfo->pal256 = 1;
			if ( graphics.screenMode & 0x000000010 ) 	//If the 'A' bit is set.
				graphics.VRAMPointer = gbaMem.u8VRAM + 0xA000;	//These two addesses are used
			else														//by programs for flipping
				graphics.VRAMPointer = gbaMem.u8VRAM;			//by toggling the A bit
				//Reset the screen pointer
						
		} else if (graphics.videoMode == 5) {
			bgInfo->type = BGTYPE_BITMAP;
			bgInfo->pal256 = 0;
			//Note, I switched the flipping order of these ONLY because it SEEMS to work
			//better.  Since I have no reference material on these, not much
			//else I can do.
			if ( graphics.screenMode & 0x000000010 ) 	//If the 'A' bit is set.
				graphics.VRAMPointer = gbaMem.u8VRAM;// + 0xA000;	//These two addesses are used
			else											//by programs for flipping
				graphics.VRAMPointer = gbaMem.u8VRAM + 0xA000;			//by toggling the A bit
				//Clear the screen so that we have only black around the edges
				//May not be necessary if color 0 of palette 0 is black . . .
				//memset (graphics.screen16bit, 0, SCREEN_SIZE);
		}//if/else	
		

		//Find out whether the bg uses 8 or 4 bit palette entries
		bgInfo->pal256 = (infoVal >> 7) & 0x1;
		//bgInfo->palNum = (infoVal >> 12) & 0xF;	//Palette number if 16 color
		//I am a dumbass.  the palette number goes with the tiles map, not the bg register.
		
		bgInfo->bgNum = bg;
		bgInfo->width = width;
		bgInfo->height = height;
		bgInfo->xTiles = width >> 3;
		bgInfo->yTiles = height >> 3;
	//} 
	
	
	//else {	//If it's a bitmapped mode
	/*	bgInfo->rotScale = 1;	//all bitmap bgs can rotate and scale.
						
		//Store the scaling and rotation regs for later use
		bgInfo->DY = graphics.BG2DY;
		bgInfo->DMX = graphics.BG2DMX;
		bgInfo->DX = graphics.BG2DX;
		bgInfo->DMY = graphics.BG2DMY;
		bgInfo->X = (u32*)(graphics.BG2X_L);
		bgInfo->Y = (u32*)(graphics.BG2Y_L);
		bgInfo->bgNum = 2;
	*/	
	
		

	//}

}


////////////////////////////////////////////////
//mode3RenderBG()
//Renders the background using mode 3
//(GBA "high color" mode -- no palette, really
//straightfoward)
///////////////////////////////////////////////////
inline void Graphics_mode3RenderBG(u16* dest, u32 y, const Window* window, 
													u32* blendArray, u32 blendFlags ) {
		u16* source = (u16*)graphics.VRAMPointer;
		//256 = 2^8 - 2^4
		source = source + (y << 8) - (y<<4) + window->winLeft;
		dest += window->winLeft;

		//Loop through each bit, copy to the screen buffer.
		//Could be unrolled.
		if (window->blend->enabled) {
			for (int i = window->winLeft; i < window->winRight; i++) {
				dest[i] = source[i];
				if (blendFlags & 0xF0) {	//If this is a source blend...
					u32 oldBlendFlags = *blendArray;	//Get the current flags for this pixel
					blendArray[i] = blendFlags | (oldBlendFlags >> 12);	//Set the current target.
				} else {
					blendArray[i] = blendFlags;
				}
			}
		} else {
			for (int i = window->winLeft; i < window->winRight; i++) {
				dest[i] = source[i];
			}
		}
}



////////////////////////////////////////////////
//mode4RenderBG()
//Renders the background to the current scanline using mode 4
//(paletted single layer mode), updates
//the pointers for the current line.
///////////////////////////////////////////////////
inline void Graphics_mode4RenderBG(u16* dest, u32 y, const Window* window, 
															u32* blendArray, u32 blendFlags) {
		u8* source = graphics.VRAMPointer + (y << 8) - (y<<4) + window->winLeft;
		dest += window->winLeft;

		//Iterate through the background.
		if (window->blend->enabled) {
			for (int i = window->winLeft; i < window->winRight; i++) {
				u32 index = source[i];
				dest[i] = gbaMem.u16Pal[index];
				if (blendFlags & 0xF0) {	//If this is a source blend...
					u32 oldBlendFlags = *blendArray;	//Get the current flags for this pixel
					blendArray[i] = blendFlags | (oldBlendFlags >> 12);	//Set the current target.
				} else {
					blendArray[i] = blendFlags;
				}
			}
		} else {
			for (int i = window->winLeft; i < window->winRight; i++) {
				u32 index = source[i];
				dest[i] = gbaMem.u16Pal[index];
			}
		}
}

////////////////////////////////////////////////
//mode5RenderBG()
//Renders the background  using mode 5
//(GBA "high color" mode with smaller screen area)
///////////////////////////////////////////////////
inline void Graphics_mode5RenderBG(u16* dest, u32 y, const Window* window, 
														u32* blendArray, u32 blendFlags) {
	u16* source;
	u32 winLeft, winRight;
	//Note that this does not take into account any rotation/scaling/positioning of the bg
	//using bg2 registers.
	winLeft = window->winLeft;
	winRight = window->winRight;
	if (winLeft > 160)
		winLeft = 0;
	if (winRight > 160)
		winRight = 160;
	
	if (y < 128) {
		source = (u16*)graphics.VRAMPointer;
		source+= y * 160 + winLeft;
		dest += winLeft;
		
		//Loop through each halfword, copy to the screen buffer.
		//for (int i = 160; i--;) {
		if (window->blend->enabled) {
			for (int i = winLeft; i < winRight; i++) {
				dest[i] = source[i];	
				if (blendFlags & 0xF0) {	//If this is a source blend...
					u32 oldBlendFlags = *blendArray;	//Get the current flags for this pixel
					blendArray[i] = blendFlags | (oldBlendFlags >> 12);	//Set the current target.
				} else {
					blendArray[i] = blendFlags;
				}
			}
		} else {
			for (int i = winLeft; i < winRight; i++) {
				dest[i] = source[i];
			}
		}
	} 

}


//////////////////////////////////////////////////////
//renderWindow()
//Use to render a window based on the given flags.
//Flags are of the format used in REG_WIN_X registers
///////////////////////////////////////////////////
inline void Graphics_renderWindow(u16* dest, u32 y, const Window* window) {
	Priority* priority;
	SpriteInfo* spriteInfo;
	BackgroundInfo *bgInfo;
	
	//Eliminate error
	//if (winLeft >= winRight)
	//	return;
	
	if (y >= window->winBottom)
		return;
	if (y < window->winTop)
		return;

	if (window->winLeft == window->winRight)
		return;

	const Blend *blend = window->blend;
	u32* blendArray = blend->blendArray;
	
	//Render backdrop.
	u16 color = gbaMem.u16Pal[0];
	
	if ((window->winLeft & 0x1) || (window->winRight & 0x1)) {	//If either winleft or winRight are odd
		for (int i = window->winLeft; i<window->winRight; i++) {
			dest[i] = color;
		}
	} else {	//If both winleft and winright are even numbers, we can do 2 pixels at a time.
		u32 color32 = color | color << 16;
		u32 winLeft = window->winLeft >> 1;
		u32 winRight = window->winRight >> 1;
		u32* dest32 = (u32*)dest;
		for (int i = winLeft; i < winRight; i++) {
			dest32[i] = color32;
		}
	}


	//For each level of priority, draw the background, then the sprites.
	for (u32 p = 4; p--;) {
		priority = & (graphics.priorityArray[p]);		
		if (priority->visible) {	//If this priority is visible...
					
			//For each background at this priority (because some people - such as me - don't
			//bother to prioritize them)
			for (int b = 0; b < priority->numBGs; b++) {
				bgInfo = &(priority->bgInfo[b]);
				if (window->winFlags & (1 << bgInfo->bgNum)) {
					switch(graphics.videoMode) {
						case 0x0: 	Graphics_renderTileBG(bgInfo, dest, y, window);
										break;
						case 0x1:	if (bgInfo->bgNum == 0x2)
											Graphics_renderRSTileBG(bgInfo, dest, y, window, blendArray, 0xFFFFFFFF);
										else
											Graphics_renderTileBG(bgInfo, dest, y, window);
										break;
						
						case 0x2:	Graphics_renderRSTileBG(bgInfo, dest, y, window, blendArray, 0xFFFFFFFF);
										break;

						case 0x3:	//Loop through each bit, copy to the screen buffer.
										//Could be unrolled.
										Graphics_mode3RenderBG(dest, y, window, blendArray, 0xFFFFFFFF);
										break;
						case 0x4:
										Graphics_mode4RenderBG(dest, y, window, blendArray, 0xFFFFFFFF);
										break;
						case 0x5:	
										Graphics_mode5RenderBG(dest, y, window, blendArray, 0xFFFFFFFF);
										break;
					}//Switch
				}//if
			
			}
		
			
			//Render as many sprites as there are listed, but stop after we reach the
			//maximum number of tiles a GBA is capable of.
			if (window->winFlags & 0x10) {
				for (int i = 0; (i < priority->numSprites); i++) {// && (tilesRendered <= 100); i++) {	//Code for tracking tiles rendered is imperfect
					spriteInfo = &(priority->sprites[i]);
					if ( (spriteInfo->oamInfo->attribute0 & 0x0C00) != 0x0800) {	//If not an obj window
						if (spriteInfo->rotScale)
							Graphics_renderRSSprite(spriteInfo, graphics.screenPointer, y, window, blendArray, 0xFFFFFFFF);
						else
							Graphics_renderSprite(spriteInfo, graphics.screenPointer, y, window, blendArray, 0xFFFFFFFF);
					}
				}//for each sprite
			}//if
		}//If this priority is visible
	}//for each priority
}

//////////////////////////////////////////////////////
//renderBlendWindow()
//Use to render a window based on the given flags.
//Flags are of the format used in REG_WIN_X registers
//Renders blends too.  There is a special format for the blend flags
//which I use to determine how to blend:
//
//F E D C  B A 9 8  7 6 5 4  3 2 1 0
//N N N N  F F F F  S S S S  T T T T
//
//S = currentSource 
//T = currentTarget
//F = fadeSource
//N = nextTarget
//
//target and source numbers are:  1 = backdrop, 2 - 5 = bg0 - bg3, 6 = objs
//If currentSource and currentTarget are nonzero, it means "blend these two"
//nextTarget is the target layer in line to be blended.  It gets moved up to
//"currentTarget" when ever a pixel from a source layer is encountered.
//A value of "0xFFFFFFFF" means not to change any blend flags, period.
///////////////////////////////////////////////////
inline void Graphics_renderBlendWindow(u16* dest, u32 y, const Window* window) {
	Priority* priority;
	SpriteInfo* spriteInfo;
	BackgroundInfo *bgInfo;
	//Blend* blend;
	u32 sourceShift, sourceNum, targetNum, blendFlags;
	u32* blendArray;
	

	if (y >= window->winBottom)
		return;
	if (y < window->winTop)
		return;
	//Eliminate error
	if (window->winLeft == window->winRight)
		return;

	const Blend *blend = window->blend;
	blendArray = blend->blendArray;

	if ( blend->bldType == 0x1 ) 	//If this is an alpha blend
		sourceShift = 4;		//We want to shift the source values to the "currentSource" position
	else 
		sourceShift = 8;	//Otherwise, shift the source values to the "fade" position
									//(Renderers behave differently for fade sources and for normal alpha sources
	
	u16 color = gbaMem.u16Pal[0];
	
	
	if (blend->bldCnt & 0x20) 	//If the backdrop is a source
			blendFlags = (1 << sourceShift);	//Mix it in
	else
			blendFlags = 0;

	//Initialize the blend array with any flags from the backdrop
	//NOTE:  Need to incorporate winLeft, winRight
	if (blend->bldCnt & 0x2000) {	//If the backdrop is a target
		blendFlags |= 0x1000;		//Say so in the blend flags.
				
		for (int i = window->winLeft; i < window->winRight; i++) {	//Then initialize the blend flags as well as the blend targets.
			dest[i] = graphics.blendTargets[1].buffer[i] = color;	//record the backdrop color.
			blendArray[i] = blendFlags;		
		}
	//If the backdrop is not a target, set the blend array to all 0.
	} else {
		for (int i = window->winLeft; i < window->winRight; i++) {	//Then initialize the blend flags as well as the blend targets.
			dest[i] = color;
			blendArray[i] = blendFlags;		
		}
	}
	
	

	
	

	//For each level of priority, draw the background, then the sprites.
	for (u32 p = 4; p--;) {
			
		priority = & (graphics.priorityArray[p]);		
		if (priority->visible) {	//If this priority is visible...
					
			//For each background at this priority (because some people - such as me - don't
			//bother to prioritize them)
			for (int b = 0; b < priority->numBGs; b++) {
				bgInfo = &(priority->bgInfo[b]);
				
				//If this layer is visible...
				if (window->winFlags & (0x1 << bgInfo->bgNum)) {
					//If source
					if (blend->bldCnt & (0x1 << bgInfo->bgNum)) {
						
						sourceNum = bgInfo->bgNum+2;
					} else {
						sourceNum = 0;
					}
					
					//if target
					if (blend->bldCnt & (0x100 << bgInfo->bgNum))
						targetNum = bgInfo->bgNum+2;
					else
						targetNum = 0;
					//Set the next target and the current source.
					blendFlags = (targetNum << 12) | (sourceNum << sourceShift);
					
					

					switch(graphics.videoMode) {
						case 0x0: 	Graphics_renderTileBGBlend(bgInfo, dest, y, window,
																				blendArray, blendFlags);
										break;
						case 0x1:	if (bgInfo->bgNum == 0x2)
											Graphics_renderRSTileBG(bgInfo, dest, y, window,
																				blendArray, blendFlags);
										else
											Graphics_renderTileBGBlend(bgInfo, dest, y, window,
																				blendArray, blendFlags);
										break;
						
						case 0x2:	Graphics_renderRSTileBG(bgInfo, dest, y, window,
																				blendArray, blendFlags);
										break;

						case 0x3:	//Loop through each bit, copy to the screen buffer.
										//Could be unrolled.
										Graphics_mode3RenderBG(dest, y, window, blendArray, blendFlags);
										break;
						case 0x4:
										Graphics_mode4RenderBG(dest, y, window, blendArray, blendFlags);
										break;
						case 0x5:	
										Graphics_mode5RenderBG(dest, y, window, blendArray, blendFlags);
										break;
					}//Switch

					if (targetNum) {	//If this is a target
						if (!graphics.blendTargets[targetNum].dirty) {	//If it's not dirty, copy it from the current scanline.
						
							u32 *tempSource = (u32*)dest;
							tempSource += (window->winLeft >> 1);
							u32 *tempDest = (u32*)graphics.blendTargets[targetNum].buffer;
							tempDest += (window->winLeft >> 1);
							
							u32 numWords = ((window->winRight - window->winLeft) >> 1) + (window->winRight & 0x1);
							while (numWords--) {
								*tempDest++ = *tempSource++;
							}
							if ((window->winLeft == 0) && (window->winRight == 240))	//If we got the whole scanline...
								graphics.blendTargets[targetNum].dirty = 1;			//Make note so that we don't have to repeat this copy
						
						}
					}
					

				}//if
			
			}
		
			
			//Render the sprites (if enabled)
			if (window->winFlags & 0x10) {
					u32 semiFlags = (6+p) << sourceShift;	//The source number for semitransparent sprites
					
					//If source
					if (blend->bldCnt & (0x10 ))
						sourceNum = semiFlags;
					else
						sourceNum = 0;
					
					//if target
					if (blend->bldCnt & (0x1000))
						targetNum = (6+p) << 12;
					else
						targetNum = 0;
					
					

					//Set the next target and the current source.
					blendFlags = targetNum | sourceNum;
					semiFlags |= targetNum;	//Set up semitransparent sprite flags
					u32 spritesRendered = 0;

					//Render the sprites.
					for (int i = 0; (i < priority->numSprites); i++) {// && (tilesRendered <= 100); i++) {	//Code for tracking tiles rendered is imperfect
						spritesRendered = 1;
						spriteInfo = &(priority->sprites[i]);
						if ( (spriteInfo->oamInfo->attribute0 & 0x0C00) != 0x0800) {	//If not an obj window
						
							if (spriteInfo->oamInfo->attribute0 & 0x0400) {	//If semitransparent
								if (spriteInfo->rotScale)
									Graphics_renderRSSprite(spriteInfo, dest, y, window, blendArray, semiFlags);
								else
									Graphics_renderSprite(spriteInfo, dest, y, window, blendArray, semiFlags);
							} else {
								if (spriteInfo->rotScale)
									Graphics_renderRSSprite(spriteInfo, dest, y, window, blendArray, blendFlags);
								else
									Graphics_renderSprite(spriteInfo, dest, y, window, blendArray, blendFlags);
							}
						}

					}//for each sprite
					

					//If if this was a target and if sprites were rendered at this priority . . .
					if (targetNum && spritesRendered) {
							
						//if (!graphics.blendTargets[6+p].dirty) {	//If it's not dirty, copy it from the current scanline.
						
							u32 *tempSource = (u32*)dest;
							tempSource += (window->winLeft >> 1);
							u32 *tempDest = (u32*)graphics.blendTargets[6+p].buffer;
							tempDest += (window->winLeft >> 1);
							
							u32 numWords = ((window->winRight - window->winLeft) >> 1) + (window->winRight & 0x1);
							while (numWords--) {
								*tempDest++ = *tempSource++;
							}
							if ((window->winLeft == 0) && (window->winRight == 240))	//If we got the whole scanline...
								graphics.blendTargets[6+p].dirty = 1;		//Make note so that we don't have to repeat this copy
						//}
						
					}

			}//if
		}//If this priority is visible

		

	}//for each priority
	
	if (window->winFlags & 0x20) {	//(redundant code)
		//Once the whole thing is done, do the blends.
		if (blend->bldType == 0x1) {	//If it's an alpha blend,
			for (int i = window->winLeft; i < window->winRight; i++) {
				blendFlags = blendArray[i];
				sourceNum = (blendFlags >> 4) & 0xF;
				targetNum = blendFlags & 0xF;	
		

				//If there is both a current source and a current target,
				if (sourceNum && targetNum) {
					
					u16* target = graphics.blendTargets[targetNum].buffer;
					dest[i] = Graphics_alphaBlend(dest[i], target[i], blend->mulOffset);
					
				}//if
			}//for
		} else if (blend->bldType == 0x2) {	//If it's a lighten
			for (int i = window->winLeft; i < window->winRight; i++) {
				if ((blendArray[i] >> 8) & 0xF) {
					dest[i] = graphics.lightenLUT[blend->mulOffset + (dest[i] & 0x7FFF)];
				}//if
				}//for
		} else  if (blend->bldType == 0x3) {	//It's a darken
			for (int i = window->winLeft; i < window->winRight; i++) {
				if ((blendArray[i] >> 8) & 0xF) {
					dest[i] = graphics.darkenLUT[blend->mulOffset + (dest[i] & 0x7FFF)];
						
				}//if
			}//for
		}
	}
}

////////////////////////////////////////////////////////
//setWindowBounds
//Use to divide screen up into windows.
////////////////////////////////////////////////////////
inline u32 Graphics_setWindowBounds(Window* windowArray, Blend* blendPtr, u32 winFlags, u32 winIndex, 
												u32 winLeft, u32 winRight, u32 winTop, u32 winBottom) {
	
	if (winLeft > 240)	//Do some bounding.
		winLeft = 240;
	if (winRight > 240)
		winRight = 240;
	if (winTop > 160)
		winTop = 160;
	if (winBottom > 160)
		winBottom = 160;
	
	if (winLeft < winRight)	{	//If it's not inverted horizontally
		if (winTop < winBottom) {	//If it's not inverted verticaly
			windowArray[winIndex].winLeft = winLeft;
			windowArray[winIndex].winRight = winRight;
			windowArray[winIndex].winTop = winTop;
			windowArray[winIndex].winBottom = winBottom;
			windowArray[winIndex].winWidth = winRight - winLeft;
			windowArray[winIndex].winFlags = winFlags;	//Get flags for window 1
			windowArray[winIndex].blend = blendPtr;
			winIndex++;
		} else if (winTop > winBottom) {	//If the window is inverted vertically, must divide up into two sections,
					//leaving the middle blank.
			//Set up the top section and the common parts of the bottom section
			windowArray[winIndex].winLeft = windowArray[winIndex+1].winLeft = winLeft;
			windowArray[winIndex].winRight = windowArray[winIndex+1].winRight = winRight;
			windowArray[winIndex].winTop = 0;
			windowArray[winIndex].winBottom = winBottom;
			windowArray[winIndex].winWidth = windowArray[winIndex+1].winWidth = winRight - winLeft;
			windowArray[winIndex].winFlags = windowArray[winIndex+1].winFlags = winFlags;	//Get flags for window 1
			windowArray[winIndex].blend = windowArray[winIndex+1].blend = blendPtr;
			winIndex++;
			//Set up the bottom section.
			windowArray[winIndex].winTop = winTop;
			windowArray[winIndex].winBottom = 160;
			winIndex++;
		}

	} else if (winLeft > winRight) {	//If the window is inverted horizontally
		if (winTop < winBottom) {		//Must split the left section and the right section, leaving middle blank.
			//Set up the left section and the common parts of the right section
			windowArray[winIndex].winTop = windowArray[winIndex+1].winTop = winTop;
			windowArray[winIndex].winBottom = windowArray[winIndex+1].winBottom = winBottom;
			windowArray[winIndex].winLeft = 0;
			windowArray[winIndex].winRight = winRight;
			windowArray[winIndex].winWidth = winRight;
			windowArray[winIndex].winFlags = windowArray[winIndex+1].winFlags = (*REG_WIN_IN) >> 8;	//Get flags for window 1
			windowArray[winIndex].blend = windowArray[winIndex+1].blend = blendPtr;
			winIndex++;
			//Set up the right section.
			windowArray[winIndex].winLeft = winLeft;
			windowArray[winIndex].winRight = 240;
			windowArray[winIndex].winWidth = 240 - winLeft;
			winIndex++;
		} else if (winTop > winBottom) {	//If the window is inverted vertically and horizontally
			//Split the window up into four corners.
			//Set up the common parts of all four windows
			windowArray[winIndex].winFlags = windowArray[winIndex+1].winFlags =
			windowArray[winIndex+2].winFlags = windowArray[winIndex+3].winFlags = winFlags;	//Get flags for window 1
			windowArray[winIndex].blend = windowArray[winIndex+1].blend = 
			windowArray[winIndex+2].blend = windowArray[winIndex+3].blend = blendPtr;
								
			//Upper left hand corner
			windowArray[winIndex].winTop = 0;
			windowArray[winIndex].winBottom = winBottom;
			windowArray[winIndex].winLeft = 0;
			windowArray[winIndex].winRight = winRight;
			windowArray[winIndex].winWidth = winRight;
			winIndex++;

			//Upper right hand corner
			windowArray[winIndex].winTop = 0;
			windowArray[winIndex].winBottom = winBottom;
			windowArray[winIndex].winLeft = winLeft;
			windowArray[winIndex].winRight = 240;
			windowArray[winIndex].winWidth = 240 - winLeft;
			winIndex++;

			//Lower left hand corner
			windowArray[winIndex].winTop = winTop;
			windowArray[winIndex].winBottom = 160;
			windowArray[winIndex].winLeft = 0;
			windowArray[winIndex].winRight = winRight;
			windowArray[winIndex].winWidth = winRight;
			winIndex++;
			
			//Lower right hand corner
			windowArray[winIndex].winTop = winTop;
			windowArray[winIndex].winBottom = 160;
			windowArray[winIndex].winLeft = winLeft;
			windowArray[winIndex].winRight = 240;
			windowArray[winIndex].winWidth = 240 - winLeft;
			winIndex++;


		}
	}
		
	
	return winIndex;

}

//////////////////////////////////////////////////////////////
//graphics.renderLine
//Renders the current line
//////////////////////////////////////////////////////////////
void Graphics_renderLine () {
	u32 winLeft, winRight, winTop, winBottom;
	u32 tilesRendered;
	u32 y;
	u32 winFlags;
	tilesRendered = 0;
	u32 blendArray[240];
	Window windowArray[10];
	Blend blend;
	Blend* blendPtr;
	u32 winIndex;
	//u32 enableBlends;

	if (graphics.framesLeft > 0)
		return;

	graphics.screenPointer = graphics.screen16bit + graphics.lcy*240;
	
	
	//Next, re-initialize sprites and backgrounds, if the data has been changed.
	if (graphics.spritesDirty)
		Graphics_initSprites(128, 0);

	if (graphics.backgroundsDirty)
		Graphics_initBackgrounds();

	//Set up our blend info in a structure.
	blend.bldCnt = *REG_BLDCNT;
	blend.bldType = (blend.bldCnt >> 6) & 0x3;
	blend.enabled = 1;	//Assume enabled to begin with
	

	//If there is no blends, don't bother with the rest of the blend structure.
	if (!blend.bldType) {		
		blend.enabled = 0;
		//blendPtr = NULL;
	} //else {
	blendPtr = &blend;
	blend.bldAlpha = *REG_BLDALPHA;
	blend.bldY = *REG_BLDY;
	
	if (blend.bldType == 0x1) {	//If it's an alpha blend
		blend.sourceMul = blend.bldAlpha & 0x1F;
		blend.destMul = (blend.bldAlpha >> 8) & 0x1F;
		if ( (blend.sourceMul == 16) && (blend.destMul == 0))	//If there is no blend, don't process.
			blend.enabled = 0;
	} else {
		blend.sourceMul = blend.bldY & 0x1F;
		
		if (!blend.sourceMul)
			blend.enabled = 0;
		else if (blend.sourceMul > 16)
			blend.sourceMul = 16;

		blend.destMul = 0;
	}
	blend.mulOffset = (blend.sourceMul << 15) + (blend.destMul << 10);
	blend.blendArray = blendArray;
	//}
	
	//Set up the furthermost window
	windowArray[0].winLeft = 0;
	windowArray[0].winRight = 240;
	windowArray[0].winTop = 0;
	windowArray[0].winBottom = 160;
	windowArray[0].winWidth = 240;
	windowArray[0].blend = blendPtr;
	winIndex = 1;

	y = *REG_VCOUNT;

	if (*REG_DSPCNT & 0xE000) {	//If any windows are enabled (period) draw win_out
		windowArray[0].winFlags = (*REG_WIN_OUT & 0x3F) & graphics.masterVisibility;
		//Graphics_renderBlendWindow(graphics.screenPointer, y, &windowArray[0]);
		
		if (blend.enabled && (windowArray[0].winFlags & 0x20)) {	//If blends enabled
			Graphics_renderBlendWindow(graphics.screenPointer, y, &windowArray[0]);
		} else {
			Graphics_renderWindow(graphics.screenPointer, y, &windowArray[0]);
		}

		//If sprite windows are enabled, render them next
		if (*REG_DSPCNT & 0x8000) {
			windowArray[1].winLeft = 0;
			windowArray[1].winRight = 240;
			windowArray[1].winTop = 0;
			windowArray[1].winBottom = 160;
			windowArray[1].winWidth = 240;
			windowArray[1].blend = blendPtr;
			windowArray[1].winFlags = graphics.masterVisibility;
			winIndex = 1;
			u32 objWinMask[240];
			//We use the blend array as a kind of mask to see where to render in the sprite window.
			//However, we must first set the blend array to 0.
			for (int j = 0; j < 240; j++) {
				objWinMask[j] = 0;
			}
			u16* scratchDest = (u16*)gbaMem.scratchMem;
			//Now we use the sprites to set the "dirty" parts of the blend array
			for (int p = 4; p--;) {
				Priority* priority = &graphics.priorityArray[p];
				for (int i = 0; (i < priority->numSprites); i++) {
					SpriteInfo* spriteInfo = &(priority->sprites[i]);
					u32 spriteFlags = 0x0600;
					
					if ((spriteInfo->oamInfo->attribute0 & 0x0C00) == 0x0800) {	//If obj window
						if (spriteInfo->rotScale)
							Graphics_renderRSSprite(spriteInfo, scratchDest, y, &windowArray[1], objWinMask, spriteFlags);
						else
							Graphics_renderSprite(spriteInfo, scratchDest, y, &windowArray[1], objWinMask, spriteFlags);
					}
				}
			}

			//Next render window to scratch mem
			windowArray[1].winFlags = (*REG_WIN_OUT >> 8) & graphics.masterVisibility;
			if (blend.enabled && (windowArray[1].winFlags & 0x20)) {	//If blends enabled
				Graphics_renderBlendWindow(scratchDest, y, &windowArray[1]);
			} else {
				Graphics_renderWindow(scratchDest, y, &windowArray[1]);
			}
			for (j = 0; j < 240; j++) {
				if (objWinMask[j]) {	//If the objWinMask is non zero, it's a sprite blend, so keep it.
					graphics.screenPointer[j] = scratchDest[j];
				}
			}

		}

		winIndex = 2;

		///////////////////////////
		//Set up window 1, if enabled
		///////////////////////////
		if (*REG_DSPCNT & 0x4000) {	
			winLeft = *REG_WIN1_H >> 8;
			winRight = *REG_WIN1_H & 0xFF;
			winTop = *REG_WIN1_V >> 8;
			winBottom = *REG_WIN1_V & 0xFF;
			winFlags = (*REG_WIN_IN >> 8) & graphics.masterVisibility;
			winIndex = Graphics_setWindowBounds(windowArray, blendPtr, winFlags , winIndex, winLeft, winRight,
																winTop, winBottom);	
		
		}	

		//////////////////////////
		//Set up window 0, if inabled
		/////////////////////////////
		if (*REG_DSPCNT & 0x2000) {	
			winLeft = *REG_WIN0_H >> 8;
			winRight = *REG_WIN0_H & 0xFF;
			winTop = *REG_WIN0_V >> 8;
			winBottom = *REG_WIN0_V & 0xFF;
			winFlags = (*REG_WIN_IN & 0x3F) & graphics.masterVisibility;
			winIndex = Graphics_setWindowBounds(windowArray, blendPtr, winFlags, winIndex, winLeft, winRight,
																winTop, winBottom);	
		}	

		for (int i = 2; i < winIndex; i++) {	//Render window 2 onward.
			if (blend.enabled && (windowArray[i].winFlags & 0x20)) {	//If blends enabled
				Graphics_renderBlendWindow(graphics.screenPointer, y, &windowArray[i]);
			} else {
				Graphics_renderWindow(graphics.screenPointer, y, &windowArray[i]);
			}
		}

	} else {
		//Otherwise there is just one window - the main screen.
		//Render it.
		windowArray[0].winFlags = graphics.masterVisibility;
		if (blend.enabled)
			//Graphics_renderWindow(graphics.screenPointer, y, &windowArray[0]);
			Graphics_renderBlendWindow(graphics.screenPointer, y, &windowArray[0]);
		else
			Graphics_renderWindow(graphics.screenPointer, y, &windowArray[0]);
	}

	

	
	//reset all the dirty bits.  Don't need to worry about the backdrop.
	graphics.blendTargets[2].dirty = 0;
	graphics.blendTargets[3].dirty = 0;
	graphics.blendTargets[4].dirty = 0;
	graphics.blendTargets[5].dirty = 0;
	
}


///////////////////////////////////////////////
//getSpriteData
//Copies sprite tiles from the sprite data buffer
//to the sprite buffer
////////////////////////////////////////////
int Graphics_loadSprite (SpriteInfo* spriteInfo, OAMInfo* oamInfo, u32 force) {
	u32* dest;
	u32* currentLine;
	u32* source;
	u8* currentLine8bit;
	u8* source8bit;
	u8* dest8bit;
	u8 byte;
	u32 attribute0, attribute1, attribute2;
	u32 index, yTiles, xTiles;
	int x, y, width, height;

	//Get the struct members into local variables.
	attribute0 = oamInfo->attribute0;
	
	//Don't even process if the sprite is turned off
	if ( ((attribute0 >> 8) & 0x3) == 0x2) {
		spriteInfo->visible = 0;
		if (!force)		//Unless we are forcing it to load, return.
			return 0;
	} else {
		spriteInfo->visible = 1;
	}
		
	
	attribute1 = oamInfo->attribute1;
	attribute2 = oamInfo->attribute2;

	y = attribute0 & 0xFF;	//y is the last part of attrite 0
	x = attribute1 & 0x1FF;	//x can be slightly larger than y.


	//Get the size code based on oam info.
	u32 sizeCode = ((attribute0 >> 12) & 0xC) | ((attribute1 >> 14) & 0x3);
				
	//Now switch on this and find the sprite width and height.
	switch (sizeCode) {
		case 0x0:	width = height = 8;
					break;
		case 0x1:  width = height = 16;
					break;
		case 0x2:	width = height = 32;
					break;
		case 0x3:	width = height = 64;
					break;
		case 0x4:	width = 16;
					height = 8;
					break;
		case 0x5:	width = 32;
					height = 8;
					break;
		case 0x6:	width = 32;
					height = 16;
					break;
		case 0x7:	width = 64;
					height = 32;
					break;
		case 0x8:	width = 8;
					height = 16;
					break;
		case 0x9:	width = 8;
					height = 32;
					break;
		case 0xA:	width = 16;
					height = 32;
					break;
		case 0xB:	width = 32;
					height = 64;
					break;
		default:	width = 8;
					height = 8;
					break;
	}//End of switch

	
	spriteInfo->rotScale = attribute0 & 0x100;	//Get the rotate/scale bit
	if (spriteInfo->rotScale && ((attribute0 >> 9) & 0x1)) {	//If double size is on
		spriteInfo->effectiveWidth = width << 1;
		spriteInfo->effectiveHeight = height << 1;
	} else {
		spriteInfo->effectiveWidth = width;
		spriteInfo->effectiveHeight = height;
	}
	

	//Don't load the sprite if it isn't on the screen
	//if ( ((x + width) < 0) || (x >= 240) || ((y + height) < 0) || (y >= 160) )
	//The reason I use "255 - height" is that y is not signed -- it simply rolls back to 255
	//when it goes past 0.  I want to keep on drawing the sprite until the entire thing is scrolled
	//off the top but not yet appearing on the bottom.
	if ( (( (x >= 240) && (x < (512 - spriteInfo->effectiveWidth))) 
			|| ( (y >= 160) && (y < (256 - spriteInfo->effectiveHeight)) )) && !force) {
		return 0;	
	}
	
	xTiles = width >> 3;
	yTiles = height >> 3;
	
	//Get a pointer to the bitmap
	//(caution:  these are 32 bit values, not 8 bit -- we're loading 4 pixels with
	//each store)
	dest = currentLine = (u32*)spriteInfo->bitmap;
	
	//Find out whether it's an 8 bit palette or a 4 bit one
	if (attribute0 & 0x2000) {	//if it's an 8 bit palette
		spriteInfo->pal256 = 1;
	}
	else {
		spriteInfo->pal256 = 0;
		spriteInfo->palNum = attribute2 >> 12;
	}
	
	//The following copies sprite data into it's own buffer, which in all cases is 64 x 64.
	if (!graphics.use2DTiles) {
		//Calculate the offset into the sprite data (64 (or 32) bytes per index)
		//index = (attribute2 >> 1) & 0x1FF;	//This was when I thought bit 0 didn't count
		index = (attribute2 & 0x3FF);
		source = (u32*)(graphics.spriteData + (index << 5));
		
		//if it's 8 bits per pixel
		if (spriteInfo->pal256) {
			
			//Can't get away with reversing it or the image comes out reversed.
			for (u32 yTile = 0; yTile < yTiles; yTile++) {
				for (u32 xTile = 0; xTile < xTiles; xTile++) {

					dest = currentLine + (xTile << 1);	//move one tile width (2 * 4 bytes)
					//Loop through the y axis of the destination
					for (int i = 8; i--;) {
						dest[0] =*source++;	//Get the first four bytes
						dest[1] = *source++;	//Get the next four bytes
						
						dest+=16;		//each time we increment y, point dest to the next line
					}
				
				}
				currentLine += 128;	//Move over 64 * 8 (or 4)bytes, (128 words)
			}
			
		} else  { //if it's 4 bits per pixel
			
			for (u32 yTile = 0; yTile < yTiles; yTile++) {
				for (u32 xTile = 0; xTile < xTiles; xTile++) {
		
					dest = currentLine + (xTile);	//move one tile width (1 * 4 bytes)
					//Loop through the y axis of the destination
					for (int i = 8; i--;) {
						*dest =*source++;	//Get the first four bytes only
												
						dest+=16;		//each time we increment y, point dest to the next line
					}
				}
				currentLine += 128;	//Move over 64 * 8 bytes, (128 words)
			}
			
		}
	} else {	//If we are in 2d tile mode, everything is upside-down and backwards.
		//Calculate the offset into the sprite data (64 bytes per index)
		index = (attribute2 >> 1) & 0x1FF;
		u32* sourceLine = (u32*)(graphics.spriteData + (index << 6));
		//if it's 8 bits per pixel
		if (spriteInfo->pal256) {
			//Can't get away with reversing it or the image comes out reversed.
			for (u32 yTile = 0; yTile < yTiles; yTile++) {
				source = sourceLine;
				for (u32 xTile = 0; xTile < xTiles; xTile++) {

					dest = currentLine + (xTile << 1);	//move one tile width (2 * 4 bytes)
					//Loop through the y axis of the destination
					for (int i = 8; i--;) {
						dest[0] =*source++;	//Get the first four bytes
						dest[1] = *source++;	//Get the next four bytes
						
						dest+=16;		//each time we increment y, point dest to the next line
					}
				
				}
				sourceLine += 256;	//Move over 32 * 32 bytes
				currentLine += 128;	//Move over 64 * 8 (or 4)bytes, (128 words)
			}

			

		} else  { //if it's 4 bits per pixel
			for (u32 yTile = 0; yTile < yTiles; yTile++) {
				source = sourceLine;
				for (u32 xTile = 0; xTile < xTiles; xTile++) {
		
					dest = currentLine + (xTile);	//move one tile width (1 * 4 bytes)
					//Loop through the y axis of the destination
					for (int i = 8; i--;) {
						*dest =*source++;	//Get the first four bytes only
												
						dest+=16;		//each time we increment y, point dest to the next line
					}
				}
				sourceLine += 256;	//Move over 32 * 32 bytes (32 * 8 words)
				currentLine += 128;	//Move over 64 * 8 bytes, (128 words)
			}
			
		}
	}//if

	

	
	if ( (attribute1 & 0x3000) && !spriteInfo->rotScale) {	//If horizontal flip or vertical flip is on
		int i, j;
		//Copy sprite from the sprite buffer to scratchmem.
		//Is the scratch memory buffer REALLY needed??
		//You really only need to swap values, no??
		source = (u32*)spriteInfo->bitmap;
		dest = (u32*)gbaMem.scratchMem;
		currentLine = (u32*)spriteInfo->bitmap;
		

		source8bit = ((u8*)gbaMem.scratchMem);
		
		//Now flip.
		if (spriteInfo->pal256) {	//If it's a 256 color sprite...
			int numWords = width >> 2;

			for (i = 0; i < height ; i++) {
				source = currentLine;
				for (j = 0; j < numWords; j++) {
					*dest++ = *source++;
				}
				currentLine+=16;	//Move down 64 bytes (width of sprite buffer, not the sprite)
			}
			switch ( (attribute1 >> 12) & 0x3) {
				case 1:	//Flip horizontal
					source8bit = source8bit + width-1;
					currentLine8bit = ((u8*)spriteInfo->bitmap);		
					for (i = 0; i < height; i++) {
						dest8bit = currentLine8bit;
						for (j = 0; j < width; j++) {
							*dest8bit++ = *source8bit--;		
						}
						source8bit= source8bit + (width << 1);	//Move to the next set in the source
						currentLine8bit += 64;	//Go to next line.
					}
					break;
				case 2:	//Flip vertical
					currentLine8bit = ((u8*)spriteInfo->bitmap) + ((height-1) << 6);
					for (i = 0; i < height; i++) {
						dest8bit = currentLine8bit;
						for (j = 0; j < width; j++) {
							*dest8bit++ = *source8bit++;		
						}
						currentLine8bit-=64;	//Go to beginning of previous line.
					}
					break;
				case 3:	//Flip both.
					source8bit = source8bit + width-1;
					currentLine8bit = ((u8*)spriteInfo->bitmap) + ((height-1) << 6);// + width - 1;
					for ( i = 0; i < height; i++) {
						dest8bit = currentLine8bit;
						for (j = 0; j < width; j++) {
							*dest8bit++ = *source8bit--;		
						}
						currentLine8bit-=64;	//Go to beginning of previous line
						source8bit = source8bit + (width << 1); //Move to the next set in the source.
					}
					break;
			}//End of switch
		} else {	//Otherwise it's a 16 color sprite
			
			int numWords = width >> 3;
			for (i = 0; i < height ; i++) {
				source = currentLine;
				for (j = 0; j < numWords; j++) {
					*dest++ = *source++;
				}
				currentLine+=16;	//Move down 64 bytes
			}
			switch ( (attribute1 >> 12) & 0x3) {
				case 1:	//Flip horizontal
					source8bit = source8bit + (width/2)-1;
					currentLine8bit = ((u8*)spriteInfo->bitmap);		
					for (i = 0; i < height; i++) {
						dest8bit = currentLine8bit;
						for (j = 0; j < width/2; j++) {
							byte = *source8bit--;
							*dest8bit++ = (byte >> 4) | (byte << 4);
						}
						source8bit= source8bit + width;//(width << 1);	//Move to the next set in the source
						currentLine8bit += 64;	//Go to next line.
					}
					break;
			case 2:	//Flip vertical
					currentLine8bit = ((u8*)spriteInfo->bitmap) + ((height-1) << 6);
					for (i = 0; i < height; i++) {
						dest8bit = currentLine8bit;
						for (j = 0; j < width/2; j++) {
							*dest8bit++= *source8bit++;
							
						}
						currentLine8bit-=64;	//Go to beginning of previous line.
					}
					break;
			case 3:	//Flip both.
					source8bit = source8bit + (width/2)-1;
					currentLine8bit = ((u8*)spriteInfo->bitmap) + ((height-1) << 6);// + (width) - 1;
					for ( i = 0; i < height; i++) {
						dest8bit = currentLine8bit;
						for (j = 0; j < width/2; j++) {
							byte = *source8bit--;
							*dest8bit++ = (byte >> 4) | (byte << 4);
						}
						currentLine8bit-=64;	//Go to beginning of previous line
						source8bit = source8bit + width;//(width << 1); //Move to the next set in the source.
					}
					break;
			}//End of switch

		}
	}//End of if

	
	//Now save all of our local variables to the sprite.
	//We're done!
	spriteInfo->x = x;
	spriteInfo->y = y;
	spriteInfo->width = width;
	spriteInfo->height = height;

	//Calculate the number of tiles the sprite will take in the x 
	//and y directions
	spriteInfo->xTiles = xTiles;
	spriteInfo->yTiles = yTiles;
	spriteInfo->oamInfo = oamInfo;
	
	index = (attribute1 >> 9) & 0x1F;	//Get index into rotate/scale data
	spriteInfo->oamRSInfo = &(graphics.OAMRS[index]);
	return 1;
}

//////////////////////////////////////////
//renderSprite()
//Renders a sprite to the current scanline (no rot/scale or alpha)
//Returns:  the number of sprite tiles rendered.
//////////////////////////////////////////
inline int Graphics_renderSprite(const SpriteInfo* spriteInfo, u16* dest, u32 scanY, 
													const Window* window, u32* blendArray, u32 blendFlags) {
	s32 y;
	s32 x;
	s32 spriteY;	//Y value relative to sprite
	s32 spriteX;
	s32 screenStartX;	//X start value relative to screen
	s32 spriteWidth;
	s32 spriteHeight;
	s32 numBytes;
	u8* source;
	y = spriteInfo->y;
	spriteHeight = spriteInfo->height;
			
		
	//If the scanline intersects with this sprite...
	//if ( (spriteY >= 0) && (spriteY < spriteInfo->height)) {
	if (y >= 160)
		spriteY = scanY + 256 - y;
	else
		spriteY = scanY - y;
			
	//if spriteY is within bounds...
	if ( (spriteY >= 0) && (spriteY < spriteHeight) ) {

		x = spriteInfo->x;

		//spriteX = spriteInfo->x;
		spriteWidth = spriteInfo->width;

		//If the sprite is halfway off the left side of the window,
		//but not off the left side of the screen...
		if (x < window->winLeft) {
			spriteX = window->winLeft - x;
			numBytes = spriteWidth - spriteX;
			screenStartX = window->winLeft;

		} else if (x >= 240) {	//If the sprite is off the left side of the screen
			spriteX = window->winLeft + 512 - x;
			
			if (spriteX >= spriteWidth)
				return 0;

			numBytes = spriteWidth - spriteX;
			//screenStartX = 0;
			screenStartX = window->winLeft;
		} else {
			
			spriteX = 0;
			numBytes = spriteWidth;
			screenStartX = x;
		}

		//If we go past win right, clip it.
		if ((screenStartX + numBytes) > window->winRight) {
			numBytes = window->winRight - screenStartX;
		}
		if (numBytes <= 0)
			return 0;

		//Source is y * 64 bytes into the bitmap plus the starting value
		source = (u8*)spriteInfo->bitmap + (spriteY << 6);// + spriteX;
		
		//Dest is the current pointer plus the starting x value
		//dest = (u16*)(graphics.screen16bit + graphics.lcy* 240 + screenStartX);
		//attribute1 = spriteInfo->oamInfo->attribute1;

		Graphics_renderSpriteBits(spriteInfo, source, spriteX, (u16*)(dest+screenStartX), numBytes, 
											(u32*)(blendArray+screenStartX), blendFlags);

		return spriteInfo->xTiles;
	} else return 0;
	
}



/////////////////////////////////////////
//renderSpriteBits
//Gets called by renderSprite
//Renders the bits of the sprite, without fade/alpha.
//////////////////////////////////////////
inline void Graphics_renderSpriteBits(const SpriteInfo* spriteInfo, u8* source, u32 startX, 
													u16* dest, u32 numBytes,
													u32* blendArray, u32 blendFlags) {
	u32 pixel;
	u32 oldBlendFlags;

	if (!numBytes)
		return;
	//if (numBytes >= 240)	//Not sure why I had this.
	//	numBytes = 240;

	//Check to see if the sprite is transparent
	if (blendFlags != 0xFFFFFFFF) {	//Pass if blend is not desired.
		//if ( ((spriteInfo->oamInfo->attribute0 >> 10) & 0x3) == 0x1) {
		
		//alpha = *REG_BLDALPHA;
		//if (alpha != 0)
		//	int blah = 0;
		//sourceMul = alpha & 0x1F;
		//destMul = (alpha >> 8) & 0x1F;
		//u32 mulOffset = (sourceMul << 15) + (destMul << 10);
		
		if (spriteInfo->pal256) {
			source += startX;
			while (numBytes--) {
				pixel = *source++;
				if (pixel) {
					*dest = graphics.spritePalette[pixel];
					if (blendFlags & 0xF0) {	//If this is a source blend...
						oldBlendFlags = *blendArray;	//Get the current flags for this pixel
						
						//If the last source or target was also a sprite, we don't want to blend with it,
						//but with whatever layer was below it.
						if (((oldBlendFlags >> 12) >= 6) || ( (oldBlendFlags & 0xF0) >= 0x60 )) {
							//Mask out the old current source and next target
							*blendArray = blendFlags | (oldBlendFlags & 0xFFFF0F0F);	
						} else {
							*blendArray = blendFlags | (oldBlendFlags >> 12);	//Set a new current target.
						}
					} else{
						*blendArray = blendFlags;
					}
					
				} 
				dest++;
				blendArray++;
			}//for
		} else {	//if it's 4 bit...
			source+=(startX >> 1); //Get the byte offset.

			numBytes = numBytes >> 1;
			u8 pixel1, pixel2;
			u32 palNum = spriteInfo->palNum;

			//Get a pointer to the palette by multiplying the palNum by 16 entries
			u16* spritePalette16 = graphics.spritePalette + (palNum << 4);

			if (startX & 0x1) {	//If startX is an odd number, we must draw the first pixel.
				pixel = *source++;
				pixel = pixel >> 4;
				if (pixel) {
					*dest = spritePalette16[pixel];
					if (blendFlags & 0xF0) {	//If this is a source blend...
						oldBlendFlags = *blendArray;	//Get the current flags for this pixel
						//If the last source or target was also a sprite, we don't want to blend with it,
						//but with whatever layer was below it.
						if (((oldBlendFlags >> 12) >= 6) || ( (oldBlendFlags & 0xF0) >= 0x60 )) {
							//Mask out the old current source and next target
							*blendArray = blendFlags | (oldBlendFlags & 0xFFFF0F0F);	
						} else {
							*blendArray = blendFlags | (oldBlendFlags >> 12);	//Set a new current target.
						}
					} else{
						*blendArray = blendFlags;
					}
				}
				dest++;
				blendArray++;

				if (!numBytes)
					return;
			}

			//Finally, draw the picture.
			while (numBytes--)  {
				//*dest = 0;
				pixel = *source++;
				pixel1 = pixel & 0x0F;
				pixel2 = pixel >> 4;
				if (pixel1) {
					*dest = spritePalette16[pixel1];
					if (blendFlags & 0xF0) {	//If this is a source blend...
						oldBlendFlags = *blendArray;	//Get the current flags for this pixel
						//If the last source or target was also a sprite, we don't want to blend with it,
						//but with whatever layer was below it.
						if (((oldBlendFlags >> 12) >= 6) || ( (oldBlendFlags & 0xF0) >= 0x60 )) {
							//Mask out the old current source and next target
							*blendArray = blendFlags | (oldBlendFlags & 0xFFFF0F0F);	
						} else {
							*blendArray = blendFlags | (oldBlendFlags >> 12);	//Set a new current target.
						}
					} else{
						*blendArray = blendFlags;
					}
				}
				dest++;
				blendArray++;
				
				if (pixel2) {
					*dest = spritePalette16[pixel2];
					if (blendFlags & 0xF0) {	//If this is a source blend...
						oldBlendFlags = *blendArray;	//Get the current flags for this pixel
						//If the last source or target was also a sprite, we don't want to blend with it,
						//but with whatever layer was below it.
						if (((oldBlendFlags >> 12) >= 6) || ( (oldBlendFlags & 0xF0) >= 0x60 )) {
							//Mask out the old current source and next target
							*blendArray = blendFlags | (oldBlendFlags & 0xFFFF0F0F);	
						} else {
							*blendArray = blendFlags | (oldBlendFlags >> 12);	//Set a new current target.
						}
					} else{
						*blendArray = blendFlags;
					}
				}
				dest++;
				blendArray++;
			}//for
			
		}
	} else {
		if (spriteInfo->pal256) {
			source+=startX;
			while (numBytes--) {
				pixel = *source++;
				if (pixel) 
					*dest = graphics.spritePalette[pixel];
				dest++;
			}//for
		} else {	//if it's 4 bit...
			source+=(startX >> 1);
			numBytes = numBytes >> 1;
			u8 pixel1, pixel2;
			u32 palNum = spriteInfo->palNum;

			//Get a pointer to the palette by multiplying the palNum by 16 entries
			u16* spritePalette16 = graphics.spritePalette + (palNum << 4);
			
			
			if (startX & 0x1) {	//If startX is an odd number, we must draw the first pixel.
					
				pixel = *source++;
				pixel = pixel >> 4;
				if (pixel)
					*dest = spritePalette16[pixel];
				dest++;
				if (!numBytes)	//If the shifting operation above reduced numBytes to 0
					return;
			}
			

			//Finally, draw the picture.
			while (numBytes--)  {
				//*dest = 0;
				pixel = *source++;
				pixel1 = pixel & 0x0F;
				pixel2 = pixel >> 4;
				if (pixel1) 		
					*dest = spritePalette16[pixel1];
				dest++;
				
				if (pixel2)
					*dest = spritePalette16[pixel2];
					//*dest = 0;
				dest++;
			}//for

			
		}

	}
}

/////////////////////////////////////////
//This function blends two pixels.
/////////////////////////////////////////
inline u32 Graphics_alphaBlend(u32 sourceColor, u32 destColor, u32 mulOffset) {
	u32 sr, sg, sb, dr, dg, db;

	//mullOffset = (sourceMul << 15) + (destMul << 10);
	sr = sourceColor & 0x1F;
	dr = destColor & 0x1F;
	dr = graphics.blendLUT[mulOffset + (sr << 5) + dr];

	sg = sourceColor & 0x03E0;		//Eliminate a shift operation -- was: (sourceColor >> 5) & 0x1F;
	dg = (destColor >> 5) & 0x1F;
	dg = graphics.blendLUT[mulOffset + sg + dg];

	sb = (sourceColor >> 5) & 0x03E0;	//Eliminates a shift operation -- was: (sourceColor >> 5) & 0x1F;
	db = (destColor >> 10) & 0x1F;
	db = graphics.blendLUT[mulOffset + sb + db];


	return (dr + (dg << 5) + (db << 10));
}

//////////////////////////////////////////
//renderRSSprite()
//Renders a sprite to the current scanline, taking
//into account rotate, scale, and alpha.
//Returns:  none
//////////////////////////////////////////
inline void Graphics_renderRSSprite(const SpriteInfo* spriteInfo, u16* dest, s32 y, const Window* window, 
											u32* blendArray, s32 blendFlags) {
	s32 x, spriteX, spriteY, X, Y, winLeft, winRight; 
	s32 startX, endX, startY, endY, numPixels;
	s32 spriteWidth, spriteHeight, midX, midY;
	u32 pixel;
	u32 oldBlendFlags;
	u16* palette;
	OAMRSInfo* oamRSInfo = spriteInfo->oamRSInfo;		

	//Rotational Registers:
	s32 DY = oamRSInfo->DY;
	s32 DMX = oamRSInfo->DMX;
	s32 DX = oamRSInfo->DX;
	s32 DMY = oamRSInfo->DMY;

	spriteHeight = spriteInfo->height;
	spriteWidth = spriteInfo->width;
	midY = spriteHeight >> 1;
	midX = spriteWidth >> 1;

	//Get the sign bit (important)
	X = spriteInfo->x;
	Y = spriteInfo->y;
	
	if (X >= 240) {
		X = X - 512;	//This makes it a negative number.  The GBA just loops it around,
	}
	
	if (Y >= 160) {
		Y = Y - 256;
	}

	winLeft = window->winLeft;
	winRight = window->winRight;

	//NOTE:  This is not the authentic method of doing it.  In this method, if a 128 x 128 sprite
	//goes off the bottom of the screen, it doesn't loop around to the top until the very top
	//pixel disappears off the bottom (at y = 160).  On a real GBA, the sprite "disappears"
	//completely off the bottom half of the screen when y = 128, and loops around to the top
	//so that the very bottom pixel is visible.  In other words it depends on the bottom pixel,
	//whereas mine depends on the top pixel.

	startX = X;	//Start drawing at the original x value
	startY = Y;
	endX = startX + spriteInfo->effectiveWidth;
	endY = startY + spriteInfo->effectiveHeight;


	if (startX < winLeft)
		startX = winLeft;
	if (startY < 0)
		startY = 0;

	//If the scanline doesn't ever intersect this sprite
	if ( (y >= endY) || (y < startY))
		return;
	

	if (endX >= winRight)
		endX = winRight;
	if (endY >= 160)
		endY = 160;

	

	if ((spriteInfo->oamInfo->attribute0 >> 9) & 0x1)	{	//If double size is on...
		X += spriteWidth;	//Translate the UL pixel to be in the middle of the double sized space
		Y += spriteHeight;
	} else {
		//This translates the UL pixel to be in the center of the normal sized space.
		X = X + midX;
		Y = Y + midY;
	}
/*
	if ( !((spriteInfo->oamInfo->attribute0 >> 9) & 0x1))	{	//If double size is off...
		X -= midX;	//Move the sprite up and to the left from where
		Y -= midY;
	}*/

	
	
	
	numPixels = spriteInfo->effectiveWidth;
	

	//This method of rendering a rotating sprite is ass backwards and slow.
	//But it's easy.
	DY = DY << 16;
	DY = DY >> 16;

	DMY = DMY << 16;
	DMY = DMY >> 16;
	
	DX = DX << 16;
	DX = DX >> 16;

	DMX = DMX << 16;
	DMX = DMX >> 16;
	
	//The code for scaling was pretty much taken from hearn and baker, p.188
	//However, you can also get these results by multiplying the matrix 
	//pa pb 0
	//pc pd 0
	//0  0  1
	//by -midX, - midY, translating it back, then applying the resulting matrix to x and y.
	
	//This gets the y coordinate for the pixel at the center of the sprite.
	//Subtract midY for calculation efficiency (for some reason I used to subtract it
	//during the render loop.  Why did I do that?)
	spriteY = y - Y;// - midY;	
	s32 spriteYS =  DMY * spriteY;// - (midY << 8);	//Scale the image in y
	s32 spriteShearX = DMX * spriteY;

	dest += startX;	//Start from the left edge of the sprite.
	blendArray += startX;
	
	if (spriteInfo->pal256) {
		palette = &gbaMem.u16Pal[256];
		for (x = startX; x < endX; x++) {
			spriteX = x - X;// - midX;
			
			s32 spriteYR = spriteYS + (DY * spriteX);	//Shear the image in y
			spriteYR = spriteYR >> 8;
			spriteYR = spriteYR + midY;	//Translate back to where the sprite was.
			
			s32 spriteXS = (DX * spriteX);				//Scale the image in x
			s32 spriteXR = spriteXS + spriteShearX;	//Shear the image in x
			spriteXR = spriteXR >> 8;
			spriteXR = spriteXR + midX;	//Translate back to where the sprite was.
			
			
			//This does bounds checking in reverse -- finds out if the pixel
			//at the specified coordinates is in bounds, prints it if so.
			//Note that even though I do preliminary bounds checking, I cannot skip this step,
			//since my bounds checking does not apply the transforms to the preliminary bounds.
			if ( (spriteYR >=0) && (spriteYR < spriteHeight)) {
				if ( (spriteXR >=0) && (spriteXR <spriteWidth) ) {
					//Remember that I chose to load sprites as 64 bytes long
					pixel = spriteInfo->bitmap[spriteXR + (spriteYR << 6)];
					
					if (pixel) {
						*dest = palette[pixel];
						if (blendFlags >= 0) {
							if (blendFlags & 0xF0) {	//If this is a source blend...
								oldBlendFlags = *blendArray;	//Get the current flags for this pixel
								//If the last source or target was also a sprite, we don't want to blend with it,
								//but with whatever layer was below it.
								if (((oldBlendFlags >> 12) >= 6) || ( (oldBlendFlags & 0xF0) >= 0x60 )) {
									//Mask out the old current source and next target
									*blendArray = blendFlags | (oldBlendFlags & 0xFFFF0F0F);	
								} else {
									*blendArray = blendFlags | (oldBlendFlags >> 12);	//Set a new current target.
								}
							} else{
								*blendArray = blendFlags;
							}
						}//if
					}//if
				}
			}//if
			dest++;
			blendArray++;
		}//for
	} else {
		palette = &gbaMem.u16Pal[256 + (spriteInfo->palNum << 4)];
		for (x = startX; x < endX; x++) {
			spriteX = x - X;// - midX;
			
			s32 spriteYR = spriteYS + (DY * spriteX);	//Shear the image in y
			spriteYR = spriteYR >> 8;
			spriteYR = spriteYR + midY;	//Translate back to where the sprite was.
			
			s32 spriteXS = (DX * spriteX);				//Scale the image in x
			s32 spriteXR = spriteXS + spriteShearX;	//Shear the image in x
			spriteXR = spriteXR >> 8;
			spriteXR = spriteXR + midX;	//Translate back to where the sprite was.
			
			if ( (spriteYR >=0) && (spriteYR < spriteHeight)) {
				if ( (spriteXR >=0) && (spriteXR <spriteWidth) ) {
					//Remember that I chose to load sprites as 64 bytes long
					//But since this is 4 bit pixels, halve the x index.
					pixel = spriteInfo->bitmap[ (spriteXR >> 1) + (spriteYR << 6)];
					
					//Now split up the pixel and get it's body parts.
					if (spriteXR & 0x1)					//If we want the second of the two pixels..
						pixel = pixel >> 4;			//Slide it over.
					else	
						pixel = pixel & 0xF;			//Otherwise wipe off the top one.
					
					if (pixel) 	{					//Must take into account palNum 16 color palettes.
						*dest = palette[pixel];
						if (blendFlags >= 0) {
							if (blendFlags & 0xF0) {	//If this is a source blend...
								oldBlendFlags = *blendArray;	//Get the current flags for this pixel
								//If the last source or target was also a sprite, we don't want to blend with it,
								//but with whatever layer was below it.
								if (((oldBlendFlags >> 12) >= 6) || ( (oldBlendFlags & 0xF0) >= 0x60 )) {
									//Mask out the old current source and next target
									*blendArray = blendFlags | (oldBlendFlags & 0xFFFF0F0F);	
								} else {
									*blendArray = blendFlags | (oldBlendFlags >> 12);	//Set a new current target.
								}
							} else{
								*blendArray = blendFlags;
							}
						}//if

					}
				}//if			
			}//if	
			dest++;
			blendArray++;
		}//for
	}//else

}

/////////////////////////////////////////////////
//graphics.renderRSBgPixel()
//This function takes an x-y coordinate, a destination pointer,
//and renders it based on the map coordinates
///////////////////////////////////////////////////
inline void Graphics_renderRSBgPixel(const BackgroundInfo* bgInfo, u16* dest, u32 bgX, u32 bgY,
													u32* blendArray, s32 blendFlags) {
	u32 tileX, tileY;
	u32 offset;		//Offset into background data.
	u32 tileOffsetX, tileOffsetY;	//Offset within the tile.
	u32 palEntry;
	u8 mapEntry;
	
	u8* tileData = bgInfo->tileData;

	//Calculate the offset into the tile map by dividing by 8.
	tileX = bgX >> 3;
	tileY = bgY >> 3;

	u8* tileMap8 = (u8*)(bgInfo->tileMap);	
	mapEntry = tileMap8[(tileY << bgInfo->xBitWidth) + tileX];	//Uncomment for mode 2

	offset = mapEntry << 6;	//Multiply the offset by 64 bytes

	tileOffsetX = (bgX & 0x7);	//Wipe off the lower bits to get a value
	tileOffsetY = (bgY & 0x7);	//from 0 - 7.

	//This should look a little familiar -- just getting an
	//index into the tile.
	offset = offset + (tileOffsetY << 3) + tileOffsetX;

	palEntry = tileData[offset];
	if (palEntry) {
		*dest = gbaMem.u16Pal[palEntry];
		if (blendFlags >= 0) {
			if (blendFlags & 0xF0) {	//If this is a source blend...
				u32 oldBlendFlags = *blendArray;	//Get the current flags for this pixel
				*blendArray = blendFlags | (oldBlendFlags >> 12);	//Set the current target.
			} else {
				*blendArray = blendFlags;
			}
		}
	}
}




/////////////////////////////////////////////////////
//This renders a pixel from the given map entry.
//Only use this in places where speed is not an issue.
//////////////////////////////////////////////////////
inline void Graphics_renderBGPixel(const BackgroundInfo* bgInfo, u16* dest, 
													u32 mapEntry, u32 bgX, u32 bgY) {
	u32 tileOffsetX, tileOffsetY;
	u32 offset = mapEntry & 0x3FF;
	u16 palEntry;
	
	offset = offset << 6;	//Multiply the offset by 64 bytes

	if (mapEntry & TILE_FLIPX)
		tileOffsetX = 7 - (bgX & 0x7);	//Wipe off the lower bits to get a value
	else 
		tileOffsetX = (bgX & 0x7);	//Wipe off the lower bits to get a value
	
	if (mapEntry & TILE_FLIPY)
		tileOffsetY = 7 - (bgY & 0x7);	//from 0 - 7.
	else 
		tileOffsetY = (bgY & 0x7);	//from 0 - 7.

	//This should look a little familiar -- just getting an
	//index into the tile.
	offset = offset + (tileOffsetY << 3) + tileOffsetX;

	//This could probably be faster...
	if (bgInfo->pal256) {
		palEntry = bgInfo->tileData[offset];
		if (palEntry) {
			*dest = gbaMem.u16Pal[palEntry];
		}
	} else {
		u32 palNum = mapEntry >> 12;
		//Find the 16 color palette entry...
		palEntry = bgInfo->tileData[offset >> 1];	//This gets two pixels.
		if (offset & 0x1)					//If we want the second of the two pixels..
			palEntry = palEntry >> 4;		//Slide it over.
		else	
			palEntry = palEntry & 0xF;		//Otherwise wipe off the top one.
		
		if (palEntry)						//Must take into account palNum 16 color palettes.
			*dest = gbaMem.u16Pal [(palNum << 4) + palEntry];
	}

}

/////////////////////////////////////////////////////
//This renders a pixel from the given map entry.
//Only use this in places where speed is not an issue.
//////////////////////////////////////////////////////
inline void Graphics_renderBGBlendPixel(const BackgroundInfo* bgInfo, u16* dest, 
														u32 mapEntry, u32 bgX, u32 bgY, 
														u32* blendArray, u32 blendFlags) {
	u32 tileOffsetX, tileOffsetY;
	u32 offset = mapEntry & 0x3FF;
	u16 palEntry;
	
	offset = offset << 6;	//Multiply the offset by 64 bytes

	if (mapEntry & TILE_FLIPX)
		tileOffsetX = 7 - (bgX & 0x7);	//Wipe off the lower bits to get a value
	else 
		tileOffsetX = (bgX & 0x7);	//Wipe off the lower bits to get a value
	
	if (mapEntry & TILE_FLIPY)
		tileOffsetY = 7 - (bgY & 0x7);	//from 0 - 7.
	else 
		tileOffsetY = (bgY & 0x7);	//from 0 - 7.

	//This should look a little familiar -- just getting an
	//index into the tile.
	offset = offset + (tileOffsetY << 3) + tileOffsetX;

	//This could probably be faster...
	if (bgInfo->pal256) {
		palEntry = bgInfo->tileData[offset];
		if (palEntry) {
			*dest = gbaMem.u16Pal[palEntry];
			if (blendFlags & 0xF0) {	//If this is a source blend...
				u32 oldBlendFlags = *blendArray;	//Get the current flags for this pixel
				*blendArray = blendFlags | (oldBlendFlags >> 12);	//Set the current target.
			} else {
				*blendArray = blendFlags;
			}

		}
	} else {
		u32 palNum = mapEntry >> 12;
		//Find the 16 color palette entry...
		palEntry = bgInfo->tileData[offset >> 1];	//This gets two pixels.
		if (offset & 0x1)					//If we want the second of the two pixels..
			palEntry = palEntry >> 4;		//Slide it over.
		else	
			palEntry = palEntry & 0xF;		//Otherwise wipe off the top one.
		
		if (palEntry) {			//Must take into account palNum 16 color palettes.
			*dest = gbaMem.u16Pal [(palNum << 4) + palEntry];
			if (blendFlags & 0xF0) {	//If this is a source blend...
				u32 oldBlendFlags = *blendArray;	//Get the current flags for this pixel
				*blendArray = blendFlags | (oldBlendFlags >> 12);	//Set the current target.
			} else {
				*blendArray = blendFlags;
			}
			
		}
	}

}


//////////////////////////////////////////
//renderTileBG()
//Renders background to the current scanline (no rot/scale or alpha)
//Returns:  none
//////////////////////////////////////////
void Graphics_renderTileBG(const BackgroundInfo* bgInfo, u16* dest, u32 y, const Window* window) {
	register u32 scy;
	register u32 scx;
	register u32 bgY;	//Y value relative to bg
	register u32 bgX;
	register u32 bgWidth;
	register u32 bgHeight;
	register u32 heightMask, widthMask, tileWidthMask;		//A masking value.
	
	bgHeight = bgInfo->height;
	bgWidth = bgInfo->width;

	//bgHeight = bgWidth = 0;
								//if width = 128, mask = 0x7F
	heightMask = bgHeight -1;	//If width = 256, mask = 0xFF		
	widthMask = bgWidth - 1;	//If width = 512, mask = 0x1FF
	tileWidthMask = widthMask >> 3;	//If mask = 0xFF, mask = 0x1F (for 32 tiles)

	//Get the y and x positions of the background
	//Note that scx and scy are offsets, not coordinates, so we must subtract
	//Should give us a value from 0 - width and 0 - height
	scx = (0x400 - *(bgInfo->scx)) & widthMask;		//Must mask them out so that values
	scy = (0x400 - *(bgInfo->scy)) & heightMask;	//Greater than the width roll over.
	
		
	//Calculate where this scanline intersects the background.
	if (scy <= y)
		bgY = y - scy;
	else
		bgY = bgHeight - scy + y;
	

	//Looks weird, but basically what it does is make them
	//roll over whenever they are greater than bgWidth and bgHeight.
	//Am I in trouble for doing it twice??  I don't know . . .
	bgY = bgY & heightMask;	
	bgX = (bgWidth - scx + window->winLeft) & widthMask;

	
	//Calculate the offset into the tile map by dividing by 8.
	u32 tileX = bgX >> 3;
	u32 tileY = bgY >> 3;
	
	u32 xTiles;
	
	
	u16 mapEntry;
	
	u32 offset;
	u32 tileOffsetY;
	u32 flippedY = (7 - (bgY & 0x7)) << 3;
	u32 normalY = (bgY & 0x7) << 3;

	u16* tileMap = bgInfo->tileMap;
	u8* tileData = bgInfo->tileData;

	
	//Start reading from the tile map at a different place depending on the tile Y value
	if (bgInfo->sizeCode & 0x1) {
		if (tileY < 32)
			tileMap = &tileMap[(tileY << 5)];
		else
			tileMap = &tileMap[2048 + ((tileY-32) << 5)];
	} else 
		tileMap = &tileMap[(tileY << 5)];
	
	//u32 numLeftPixels = (8 - (bgX & 0x7)) & 0x7;	//Number of pixels to render on the winLeft edge
	u32 numRightPixels = (bgX + (window->winWidth)) & 0x7;	//Number of pixels to render on the winRight edge
	
	xTiles = (window->winWidth - numRightPixels) >> 3;

	//(winWidth - numRightPixels - numLeftPixels) >> 3;	//Get a raw calculation for the number of tiles

	dest+= window->winLeft;	//Now it's safe to increment dest to the winLeft edge.


	//////////////////////////////////////////////////////////////////////////
	//If the image was clipped on a non 8-pixel boundary,
	//we have to render those tiles. Rendering of the clipped tiles on the
	//edges is non-optomized, because they only represent 1/30th of the
	//rendering time and it would bloat the function a lot.
	////////////////////////////////////////////////////////////////////////
	if (numRightPixels) {	//First do the winRight edge.
		//Render the first part of the first tile and the last part of the last tile
		u32 lastTileX = ((bgX + window->winWidth) >> 3) & tileWidthMask;
		u16 lastMapEntry;
	
		if (lastTileX < 32)	//If it's in the winLeft segment
			lastMapEntry = tileMap[lastTileX];
		else 				//Otherwise it's in the winRight segment
			lastMapEntry = tileMap[992 + lastTileX];

		//u32 tempX = 240 - numPixels;
		u32 tempX = window->winWidth - numRightPixels;
		u16* tempDest = &dest[tempX];
		tempX = (bgX + tempX) & widthMask;
		//tempX = (bgX + winWidth-numRightPixels) & widthMask;
		
		//Render the last tile up until the end of the screen.
		while ( numRightPixels--) {
			Graphics_renderBGPixel(bgInfo, tempDest++, lastMapEntry, tempX, bgY);
			tempX = (tempX + 1) & widthMask;
			
		}
		//If the last tile and the first tile are the same, just render the last tile.
		if (window->winWidth < 8)
			if (tileX == lastTileX)
				return;
	}//if

	
	//if (numLeftPixels)	{	//If x is not perfectly aligned to a tile

	if (bgX & 0x7) {
			
		if (tileX < 32)	//If it's in the winLeft segment
			mapEntry = tileMap[tileX];
		else 				//Otherwise it's in the winRight segment
			mapEntry = tileMap[992 + tileX];
			
		//Render the first tile up until we reach the next tile.
		while ( bgX & 0x7 ) {
			Graphics_renderBGPixel(bgInfo, dest++, mapEntry, bgX, bgY);
			bgX = (bgX+1) & widthMask;
			
		}
		tileX = bgX >> 3;	//Go to first complete x tile.
	} 
	

	////////////////////////////////////////////////////
	//Render the tiles as 256 color
	////////////////////////////////////////////////////
	if (bgInfo->pal256) {
		//Loop through all remaining x tiles visible on screen
		while (xTiles--) {
			
			if (tileX < 32)	//If it's in the winLeft segment
				mapEntry = tileMap[tileX];
			else 				//Otherwise it's in the winRight segment
				mapEntry = tileMap[992 + tileX];
						
			offset = ((mapEntry & 0x3FF) << 6);
			
			//Flip vertically
			if (mapEntry & TILE_FLIPY)
				tileOffsetY = flippedY;	//from 7 - 0.
			else 
				tileOffsetY = normalY;	//from 0 - 7.

			//Flip horizontally
			if (mapEntry & TILE_FLIPX) {
				offset = offset + tileOffsetY + 7;
				for ( int i = 8; i--;) {
					u8 palEntry = tileData[offset--];
					if (palEntry) {
						*dest = gbaMem.u16Pal[palEntry];
					}
					dest++;
				}
			} else {	//Don't flip horizontally.
				offset = offset + tileOffsetY;
				for ( int i = 8; i--;) {
					u8 palEntry = tileData[offset++];
					if (palEntry) {
						*dest = gbaMem.u16Pal[palEntry];
					}
					dest++;
				}	
			}
			tileX = (tileX + 1) & tileWidthMask;	//Go to the next tile.
		}
	
	////////////////////////////////////////////////////
	//Render the tiles as 16 color
	////////////////////////////////////////////////////
	} else {		//16 color
		//Loop through all remaining x tiles visible on screen
		while (xTiles--) {
			
			if (tileX < 32)	//If it's in the winLeft segment
				mapEntry = tileMap[tileX];
			else 				//Otherwise it's in the winRight segment
				mapEntry = tileMap[992 + tileX];
						
			offset = ((mapEntry & 0x3FF) << 6);
			u16* palette = &gbaMem.u16Pal[(mapEntry >> 8) & 0xF0];
			
			//Flip vertically
			if (mapEntry & TILE_FLIPY)
				tileOffsetY = flippedY;	//from 7 - 0.
			else 
				tileOffsetY = normalY;	//from 0 - 7.

			//Flip horizontally
			if (mapEntry & TILE_FLIPX) {
				offset = (offset + tileOffsetY + 7) >> 1;
				
				for (int i = 4; i--;) {
					u8 doubleEntry = tileData[offset];	//This gets two pixels.
					u8 palEntry1 = doubleEntry & 0xF;
					u8 palEntry2 = doubleEntry >> 4;
					if (palEntry2)
						*dest = palette[palEntry2];		//Get the left pixel
					dest++;
					if (palEntry1)
						*dest = palette[palEntry1];		//Get the right pixel
					dest++;
					offset--;
				}
			
			} else {	//Don't flip horizontally.
				offset = (offset + tileOffsetY) >> 1;
						
				for (int i = 4; i--;) {
					u8 doubleEntry = tileData[offset];	//This gets two pixels.
					u8 palEntry1 = doubleEntry & 0xF;
					u8 palEntry2 = doubleEntry >> 4;
					if (palEntry1)
						*dest = palette[palEntry1];		//Get the right pixel
					dest++;
					if (palEntry2)
						*dest = palette[palEntry2];		//Get the left pixel
					dest++;
					offset++;
				}
			}
			tileX = (tileX + 1) & tileWidthMask;	//Go to the next tile.
		}//while
	}//else
	return;
}

//////////////////////////////////////////
//renderTileBG()
//Renders background to the current scanline (no rot/scale or alpha)
//Returns:  none
//////////////////////////////////////////
void Graphics_renderTileBGBlend(const BackgroundInfo* bgInfo, u16* dest, u32 y, const Window* window,
											u32* blendArray, u32 blendFlags) {
	u32 scy, scx, bgY, bgX, bgWidth, bgHeight;
	u32 heightMask, widthMask, tileWidthMask;		//A masking value.
	u32 xTiles, offset, tileOffsetY;
	u16 mapEntry;
	
	bgHeight = bgInfo->height;
	bgWidth = bgInfo->width;
										//if width = 128, mask = 0x7F
	heightMask = bgHeight -1;	//If width = 256, mask = 0xFF		
	widthMask = bgWidth - 1;	//If width = 512, mask = 0x1FF
	tileWidthMask = widthMask >> 3;	//If mask = 0xFF, mask = 0x1F (for 32 tiles)

	//Get the y and x positions of the background
	//Note that scx and scy are offsets, not coordinates, so we must subtract
	//Should give us a value from 0 - width and 0 - height
	scx = (0x400 - *(bgInfo->scx)) & widthMask;		//Must mask them out so that values
	scy = (0x400 - *(bgInfo->scy)) & heightMask;	//Greater than the width roll over.
	
	//Calculate where this scanline intersects the background.
	if (scy <= y)
		bgY = y - scy;
	else
		bgY = bgHeight - scy + y;

	//Looks weird, but basically what it does is make them
	//roll over whenever they are greater than bgWidth and bgHeight.
	//Am I in trouble for doing it twice??  I don't know . . .
	bgY = bgY & heightMask;	
	bgX = (bgWidth - scx + window->winLeft) & widthMask;
	
	//Calculate the offset into the tile map by dividing by 8.
	u32 tileX = bgX >> 3;
	u32 tileY = bgY >> 3;
	
	u32 flippedY = (7 - (bgY & 0x7)) << 3;
	u32 normalY = (bgY & 0x7) << 3;

	u16* tileMap = bgInfo->tileMap;
	u8* tileData = bgInfo->tileData;

	
	//Start reading from the tile map at a different place depending on the tile Y value
	if (bgInfo->sizeCode & 0x1) {
		if (tileY < 32)
			tileMap = &tileMap[(tileY << 5)];
		else
			tileMap = &tileMap[2048 + ((tileY-32) << 5)];
	} else 
		tileMap = &tileMap[(tileY << 5)];
	
	
	u32 numRightPixels = (bgX + (window->winWidth)) & 0x7;	//Number of pixels to render on the winRight edge
	
	xTiles = (window->winWidth - numRightPixels) >> 3;
	
	dest+= window->winLeft;	//Now it's safe to increment dest to the winLeft edge.
	blendArray += window->winLeft;

	//////////////////////////////////////////////////////////////////////////
	//If the image was clipped on a non 8-pixel boundary,
	//we have to render those tiles. Rendering of the clipped tiles on the
	//edges is non-optomized, because they only represent 1/30th of the
	//rendering time and it would bloat the function a lot.
	////////////////////////////////////////////////////////////////////////
	if (numRightPixels) {	//First do the winRight edge.
		//Render the first part of the first tile and the last part of the last tile
		u32 lastTileX = ((bgX + window->winWidth) >> 3) & tileWidthMask;
		u16 lastMapEntry;
		
			

		if (lastTileX < 32)	//If it's in the winLeft segment
			lastMapEntry = tileMap[lastTileX];
		else 				//Otherwise it's in the winRight segment
			lastMapEntry = tileMap[992 + lastTileX];

		u32 tempX = window->winWidth - numRightPixels;
		u16* tempDest = &dest[tempX];
		u32* tempBlendArray = &blendArray[tempX];
		tempX = (bgX + tempX) & widthMask;
		
		

		//Render the last tile up until the end of the screen.
		while ( numRightPixels--) {
			//Graphics_renderBGPixel(bgInfo, tempDest++, lastMapEntry, tempX, bgY);
			Graphics_renderBGBlendPixel(bgInfo, tempDest++, lastMapEntry, tempX, bgY, 
																tempBlendArray++, blendFlags);
			tempX = (tempX + 1) & widthMask;
			
		}
		//If this the last tile and the first tile are the same, just render the last tile.
		if (window->winWidth < 8)
			if (tileX == lastTileX)
				return;
	}//if*/
	
	
	//if (numLeftPixels)	{	//If x is not perfectly aligned to a tile
	if (bgX & 0x7) {
			
		if (tileX < 32)	//If it's in the winLeft segment
			mapEntry = tileMap[tileX];
		else 				//Otherwise it's in the winRight segment
			mapEntry = tileMap[992 + tileX];
			
		//Render the first tile up until we reach the next tile.
		while ( bgX & 0x7 ) {
			//Graphics_renderBGPixel(bgInfo, dest++, mapEntry, bgX, bgY);
			Graphics_renderBGBlendPixel(bgInfo, dest++, mapEntry, bgX, bgY,
											blendArray++, blendFlags);
			bgX = (bgX+1) & widthMask;
			
		}
		tileX = bgX >> 3;	//Go to first complete x tile.
	} 
	

	////////////////////////////////////////////////////
	//Render the tiles as 256 color
	////////////////////////////////////////////////////
	if (bgInfo->pal256) {
		//Loop through all remaining x tiles visible on screen
		while (xTiles--) {
			
			if (tileX < 32)	//If it's in the winLeft segment
				mapEntry = tileMap[tileX];
			else 				//Otherwise it's in the winRight segment
				mapEntry = tileMap[992 + tileX];
						
			offset = ((mapEntry & 0x3FF) << 6);
			
			//Flip vertically
			if (mapEntry & TILE_FLIPY)
				tileOffsetY = flippedY;	//from 7 - 0.
			else 
				tileOffsetY = normalY;	//from 0 - 7.

			//Flip horizontally
			if (mapEntry & TILE_FLIPX) {
				offset = offset + tileOffsetY + 7;
				for ( int i = 8; i--;) {
					u8 palEntry = tileData[offset--];
					if (palEntry) {
						*dest = gbaMem.u16Pal[palEntry];
						if (blendFlags & 0xF0) {	//If this is a source blend...
							u32 oldBlendFlags = *blendArray;	//Get the current flags for this pixel
							*blendArray = blendFlags | (oldBlendFlags >> 12);	//Set the current target.
						} else {
							*blendArray = blendFlags;
						}
					}
					dest++;
					blendArray++;
				}
			} else {	//Don't flip horizontally.
				offset = offset + tileOffsetY;
				for ( int i = 8; i--;) {
					u8 palEntry = tileData[offset++];
					if (palEntry) {
						*dest = gbaMem.u16Pal[palEntry];
						if (blendFlags & 0xF0) {	//If this is a source blend...
							u32 oldBlendFlags = *blendArray;	//Get the current flags for this pixel
							*blendArray = blendFlags | (oldBlendFlags >> 12);	//Set the current target.
						} else {
							*blendArray = blendFlags;
						}
					}
					dest++;
					blendArray++;
				}	
			}
			tileX = (tileX + 1) & tileWidthMask;	//Go to the next tile.
		}
	
	////////////////////////////////////////////////////
	//Render the tiles as 16 color
	////////////////////////////////////////////////////
	} else {		//16 color
		//Loop through all remaining x tiles visible on screen
		while (xTiles--) {
			
			if (tileX < 32)	//If it's in the winLeft segment
				mapEntry = tileMap[tileX];
			else 				//Otherwise it's in the winRight segment
				mapEntry = tileMap[992 + tileX];
						
			offset = ((mapEntry & 0x3FF) << 6);
			u16* palette = &gbaMem.u16Pal[(mapEntry >> 8) & 0xF0];
			
			//Flip vertically
			if (mapEntry & TILE_FLIPY)
				tileOffsetY = flippedY;	//from 7 - 0.
			else 
				tileOffsetY = normalY;	//from 0 - 7.

			//Flip horizontally
			if (mapEntry & TILE_FLIPX) {
				offset = (offset + tileOffsetY + 7) >> 1;
				
				for (int i = 4; i--;) {
					u8 doubleEntry = tileData[offset];	//This gets two pixels.
					u8 palEntry1 = doubleEntry & 0xF;
					u8 palEntry2 = doubleEntry >> 4;
					if (palEntry2) {
						*dest = palette[palEntry2];		//Get the left pixel
						if (blendFlags & 0xF0) {	//If this is a source blend...
							u32 oldBlendFlags = *blendArray;	//Get the current flags for this pixel
							*blendArray = blendFlags | (oldBlendFlags >> 12);	//Set the current target.
						} else {
							*blendArray = blendFlags;
						}
					}
					dest++;
					blendArray++;
					if (palEntry1) {
						*dest = palette[palEntry1];		//Get the right pixel
						if (blendFlags & 0xF0) {	//If this is a source blend...
							u32 oldBlendFlags = *blendArray;	//Get the current flags for this pixel
							*blendArray = blendFlags | (oldBlendFlags >> 12);	//Set the current target.
						} else {
							*blendArray = blendFlags;
						}
					}
					dest++;
					blendArray++;
					offset--;
				}
			
			} else {	//Don't flip horizontally.
				offset = (offset + tileOffsetY) >> 1;
						
				for (int i = 4; i--;) {
					u8 doubleEntry = tileData[offset];	//This gets two pixels.
					u8 palEntry1 = doubleEntry & 0xF;
					u8 palEntry2 = doubleEntry >> 4;
					if (palEntry1) {
						*dest = palette[palEntry1];		//Get the right pixel
						if (blendFlags & 0xF0) {	//If this is a source blend...
							u32 oldBlendFlags = *blendArray;	//Get the current flags for this pixel
							*blendArray = blendFlags | (oldBlendFlags >> 12);	//Set the current target.
						} else {
							*blendArray = blendFlags;
						}
					}
					dest++;
					blendArray++;
					if (palEntry2) {
						*dest = palette[palEntry2];		//Get the left pixel
						if (blendFlags & 0xF0) {	//If this is a source blend...
							u32 oldBlendFlags = *blendArray;	//Get the current flags for this pixel
							*blendArray = blendFlags | (oldBlendFlags >> 12);	//Set the current target.
						} else {
							*blendArray = blendFlags;
						}
					}
					dest++;
					blendArray++;
					offset++;
				}
			}
			tileX = (tileX + 1) & tileWidthMask;	//Go to the next tile.
		}//while
	}//else
	return;
}



//////////////////////////////////////////
//renderRSTileBG()
//Renders background to the current scanline, taking
//into account rotate and scale
//Returns:  none
//////////////////////////////////////////
void Graphics_renderRSTileBG(const BackgroundInfo* bgInfo, u16* dest, u32 y, const Window* window,
										u32* blendArray, u32 blendFlags) {
	s32 bgWidth, bgHeight, winLeft, winRight;
	u32 heightMask, widthMask;		//A masking value.
			

	bgHeight = bgInfo->height;
	bgWidth = bgInfo->width;

	heightMask = bgHeight -1;	//If width = 256, mask = 0xFF		
	widthMask = bgWidth - 1;	//If width = 512, mask = 0x1FF

	//Seems to not even be necessary . . . damned if I can figure out the logic of it
	//Looking at how I did mode0 backgrounds might clarify though.	
	//Calculate where this scanline intersects the background.
	//y = 0;
	//x = 0;

	//Rotational Registers:
	//When BGDY is 256 (100), we want the image to be sheared
	//with a slope of 1 (don't ask me why, I'm just replicating the rsdemo)
	//In order to speed things up, we make BGDY be a u32 and shift it left
	//by 16 so that when we divide, the lower 16 bits of our result become
	//a "fraction", and we can use this to multiply with, so long as we shift
	//right again at the end.  TO save the step of dividing by 256, we only
	//shif left by 8 rather than 16 to get the equivalent value.
	s32 BGDY = *bgInfo->DY;//(*(u16*)(gbaMem.u8IORAM + 0x24));
	s32 BGDMX = *bgInfo->DMX;//(*(u16*)(gbaMem.u8IORAM + 0x22));
	
	
	//Scaling registers
	s32 BGDX = *bgInfo->DX;//(*(u16*)(gbaMem.u8IORAM + 0x20));
	s32 BGDMY = *bgInfo->DMY;//(*(u16*)(gbaMem.u8IORAM + 0x26));
	
	s32 BGY = *bgInfo->Y;
	s32 BGX = *bgInfo->X;
	
	//Start rendering at winleft.
	winLeft = window->winLeft;
	winRight = window->winRight;
	dest+= winLeft;
	blendArray+=winLeft;

	//Finally, draw the picture, by converting screen coords to background coords.
	if (*bgInfo->info & 0x2000 ) {	//If area overflow is on . . .
		u32 bgYS = (y * BGDMY) + BGY;
		u32 bgShearX = (y * BGDMX) + BGX;
		
		while (winLeft < winRight) {
			
			//s32 bgYR = bgYS + (BGDY * winLeft);	//Shear the image in y
			//bgYR = (bgYR + BGY) >> 8;
			s32 bgYR = (bgYS + (BGDY * winLeft)) >> 8;	//Shear the image in y
			u32 bgYRAbs = bgYR & heightMask;	//Roll over
			

			//s32 bgXR = bgXS + bgShearX;//(BGDMX * graphics.lcy);	//Shear the image in x
			//bgXR = (bgXR + BGX) >> 8;					
			s32 bgXS = (winLeft * BGDX);				//Scale the image in x
			s32 bgXR = (bgXS + bgShearX) >> 8;	//Shear the image in x
			
			u32 bgXRAbs = bgXR & widthMask;				//Roll over.
			
			Graphics_renderRSBgPixel(bgInfo, dest, bgXRAbs, bgYRAbs, blendArray, blendFlags);
			dest++;
			blendArray++;
			winLeft++;
		}//for
	} else {	//The following method is less efficient than if I could calculate
				//starting and ending x values...
		//First get the sign bit for all values (if we encounter a situation where
		//the code above doesn't work, may be worthwhile to transplant this code there
		//as well
		BGDY = BGDY << 16;
		BGDY = BGDY >> 16;

		BGDMY = BGDMY << 16;
		BGDMY = BGDMY >> 16;
		
		BGY = BGY << 4;
		BGY = BGY >> 4;
		
		BGDX = BGDX << 16;
		BGDX = BGDX >> 16;

		BGDMX = BGDMX << 16;
		BGDMX = BGDMX >> 16;
			
		BGX = BGX << 4;
		BGX = BGX >> 4;
		
		u32 bgYS = (y * BGDMY) + BGY;
		u32 bgShearX = (y * BGDMX) + BGX;
		
		while (winLeft < winRight) {
		
			s32 bgYR = (bgYS + (BGDY * winLeft)) >> 8;	//Shear the image in y
			
			s32 bgXS = (winLeft * BGDX);				//Scale the image in x
			s32 bgXR = (bgXS + bgShearX) >> 8;
						
			//This does bounds checking in reverse -- finds out if the pixel
			//at the specified coordinates is in bounds, prints it if so.
			//NOT as efficient as if we calcualted bounds FIRST and then iterated
			//within the bounds only -- but I'm having trouble getting that to work.
			if ( (bgYR >=0) && (bgYR < bgHeight)) {
				if ( (bgXR >=0) && (bgXR <bgWidth) ) {
					Graphics_renderRSBgPixel(bgInfo, dest, bgXR, bgYR, blendArray, blendFlags);
				}
			}
			dest++;
			blendArray++;
			winLeft++;
			
		}//for
		
	}
	return;
}

