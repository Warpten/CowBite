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

#include <windows.h>
#include <commctrl.h>
#include "Gui.h"
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////////////////
//SpriteTilesProc
//The windows callback for the sprite-tile drawing window.
////////////////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK TileGui_SpriteTilesProc (HWND hSpriteDlg, UINT wMessage, 
                                                        WPARAM wParam, LPARAM lParam)
{
	ARMDebugger* myDebugger = appState.armDebugger;
     
	 switch (wMessage)       /* process messages */
    {
        case WM_INITDIALOG:
			appState.hSpriteDlg = hSpriteDlg;
			SetWindowPos(hSpriteDlg, HWND_NOTOPMOST, 128, 0, 322, 400, SWP_NOZORDER | SWP_NOMOVE);
			ShowWindow(hSpriteDlg, SW_SHOW);
			SendDlgItemMessage (hSpriteDlg, IDC_SPRITEPALNUM, WM_SETTEXT, 
						(WPARAM)0, (LPARAM)"0" );
			SendDlgItemMessage (hSpriteDlg, IDC_SPRITEPALSPIN, UDM_SETBUDDY, 
						(WPARAM)GetDlgItem(hSpriteDlg, IDC_SPRITEPALNUM) , (LPARAM)0 );
			SendDlgItemMessage (hSpriteDlg, IDC_SPRITEPALSPIN, UDM_SETRANGE, 
						(WPARAM)0 , (LPARAM)15);
			SendMessage (hSpriteDlg, WM_COMMAND, (WPARAM)IDC_SPRITETILESUPDATE , (LPARAM)0);			
			return (TRUE);

		case WM_SYSCOMMAND:	/*User clicks on close x button*/
			if (wParam == SC_CLOSE) {
				DestroyWindow(hSpriteDlg);
				appState.hSpriteDlg = NULL;
				return (TRUE) ;
			}
			break;			
		//case WM_MOVE:
		case WM_PAINT:
			TileGui_refreshSpriteTiles();
			break;
		case WM_VSCROLL:
				TileGui_refreshSpriteTiles();
				break;	
		case WM_COMMAND:    /* controls in dialog box activated */
            			
			switch (LOWORD(wParam))
            {
				
				//case WM_ERASEBKGND:
				//case WM_NCPAINT:
				//case IDC_SPRITEPALNUM:
				//case IDC_SPRITEPALSPIN:
				case IDC_16COLORSPRITE:
				case IDC_SPRITETILESUPDATE:	
					TileGui_refreshSpriteTiles();
						
						/*
						palType = !SendDlgItemMessage (hSpriteDlg, IDC_16COLORSPRITE, BM_GETCHECK, (WPARAM)0 , (LPARAM)0 );
						SendDlgItemMessage (hSpriteDlg, IDC_SPRITEPALNUM, WM_GETTEXT, (WPARAM)(16) , (LPARAM)text);		
						palNum = strtoul(text, &((char*)nullString), 16);
					
						tilePointer = graphics.spriteData;	//Get a pointer to the start of sprite data
						palettePointer = (u16*)(gbaMem.u8Pal + 0x200);
						
						//Width and height are 256; 256*256 = 64k = size of bitmap tile memory.
						hBitmap = TileGui_createTileBitmapHandle(tilePointer, palettePointer, (256), (256 >> palType), palType, palNum);	
						
						SendDlgItemMessage (hSpriteDlg, IDC_SPRITEBITMAP, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP , (LPARAM)hBitmap);
						DeleteObject(hBitmap);
					//} else {

					//}*/
					return DefWindowProc (hSpriteDlg, wMessage, wParam, lParam);
					break;
				

					
			default:
				break;
            }//ENd of switch (LOWORD(wParam);
	}//End of switch(message)
	return (FALSE) ;    /* return FALSE to signify message not processed */
}//spriteTilesProc

