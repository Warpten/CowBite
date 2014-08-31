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



#include <windows.h>
#include <math.h>
#include <time.h>	//For the clock() function
#include <conio.h>
#include <stdio.h>	//for sprintf
#include <direct.h> //for cwd
#include <string.h>	//For strtock

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
#include <commctrl.h>	//For the toolbar

//GLOBAL VARIABLES (As every computer science student knows,
//global variables are evil incarnate.  Try not to use them
//too much.

State appState;
WindowsInterface* myInterface;
InputSettings* myInputSettings;
//GBA* myGBA;
ARMDebugger* myDebugger;
u32 *hello;	//It looks harmless, but it's a global variable, and therefore EVIL.

///////////////////////////////////////////////
//WinMain()
//The function that creates and initializes a window, then
//runs the main windows loop.  
///////////////////////////////////////////////////////
int PASCAL WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                          LPSTR lpszCmdLine, int nCmdShow)
{
	//If you don't load this in, the registers window won't open
	LoadLibrary(TEXT("riched32.dll"));
	// Ensure that the common control DLL is loaded. 
    InitCommonControls(); 
 

    HWND        hWnd;      // the window's "handle"
    MSG         msg;       // a message structure
    WNDCLASS    wndclass;  // window class structure

    wndclass.style          = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc    = Gui_WndProc;
    wndclass.cbClsExtra     = 0;
    wndclass.cbWndExtra     = 0;
    wndclass.hInstance      = hInstance;
    wndclass.hIcon          = LoadIcon (hInstance, MAKEINTRESOURCE(IDI_CBICON2));//LoadIcon (NULL, IDI_WINLOGO);//
    wndclass.hCursor        = LoadCursor (NULL, IDC_ARROW);
    wndclass.hbrBackground  = (HBRUSH)GetStockObject (WHITE_BRUSH);
    wndclass.lpszMenuName   = NULL; //"Menu";
    wndclass.lpszClassName  = "MyClass";
    // register the window class
    if (!RegisterClass (&wndclass))
        return 0;


	//Create the main window
	appState.hWnd = hWnd = CreateWindow ("MyClass","CowBite", WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 
			640 + 2*GetSystemMetrics (SM_CXFIXEDFRAME), 
			480 + 2*GetSystemMetrics (SM_CYFIXEDFRAME) + GetSystemMetrics (SM_CYSIZE) + GetSystemMetrics (SM_CYMENU),//SystemMetrics (SM_CYFIXEDFRAME) + 
	NULL, NULL, hInstance, NULL);

	

	//Next, parse the command line.  Just get the executable and ROM name.
	char* commandLine = GetCommandLine();
	char* programName = strtok(commandLine, "\" ,\t\n");
	char* arg;
	char* romName = NULL;
//	char text[256];
	char path[_MAX_PATH];	
	int cygwin = 0;
	while ( (arg = strtok(NULL, "\" ,\t\n")) != NULL  ) {
		//printf("Token: %s\n\n", arg);
		////If it's "-cygwin"
		if ( strstr( arg, "-cygwin") != NULL ) {
			cygwin = 1;
			
		} else {	//Otherwise it's a rom name.
			romName = arg;
		}
	}

	//If we're using cygwin, we can expect the commandline to behave in a certain
	//way.
	//if (cygwin) {
		//Find the path to this executable, since that is where the
		//prefs, etc. will be stored.
		
	char* lastSlash = strrchr( programName, '\\' );
	//Ok, if there is a last slash (or any slashes), we know that
	//a full path was specified.
	if (lastSlash != NULL) {
		int i;
		i = lastSlash - programName;
			//For some reason the output varies depending on wehther you start from
			//the console or from windows.  Windows puts a " character in front.
		strcpy (path, (programName));
		path[i+1] = '\0';
	} else {
		//Otherwise set the path to the pwd.
		_getcwd((char*)path, _MAX_PATH);
		strcat(path,"\\");
	}

//	for (int i = 0; i < 4; i++) {
//		i = i;
//	}
	
	//This is where the GBA gets initialized!
	GBA_init(path);
	//This is where the GBA debugger (for hardware info) gets initialized
	GBADebugger_init();
	
	//The ARM debugger is concerned more with CPU and memory than with the rest of the
	//GBA hardware
	myDebugger = new ARMDebugger(&gba);
	myInterface = new WindowsInterface (hWnd, &gba);
	
	//After creating the windows interface and getting our settings, create 
	//the console window, but keep it hidden.  The console window
	//needs to always remain in memory so that ALL debug messages can be printed to it.
	
	Console_init();
	
	//appState.gba = myGBA;
	appState.armDebugger = myDebugger;

	//Creating the interface also loads all the previous settings (if any)
	//or default settings (if no previous settings).
	appState.windowsInterface = (WindowsInterface*)myInterface;
	

	//Note that myARM must already be instantiated for this to work
	

	//Initalize the state variables
	appState.hWnd = hWnd;
	myInputSettings = &appState.settings.inputSettings;

    ShowWindow (hWnd, nCmdShow);   // display the window
    UpdateWindow (hWnd);           // update window's client area

	///////////////////////////////////////////////////
	//Show any of the main windows that were open on exit.
	///////////////////////////////////////////////////
	if (appState.settings.consoleOpen)
		ShowWindow(appState.hConsoleDlg, SW_SHOW);
	if (appState.settings.hDlg != NULL) {
		appState.hDlg = CreateDialogParam (appState.hInstance,MAKEINTRESOURCE(IDD_DEBUG),
											appState.hWnd,(DLGPROC)DebugGui_DebugProc, (LPARAM)&appState) ;
	}
	if (appState.settings.hVariablesDlg != NULL)
		appState.hVariablesDlg = CreateDialogParam (appState.hInstance,MAKEINTRESOURCE(IDD_VARIABLES),
											appState.hWnd,(DLGPROC)VariablesGui_Proc, (LPARAM)&appState) ;
	if (appState.settings.hHardwareRegDlg != NULL)
		appState.hHardwareRegDlg = CreateDialogParam (appState.hInstance,MAKEINTRESOURCE(IDD_HARDWAREREGS),
										appState.hWnd,(DLGPROC)HardwareRegGui_Proc, (LPARAM)&appState) ;
	if (appState.settings.hStatsDlg != NULL)
		appState.hStatsDlg = CreateDialogParam (appState.hInstance,MAKEINTRESOURCE(IDD_STATISTICS),
								appState.hWnd,(DLGPROC)StatsGui_Proc, (LPARAM)&appState) ;
	if (appState.settings.hBreakpointDlg != NULL)
		appState.hBreakpointDlg = CreateDialogParam (appState.hInstance,MAKEINTRESOURCE(IDD_BREAKPOINTS),
										appState.hWnd,(DLGPROC)BreakpointGui_Proc, (LPARAM)&appState) ;
	if (appState.settings.hObjDlg != NULL)
		appState.hObjDlg = CreateDialogParam (appState.hInstance,MAKEINTRESOURCE(IDD_OBJS),
										appState.hWnd,(DLGPROC)ObjGui_Proc, (LPARAM)&appState) ;
	if (appState.settings.hPaletteDlg != NULL)
		appState.hPaletteDlg = CreateDialogParam (appState.hInstance,MAKEINTRESOURCE(IDD_PALETTE),
										appState.hWnd,(DLGPROC)PaletteGui_Proc, (LPARAM)&appState) ;
	if (appState.settings.hMapDlg != NULL)
		appState.hMapDlg = CreateDialogParam (appState.hInstance,MAKEINTRESOURCE(IDD_BGMAPS),
										appState.hWnd,(DLGPROC)MapGui_Proc, (LPARAM)&appState) ;
	if (appState.settings.hRegistersDlg != NULL)
		appState.hRegistersDlg = CreateDialogParam (appState.hInstance,MAKEINTRESOURCE(IDD_REGISTERS),
											appState.hWnd,(DLGPROC)RegistersGui_Proc, (LPARAM)&appState) ;
			
			

	///////////////////////////////////////////////////						

	hello = (u32*)&(appState.settings.sourceGuis[0].hWnd);

	//Set a timer to go off once every three seconds to report the FPS
	while (!SetTimer (hWnd, 1, 1000, (TIMERPROC)Gui_timerProc))

          if (IDCANCEL == MessageBox (hWnd,

              "Too many clocks or timers!", "CowBite",

               MB_ICONEXCLAMATION | MB_RETRYCANCEL))

                    return FALSE ;   /* exit program */
	

	if ( romName != NULL) {
		GBA_loadROM (romName);
		
		if (arm.debugging)
			Gui_refreshAllDialogs (1);
	}
	//char text[255];

	//FindExecutable( programName, "C:\\GBA\\", text);
	//Console_printf("\n\npath: %s", path);
	//fflush(stdout);

	

	//freopen( "freopen.out", "w", stdout );
	//fprintf( stdout, "error on freopen\ntest.test.\ntest." );
	//fflush(stdout);
	//fprintf( stdout, "error on freopen\ntest.test.\ntest." );
	//fflush(stdout);
   
/*
   char pathbuffer[_MAX_PATH];
   char searchfile[] = "CL.EXE";
   char envvar[] = "PATH";

    //Search for file in PATH environment variable: 
   _searchenv( searchfile, envvar, pathbuffer );
   if( *pathbuffer != '\0' )
      printf( "Path for %s: %s\n", searchfile, pathbuffer );
   else
      printf( "%s not found\n", searchfile );

*/

	//LARGE_INTEGER largeInt;
	//QueryPerformanceFrequency(&largeInt);
	//u32 refreshTime = largeInt.LowPart / 60;
	//QueryPerformanceCounter(&largeInt);
	
	//static long renderTime = 0;
	//static long lastRenderTime = 0;
	//u32 currentTime;
	//u32 lastTime;
	while (1)                        /* peek message loop */
    {
		//First, continusously loop, checking to see if the
		//user has loaded a rom.
		while (! gba.loaded || arm.paused) {
			//If the user hits a key, send a keystroke message to everyone interested.
			myInterface->checkInput();	//Especially useful for step over/into.

			if (!Gui_getMessage(&msg))	//This will allow the user's cpu to idle.
				return msg.wParam;
		}

		//Once they have, enter a second, more optimal loop where we check
		//to see if the game has been paused.
		QueryPerformanceCounter(&gba.lastTime64);	//Start timer count
		while ( 1) {
			
																		
			if (appState.settings.debugSettings.framesPerRefresh) {
				if (--appState.framesUntilDebugRefresh <= 0) {
					appState.framesUntilDebugRefresh = appState.settings.debugSettings.framesPerRefresh;
					Gui_refreshAllDialogs(0);	//Frameskip can ovveride dynamic refresh.
				}
			}

			//checkInput must go first to "initialize" the button settings.
			myInterface->checkInput();

			//Notice how we recieve windows messages *before* running the frame.
			if (!Gui_peekMessage(&msg))	//Use peek to crank that CPU
				return msg.wParam;
			
			//Check to see if we're in debug mode or normal mode
			if (!arm.paused) {//arm.debugging) {
				//gba.stats.lineTickTally = gba.stats.frameTickTally % 1232;	//Find out where we are
				
				
				GBA_debugVDraw();
				myInterface->render(0);
				GBA_debugVBlank();
				
				//renderTime = clock() - lastRenderTime;
				//Skip a frame for every 1/60th of a second that passed by in
				//the interimm
				//frameSkip = min ((int)(frameSkip/16.67), 15);
				//graphics.frameSkip = min ( (int)(renderTime >> 4), 15);
				//graphics.frameSkip--;
				//lastRenderTime = clock();
				
			} else {
				Audio_stopSound();
				appState.windowsInterface->setPaused(1);	//Tell the app we're paused
				break;	//Break out of the loop if paused
			}
			


			/*else {
				GBA_runVDraw();
				myInterface->render(0);
				GBA_runVBlank();
			}*/
			

			
			
			
		}//while(1)
		
    }//while(1)
	return msg.wParam ;
}


