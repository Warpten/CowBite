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

#ifndef WINDOWSINTERFACE_H
#define WINDOWSINTERFACE_H

#include "GBA.h"
#include "ARM.h"
#include "Windows.h"
//#include "Gui.h"
#include "2xsai.h"

#define INTERFACE_LOAD 0
#define INTERFACE_RESET 1
#define INTERFACE_OPTIONS 2
#define INTERFACE_EXIT 3
#define INTERFACE_I 4
#define INTERFACE_II 5
#define INTERFACE_III 6
#define INTERFACE_EAGLE 7
#define INTERFACE_2XSAI 8
#define INTERFACE_PAUSE 9
#define INTERFACE_FRAMESKIP 10
#define INTERFACE_STATESAVE 11
#define INTERFACE_STATELOAD 12

#define INTERP_1X 1
#define INTERP_2X 2
#define INTERP_3X 3

#define INTERP_SUPEREAGLE 4
#define INTERP_SUPER2XSAI 5

#define FRAMESKIP_QUEUE_SIZE 16

typedef struct tCBButton {
	int x;
	int y;
	int width;
	int height;
	HBITMAP bitmap;
	HBITMAP bitmapY;
	HBITMAP bitmapR;
	HBITMAP currentBitmap;
	UINT msg;	//Windows message to send on click
	WPARAM wParam;	//Wparam to go with message
	LPARAM lParam;	//lParam
} CBButton;

///////////////////////////////////////
//WindowsInteface
//In theory, this class is a kind of go-between for the
//windows loop and CowBite's internal data
//////////////////////////////////////////
class WindowsInterface {

	public:
		WindowsInterface (HWND nHWND, GBA* nGBA);
		~WindowsInterface();
		void enterDebug(); 
		void checkInput();
		void initButtons();
		
		void render(int force);
		void redrawBackground();
		void redrawFrame();
		void blit(HBITMAP hBitmap, int x, int y);
		void selectState (int stateNum);
		void saveSettings();
		void loadSettings();

		void mouseMove(int x, int y);
		void lButtonDown (int x, int y);
		void lButtonUp (int x, int y);
		void lostFocus();
		void setPaused (int pause);
		void setFrameSkip (int useFrameSkip);
		void convert16to32 (u8* dest, u16* source, u32 numEntries);
		void convertGBAto32Win (u8* dest, u16* source, u32 numEntries);
		void convertGBAtoWin (u32* dest, u32* source, u32 numEntires);
			
		void set1X();

		void set2X ();
	
		//Note that 3X is not really 3X -- it's 640 x 480
		void set3X ();

		void setSuperEagle ();

		void setSuper2xSaI ();

		//public data
		//int interpolation;
		//int useFrameSkip;
		//int selectedState;

		//char fileName[_MAX_PATH];	//The filename
		char pwd[_MAX_PATH];		//The present working directory
		char configPath[_MAX_PATH];	//The path to the config file.
	

		//int frameSkip;
		int framesRendered;
		//char saveDir[256];

		int bitDepth;
		int bitPlanes;

		
	private:
		HWND hWnd;
		HDC hDC;
		HDC hMemDC;
		HDC hMemDC2;
		
		HINSTANCE hInstance;
		HBITMAP hBackground;
		HBITMAP hScreen;
		HBITMAP hEagleScreen;
		HBITMAP hRulerLeft;
		HBITMAP hRulerTop;
		
		BITMAPINFO screenInfo;
		BITMAPINFO eagleInfo;

	
		
		int screenWidth;
		int screenHeight;
		int screenX;
		int screenY;
		int rulerX;
		int rulerY;
		int rulerWidth;
		int rulerHeight;
		int useSuperEagle;
		int useSuper2XSAI;

		int lastMouseX;
		int lastMouseY;
		long renderTimes[FRAMESKIP_QUEUE_SIZE];	//Calculate rendering times
												//based previous values
		u32 timeIndex;

		u16* eagleScreen;
		u16* screen;
		u16* winScreen;
		u16* paletteTable;
		u8* screen32bit;

		CBButton buttonArray[64];
		CBButton stateButtonArray[10];
		CBButton stateButtonPositions[10];
		int numButtons;

		
};


#endif