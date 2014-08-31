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
#include <richedit.h>
#include <stdio.h>
#include "Gui.h"
#include "GBADebugger.h"
#include "resource.h"

#include "Constants.h"
#include "WindowsInterface.h"

extern State appState;



/////////////////////////////////////////////////////////////////////////////////////////
//StatsGui_Proc
//The windows callback for the stats dialog
//Presents the user with statistics about the GBA.
////////////////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK StatsGui_Proc (HWND hStatsDlg, UINT wMessage, 
                                                        WPARAM wParam, LPARAM lParam)
{
	
	 switch (wMessage)       /* process messages */
    {
        case WM_INITDIALOG:
			appState.hStatsDlg = hStatsDlg;
			//appState.settings.dumpSetting = IDC_RALL;
			//CheckRadioButton( hStatsDlg, IDC_RALL, IDC_RSOUND, IDC_RALL);
			CheckDlgButton(hStatsDlg, IDC_GENERALSTATS, appState.settings.debugSettings.generalStatSettings);
			CheckDlgButton(hStatsDlg, IDC_MEMORYSTATS, appState.settings.debugSettings.memoryStatSettings);
			CheckDlgButton(hStatsDlg, IDC_INTERRUPTSTATS, appState.settings.debugSettings.interruptStatSettings);
			CheckDlgButton(hStatsDlg, IDC_SOUNDSTATS, appState.settings.debugSettings.soundStatSettings);
  
			SetWindowPos( hStatsDlg, NULL, appState.settings.statsPos.x,
				appState.settings.statsPos.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER );

			ShowWindow(hStatsDlg, SW_SHOW);
			StatsGui_refresh();
			return (TRUE);
		
		case WM_MOVE:
			{	RECT rect;
				GetWindowRect(hStatsDlg, &rect);
				appState.settings.statsPos.x = rect.left;//LOWORD(lParam);
				appState.settings.statsPos.y = rect.top;//HIWORD(lParam);
			}
			break;


		case WM_SYSCOMMAND:	/*User clicks on close x button*/
			if (wParam == SC_CLOSE) {
				DestroyWindow(hStatsDlg);
				
				return (TRUE) ;
				
			}
			break;			

        case WM_COMMAND:    /* controls in dialog box activated */
            			
			switch (LOWORD(wParam))
			{
				case IDSTATREFRESH:
						Gui_refreshAllDialogs (1);
						break;	
				case IDCLOSE:
						DestroyWindow(hStatsDlg);
						break;

				case IDC_GENERALSTATS:
						appState.settings.debugSettings.generalStatSettings = !appState.settings.debugSettings.generalStatSettings;
						Gui_refreshAllDialogs (1);
						break;
				case IDC_MEMORYSTATS:
						appState.settings.debugSettings.memoryStatSettings = !appState.settings.debugSettings.memoryStatSettings;
						Gui_refreshAllDialogs (1);
						break;
				case IDC_INTERRUPTSTATS:
						appState.settings.debugSettings.interruptStatSettings = !appState.settings.debugSettings.interruptStatSettings;
						Gui_refreshAllDialogs (1);
						break;
				case IDC_SOUNDSTATS:
						appState.settings.debugSettings.soundStatSettings = !appState.settings.debugSettings.soundStatSettings;
						Gui_refreshAllDialogs (1);
						break;
				default:
					return (FALSE);	//Otherwise we get a stack overflow.
					break;
            }//ENd of switch (LOWORD(wParam);
			return (TRUE);
			break;

	
		case WM_DESTROY:
				appState.hStatsDlg = NULL;
				//return (TRUE);
				break;
		
	}//End of switch(message)
	return (FALSE) ;    /* return FALSE to signify message not processed */
}

//////////////////////////////////////////////////////
//This refreshes the the stat gui screen.
void StatsGui_refresh() {
	char text[8192];
	CHARRANGE range;
	PARAFORMAT format;
	memset(&format, 0, sizeof(PARAFORMAT));
	format.cbSize = sizeof (PARAFORMAT);
	format.dwMask = PFM_TABSTOPS;
	range.cpMin = 0;
	range.cpMax = -1;
	
	//u32 tabstops[] = {15, 30, 45, 60};
	//SendDlgItemMessage (appState.hStatsDlg, IDC_STATBOX, EM_SETTABSTOPS, (WPARAM)4 , (LPARAM)&tabstops);	//Selct it all
	//int blah = SendDlgItemMessage (appState.hStatsDlg, IDC_STATBOX, EM_EXSETSEL, (WPARAM)0 , (LPARAM)&range);	//Selct it all
	//int blah2 = SendDlgItemMessage (appState.hStatsDlg, IDC_STATBOX, EM_GETPARAFORMAT, (WPARAM)0 , (LPARAM)&format);
	
	
	//Do something different depending on what mode we're in.
	//switch (appState.settings.statsSetting) {
	//	case IDC_RALL:			break;
	//	case IDC_RGENERAL:		
	strcpy(text, "");
	if (IsDlgButtonChecked(appState.hStatsDlg, IDC_GENERALSTATS))
		strcat(text, GBADebugger_getMiscStats());
	if (IsDlgButtonChecked(appState.hStatsDlg, IDC_MEMORYSTATS))
		strcat(text, GBADebugger_getMemoryStats());
	if (IsDlgButtonChecked(appState.hStatsDlg, IDC_INTERRUPTSTATS))
		strcat(text, GBADebugger_getInterruptStats());
	if (IsDlgButtonChecked(appState.hStatsDlg, IDC_SOUNDSTATS))
		strcat(text, GBADebugger_getSoundStats());
	
	SendDlgItemMessage (appState.hStatsDlg, IDC_STATBOX, WM_SETTEXT, (WPARAM)0 , (LPARAM)text);	//Selct it all

	memset(&format, 0, sizeof(PARAFORMAT));
	format.cbSize = sizeof (PARAFORMAT);
	format.dwMask = PFM_TABSTOPS;
	format.rgxTabs[0] = 1000;
	format.rgxTabs[1] = 2000;
	format.rgxTabs[2] = 3000;
	format.rgxTabs[3] = 4000;
	format.rgxTabs[4] = 5000;
	format.rgxTabs[5] = 6000;
	format.wAlignment = PFA_RIGHT;
	format.cTabCount = 6;
	SendDlgItemMessage (appState.hStatsDlg, IDC_STATBOX, EM_EXSETSEL, (WPARAM)0 , (LPARAM)&range);	//Selct it all
	int blah3 = SendDlgItemMessage (appState.hStatsDlg, IDC_STATBOX, EM_SETPARAFORMAT, (WPARAM)0 , (LPARAM)&format);
	SendDlgItemMessage (appState.hStatsDlg, IDC_STATBOX, EM_EXSETSEL, (WPARAM)0 , (LPARAM)&range);	//Selct it all
}