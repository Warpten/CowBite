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

#ifndef GUI_H
#define GUI_H

#include <windows.h>
#include <time.h>
#include "ARM.h"
#include "ARMDebugger.h"
#include "WindowsInterface.h"
#include "Stabs.h"
#include "SourceGui.h"

#define INPUT_ARROWKEYS 1
#define INPUT_JOYSTICK1 2
#define INPUT_JOYSTICK2 3
#define INPUT_JOYUP 4
#define INPUT_JOYDOWN 5
#define INPUT_JOYLEFT 6
#define INPUT_JOYRIGHT 7


typedef struct tDebugSettings {
	char lastBreakpointText[255];	//The last breakpoint added through the debugger
	ConditionalBreakpoint lastCondBreakpoint;	//Last conditional breakpoint
	
	u32 autoOpenSource;				//Whether to automatically open source when reached in code
	u32 autoFocusSource;			//Whether to automatically bring source info focus
	
	u32 followCode;				//In the variables viewer, whether to follow with the
									//current source and function
	u32 autoVariables;				//In the variables viewer, whether to automatically
									//pick the most recently accessed variables.

	u32 dynamicRefresh;				//Dynamic refresh menu setting.
	u32 framesPerRefresh;			//The number of frames to wait between debug refreshes.

	u32 dumpSetting;							//The last selected dump radio button.
	u32 generalStatSettings;				//The last selected stat setting
	u32 memoryStatSettings;
	u32 interruptStatSettings;
	u32 soundStatSettings;

	u32 consoleWinDisable;				//Whether to output to console
	u32 consoleStdoutDisable;

	u32 bgNum;								//Currently selected bg in the map viewer
	u32 mapWidth;							//Currently selected map width
	u32 mapHeight;							//Currently selected map width

	u32 debugging;							//Currently debugging or not
	u32 viewBanked;						//WHether to view banked registers in the reg viewer
} DebugSettings;

typedef struct tInputSettings {
	int centerX;
	int centerY;
	int xDivisor;
	int yDivisor;
	u32 dPad;				//use dpad or keyboard
	u32 buttonA;	
	u32 buttonAJoystick;	//If button A is a joystick button
	u32 buttonB;
	u32 buttonBJoystick;	//same here
	u32 buttonL;
	u32 buttonLJoystick;
	u32 buttonR;
	u32 buttonRJoystick;
	u32 select;
	u32 selectJoystick;
	u32 start;
	u32 startJoystick;
} InputSettings;

//A structure just for application settings.
typedef struct tSettings {
	u32 size;	//The size of this structure.
	
	InputSettings inputSettings;
	
	u32 screenMode;
	u32 framesPerRefresh;
	u32 selectedState;
	

	//The following values will be 0 if these windows were closed on exit;
	//otherwise they will be a nonzero value.
	u32 consoleOpen;	//If the console was open.
	HWND hDlg;				//The debug dialog box.
	HWND hDumpDlg;			//The dialog box for dumping/loading
	HWND hBreakpointDlg;	//What it sounds like.
	HWND hHardwareRegDlg;	//The dialog for browsing the virtual hardwre reg info
	HWND hStatsDlg;			//The dialog for displaying statistical info
	HWND hSpriteDlg;		//The dialog box for sprites
	HWND hBackgroundDlg;			//dialog box for the bg
	HWND hInputDlg;
	HWND hVariablesDlg;
	HWND hPaletteDlg;
	HWND hObjDlg;
	HWND hMapDlg;
	HWND hRegistersDlg;

	WINDOWPOS winPos;		//Position of the various windows.
	WINDOWPOS consolePos;
	WINDOWPOS debugPos;
	WINDOWPOS dumpPos;
	WINDOWPOS breakpointPos;
	WINDOWPOS hardwareRegPos;
	WINDOWPOS statsPos;
	WINDOWPOS spritePos;
	WINDOWPOS backgroundPos;
	WINDOWPOS inputPos;
	WINDOWPOS sourcePos;	//Position of the last source gui.
	WINDOWPOS variablesPos;
	WINDOWPOS palettePos;
	WINDOWPOS objPos;
	WINDOWPOS mapPos;
	WINDOWPOS displayPos;
	WINDOWPOS regPos;


	SourceGui sourceGuis[MAX_SOURCEWINDOWS];	//a list of source windows.
	u32 numSourceGuis;

	DebugSettings debugSettings;
	
} Settings;

//Structure for mainting a list of ids used in our custom
//register "list box" in the hardware reg gui dlg
typedef struct tRegIds {
	u32 bits;
	u32 hex;
	u32 bin;
	u32 name;
} RegIds;

