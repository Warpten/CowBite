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
#include <stdio.h>
#include "Gui.h"
#include "SourceGui.h"
#include "resource.h"

#include "Constants.h"
#include "WindowsInterface.h"
#include "GBA.h"



#define DEBUG_NUMPROPSHEETS 1


extern State appState;

//extern ARMDebugger* armDebugger;
//extern WindowsInterface* myInterface;

/////////////////////////////////////////////////////////////////////////////////////////
//Gui_DebugProc
//This is the primary callback for the CowBite debugger.  It's huge, but it
//does get the job done.
////////////////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK DebugGui_DebugProc (HWND hDlg, UINT wMessage, 
                                                        WPARAM wParam, LPARAM lParam)
{

	WINDOWPLACEMENT windowPlacement;

	RECT windowPosition;


	static BOOL retValue = FALSE;
	char text[255];
	char* nullString = "";
	u32 address;
	static HFONT listFont;
	HDC hDC;
	int fontHeight;
	int selectedState;
	int currentIndex;
	int selectedIndex;
	int i, temp, x, y;
	ARMDebugger* myDebugger = appState.armDebugger;
     
	temp = 0;
    switch (wMessage)       /* process messages */
    {

		///////////////////////////////////////

		//WM_INITDIALOG

		//Initializes the the debug gui

		///////////////////////////////////////
        case WM_INITDIALOG:
			hDC = GetDC(hDlg);
			appState.hDlg = hDlg;

			SetWindowPos( hDlg, NULL, appState.settings.debugPos.x,
				appState.settings.debugPos.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER );


			ShowWindow(hDlg, SW_SHOW);
			//appState.windowsInterface->setPaused(1);
			//From the old days, when this was the main debugger, we wanted to pause when
			//this got opened.

			

/*

			//Create a menu for the right mouse lick
			appState.hDebugPopupMenu = CreatePopupMenu();
			appState.hDebugCPUMenu = CreatePopupMenu();
			appState.hDebugMemoryMenu = CreatePopupMenu();

			appState.hDebugDlgMenu = GetMenu(hDlg);
			appState.hDebugSourceMenu = CreatePopupMenu();
			AppendMenu (appState.hDebugDlgMenu, MF_POPUP, (UINT)appState.hDebugSourceMenu, "&Source");

			AppendMenu (appState.hDebugSourceMenu, MF_STRING, IDM_NULL, "(Only available for ELF files compiled with -gstabs)") ;

//			AppendMenu (appState.hDebugPopupMenu, MF_POPUP, (UINT)appState.hDebugCPUMenu, "Browse Instruction Address") ;
			
			DrawMenuBar(appState.hDlg);*/

			//Set the font to be fixed width
			SendDlgItemMessage (hDlg, IDC_DEBUGLIST, WM_SETFONT, (WPARAM)0, (LPARAM)TRUE);

            //This creates a fixed width font for use with the debug dialog			
			SetMapMode (hDC, MM_TEXT);
			fontHeight = -MulDiv(8, GetDeviceCaps(hDC, LOGPIXELSY), 72);
			listFont = CreateFont (fontHeight, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 
									OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
									DEFAULT_PITCH, "Lucida Console"); 


			
			//Set the font for the list box
			SendDlgItemMessage (hDlg, IDC_DEBUGLIST, WM_SETFONT, (WPARAM)(listFont) , (LPARAM)TRUE);		
			SendDlgItemMessage (hDlg, IDC_DEBUGLIST, LB_SETHORIZONTALEXTENT, (WPARAM)(640) , (LPARAM)0);		
			
			//Set the text for the text inputs
			SendDlgItemMessage (hDlg, IDC_DEBUGADDRESS, WM_SETTEXT, (WPARAM)0 , (LPARAM)appState.settings.debugSettings.lastBreakpointText);
			SendDlgItemMessage (hDlg, IDC_DEBUGMEMADDRESS, WM_SETTEXT, (WPARAM)0 , (LPARAM)"8000000");


			
			//fill the combo box with state chocies
			for (i = 1; i < 11; i++) {
				sprintf (text, "%d", i);
				SendDlgItemMessage (hDlg, IDC_STATENUM, CB_ADDSTRING, (WPARAM)0, (LPARAM)text);
			}

			//Start it off with the first item
			SendDlgItemMessage (hDlg, IDC_STATENUM, CB_SETCURSEL, (WPARAM)(0) , (LPARAM)FALSE);

			SendDlgItemMessage (hDlg, IDC_BROWSECPUADDRESS, CB_ADDSTRING, (WPARAM)0, (LPARAM)"00000000");
			SendDlgItemMessage (hDlg, IDC_BROWSECPUADDRESS, CB_ADDSTRING, (WPARAM)0, (LPARAM)"02000000");
			SendDlgItemMessage (hDlg, IDC_BROWSECPUADDRESS, CB_ADDSTRING, (WPARAM)0, (LPARAM)"03000000");
			SendDlgItemMessage (hDlg, IDC_BROWSECPUADDRESS, CB_ADDSTRING, (WPARAM)0, (LPARAM)"08000000");
			SendDlgItemMessage (hDlg, IDC_BROWSECPUADDRESS, CB_ADDSTRING, (WPARAM)0, (LPARAM)"0A000000");
			SendDlgItemMessage (hDlg, IDC_BROWSECPUADDRESS, CB_ADDSTRING, (WPARAM)0, (LPARAM)"0C000000");
			SendDlgItemMessage (hDlg, IDC_BROWSECPUADDRESS, CB_ADDSTRING, (WPARAM)0, (LPARAM)"0E000000");


			//Send a bogus message to our scroll bar because I don't know of another way to get the scrool
			//bar handle.
			SendDlgItemMessage (hDlg, IDC_MEMSCROLL, SBM_SETPOS, (WPARAM)0, (LPARAM)TRUE);

			//Check the "auto" button
			CheckRadioButton( hDlg, IDC_RMEMADDRESS, IDC_RCARTRAM, IDC_RAUTO);
 

			//SendDlgItemMessage(hDlg, IDC_OUTPUT,EM_SETREADONLY, 1, 0);
			Gui_refreshAllDialogs(1);
			
			ReleaseDC(hDlg, hDC);
			return (TRUE);

		case WM_MOVE:
			{	RECT rect;
				GetWindowRect(hDlg, &rect);
				appState.settings.debugPos.x = rect.left;//LOWORD(lParam);
				appState.settings.debugPos.y = rect.top;//HIWORD(lParam);
			}
			break;
	case WM_ERASEBKGND:
	case WM_NCPAINT:
		myInterface->redrawFrame();
		
		//This redraws the borders, etc. of the window
		return DefWindowProc (hDlg, wMessage, wParam, lParam);
		break;

		case WM_SYSCOMMAND:	/*User clicks on close x button*/
			if (wParam == SC_CLOSE) {
				DeleteObject (listFont);
				DestroyWindow(hDlg);

				
				return (TRUE) ;
			}
			break;			


		case WM_DESTROY:	//Window is destroyed.  we are no longer debugging.
			appState.hDlg = NULL;
			break;


        case WM_COMMAND:    /* controls in dialog box activated */
			switch (LOWORD(wParam))
            {

				/////////////////////////////////////////////////
				//This toggles whether we are going or stopping

				////////////////////////////////////////////////
				case IDC_DEBUGGO:				//Let it run it's course
					if (!gba.loaded)
						Gui_openFile(hDlg);

					

					//if (arm.paused) {
						appState.armDebugger->saveUndo();	//Save how we were before we did this.
						appState.windowsInterface->setPaused(0);
					//}
					//else
					//	appState.windowsInterface->setPaused(1);
					

					appState.armDebugger->browsingProgram = 0;

					Gui_refreshAllDialogs(1);
					break;
				//This just breaks program execution.
				case IDC_DEBUGBREAK:
					appState.windowsInterface->setPaused(1);
					appState.armDebugger->browsingProgram = 0;
					Gui_refreshAllDialogs(1);
					break;

				///////////////////////////////////////////////

				//The following two cases add a breakpoint entered by the user

				//into the dropdown list

				///////////////////////////////////////////////

				case IDC_DEBUGADDBREAKPOINT:
					//Get the text form of the address
					SendDlgItemMessage (hDlg, IDC_DEBUGADDRESS, WM_GETTEXT, (WPARAM)(255) , (LPARAM)text);		
					strcpy(appState.settings.debugSettings.lastBreakpointText, text);
					address = strtoul(text, &((char*)nullString), 16);
					

					if (ARM_containsBreakpoint(address))
						appState.armDebugger->removeBreakpoint(address);
					else
						appState.armDebugger->addBreakpoint(address);

					appState.windowsInterface->render(1);
					Gui_refreshAllDialogs(1);

					break;

				case IDC_DEBUGSTEP:	//Step through Instructions
					if (!gba.loaded) {
						Gui_openFile(hDlg);

						appState.windowsInterface->setPaused(1);

						break;

					}

					appState.armDebugger->browsingProgram = 0;
					appState.armDebugger->saveUndo();	//Save how we were before we did this.
					arm.steps = 1;
					appState.windowsInterface->setPaused(0);
					break;

				case IDC_DEBUGUNDO:	//Undo the last instruction
					appState.armDebugger->undo();
					appState.windowsInterface->setPaused(1);
					break;

				case IDC_DEBUGSAVE:
					selectedState = SendDlgItemMessage (hDlg, IDC_STATENUM, CB_GETCURSEL, (WPARAM)0 , (LPARAM)0);

					if (gba.loaded && (selectedState != CB_ERR)) 
						GBA_saveState(selectedState+1);
					break;

				case IDC_DEBUGLOAD:
					if (!gba.loaded)
						Gui_openFile(hDlg);
					
					if (gba.loaded) {	//If the user did load the file
						selectedState = SendDlgItemMessage (hDlg, IDC_STATENUM, CB_GETCURSEL, (WPARAM)0 , (LPARAM)0);
						
						if (selectedState != CB_ERR) {
							appState.armDebugger->saveUndo();	//Save the previous state	
							GBA_loadState(selectedState+1);
						}
					}
					DebugGui_refresh();
					break;


				
					


				////////////////////////////////////////////////////////////
				//The user modified the text field for the CPU browse address
				//////////////////////////////////////////////////////////
				case IDC_BROWSECPUADDRESS:
					if ( HIWORD(wParam) == CBN_EDITUPDATE) {
						appState.armDebugger->browsingProgram = 1;
						SendDlgItemMessage (hDlg, IDC_BROWSECPUADDRESS, WM_GETTEXT, (WPARAM)(255) , (LPARAM)text);		
						address = strtoul(text, &((char*)nullString), 16) + 8;
						appState.armDebugger->parseInstructionContext(address);
						DebugGui_refresh();

					} else if (HIWORD(wParam) == CBN_SELCHANGE) {
						i = SendDlgItemMessage (hDlg, IDC_BROWSECPUADDRESS, CB_GETCURSEL, (WPARAM)0 , (LPARAM)0);		
						SendDlgItemMessage (hDlg, IDC_BROWSECPUADDRESS, CB_GETLBTEXT, (WPARAM)i , (LPARAM)text);		
						appState.armDebugger->browsingProgram = 1;
						address = strtoul(text, &((char*)nullString), 16) + 8;
						appState.armDebugger->parseInstructionContext(address);
						DebugGui_refresh();
					}	

					break;

					

					//////////////////////////////////////////
				//The user modified the text field for the memory address
				////////////////////////////////////////////////////////
				case IDC_DEBUGMEMADDRESS:
					if (SendDlgItemMessage (hDlg, IDC_RMEMADDRESS, BM_GETCHECK, (WPARAM)0 , (LPARAM)0 ) == BST_UNCHECKED)		
						break;	//Only break if it isn't checked.
	
				case IDC_RMEMADDRESS:
					//Get the memory address set by the user
					appState.armDebugger->useAutoLocation = 0;
					SendDlgItemMessage (hDlg, IDC_DEBUGMEMADDRESS, WM_GETTEXT, (WPARAM)(255) , (LPARAM)text);		
					address = strtoul(text, &((char*)nullString), 16);
					appState.armDebugger->parseMemory(address);
					DebugGui_refresh();
					break;

				case IDC_RAUTO:
					appState.armDebugger->useAutoLocation = 1;
					appState.armDebugger->parseMemory();
					DebugGui_refresh();
					break;

				case IDC_RZERO:
					appState.armDebugger->useAutoLocation = 0;
					appState.armDebugger->parseMemory(ZERO_PAGE_START);
					DebugGui_refresh();
					break;
								
				case IDC_RWRAMEXT:
					appState.armDebugger->useAutoLocation = 0;
					appState.armDebugger->parseMemory(WRAM_EXT_START);
					DebugGui_refresh();
					break;

				case IDC_RWRAMINT:
					appState.armDebugger->useAutoLocation = 0;
					appState.armDebugger->parseMemory(WRAM_INT_START);
					DebugGui_refresh();
					break;

				case IDC_RIO:
					appState.armDebugger->useAutoLocation = 0;
					appState.armDebugger->parseMemory(IORAM_START);
					DebugGui_refresh();
					break;
				
				case IDC_RPAL:
					appState.armDebugger->useAutoLocation = 0;
					appState.armDebugger->parseMemory(PAL_START);
					DebugGui_refresh();
					break;

				case IDC_RVRAM:
					appState.armDebugger->useAutoLocation = 0;
					appState.armDebugger->parseMemory(VRAM_START);
					DebugGui_refresh();
					break;

				case IDC_ROAM:
					appState.armDebugger->useAutoLocation = 0;
					appState.armDebugger->parseMemory(OAM_START);
					DebugGui_refresh();
					break;

				case IDC_RROM:
					appState.armDebugger->useAutoLocation = 0;
					appState.armDebugger->parseMemory(ROM_START);
					DebugGui_refresh();
					break;

				case IDC_RROM1:
					appState.armDebugger->useAutoLocation = 0;
					appState.armDebugger->parseMemory(ROM1_START);
					DebugGui_refresh();
					break;

				case IDC_RROM2:
					appState.armDebugger->useAutoLocation = 0;
					appState.armDebugger->parseMemory(ROM2_START);
					DebugGui_refresh();
					break;

				case IDC_RCARTRAM:
					appState.armDebugger->useAutoLocation = 0;
					appState.armDebugger->parseMemory(CARTRAM_START);
					DebugGui_refresh();
					break;

				case IDC_DEBUGREFRESH:
					Gui_refreshAllDialogs(1);
					break;

				case IDC_BREAKPOINTS:
				if (appState.hBreakpointDlg == NULL)
					appState.hBreakpointDlg = CreateDialogParam (appState.hInstance,MAKEINTRESOURCE(IDD_BREAKPOINTS),
											appState.hWnd,(DLGPROC)BreakpointGui_Proc, (LPARAM)&appState) ;
					break;

             case IDC_DEBUGCLOSE:  //CAncel button
					DeleteObject (listFont);
					DestroyWindow(hDlg);
					return (TRUE) ;
			}
			switch (HIWORD(wParam)) {
				///////////////////////////////////////
				//If the list box got clicked
				/////////////////////////////////////
				case LBN_DBLCLK:
					if (LOWORD(wParam) == IDC_DEBUGLIST) {
						//Determine the address clicked and add or remove a breakpoint.
						if (!gba.loaded) {
							break;
						}
		
						currentIndex = DEBUG_MAXLINES >> 1;	//Index of current instruction
						selectedIndex = SendDlgItemMessage (hDlg, IDC_DEBUGLIST, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);

						if (selectedIndex != LB_ERR) {
							//This calculates the address relative to the address being browsed (
							//which is always in the center of the list box)
							if (arm.thumbMode) {
								selectedIndex = (selectedIndex - currentIndex) << 1;
								address = appState.armDebugger->browseInstructionAddress + (selectedIndex) - 4;
							} else {
								selectedIndex = (selectedIndex - currentIndex) << 2;
								address = appState.armDebugger->browseInstructionAddress + (selectedIndex) - 8;
							}

							if (ARM_containsBreakpoint (address))
								appState.armDebugger->removeBreakpoint(address);
							else

								appState.armDebugger->addBreakpoint(address);

							Gui_refreshAllDialogs(1);
						}
					}
					break;
				default:
					break;
			}
			appState.windowsInterface->render(1);

			//I can't put this here because updateDebugList sends messages
			//to the loop itself, thus causing infinite recursion.
			//DebugGui_refresh();

			return (TRUE);
            break ;

			///////////////////////////////////////////////////////
			//The user clicked on the scroll bar for the memory
			//////////////////////////////////////////////////////
			case WM_VSCROLL:    /* scroll bar was activated */
				//Find out if the message came from the memory scroll bar or the
				//CPU scroll bar.
				if (GetDlgCtrlID((HWND)lParam) == IDC_MEMSCROLL) {
					DebugGui_memScroll(wParam, lParam);
				} else {
					DebugGui_debugScroll(wParam, lParam);
				}

			return (TRUE);
			break;// End of case (WM_HSCROLL)
			

			case WM_RBUTTONDOWN:	//Right click causes mouse popup
				GetWindowPlacement(appState.hDlg, &windowPlacement);
				windowPosition = windowPlacement.rcNormalPosition;
				x = LOWORD(lParam) + windowPosition.left + GetSystemMetrics (SM_CXFIXEDFRAME);
				y = HIWORD(lParam) + windowPosition.top + GetSystemMetrics (SM_CYFIXEDFRAME) + GetSystemMetrics (SM_CYSIZE) + GetSystemMetrics (SM_CYMENU);

				TrackPopupMenuEx (appState.hDebugPopupMenu, TPM_LEFTALIGN|TPM_TOPALIGN|TPM_RIGHTBUTTON, x, y,
						appState.hDlg, NULL);

				break;
	}//End of switch(message)
	
    
	return (FALSE) ;    /* return FALSE to signify message not processed */
}


