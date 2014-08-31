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
//Obj_Proc
//The windows callback for the obj drawing window.
////////////////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK ObjGui_Proc (HWND hObjDlg, UINT wMessage, 
                                                        WPARAM wParam, LPARAM lParam)
{
	ARMDebugger* myDebugger = appState.armDebugger;
     
	 switch (wMessage)       /* process messages */
    {
        case WM_INITDIALOG:
			appState.hObjDlg = hObjDlg;
			//SetWindowPos(hObjDlg, HWND_NOTOPMOST, 128, 0, 322, 400, SWP_NOZORDER | SWP_NOMOVE);
			ShowWindow(hObjDlg, SW_SHOW);
			//SendDlgItemMessage (hObjDlg, IDC_SPRITEPALNUM, WM_SETTEXT, 
			//			(WPARAM)0, (LPARAM)"0" );
			//SendDlgItemMessage (hObjDlg, IDC_SPRITEPALSPIN, UDM_SETBUDDY, 
			//			(WPARAM)GetDlgItem(hObjDlg, IDC_SPRITEPALNUM) , (LPARAM)0 );
			//SendDlgItemMessage (hObjDlg, IDC_SPRITEPALSPIN, UDM_SETRANGE, 
			//			(WPARAM)0 , (LPARAM)15);
			
			//Set the scroll range to include 128 sprites (-5 for the ones currently in view)
			SendDlgItemMessage (hObjDlg, IDC_OBJSCROLL, SBM_SETRANGE, (WPARAM)0, (LPARAM)123);
			ObjGui_refresh();
			return (TRUE);

		case WM_SYSCOMMAND:	/*User clicks on close x button*/
			if (wParam == SC_CLOSE) {
				DestroyWindow(hObjDlg);
				appState.hObjDlg = NULL;
				return (TRUE) ;
			}
			break;			
		//case WM_MOVE:
		case WM_PAINT:
			ObjGui_refresh();
			break;
		case WM_COMMAND:    /* controls in dialog box activated */
            			
			switch (LOWORD(wParam))
            {
				
				//case WM_ERASEBKGND:
				//case WM_NCPAINT:
				//case IDC_SPRITEPALNUM:
				//case IDC_SPRITEPALSPIN:
				case IDC_OBJREFRESH:	
					ObjGui_refresh();
						
						/*
						palType = !SendDlgItemMessage (hObjDlg, IDC_16COLORSPRITE, BM_GETCHECK, (WPARAM)0 , (LPARAM)0 );
						SendDlgItemMessage (hObjDlg, IDC_SPRITEPALNUM, WM_GETTEXT, (WPARAM)(16) , (LPARAM)text);		
						palNum = strtoul(text, &((char*)nullString), 16);
					
						tilePointer = graphics.spriteData;	//Get a pointer to the start of sprite data
						palettePointer = (u16*)(gbaMem.u8Pal + 0x200);
						
						//Width and height are 256; 256*256 = 64k = size of bitmap tile memory.
						hBitmap = ObjGui_createTileBitmapHandle(tilePointer, palettePointer, (256), (256 >> palType), palType, palNum);	
						
						SendDlgItemMessage (hObjDlg, IDC_SPRITEBITMAP, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP , (LPARAM)hBitmap);
						DeleteObject(hBitmap);
					//} else {

					//}*/
					return DefWindowProc (hObjDlg, wMessage, wParam, lParam);
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
				if (GetDlgCtrlID((HWND)lParam) == IDC_OBJSCROLL) {
					ObjGui_scroll(wParam, lParam);
				}
			return (TRUE);
			break;// End of case (WM_HSCROLL)
		}//End of switch(message)
	return (FALSE) ;    /* return FALSE to signify message not processed */
}//procodile dundee

/////////////////////////////////////////////////////////////
//ObjGui_refresh()
/////////////////////////////////////////////////////////////
void ObjGui_refresh() {
	HBITMAP hBitmap;
	char text[16];
	char* nullString = "";
	
	SpriteInfo obj;

	u32 objNum = SendDlgItemMessage (appState.hObjDlg, IDC_OBJSCROLL, SBM_GETPOS, (WPARAM)0, (LPARAM)0);

	

	Graphics_loadSprite(&obj, &graphics.OAM[objNum], 1);
	//Width and height are 256; 256*256 = 64k = size of bitmap tile memory.
	hBitmap = ObjGui_createObjBitmapHandle(&obj);//tilePointer, palettePointer, (256), (256 >> palType), palType, palNum);	
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJBITMAP1, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP , (LPARAM)hBitmap);
	DeleteObject(hBitmap);
	sprintf(text, "%d", objNum);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJNUM1, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", obj.x);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJX1, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", obj.y);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJY1, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", obj.width);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJWIDTH1, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", obj.height);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJHEIGHT1, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%03X", (obj.oamInfo->attribute2 & 0x3FF));
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJTILENUM1, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", (obj.pal256));
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJPALTYPE1, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", (obj.oamInfo->attribute2 >> 12));
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJPALNUM1, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", (obj.oamInfo->attribute2 >> 10) & 0x3);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJPRIORITY1, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", (obj.rotScale >> 8));
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJROTSCALE1, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", (obj.oamInfo->attribute0 >> 9) & 0x1);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJDOUBLE1, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", (obj.oamInfo->attribute0 >> 10) & 0x3);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJTT1, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", (obj.oamInfo->attribute0 >> 12) & 0x1);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJMOSAIC1, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", (obj.oamInfo->attribute1 >> 12) & 0x1);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJHFLIP1, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", (obj.oamInfo->attribute1 >> 13) & 0x1);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJVFLIP1, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", (obj.oamInfo->attribute1 >> 8) & 0x1F);
	SendDlgItemMessage (appState.hObjDlg, IDC_RSIND1, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%04X", (u16)obj.oamRSInfo->DX);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJDX1, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%04X", (u16)obj.oamRSInfo->DMX);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJDMX1, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%04X", (u16)obj.oamRSInfo->DY);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJDY1, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%04X", (u16)obj.oamRSInfo->DMY);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJDMY1, WM_SETTEXT, (WPARAM)0, (LPARAM)text);


	objNum++;
	Graphics_loadSprite(&obj, &graphics.OAM[objNum], 1);
	hBitmap = ObjGui_createObjBitmapHandle(&obj);//tilePointer, palettePointer, (256), (256 >> palType), palType, palNum);	
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJBITMAP2, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP , (LPARAM)hBitmap);
	DeleteObject(hBitmap);
	sprintf(text, "%d", objNum);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJNUM2, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", obj.x);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJX2, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", obj.y);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJY2, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", obj.width);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJWIDTH2, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", obj.height);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJHEIGHT2, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%03X", (obj.oamInfo->attribute2 & 0x3FF));
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJTILENUM2, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", (obj.pal256));
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJPALTYPE2, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", (obj.oamInfo->attribute2 >> 12));
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJPALNUM2, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", (obj.oamInfo->attribute2 >> 10) & 0x3);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJPRIORITY2, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", (obj.rotScale >> 8));
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJROTSCALE2, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", (obj.oamInfo->attribute0 >> 9) & 0x1);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJDOUBLE2, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", (obj.oamInfo->attribute0 >> 10) & 0x3);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJTT2, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", (obj.oamInfo->attribute0 >> 12) & 0x1);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJMOSAIC2, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", (obj.oamInfo->attribute1 >> 12) & 0x1);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJHFLIP2, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", (obj.oamInfo->attribute1 >> 13) & 0x1);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJVFLIP2, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", (obj.oamInfo->attribute1 >> 8) & 0x1F);
	SendDlgItemMessage (appState.hObjDlg, IDC_RSIND2, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%04X", (u16)obj.oamRSInfo->DX);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJDX2, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%04X", (u16)obj.oamRSInfo->DMX);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJDMX2, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%04X", (u16)obj.oamRSInfo->DY);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJDY2, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%04X", (u16)obj.oamRSInfo->DMY);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJDMY2, WM_SETTEXT, (WPARAM)0, (LPARAM)text);

	objNum++;
	Graphics_loadSprite(&obj, &graphics.OAM[objNum], 1);
	hBitmap = ObjGui_createObjBitmapHandle(&obj);//tilePointer, palettePointer, (256), (256 >> palType), palType, palNum);	
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJBITMAP3, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP , (LPARAM)hBitmap);
	DeleteObject(hBitmap);
	sprintf(text, "%d", objNum);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJNUM3, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", obj.x);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJX3, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", obj.y);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJY3, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", obj.width);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJWIDTH3, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", obj.height);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJHEIGHT3, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%03X", (obj.oamInfo->attribute2 & 0x3FF));
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJTILENUM3, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", (obj.pal256));
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJPALTYPE3, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", (obj.oamInfo->attribute2 >> 12));
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJPALNUM3, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", (obj.oamInfo->attribute2 >> 10) & 0x3);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJPRIORITY3, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", (obj.rotScale >> 8));
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJROTSCALE3, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", (obj.oamInfo->attribute0 >> 9) & 0x1);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJDOUBLE3, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", (obj.oamInfo->attribute0 >> 10) & 0x3);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJTT3, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", (obj.oamInfo->attribute0 >> 12) & 0x1);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJMOSAIC3, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", (obj.oamInfo->attribute1 >> 12) & 0x1);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJHFLIP3, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", (obj.oamInfo->attribute1 >> 13) & 0x1);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJVFLIP3, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", (obj.oamInfo->attribute1 >> 8) & 0x1F);
	SendDlgItemMessage (appState.hObjDlg, IDC_RSIND3, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%04X", (u16)obj.oamRSInfo->DX);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJDX3, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%04X", (u16)obj.oamRSInfo->DMX);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJDMX3, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%04X", (u16)obj.oamRSInfo->DY);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJDY3, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%04X", (u16)obj.oamRSInfo->DMY);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJDMY3, WM_SETTEXT, (WPARAM)0, (LPARAM)text);


	objNum++;
	Graphics_loadSprite(&obj, &graphics.OAM[objNum], 1);
	hBitmap = ObjGui_createObjBitmapHandle(&obj);//tilePointer, palettePointer, (256), (256 >> palType), palType, palNum);	
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJBITMAP4, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP , (LPARAM)hBitmap);
	DeleteObject(hBitmap);
	sprintf(text, "%d", objNum);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJNUM4, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", obj.x);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJX4, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", obj.y);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJY4, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", obj.width);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJWIDTH4, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", obj.height);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJHEIGHT4, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%03X", (obj.oamInfo->attribute2 & 0x3FF));
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJTILENUM4, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", (obj.pal256));
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJPALTYPE4, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", (obj.oamInfo->attribute2 >> 12));
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJPALNUM4, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", (obj.oamInfo->attribute2 >> 10) & 0x3);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJPRIORITY4, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", (obj.rotScale >> 8));
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJROTSCALE4, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", (obj.oamInfo->attribute0 >> 9) & 0x1);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJDOUBLE4, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", (obj.oamInfo->attribute0 >> 10) & 0x3);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJTT4, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", (obj.oamInfo->attribute0 >> 12) & 0x1);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJMOSAIC4, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", (obj.oamInfo->attribute1 >> 12) & 0x1);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJHFLIP4, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", (obj.oamInfo->attribute1 >> 13) & 0x1);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJVFLIP4, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", (obj.oamInfo->attribute1 >> 8) & 0x1F);
	SendDlgItemMessage (appState.hObjDlg, IDC_RSIND4, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%04X", (u16)obj.oamRSInfo->DX);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJDX4, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%04X", (u16)obj.oamRSInfo->DMX);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJDMX4, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%04X", (u16)obj.oamRSInfo->DY);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJDY4, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%04X", (u16)obj.oamRSInfo->DMY);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJDMY4, WM_SETTEXT, (WPARAM)0, (LPARAM)text);

	objNum++;
	Graphics_loadSprite(&obj, &graphics.OAM[objNum], 1);
	hBitmap = ObjGui_createObjBitmapHandle(&obj);//tilePointer, palettePointer, (256), (256 >> palType), palType, palNum);	
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJBITMAP5, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP , (LPARAM)hBitmap);
	DeleteObject(hBitmap);
	sprintf(text, "%d", objNum);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJNUM5, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", obj.x);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJX5, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", obj.y);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJY5, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", obj.width);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJWIDTH5, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", obj.height);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJHEIGHT5, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%03X", (obj.oamInfo->attribute2 & 0x3FF));
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJTILENUM5, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", (obj.pal256));
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJPALTYPE5, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", (obj.oamInfo->attribute2 >> 12));
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJPALNUM5, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", (obj.oamInfo->attribute2 >> 10) & 0x3);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJPRIORITY5, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", (obj.rotScale >> 8));
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJROTSCALE5, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", (obj.oamInfo->attribute0 >> 9) & 0x1);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJDOUBLE5, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", (obj.oamInfo->attribute0 >> 10) & 0x3);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJTT5, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", (obj.oamInfo->attribute0 >> 12) & 0x1);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJMOSAIC5, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", (obj.oamInfo->attribute1 >> 12) & 0x1);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJHFLIP5, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", (obj.oamInfo->attribute1 >> 13) & 0x1);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJVFLIP5, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%d", (obj.oamInfo->attribute1 >> 8) & 0x1F);
	SendDlgItemMessage (appState.hObjDlg, IDC_RSIND5, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%04X", (u16)obj.oamRSInfo->DX);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJDX5, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%04X", (u16)obj.oamRSInfo->DMX);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJDMX5, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%04X", (u16)obj.oamRSInfo->DY);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJDY5, WM_SETTEXT, (WPARAM)0, (LPARAM)text);
	sprintf(text, "%04X", (u16)obj.oamRSInfo->DMY);
	SendDlgItemMessage (appState.hObjDlg, IDC_OBJDMY5, WM_SETTEXT, (WPARAM)0, (LPARAM)text);

}

