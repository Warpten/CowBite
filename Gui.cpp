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

//Thomas Happ
//CowKnight




//#include <afxext.h>	//Already has windows.h.
#include <windows.h>
#include <math.h>
#include <time.h>	//For the clock() function
#include <conio.h>
#include <stdio.h>	//for sprintf
#include <direct.h> //for cwd
#include <string.h>	//For strtock
#include <commctrl.h>	//For the toolbar


#include "resource.h"
#include "Gui.h"


#include "GBA.h"
#include "ARM.h"
#include "GBADebugger.h"
#include "ARMDebugger.h"
#include "Graphics.h"
#include "IO.h"
#include "Console.h"
//#include "ARM.cpp"

#include "WindowsInterface.h"
#include "Support.h"
#include "2xsai.h"

//extern void ARM_execute();


//////////////////////////////////////////////////
//WndProc
//The main windows callback.  Obligatorily huge and nasty.
////////////////////////////////////////////////
LRESULT CALLBACK Gui_WndProc (HWND hWnd, UINT wMessage,
                                    WPARAM wParam, LPARAM lParam)
{
	appState.hWnd = hWnd;
	WINDOWPLACEMENT windowPlacement;
	RECT windowPosition;
	char text[255];
	long i;

	int x, y;

	static int nRetVal = 0;
	static int blah;
	//Audio_stopSound();

    switch (wMessage)                 // process Windows messages
    {
			case WM_CREATE:
			//Get the instance for the window.  THis is used in opening the file.
			appState.hInstance = (HINSTANCE)GetWindowLong (hWnd, GWL_HINSTANCE) ; 
			

			//appState.hMainMenu = CreateMenu () ;
			appState.hMainMenu = LoadMenu(appState.hInstance, MAKEINTRESOURCE(IDR_MAINMENU));
			appState.hPopupMenu = CreatePopupMenu () ;
			appState.hGraphicsMenu = LoadMenu(appState.hInstance, MAKEINTRESOURCE(IDR_GRAPHICSMENU));//CreatePopupMenu();
			appState.hDebugMenu = LoadMenu(appState.hInstance, MAKEINTRESOURCE(IDR_DEBUGMENU));
			appState.hRefreshMenu = CreatePopupMenu();
			appState.hGraphicsDebugMenu = LoadMenu(appState.hInstance, MAKEINTRESOURCE(IDR_GRAPHICSDEBUGMENU));
			appState.hViewMenu = LoadMenu (appState.hInstance, MAKEINTRESOURCE(IDR_VIEWMENU));
			appState.hDebugSourceMenu = CreatePopupMenu();
			
			CheckMenuRadioItem (appState.hMainMenu, IDM_LOAD1, IDM_LOAD10, IDM_LOAD1, MF_BYCOMMAND);
			CheckMenuRadioItem (appState.hMainMenu, IDM_SAVE1, IDM_SAVE10, IDM_SAVE1, MF_BYCOMMAND);

			CheckMenuItem(appState.hGraphicsDebugMenu, IDM_BG0, MF_BYCOMMAND | MF_CHECKED);
			CheckMenuItem(appState.hGraphicsDebugMenu, IDM_BG1, MF_BYCOMMAND | MF_CHECKED);
			CheckMenuItem(appState.hGraphicsDebugMenu, IDM_BG2, MF_BYCOMMAND | MF_CHECKED);
			CheckMenuItem(appState.hGraphicsDebugMenu, IDM_BG3, MF_BYCOMMAND | MF_CHECKED);
			CheckMenuItem(appState.hGraphicsDebugMenu, IDM_OBJ, MF_BYCOMMAND | MF_CHECKED);
			CheckMenuItem(appState.hGraphicsDebugMenu, IDM_BLENDS, MF_BYCOMMAND | MF_CHECKED);
			CheckMenuItem(appState.hGraphicsDebugMenu, IDM_PRIORITY0, MF_BYCOMMAND | MF_CHECKED);
			CheckMenuItem(appState.hGraphicsDebugMenu, IDM_PRIORITY1, MF_BYCOMMAND | MF_CHECKED);
			CheckMenuItem(appState.hGraphicsDebugMenu, IDM_PRIORITY2, MF_BYCOMMAND | MF_CHECKED);
			CheckMenuItem(appState.hGraphicsDebugMenu, IDM_PRIORITY3, MF_BYCOMMAND | MF_CHECKED);
			AppendMenu (appState.hGraphicsMenu, MF_POPUP, (UINT)appState.hGraphicsDebugMenu, "&Debug");
	
			CheckMenuRadioItem (appState.hMainMenu, IDM_REFRESH0, IDM_REFRESH120, 
				appState.settings.debugSettings.dynamicRefresh+IDM_REFRESH0, MF_BYCOMMAND);
			
			//This turns IDM_1X - IDM_SUPER2XSAI into radio items
			CheckMenuRadioItem (appState.hGraphicsMenu, IDM_1X, IDM_SUPER2XSAI, IDM_2X, MF_BYCOMMAND);
            
			//attach the graphics debug menu to the view menu
			InsertMenu( appState.hViewMenu, IDM_HARDWAREREGS, MF_BYCOMMAND|MF_POPUP, 
						(UINT)appState.hGraphicsDebugMenu, "&Graphics");
 
			if (arm.debugging)
				CheckMenuItem(appState.hDebugMenu, IDM_ENTERDEBUG, MF_BYCOMMAND | MF_CHECKED);
			else
				CheckMenuItem(appState.hDebugMenu, IDM_ENTERDEBUG, MF_BYCOMMAND | MF_UNCHECKED);

	
			        
			//Attach them to the main menu
			AppendMenu (appState.hMainMenu, MF_POPUP, (UINT)appState.hGraphicsMenu, "&Graphics");
			AppendMenu (appState.hMainMenu, MF_POPUP, (UINT)appState.hDebugMenu, "&Debug");						
			AppendMenu (appState.hMainMenu, MF_POPUP, (UINT)appState.hViewMenu, "&View");	
			AppendMenu (appState.hMainMenu, MF_POPUP, (UINT)appState.hDebugSourceMenu, "&Source");

			//Attach all of the seperate menus to the popup menu
			AppendMenu (appState.hPopupMenu, MF_POPUP, (UINT)appState.hGraphicsMenu, "&Graphics");
			AppendMenu (appState.hPopupMenu, MF_POPUP, (UINT)appState.hDebugMenu, "&Debug");						
			AppendMenu (appState.hPopupMenu, MF_POPUP, (UINT)appState.hViewMenu, "&View");	
			AppendMenu (appState.hPopupMenu, MF_POPUP, (UINT)appState.hDebugSourceMenu, "&Source");
			
			AppendMenu (appState.hDebugSourceMenu, MF_STRING, IDM_AUTOSOURCE, "Auto Focus Source Windows") ;				
			AppendMenu (appState.hDebugSourceMenu, MF_STRING, IDM_NULL, "(Only available for ELF files compiled with -gstabs)") ;				
			CheckMenuItem(appState.hDebugSourceMenu, IDM_AUTOSOURCE, MF_BYCOMMAND | MF_CHECKED);

			AppendMenu (appState.hMainMenu, MF_STRING, IDM_ABOUT, "&About") ;
				

			//Set it to be our menu
			SetMenu (hWnd, appState.hMainMenu);

			appState.hInstance = (HINSTANCE)GetWindowLong (appState.hWnd, GWL_HINSTANCE) ;

			appState.hBackgroundDlg = NULL;
			appState.hSpriteDlg = NULL;
			appState.hHardwareRegDlg = NULL;


			

            break;     
	
	case WM_SHOWWINDOW:
			SetWindowPos( hWnd, NULL, appState.settings.winPos.x,
				appState.settings.winPos.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER );

			break;
		case WM_MOVE:
			{	RECT rect;
				GetWindowRect(hWnd, &rect);
				appState.settings.winPos.x = rect.left;//LOWORD(lParam);
				appState.settings.winPos.y = rect.top;//HIWORD(lParam);
			}
			break;
	
	case WM_COMMAND:
		//Handle saving and loading of states
		if ( (wParam >= IDM_LOAD1) && (wParam <= IDM_LOAD10)) {
			myInterface->selectState (wParam - IDM_LOAD1 + 1);
			GBA_loadState(wParam - IDM_LOAD1 + 1);
					
		} else if ( (wParam >= IDM_SAVE1) && (wParam <= IDM_SAVE10)) {
			myInterface->selectState (wParam - IDM_SAVE1 + 1);
			GBA_saveState(wParam - IDM_SAVE1 + 1);
		}

		//Handle the refresh buttons.
		else if ( (wParam >= IDM_REFRESH0) && (wParam <= IDM_REFRESH120)) {
			CheckMenuRadioItem (appState.hMainMenu, IDM_REFRESH0, IDM_REFRESH120, wParam, MF_BYCOMMAND);
			switch (wParam) {
				case IDM_REFRESH0:	appState.settings.debugSettings.framesPerRefresh = 0;	break;
				case IDM_REFRESH1:	appState.settings.debugSettings.framesPerRefresh = 1;	break;
				case IDM_REFRESH2:	appState.settings.debugSettings.framesPerRefresh = 2;	break;
				case IDM_REFRESH4:	appState.settings.debugSettings.framesPerRefresh = 4;	break;
				case IDM_REFRESH8:	appState.settings.debugSettings.framesPerRefresh = 8;	break;
				case IDM_REFRESH15:	appState.settings.debugSettings.framesPerRefresh = 15;	break;
				case IDM_REFRESH30:	appState.settings.debugSettings.framesPerRefresh = 30;	break;
				case IDM_REFRESH60:	appState.settings.debugSettings.framesPerRefresh = 60;	break;
				case IDM_REFRESH120:	appState.settings.debugSettings.framesPerRefresh = 120;	break;
			}
			appState.settings.debugSettings.dynamicRefresh = wParam - IDM_REFRESH0;
			appState.framesUntilDebugRefresh = appState.settings.debugSettings.framesPerRefresh;
		}
		if ( (LOWORD(wParam) >= IDM_SOURCEFILE1) && (LOWORD(wParam) <= IDM_SOURCEFILE256)) {
				SourceGui* sourceGui;
				char filename[_MAX_PATH];
				GetMenuString(appState.hDebugSourceMenu, LOWORD(wParam), filename, _MAX_PATH, MF_BYCOMMAND);
				sourceGui = SourceGui_getByFilename(&gba.stabs, filename);
				if (sourceGui!= NULL && sourceGui->hWnd != NULL) {

					SetFocus(sourceGui->hWnd);
				}
		}


		switch (wParam)               // menu item selected
      {
		case IDM_AUTOSOURCE:
			if (GetMenuState(appState.hDebugSourceMenu, IDM_AUTOSOURCE, MF_BYCOMMAND) & MF_CHECKED) {
				CheckMenuItem(appState.hDebugSourceMenu, IDM_AUTOSOURCE, MF_BYCOMMAND | MF_UNCHECKED);
			} else {
				CheckMenuItem(appState.hDebugSourceMenu, IDM_AUTOSOURCE, MF_BYCOMMAND | MF_CHECKED);
			}
			break;

		case IDM_ABOUT:
				MessageBox (hWnd, "\tCowBite\n\n\tby Thomas Happ\t\n\n\tSorcererXIII@yahoo.com\t\n\n\tCopyright 2001,2002\n\n\tSuper Eagle and Super 2xSaI filters by Kreed\t", 
								"CowBite", MB_OK);
			break;

		case IDM_OPEN:
			Gui_openFile (hWnd);
			break ;
	
		case IDM_1X:
			myInterface->set1X();
			break;
		case IDM_2X:
			myInterface->set2X();
			break;
		case IDM_3X:
			myInterface->set3X();
			break;
		case IDM_SUPEREAGLE:
			myInterface->setSuperEagle();
			break;
		case IDM_SUPER2XSAI:
			myInterface->setSuper2xSaI();
			break;
		
		/////////////////////////////////////
		//This is the command used when the user hits the frameskip button
		/////////////////////////////////////////////////////////////
		case IDM_FRAMESKIP:
			
			if (appState.settings.framesPerRefresh > 1) {
				myInterface->setFrameSkip(1);	
			} else {
				if (appState.fps) {
					u32 frames = (60 << 8)/appState.fps;
					appState.settings.framesPerRefresh = (frames >> 8) + ((frames & 0xFF) > 8);
					if (appState.settings.framesPerRefresh > 6)	//Set a limit of 6
						appState.settings.framesPerRefresh = 6;
					if (appState.settings.framesPerRefresh < 1)	//Set a limit of 6
						appState.settings.framesPerRefresh = 1;

				} else {
					appState.settings.framesPerRefresh = 2;
				}
				myInterface->setFrameSkip(appState.settings.framesPerRefresh);
			} 
			
			break;
		case IDM_FRAMESKIP0:
		case IDM_FRAMESKIP1:
		case IDM_FRAMESKIP2:
		case IDM_FRAMESKIP3:
		case IDM_FRAMESKIP4:
		case IDM_FRAMESKIP5:
		case IDM_FRAMESKIP6:
			myInterface->setFrameSkip(1 + wParam - IDM_FRAMESKIP0);
			break;
		

		case IDM_ENTERDEBUG:
			myInterface->enterDebug();
			break;
		case IDM_DISASSEMBLY:
			if (appState.hDlg == NULL)
				appState.hDlg = CreateDialogParam (appState.hInstance,MAKEINTRESOURCE(IDD_DEBUG),
											appState.hWnd,(DLGPROC)DebugGui_DebugProc, (LPARAM)&appState) ;
			break;

		case IDM_REGISTERS:
			if (appState.hRegistersDlg == NULL)
				appState.hRegistersDlg = CreateDialogParam (appState.hInstance,MAKEINTRESOURCE(IDD_REGISTERS),
											appState.hWnd,(DLGPROC)RegistersGui_Proc, (LPARAM)&appState) ;
			
			break;
		case IDM_VERSIONVII:
			if (appState.hDlg == NULL)
				appState.hDlg = CreateDialogParam (appState.hInstance,MAKEINTRESOURCE(IDD_DEBUG),
											appState.hWnd,(DLGPROC)DebugGui_DebugProc, (LPARAM)&appState) ;
			break;

		case IDM_CONSOLE:
			//I the case of the console, we are showing something that otherwise
			//remains hidden.
			ShowWindow(appState.hConsoleDlg, SW_SHOW);
			
			break;

		case IDM_HARDWAREREGS:
			if (appState.hHardwareRegDlg == NULL)
				appState.hHardwareRegDlg = CreateDialogParam (appState.hInstance,MAKEINTRESOURCE(IDD_HARDWAREREGS),
										appState.hWnd,(DLGPROC)HardwareRegGui_Proc, (LPARAM)&appState) ;
			break;
		
		case IDM_OBJS:
			//Necessary to check for NULL because it'sa  non modal dialog
			if (appState.hObjDlg == NULL)
				appState.hObjDlg = CreateDialogParam (appState.hInstance,MAKEINTRESOURCE(IDD_OBJS),
										appState.hWnd,(DLGPROC)ObjGui_Proc, (LPARAM)&appState) ;
			break;

			
		case IDM_SPRITETILES:
			//Necessary to check for NULL because it'sa  non modal dialog
			if (appState.hSpriteDlg == NULL)
				appState.hSpriteDlg = CreateDialogParam (appState.hInstance,MAKEINTRESOURCE(IDD_SPRITEDIALOG),
										appState.hWnd,(DLGPROC)TileGui_SpriteTilesProc, (LPARAM)&appState) ;

			break;
		case IDM_BGMAPS:
			//Necessary to check for NULL because it'sa  non modal dialog
			if (appState.hMapDlg == NULL)
				appState.hMapDlg = CreateDialogParam (appState.hInstance,MAKEINTRESOURCE(IDD_BGMAPS),
										appState.hWnd,(DLGPROC)MapGui_Proc, (LPARAM)&appState) ;
			break;

		case IDM_BGTILES:
			if (appState.hBackgroundDlg == NULL)
				appState.hBackgroundDlg = CreateDialogParam (appState.hInstance,MAKEINTRESOURCE(IDD_BGDIALOG),
										appState.hWnd,(DLGPROC)TileGui_BackgroundTilesProc, (LPARAM)&appState) ;
			break;
		case IDM_PALETTE:
			if (appState.hPaletteDlg == NULL)
				appState.hPaletteDlg = CreateDialogParam (appState.hInstance,MAKEINTRESOURCE(IDD_PALETTE),
										appState.hWnd,(DLGPROC)PaletteGui_Proc, (LPARAM)&appState) ;
			break;

		case IDM_STATISTICS:
			if (appState.hStatsDlg == NULL)
				appState.hStatsDlg = CreateDialogParam (appState.hInstance,MAKEINTRESOURCE(IDD_STATISTICS),
										appState.hWnd,(DLGPROC)StatsGui_Proc, (LPARAM)&appState) ;
			break;
		
		case IDM_VARIABLES:
					if (appState.hVariablesDlg == NULL)
						appState.hVariablesDlg = CreateDialogParam (appState.hInstance,MAKEINTRESOURCE(IDD_VARIABLES),
												appState.hWnd,(DLGPROC)VariablesGui_Proc, (LPARAM)&appState);
				break;
		case IDM_INPUT:
			DialogBoxParam (appState.hInstance,MAKEINTRESOURCE(IDD_INPUTDIALOG),
										appState.hWnd,(DLGPROC)InputGui_Proc, (LPARAM)&appState) ;
			
			break;
		////////////////////////////////////////////////
		//Opens up a window for adding breakpionts
		////////////////////////////////////////////////
		case IDM_BREAKPOINTS:
			if (appState.hBreakpointDlg == NULL)
				appState.hBreakpointDlg = CreateDialogParam (appState.hInstance,MAKEINTRESOURCE(IDD_BREAKPOINTS),
										appState.hWnd,(DLGPROC)BreakpointGui_Proc, (LPARAM)&appState) ;
				break;

		/////////////////////////////////////////////////
		//opens up a window for dumping and loading.
		////////////////////////////////////////////////
		case IDM_DEBUGDUMP:
			if (appState.hDumpDlg == NULL)
			appState.hDumpDlg = CreateDialogParam (appState.hInstance,MAKEINTRESOURCE(IDD_DUMPDIALOG),
										appState.hWnd,(DLGPROC)DumpGui_Proc, (LPARAM)&appState) ;
			break;


		case IDM_PAUSE: {
		
		} 
			myInterface->setPaused(!arm.paused);
			break;
/*
		case IDM_CUT:
			myInterface->setPaused(!arm.paused);
			break;*/

		case IDM_RESET:
			GBA_reset();
			appState.armDebugger->reset();
			myInterface->render(1);	//Force it to update
			Gui_refreshAllDialogs (1);
			//Gui_pause(0);	//Unpause it so as not to confuse the user.
			break;
	
		case IDM_STATESAVE:
				GBA_saveState(appState.settings.selectedState);
				break;

		case IDM_STATELOAD:
				
				if (!gba.loaded)
					Gui_openFile(appState.hDlg);
					
				GBA_loadState(appState.settings.selectedState);
				appState.windowsInterface->render(1);
				Gui_refreshAllDialogs (1);
				break;
		case IDM_BG0:
		case IDM_BG1:
		case IDM_BG2:
		case IDM_BG3:
		case IDM_OBJ:
		case IDM_BLENDS:
			if (GetMenuState(appState.hGraphicsDebugMenu, wParam, MF_BYCOMMAND) & MF_CHECKED)  {
				CheckMenuItem(appState.hGraphicsDebugMenu, wParam, MF_BYCOMMAND | MF_UNCHECKED);
				graphics.masterVisibility &= (0xFFFFFFDF >> (5 - (wParam - IDM_BG0)));
			} else {
				CheckMenuItem(appState.hGraphicsDebugMenu, wParam, MF_BYCOMMAND | MF_CHECKED);
				graphics.masterVisibility |= 1 << ((wParam - IDM_BG0));
				
			}
			
			break;
		
		case IDM_PRIORITY0:
		case IDM_PRIORITY1:
		case IDM_PRIORITY2:
		case IDM_PRIORITY3:
			if (GetMenuState(appState.hGraphicsDebugMenu, wParam, MF_BYCOMMAND) & MF_CHECKED)  {
				CheckMenuItem(appState.hGraphicsDebugMenu, wParam, MF_BYCOMMAND | MF_UNCHECKED);
				graphics.priorityArray[wParam - IDM_PRIORITY0].visible = 0;
			} else {
				CheckMenuItem(appState.hGraphicsDebugMenu, wParam, MF_BYCOMMAND | MF_CHECKED);
				graphics.priorityArray[wParam - IDM_PRIORITY0].visible = 1;
			}
			break;
		case IDM_QUIT:
			//Must put this here as DestroyWindow will automatically destroy
			//all windows, so we can't record whether they were open at exit.
			myInterface->saveSettings();
	      DestroyWindow (appState.hWnd);  // destroy window
         break;  
   	}


		break;
	 
	 case WM_NOTIFY:	//Handle tooltip messages
		 switch (((LPNMHDR) lParam)->code) { 
			  case TTN_NEEDTEXT: 
					{ 
						LPTOOLTIPTEXT lpttt; 
 
						lpttt = (LPTOOLTIPTEXT) lParam; 
						lpttt->hinst = NULL;
 
						// Specify the resource identifier of the descriptive 
						// text for the given button. 
						u32 idButton = lpttt->hdr.idFrom; 
						//lpttt->lpszText = "Open a file.";
						
						switch (idButton) { 
							case IDM_OPEN: 
								  lpttt->lpszText = "Open"; 
								  break; 
							default:
									lpttt->lpszText = "Unimplemented."; 
									break;

                
						} 
						break; 
					} 
					break;
			  default: 
					break; 
		 } 
		break;

    
  case WM_DESTROY:               // stop application 
		
		KillTimer (appState.hWnd, 1) ;
      PostQuitMessage (0);
    	break;

	case WM_MOUSEMOVE:
		
		switch (wParam) {
			case MK_LBUTTON:
				myInterface->lButtonDown (LOWORD (lParam), HIWORD (lParam));
				break;
			
			default: 
				myInterface->mouseMove (LOWORD (lParam), HIWORD (lParam));
				break;
		}
		break;
	
	case WM_RBUTTONDOWN:	//Right click causes mouse popup
		
		GetWindowPlacement(appState.hWnd, &windowPlacement);
		windowPosition = windowPlacement.rcNormalPosition;
		x = LOWORD(lParam) + windowPosition.left + GetSystemMetrics (SM_CXFIXEDFRAME);
		y = HIWORD(lParam) + windowPosition.top + GetSystemMetrics (SM_CYFIXEDFRAME) + GetSystemMetrics (SM_CYSIZE) + GetSystemMetrics (SM_CYMENU);

		TrackPopupMenuEx (appState.hPopupMenu, TPM_LEFTALIGN|TPM_TOPALIGN|TPM_RIGHTBUTTON, x, y,
						hWnd, NULL);
		//TrackPopupMenuEx (appState.hMainMenu, TPM_LEFTALIGN|TPM_TOPALIGN|TPM_RIGHTBUTTON, x, y,
		//				hWnd, NULL);
		
		break;
	case WM_LBUTTONDOWN:
		myInterface->lButtonDown (LOWORD (lParam), HIWORD (lParam));
		break;
	
	case WM_LBUTTONUP:
		myInterface->lButtonUp (LOWORD (lParam), HIWORD (lParam));
		break;
	
	case WM_ERASEBKGND:
	case WM_NCPAINT:
		myInterface->redrawFrame();
		
		//This redraws the borders, etc. of the window
		return DefWindowProc (hWnd, wMessage, wParam, lParam);
		break;


	/*
	case WM_PAINT:	//This must be commented out for debugging
			//If there's not update rectangle, break out.
			
			myInterface->redrawFrame();
			break;
	*//*
	case WM_CAPTURECHANGED:			//We lost focus:(
		myInterface->lostFocus();	//Doesn't work!
		break;*/

	case WM_KEYDOWN:
		//Save or load a state
		//(According to the windows header files, "VK_1 - VK - 0" no longer exist)
		//
		if ( (wParam >= '0' ) && (wParam <= '9' )) {
			blah = wParam - '0';
			if (blah == 0)
				blah = 10;

			myInterface->selectState(blah);

			//If the hight order bit is 1, the key is down.
			if (GetKeyState (VK_SHIFT) < 0)
				GBA_saveState (blah);
			//otherwise, no.
			else 
				GBA_loadState (blah);
		}
		

		break;



		case WM_SYSCOMMAND:	/*User clicks on close x button*/
				if (wParam == SC_CLOSE) {
					//Must put this here as DestroyWindow will automatically destroy
					//all windows, so we can't record whether they were open at exit.
					myInterface->saveSettings();
				}
				return DefWindowProc (hWnd, wMessage, wParam, lParam);
		
		break;			

	//case WM_TIMER:
			//myARM->Graphics_render();				//makes it real slow, I bet
			//myInterface->render();
	//		break;
		
    default:                 // default Windows message processing
    	return DefWindowProc (hWnd, wMessage, wParam, lParam);

    }
	return (0);
}





