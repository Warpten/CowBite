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
//Display_Proc
//The windows callback for the display window in debug mode
////////////////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK DisplayGui_Proc (HWND hDisplayDlg, UINT wMessage, 
                                                        WPARAM wParam, LPARAM lParam)
{
	ARMDebugger* myDebugger = appState.armDebugger;
    switch (wMessage)       /* process messages */
    {
        case WM_INITDIALOG:
			appState.hDisplayDlg = hDisplayDlg;
			ShowWindow(hDisplayDlg, SW_SHOW);
			
			SetWindowPos( hDisplayDlg, NULL, appState.settings.displayPos.x, appState.settings.displayPos.y, 
								0, 0, SWP_NOSIZE|SWP_NOZORDER );


			//Set the scroll range to the size of the bg, or to 512, depending on the mode
			//SendDlgItemMessage (hDisplayDlg, IDC_MAPHSCROLL, SBM_SETRANGE, (WPARAM)0, (LPARAM)123);
			DisplayGui_refresh();
			return (TRUE);

		case WM_SYSCOMMAND:	/*User clicks on close x button*/
			if (wParam == SC_CLOSE) {
				DestroyWindow(hDisplayDlg);
				appState.hDisplayDlg = NULL;
				return (TRUE) ;
			}
			break;			
		//case WM_MOVE:
		case WM_PAINT:
			DisplayGui_refresh();
			break;

		case WM_ERASEBKGND:
		case WM_NCPAINT:
			
			appState.windowsInterface->redrawFrame();
			//redundant, but the only way I could get it to work!!
			appState.windowsInterface->redrawBackground();
			return DefWindowProc (hDisplayDlg, wMessage, wParam, lParam);
			break;
		case WM_MOVE:
			{	RECT rect;
				GetWindowRect(hDisplayDlg, &rect);
				appState.settings.displayPos.x = rect.left;//LOWORD(lParam);
				appState.settings.displayPos.y = rect.top;//HIWORD(lParam);
			}
			break;
		case WM_COMMAND:    /* controls in dialog box activated */
            			
			switch (LOWORD(wParam))
            {
				//If the user enables/disables a layer
				case IDC_BG0CHECK:
				case IDC_BG1CHECK:
				case IDC_BG2CHECK:
				case IDC_BG3CHECK:
				case IDC_OBJCHECK:
				case IDC_BLENDCHECK:
					{
						u32 layerNum = wParam - IDC_BG0CHECK;
						//Check/uncheck the corresponding item on the menu
						if (GetMenuState(appState.hGraphicsDebugMenu, IDM_BG0+layerNum, MF_BYCOMMAND) & MF_CHECKED)  {
							CheckMenuItem(appState.hGraphicsDebugMenu, IDM_BG0+layerNum, MF_BYCOMMAND | MF_UNCHECKED);
							graphics.masterVisibility &= (0xFFFFFFDF >> (5 - layerNum));
						} else {
							CheckMenuItem(appState.hGraphicsDebugMenu, IDM_BG0+layerNum, MF_BYCOMMAND | MF_CHECKED);
							graphics.masterVisibility |= 1 << layerNum;
							
						}
					}
					break;
					DisplayGui_refresh();
					break;
			
			}//ENd of switch (LOWORD(wParam);
			break;
			
		}//End of switch(message)
	return (FALSE) ;    /* return FALSE to signify message not processed */
}//procodile dundee

/////////////////////////////////////////////////////////////
//DisplayGui_refresh()
/////////////////////////////////////////////////////////////
void DisplayGui_refresh() {
	char text[16];
	//Check the appropriate layer boxes
	for (u32 layerNum = 0; layerNum <6; layerNum++) {
		if (graphics.masterVisibility & (1 << layerNum) )	//If the layer is enabled
			CheckDlgButton(appState.hDisplayDlg, IDC_BG0CHECK+layerNum, BST_CHECKED);	//check box
		else 
			CheckDlgButton(appState.hDisplayDlg, IDC_BG0CHECK+layerNum, BST_UNCHECKED);	//Uncheck box
	}

	//Print the mode
	sprintf(text, "%d", *REG_DSPCNT & 0x7);
	SendDlgItemMessage (appState.hDisplayDlg, IDC_DISPLAYMODE, WM_SETTEXT, (WPARAM)0, (LPARAM)text);

	//Print the status reg
	sprintf(text, "%04X", *REG_STAT);
	SendDlgItemMessage (appState.hDisplayDlg, IDC_DISPSTAT, WM_SETTEXT, (WPARAM)0, (LPARAM)text);

	//Print vcount
	sprintf(text, "%d", *REG_VCOUNT);
	SendDlgItemMessage (appState.hDisplayDlg, IDC_DISPLAYVCOUNT, WM_SETTEXT, (WPARAM)0, (LPARAM)text);

}
/*
void DisplayGui_redrawFrame() {
	PAINTSTRUCT ps;
	RECT updateRect;
	HDC hDC, hMemDC;

	//If there's no update rectangle, break out.
	if (! GetUpdateRect (appState.hDisplayDlg, &updateRect, 0))
		return;

	BeginPaint (appState.hDisplayDlg, &ps) ;
	hDC = ps.hdc;
	hMemDC = CreateCompatibleDC (hDC) ;
	
	//hBackground = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_BACKGROUND));

	SelectObject (hMemDC, hRulerLeft) ;
	BitBlt (hDC, rulerX, rulerY+22, 22, rulerHeight, hMemDC, 0, 0, SRCCOPY) ;
	SelectObject (hMemDC, hRulerTop) ;
	BitBlt (hDC, rulerX, rulerY, rulerWidth, 22, hMemDC, 0, 0, SRCCOPY) ;
     
	DeleteDC (hMemDC) ;
   EndPaint (hWnd, &ps) ;


	
	DeleteDC (hMemDC) ;
   EndPaint (hWnd, &ps) ;

	//Redraw the image on screen	(will this break it?)
	render(1);
}*/