//////////////////////////////////////////////////////////
//ObjGui_createObjBitmapHandle
//Creates a bitmap based on the tile data.
//Does not need to be fast.
///////////////////////////////////////////////////////////
HBITMAP ObjGui_createObjBitmapHandle(SpriteInfo* obj) { //u8* data, u16* palette, u32 width, u32 height, u32 palType, u32 palNum) {
	HBITMAP hBitmap;
	u32 width;
	u32 height;
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
	u16* palette;
	u8* data;
	u8* currentSpriteLine;
	WindowsInterface* myInterface = appState.windowsInterface;
	
	//if (obj->visible) {
		width = obj->width;
		height = obj->height;

		//First create a buffer for our data
		xTiles = width >> 3;
		yTiles = height >>3;
		width = width + xTiles +1;
		height = height + yTiles +1;

		size = width * height;	//Add a little gridline between each tile
		numTiles = xTiles * yTiles;
		//colors16 = new u16[size];
		colors16 = new u16[size];

		palette = (u16*)(gbaMem.u8Pal + 0x200);
		data = currentSpriteLine = obj->bitmap;
		
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
				if (obj->pal256) {	//If it's 256 color...
					data = currentSpriteLine = obj->bitmap + (yTile * 64 * 8) + xTile * 8;
					for (int i = 8; i--;) {
						for (int j = 0; j<8; j++) {
							pixel = *data++;
							palEntry = palette[pixel];
							dest[j] = ((palEntry & 0x1F) << 11) | ((palEntry & 0x3E0) << 1 ) | ((palEntry & 0x7C00) >> 10);
						}
						dest[8] = 0x8410;	//draw a white line on the right
						dest+=width;		//each time we increment y, point dest to the next line
						currentSpriteLine += 64;	//I made the sprite buffers 64 * 64 for a reason.
						data = currentSpriteLine;
					}
					
				} else {
					data = currentSpriteLine = obj->bitmap + (yTile * 64 * 8) + xTile * 4;
					u8 pixel1, pixel2;
					u16* spritePalette16 = palette + (obj->palNum << 4);
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
						currentSpriteLine += 64;	//I made the sprite buffers 64 * 64 for a reason.
						data = currentSpriteLine;
					}

				}
				for (int m = 0; m < 9; m++)
					dest[m] = 0x8410;
		
			}
			
			currentLine += width * 9;
		}
	/*} else {
		width = height = 8;	//Otherwise just make it blank.
		size = width*height;
		colors16 = new u16[size];
		memset(colors16, 0, (size << 1));
	}*/
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

