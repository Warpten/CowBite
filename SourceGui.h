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

#ifndef SOURCEGUI_H
#define SOURCEGUI_H

#include <Windows.h>
#include "Stabs.h"

#define MAX_SOURCEWINDOWS 32

typedef struct tSourceGui {
	HWND hWnd;
	StabSourceFile* sourceFile;
	WINDOWPOS pos;
	u32 showAsm;
	RECT initRect;	//The original rect of this window.
	RECT lbInitRect;	//The original rect of the list box.
} SourceGui;

void SourceGui_refresh(SourceGui* sourceGui, u32 setCursor);
StabSourceFile* SourceGui_loadSource(StabSourceFile* sourceFile);
SourceGui* SourceGui_findByHwnd(HWND hWnd);
SourceGui* SourceGui_getByAddress(Stabs* stabs, u32 address);
SourceGui* SourceGui_getWindow(StabSourceFile* sourceFile);
SourceGui* SourceGui_getByFilename(Stabs *stabs, char* filename);
void SourceGui_stepInto();
void SourceGui_stepOver();
void SourceGui_print(char* text);

#endif