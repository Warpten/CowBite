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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <direct.h>
//#include <memory.h>
#include "WindowsInterface.h"
#include "Gui.h"
#include "Graphics.h"
#include "GBA.h"
#include "IO.h"
#include <commctrl.h>	//For the toolbar

//#include "ARM.cpp"
extern State appState;
extern InputSettings *myInputSettings;

#include "ARM.h"
#include "resource.h"
//#include "ARMExtern.h"


///////////////////////////////////////////////////
//Constructor
///////////////////////////////////////////////////
WindowsInterface::WindowsInterface (HWND nHWND, GBA* nGBA) {
	JOYCAPS joyCaps;
	hWnd = nHWND;	//Get handle to window
	hInstance = (HINSTANCE)GetWindowLong (hWnd, GWL_HINSTANCE) ; 
	appState.windowsInterface = this;

	hDC = GetDC(hWnd);

	bitDepth = GetDeviceCaps (hDC, BITSPIXEL);
	bitPlanes = GetDeviceCaps (hDC, PLANES);
	
	
	if (bitDepth != 16) {
		MessageBox (hWnd,
        "CowBite works best in 16 bit (high color) Mode. \nSwitch to 16 bit and hit \"OK\".  Otherwise you take your chances with the current mode.",
		"CowBite", MB_ICONEXCLAMATION | MB_OK);

		bitDepth = GetDeviceCaps (hDC, BITSPIXEL);
		bitPlanes = GetDeviceCaps (hDC, PLANES);
                
	}
	

	ReleaseDC (hWnd, hDC) ;
	//graphics.bitDepth = bitDepth;
	
	//Initialize Kreed's SuperEagle
	Init_2xSaI(16, 240, 160);
	eagleScreen = new u16 [153600];   //480 x 320
	screen32bit = new u8 [614400];		//480 x 320 x 4
	winScreen = new u16 [38400];		//240 x 160
	
	paletteTable = new u16[0x10000];	//A lookup table for the palette.
									//Making it especially large to account for
									//roms that use the first color bit for no reason (morons...)
	for (u32 i = 0x10000; i--;) {
		paletteTable[i] = ((i & 0x1F) << 11) | ((i & 0x3E0) << 1 ) | ((i & 0x7C00) >> 10);

	}

	//Initialize our GDI drawing surfaces

	
	screen = (u16*)graphics.screen;
	
	hBackground = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_BACKGROUND));
	hRulerLeft = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_RULER1XLEFT));
	hRulerTop = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_RULER1XTOP));
	screenX = 128;
	screenY = 32;
	rulerX = 0;
	rulerY = 40;
	graphics.framesPerRefresh = 1;
	
	
	//Button initialization
	numButtons = 13;
	initButtons();
	
	//Find out the present working directory for use with this session
	_getcwd((char*)pwd, 255);

	//strcpy (saveDir, "");

	//Zero out current settings
	memset(&appState.settings, 0, sizeof(Settings));

	//Establish default settings
	memset(&appState.defaultSettings, 0, sizeof(Settings));
	appState.defaultSettings.screenMode = INTERP_1X;
	appState.defaultSettings.framesPerRefresh = 1;
	strcpy (appState.defaultSettings.debugSettings.lastBreakpointText, "8000000");
	appState.defaultSettings.debugSettings.dumpSetting = IDC_RSTATE;
	appState.defaultSettings.debugSettings.generalStatSettings = 1;
	appState.defaultSettings.debugSettings.memoryStatSettings = 1;
	appState.defaultSettings.debugSettings.interruptStatSettings = 1;
	appState.defaultSettings.debugSettings.soundStatSettings = 1;
	appState.defaultSettings.debugSettings.followCode = BST_CHECKED;

	appState.defaultSettings.selectedState = 1;
	
	//Get the path to the config file.
	strcpy(configPath, gba.path);
	strcat(configPath, "CowBite.cfg");
	
	//Load settings from a file (loads the default settings above if file not found)
	loadSettings();	

	//Get the center joystick position.  Note that it is best to do this
	//AFTER calling loadsettings, since otherwise we will load the previous settings
	//for the joystick (which may be incorrect if the user has a different joystick)
	if (joyGetDevCaps (JOYSTICKID1, &joyCaps, sizeof (joyCaps)) == JOYERR_NOERROR) { 
		appState.settings.inputSettings.centerX = (joyCaps.wXmax + joyCaps.wXmin)/2;
		appState.settings.inputSettings.centerY = (joyCaps.wYmax + joyCaps.wYmin)/2;
		appState.settings.inputSettings.xDivisor = (joyCaps.wXmax - joyCaps.wXmin)/4;
		appState.settings.inputSettings.yDivisor = (joyCaps.wYmax - joyCaps.wYmin)/4;
	}

	//Enter debug mode if needed
	arm.debugging = !arm.debugging;	//Since the nature of the message is to toggle, we must unset
	enterDebug();

	CheckMenuRadioItem (appState.hViewMenu, IDM_REFRESH0, IDM_REFRESH120, 
				appState.settings.debugSettings.dynamicRefresh+IDM_REFRESH0, MF_BYCOMMAND);
	
	//Change screen mode *after* entering debug (so that the needed changes will be applied)
/*	switch (appState.settings.screenMode) {
		case INTERP_1X:	set1X();
			break;
		case INTERP_2X: set2X();
			break;
		case INTERP_3X: set3X();
			break;
		case INTERP_SUPEREAGLE: setSuperEagle();
			break;
		case INTERP_SUPER2XSAI: setSuper2xSaI();
			break;
		default:
			set1X();
			break;
	}*/
	//SendMessage(hWnd, WM_COMMAND, IDM_ENTERDEBUG, 0);
}

////////////////////////////////////////
//Destructor
/////////////////////////////////////
WindowsInterface::~WindowsInterface() {
	Delete_2xSaI();
	delete [] eagleScreen;
	delete [] winScreen;
	delete [] screen32bit;
	delete [] paletteTable;
	DeleteObject (hBackground) ;
	DeleteObject(hRulerLeft);
	DeleteObject(hRulerTop);
	for (int i = 0; i < numButtons; i++) {
		DeleteObject (buttonArray[i].bitmap);
		DeleteObject (buttonArray[i].bitmapR);
		DeleteObject (buttonArray[i].bitmapY);

	}

}