//////////////////////////////////////////////////////
//ObjGui_scroll
//Handless scrolling of the objs.
///////////////////////////////////////////////////////
void ObjGui_scroll(WPARAM wParam, LPARAM lParam) {
	u32 pos = SendDlgItemMessage (appState.hObjDlg, IDC_OBJSCROLL, SBM_GETPOS, (WPARAM)0, (LPARAM)0);
	
	
	switch (LOWORD(wParam)) /* check what part was pressed */
	{
		case SB_THUMBTRACK:  /* user movded scroll thumb */
			pos = HIWORD(wParam);
	      break;
		
		case SB_THUMBPOSITION:  /* user movded scroll thumb */
			pos = HIWORD(wParam);
			break ;

		case SB_LINEDOWN:       /* user clicked down (right) arrow */
			pos++;
			break ;

		case SB_LINEUP:         /* user clicked up (left) arrow */
			pos--;
			break ;

		case SB_PAGEDOWN:       /* user clicked down (right) area */
			pos+=5;
			break ;

		case SB_PAGEUP:         /* user clicked up (right) area */
			pos-=5;
			break ;

			default:
			break;
		}//end of switch (LOWORD(wParam)
		SendDlgItemMessage (appState.hObjDlg, IDC_OBJSCROLL, SBM_SETPOS, (WPARAM)pos, (LPARAM)TRUE);				
		ObjGui_refresh();
}//regScroll()

