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
#include <mmsystem.h>
#include <stdio.h>
#include "Gui.h"
#include "resource.h"

#include "Constants.h"
#include "WindowsInterface.h"

extern State appState;



/////////////////////////////////////////////////////////////////////////////////////////
//ConsoleGui_Proc
//The windows callback for the console.
////////////////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK ConsoleGui_Proc (HWND hConsoleDlg, UINT wMessage, 
                                                        WPARAM wParam, LPARAM lParam)
{
	
		
	 switch (wMessage)       /* process messages */
    {
        case WM_INITDIALOG:
			appState.hConsoleDlg = hConsoleDlg;
			SendDlgItemMessage (appState.hConsoleDlg, IDC_STDDISABLE, BM_SETCHECK, (WPARAM)appState.settings.debugSettings.consoleStdoutDisable, (LPARAM)0 );
			SendDlgItemMessage (appState.hConsoleDlg, IDC_CONSOLEDISABLE, BM_SETCHECK, (WPARAM)appState.settings.debugSettings.consoleWinDisable, (LPARAM)0 );
			//Set the window position.
			
			
			ShowWindow(hConsoleDlg, SW_HIDE);
			return (TRUE);
		case WM_SHOWWINDOW:
			SetWindowPos( hConsoleDlg, NULL, appState.settings.consolePos.x,
				appState.settings.consolePos.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER );

			break;
		case WM_MOVE:
			{	RECT rect;
				GetWindowRect(hConsoleDlg, &rect);
				appState.settings.consolePos.x = rect.left;//LOWORD(lParam);
				appState.settings.consolePos.y = rect.top;//HIWORD(lParam);
			}
			break;
		case WM_SYSCOMMAND:	/*User clicks on close x button*/
			if (wParam == SC_CLOSE) {
				ShowWindow(hConsoleDlg, SW_HIDE);
				return (TRUE) ;
				
			}
			break;			

        case WM_COMMAND:    /* controls in dialog box activated */
            			
			switch (LOWORD(wParam))
			{
				case IDC_STDDISABLE:
						appState.settings.debugSettings.consoleStdoutDisable = SendDlgItemMessage (appState.hConsoleDlg, IDC_STDDISABLE, BM_GETCHECK, (WPARAM)0, (LPARAM)0 );
						break;
				case IDC_CONSOLEDISABLE:
						appState.settings.debugSettings.consoleWinDisable = SendDlgItemMessage (appState.hConsoleDlg, IDC_CONSOLEDISABLE, BM_GETCHECK, (WPARAM)0, (LPARAM)0 );
						break;
						
				case IDCONSOLECLEAR:
						SendDlgItemMessage (hConsoleDlg, IDC_CONSOLEEDIT, WM_SETTEXT, (WPARAM)0, (LPARAM)"");
						break;	
				case IDCONSOLECLOSE:
						ShowWindow(hConsoleDlg, SW_HIDE);
						break;

				default:
					return (FALSE);	//Otherwise we get a stack overflow.
					break;
            }//ENd of switch (LOWORD(wParam);
			return (TRUE);
			break;

	
		case WM_DESTROY:
				//return (TRUE);
				break;
		
	}//End of switch(message)
	return (FALSE) ;    /* return FALSE to signify message not processed */
}

