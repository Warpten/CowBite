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

WNDPROC originalProc;

char keyStringTable[256];

/////////////////////////////////////////////////////////////////////////////////////////
//InputGui_Proc
//The windows callback for the input settings dialog.
//This nosty function resorts to using a timer to poll the keyboard and joystick,
//setting the appropriate fields to whatever virtual keys or buttons are pressed.
////////////////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK InputGui_Proc (HWND hInputDlg, UINT wMessage, 
                                                        WPARAM wParam, LPARAM lParam)
{
	u32 buttonID;
	HWND focusWnd;
	JOYCAPS joyCaps;
	char temp[255];
	u8 keyState[255];
	static InputSettings tempSettings;
	int selectedInput;
	int i;
	
	static int timerPause;	//milliseconds for timer to pause after
								//user inputs.
		
	 switch (wMessage)       /* process messages */
    {
        case WM_INITDIALOG:
			appState.hInputDlg = hInputDlg;

			//Get the old settings.
			tempSettings = appState.settings.inputSettings;

			//Set a timer to go off once every milliseconds so we can check the controller
			while (!SetTimer (appState.hInputDlg, 1, 1, NULL))

					if (IDCANCEL == MessageBox (hInputDlg,

					  "Too many clocks or timers!", "CowBite",

					   MB_ICONEXCLAMATION | MB_RETRYCANCEL)) {
						
						EndDialog (appState.hInputDlg, 0);
						appState.hInputDlg = NULL;
					}

		
			
			//Get the center joystick position.
			joyGetDevCaps (JOYSTICKID1, &joyCaps, sizeof (joyCaps)); 
			tempSettings.centerX = (joyCaps.wXmax + joyCaps.wXmin)/2;
			tempSettings.centerY = (joyCaps.wYmax + joyCaps.wYmin)/2;
			tempSettings.xDivisor = (joyCaps.wXmax - joyCaps.wXmin)/4;
			tempSettings.yDivisor = (joyCaps.wYmax - joyCaps.wYmin)/4;

			//capture joystick messages
			//joySetCapture(hInputDlg, JOYSTICKID1, 0, FALSE);
			//joySetThreshold (JOYSTICKID1, 100);
						
			
			SendDlgItemMessage (hInputDlg, IDC_BUTTONA, WM_SETFOCUS, (WPARAM)0 , (LPARAM)0);
			SendMessage (hInputDlg, WM_SETFOCUS, 0, 0);

			//Initialize the combo box
			SendDlgItemMessage (hInputDlg, IDC_DPADCOMBO, CB_ADDSTRING, (WPARAM)0, (LPARAM)"Arrow Keys");
			SendDlgItemMessage (hInputDlg, IDC_DPADCOMBO, CB_ADDSTRING, (WPARAM)0, (LPARAM)"Joystick 1");
			SendDlgItemMessage (hInputDlg, IDC_DPADCOMBO, CB_SETCURSEL, (WPARAM)(tempSettings.dPad -1) , (LPARAM)FALSE);

			//for (i = 0; i < 256; i++) {
			//	SendDlgItemMessage (hInputDlg, IDC_BUTTONACOMBO, CB_ADDSTRING, (WPARAM)0, (LPARAM)InputGui_getButtonString(i));

			//}

			//Initialzie the button names.			
			//SendDlgItemMessage (hInputDlg, IDC_BUTTONA, EM_SETREADONLY, (WPARAM)TRUE, (LPARAM)0);
			SendDlgItemMessage (hInputDlg, IDC_BUTTONA, WM_SETTEXT, 0, 
				(LPARAM)InputGui_getButtonString(tempSettings.buttonA, tempSettings.buttonAJoystick));
			SendDlgItemMessage (hInputDlg, IDC_BUTTONB, WM_SETTEXT, 0, 
				(LPARAM)InputGui_getButtonString(tempSettings.buttonB, tempSettings.buttonBJoystick));
			SendDlgItemMessage (hInputDlg, IDC_BUTTONL, WM_SETTEXT, 0, 
				(LPARAM)InputGui_getButtonString(tempSettings.buttonL, tempSettings.buttonLJoystick));
			SendDlgItemMessage (hInputDlg, IDC_BUTTONR, WM_SETTEXT, 0, 
				(LPARAM)InputGui_getButtonString(tempSettings.buttonR, tempSettings.buttonRJoystick));
			SendDlgItemMessage (hInputDlg, IDC_SELECT, WM_SETTEXT, 0, 
				(LPARAM)InputGui_getButtonString(tempSettings.select, tempSettings.selectJoystick));
			SendDlgItemMessage (hInputDlg, IDC_START, WM_SETTEXT, 0, 
				(LPARAM)InputGui_getButtonString(tempSettings.start, tempSettings.startJoystick));

			
			//originalProc = (WNDPROC)SetClassLong (GetDlgItem (hInputDlg, IDC_BUTTONA), GCL_WNDPROC, (u32)InputGui_editProc);
			//if (!originalProc)
			//	SendDlgItemMessage (hInputDlg, IDC_START, WM_SETTEXT, 0, (LPARAM)"FAILED.");
			timerPause = 0;

			return (TRUE);

		case WM_SYSCOMMAND:	/*User clicks on close x button*/
			if (wParam == SC_CLOSE) {
				//joyReleaseCapture(JOYSTICKID1);
				KillTimer (appState.hInputDlg, 1) ;
				EndDialog (appState.hInputDlg, 0);
				appState.hInputDlg = NULL;
				return (TRUE) ;
			}
			break;			

        case WM_COMMAND:    /* controls in dialog box activated */
            			
			switch (LOWORD(wParam))
			{
				case IDINPUTOK:
						selectedInput = SendDlgItemMessage (hInputDlg, IDC_DPADCOMBO, CB_GETCURSEL, (WPARAM)0 , (LPARAM)0);
						tempSettings.dPad = selectedInput+1;
						appState.settings.inputSettings = tempSettings;
										
				case IDINPUTCANCEL:
						KillTimer (appState.hInputDlg, 1) ;
						EndDialog (appState.hInputDlg, 0);
						appState.hInputDlg = NULL;
						return (TRUE) ;
						break;
				
				default:
					return (FALSE);	//Otherwise we get a stack overflow.
					break;
            }//ENd of switch (LOWORD(wParam);
			return (FALSE);
			break;

		//NOTE:  This method is buggy, and almost not worth it.  But it's
		//the only one that works...
		case WM_TIMER:
			if (timerPause > 0) {
				timerPause--;
				break;
			}
			buttonID = InputGui_returnSelectedButton();
			//Get the name of the button
			if (buttonID) {
				focusWnd = GetFocus();
				InputGui_assignButtonID (GetDlgCtrlID(focusWnd), buttonID, &tempSettings, 1);
				SendMessage (focusWnd, WM_SETTEXT, 0, (LPARAM)InputGui_getButtonString(buttonID, 1));					
				//Move to the next item.
				SendMessage (hInputDlg, WM_NEXTDLGCTL, 0, 0);
				timerPause = 20;
			} else {
				GetKeyboardState(keyState);
				focusWnd = GetFocus();
				for (i = 0xC; i < 256; i++) {
					if (keyState[i] & 0x80) {
						InputGui_assignButtonID (GetDlgCtrlID(focusWnd), i, &tempSettings, 0);
						SendMessage (focusWnd, WM_SETTEXT, 0, (LPARAM)InputGui_getButtonString(i, 0));
						SendMessage (hInputDlg, WM_NEXTDLGCTL, 0, 0);
						timerPause = 20;
						break;
					}
				}
				
			}
			return (TRUE);	
	
		//If the user hits a key, change the field and save it.
	
		//case WM_CHAR:
	//	return (TRUE);
		//	break;
		
	/*	case WM_NOTIFY:
			nmhdr = *((LPNMHDR)wParam);
			focusWnd = GetFocus();
			if (nmhdr.code == WM_CHAR) {
				buttonID = wParam;
				focusWnd = GetFocus();
				InputGui_assignButtonID (GetDlgCtrlID(focusWnd), buttonID, &tempSettings);
				//SendMessage (focusWnd, WM_SETTEXT, 0, (LPARAM)InputGui_getButtonString(buttonID));
				SendMessage (focusWnd, WM_SETTEXT, 0, (LPARAM)"hi");
				SendMessage (hInputDlg, WM_NEXTDLGCTL, 0, 0);
				return (TRUE);
			}
			SendMessage (focusWnd, WM_SETTEXT, 0, (LPARAM)"hi");
			return (FALSE);
			break;
	*/		

		
	}//End of switch(message)
	return (FALSE) ;    /* return FALSE to signify message not processed */
}