/////////////////////////////////////
//Use this method to toggle debug mode.
/////////////////////////////////////
void WindowsInterface::enterDebug() {
	//If we want to exit debug mode
	if (arm.debugging) {
		//Destroy the diplay window
		if (appState.hDisplayDlg != NULL) {
			DestroyWindow(appState.hDisplayDlg);
			appState.hDisplayDlg = NULL;
		}
		EnableMenuItem( appState.hGraphicsMenu, IDM_3X, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		EnableMenuItem( appState.hGraphicsMenu, IDM_SUPEREAGLE, MF_BYCOMMAND | MF_ENABLED);
		EnableMenuItem( appState.hGraphicsMenu, IDM_SUPER2XSAI, MF_BYCOMMAND | MF_ENABLED);
		CheckMenuItem(appState.hDebugMenu, IDM_ENTERDEBUG, MF_BYCOMMAND | MF_UNCHECKED);
		arm.debugging = 0;
		SetWindowText (appState.hWnd, "CowBite");
		if (appState.hWndToolBar != NULL) {
			DestroyWindow(appState.hWndToolBar);
		}
		SetWindowPos( appState.hWnd, NULL, 0, 0, 
			640 + 2*GetSystemMetrics (SM_CXFIXEDFRAME), 
			480 + 2*GetSystemMetrics (SM_CYFIXEDFRAME) + GetSystemMetrics (SM_CYSIZE) + GetSystemMetrics (SM_CYMENU),
			SWP_NOMOVE | SWP_NOZORDER );
		if (appState.settings.screenMode == INTERP_3X)
			appState.settings.screenMode = INTERP_2X;
		
	//If we are entering debug mode,
	} else {
		screenX = this->rulerX;
		screenY = this->rulerY;
		//Open up the display window
		if (appState.hDisplayDlg == NULL)
			appState.hDisplayDlg = CreateDialogParam (appState.hInstance,MAKEINTRESOURCE(IDD_DISPLAY),
								appState.hWnd,(DLGPROC)DisplayGui_Proc, (LPARAM)&appState) ;

		EnableMenuItem( appState.hGraphicsMenu, IDM_3X, MF_BYCOMMAND | MF_ENABLED);
		EnableMenuItem( appState.hGraphicsMenu, IDM_SUPEREAGLE, MF_BYCOMMAND | MF_DISABLED| MF_GRAYED);
		EnableMenuItem( appState.hGraphicsMenu, IDM_SUPER2XSAI, MF_BYCOMMAND | MF_DISABLED| MF_GRAYED);
		CheckMenuItem(appState.hDebugMenu, IDM_ENTERDEBUG, MF_BYCOMMAND | MF_CHECKED);
		arm.debugging = 1;
		SetWindowText (appState.hWnd, "CowBite (Debug mode)");
		
		HWND hWndToolBar;
		TBADDBITMAP tbab; 
		TBBUTTON tbb[20]; 
		char szBuf[16]; 
		int iCut, iCopy, iPaste; 

		// Create a toolbar that the user can customize and that has a 
		// tooltip associated with it. 
		hWndToolBar = CreateWindowEx(0, TOOLBARCLASSNAME, (LPSTR) NULL, 
		  WS_CHILD | TBSTYLE_TOOLTIPS, // CCS_ADJUSTABLE, 
		  0, 0, 0, 0, hWnd, (HMENU) IDR_TOOLBAR1, appState.hInstance, NULL); 
 
		// Send the TB_BUTTONSTRUCTSIZE message, which is required for 
		// backward compatibility. 
		SendMessage(hWndToolBar, TB_BUTTONSTRUCTSIZE, 
		  (WPARAM) sizeof(TBBUTTON), 0); 

	
		tbab.hInst = appState.hInstance; 
		tbab.nID   = IDB_TOOLBITMAP;//ID_BUTTON40028;//IDB_BUTTONS; 
		SendMessage(hWndToolBar, TB_ADDBITMAP, (WPARAM) 1, (WPARAM) &tbab); 	
			
			

		// Add the button strings to the toolbar. 
		//iCut = SendMessage(hWndToolBar, TB_ADDSTRING, 0, (LPARAM) (LPSTR) ""); 
 
		// buttons to the toolbar. 
		//tbb[0].iBitmap = STD_COPY;//0; 
		
		
		for (int i = 0; i < 19; i++) {
			tbb[i].iBitmap = 0; 
			tbb[i].idCommand = 0;		//For separators only
			tbb[i].fsState = TBSTATE_ENABLED; 
			tbb[i].fsStyle =  TBSTYLE_BUTTON; //TBSTYLE_SEP;
			tbb[i].dwData = 0; 
			tbb[i].iString = NULL; 
		}
		tbb[0].iBitmap = 0;
		tbb[0].idCommand = IDM_OPEN;
		tbb[1].iBitmap = 1;
		tbb[1].idCommand = 0;//IDM_RELOADIDM_OPEN;
		
		tbb[2].fsStyle = TBSTYLE_SEP;
		tbb[3].fsStyle = TBSTYLE_SEP;

		tbb[4].iBitmap = 2;
		tbb[4].idCommand = IDM_PAUSE;
		tbb[5].iBitmap = 3;
		tbb[5].idCommand = IDM_RESET;
		tbb[6].iBitmap = 4;
		tbb[6].idCommand = 0;//IDM_RUNNODEBUG;
		tbb[7].iBitmap = 12;
		tbb[8].idCommand = 0;//IDM_REFRESH
		
		
		tbb[8].fsStyle = TBSTYLE_SEP;//IDM_GO;
		tbb[9].fsStyle = TBSTYLE_SEP;//IDM_GO;

		tbb[10].iBitmap = 5;
		tbb[10].idCommand = 0;//IDM_GO;
		tbb[11].iBitmap = 6;
		tbb[11].idCommand = 0;//IDM_STEPINTO;
		tbb[12].iBitmap = 7;
		tbb[12].idCommand = 0;//IDM_STEPOVER;
		tbb[13].iBitmap = 8;
		tbb[13].idCommand = 0;//IDM_STEPOUT;
		tbb[14].iBitmap = 9;
		tbb[14].idCommand = 0;//IDM_RUNTOCURSOR;
		
		tbb[15].fsStyle = TBSTYLE_SEP;
		tbb[16].fsStyle = TBSTYLE_SEP;

		tbb[17].iBitmap = 10;
		tbb[17].idCommand = 0;//IDM_ASMSTEPINTO
		tbb[18].iBitmap = 11;
		tbb[18].idCommand = 0;//IDM_ASMSTEPOVER





		SendMessage(hWndToolBar, TB_ADDBUTTONS, (WPARAM) 19, 
			  (LPARAM) (LPTBBUTTON) &tbb); 
			
		ShowWindow(hWndToolBar, SW_SHOW);
		appState.hWndToolBar = hWndToolBar; 

		RECT windowRect;
		GetWindowRect(appState.hWnd, &windowRect);
		RECT clientRect;
		GetClientRect(appState.hWnd, &clientRect);
		RECT toolbarRect;
		GetWindowRect(appState.hWndToolBar, &toolbarRect);

		SetWindowPos( appState.hWnd, NULL, 0, 0, 
			(toolbarRect.right - toolbarRect.left) + 2*GetSystemMetrics (SM_CXFIXEDFRAME),
			(toolbarRect.bottom - toolbarRect.top) + 2*GetSystemMetrics (SM_CYFIXEDFRAME) + GetSystemMetrics (SM_CYSIZE) + GetSystemMetrics (SM_CYMENU), 
			SWP_NOMOVE | SWP_NOZORDER );

		if (appState.settings.screenMode == INTERP_SUPEREAGLE)
			appState.settings.screenMode = INTERP_2X;
		if (appState.settings.screenMode == INTERP_SUPER2XSAI)
			appState.settings.screenMode = INTERP_2X;

		this->setPaused(1);	//Pause it whenever we enter debug mode
			
	}
	switch (appState.settings.screenMode) {
		case INTERP_1X:	set1X();
			break;
		case INTERP_2X: set2X();
			break;
		case INTERP_3X: set3X();
			break;
		case INTERP_SUPEREAGLE: setSuperEagle();
			break;
		case INTERP_SUPER2XSAI: setSuper2xSaI();
			break;
		default:
			set1X();
			break;
	}

	

	//redrawBackground();

}

/////////////////////////////////////////////////////////////////////////
//redrawFrame
//This function redraws the parts of the background specified by windows
//////////////////////////////////////////////////////////////////////////
void WindowsInterface::redrawFrame() {
		PAINTSTRUCT ps;
		RECT updateRect;

		//If the display dlg is null, we're in normal mode
		if (appState.hDisplayDlg == NULL) {
			//If there's no update rectangle, break out.
			if (! GetUpdateRect (hWnd, &updateRect, 0))
				return;

			if (appState.settings.screenMode == INTERP_3X)
				return;		
			
			//If it's 3X then our client is taking up the whole screen
			BeginPaint (hWnd, &ps) ;
	  		hDC = ps.hdc;
			hMemDC = CreateCompatibleDC (hDC) ;
		
			//hBackground = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_BACKGROUND));

			SelectObject (hMemDC, hBackground) ;
			//BitBlt (hDC, 0, 0, 320, 240, hMemDC, 0, 0, SRCCOPY) ;
			BitBlt (hDC, 0, 0, 640, 480, hMemDC, 0, 0, SRCCOPY) ;
           
			
		
		//Otherwise, we're in debug mode and must apply our changes to the
		//display dlg
		} 
		
		else {
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
           
			
		}
		
		DeleteDC (hMemDC) ;
    	EndPaint (hWnd, &ps) ;

		//Redraw the image on screen	(will this break it?)
		render(1);
	
}

//////////////////////////////////////////
//redrawBackground()
//This redraws the background only
/////////////////////////////////////
void WindowsInterface::redrawBackground() {
		
		//If it's 3X then our client is taking up the whole screen
		
		
	
		//hBackground = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_BACKGROUND));
		if (appState.hDisplayDlg == NULL) {
			hDC = GetDC (hWnd);
			hMemDC = CreateCompatibleDC (hDC) ;
			SelectObject (hMemDC, hBackground) ;
			//BitBlt (hDC, 0, 0, 320, 240, hMemDC, 0, 0, SRCCOPY) ;
			BitBlt (hDC, 0, 0, 720, 480, hMemDC, 0, 0, SRCCOPY) ;
		} else {
			hDC = GetDC (appState.hDisplayDlg);
			hMemDC = CreateCompatibleDC (hDC) ;
			//Resize the frame to fit the rulers
			SetWindowPos( appState.hDisplayDlg, NULL, 0, 0, 
					rulerWidth + 2*GetSystemMetrics (SM_CXFIXEDFRAME), 
					rulerHeight + 22 + 2*GetSystemMetrics (SM_CYFIXEDFRAME) + GetSystemMetrics (SM_CYSIZE) + GetSystemMetrics (SM_CYMENU),
					SWP_NOMOVE | SWP_NOZORDER );

			

			SelectObject (hMemDC, hRulerLeft) ;
			BitBlt (hDC, rulerX, rulerY+22, 22, rulerHeight, hMemDC, 0, 0, SRCCOPY) ;
			SelectObject (hMemDC, hRulerTop) ;
			BitBlt (hDC, rulerX, rulerY, rulerWidth, 22, hMemDC, 0, 0, SRCCOPY) ;
		}
           
		DeleteDC (hMemDC) ;
    	ReleaseDC (hWnd, hDC);
}


///////////////////////////////////
//render(int force)
//This draws the frame
//Note that "force" forces it to render.
/////////////////////////////////////
void WindowsInterface::render(int force) {
	static long renderTime = 0;
	static long lastRenderTime = 0;
	
	if (!gba.loaded)
		return;
	
	if (appState.settings.framesPerRefresh == 1)
		graphics.framesLeft = 0;
	//if (graphics.framesLeft > 0)
	//	int blah = 0;

	
	if ( (graphics.framesLeft <= 0)   || force) {
		if (appState.hDisplayDlg == NULL)
			hDC = GetDC (hWnd);
		else
			hDC = GetDC (appState.hDisplayDlg);
		hMemDC = CreateCompatibleDC (hDC) ;

		switch (appState.settings.screenMode) {
		case INTERP_SUPEREAGLE:
			convertGBAtoWin ((u32*)winScreen, (u32*)screen, 0);
			//(note -- should figure out how to get supereagle working with
			//gba format)
			SuperEagle((u8*)winScreen, (u8*)eagleScreen);
			if (bitDepth == 32) {
				convert16to32(screen32bit, eagleScreen, 153600);
				hScreen = CreateBitmap (480, 320, bitPlanes, bitDepth, screen32bit);
			} else {
				hScreen = CreateBitmap (480, 320, bitPlanes, bitDepth, eagleScreen);
			}
			//StretchDIBits(hDC,screenX,screenY, 480, 320,0,0,480, 320,eagleScreen,&eagleInfo,DIB_RGB_COLORS,SRCCOPY);
			SelectObject (hMemDC, hScreen) ;
			BitBlt (hDC, screenX, screenY, 480, 320, hMemDC, 0, 0, SRCCOPY);
			break;
		
		case INTERP_SUPER2XSAI:
			convertGBAtoWin ((u32*)winScreen, (u32*)screen,0);
			//(note -- should figure out how to get supereagle working with
			//gba format)
			Super2xSaI((u8*)winScreen, (u8*)eagleScreen);
			if (bitDepth == 32) {
				convert16to32(screen32bit, eagleScreen, 153600);
				hScreen = CreateBitmap (480, 320, bitPlanes, bitDepth, screen32bit);
			} else {
				hScreen = CreateBitmap (480, 320, bitPlanes, bitDepth, eagleScreen);
			}
			//StretchDIBits(hDC,screenX,screenY, 480, 320,0,0,480, 320,eagleScreen,&eagleInfo,DIB_RGB_COLORS,SRCCOPY);
			SelectObject (hMemDC, hScreen) ;
			BitBlt (hDC, screenX, screenY, 480, 320, hMemDC, 0, 0, SRCCOPY);
			break;
	
			
		default:
			if (bitDepth == 32) {
				convertGBAto32Win(screen32bit, screen, 38400);
				hScreen = CreateBitmap (240, 160, bitPlanes, bitDepth , screen32bit);
				
			} else {
				convertGBAtoWin ((u32*)winScreen, (u32*)screen,0);
				hScreen = CreateBitmap (240, 160, bitPlanes, bitDepth , winScreen);
			}
			SelectObject (hMemDC, hScreen) ;
			StretchBlt (hDC, screenX, screenY, screenWidth, screenHeight, hMemDC, 0, 0, 240, 160, SRCCOPY) ;
			//StretchDIBits(hDC,screenX,screenY, screenWidth, screenHeight,0,0,240, 160,screen,&screenInfo,DIB_RGB_COLORS,SRCCOPY);
			break;

		}

		//GdiFlush();	//Force it to update

		//This is the amount of milliseconds that elapsed
		//since the last time this function was called.  This will include the
		//time for the rendering that just ocurred, plus any other overhead
		//that occurs in the meantime.
		//renderTime = clock() - lastRenderTime;
		
		//Calculate our index into the render time queue
		//Note that timeindex must be unsigned, or we're in big trouble!
		
		//timeIndex = (timeIndex+1) & 0xF;

		//renderTimes[timeIndex] = renderTime;
		//renderTimes[0] = renderTime;

		//This takes into account the time it took for the last frame
		//to render, as well.
		//frameSkip = 0;
		//for (int i = 0; i < FRAMESKIP_QUEUE_SIZE; i++) 
		//	frameSkip = frameSkip + renderTimes[i];

		//Assume a queue size of 16
		//frameSkip = frameSkip >> 4;
		
		//Skip a frame for every 1/60th of a second that passed by in
		//the interimm
		//frameSkip = min ((int)(frameSkip/16.67), 15);
		//graphics.frameSkip = min ( (int)(renderTime >> 4), 15);
		
		
	
	    DeleteDC (hMemDC) ;
		ReleaseDC (hWnd, hDC) ;
		DeleteObject (hScreen) ;
		
	} 
	//graphics.frameSkip--;
    framesRendered++;
	//Start timing everything from the last occurance of this function on
	//lastRenderTime = clock();
	//return frameSkip;
}

////////////////////////////////////////////////
//blit()
//this blits a bitmap to the client area
//////////////////////////////////////////////
void WindowsInterface::blit (HBITMAP hBitmap, int x, int y) {
	BITMAP bitmap;

	hDC = GetDC(hWnd);
	hMemDC = CreateCompatibleDC (hDC) ;
	hMemDC2 = CreateCompatibleDC (hDC) ;

	//Draw right onto the background bitmap
	SelectObject (hMemDC, hBackground);
	SelectObject (hMemDC2, hBitmap);
			
	GetObject (hBitmap, sizeof (BITMAP), &bitmap);


	BitBlt (hMemDC, x, y, bitmap.bmWidth, bitmap.bmHeight, hMemDC2, 0, 0, SRCCOPY);

	//Then draw this portion of the background to the scren
	BitBlt (hDC, x, y, bitmap.bmWidth, bitmap.bmHeight, hMemDC, x, y, SRCCOPY);

			
	DeleteDC (hMemDC);
	DeleteDC (hMemDC2);
	ReleaseDC (hWnd, hDC);

}




////////////////////////////////////////////////////////////
//convert16to32
//This converts a 16 bit image to a 32 bit one.
///////////////////////////////////////////////////////////
void WindowsInterface::convert16to32 (u8* dest, u16* source, u32 numEntries) {
	
	while (numEntries--) {
		register color = *source;
		*dest = (color & 0x1F) << 3;	//get b
		dest++;
		*dest = ((color & 0x7E0) >> 3);	//get g
		dest++;
		*dest = ((color & 0xF800) >> 8);	//get R.
		dest+=2;
		source++;
	}
}

////////////////////////////////////////////////////////////
//convertGBAto32Win
//This converts a 16 bit GBA image to a 32 bit windows one.
///////////////////////////////////////////////////////////
void WindowsInterface::convertGBAto32Win (u8* dest, u16* source, u32 numEntries) {
	
	while (numEntries--) {
	//((palEntry & 0x1F) << 11) | ((palEntry & 0x3E0) << 1 ) | ((palEntry & 0x7C00) >> 10)
		register color = *source;
		//*dest = (color & 0x1F) << 3;	//get b
		*dest = ((color & 0x7C00) >> 7);	//get R.
		dest++;
		*dest = ((color & 0x3E0) >> 2);	//get g
		dest++;
		*dest = (color & 0x1F) << 3;	//get b
		dest+=2;
		source++;
	}
}

/////////////////////////////////////////////////
//convertGBAtoWin
//This convers the GBA graphics format to the windows one.
//Uses 32 bit transfers to make it a little quicker.
/////////////////////////////////////////////////
void WindowsInterface::convertGBAtoWin (u32* dest, u32* source, u32 numEntries) {
	
	u32 color32;
	
	if (numEntries) {	//If we specified a number of entries, just do them (must be even though)
		numEntries >>= 1;
		while (numEntries--) {
			color32 = *source++;
			*dest++ = (paletteTable[color32 & 0xFFFF] ) | (paletteTable[color32 >> 16] << 16);
		}

	} else {
		//Otherwise do 240 * 160 entries, 16 at a time
		for (int i = 2400; i--;) {
				color32 = *source++;
				*dest++ = (paletteTable[color32 & 0xFFFF] ) | (paletteTable[color32 >> 16] << 16);
				color32 = *source++;
				*dest++ = (paletteTable[color32 & 0xFFFF] ) | (paletteTable[color32 >> 16] << 16);
				color32 = *source++;
				*dest++ = (paletteTable[color32 & 0xFFFF] ) | (paletteTable[color32 >> 16] << 16);
				color32 = *source++;
				*dest++ = (paletteTable[color32 & 0xFFFF] ) | (paletteTable[color32 >> 16] << 16);
				color32 = *source++;
				*dest++ = (paletteTable[color32 & 0xFFFF] ) | (paletteTable[color32 >> 16] << 16);
				color32 = *source++;
				*dest++ = (paletteTable[color32 & 0xFFFF] ) | (paletteTable[color32 >> 16] << 16);
				color32 = *source++;
				*dest++ = (paletteTable[color32 & 0xFFFF] ) | (paletteTable[color32 >> 16] << 16);
				color32 = *source++;
				*dest++ = (paletteTable[color32 & 0xFFFF] ) | (paletteTable[color32 >> 16] << 16);

				//color32 = *source++;
				//*dest++ = ((color32 & 0x1F) << 11) | ((color32 & 0x3E0) << 1 ) | ((color32 & 0x7C00) >> 10) |
				//	  ((color32 & 0x1F0000) << 11) | ((color32 & 0x3E00000) << 1 ) | ((color32 & 0x7C000000) >> 10);
		}
	}
	
}


//////////////////////////////////////////////
//checkInput()
//This function checks the joystick and keyboard codes, then calls
//the appropriate IO function to set the GBA keyboard reg.
///////////////////////////////////////////////
void WindowsInterface::checkInput() {
	u8 keyState[255];
	JOYCAPS joyCaps;
	JOYINFOEX joyInfo;
	static u32 f11Down;
	//zero out joyInfo

	GetKeyboardState(keyState);

	memset (&joyInfo, 0, sizeof (JOYINFOEX));
	joyInfo.dwSize = sizeof (JOYINFOEX);
	joyInfo.dwFlags = JOY_RETURNBUTTONS | JOY_RETURNX | JOY_RETURNY;
	
	if (joyGetDevCaps (JOYSTICKID1, &joyCaps, sizeof (joyCaps)) == JOYERR_NOERROR) { 
		joyGetPosEx (JOYSTICKID1, &joyInfo);
	}

	//BUTTONS						
	//Check each button to see if it is a keyboard or a joystick buttton,
	//then check to see if it was pressed.  If pressed, set
	//buttonID to the id of the button pressed.  This is kind of clugey,
	//as it only allows the users to use joystick buttons 1 - 6
	if (myInputSettings->buttonAJoystick) {
		if (joyInfo.dwButtons & myInputSettings->buttonA) IO_setA(0);
		else IO_setA(1);
	} else if (keyState[myInputSettings->buttonA] & 0x80) IO_setA(0);	//if the high order bit is up...
		else IO_setA(1);

	if (myInputSettings->buttonBJoystick) {
		if (joyInfo.dwButtons & myInputSettings->buttonB) IO_setB(0);
		else IO_setB(1);
	} else if (keyState[myInputSettings->buttonB] & 0x80) IO_setB(0);	//if the high order bit is up...
		else IO_setB(1);
			
	if (myInputSettings->buttonLJoystick) {
		if (joyInfo.dwButtons & myInputSettings->buttonL) IO_setL(0);
		else IO_setL(1);
	} else if (keyState[myInputSettings->buttonL] & 0x80) IO_setL(0);	//if the high order bit is up...
		else IO_setL(1);
			
	if (myInputSettings->buttonRJoystick) {
		if (joyInfo.dwButtons & myInputSettings->buttonR) IO_setR(0);
		else IO_setR(1);
	} else if (keyState[myInputSettings->buttonR] & 0x80) IO_setR(0);	//if the high order bit is up...
		else IO_setR(1);

	if (myInputSettings->selectJoystick) {
		if (joyInfo.dwButtons & myInputSettings->select) IO_setSelect(0);
		else IO_setSelect(1);
	} else if (keyState[myInputSettings->select] & 0x80) IO_setSelect(0);	//if the high order bit is up...
		else IO_setSelect(1);

	if (myInputSettings->startJoystick) {
		if (joyInfo.dwButtons & myInputSettings->start) IO_setStart(0);
		else IO_setStart(1);
	} else if (keyState[myInputSettings->start] & 0x80) IO_setStart(0);	//if the high order bit is up...
		else IO_setStart(1);



	//DPAD
	//If it's the joypad, calculate the current x and y positions
	if (myInputSettings->dPad == INPUT_JOYSTICK1) { 
		int x = joyInfo.dwXpos;
		int y = joyInfo.dwYpos;
				
		//Check the joystick x axis
		if  (x > myInputSettings->centerX + myInputSettings->xDivisor) IO_setRight(0);
		else IO_setRight(1);
							
				
		if (x < myInputSettings->centerX - myInputSettings->xDivisor) IO_setLeft(0);
		else IO_setLeft(1);
				
		//Check the joystick y axis.
		if (y < myInputSettings->centerY - myInputSettings->yDivisor) IO_setUp(0);
		else IO_setUp(1);

		if (y > myInputSettings->centerY + myInputSettings->yDivisor) IO_setDown(0);
		else IO_setDown(1);
			
	//Otherwise, it's the arrowkeys...
	} else if (myInputSettings->dPad = INPUT_ARROWKEYS) {
		if (keyState[VK_UP] & 0x80) IO_setUp(0);	//if the high order bit is up...
		else IO_setUp(1);

		if (keyState[VK_DOWN] & 0x80) IO_setDown(0);	//if the high order bit is up...
		else IO_setDown(1);

		if (keyState[VK_LEFT] & 0x80) IO_setLeft(0);	//if the high order bit is up...
		else IO_setLeft(1);

		if (keyState[VK_RIGHT] & 0x80) IO_setRight(0);	//if the high order bit is up...
		else IO_setRight(1);
	}

	//Now handle interface key presses, such as "step into"
	/*if ( (GetKeyState(VK_F11) & 0x80) && (!f11Down)) {//(keyState[VK_F11] & 0x80) && (!f11Down)) {
		f11Down = 0;
		SourceGui_stepInto();
	} else {
		f11Down = 0;
	}*/

	//high order bit specifies whether the key is currently down, low order specifies
	//whether it was pressed after the pervious call.
	if ((GetAsyncKeyState(VK_F11) & 0x81) && !(f11Down)) {
		f11Down = 1;
		SourceGui_stepInto();
	} else {
		f11Down = 0;
	} 
	if ((GetAsyncKeyState(VK_F10) & 0x81)) {
		SourceGui_stepOver();
	}

	
}



///////////////////////////////////////////////////
//selectState
//This selects a current state
//Note that state nums are 1 relative, so we must
//make some adjustments.
/////////////////////////////////////////////////
void WindowsInterface::selectState (int stateNum) {
	appState.settings.selectedState = stateNum;

	blit (stateButtonArray[stateNum-1].bitmap, stateButtonArray[stateNum-1].x, stateButtonArray[stateNum-1].y);

	CheckMenuRadioItem (appState.hFileMenu, IDM_SAVE1, IDM_SAVE10, IDM_SAVE1 + stateNum - 1, MF_BYCOMMAND);
	CheckMenuRadioItem (appState.hFileMenu, IDM_LOAD1, IDM_LOAD10, IDM_LOAD1 + stateNum - 1, MF_BYCOMMAND);
}

/////////////////////////////////////////////////////
//saveSettings
//This takes the current appState and writes it to a file.
///////////////////////////////////////////////////////
void WindowsInterface::saveSettings() {
	FILE *file;
	



		//This opens the file, creating it if it
	//doesn't exist.
	if (!(file = fopen(configPath, "wb")))
		return;

	//The following values will be NULL if these windows were closed on exit;
	//otherwise they will be a nonzero value.
	/*if (IsWindowVisible(appState.hConsoleDlg))
		appState.settings.consoleOpen = 1;
	else 
		appState.settings.consoleOpen = 0;*/
	appState.settings.size = sizeof(appState.settings);
	appState.settings.consoleOpen = IsWindowVisible(appState.hConsoleDlg);				
	appState.settings.hDlg = appState.hDlg;				//The debug dialog box.
	appState.settings.hDumpDlg = appState.hDumpDlg;			//The dialog box for dumping/loading
	appState.settings.hBreakpointDlg = appState.hBreakpointDlg;	//What it sounds like.
	appState.settings.hHardwareRegDlg = appState.hHardwareRegDlg;	//The dialog for browsing the virtual hardwre reg info
	appState.settings.hStatsDlg = appState.hStatsDlg;			//The dialog for displaying statistical info
	appState.settings.hSpriteDlg = appState.hSpriteDlg;		//The dialog box for sprites
	appState.settings.hBackgroundDlg = appState.hBackgroundDlg;			//dialog box for the bg
	appState.settings.hInputDlg = appState.hInputDlg;
	appState.settings.hVariablesDlg = appState.hVariablesDlg;
	appState.settings.hPaletteDlg = appState.hPaletteDlg;
	appState.settings.hObjDlg = appState.hObjDlg;
	appState.settings.hMapDlg = appState.hMapDlg;
	appState.settings.hRegistersDlg = appState.hRegistersDlg;
	appState.settings.debugSettings.debugging = arm.debugging;

	//Set all the source guis to NULL.
	memset (&appState.settings.sourceGuis, 0, MAX_SOURCEWINDOWS * sizeof(SourceGui));

	fwrite (&appState.settings, 1, sizeof(Settings), file);		//Save the file
	fclose(file);			
	
}

/////////////////////////////////////////////////////
//loadSettings
//This reads the last settings from a file.
//Blows away everything in the current appstate.
///////////////////////////////////////////////////////
void WindowsInterface::loadSettings() {
	FILE *file;
	
	//First find the full path to the config file
	//char *filename = "CowBite.cfg";
	//strcpy (configPath, pwd);			//Coppy the pwd to the path.
	//strcat(configPath, "\\");
	//strcat(configPath, filename);		//Originally we used the pwd.  Bad.
	
	//This opens the file, using some default settings
	//if it doesn't exist.
	if (!(file = fopen(configPath, "rb"))) {
		memcpy (&appState.settings, &appState.defaultSettings, sizeof(Settings));
	} else {
		fread (&appState.settings, 1, sizeof(Settings), file);		//Save the file
		fclose(file);										//Close the file.
	}
	
	

	arm.debugging = appState.settings.debugSettings.debugging;
	setFrameSkip(appState.settings.framesPerRefresh);


}


///////////////////////////////////////////////
//mouseMove
//This handles mouse movement in the window
////////////////////////////////////////////////
void WindowsInterface::mouseMove (int x, int y) {
	BITMAP bitmap;

	//First make sure we're not at 3X
	if (arm.debugging || (appState.settings.screenMode == INTERP_3X))
		return;

	

	//Loop through each button
	for (int i = 0; i < numButtons; i++) {
		GetObject (buttonArray[i].bitmap, sizeof (BITMAP), &bitmap);
		
		//If the mouse moved into or within the button area . .. 
		if ( (x >= buttonArray[i].x) && ( x < (buttonArray[i].x + bitmap.bmWidth) ) 
			&& ( y >= buttonArray[i].y) && (y < (buttonArray[i].y + bitmap.bmHeight) )) {

			blit (buttonArray[i].bitmapY, buttonArray[i].x, buttonArray[i].y);

			

		//if the mouse moved out of the button area
		} else if ( (lastMouseX >= buttonArray[i].x) && ( lastMouseX < (buttonArray[i].x + bitmap.bmWidth) ) 
			&& ( lastMouseY >= buttonArray[i].y) && (lastMouseY < (buttonArray[i].y + bitmap.bmHeight) )) {

			blit (buttonArray[i].currentBitmap, buttonArray[i].x, buttonArray[i].y);
		}
	}

	lastMouseX = x;
	lastMouseY = y;

}//Mousemove

////////////////////////////////////////////////////
//lButtonDown
//This handles mouse clicks and movement with the button down
///////////////////////////////////////////////////
void WindowsInterface::lButtonDown (int x, int y) {
	BITMAP bitmap;

	//First make sure we're not at 3X
	if (arm.debugging || (appState.settings.screenMode == INTERP_3X))
		return;

	//Loop through each button
	for (int i = 0; i < numButtons; i++) {
		GetObject (buttonArray[i].bitmap, sizeof (BITMAP), &bitmap);

		//If the mouse moved into or within the button area . ..
		if ( (x >= buttonArray[i].x) && ( x < (buttonArray[i].x + bitmap.bmWidth) )
			&& ( y >= buttonArray[i].y) && (y < (buttonArray[i].y + bitmap.bmHeight) )) {

			blit (buttonArray[i].bitmapR, buttonArray[i].x, buttonArray[i].y);
		//	PlaySound((char*)IDR_CLICKWAV, hInstance, SND_RESOURCE | SND_ASYNC);

		//This handles the case where the user moves the mouse out of the area with the button
		//still held down
		} else if ( (lastMouseX >= buttonArray[i].x) && ( lastMouseX < (buttonArray[i].x + bitmap.bmWidth) )
			&& ( lastMouseY >= buttonArray[i].y) && (lastMouseY < (buttonArray[i].y + bitmap.bmHeight) )) {

			blit (buttonArray[i].currentBitmap, buttonArray[i].x, buttonArray[i].y);
		}
	}

	//Go through the state buttons and check them as well
	for (i = 0; i < 10; i++) {
		if ( (x >= stateButtonPositions[i].x) && ( x < (stateButtonPositions[i].x + stateButtonPositions[i].width) )
			&& ( y >= stateButtonPositions[i].y) && (y < (stateButtonPositions[i].y + stateButtonPositions[i].height) )) {
					//Set this to be the current state.
					selectState(i+1);
		}
	}


	lastMouseX = x;
	lastMouseY = y;

}//lButtonDown

/////////////////////////////////////////////////
//lButtonUp
//This handles mouse movement in the window - particularly, if the user
//lifts the mouse button over an item, it means they want to execute it.
////////////////////////////////////////////////////
void WindowsInterface::lButtonUp (int x, int y) {
	BITMAP bitmap;

	//First make sure we're not at 3X
	if (arm.debugging || (appState.settings.screenMode == INTERP_3X))
		return;
	
	//Loop through each button
	for (int i = 0; i < numButtons; i++) {
		GetObject (buttonArray[i].bitmap, sizeof (BITMAP), &bitmap);


		//If the mouse is within the button area . .. 
		if ( (x >= buttonArray[i].x) && ( x < (buttonArray[i].x + bitmap.bmWidth) ) 
			&& ( y >= buttonArray[i].y) && (y < (buttonArray[i].y + bitmap.bmHeight) )) {

			//Don't preform the blit in the case of pause and frameskip -- they do
			//their own blitting.
			if ( ! ((i >= INTERFACE_I) || (i <= INTERFACE_FRAMESKIP)) )
				blit (buttonArray[i].bitmapY, buttonArray[i].x, buttonArray[i].y);
			
			//It's safe to assume we're not using lParam for anything other than mouse clicks
			buttonArray[i].lParam = x;					//loword
			buttonArray[i].lParam |= y <<16;			//hi word

			
			//Now Perform our action
			SendMessage	(hWnd, buttonArray[i].msg, buttonArray[i].wParam, buttonArray[i].lParam);	
	
		
		} 
	}

	lastMouseX = x;
	lastMouseY = y;

}//lButtonup

/////////////////////////////////////////////////
//lostFocus
//updates the buttons accordingly
////////////////////////////////////////////////
void WindowsInterface::lostFocus() {
	for (int i = 0; i < numButtons; i++) {
		blit (buttonArray[i].currentBitmap, buttonArray[i].x, buttonArray[i].y);

	}
}//lostFocus()

//////////////////////////////////////////////////////////
//set1X, set2X, set3X, setSuperEagle, setSuper2XSai -- these set the different
//viewing modes.
/////////////////////////////////////////////////////////////
void WindowsInterface::set1X() { 
	CheckMenuRadioItem (appState.hGraphicsMenu, IDM_1X, IDM_SUPER2XSAI, IDM_1X, MF_BYCOMMAND);
	
	//Free the old ruler bitmaps, load in new ones
	DeleteObject(hRulerLeft);
	DeleteObject(hRulerTop);
	hRulerLeft = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_RULER1XLEFT));
	hRulerTop = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_RULER1XTOP));
	rulerWidth = 262;
	rulerHeight = 182;

	
	
	screenWidth = 240, screenHeight = 160;
	appState.settings.screenMode = INTERP_1X;
	//Redraw the buttons with their states toggled
	for (int i = INTERFACE_I; i <= INTERFACE_2XSAI; i++) {
		buttonArray[i].currentBitmap = buttonArray[i].bitmap;
		buttonArray[INTERFACE_I].currentBitmap = buttonArray[INTERFACE_I].bitmapR;
		blit (buttonArray[i].currentBitmap, buttonArray[i].x, buttonArray[i].y);
	}

	
	if (appState.hDisplayDlg == NULL) {
		screenX = 248;
		screenY = 112;
	} else {
		screenX = rulerX + 22;
		screenY = rulerY + 22;
	}
	redrawBackground();
	myInterface->render(1);	//Force it to update
	
}