//This opens a file
void Gui_openFile (HWND hWnd) {
	OPENFILENAME ofn;
	HINSTANCE hInstance;

	//Variables for opening the .X file.
	static  char     szFileTitle[_MAX_FNAME + _MAX_EXT]; /* filename and extension */
    //                 szFile[_MAX_PATH];                  /* filename including path */
    static  char     szFilter[] = "Game Boy Advance Files\0*.bin;*.gba;*.agb;*.elf\0" \
									"BIN Files (*.bin)\0*.bin\0" \
									"GBA Files (*.gba)\0*.gba\0" \
									"AGB Files (*.agb)\0*.agb\0" \
									"ELF Files (*.elf)\n*.elf\0" \
                                  "All Files (*.*)\0*.*\0\0";
	

	hInstance = (HINSTANCE)GetWindowLong (hWnd, GWL_HINSTANCE) ; 

	ZeroMemory(&ofn, sizeof(ofn));
	strcpy(gba.filename,"");

	ofn.hInstance = hInstance;
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = szFilter;
	ofn.lpstrFile = gba.filename;
	ofn.nMaxFile = _MAX_PATH;
	ofn.lpstrFileTitle = szFileTitle;
	ofn.nMaxFileTitle = _MAX_FNAME + _MAX_EXT;
	ofn.lpstrInitialDir = NULL;     /* use current directory */
	ofn.lpstrDefExt = "*.x";        /* default file extension */
                                
	//open a file dialog
	if ( GetOpenFileName(&ofn) ) {  /* fill ofn struct */
		lstrcat(gba.filename,"\0");   /* make sure it's null terminated */
		GBA_loadROM (gba.filename);
		if (arm.debugging)
			Gui_refreshAllDialogs (1);
			
	}
	u32 menuId = GetMenuItemID(appState.hDebugSourceMenu, 1);
	//If stabs is loaded, draw the source menu.
	if (gba.stabs.loaded) {
		StabSourceFile* sourceFile;
		//If the first menu item is NULL, it means the source menu isn't loaded.
		
		if (menuId == IDM_NULL) {
			//Delete the NULL identifier
			DeleteMenu( appState.hDebugSourceMenu, IDM_NULL, MF_BYCOMMAND);
			//Next, add all the source files from the STABS file to the menu.
			sourceFile = gba.stabs.sourceList.next;
			menuId = IDM_SOURCEFILE1;
			while (sourceFile != NULL) {
				AppendMenu (appState.hDebugSourceMenu, MF_STRING, menuId, sourceFile->filename);
				sourceFile = sourceFile->next;
				menuId++;
			}
			DrawMenuBar(appState.hWnd);
 		}
		//SourceGui* sourceGui = SourceGui_getByAddress(&gba.stabs, R15CURRENT);
		//sourceGui = SourceGui_getByFilename(&gba.stabs, filename);
		//		if (sourceGui!= NULL && sourceGui->hWnd != NULL) {
		//			SetFocus(sourceGui->hWnd);
		//		}
		
	} else {
		//Otherwise, make sure that we don't leave useless menus lying around.
		if (menuId != IDM_NULL) {
			//Delete the source menu.
			DeleteMenu( appState.hMainMenu, 5, MF_BYPOSITION);
			appState.hDebugSourceMenu = CreatePopupMenu();
			InsertMenu (appState.hMainMenu, 5, MF_BYPOSITION|MF_POPUP, (UINT)appState.hDebugSourceMenu, "&Source");
			AppendMenu (appState.hDebugSourceMenu, MF_STRING, IDM_AUTOSOURCE, "Auto Focus Source Windows") ;				
			AppendMenu (appState.hDebugSourceMenu, MF_STRING, IDM_NULL, "(Only available for ELF files compiled with -gstabs)") ;
			DrawMenuBar(appState.hWnd);

		}
	}

}