///////////////////////////////////////////////////
//InputGui_returnSelectedButton()
//returns the buttonID if pressed, 0 if not.
///////////////////////////////////////////////////
int InputGui_returnSelectedButton() {
	HWND focusWnd;
	JOYCAPS joyCaps;
	JOYINFOEX joyInfo;
	char temp[255];

	//zero out joyInfo
	memset (&joyInfo, 0, sizeof (JOYINFOEX));
	joyInfo.dwSize = sizeof (JOYINFOEX);
	joyInfo.dwFlags = JOY_RETURNBUTTONS | JOY_RETURNX | JOY_RETURNY;
	
	//If the joystick is not plugged in or not available, it will return an error
	if ( joyGetDevCaps (JOYSTICKID1, &joyCaps, sizeof (joyCaps)) != JOYERR_NOERROR)
		return 0;
	
	joyGetPosEx (JOYSTICKID1, &joyInfo);
			
	focusWnd = GetFocus();	//****

	//If the return buttons field is valid...
	if (joyInfo.dwFlags & JOY_RETURNBUTTONS) {
							
		//Check each button to see if it was pressed.  If pressed, set
		//buttonID to the id of the button pressed.  This is kind of clugey,
		//as it only allows the users to use buttons 1 - 6
		if (joyInfo.dwButtons & JOY_BUTTON1) {
			//SendMessage (focusWnd, WM_SETTEXT, 0, (LPARAM)"Button 1");
			return JOY_BUTTON1;
		} else if (joyInfo.dwButtons & JOY_BUTTON2) {
			//SendMessage (focusWnd, WM_SETTEXT, 0, (LPARAM)"Button 2");
			return  JOY_BUTTON2;
		} else if (joyInfo.dwButtons & JOY_BUTTON3) {
			//SendMessage (focusWnd, WM_SETTEXT, 0, (LPARAM)"Button 3");
			return JOY_BUTTON3;
		} else if (joyInfo.dwButtons & JOY_BUTTON4) {
			//SendMessage (focusWnd, WM_SETTEXT, 0, (LPARAM)"Button 4");
			return JOY_BUTTON4;
		} else if (joyInfo.dwButtons & JOY_BUTTON5) {
			//SendMessage (focusWnd, WM_SETTEXT, 0, (LPARAM)"Button 5");
			return JOY_BUTTON5;
		} else if (joyInfo.dwButtons & JOY_BUTTON6) {
			//SendMessage (focusWnd, WM_SETTEXT, 0, (LPARAM)"Button 6");
			return JOY_BUTTON6;
		} 
		
		
	
	}
	return 0;
}