//////////////////////////////////////////////////////////
//set1X, set2X, set3X, setSuperEagle, setSuper2XSai -- these set the different
//viewing modes.
/////////////////////////////////////////////////////////////
void WindowsInterface::set2X () {
	CheckMenuRadioItem (appState.hGraphicsMenu, IDM_1X, IDM_SUPER2XSAI, IDM_2X, MF_BYCOMMAND);
	
	//Free the old ruler bitmaps, load in new ones
	DeleteObject(hRulerLeft);
	DeleteObject(hRulerTop);
	hRulerLeft = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_RULER2XLEFT));
	hRulerTop = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_RULER2XTOP));
	rulerWidth = 502;	//Width of ruler along the top
	rulerHeight= 342;	//Height of ruler along the side
	
	//if (appState.settings.screenMode == INTERP_3X)
	screenWidth = 480, screenHeight = 320; 
	appState.settings.screenMode = INTERP_2X;
	//Redraw the buttons with their states toggled
	for (int i = INTERFACE_I; i <= INTERFACE_2XSAI; i++) {
		buttonArray[i].currentBitmap = buttonArray[i].bitmap;
		buttonArray[INTERFACE_II].currentBitmap = buttonArray[INTERFACE_II].bitmapR;
		blit (buttonArray[i].currentBitmap, buttonArray[i].x, buttonArray[i].y);
	}
	if (appState.hDisplayDlg == NULL) {
		screenX = 128;
		screenY = 32;
	} else {
		screenX = rulerX + 22;
		screenY = rulerY + 22;
	}
	redrawBackground();
	
	render(1);

}

