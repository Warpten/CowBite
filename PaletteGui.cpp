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
//Proc
//The windows callback for the palette-drawing window.
////////////////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK PaletteGui_Proc (HWND hPaletteDlg, UINT wMessage, 
                                                        WPARAM wParam, LPARAM lParam)
{
	ARMDebugger* myDebugger = appState.armDebugger;
   u32 fontHeight;
	HDC hDC;
	static HFONT indexFont;
	 switch (wMessage)       /* process messages */
    {
        case WM_INITDIALOG:
			appState.hPaletteDlg = hPaletteDlg;

			//Create a tiny font for the inices.
			hDC = GetDC(GetDlgItem(hPaletteDlg, IDC_BGPALINDICES));
			
			SetMapMode (hDC, MM_TEXT);
			fontHeight = -MulDiv(7, GetDeviceCaps(hDC, LOGPIXELSY), 72);
			indexFont = CreateFont (fontHeight, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 
									OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
									DEFAULT_PITCH, "Lucida Console"); 

			//Set the font for the list box
			SendDlgItemMessage (hPaletteDlg, IDC_BGPALINDICES, WM_SETFONT, (WPARAM)(indexFont) , (LPARAM)TRUE);
			//Set the font for the list box
			SendDlgItemMessage (hPaletteDlg, IDC_OBJPALINDICES, WM_SETFONT, (WPARAM)(indexFont) , (LPARAM)TRUE);

			//SetWindowPos(hPaletteDlg, HWND_NOTOPMOST, 128, 0, 640, 450, SWP_NOZORDER | SWP_NOMOVE);
			ShowWindow(hPaletteDlg, SW_SHOW);
			
			PaletteGui_refresh();	
			return (TRUE);

		case WM_SYSCOMMAND:	/*User clicks on close x button*/
			if (wParam == SC_CLOSE) {
				DeleteObject (indexFont);
				DestroyWindow(hPaletteDlg);
				appState.hPaletteDlg = NULL;
				return (TRUE) ;
			}
			break;			

		case WM_PAINT:
				PaletteGui_refresh();
				break;
		case WM_COMMAND:    /* controls in dialog box activated */
            			
			switch (LOWORD(wParam))
            {
				
				case IDC_PALETTEREFRESH:				
					PaletteGui_refresh();
					
					return DefWindowProc (hPaletteDlg, wMessage, wParam, lParam);
					break;
			default:
				break;
            }//ENd of switch (LOWORD(wParam);
	}//End of switch(message)
	return (FALSE) ;    /* return FALSE to signify message not processed */
}//backgroundTilesProc()

/////////////////////////////////////////////////////////////
//PaletteGui_refreshPalette()
/////////////////////////////////////////////////////////////
void PaletteGui_refresh() {
	HBITMAP hBitmap;
	u16* palettePointer;
	
	palettePointer = (u16*)(gbaMem.u8Pal);	//First to bg tiles

	hBitmap = PaletteGui_createBitmapHandle(palettePointer);	
	SendDlgItemMessage (appState.hPaletteDlg, IDC_BGPAL, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP , (LPARAM)hBitmap);
	DeleteObject(hBitmap);

	palettePointer = (u16*)(gbaMem.u8Pal+0x200);	//Next to sprite pal
	hBitmap = PaletteGui_createBitmapHandle(palettePointer);	
	SendDlgItemMessage (appState.hPaletteDlg, IDC_OBJPAL, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP , (LPARAM)hBitmap);
	DeleteObject(hBitmap);
}

//////////////////////////////////////////////////////////
//PaletteGui_createTileBitmapHandle
//Creates a bitmap based on the palette data.
//Does not need to be fast.
///////////////////////////////////////////////////////////
HBITMAP PaletteGui_createBitmapHandle(u16* palette) {
	HBITMAP hBitmap;
	u32 xTiles, yTiles;
	u32 width, height;
	u32 size;
	u32 palEntry;
	u16* colors16;
	u8* colors32;
	u16* dest;
	u16* currentLine;
	WindowsInterface* myInterface = appState.windowsInterface;
	
	//width = 128;
	//height = 128;

	//First create a buffer for our data
	xTiles = 16;
	yTiles = 16;
	width = 145;	//= 128 + 17 dividing lines
	height = 145;
	//width = width + xTiles +1;
	//height = height + yTiles +1;

	size = width * height;	//Add a little gridline between each tile
	colors16 = new u16[size];



	
	//For each tile . . .
	//source = data;// + (index <<6);
	currentLine = colors16;

	for (u32 yTile = 0; yTile < yTiles; yTile++) {
		for (u32 xTile = 0; xTile < xTiles; xTile++) {

			//yTile = index / xTiles;		//calculate y Tile offset
			//xTile = index % xTiles;
				
			dest = currentLine + (xTile * 9);	//move one tile width over in dest
			//dest = colors16 + index * 64;

			//Loop through the y axis of the destination
			for (int i = 8; i--;) {
				for (int j = 0; j<8; j++) {
					palEntry = palette[(yTile << 4) + xTile];
					dest[j] = ((palEntry & 0x1F) << 11) | ((palEntry & 0x3E0) << 1 ) | ((palEntry & 0x7C00) >> 10);
				}
				dest[8] = 0x8410;	//draw a white line on the right
				dest+=width;		//each time we increment y, point dest to the next line
			}
			//Draw a white line on the bottom
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