////////////////////////////////////////////////////////
//DebugGui_refresh()
//This updates the debug dialogue with proper information
//
//
////////////////////////////////////////////////////////
void DebugGui_refresh () {
	HWND hDlg = appState.hDlg;
	//u32 address;
	int pos;
	char tempText[255];
	char* myString;
	//char* nullString = "";

	//if (!gba.loaded)
	//	return;

	//This gets the contents of the registers into a list of strings
	appState.armDebugger->parseRegisters();
	
		
	//NOW update register controls
	for (int i = 0; i < 17; i++) {
		SendDlgItemMessage (hDlg, IDC_R0 + i, WM_SETTEXT, (WPARAM)0 , (LPARAM)appState.armDebugger->regArray[i]);
	}
	
	//Update the CPSR flags
	if (arm.N)
		CheckDlgButton(hDlg, IDC_NCHECK, BST_CHECKED);
	else 
		CheckDlgButton(hDlg, IDC_NCHECK, BST_UNCHECKED);

	if (arm.Z)
		CheckDlgButton(hDlg, IDC_ZCHECK, BST_CHECKED);
	else 
		CheckDlgButton(hDlg, IDC_ZCHECK, BST_UNCHECKED);

	if (arm.C)
		CheckDlgButton(hDlg, IDC_CCHECK, BST_CHECKED);
	else 
		CheckDlgButton(hDlg, IDC_CCHECK, BST_UNCHECKED);

	if (arm.V)
		CheckDlgButton(hDlg, IDC_VCHECK, BST_CHECKED);
	else 
		CheckDlgButton(hDlg, IDC_VCHECK, BST_UNCHECKED);



	if (arm.CPSR & 0x80)
		CheckDlgButton(hDlg, IDC_ICHECK, BST_CHECKED);
	else 
		CheckDlgButton(hDlg, IDC_ICHECK, BST_UNCHECKED);



	if (arm.CPSR & 0x40)
		CheckDlgButton(hDlg, IDC_FCHECK, BST_CHECKED);
	else 
		CheckDlgButton(hDlg, IDC_FCHECK, BST_UNCHECKED);



	if (arm.CPSR & 0x20)
		CheckDlgButton(hDlg, IDC_TCHECK, BST_CHECKED);
	else 
		CheckDlgButton(hDlg, IDC_TCHECK, BST_UNCHECKED);

	//Display the current mode
	switch (arm.CPSR & 0x0F) {
		case 0x0:	SendDlgItemMessage (hDlg, IDC_DEBUGMODE, WM_SETTEXT, (WPARAM)0 , (LPARAM)"User");
			break;
		case 0x1:	SendDlgItemMessage (hDlg, IDC_DEBUGMODE, WM_SETTEXT, (WPARAM)0 , (LPARAM)"FIQ");
			break;
		case 0x2:	SendDlgItemMessage (hDlg, IDC_DEBUGMODE, WM_SETTEXT, (WPARAM)0 , (LPARAM)"IRQ");
			break;
		case 0x3:	SendDlgItemMessage (hDlg, IDC_DEBUGMODE, WM_SETTEXT, (WPARAM)0 , (LPARAM)"Super");
			break;
		case 0x7:	SendDlgItemMessage (hDlg, IDC_DEBUGMODE, WM_SETTEXT, (WPARAM)0 , (LPARAM)"Abort");
			break;
		case 0xB:	SendDlgItemMessage (hDlg, IDC_DEBUGMODE, WM_SETTEXT, (WPARAM)0 , (LPARAM)"Undef.");
			break;
		case 0xF:	SendDlgItemMessage (hDlg, IDC_DEBUGMODE, WM_SETTEXT, (WPARAM)0 , (LPARAM)"System");
			break;
		default:	SendDlgItemMessage (hDlg, IDC_DEBUGMODE, WM_SETTEXT, (WPARAM)0 , (LPARAM)"Invalid");
			break;

	}

	//Display the current vcount
	sprintf(tempText, "%d", *REG_VCOUNT);
	SendDlgItemMessage (hDlg, IDC_DEBUGVCOUNT, WM_SETTEXT, (WPARAM)0 , (LPARAM)tempText);
	//Display the dispcnt value
	sprintf(tempText, "%04X", *REG_STAT);
	SendDlgItemMessage (hDlg, IDC_DEBUGDISPSTAT, WM_SETTEXT, (WPARAM)0 , (LPARAM)tempText);

	//Take care of setting the scroll bar range for memory
	SendDlgItemMessage (hDlg, IDC_MEMSCROLL, SBM_SETRANGE, (WPARAM)0, (LPARAM)appState.armDebugger->browseMemSize/16);


	
	//Set the scroll range for the instruction code debugger
	if (arm.thumbMode)
		SendDlgItemMessage (hDlg, IDC_DEBUGSCROLL, SBM_SETRANGE, (WPARAM)0, (LPARAM)appState.armDebugger->browseInstructionMemSize >> 1);
	else
		SendDlgItemMessage (hDlg, IDC_DEBUGSCROLL, SBM_SETRANGE, (WPARAM)0, (LPARAM)appState.armDebugger->browseInstructionMemSize >> 2);

	//Create string representations of current debug data for CPU and memory.
	appState.armDebugger->parseInstructionContext();
	appState.armDebugger->parseMemory();

	//Set the scroll bar positions based on the current browsing addresses
	pos = (appState.armDebugger->browseMemAddress & 0x00FFFFFF)/16;
	SendDlgItemMessage (hDlg, IDC_MEMSCROLL, SBM_SETPOS, (WPARAM)pos, (LPARAM)TRUE);

	//The position of the scroll bar will be different if we are in thumb mode
	if (arm.thumbMode)
		pos = (appState.armDebugger->browseInstructionAddress & 0x00FFFFFF) >> 1;
	else
		pos = (appState.armDebugger->browseInstructionAddress & 0x00FFFFFF) >> 2;

	SendDlgItemMessage (hDlg, IDC_DEBUGSCROLL, SBM_SETPOS, (WPARAM)pos, (LPARAM)TRUE);

	//Turn of redraw for the asm list and the memory list
	SendDlgItemMessage (hDlg, IDC_MEMORYLIST, WM_SETREDRAW, FALSE, 0);  
	SendDlgItemMessage (hDlg, IDC_DEBUGLIST, WM_SETREDRAW, FALSE, 0);  

	//Clear out the list boxes so that they can be refilled
	SendDlgItemMessage (hDlg, IDC_MEMORYLIST, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
	SendDlgItemMessage (hDlg, IDC_DEBUGLIST, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);

	//Fill up the list boxes
	for (i = 0; i < DEBUG_MAXMEMLINES; i ++) {
			myString = appState.armDebugger->memArray[i];
			SendDlgItemMessage (hDlg, IDC_MEMORYLIST, LB_INSERTSTRING, (WPARAM)i, (LPARAM)myString);
	}

	for (i = 0; i < DEBUG_MAXLINES; i++) {
			myString = appState.armDebugger->instructionArray[i];
			SendDlgItemMessage (hDlg, IDC_DEBUGLIST, LB_INSERTSTRING, (WPARAM)i, (LPARAM)myString);
	}


	//Set the cursor to point to the instruction that is halfway down the screen.			

	int currentIndex = DEBUG_MAXLINES >> 1;
	SendDlgItemMessage (hDlg, IDC_DEBUGLIST, LB_SETCURSEL, (WPARAM)(currentIndex) , (LPARAM)FALSE);

	
	//Fill in the user entry with the current value
	SendDlgItemMessage (hDlg, IDC_DEBUGMEMVAL, WM_SETTEXT, (WPARAM)0 , (LPARAM)appState.armDebugger->memVal);
	SendDlgItemMessage (hDlg, IDC_DEBUGCURRENTMEMADDRESS, WM_SETTEXT, (WPARAM)0 , (LPARAM)appState.armDebugger->locationVal);
	

	//Fill up the combo box with a list of breakpoints
	SendDlgItemMessage (hDlg, IDC_DEBUGADDRESS, CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);

	for (i = 0; i < arm.numBreakpoints; i++) {
		sprintf(tempText, "%X", arm.breakpoints[i]);
		SendDlgItemMessage (hDlg, IDC_DEBUGADDRESS, CB_ADDSTRING, (WPARAM)0, (LPARAM)&tempText);
	}

	//Set the text of the combo box list entry to the last breakpoint entered
	SendDlgItemMessage (hDlg, IDC_DEBUGADDRESS, WM_SETTEXT, (WPARAM)0 , (LPARAM)appState.settings.debugSettings.lastBreakpointText);

	//Is this really the best place for this code?  It would be nice
	//to load the menus when a stab is first opened, but that would
	//require the stab loader module to have a handle to the menu.  Yuck.
	StabSourceFile* sourceFile;
	
	
	
	
	
	//Turn back on redraw for the asm list and the memory list
	SendDlgItemMessage (hDlg, IDC_MEMORYLIST, WM_SETREDRAW, TRUE, 0);  
	SendDlgItemMessage (hDlg, IDC_DEBUGLIST, WM_SETREDRAW, TRUE, 0);  

	//Force windows to redraw the dialog (and the list boxes, which have since changed).
	InvalidateRect(appState.hDlg, NULL, FALSE);

	//if (sourceGui != NULL)
	//	SourceGui_refres


}//updateDebugList





//////////////////////////////////////////////////////
//DebugGui_memScroll
//Handles scrolling for the memory scroll bar.
///////////////////////////////////////////////////////
void DebugGui_memScroll(WPARAM wParam, LPARAM lParam) {
	u32 temp, address;
	appState.hMemScroll = (HWND)lParam;
	temp = 0;
	//If the user is clicking the scrollbar, they're
	//not using auto location
	//Turn off auto location
	if (appState.armDebugger->useAutoLocation) {
		appState.armDebugger->useAutoLocation = 0;
		temp = 1;
	}

	switch (LOWORD(wParam)) /* check what part was pressed */
	{
		case SB_THUMBTRACK:  /* user movded scroll thumb */
			//Get the offset into memory
			address = appState.armDebugger->browseMemAddress & 0xFF000000;

			//Add the value from the scroll bar.
			//Note that this should break for values larger than 65,536 :(
			appState.armDebugger->parseMemory(address + HIWORD (wParam) * 16);// - 16);
            break;

		case SB_THUMBPOSITION:  /* user movded scroll thumb */
			//Get the offset into memory
			address = appState.armDebugger->browseMemAddress & 0xFF000000;

			//Add the value from the scroll bar.
			appState.armDebugger->parseMemory(address + GetScrollPos((HWND)lParam, SB_CTL) * 16 ); //- 16);
			break ;

		case SB_LINEDOWN:       /* user clicked down (right) arrow */
			appState.armDebugger->parseMemory(appState.armDebugger->browseMemAddress+16);
			break ;

		case SB_LINEUP:         /* user clicked up (left) arrow */
			appState.armDebugger->parseMemory(appState.armDebugger->browseMemAddress-16);
			break ;

		case SB_PAGEDOWN:       /* user clicked down (right) area */
			//(this actually prevents the user from scrolling to the last 
			//256 bytes of memory without using the little arrow buttons)
			appState.armDebugger->parseMemory(appState.armDebugger->browseMemAddress+256);
			break ;

		case SB_PAGEUP:         /* user clicked up (right) area */
			//(this actually prevents the user from scrolling to the first 
			//256 bytes of memory without using the little arrow buttons)
			appState.armDebugger->parseMemory(

			//Find the minimum for this unit of memory
			max(appState.armDebugger->browseMemAddress-256,appState.armDebugger->browseMemAddress & 0xFF000000));
			break ;

		default:
			break;
		}//end of switch (LOWORD(wParam)

		//Set it to be a user defined address
		//CheckRadioButton( hDlg, IDC_RMEMADDRESS, IDC_RZERO, IDC_RMEMADDRESS);

		//For all actions, update the screen.
		appState.windowsInterface->render(1);
		DebugGui_refresh();

		//See if we turned off useAutoLocation
		if (temp) {
			appState.armDebugger->useAutoLocation = 1;
			temp = 0;
		}
}



//////////////////////////////////////////////////////
//DebugGui_debugScroll
//Handles scrolling for the cpu scroll bar.
//Note that I don't really like the way this interfaces with the
//debugger.
///////////////////////////////////////////////////////

void DebugGui_debugScroll(WPARAM wParam, LPARAM lParam) {
	u32 address;
	appState.hDebugScroll = (HWND)lParam;
	u32 instructionSize;

	//When the user scrolls in the debug window, we want to check a flag,
	//and uncheck it when the user resumes normal debugging (stepping, etc.)
	//in order to return the cursor to the PC location.
	appState.armDebugger->browsingProgram = 1;

	if (arm.thumbMode)
		instructionSize = 2;
	else
		instructionSize = 4;

	switch (LOWORD(wParam)) /* check what part was pressed */
	{
		case SB_THUMBTRACK:  /* user movded scroll thumb */
			//Get the offset into memory
			address = appState.armDebugger->browseInstructionAddress & 0xFF000000;

			//Add the value from the scroll bar.
			//Note that this should break for values larger than 65,536 :(
			appState.armDebugger->parseInstructionContext(address + HIWORD (wParam) * instructionSize);// - 16);
            break;

		case SB_THUMBPOSITION:  /* user movded scroll thumb */

			//Get the offset into memory

			address = appState.armDebugger->browseInstructionAddress & 0xFF000000;



			//Add the value from the scroll bar.

			appState.armDebugger->parseInstructionContext(address + GetScrollPos((HWND)lParam, SB_CTL) * instructionSize ); //- 16);

			break ;



		case SB_LINEDOWN:       /* user clicked down (right) arrow */

			appState.armDebugger->parseInstructionContext(appState.armDebugger->browseInstructionAddress+instructionSize);

			break ;



		case SB_LINEUP:         /* user clicked up (left) arrow */

			appState.armDebugger->parseInstructionContext(appState.armDebugger->browseInstructionAddress-instructionSize);

			break ;



		case SB_PAGEDOWN:       /* user clicked down (right) area */

			//(this actually prevents the user from scrolling to the last 

			//64 bytes of memory without using the little arrow buttons)

			appState.armDebugger->parseInstructionContext (appState.armDebugger->browseInstructionAddress + (instructionSize*DEBUG_MAXLINES));

			break ;



		case SB_PAGEUP:         /* user clicked up (right) area */

			//(this actually prevents the user from scrolling to the first 

			//256 bytes of memory without using the little arrow buttons)

			appState.armDebugger->parseInstructionContext(

		

			//Find the minimum for this segment of memory.

			//We want to scroll either 16 lines back or to the beginning of this segment,

			//whichever is greater.

			max(appState.armDebugger->browseInstructionAddress - (instructionSize*DEBUG_MAXLINES),arm.r[15] & 0xFF000000));

			break ;



		default:

		break;

	}//end of switch (LOWORD(wParam)

				

	//Set it to be a user defined address

	//CheckRadioButton( hDlg, IDC_RMEMADDRESS, IDC_RZERO, IDC_RMEMADDRESS);



	//For all actions, update the screen.

	appState.windowsInterface->render(1);

	DebugGui_refresh();

}//debugScroll