//////////////////////////////////////////////////////////
//set1X, set2X, set3X, setSuperEagle, setSuper2XSai -- these set the different
//viewing modes.
//////////////////////////////////////////////////////
void WindowsInterface::set3X () {
	//Free the old ruler bitmaps, load in new ones
	DeleteObject(hRulerLeft);
	DeleteObject(hRulerTop);
	hRulerLeft = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_RULER3XLEFT));
	hRulerTop = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_RULER3XTOP));
	rulerWidth = 742;	//Width of ruler along the top
	rulerHeight= 502;	//Height of ruler along the side
	
	if (appState.hDisplayDlg == NULL) {
		set2X();	//Go to the next highest mode
		return;
	}
	
	
	CheckMenuRadioItem (appState.hGraphicsMenu, IDM_1X, IDM_SUPER2XSAI, IDM_3X, MF_BYCOMMAND);
				
	screenWidth = 720, screenHeight = 480;
	//screenWidth = 640, screenHeight = 480;
	appState.settings.screenMode = INTERP_3X;
	//Redraw the buttons with their states toggled
	for (int i = INTERFACE_I; i <= INTERFACE_2XSAI; i++) {
		buttonArray[i].currentBitmap = buttonArray[i].bitmap;
		blit (buttonArray[i].currentBitmap, buttonArray[i].x, buttonArray[i].y);
	}
	screenX = rulerX + 22;
	screenY = rulerY + 22;
	redrawBackground();
	render(1);
}