typedef struct tState {
	HWND hWnd;				//The main application window handle
	HINSTANCE hInstance;
	HWND hDlg;				//The debug dialog box.
	HWND hDumpDlg;			//The dialog box for dumping/loading
	HWND hBreakpointDlg;	//What it sounds like.
	HWND hHardwareRegDlg;	//The dialog for browsing the virtual hardwre reg info
	HWND hStatsDlg;			//The dialog for displaying statistical info
	HWND hSpriteDlg;		//The dialog box for sprites
	HWND hBackgroundDlg;			//dialog box for the bg
	HWND hInputDlg;
	HWND hConsoleDlg;		//The console window.
	HWND hSourceDlg;
	HWND hVariablesDlg;
	HWND hPaletteDlg;
	HWND hObjDlg;
	HWND hMapDlg;
	HWND hDisplayDlg;
	HWND hWndToolBar;
	HWND hRegistersDlg;

	HWND hMemScroll;		//A scroll bar window
	HWND hDebugScroll;		//A scroll bar window
	HMENU hMainMenu;		//Main menu
	HMENU hPopupMenu;
	HMENU hFileMenu;		//Handle to the file menu
	HMENU hGraphicsMenu;	//Handle to the graphics menu
	
	HMENU hRefreshMenu;
	HMENU hDebugMenu;
	HMENU hDebugPopupMenu;	//Right click menu for the debug window
	HMENU hDebugCPUMenu;
	HMENU hDebugMemoryMenu;
	
	HMENU hDebugDlgMenu;	//The menu in the debug dialog box
	HMENU hDebugSourceMenu;	//A menu listing all the source files in a stab.
	HMENU hSaveStateMenu;
	HMENU hLoadStateMenu;
	HMENU hGraphicsDebugMenu;
	HMENU hViewMenu;
	

	RegIds regIds[8];		//List of resource ids for the hardware registers
	
	
	ARMDebugger* armDebugger;
	WindowsInterface* windowsInterface;

	Settings settings;
	Settings defaultSettings;
	//u32 avoidRegUpdate;
	
	s32 framesUntilDebugRefresh;
	//int paused;					//interface
	//int debugging;
	s32 fps;

} State;						

extern State appState;
extern WindowsInterface* myInterface;
extern InputSettings* myInputSettings;
//GBA* myGBA;
extern ARMDebugger* myDebugger;



/////////////////////////////////////
//This is the WndProc prototype.
//////////////////////////////////
LRESULT CALLBACK Gui_WndProc(HWND, UINT, WPARAM, LPARAM);

//This runs the peekmessage check.
int Gui_peekMessage(MSG *msg);
int Gui_getMessage(MSG* msg);
int Gui_handleMessage(MSG* msg);

//This is the timer callback
VOID CALLBACK Gui_timerProc ( HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime);

void Gui_openFile (HWND hWnd);
void Gui_refreshAllDialogs(u32 force);
void Gui_compResize(HWND pHWND, u32 id, s32 xDif, s32 yDif, s32 widthDif, s32 heightDif);
///////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK DisplayGui_Proc (HWND hDisplayDlg, UINT wMessage, 
                                                        WPARAM wParam, LPARAM lParam);
void DisplayGui_refresh();
void DisplayGui_redrawFrame();
/////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////
//This is the debug dialog box
/////////////////////////////////////
BOOL CALLBACK DebugGui_DebugProc (HWND hDlg, UINT wMessage, WPARAM wParam, LPARAM lParam);
void DebugGui_memScroll(WPARAM wParam, LPARAM lParam);
void DebugGui_debugScroll(WPARAM wParam, LPARAM lParam);

void DebugGui_refresh ();
//This creates a property sheet for the hardware info.
void DebugGui_hardwareInfoSheet(HWND hwnd);
/////////////////////////////////////

///////////////////////////////////////
//The registers dialog box
/////////////////////////////////////////
BOOL CALLBACK RegistersGui_Proc (HWND hRegistersDlg, UINT wMessage, 
                                                        WPARAM wParam, LPARAM lParam);
void RegistersGui_refresh ();
///////////////////////////////////

////////////////////////////////////
//This displays tiles.
///////////////////////////////////////
BOOL CALLBACK TileGui_SpriteTilesProc (HWND hSpriteDlg, UINT wMessage, 
                                                        WPARAM wParam, LPARAM lParam);
BOOL CALLBACK TileGui_BackgroundTilesProc (HWND hBackgroundDlg, UINT wMessage, 
                                                        WPARAM wParam, LPARAM lParam);
void TileGui_refreshSpriteTiles();
void TileGui_refreshBackgroundTiles();
HBITMAP TileGui_createTileBitmapHandle(u8* data, u16* palette, u32 width, u32 height, u32 palType, u32 palNum);

