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



/////////////////////////////////////////////////////////////////////////////////////////
//DumpGui_Proc
//The windows callback for dumpinfo dialog.
//Presents the user with options for dumping files.
////////////////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK DumpGui_Proc (HWND hDumpDlg, UINT wMessage, 
                                                        WPARAM wParam, LPARAM lParam)
{

		
	 switch (wMessage)       /* process messages */
    {
        case WM_INITDIALOG:
			appState.hDumpDlg = hDumpDlg;
			appState.settings.debugSettings.dumpSetting = IDC_RSTATE;
			CheckRadioButton( hDumpDlg, IDC_RSTATE, IDC_RTHUMBLOCATION, IDC_RSTATE);
			SendDlgItemMessage (hDumpDlg, IDC_DUMPSTART, WM_SETTEXT, (WPARAM)0 , (LPARAM)"8000000");
			SendDlgItemMessage (hDumpDlg, IDC_DUMPEND, WM_SETTEXT, (WPARAM)0 , (LPARAM)"8000000");
			
			SetWindowPos( hDumpDlg, NULL, appState.settings.dumpPos.x,
				appState.settings.dumpPos.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
			ShowWindow(hDumpDlg, SW_SHOW);
			return (TRUE);

		case WM_MOVE:
			{	RECT rect;
				GetWindowRect(hDumpDlg, &rect);
				appState.settings.dumpPos.x = rect.left;//LOWORD(lParam);
				appState.settings.dumpPos.y = rect.top;//HIWORD(lParam);
			}
			break;
        
		
		case WM_SYSCOMMAND:	/*User clicks on close x button*/
			if (wParam == SC_CLOSE) {
				DestroyWindow(hDumpDlg);
				appState.hDumpDlg = NULL;
				return (TRUE) ;
				
			}
			break;			
		case WM_COMMAND:    /* controls in dialog box activated */
            			
			switch (LOWORD(wParam))
			{
				case IDDUMPLOAD:
						DumpGui_load(hDumpDlg);
						Gui_refreshAllDialogs(1);
						break;	
				case IDDUMPDUMP:
						DumpGui_dump(hDumpDlg);
						break;	
				case IDDUMPCANCEL:
						DestroyWindow(hDumpDlg);
						break;

				case IDC_RSTATE:
						SendDlgItemMessage (hDumpDlg, IDDUMPDUMP, WM_SETTEXT, (WPARAM)0 , (LPARAM)"Save");
						EnableWindow( GetDlgItem (hDumpDlg, IDDUMPLOAD), TRUE);
						CheckRadioButton( hDumpDlg, IDC_RSTATE, IDC_RTHUMBLOCATION, IDC_RSTATE);
						appState.settings.debugSettings.dumpSetting = LOWORD(wParam);
						break;
				case IDC_RMEMLOCATION:
						SendDlgItemMessage (hDumpDlg, IDDUMPDUMP, WM_SETTEXT, (WPARAM)0 , (LPARAM)"Dump");
						EnableWindow( GetDlgItem (hDumpDlg, IDDUMPLOAD), TRUE);
						CheckRadioButton( hDumpDlg, IDC_RSTATE, IDC_RTHUMBLOCATION, IDC_RMEMLOCATION);
						appState.settings.debugSettings.dumpSetting = LOWORD(wParam);
						break;
				case IDC_RARMLOCATION:
						SendDlgItemMessage (hDumpDlg, IDDUMPDUMP, WM_SETTEXT, (WPARAM)0 , (LPARAM)"Dump");
						EnableWindow( GetDlgItem (hDumpDlg, IDDUMPLOAD), FALSE);
						//SendDlgItemMessage (hDumpDlg, IDDUMPDUMP, WM_ENABLE, (WPARAM)FALSE , (LPARAM)0);
						CheckRadioButton( hDumpDlg, IDC_RSTATE, IDC_RTHUMBLOCATION, IDC_RARMLOCATION);
						appState.settings.debugSettings.dumpSetting = LOWORD(wParam);
						break;
				case IDC_RTHUMBLOCATION:
						SendDlgItemMessage (hDumpDlg, IDDUMPDUMP, WM_SETTEXT, (WPARAM)0 , (LPARAM)"Dump");
						EnableWindow( GetDlgItem (hDumpDlg, IDDUMPLOAD), FALSE);
						//SendDlgItemMessage (hDumpDlg, IDDUMPDUMP, WM_ENABLE, (WPARAM)FALSE , (LPARAM)0);
						CheckRadioButton( hDumpDlg, IDC_RSTATE, IDC_RTHUMBLOCATION, IDC_RTHUMBLOCATION);
						appState.settings.debugSettings.dumpSetting = LOWORD(wParam);
						break;
				default:
					return (FALSE);	//Otherwise we get a stack overflow.
					break;
            }//ENd of switch (LOWORD(wParam);
			return (TRUE);
			break;

	
		case WM_DESTROY:
				//return (TRUE);
				break;
		
	}//End of switch(message)
	return (FALSE) ;    /* return FALSE to signify message not processed */
}


//////////////////////////////////////////////////////
//load()
//This performs a load.  Operation varies depending on the current dumpSetting.
///////////////////////////////////////////////////////
void DumpGui_load(HWND hWnd) {
	FILE* file;
	char filename[255];
	u32 size, startAddress, endAddress;
	u8* dest;
	
	if (appState.settings.debugSettings.dumpSetting == IDC_RSTATE) {
		DumpGui_getFilename(hWnd, filename, 1);
		GBA_loadState(filename);
		return;
	}

	dest = DumpGui_getMemory(hWnd, &size, &startAddress, &endAddress);
	if (dest == NULL)
		return;

	//Get the file name.
	if (DumpGui_getFilename(hWnd, filename, 1) == NULL)
		return;
	
	//This opens the file, exiting if it doesn't exist.
	if (!(file = fopen(filename, "rb"))) 
		return;

	//Go to beginning of file
	fseek (file, 0, SEEK_SET);
	
	//read into memory
	fread (dest, 1, size, file);		//Load the file into this memory
	fclose(file);			


}//load()

//////////////////////////////////////////////////////
//dump()
//This performs a dump.  Operation varies depending on the current dumpSetting.
///////////////////////////////////////////////////////
void DumpGui_dump(HWND hWnd) {
	FILE* file;
	char filename[255];
	u32 startAddress, endAddress, size;
	u8* source;
	
	if (appState.settings.debugSettings.dumpSetting == IDC_RSTATE) {
		DumpGui_getFilename(hWnd, filename, 0);
		GBA_saveState(filename);
		return;
	}

	source = DumpGui_getMemory(hWnd, &size, &startAddress, &endAddress);
	if (source == NULL)
		return;

	//Get the file name.
	if (DumpGui_getFilename(hWnd, filename, 0) == NULL)
		return;

	//Do something different depending on what mode we're in.
	switch (appState.settings.debugSettings.dumpSetting) {
		case IDC_RMEMLOCATION:	break;
		
		case IDC_RARMLOCATION:	
								appState.armDebugger->outputDisassembler(filename, source, startAddress, endAddress, 0);
								return;
								break;
		case IDC_RTHUMBLOCATION:	appState.armDebugger->outputDisassembler(filename, source, startAddress, endAddress, 1);
									return;
									break;
		default:
							break;
	}

	//This opens the file, exiting if it doesn't exist.
	if (!(file = fopen(filename, "wb"))) 
		return;

	//Go to beginning of file
	fseek (file, 0, SEEK_SET);
	
	//read into file
	fwrite (source, 1, size, file);		//Save the file.
	fclose(file);		

}//dump()

////////////////////////////////////////////////////////
//getMemory
//This gets a pointer to the memory entered in the start and end
//fields in the dump box.  Note that I pass paramters by reference
//so that the caller can use the values retrieved.
////////////////////////////////////////////////////////
u8* DumpGui_getMemory(HWND hWnd, u32* size, u32* start, u32* end) {
	u32  startIndex, endIndex, offset;
	u8* pointer;
	char* nullString = "";
	char text[255];

	//If it wasn't a load state, it must be a load to memory.
	//Get the start and ending points.
	SendDlgItemMessage (hWnd, IDC_DUMPSTART, WM_GETTEXT, (WPARAM)(255) , (LPARAM)text);		
	*start = strtoul(text, &((char*)nullString), 16);
	SendDlgItemMessage (hWnd, IDC_DUMPEND, WM_GETTEXT, (WPARAM)(255) , (LPARAM)text);		
	*end = strtoul(text, &((char*)nullString), 16);

	startIndex = *start >> 24;
	endIndex = *end >> 24;

	if ( (startIndex >= MEMORY_NUMSECTIONS) || (gbaMem.memorySize[startIndex] == 0) ) {
		MessageBox (hWnd,
        "This is an invalid region of memory.",
		"Invalid Range", MB_ICONEXCLAMATION | MB_OK);
		return NULL;
	}

	*size = *end - *start +1;
	offset = *start & 0xFFFFFF;

	//This simple code actually eliminates the need for checking the start and
	//end indexes.
	if (*size > gbaMem.memorySize[startIndex]) {
		MessageBox (hWnd,
        "This operation exceeds the memory size at that location.",
		"Invalid Range", MB_ICONEXCLAMATION | MB_OK);
		return NULL;
	}
	
	pointer = gbaMem.u8Mem[startIndex];
	pointer += offset;
	return pointer;
}

//////////////////////////////////////////////////////
//This gets a filename to open or close, depending on the
//value of "open"
//////////////////////////////////////////////////////
char* DumpGui_getFilename (HWND hWnd, char* filename, u32 open) {
	OPENFILENAME ofn;
	HINSTANCE hInstance;

	//Variables for opening the 
	static  char     szFileTitle[_MAX_FNAME + _MAX_EXT]; /* filename and extension */
    //                 szFile[_MAX_PATH];                  /* filename including path */
    static  char     szFilter[] = "All Files (*.*)\0*.*\0\0";
	

	hInstance = (HINSTANCE)GetWindowLong (hWnd, GWL_HINSTANCE) ; 

	ZeroMemory(&ofn, sizeof(ofn));
	strcpy(filename,"");

	ofn.hInstance = hInstance;
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = szFilter;
	ofn.lpstrFile = filename;
	ofn.nMaxFile = _MAX_PATH;
	ofn.lpstrFileTitle = szFileTitle;
	ofn.nMaxFileTitle = _MAX_FNAME + _MAX_EXT;
	ofn.lpstrInitialDir = NULL;     /* use current directory */
	ofn.lpstrDefExt = "*.bin";        /* default file extension */
                                
	//open a file dialog
	if (open) {
		if ( !GetOpenFileName(&ofn) )   /* fill ofn struct */
			return NULL;
	} else {
		if ( !GetSaveFileName(&ofn) )   /* fill ofn struct */
			return NULL;
	}

	lstrcat(filename,"\0");   /* make sure it's null terminated */
	return filename;
	
}