//////////////////////////////////////////////////////////
//set1X, set2X, set3X, setSuperEagle, setSuper2XSai -- these set the different
//viewing modes.
/////////////////////////////////////////////////////////////
void WindowsInterface::setSuperEagle () {
	CheckMenuRadioItem (appState.hGraphicsMenu, IDM_1X, IDM_SUPER2XSAI, IDM_SUPEREAGLE, MF_BYCOMMAND);
			
	if (appState.settings.screenMode == INTERP_3X)
		redrawBackground();
	screenWidth = 480, screenHeight = 320;
	appState.settings.screenMode = INTERP_SUPEREAGLE;
	
	//Redraw the buttons with their states toggled
	for (int i = INTERFACE_I; i <= INTERFACE_2XSAI; i++) {
		buttonArray[i].currentBitmap = buttonArray[i].bitmap;
		buttonArray[INTERFACE_EAGLE].currentBitmap = buttonArray[INTERFACE_EAGLE].bitmapR;
		blit (buttonArray[i].currentBitmap, buttonArray[i].x, buttonArray[i].y);
	}
	screenX = 128;
	screenY = 32;
	render(1);
}

//////////////////////////////////////////////////////////
//set1X, set2X, set3X, setSuperEagle, setSuper2XSai -- these set the different
//viewing modes.
/////////////////////////////////////////////////////////////
void WindowsInterface::setSuper2xSaI () {
	CheckMenuRadioItem (appState.hGraphicsMenu, IDM_1X, IDM_SUPER2XSAI, IDM_SUPER2XSAI, MF_BYCOMMAND);
			
	if (appState.settings.screenMode == INTERP_3X)
		redrawBackground();
	screenWidth = 480, screenHeight = 320;
	appState.settings.screenMode = INTERP_SUPER2XSAI;
	
	//Redraw the buttons with their states toggled
	for (int i = INTERFACE_I; i <= INTERFACE_2XSAI; i++) {
		buttonArray[i].currentBitmap = buttonArray[i].bitmap;
		buttonArray[INTERFACE_2XSAI].currentBitmap = buttonArray[INTERFACE_2XSAI].bitmapR;
		blit (buttonArray[i].currentBitmap, buttonArray[i].x, buttonArray[i].y);
	}
		
	screenX = 128;
	screenY = 32;

	myInterface->render(1);	//Force it to update
			
}