/////////////////////////////////////////////////////////////////////////////////////////
//BackgroundTilesProc
//The windows callback for the sprite-tile drawing window.
////////////////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK TileGui_BackgroundTilesProc (HWND hBackgroundDlg, UINT wMessage, 
                                                        WPARAM wParam, LPARAM lParam)
{
	ARMDebugger* myDebugger = appState.armDebugger;
     
	 switch (wMessage)       /* process messages */
    {
        case WM_INITDIALOG:
			appState.hBackgroundDlg = hBackgroundDlg;
			SetWindowPos(hBackgroundDlg, HWND_NOTOPMOST, 128, 0, 640, 450, SWP_NOZORDER | SWP_NOMOVE);
			ShowWindow(hBackgroundDlg, SW_SHOW);
			SendDlgItemMessage (hBackgroundDlg, IDC_BGPALNUM, WM_SETTEXT, 
						(WPARAM)0, (LPARAM)"0" );
			SendDlgItemMessage (hBackgroundDlg, IDC_BGPALSPIN, UDM_SETBUDDY, 
						(WPARAM)GetDlgItem(hBackgroundDlg, IDC_BGPALNUM) , (LPARAM)0 );
			SendDlgItemMessage (hBackgroundDlg, IDC_BGPALSPIN, UDM_SETRANGE, 
						(WPARAM)0 , (LPARAM)15);
			
			SendMessage (hBackgroundDlg, WM_COMMAND, (WPARAM)IDC_BGTILESUPDATE , (LPARAM)0);			
			return (TRUE);

		case WM_SYSCOMMAND:	/*User clicks on close x button*/
			if (wParam == SC_CLOSE) {
				DestroyWindow(hBackgroundDlg);
				appState.hBackgroundDlg = NULL;
				return (TRUE) ;
			}
			break;			

		case WM_PAINT:
				TileGui_refreshBackgroundTiles();
				break;
		case WM_VSCROLL:
				//nmhdr = ((LPNMHDR)lParam);
				//if (nmhdr->code == UDN_DELTAPOS)
				//if (LOWORD(lParam) == IDC_SPRITEPALSPIN)
				TileGui_refreshBackgroundTiles();
				break;		
        case WM_COMMAND:    /* controls in dialog box activated */
            			
			switch (LOWORD(wParam))
            {
				
				case IDC_16COLORBG:
				case IDC_BGTILESUPDATE:				
					TileGui_refreshBackgroundTiles();
					
					return DefWindowProc (hBackgroundDlg, wMessage, wParam, lParam);
					break;
			default:
				break;
            }//ENd of switch (LOWORD(wParam);
	}//End of switch(message)
	return (FALSE) ;    /* return FALSE to signify message not processed */
}//backgroundTilesProc()

/////////////////////////////////////////////////////////////
//TileGui_refreshSpriteTiles()
/////////////////////////////////////////////////////////////
void TileGui_refreshSpriteTiles() {
	HBITMAP hBitmap;
	u8* tilePointer;
	u16* palettePointer;
	u32 palType;
	u32 palNum;
	char text[16];
	char* nullString = "";
	
	
	palType = !SendDlgItemMessage (appState.hSpriteDlg, IDC_16COLORSPRITE, BM_GETCHECK, (WPARAM)0 , (LPARAM)0 );
	SendDlgItemMessage (appState.hSpriteDlg, IDC_SPRITEPALNUM, WM_GETTEXT, (WPARAM)(16) , (LPARAM)text);		
	palNum = strtoul(text, &((char*)nullString), 16);
					
	tilePointer = graphics.spriteData;	//Get a pointer to the start of sprite data
	palettePointer = (u16*)(gbaMem.u8Pal + 0x200);
						
	//Width and height are 256; 256*256 = 64k = size of bitmap tile memory.
	hBitmap = TileGui_createTileBitmapHandle(tilePointer, palettePointer, (256), (256 >> palType), palType, palNum);	
					
	SendDlgItemMessage (appState.hSpriteDlg, IDC_SPRITEBITMAP, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP , (LPARAM)hBitmap);
	DeleteObject(hBitmap);

}

/////////////////////////////////////////////////////////////
//TileGui_refreshBackgroundTiles()
/////////////////////////////////////////////////////////////
void TileGui_refreshBackgroundTiles() {
	HBITMAP hBitmap;
	u8* tilePointer;
	u16* palettePointer;
	u32 palType;
	u32 palNum;
	char text[16];
	char* nullString = "";
	int blah;	
	palType = !SendDlgItemMessage (appState.hBackgroundDlg, IDC_16COLORBG, BM_GETCHECK, (WPARAM)0 , (LPARAM)0 );
	blah = SendDlgItemMessage (appState.hBackgroundDlg, IDC_BGPALNUM, WM_GETTEXT, (WPARAM)(16) , (LPARAM)text);		
	palNum = strtoul(text, &((char*)nullString), 10);
	
	//Sometimes you screw up and accidentally enter text into the field
	if (palNum > 16 ) {
		//appState.windowsInterface->setPaused(1);
		return;
	}
		
	tilePointer = gbaMem.u8VRAM;	//Get a pointer to the start of background data
	palettePointer = (u16*)(gbaMem.u8Pal);

	//Width is 256, height is 64 or 128 (16 color mode), in 4 inrements; 256*256 = 64k = size of bitmap tile memory.
	hBitmap = TileGui_createTileBitmapHandle(tilePointer, palettePointer, 256, (128>>palType), palType, palNum);	
	SendDlgItemMessage (appState.hBackgroundDlg, IDC_BACKGROUNDBITMAP1, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP , (LPARAM)hBitmap);
	DeleteObject(hBitmap);

	tilePointer += 0x4000;
	hBitmap = TileGui_createTileBitmapHandle(tilePointer, palettePointer, 256, (128>>palType), palType, palNum);	
	SendDlgItemMessage (appState.hBackgroundDlg, IDC_BACKGROUNDBITMAP2, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP , (LPARAM)hBitmap);
	DeleteObject(hBitmap);

	tilePointer += 0x4000;
	hBitmap = TileGui_createTileBitmapHandle(tilePointer, palettePointer, 256, (128>>palType), palType, palNum);	
	SendDlgItemMessage (appState.hBackgroundDlg, IDC_BACKGROUNDBITMAP3, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP , (LPARAM)hBitmap);
	DeleteObject(hBitmap);

	tilePointer += 0x4000;
	hBitmap = TileGui_createTileBitmapHandle(tilePointer, palettePointer, 256, (128>>palType), palType, palNum);	
	SendDlgItemMessage (appState.hBackgroundDlg, IDC_BACKGROUNDBITMAP4, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP , (LPARAM)hBitmap);
	DeleteObject(hBitmap);

}

