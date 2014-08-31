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
#include "Graphics.h"
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////////////////
//Map_Proc
//The windows callback for the map drawing window.
////////////////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK MapGui_Proc (HWND hMapDlg, UINT wMessage, 
                                                        WPARAM wParam, LPARAM lParam)
{
	ARMDebugger* myDebugger = appState.armDebugger;
     BackgroundInfo bgInfo;
	 switch (wMessage)       /* process messages */
    {
        case WM_INITDIALOG:
			appState.hMapDlg = hMapDlg;
			ShowWindow(hMapDlg, SW_SHOW);
			CheckRadioButton( hMapDlg, IDC_BG0RADIO, IDC_BG3RADIO, IDC_BG0RADIO);
			appState.settings.debugSettings.bgNum = 0;

			CheckRadioButton( hMapDlg, IDC_BGMAPONLY, IDC_BGCOMPOSITE, IDC_BGCOMPOSITE);
			CheckRadioButton( hMapDlg, IDC_MAPWIDTH256, IDC_MAPWIDTH512, IDC_MAPWIDTH256);
			appState.settings.debugSettings.mapWidth = 256;
			CheckRadioButton( hMapDlg, IDC_MAPHEIGHT256, IDC_MAPHEIGHT512, IDC_MAPHEIGHT256);
			appState.settings.debugSettings.mapHeight = 256;
			SetWindowPos( hMapDlg, NULL, appState.settings.mapPos.x, appState.settings.mapPos.y, 
								appState.settings.debugSettings.mapWidth + 190, 
										560, SWP_NOZORDER );


			//Set the scroll range to the size of the bg, or to 512, depending on the mode
			//SendDlgItemMessage (hMapDlg, IDC_MAPHSCROLL, SBM_SETRANGE, (WPARAM)0, (LPARAM)123);
			MapGui_refresh();
			return (TRUE);

		case WM_SYSCOMMAND:	/*User clicks on close x button*/
			if (wParam == SC_CLOSE) {
				DestroyWindow(hMapDlg);
				appState.hMapDlg = NULL;
				return (TRUE) ;
			}
			break;			
		//case WM_MOVE:
		case WM_PAINT:
			MapGui_refresh();
			break;

		case WM_MOVE:
			{	RECT rect;
				GetWindowRect(hMapDlg, &rect);
				appState.settings.mapPos.x = rect.left;//LOWORD(lParam);
				appState.settings.mapPos.y = rect.top;//HIWORD(lParam);
			}
			break;
		case WM_COMMAND:    /* controls in dialog box activated */
            			
			switch (LOWORD(wParam))
            {
				case IDC_BG0RADIO:
				case IDC_BG1RADIO:
				case IDC_BG2RADIO:
				case IDC_BG3RADIO:
					appState.settings.debugSettings.bgNum = LOWORD(wParam) - IDC_BG0RADIO;
					Graphics_initBackground(&bgInfo, appState.settings.debugSettings.bgNum);
					if (bgInfo.width > 256)
						SendMessage(hMapDlg, WM_COMMAND, IDC_MAPWIDTH512,0);
					else
						SendMessage(hMapDlg, WM_COMMAND, IDC_MAPWIDTH256,0);
					if (bgInfo.height > 256)
						SendMessage(hMapDlg, WM_COMMAND, IDC_MAPHEIGHT512,0);
					else
						SendMessage(hMapDlg, WM_COMMAND, IDC_MAPHEIGHT256,0);



					MapGui_refresh();
					break;
				
				case IDC_BGMAPONLY:	
					CheckRadioButton( hMapDlg, IDC_BGMAPONLY, IDC_BGCOMPOSITE, IDC_BGMAPONLY);
					Graphics_initBackground(&bgInfo, appState.settings.debugSettings.bgNum);
					
					//If the user wants the map, set the dimensions to be that of the BG
					if (bgInfo.width > 256)
						SendMessage(hMapDlg, WM_COMMAND, IDC_MAPWIDTH512,0);
					else
						SendMessage(hMapDlg, WM_COMMAND, IDC_MAPWIDTH256,0);
					if (bgInfo.height > 256)
						SendMessage(hMapDlg, WM_COMMAND, IDC_MAPHEIGHT512,0);
					else
						SendMessage(hMapDlg, WM_COMMAND, IDC_MAPHEIGHT256,0);
					
					EnableWindow( GetDlgItem (hMapDlg, IDC_BGOBJS), FALSE);
					EnableWindow( GetDlgItem (hMapDlg, IDC_MAPWIDTH256), FALSE);
					EnableWindow( GetDlgItem (hMapDlg, IDC_MAPWIDTH512), FALSE);
					EnableWindow( GetDlgItem (hMapDlg, IDC_MAPHEIGHT256), FALSE);
					EnableWindow( GetDlgItem (hMapDlg, IDC_MAPHEIGHT512), FALSE);
					MapGui_refresh();
					break;
				case IDC_BGSINGLE:	
				case IDC_BGCOMPOSITE:
					CheckRadioButton( hMapDlg, IDC_BGMAPONLY, IDC_BGCOMPOSITE,LOWORD(wParam));
					EnableWindow( GetDlgItem (hMapDlg, IDC_BGOBJS), TRUE);
					EnableWindow( GetDlgItem (hMapDlg, IDC_MAPWIDTH256), TRUE);
					EnableWindow( GetDlgItem (hMapDlg, IDC_MAPWIDTH512), TRUE);
					EnableWindow( GetDlgItem (hMapDlg, IDC_MAPHEIGHT256), TRUE);
					EnableWindow( GetDlgItem (hMapDlg, IDC_MAPHEIGHT512), TRUE);
					MapGui_refresh();
					break;
				case IDC_MAPWIDTH256:
					CheckRadioButton( hMapDlg, IDC_MAPWIDTH256, IDC_MAPWIDTH512, IDC_MAPWIDTH256);
					appState.settings.debugSettings.mapWidth = 256;
					SetWindowPos( hMapDlg, NULL, 0, 0, appState.settings.debugSettings.mapWidth + 190, 
								560, SWP_NOMOVE | SWP_NOZORDER );
					MapGui_refresh();
					break;
				case IDC_MAPWIDTH512:
					CheckRadioButton( hMapDlg, IDC_MAPWIDTH256, IDC_MAPWIDTH512, IDC_MAPWIDTH512);
					appState.settings.debugSettings.mapWidth = 512;
					SetWindowPos( hMapDlg, NULL, 0, 0, appState.settings.debugSettings.mapWidth + 190, 
								560, SWP_NOMOVE | SWP_NOZORDER );
					MapGui_refresh();
					break;
				case IDC_MAPHEIGHT256:
					CheckRadioButton( hMapDlg, IDC_MAPHEIGHT256, IDC_MAPHEIGHT512, IDC_MAPHEIGHT256);
					appState.settings.debugSettings.mapHeight = 256;
					MapGui_refresh();
					break;
				
				case IDC_MAPHEIGHT512:
					CheckRadioButton( hMapDlg, IDC_MAPHEIGHT256, IDC_MAPHEIGHT512, IDC_MAPHEIGHT512);
					appState.settings.debugSettings.mapHeight = 512;
					MapGui_refresh();
					break;

				//case WM_ERASEBKGND:
				//case WM_NCPAINT:
				//case IDC_SPRITEPALNUM:
				//case IDC_SPRITEPALSPIN:
				case IDC_MAPREFRESH:	
					MapGui_refresh();
					return DefWindowProc (hMapDlg, wMessage, wParam, lParam);
					break;
			
				default:
					break;
			}//ENd of switch (LOWORD(wParam);
			break;
			///////////////////////////////////////////////////////
			//The user clicked on the scroll bar for the memory
			//////////////////////////////////////////////////////
			case WM_HSCROLL:    /* scroll bar was activated */
				
				//Find out if the message came from the reg scroll bar or the
				//bits scroll bar.
				if (GetDlgCtrlID((HWND)lParam) == IDC_MAPHSCROLL) {
					//MapGui_scroll(wParam, lParam);
				}
			return (TRUE);
			break;// End of case (WM_HSCROLL)
		}//End of switch(message)
	return (FALSE) ;    /* return FALSE to signify message not processed */
}//procodile dundee