////////////////////////////////////////////////////////////////////
//setPaused
//Updates windows and the pause button graphic.
////////////////////////////////////////////////////////////////////
void WindowsInterface::setPaused(int pause) {
	arm.paused = pause;
	if (arm.paused) {
		CheckMenuItem (appState.hFileMenu, IDM_PAUSE, MF_BYCOMMAND | MF_CHECKED);
		buttonArray[INTERFACE_PAUSE].currentBitmap = buttonArray[INTERFACE_PAUSE].bitmapR;
		blit (buttonArray[INTERFACE_PAUSE].currentBitmap, buttonArray[INTERFACE_PAUSE].x, buttonArray[INTERFACE_PAUSE].y);
		Gui_refreshAllDialogs (1);	//If we're paused, good chance we want all the debug
								//dialogs to refresh.
		if (arm.debugging) {
		//Set the pause item in the menu toolbar to pushed
		SendMessage(appState.hWndToolBar, TB_CHECKBUTTON, (WPARAM) IDM_PAUSE, 
			  (LPARAM) MAKELONG(TRUE, 0)); 
		}
	} else {
		CheckMenuItem (appState.hFileMenu, IDM_PAUSE, MF_BYCOMMAND | MF_UNCHECKED);
		buttonArray[INTERFACE_PAUSE].currentBitmap = buttonArray[INTERFACE_PAUSE].bitmap;
		blit (buttonArray[INTERFACE_PAUSE].currentBitmap, buttonArray[INTERFACE_PAUSE].x, buttonArray[INTERFACE_PAUSE].y);
		if (arm.debugging) {
			//Set the pause item in the menu toolbar to pushed
			SendMessage(appState.hWndToolBar, TB_CHECKBUTTON, (WPARAM) IDM_PAUSE, 
				  (LPARAM) MAKELONG(FALSE, 0)); 
		}
	}
	
}