//////////////////////////////////////////////////////////
//TileGui_createTileBitmapHandle
//Creates a bitmap based on the tile data.
//Does not need to be fast.
///////////////////////////////////////////////////////////
HBITMAP TileGui_createTileBitmapHandle(u8* data, u16* palette, u32 width, u32 height, u32 palType, u32 palNum) {
	HBITMAP hBitmap;
	u32 numTiles;
	u32 xTiles;
	u32 yTiles;
	u32 size;
	u32 pixel;
	u32 palEntry;
	u16* colors16;
	u8* colors32;
	u16* dest;
	u16* currentLine;
	WindowsInterface* myInterface = appState.windowsInterface;
	

	//First create a buffer for our data
	xTiles = width >> 3;
	yTiles = height >>3;
	width = width + xTiles +1;
	height = height + yTiles +1;

	size = width * height;	//Add a little gridline between each tile
	numTiles = xTiles * yTiles;
	colors16 = new u16[size];



	
	//For each tile . . .
	//source = data;// + (index <<6);
	currentLine = colors16;

	//Can't get away with reversing it or the image comes out reversed.
	//for (u32 index = 0; index < numTiles; index++) {
	for (u32 yTile = 0; yTile < yTiles; yTile++) {
		for (u32 xTile = 0; xTile < xTiles; xTile++) {

			//yTile = index / xTiles;		//calculate y Tile offset
			//xTile = index % xTiles;
				
			dest = currentLine + (xTile * 9);	//move one tile width over in dest
			//dest = colors16 + index * 64;

			//Loop through the y axis of the destination
			if (palType) {	//If it's 256 color...
				for (int i = 8; i--;) {
					for (int j = 0; j<8; j++) {
						pixel = *data++;
						palEntry = palette[pixel];
						dest[j] = ((palEntry & 0x1F) << 11) | ((palEntry & 0x3E0) << 1 ) | ((palEntry & 0x7C00) >> 10);
					}
					dest[8] = 0x8410;	//draw a white line on the right
					dest+=width;		//each time we increment y, point dest to the next line
				}
			} else {
				u8 pixel1, pixel2;
				u16* spritePalette16 = palette + (palNum << 4);
				//If it's 16 bit color
				for (int i = 8; i--;) {
					for (int j = 0; j<8; j+=2) {
						pixel = *data++;
						pixel1 = pixel & 0x0F;
						pixel2 = pixel >> 4;
						palEntry = spritePalette16[pixel1];
						dest[j] = ((palEntry & 0x1F) << 11) | ((palEntry & 0x3E0) << 1 ) | ((palEntry & 0x7C00) >> 10);
						palEntry = spritePalette16[pixel2];
						dest[j+1] = ((palEntry & 0x1F) << 11) | ((palEntry & 0x3E0) << 1 ) | ((palEntry & 0x7C00) >> 10);

					}
					dest[8] = 0x8410;	//draw a white line on the right
					dest+=width;		//each time we increment y, point dest to the next line
				}

			}
			for (int m = 0; m < 9; m++)
				dest[m] = 0x8410;
	
		}
		currentLine += width * 9;
	}
		
	//Convert to the correct bit depth, create a bitmap
	if (myInterface->bitDepth == 32) {
		colors32 = new u8[size << 2];
		myInterface->convert16to32(colors32, colors16, size);
		hBitmap = CreateBitmap (width, height, myInterface->bitPlanes, myInterface->bitDepth , colors32);
		delete [] colors32;
	} else {
		hBitmap = CreateBitmap (width, height, myInterface->bitPlanes, myInterface->bitDepth , colors16);
	}
	delete [] colors16;
	return hBitmap;
}