///////////////////////////////////////////////////
//InputGui_assignButtonID
//Assign a particular button or keystroke to the joystick.
///////////////////////////////////////////////////
void InputGui_assignButtonID (u32 resourceID, u32 buttonID, InputSettings* inputSettings, u32 isJoystickButton) {
	switch (resourceID) {
		
		case IDC_BUTTONA:
			inputSettings->buttonA = buttonID;
			inputSettings->buttonAJoystick = isJoystickButton;
			break;
		case IDC_BUTTONB:
			inputSettings->buttonB = buttonID;
			inputSettings->buttonBJoystick = isJoystickButton;
			break;
		case IDC_BUTTONL:
			inputSettings->buttonL = buttonID;
			inputSettings->buttonLJoystick = isJoystickButton;
			break;
		case IDC_BUTTONR:
			inputSettings->buttonR = buttonID;
			inputSettings->buttonRJoystick = isJoystickButton;
			break;
		case IDC_SELECT:
			inputSettings->select = buttonID;
			inputSettings->selectJoystick = isJoystickButton;
			break;
		case IDC_START:
			inputSettings->start = buttonID;
			inputSettings->startJoystick = isJoystickButton;
			break;
	}
}

/////////////////////////////////////////////////////
//InputGui_getButtonString
//Returns the string representation of the button.
////////////////////////////////////////////////////
char* InputGui_getButtonString(u32 code, u32 isJoystickButton) {
	static char text[255];
	

	//If the code maps to a button number...
	if (isJoystickButton) {
		if (code == JOY_BUTTON1) {
			sprintf (text, "Button 1");
		} else if (code == JOY_BUTTON2) {
			sprintf (text, "Button 2");
		} else if (code == JOY_BUTTON3) {
			sprintf (text, "Button 3");
		} else if (code == JOY_BUTTON4) {
			sprintf (text, "Button 4");
		} else if (code == JOY_BUTTON5) {
			sprintf (text, "Button 5");
		} else if (code == JOY_BUTTON6) {
			sprintf (text, "Button 6");
		}
	}		
	//otherwise, try to map it to a keyboard key.
	else {
		if ( ((code >= 'A') && (code <= 'Z')) || ((code >= '1') && (code <= '9'))) {
			text[0] = code;
			text[1] = '\0';
		} else {
			switch (code) {
				case VK_TAB:	sprintf(text, "Tab");
					break;
				case VK_RETURN:	sprintf(text, "Return");
					break;
				case VK_SHIFT:	sprintf(text, "Shift");
					break;
				case VK_CONTROL:	sprintf(text, "Control");
					break;
				case VK_SPACE:	sprintf(text, "Space");
					break;
				case VK_LEFT:	sprintf(text, "Left");
					break;
				case VK_RIGHT:	sprintf(text, "Right");
					break;
				case VK_UP:	sprintf(text, "Up");
					break;
				case VK_DOWN:	sprintf(text, "Down");
					break;
				case VK_MENU:	sprintf(text, "Alt");
					break;
				default:	sprintf(text, "Key Code 0x%X", code);
					break;
			}
			
		} 
	}
	return text;
}

/////////////////////////////////////////////////////////////////////////////////////////
//EditProc
//A replacement for the defautl edit control procedure.
////////////////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK InputGui_editProc (HWND hEditWnd, UINT wMessage, 
                                                        WPARAM wParam, LPARAM lParam) {

	switch (wMessage)  {
		case WM_KEYDOWN:  SendMessage (hEditWnd, WM_SETTEXT, 0, (LPARAM)"Test");
			return (TRUE);
		default:
			return (TRUE);
		//default:  return originalProc(hEditWnd, wMessage, wParam, lParam);


	}


}