////////////////////////////////////////////////////////////////////
//setFrameSkip
//Updates windows and the pause button graphic.
////////////////////////////////////////////////////////////////////
void WindowsInterface::setFrameSkip(int framesPerRefresh) {
	if (framesPerRefresh < 1)
		framesPerRefresh = 1;

	graphics.framesPerRefresh = appState.settings.framesPerRefresh = framesPerRefresh;
	graphics.framesLeft = framesPerRefresh;
	if (appState.settings.framesPerRefresh > 1) {
		//CheckMenuItem (appState.hFileMenu, IDM_FRAMESKIP, MF_BYCOMMAND | MF_CHECKED);
		buttonArray[INTERFACE_FRAMESKIP].currentBitmap = buttonArray[INTERFACE_FRAMESKIP].bitmapR;
		blit (buttonArray[INTERFACE_FRAMESKIP].currentBitmap, buttonArray[INTERFACE_FRAMESKIP].x, buttonArray[INTERFACE_FRAMESKIP].y);
	} else {
		//CheckMenuItem (appState.hFileMenu, IDM_FRAMESKIP, MF_BYCOMMAND | MF_UNCHECKED);
		buttonArray[INTERFACE_FRAMESKIP].currentBitmap = buttonArray[INTERFACE_FRAMESKIP].bitmap;
		blit (buttonArray[INTERFACE_FRAMESKIP].currentBitmap, buttonArray[INTERFACE_FRAMESKIP].x, buttonArray[INTERFACE_FRAMESKIP].y);
	}
	CheckMenuRadioItem (appState.hGraphicsMenu, IDM_FRAMESKIP0, IDM_FRAMESKIP6, 
												IDM_FRAMESKIP0+framesPerRefresh-1, MF_BYCOMMAND);
}