BOOL CALLBACK PaletteGui_Proc (HWND hPaletteDlg, UINT wMessage, 
                                                        WPARAM wParam, LPARAM lParam);
void PaletteGui_refresh();
HBITMAP PaletteGui_createBitmapHandle(u16* palette);


//////////////////////////////////////////
//Obj Gui stuff
///////////////////////////////////////////////////
BOOL CALLBACK ObjGui_Proc (HWND hObjDlg, UINT wMessage, 
                                                        WPARAM wParam, LPARAM lParam);
void ObjGui_refresh();
HBITMAP ObjGui_createObjBitmapHandle(SpriteInfo* obj);
void ObjGui_scroll(WPARAM wParam, LPARAM lParam);
/////////////////////////////////////

//////////////////////////////////////////
//Map Gui stuff
///////////////////////////////////////////////////
BOOL CALLBACK MapGui_Proc (HWND hObjDlg, UINT wMessage, 
                                                        WPARAM wParam, LPARAM lParam);
void MapGui_refresh();
void MapGui_drawScreenOutline (u16* dest, u32 screenX, u32 screenY, u32 winWidth, u32 winHeight);
//HBITMAP ObjGui_createObjBitmapHandle(SpriteInfo* obj);
//void ObjGui_scroll(WPARAM wParam, LPARAM lParam);
/////////////////////////////////////

//////////////////////////////////
//Breakpoitn gui stuff
//////////////////////////////
BOOL CALLBACK BreakpointGui_Proc (HWND hBreakpointDlg, UINT wMessage, 
                                                        WPARAM wParam, LPARAM lParam);
void BreakpointGui_fillValueBox(u32 iD, BreakpointValue* value);
void BreakpointGui_refresh();
int BreakpointGui_getValue(u32 iD, BreakpointValue* value);
////////////////////////////


//////////////////////////////////////////
//This is the hardware info dialog box
/////////////////////////////////////////
BOOL CALLBACK HardwareRegGui_Proc (HWND hDlg, UINT wMessage, WPARAM wParam, LPARAM lParam);
//void HardwareRegGui_update();
void HardwareRegGui_refresh();
void HardwareRegGui_refreshCurrentReg ();
void HardwareRegGui_updateRegField (u32 controlID);
void HardwareRegGui_bitScroll(WPARAM wParam, LPARAM lParam);
/////////////////////////////////////


/////////////////////////////////
//This is the dump dialog box
//////////////////////////////
BOOL CALLBACK DumpGui_Proc (HWND hDlg, UINT wMessage, WPARAM wParam, LPARAM lParam);
char* DumpGui_getFilename (HWND hWnd, char* filename, u32 open);
void DumpGui_load(HWND hWnd);
void DumpGui_dump(HWND hWnd);
u8* DumpGui_getMemory(HWND hWnd, u32* size, u32* start, u32* end);
////////////////////////////

//////////////////////////////
//Refresh the stats gui
BOOL CALLBACK StatsGui_Proc (HWND hStatsDlg, UINT wMessage, 
                                                        WPARAM wParam, LPARAM lParam);
////////////////////////////

void StatsGui_refresh();

//Callback for the input selection gui
BOOL CALLBACK InputGui_Proc (HWND hInputDlg, UINT wMessage, WPARAM wParam, LPARAM lParam);


BOOL CALLBACK InputGui_editProc (HWND hEditWnd, UINT wMessage, 
                                                        WPARAM wParam, LPARAM lParam);

//Cluge to get a resource ID from an HWND.
u32 InputGui_getButtonResourceID(HWND hWnd);

//Get the current button pressed by the user.
InputGui_returnSelectedButton();

//Assigned a button to a control pad function
void InputGui_assignButtonID (u32 resourceID, u32 buttonID, 
					InputSettings *inputSettings, u32 isJoystickButton);

//Get the string correlating to a keyboard button
char* InputGui_getButtonString(u32 virtualCode, u32 isJoystickButton);


BOOL CALLBACK ConsoleGui_Proc (HWND hConsoleDlg, UINT wMessage, 
                                                        WPARAM wParam, LPARAM lParam);

BOOL CALLBACK SourceGui_Proc (HWND hSourceDlg, UINT wMessage, 
                                                        WPARAM wParam, LPARAM lParam);

BOOL CALLBACK VariablesGui_Proc (HWND hVariablesDlg, UINT wMessage, 
                                                        WPARAM wParam, LPARAM lParam);
void VariablesGui_reload();
void VariablesGui_refresh();
void VariablesGui_variableScroll(WPARAM wParam, LPARAM lParam);
char* VariablesGui_getMemoryString(u32 address, char* name);
	
#endif