////////////////////////////////////////////////////
//compResize
//This will resize/position a component by the given factor
/////////////////////////////////////////////////////
void Gui_compResize(HWND pHWND, u32 id, s32 xDif, s32 yDif, s32 widthDif, s32 heightDif) {
	RECT rect;
	u32 width, height;
	GetWindowRect(GetDlgItem(pHWND, id), &rect);
	width = rect.right - rect.left + widthDif;
	height = rect.bottom - rect.top + heightDif;
	if (xDif || yDif)
		SetWindowPos( GetDlgItem(pHWND, id), NULL, rect.left + xDif, rect.top + yDif, 
			0, 0, SWP_NOSIZE | SWP_NOZORDER );
	
	if (heightDif || widthDif)
		SetWindowPos( GetDlgItem(pHWND, id), NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER );
			
}

////////////////////////////////////////////////////////////////////
//refreshAllDialogs
//This guy updates ALL of the dialogs currently open.
///////////////////////////////////////////////////////////////////
void Gui_refreshAllDialogs(u32 force) {
	static u32 last[MAX_SOURCEWINDOWS];
	SourceGui* sourceGui;
	

	//if (force || (graphics.framesLeft <= 0)) {
	
		//Update any of the other dialogs if they are opened.
		if (appState.hDlg != NULL)
			DebugGui_refresh();

		if (appState.hStatsDlg != NULL)
			StatsGui_refresh();

		if (appState.hHardwareRegDlg != NULL)
			HardwareRegGui_refresh();

		if (appState.hBreakpointDlg != NULL)
			BreakpointGui_refresh();

		if (appState.hVariablesDlg != NULL)
			VariablesGui_refresh();
		
		if (appState.hSpriteDlg != NULL)
			TileGui_refreshSpriteTiles();

		if (appState.hBackgroundDlg != NULL)
			TileGui_refreshBackgroundTiles();

		if (appState.hPaletteDlg != NULL)
			PaletteGui_refresh();

		if (appState.hObjDlg != NULL)
			ObjGui_refresh();

		if (appState.hMapDlg != NULL)
			MapGui_refresh();

		if (appState.hDisplayDlg != NULL)
			DisplayGui_refresh();

		if (appState.hRegistersDlg != NULL)
			RegistersGui_refresh();

		//The following will load a source gui if we have entered one.
		if (GetMenuState(appState.hDebugSourceMenu, IDM_AUTOSOURCE, MF_BYCOMMAND) & MF_CHECKED)
			sourceGui = SourceGui_getByAddress(&gba.stabs, R15CURRENT);
		
		for (int i = 0; i < MAX_SOURCEWINDOWS; i++) {
			sourceGui = &appState.settings.sourceGuis[i];
			if (sourceGui->hWnd != NULL) {
				//If it's in the file, refresh and set the cursor
				if ((R15CURRENT >= sourceGui->sourceFile->fileStart) 
					&& (R15CURRENT < sourceGui->sourceFile->fileEnd)) 
					SourceGui_refresh(sourceGui, 1);	//refresh + set the cursor.
				else
					SourceGui_refresh(sourceGui, 0);	//refresh + set the cursor.
				last[i] = 0xCB;
			} else {
				if (last[i] == 0xCB) 
					last[i] = 0;
				last[i] = 0;
			}
		}
	//}
}