//This initializes the buttons.
//Fairly tedious to write.
//Could potentially be resolved by loading button positions from a file...
void WindowsInterface::initButtons() {
	buttonArray[INTERFACE_LOAD].x = 23;
	buttonArray[INTERFACE_LOAD].y = 80;
	buttonArray[INTERFACE_LOAD].bitmap = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_LOAD));
	buttonArray[INTERFACE_LOAD].bitmapR = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_LOADR));
	buttonArray[INTERFACE_LOAD].bitmapY = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_LOADY));
	buttonArray[INTERFACE_LOAD].msg = WM_COMMAND;
	buttonArray[INTERFACE_LOAD].wParam = IDM_OPEN;

	buttonArray[INTERFACE_PAUSE].x = 23;
	buttonArray[INTERFACE_PAUSE].y = 119;
	buttonArray[INTERFACE_PAUSE].bitmap = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_PAUSE));
	buttonArray[INTERFACE_PAUSE].bitmapR = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_PAUSER));
	buttonArray[INTERFACE_PAUSE].bitmapY = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_PAUSEY));
	buttonArray[INTERFACE_PAUSE].msg = WM_COMMAND;
	buttonArray[INTERFACE_PAUSE].wParam = IDM_PAUSE;


	buttonArray[INTERFACE_RESET].x = 23;
	buttonArray[INTERFACE_RESET].y = 160;
	buttonArray[INTERFACE_RESET].bitmap = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_RESET));
	buttonArray[INTERFACE_RESET].bitmapR = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_RESETR));
	buttonArray[INTERFACE_RESET].bitmapY = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_RESETY));
	buttonArray[INTERFACE_RESET].msg = WM_COMMAND;
	buttonArray[INTERFACE_RESET].wParam = IDM_RESET;

	buttonArray[INTERFACE_OPTIONS].x = 23;
	buttonArray[INTERFACE_OPTIONS].y = 196;
	buttonArray[INTERFACE_OPTIONS].bitmap = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_OPTIONS));
	buttonArray[INTERFACE_OPTIONS].bitmapR = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_OPTIONSR));
	buttonArray[INTERFACE_OPTIONS].bitmapY = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_OPTIONSY));
	buttonArray[INTERFACE_OPTIONS].msg = WM_RBUTTONDOWN;
	
	buttonArray[INTERFACE_EXIT].x = 23;
	buttonArray[INTERFACE_EXIT].y = 235;
	buttonArray[INTERFACE_EXIT].bitmap = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_EXIT));
	buttonArray[INTERFACE_EXIT].bitmapR = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_EXITR));
	buttonArray[INTERFACE_EXIT].bitmapY = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_EXITY));
	buttonArray[INTERFACE_EXIT].msg = WM_COMMAND;
	buttonArray[INTERFACE_EXIT].wParam = IDM_QUIT;



	buttonArray[INTERFACE_I].x = 231;
	buttonArray[INTERFACE_I].y = 355;
	buttonArray[INTERFACE_I].bitmap = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_I));
	buttonArray[INTERFACE_I].bitmapR = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_IR));
	buttonArray[INTERFACE_I].bitmapY = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_IY));
	buttonArray[INTERFACE_I].msg = WM_COMMAND;
	buttonArray[INTERFACE_I].wParam = IDM_1X;
	

	buttonArray[INTERFACE_II].x = 268;
	buttonArray[INTERFACE_II].y = 355;
	buttonArray[INTERFACE_II].bitmap = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_II));
	buttonArray[INTERFACE_II].bitmapR = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_IIR));
	buttonArray[INTERFACE_II].bitmapY = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_IIY));
	buttonArray[INTERFACE_II].msg = WM_COMMAND;
	buttonArray[INTERFACE_II].wParam = IDM_2X;

	buttonArray[INTERFACE_III].x = 310;
	buttonArray[INTERFACE_III].y = 355;
	buttonArray[INTERFACE_III].bitmap = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_III));
	buttonArray[INTERFACE_III].bitmapR = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_IIIR));
	buttonArray[INTERFACE_III].bitmapY = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_IIIY));
	buttonArray[INTERFACE_III].msg = WM_COMMAND;
	buttonArray[INTERFACE_III].wParam = IDM_3X;


	buttonArray[INTERFACE_EAGLE].x = 355;
	buttonArray[INTERFACE_EAGLE].y = 355;
	buttonArray[INTERFACE_EAGLE].bitmap = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_EAGLE));
	buttonArray[INTERFACE_EAGLE].bitmapR = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_EAGLER));
	buttonArray[INTERFACE_EAGLE].bitmapY = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_EAGLEY));
	buttonArray[INTERFACE_EAGLE].msg = WM_COMMAND;
	buttonArray[INTERFACE_EAGLE].wParam = IDM_SUPEREAGLE;

	buttonArray[INTERFACE_2XSAI].x = 432;
	buttonArray[INTERFACE_2XSAI].y = 355;
	buttonArray[INTERFACE_2XSAI].bitmap = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_2XSAI));
	buttonArray[INTERFACE_2XSAI].bitmapR = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_2XSAIR));
	buttonArray[INTERFACE_2XSAI].bitmapY = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_2XSAIY));
	buttonArray[INTERFACE_2XSAI].msg = WM_COMMAND;
	buttonArray[INTERFACE_2XSAI].wParam = IDM_SUPER2XSAI;

	buttonArray[INTERFACE_FRAMESKIP].x = 310;
	buttonArray[INTERFACE_FRAMESKIP].y = 384;
	buttonArray[INTERFACE_FRAMESKIP].bitmap = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_FRAMESKIP));
	buttonArray[INTERFACE_FRAMESKIP].bitmapR = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_FRAMESKIPR));
	buttonArray[INTERFACE_FRAMESKIP].bitmapY = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_FRAMESKIPY));
	buttonArray[INTERFACE_FRAMESKIP].msg = WM_COMMAND;
	buttonArray[INTERFACE_FRAMESKIP].wParam = IDM_FRAMESKIP;

	buttonArray[INTERFACE_STATESAVE].x = 170;
	buttonArray[INTERFACE_STATESAVE].y = 398;
	buttonArray[INTERFACE_STATESAVE].bitmap = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_STATESAVE));
	buttonArray[INTERFACE_STATESAVE].bitmapR = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_STATESAVER));
	buttonArray[INTERFACE_STATESAVE].bitmapY = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_STATESAVEY));
	buttonArray[INTERFACE_STATESAVE].msg = WM_COMMAND;
	buttonArray[INTERFACE_STATESAVE].wParam = IDM_STATESAVE;

	buttonArray[INTERFACE_STATELOAD].x = 170;
	buttonArray[INTERFACE_STATELOAD].y = 423;
	buttonArray[INTERFACE_STATELOAD].bitmap = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_STATELOAD));
	buttonArray[INTERFACE_STATELOAD].bitmapR = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_STATELOADR));
	buttonArray[INTERFACE_STATELOAD].bitmapY = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_STATELOADY));
	buttonArray[INTERFACE_STATELOAD].msg = WM_COMMAND;
	buttonArray[INTERFACE_STATELOAD].wParam = IDM_STATELOAD;

	for (int i = 0; i < numButtons; i++) {
		buttonArray[i].currentBitmap = buttonArray[i].bitmap;	//This lets us set a current bitmap
	}

	//Initialize all of the state buttons
	//note that we have a seperate array for the positions than the buttons,
	//since I did not make seprate depressed and relief versions of each button
	for (i = 0; i < 10; i++) {
		stateButtonArray[i].x = 23;
		stateButtonArray[i].y = 398;
		
		stateButtonPositions[i].x = 34 + (i%5) * 27;	//These store the position of the mouse
		stateButtonPositions[i].width = 27;		//click
		stateButtonPositions[i].height = 25;
		stateButtonPositions[i].y = 398 + (int)(i/5)* 25;	//Pretty sick, huh?
		
		stateButtonArray[i].bitmap = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_1STATE + i));
	}

}

