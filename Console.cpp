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

#include "Console.h"
#include "Gui.h"
#include "resource.h"
#include <stdio.h>
#include <stdarg.h>
#include <richedit.h>
//#include <varargs.h>

Console console;

//Initialize the console.  To begin with 
//It should hold enough memory for one "page" (1024 bytes);
void Console_init() {
	appState.hConsoleDlg = CreateDialogParam (appState.hInstance,MAKEINTRESOURCE(IDD_CONSOLE),
											appState.hWnd,(DLGPROC)ConsoleGui_Proc, (LPARAM)&appState) ;
	//Set the console text to "" or the copying buffer/concatenating/copying back won't work well
	SendDlgItemMessage (appState.hConsoleDlg, IDC_CONSOLEEDIT, WM_SETTEXT, (WPARAM)0, (LPARAM)"");
	console.bufferSize = CONSOLE_PAGE_SIZE;
	console.buffer = new char[console.bufferSize];
}

void Console_delete() {
	console.bufferSize = 0;
	delete [] console.buffer;
}

/////////////////////////////////////////////////////
//Console_printf
//Broken.
//Does not necessarily work so well.  Usually better to just
//Use an sprintf followed by a Console_print.
//////////////////////////////////////////////////////
void _cdecl  Console_printf(const char * text, ...) {
	if (text != NULL) {
		char* buffer;
		int i;
		int j;
		void* args[8];	//Limit this function to 8 args (to make life easier on Tom)
		
		va_list marker;
		va_start( marker, text );     /* Initialize variable arguments. */
		i = 0;
		for (j = 0; (j < 8) && (i != -1); j++) {
			
			args[j] = va_arg(marker, void*);
			i = (int)args[j];
		}
		va_end(marker);

		printf(text, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7],
					args[8]);
		fflush(stdout);
	
		//Also print to our console window.  Size does not include null terminator.
		//Potential for app breakage is high.
		u32 size = 1 + SendDlgItemMessage (appState.hConsoleDlg, IDC_CONSOLEEDIT, WM_GETTEXTLENGTH, (WPARAM)0, (LPARAM)0);
		buffer = new char[size + strlen(text)];	//ALLOCATE memory (i bet this is butt slow) for the buffer.
		SendDlgItemMessage (appState.hConsoleDlg, IDC_CONSOLEEDIT, WM_GETTEXT, (WPARAM)size, (LPARAM)buffer);
		strcat(buffer, text);
		SendDlgItemMessage (appState.hConsoleDlg, IDC_CONSOLEEDIT, WM_SETTEXT, (WPARAM)0, (LPARAM)buffer);
		delete [] buffer;
	}
}

////////////////////////////////////
//Print text to the console.
///////////////////////////////////
void Console_print(char* text) {
	TEXTRANGE textRange;
	textRange.chrg.cpMin;
	textRange.chrg.cpMax;
	if (text != NULL) {
		if (IsDlgButtonChecked(appState.hConsoleDlg, IDC_STDDISABLE) == BST_UNCHECKED) {
			fprintf(stdout, "%s", text);
			fflush(stdout);
		}

		if (IsDlgButtonChecked(appState.hConsoleDlg, IDC_CONSOLEDISABLE) == BST_UNCHECKED) {
			//Also print to our console window.
			//Potential for app breakage is high.
			u32 size = 1+SendDlgItemMessage (appState.hConsoleDlg, IDC_CONSOLEEDIT, WM_GETTEXTLENGTH, (WPARAM)0, (LPARAM)0);
			u32 length = strlen(text);
			//While the buffer size requested is too small...
			while ((size +length) >= console.bufferSize) {	
				if (console.buffer != NULL) {
					delete [] console.buffer;
					console.buffer = NULL;
				} 
				console.bufferSize += CONSOLE_PAGE_SIZE;
				console.buffer = new char[console.bufferSize];
			}

			
			//buffer = new char[size + strlen(text)];	//ALLOCATE memory (i bet this is butt slow) for the buffer.
			int blah = SendDlgItemMessage (appState.hConsoleDlg, IDC_CONSOLEEDIT, WM_GETTEXT, (WPARAM)size, (LPARAM)console.buffer);
			strcat(console.buffer, text);
			SendDlgItemMessage (appState.hConsoleDlg, IDC_CONSOLEEDIT, WM_SETTEXT, (WPARAM)0, (LPARAM)console.buffer);
		}
	}
}