////////////////////////////////////////////////
//Gui_peekMessage()
//Quickly checks to see if there is a message in the queue, executes it if so.
//returns 0 on quit
///////////////////////////////////////////////
int Gui_peekMessage(MSG* msg) {
//	char text1[256];
//	char text2[32];
   /* DWORD dwEvt = MsgWaitForMultipleObjects(
            audio.numEvents,      // How many possible events
            audio.cursorEvents,       // Location of handles
            FALSE,          // Wait for all?
            0, //INFINITE,       // How long to wait
            QS_ALLINPUT);   // Any message is an event
 
    // WAIT_OBJECT_0 == 0 but is properly treated as an arbitrary
    // index value assigned to the first event, therefore we subtract

    // it from dwEvt to get the zero-based index of the event.
 
    dwEvt -= WAIT_OBJECT_0;
 
    // If the event was set by the buffer, there's input
    // to process. 
	//strcpy(text1, "");
    if (dwEvt < audio.numEvents) {
		//Audio_callback();
			//sprintf(text1, "pos: %d", dwEvt);
			//strcat(text1, text);
			//SetWindowText (appState.hWnd, text1);
        //StreamToBuffer(dwEvt); // copy data to output stream
    } else if (dwEvt == audio.numEvents) {
    	
*/

		while (PeekMessage (msg, NULL, 0, 0, PM_REMOVE)) {
			//Audio_stopSound();
			if ( !Gui_handleMessage(msg) )
				return 0;
		}
		return 1;
/*
		if (PeekMessage (msg, NULL, 0, 0, PM_REMOVE)) {
			return Gui_handleMessage(msg);
		}
		return 1;*/
	//}
}