/////////////////////////////////////////////////////////////
//MapGui_refresh()
/////////////////////////////////////////////////////////////
void MapGui_refresh() {
	HBITMAP hBitmap;
	char text[64];
	char* nullString = "";
	u32 size, width, height;
	u16* colors16;
	Window window;	
	Blend blend;		
	BackgroundInfo bgInfo;
	
	u32 mapNum = appState.settings.debugSettings.bgNum;
	Graphics_initBackgrounds();
	//Fill our bgInfo structure with settings
	Graphics_initBackground(&bgInfo, mapNum);
	
	graphics.backgroundsDirty = 1;	//Set them to dirty so that the "normal" rendering is forced to repeat this
	
	u32 widthMask = bgInfo.width - 1;
	u32 heightMask = bgInfo.height -1;
	
	width = appState.settings.debugSettings.mapWidth;
	height = appState.settings.debugSettings.mapHeight;
	window.winLeft = 0;
	window.winRight = width;
	window.winTop = 0;
	window.winBottom = height;
	window.winWidth = width;
	window.winFlags = 0x0000003F & graphics.masterVisibility;
	
	size = width*height;
	colors16 = new u16[size];
			

	//If we have it set on map only, don't do any extra stuff (blends, etc.)
	if (SendDlgItemMessage (appState.hMapDlg, IDC_BGMAPONLY, BM_GETCHECK, (WPARAM)0 , (LPARAM)0 ) == BST_CHECKED) {
		u32 screenX;
		u32 screenY;
		static u32 zero = 0;
		window.winFlags = 1 << mapNum;
		
		appState.settings.debugSettings.mapWidth = min(bgInfo.width,512);
		appState.settings.debugSettings.mapHeight = min(bgInfo.height,512);
		
		//Resize the window
		SetWindowPos( appState.hMapDlg, NULL, 0, 0, appState.settings.debugSettings.mapWidth + 190, 
						560, SWP_NOMOVE | SWP_NOZORDER );

		
		//If it's a text background
		if (bgInfo.type == BGTYPE_TEXT) {
			screenX = (*bgInfo.scx) & widthMask;	
			screenY = (*bgInfo.scy) & heightMask;
			bgInfo.scx = (u16*)&zero;	//Translate to zero
			bgInfo.scy = (u16*)&zero;
			for (int i = 0; i < height; i++) {
				u16* temp = &colors16[i*width];
				//render the backdrop :)
				for (int j = 0; j < width; j++) {
					temp[j] = gbaMem.u16Pal[0];
				}
				Graphics_renderTileBG(&bgInfo, temp, i, &window);
			}
			//Draw an outline of where the screen is
			MapGui_drawScreenOutline (colors16, screenX, screenY, width, height);
		} else {
			bgInfo.X = bgInfo.Y = &zero;
			bgInfo.DX = bgInfo.DY = bgInfo.DMX = bgInfo.DMY = (u16*)&zero;
			for (int i = 0; i < height; i++) {
				u16* temp = &colors16[i*width];
				for (int j = 0; j < bgInfo.width; j++) {
					temp[j] = gbaMem.u16Pal[0];//the backdrop
					Graphics_renderRSBgPixel(&bgInfo, &temp[j], j, i, NULL, 0xFFFFFFFF);
					//Graphics_renderRSTileBG(&bgInfo, temp, i, &window, NULL, 0xFFFFFFFF);
				}
			}

		}
		
		//Refill our bgInfo structure with settings, since we changed some of them.
		Graphics_initBackground(&bgInfo, mapNum);


	}  else {	//Otherwise, set up structures.

		//If single background is selected, just render that one.
		if (SendDlgItemMessage (appState.hMapDlg, IDC_BGSINGLE, BM_GETCHECK, (WPARAM)0 , (LPARAM)0 ) == BST_CHECKED) {
			window.winFlags = 0x10 | (1 << mapNum);
		
		} 
		
		//If the sprites checkbox is unchecked
		if (SendDlgItemMessage (appState.hMapDlg, IDC_BGOBJS, BM_GETCHECK, (WPARAM)0 , (LPARAM)0 ) == BST_UNCHECKED) {
			window.winFlags &= 0xFFFFFFEF;	//disable sprites
		} else {
			Graphics_initSprites(128, 0);	
		}

		
		
		window.blend = &blend;
		
		//Set up our blend info in a structure.
		blend.bldCnt = *REG_BLDCNT;
		blend.bldType = (blend.bldCnt >> 6) & 0x3;
		blend.enabled = 1;	//Assume enabled to begin with
		

		//If there is no blends, don't bother with the rest of the blend structure.
		if (!blend.bldType) {
			blend.enabled = 0;
		} else {
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
			
		}
		blend.blendArray = new u32[width];
		
		
		

		//Width and height are 256; 256*256 = 64k = size of bitmap tile memory.
		//hBitmap = MapGui_createMapBitmapHandle(&map);//tilePointer, palettePointer, (256), (256 >> palType), palType, palNum);	
		
		

		u32 numXDivisions = 0;	//If we want to dilineate x=0 witha  line, there will be at most 2
		u32 xDivisions[4];
		if (!bgInfo.rotScale) {	//Though what I really want is if (!bitmapped)
			for (int k = 0; k < width; k++) {
				if ( (bgInfo.width -1 - (*bgInfo.scx & heightMask)) == (k & widthMask)) {
						xDivisions[numXDivisions] = k;
						numXDivisions++;
				}
			}
		}
		u16* temp;
		//Render the graphic using our normal graphics routines
		for (int i = 0; i < height; i++) {
			temp = &colors16[i*width];
			Graphics_renderBlendWindow(temp,i,&window);
			//Draw lines representing x=0, y=0, and screen boundaries.
			if (!bgInfo.rotScale) {
				// This only works for non-rotational backgrounds at the moment
				if ( (bgInfo.height -1 - (*bgInfo.scy & heightMask)) == (i & heightMask)) {
					for (int j = 0; j < width; j++) {
						//temp[j] = 0x4210;
						temp[j] = 0x7FFF + (j & 0x1);	//Draw a dotted line
					}
				}
				for (int l = 0; l < numXDivisions; l++) {
					temp[xDivisions[l]] = 0x7FFF + (i & 0x1); //a dotted line
				}
				
			}
			if (i <= 160)
				temp[240] = 0x001F;	//Draw screen boundary
		}
		temp = &colors16[160*width];	//Draw screen boundary
		for (int j = 0; j < 240; j++) {
				//temp[j] = 0x4210;
				temp[j] = 0x001F;
		}


		delete [] blend.blendArray;
	}

	

	//Convert to the correct bit depth, create a bitmap
	if (myInterface->bitDepth == 32) {
		u8* colors32 = new u8[size << 2];
		//myInterface->convert16to32(colors32, colors16, size);
		myInterface->convertGBAto32Win((u8*)colors32, colors16, size);
		hBitmap = CreateBitmap (width, height, myInterface->bitPlanes, myInterface->bitDepth , colors32);
		delete [] colors32;
	} else {
		u16* temp16 = new u16[size];
		myInterface->convertGBAtoWin((u32*)temp16, (u32*)colors16, size);
		hBitmap = CreateBitmap (width, height, myInterface->bitPlanes, myInterface->bitDepth , temp16);
		delete [] temp16;
	}
	delete [] colors16;
	
	SendDlgItemMessage (appState.hMapDlg, IDC_MAP, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP , (LPARAM)hBitmap);
	DeleteObject(hBitmap);


	//See if it's enabled
	if (*REG_DSPCNT & (1 << (mapNum + 8)) )
		SendDlgItemMessage (appState.hMapDlg, IDC_BGENABLED, WM_SETTEXT, (WPARAM)0, (LPARAM)"Yes");
	else
		SendDlgItemMessage (appState.hMapDlg, IDC_BGENABLED, WM_SETTEXT, (WPARAM)0, (LPARAM)"No");
	
	//If it's a text background...
	if (bgInfo.type == BGTYPE_TEXT) {
		SendDlgItemMessage (appState.hMapDlg, IDC_BGTYPE, WM_SETTEXT, (WPARAM)0, (LPARAM)"Text");
	
		if (bgInfo.pal256)
			SendDlgItemMessage (appState.hMapDlg, IDC_BGCOLORDEPTH, WM_SETTEXT, (WPARAM)0, (LPARAM)"8 bit");	
		else
			SendDlgItemMessage (appState.hMapDlg, IDC_BGCOLORDEPTH, WM_SETTEXT, (WPARAM)0, (LPARAM)"4 bit");	
	
	//If it's a rotate/scale backround
	} else if (bgInfo.type == BGTYPE_ROTSCALE) {
		SendDlgItemMessage (appState.hMapDlg, IDC_BGTYPE, WM_SETTEXT, (WPARAM)0, (LPARAM)"R/S");
		SendDlgItemMessage (appState.hMapDlg, IDC_BGCOLORDEPTH, WM_SETTEXT, (WPARAM)0, (LPARAM)"8 bit");	
	//If it's a bitmapped backround
	} else {	
		SendDlgItemMessage (appState.hMapDlg, IDC_BGTYPE, WM_SETTEXT, (WPARAM)0, (LPARAM)"Bitmap");
		if (bgInfo.pal256)
			SendDlgItemMessage (appState.hMapDlg, IDC_BGCOLORDEPTH, WM_SETTEXT, (WPARAM)0, (LPARAM)"8 bit");	
		else
			SendDlgItemMessage (appState.hMapDlg, IDC_BGCOLORDEPTH, WM_SETTEXT, (WPARAM)0, (LPARAM)"16 bit");	

	}
	
	sprintf(text, "%d", *bgInfo.info &0x3);
	SendDlgItemMessage (appState.hMapDlg, IDC_BGPRIORITY, WM_SETTEXT, (WPARAM)0, (LPARAM)text);

	sprintf(text, "%08X", (bgInfo.tileData - gbaMem.u8VRAM) + 0x6000000);
	SendDlgItemMessage (appState.hMapDlg, IDC_BGTILEBLOCK, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%08X", ((u8*)bgInfo.tileMap - gbaMem.u8VRAM) + 0x6000000);
	SendDlgItemMessage (appState.hMapDlg, IDC_BGMAPBLOCK, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", bgInfo.width);
	if (*bgInfo.info & 0x40)
		SendDlgItemMessage (appState.hMapDlg, IDC_BGMOSAIC, WM_SETTEXT, (WPARAM)0, (LPARAM)"On");	
	else
		SendDlgItemMessage (appState.hMapDlg, IDC_BGMOSAIC, WM_SETTEXT, (WPARAM)0, (LPARAM)"Off");	
	
	
	if (*bgInfo.info & 0x2000)
		SendDlgItemMessage (appState.hMapDlg, IDC_BGOVERFLOW, WM_SETTEXT, (WPARAM)0, (LPARAM)"Off");	
	else
		SendDlgItemMessage (appState.hMapDlg, IDC_BGOVERFLOW, WM_SETTEXT, (WPARAM)0, (LPARAM)"On");	

	SendDlgItemMessage (appState.hMapDlg, IDC_BGWIDTH, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", bgInfo.height);
	SendDlgItemMessage (appState.hMapDlg, IDC_BGHEIGHT, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", *bgInfo.scx);
	SendDlgItemMessage (appState.hMapDlg, IDC_BGHOFFSET, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", *bgInfo.scy);
	SendDlgItemMessage (appState.hMapDlg, IDC_BGVOFFSET, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	
	sprintf(text, "%04X", *bgInfo.DX);
	SendDlgItemMessage (appState.hMapDlg, IDC_BGPA, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%04X", *bgInfo.DMX);
	SendDlgItemMessage (appState.hMapDlg, IDC_BGPB, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%04X", *bgInfo.DY);
	SendDlgItemMessage (appState.hMapDlg, IDC_BGPC, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%04X", *bgInfo.DMY);
	SendDlgItemMessage (appState.hMapDlg, IDC_BGPD, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%08X", *bgInfo.X);
	SendDlgItemMessage (appState.hMapDlg, IDC_BGX, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%08X", *bgInfo.Y);
	SendDlgItemMessage (appState.hMapDlg, IDC_BGY, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
}

//////////////////////////////
//Use this function to draw the outline of a window on the map
//////////////////////////////
inline void MapGui_drawScreenOutline (u16* dest, u32 screenX, u32 screenY, u32 winWidth, u32 winHeight) {
	u32 screenWidth = 240;
	u32 screenHeight = 160;
	u32 widthMask = winWidth - 1;
	u32 heightMask = winHeight - 1;
	screenX = screenX & widthMask;
	screenY = screenY & widthMask;
	u32 screenRight = (screenX + screenWidth) & widthMask;
	u32 screenBottom = (screenY + screenHeight) & heightMask;
	u32 i;
	
	
	if (screenRight > screenX) {	//If the top and bottom both fit

		for (i = screenX; i < screenRight; i++) {
			dest[screenY*winWidth + i] = 0x001F;//Draw the top border
			dest[screenBottom*winWidth + i] = 0x001F;//Draw the bottom border
		}
			
	} else {	//We're off the right edge
		//Draw the right hand portion
		for (i = screenX; i < winWidth; i++) {
			dest[screenY*winWidth + i] = 0x001F;	//Top
			dest[screenBottom*winWidth + i] = 0x001F;	//Bottom
		}
		//Draw the left hand portion
		for (i = 0; i < screenRight; i++) {
			dest[screenY*winWidth + i] = 0x001F;
			dest[screenBottom*winWidth + i] = 0x001F;	//Bottom
		}
	}
	

	if (screenBottom > screenY) {
		for (i = screenY; i < screenBottom; i++) {
			dest[i * winWidth + screenX] = 0x001F;         //Draw the left border
			dest[i * winWidth + screenRight] = 0x001F;  //Draw the right border
		}
	} else {	//We're off the bottom border
		for (i = screenY; i < winHeight; i++) {
			dest[i * winWidth + screenX] = 0x001F;         //Draw the left border
			dest[i * winWidth + screenRight] = 0x001F;  //Draw the right border
		}
		
		for (i = 0; i < screenBottom; i++) {
			dest[i * winWidth + screenX] = 0x001F;         //Draw the left border
			dest[i * winWidth + screenRight] = 0x001F;  //Draw the right border
		}
		

	}
	

}