////////////////////////////////////////////////
//Gui_getMessage()
//Gets the next message.  Allows the cpu to idle.
//returns 0 on quit
///////////////////////////////////////////////
int Gui_getMessage(MSG* msg) {
	

	if (GetMessage (msg, NULL, 0, 0)) {
        return Gui_handleMessage(msg);
	} else return 0;
}

///////////////////////////////////////
//Handle messages.
int Gui_handleMessage(MSG* msg) {
	if (msg->message == WM_QUIT) { /* if WM_QUIT, quit! */
			GBA_delete();
			//delete myARM;	//Arm now gets deleted in GBA_delete();
			delete myInterface;
			return 0;
	}
    else                        /* else, process message */
    {       
		TranslateMessage (msg) ;
		DispatchMessage (msg) ;
		return 1;
	}

}


//////////////////////////////////////////////////////////////////
//Gui_timerProc
//I'm using it to count the fps.
////////////////////////////////////////////////////////////////
VOID CALLBACK Gui_timerProc ( HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime) {
	char  title[256];
	appState.fps = myInterface->framesRendered;///3;
	myInterface->framesRendered = 0;
	char* debugMode;
	if (arm.debugging)
		debugMode = "(Debug mode) ";
	else
		debugMode = "";

	if (gba.loaded)
		sprintf (title, "CowBite %s(%s), %d fps", debugMode, gba.filename, appState.fps);
	else
		sprintf (title, "CowBite %s", debugMode);
	SetWindowText (appState.hWnd, title);
}//timerProc


