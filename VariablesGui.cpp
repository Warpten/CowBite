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
#include "GBADebugger.h"

//extern State appState;
#define VARIABLELIST_NUMROWS 8


/////////////////////////////////////////////////////////////////////////////////////////
//VariablesGui_Proc
//The windows callback for the variables gui.
////////////////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK VariablesGui_Proc (HWND hVariablesDlg, UINT wMessage, 
                                                        WPARAM wParam, LPARAM lParam)
{
	char *nullString = "";
		
	 switch (wMessage)       /* process messages */
    {
        case WM_INITDIALOG:
			appState.hVariablesDlg = hVariablesDlg;
					
			//Determine the type of value
			SendDlgItemMessage (hVariablesDlg, IDC_SYMBOLTYPE, CB_ADDSTRING, (WPARAM)0, (LPARAM)"Local Variables");
			SendDlgItemMessage (hVariablesDlg, IDC_SYMBOLTYPE, CB_ADDSTRING, (WPARAM)0, (LPARAM)"Static Variables");
			SendDlgItemMessage (hVariablesDlg, IDC_SYMBOLTYPE, CB_ADDSTRING, (WPARAM)0, (LPARAM)"Global Variables");
			SendDlgItemMessage (hVariablesDlg, IDC_SYMBOLTYPE, CB_ADDSTRING, (WPARAM)0, (LPARAM)"Specify Name:");
			SendDlgItemMessage (hVariablesDlg, IDC_SYMBOLTYPE, CB_ADDSTRING, (WPARAM)0, (LPARAM)"Specify Address:");
			SendDlgItemMessage (hVariablesDlg, IDC_SYMBOLTYPE, CB_ADDSTRING, (WPARAM)0, (LPARAM)"Files");
			SendDlgItemMessage (hVariablesDlg, IDC_SYMBOLTYPE, CB_ADDSTRING, (WPARAM)0, (LPARAM)"Functions");
			SendDlgItemMessage (hVariablesDlg, IDC_SYMBOLTYPE, CB_ADDSTRING, (WPARAM)0, (LPARAM)"Elf Sections");
			SendDlgItemMessage (hVariablesDlg, IDC_SYMBOLTYPE, CB_ADDSTRING, (WPARAM)0, (LPARAM)"Elf Program Entries");
			SendDlgItemMessage (hVariablesDlg, IDC_SYMBOLTYPE, CB_ADDSTRING, (WPARAM)0, (LPARAM)"Elf Data Symbols");
			SendDlgItemMessage (hVariablesDlg, IDC_SYMBOLTYPE, CB_ADDSTRING, (WPARAM)0, (LPARAM)"All Elf Symbols");
			SendDlgItemMessage (hVariablesDlg, IDC_SYMBOLTYPE, CB_ADDSTRING, (WPARAM)0, (LPARAM)"Stabs Source Lines");

								
			//appState.settings.variablesPos.x = appState.settings.variablesPos.y = 0;
			//SendDlgItemMessage (appState.hVariablesDlg, IDC_SYMBOLTYPE, CB_SELECTSTRING , 
			//	(WPARAM)0, (LPARAM)"Local Variables");

			//Set the "follow" to be checked by default (it's just easier that way).
			
			SendDlgItemMessage (appState.hVariablesDlg, IDC_VARFOLLOW, BM_SETCHECK, (WPARAM)appState.settings.debugSettings.followCode, (LPARAM)0 );
			VariablesGui_reload();

			SetWindowPos( hVariablesDlg, NULL, appState.settings.variablesPos.x,
						appState.settings.variablesPos.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
			ShowWindow(hVariablesDlg, SW_SHOW);

			return (TRUE);

		case WM_MOVE:
			{	RECT rect;
				GetWindowRect(hVariablesDlg, &rect);
				appState.settings.variablesPos.x = LOWORD(lParam);
				appState.settings.variablesPos.y = HIWORD(lParam);
			}
			break;

		case WM_SYSCOMMAND:	/*User clicks on close x button*/
			if (wParam == SC_CLOSE) {
				DestroyWindow(hVariablesDlg);
				return (TRUE) ;
				
			}
			break;			

        case WM_COMMAND:    /* controls in dialog box activated */
            			
			switch (LOWORD(wParam))
				{
				case ID_VARIABLESCLOSE:
						DestroyWindow(hVariablesDlg);
						break;
				
				case IDC_SYMBOLADDRESS:
					if (HIWORD(wParam) == EN_CHANGE)
						VariablesGui_refresh();
					break;
				case IDC_VARFOLLOW:
						appState.settings.debugSettings.followCode = SendDlgItemMessage (appState.hVariablesDlg, IDC_VARFOLLOW, BM_GETCHECK, (WPARAM)0, (LPARAM)0 );
						VariablesGui_refresh();
					//}
					break;
				case IDC_VARIABLESFILENAME:
				case IDC_FUNCTION:
				case IDC_SYMBOLTYPE:
					if (HIWORD(wParam) == CBN_SELENDOK) {
						VariablesGui_refresh();
						//Display a different list of symbols
					}

					break;
				case ID_VARIABLESREFRESH:
					VariablesGui_refresh();
					break;
				default:
					return (FALSE);	//Otherwise we get a stack overflow.
					break;
            }//ENd of switch (LOWORD(wParam);
			return (TRUE);
			break;
		
		///////////////////////////////////////////////////////
		//The user clicked on the scroll bar for the memory
		//////////////////////////////////////////////////////
		case WM_VSCROLL:    /* list box was activated */
			
			//Find out if the message came from the reg edit box or the
			//bits rich edit
			if (GetDlgCtrlID((HWND)lParam) == IDC_VARIABLESCROLL) {
				VariablesGui_variableScroll(wParam, lParam);
			}
			//Destroy the window, once and for all
			return (TRUE);
			break;// End of case (WM_HSCROLL)
	
		case WM_DESTROY:
				appState.hVariablesDlg = NULL;
				//return (TRUE);
				break;
		
	}//End of switch(message)
	return (FALSE) ;    /* return FALSE to signify message not processed */
}

////////////////////////////////
//Call this function to tell the variables GUI
//to reload (i.e. a new elf file was loaded
//and we need to update.)
////////////////////////////////
void VariablesGui_reload() {
	StabFunction* stabFunction;
	StabSourceFile* stabSourceFile;
	
	if (appState.hVariablesDlg != NULL) {
		//Clear out the combo boxes
		SendDlgItemMessage (appState.hVariablesDlg, IDC_FUNCTION, CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
		SendDlgItemMessage (appState.hVariablesDlg, IDC_VARIABLESFILENAME, CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
		//Set the first item to be "No current function"
		SendDlgItemMessage (appState.hVariablesDlg, IDC_FUNCTION, CB_ADDSTRING, (WPARAM)0, (LPARAM)"None");
		SendDlgItemMessage (appState.hVariablesDlg, IDC_FUNCTION, CB_ADDSTRING, (WPARAM)0, (LPARAM)"All");
		SendDlgItemMessage (appState.hVariablesDlg, IDC_VARIABLESFILENAME, CB_ADDSTRING, (WPARAM)0, (LPARAM)"None");
		SendDlgItemMessage (appState.hVariablesDlg, IDC_VARIABLESFILENAME, CB_ADDSTRING, (WPARAM)0, (LPARAM)"All");
		//If stabs was loaded, fill up one combobox with functions and the other with filenames
		if (gba.stabs.loaded) {
		
			stabFunction = gba.stabs.functionList.next;
			while (stabFunction != NULL) {
				SendDlgItemMessage (appState.hVariablesDlg, IDC_FUNCTION, CB_ADDSTRING, 
					(WPARAM)0, (LPARAM)Stabs_demangle(stabFunction->name));
				//Add this item to the menu.
				stabFunction = stabFunction->next;
			}
		
			stabSourceFile = gba.stabs.sourceList.next;
			while (stabSourceFile != NULL) {
				SendDlgItemMessage (appState.hVariablesDlg, IDC_VARIABLESFILENAME, CB_ADDSTRING, 
					(WPARAM)0, (LPARAM)stabSourceFile->filename);
				//Add this item to the menu.
				stabSourceFile = stabSourceFile->next;
			}
		
			SendDlgItemMessage (appState.hVariablesDlg, IDC_SYMBOLTYPE, CB_SELECTSTRING , 
				(WPARAM)0, (LPARAM)"Local Variables");
		} 
		SendDlgItemMessage (appState.hVariablesDlg, IDC_FUNCTION, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
		SendDlgItemMessage (appState.hVariablesDlg, IDC_VARIABLESFILENAME, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

		VariablesGui_refresh();
	}
	
}

/////////////////////////////////
//This refreshes the variables gui
//dialog.
/////////////////////////////////
void VariablesGui_refresh() {
	u32 address;
	s32 range;
	char text1[256];
	char* text2;
	char text3[256];
	StabFunction* stabFunction;
	StabSourceFile* sourceFile;
	StabData* stabData;
	Elf32_Sym* elfData;
	LONG listStyles;
	char* nullString = "";
	range = 0;
	u32 currentSelection, topIndex;

	SendDlgItemMessage (appState.hVariablesDlg, IDC_VARIABLELIST, WM_SETREDRAW, 
		FALSE, 0);
	SendDlgItemMessage (appState.hVariablesDlg, IDC_VARTITLE, WM_SETREDRAW, 
		FALSE, 0);
	//SendDlgItemMessage (appState.hVariablesDlg, IDC_SYMBOLADDRESS, WM_SETREDRAW, 
	//	FALSE, 0);

	//Disable the address box
	//EnableWindow( GetDlgItem (appState.hVariablesDlg, IDC_SYMBOLADDRESS), FALSE);
	//Get the current index before clearing it out
	topIndex = SendDlgItemMessage (appState.hVariablesDlg, IDC_VARIABLELIST, LB_GETTOPINDEX , (WPARAM)0, (LPARAM)0);
	currentSelection = SendDlgItemMessage (appState.hVariablesDlg, IDC_VARIABLELIST, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
	

	//Clear out the variables list.
	SendDlgItemMessage (appState.hVariablesDlg, IDC_VARIABLELIST, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
	
	//Set it to be sorted.
	listStyles = GetWindowLong(GetDlgItem(appState.hVariablesDlg, IDC_VARIABLELIST), GWL_STYLE);
	SetWindowLong(GetDlgItem(appState.hVariablesDlg, IDC_VARIABLELIST), GWL_STYLE, listStyles | LBS_SORT); 
	
	SendDlgItemMessage (appState.hVariablesDlg, IDC_VARTITLE, WM_SETTEXT, (WPARAM)0, (LPARAM)"LVal     RVal     *RVal    Name");
	/*
		sourceFile = Stabs_findSourceByAddress(&gba.stabs, R15CURRENT);
		if (sourceFile!= NULL)
			SendDlgItemMessage (appState.hVariablesDlg, IDC_VARIABLESFILENAME, WM_SETTEXT, (WPARAM)0, (LPARAM)sourceFile->filename);
		else 
			SendDlgItemMessage (appState.hVariablesDlg, IDC_VARIABLESFILENAME, WM_SETTEXT, (WPARAM)0, (LPARAM)"N/A");
	} else
		SendDlgItemMessage (appState.hVariablesDlg, IDC_VARIABLESFILENAME, WM_SETTEXT, (WPARAM)0, (LPARAM)"N/A");

	*/

	//If there is no current function, set it to be "none".
	/*stabFunction = Stabs_findFunctionByAddress(&gba.stabs, R15CURRENT);
	if (stabFunction == NULL) {
		SendDlgItemMessage (appState.hVariablesDlg, IDC_FUNCTION, CB_SELECTSTRING , 
				(WPARAM)0, (LPARAM)"None");	
	}*/
	
	//Get the current source file and function, since they are useful.
	sourceFile = Stabs_findSourceByAddress(&gba.stabs, R15CURRENT);
	stabFunction = Stabs_findFunctionByAddress(&gba.stabs, R15CURRENT);

	//If the user has "follow" checked, set them to the current file/function, or "None"
	if (SendDlgItemMessage (appState.hVariablesDlg, IDC_VARFOLLOW, BM_GETCHECK, (WPARAM)0 , (LPARAM)0 )) {
		if (sourceFile != NULL) 
			SendDlgItemMessage (appState.hVariablesDlg, IDC_VARIABLESFILENAME, CB_SELECTSTRING , 
				(WPARAM)0, (LPARAM)Stabs_demangle(sourceFile->filename));	
		if (stabFunction != NULL) 
			SendDlgItemMessage (appState.hVariablesDlg, IDC_FUNCTION, CB_SELECTSTRING , 
				(WPARAM)0, (LPARAM)Stabs_demangle(stabFunction->name));	
	}

	//Find out what the user selected.
	SendDlgItemMessage (appState.hVariablesDlg, IDC_SYMBOLTYPE, WM_GETTEXT, (WPARAM)255, (LPARAM)text1);			
	
	//disable the symbol address edit box if (and only if) the current selection is not
	//"Specify Address"
	if ( (strcmp(text1, "Specify Address:")) != 0 && (strcmp(text1, "Specify Name:"))) {
		EnableWindow( GetDlgItem (appState.hVariablesDlg, IDC_SYMBOLADDRESS), FALSE);
	}

	//////////////////////////////////////////////
	//If the user chooses local variables
	//////////////////////////////////////////////
	if ( strcmp(text1, "Local Variables") == 0) {
		

		SendDlgItemMessage (appState.hVariablesDlg, IDC_FUNCTION, WM_GETTEXT, (WPARAM)255, (LPARAM)text1);			
		if (strcmp(text1, "All") == 0 ) {	
			
			SendDlgItemMessage (appState.hVariablesDlg, IDC_VARTITLE, WM_SETTEXT, 
				(WPARAM)0, (LPARAM)"Stack Offset  Name");
			
			//Set the list to be unsorted (otherwise it will sort by stack offset, which isn't too useful)
			//Doesn't seem to have any effect.  Oh well, it's the thought that counts.
			listStyles = GetWindowLong(GetDlgItem(appState.hVariablesDlg, IDC_VARIABLELIST), GWL_STYLE);
			listStyles &= (~LBS_SORT);
			//u32 blah = SetWindowLong(GetDlgItem(appState.hVariablesDlg, IDC_VARIABLELIST), GWL_STYLE, listStyles); 
			
			stabFunction = gba.stabs.functionList.next;
						
			while (stabFunction != NULL) {
				//First print parameters
				stabData = stabFunction->paramList.next;
				while (stabData != NULL) {
					sprintf(text1, "%08X      %s (%s())", stabData->entry.n_value, stabData->name,
						stabFunction->name);
					SendDlgItemMessage (appState.hVariablesDlg, IDC_VARIABLELIST, LB_ADDSTRING, 
						(WPARAM)0, (LPARAM)text1);
					stabData = stabData->next;
				}

				//Then local variables.
				stabData = stabFunction->localVarList.next;
				while (stabData != NULL) {
					sprintf(text1, "%08X      %s (%s())", stabData->entry.n_value, stabData->name,
						stabFunction->name);
					SendDlgItemMessage (appState.hVariablesDlg, IDC_VARIABLELIST, LB_ADDSTRING, 
						(WPARAM)0, (LPARAM)text1);
					stabData = stabData->next;
				}
				stabFunction = stabFunction->next;
			}
		//Otherwise set the user to the current function
		} else if (stabFunction != NULL) {	//If we are currently in this function, print out the info.
			u32 stackReg;
			if (arm.thumbMode)
				stackReg = 7;
			else
				stackReg = 11;

			if (strcmp(text1, Stabs_demangle(stabFunction->name)) == 0) {
				stabData = stabFunction->paramList.next;
				while (stabData != NULL) {
					text2 = VariablesGui_getMemoryString((s32)stabData->entry.n_value+(s32)arm.r[stackReg], stabData->name );
					SendDlgItemMessage (appState.hVariablesDlg, IDC_VARIABLELIST, LB_ADDSTRING, 
						(WPARAM)0, (LPARAM)text2);
					stabData = stabData->next;
				}

				//Then local variables.
				stabData = stabFunction->localVarList.next;
				while (stabData != NULL) {
					text2 = VariablesGui_getMemoryString((s32)stabData->entry.n_value+(s32)arm.r[stackReg], stabData->name );
					SendDlgItemMessage (appState.hVariablesDlg, IDC_VARIABLELIST, LB_ADDSTRING, 
						(WPARAM)0, (LPARAM)text2);
					stabData = stabData->next;
				}
			}
		} else {	//Otherwise print out offsets only.
				SendDlgItemMessage (appState.hVariablesDlg, IDC_VARTITLE, WM_SETTEXT, 
					(WPARAM)0, (LPARAM)"Stack Offset  Name");
				
				//THIS IS IDENTICAL TO THE CODE FOR "All" FUNCTIONS (ABOVE)
				//First print parameters
				stabFunction = (StabFunction*)Stabs_findByDemangledName((StabListEntry*)&gba.stabs.functionList, text1);
				if (stabFunction != NULL) {
					stabData = stabFunction->paramList.next;
					while (stabData != NULL) {
						sprintf(text1, "%08X      %s (%s())", stabData->entry.n_value, stabData->name,
							stabFunction->name);
						SendDlgItemMessage (appState.hVariablesDlg, IDC_VARIABLELIST, LB_ADDSTRING, 
							(WPARAM)0, (LPARAM)text1);
						stabData = stabData->next;
					}

					//Then local variables.
					stabData = stabFunction->localVarList.next;
					while (stabData != NULL) {
						sprintf(text1, "%08X      %s (%s())", stabData->entry.n_value, stabData->name,
							stabFunction->name);
						SendDlgItemMessage (appState.hVariablesDlg, IDC_VARIABLELIST, LB_ADDSTRING, 
							(WPARAM)0, (LPARAM)text1);
						stabData = stabData->next;
					}//while
				}//if
			
		} //else
		
		
	}
	//////////////////////////////////////////////
	//If the user chooses static variables
	//////////////////////////////////////////////
	if (strcmp(text1, "Static Variables") == 0) {
		SendDlgItemMessage (appState.hVariablesDlg, IDC_FUNCTION, WM_GETTEXT, (WPARAM)255, (LPARAM)text1);
		if (strcmp(text1, "All") == 0) {	//If the user wants to look in ALL functions, just list em all.
			
			stabFunction = gba.stabs.functionList.next;
						
			while (stabFunction != NULL) {
				stabData = stabFunction->staticVarList.next;
				while (stabData != NULL) {
					text2 = VariablesGui_getMemoryString(stabData->entry.n_value, stabData->name );
					SendDlgItemMessage (appState.hVariablesDlg, IDC_VARIABLELIST, LB_ADDSTRING, 
						(WPARAM)0, (LPARAM)text2);
					stabData = stabData->next;
				}
				stabFunction = stabFunction->next;
			}
			
		} else  {
			stabFunction = (StabFunction*)Stabs_findByDemangledName((StabListEntry*)&gba.stabs.functionList, text1);
			if (stabFunction != NULL) {
				stabData = stabFunction->staticVarList.next;
				while (stabData != NULL) {
						text2 = VariablesGui_getMemoryString(stabData->entry.n_value, stabData->name );
						SendDlgItemMessage (appState.hVariablesDlg, IDC_VARIABLELIST, LB_ADDSTRING, 
							(WPARAM)0, (LPARAM)text2);
						stabData = stabData->next;
				}
			}
		}//if/else
	}
	//////////////////////////////////////////////
	//If the user chose "global variables"
	//////////////////////////////////////////////
	else  if (strcmp(text1, "Global Variables") == 0) {
		//Get the name of the function listed in the combo box.
		SendDlgItemMessage (appState.hVariablesDlg, IDC_VARIABLESFILENAME, WM_GETTEXT , 
				(WPARAM)255, (LPARAM)text1);
		if (strcmp(text1, "All") == 0) {	//If the user wants to find all global variables, get them from
									//The ELF file
			for (int i = 0; i < gba.elfFile.numObjectSymbols; i++) {
				elfData = &gba.elfFile.objectSymbols[i];
				if (ELF32_ST_BIND(elfData->st_info) == STB_GLOBAL) {
					text2 = VariablesGui_getMemoryString(elfData->st_value,  ELFFile_getSymbolName(&gba.elfFile, elfData));
					SendDlgItemMessage (appState.hVariablesDlg, IDC_VARIABLELIST, LB_ADDSTRING, 
							(WPARAM)0, (LPARAM)text2);
				}
			}
		} else {
			sourceFile = (StabSourceFile*)Stabs_findByName((StabListEntry*)&gba.stabs.sourceList, text1);

			if (sourceFile != NULL) {
				//Print out the global variables associated with this file.
				stabData = sourceFile->globalList.next;
				while (stabData != NULL) {
					//The tricky thing about global variables is that stabs contains no data on them.
					//However, ELF does...
					elfData = ELFFile_findByName(&gba.elfFile, Stabs_demangle(stabData->name));
					if (elfData != NULL) {
						text2 = VariablesGui_getMemoryString(elfData->st_value, Stabs_demangle(stabData->name) );
						SendDlgItemMessage (appState.hVariablesDlg, IDC_VARIABLELIST, LB_ADDSTRING, 
						(WPARAM)0, (LPARAM)text2);
					}
					stabData = stabData->next;
				}

			
			}		
		}
		
	}
	
	///////////////////////////////////////////////
	//If the user wants functions
	///////////////////////////////////////////////
	else  if (strcmp(text1, "Functions") == 0) {
		SendDlgItemMessage (appState.hVariablesDlg, IDC_VARIABLESFILENAME, WM_GETTEXT , 
				(WPARAM)255, (LPARAM)text1);
		if (strcmp(text1, "All") == 0) {	//If the user wants to find all global variables, get them from
											//The ELF file
			for (int i = 0; i < gba.elfFile.numFuncSymbols; i++) {
				elfData = &gba.elfFile.funcSymbols[i];
				text2 = VariablesGui_getMemoryString(elfData->st_value,  ELFFile_getSymbolName(&gba.elfFile, elfData));
				SendDlgItemMessage (appState.hVariablesDlg, IDC_VARIABLELIST, LB_ADDSTRING, 
							(WPARAM)0, (LPARAM)text2);
			}
		} else {
			SendDlgItemMessage (appState.hVariablesDlg, IDC_VARTITLE, WM_SETTEXT, (WPARAM)0, 
				(LPARAM)"Start    End      Name");
			//Print out the functions associated with this file.
			stabFunction = gba.stabs.functionList.next;
			while (stabFunction != NULL) {
				if (strcmp(text1, stabFunction->filename) == 0) {	//If the 
					sprintf(text3, "%08X %08X %s", stabFunction->functionBegin, stabFunction->blockEnd, stabFunction->name); 
					//text2 = VariablesGui_getMemoryString(stabFunction->functionBegin, Stabs_demangle(stabFunction->name) );
					SendDlgItemMessage (appState.hVariablesDlg, IDC_VARIABLELIST, LB_ADDSTRING, 
					(WPARAM)0, (LPARAM)text3);
				}
				stabFunction = stabFunction->next;
			}

			
		}
	}
	///////////////////////////////////////////
	//If the user wants to specify an address
	//////////////////////////////////////////
	else if (strcmp(text1, "Specify Address:") == 0) {
		//Enable the address box
		EnableWindow( GetDlgItem (appState.hVariablesDlg, IDC_SYMBOLADDRESS), TRUE);
		//Get the text form of the address
		SendDlgItemMessage (appState.hVariablesDlg, IDC_SYMBOLADDRESS, WM_GETTEXT, 
			(WPARAM)(255) , (LPARAM)text1);		
		address = strtoul(text1, &((char*)nullString), 16);
		text2 = ELFFile_getSymbolName(&gba.elfFile, address);
		//If there is a symbol that matches this variable, print it
		if (text2 != NULL)
			text2 = VariablesGui_getMemoryString(address, text2 );
		//Otherwise, "".
		else 
			text2 = VariablesGui_getMemoryString(address, "" );
		SendDlgItemMessage (appState.hVariablesDlg, IDC_VARIABLELIST, LB_ADDSTRING, 
					(WPARAM)0, (LPARAM)text2);
	}
	///////////////////////////////////////////
	//If the user wants to specify a name
	//This could be revamped a whole lot to find all matches.
	//////////////////////////////////////////
	else if (strcmp(text1, "Specify Name:") == 0) {
		Elf32_Sym* symbol;
		//Enable the address box
		EnableWindow( GetDlgItem (appState.hVariablesDlg, IDC_SYMBOLADDRESS), TRUE);
		//Get the text form of the address
		SendDlgItemMessage (appState.hVariablesDlg, IDC_SYMBOLADDRESS, WM_GETTEXT, 
			(WPARAM)(255) , (LPARAM)text1);		
		
		//This will not find local variables, or even partial matches.
		symbol = ELFFile_findByName(&gba.elfFile, text1);
		if (symbol != NULL) {
			text2 = VariablesGui_getMemoryString(symbol->st_value, ELFFile_getSymbolName(&gba.elfFile, symbol) );
			SendDlgItemMessage (appState.hVariablesDlg, IDC_VARIABLELIST, LB_ADDSTRING, 
						(WPARAM)0, (LPARAM)text2);
		}
	}

	////////////////////////////////////////////////
	//If the user (probably me:) wants (stabs) files
	/////////////////////////////////////////
	else  if (strcmp(text1, "Files") == 0) {
			SendDlgItemMessage (appState.hVariablesDlg, IDC_VARTITLE, WM_SETTEXT, (WPARAM)0, 
				(LPARAM)"Start    End      Lines(source) Lines(asm) Functions Globals  Name");
			//Print out the functions associated with this file.
			sourceFile = gba.stabs.sourceList.next;
			while (sourceFile != NULL) {
					sprintf(text1, "%08X %08X %-13d %-10d %-9d %-7d  %s", sourceFile->fileStart, 
									sourceFile->fileEnd, sourceFile->numSourceLines,
									sourceFile->numAsmLines, sourceFile->numFunctions, 
									sourceFile->numGlobalVars, sourceFile->filename);
					SendDlgItemMessage (appState.hVariablesDlg, IDC_VARIABLELIST, LB_ADDSTRING, 
						(WPARAM)0, (LPARAM)text1);
				
					sourceFile = sourceFile->next;
			}
	}

	///////////////////////////////////////////////
	//If the user wants ELF Sections
	///////////////////////////////////////////////
	else  if (strcmp(text1, "Elf Sections") == 0) {
		Elf32_Shdr* sHeader;
		SendDlgItemMessage (appState.hVariablesDlg, IDC_VARTITLE, WM_SETTEXT, (WPARAM)0, 
			(LPARAM)"Address  File Off. Size     Type         Name");
		for (int i = 0; i < gba.elfFile.header.e_shnum; i++) {
			sHeader = &gba.elfFile.sectionHeaders[i];
			sprintf(text1, "%s", ELFFile_getSectionInfo(&gba.elfFile, sHeader));
			SendDlgItemMessage (appState.hVariablesDlg, IDC_VARIABLELIST, LB_ADDSTRING, 
						(WPARAM)0, (LPARAM)text1);
		}
	}

	///////////////////////////////////////////////
	//If the user wants ELF Program Entries
	///////////////////////////////////////////////
	else  if (strcmp(text1, "Elf Program Entries") == 0) {
		Elf32_Phdr* pHeader;
		SendDlgItemMessage (appState.hVariablesDlg, IDC_VARTITLE, WM_SETTEXT, (WPARAM)0, 
			(LPARAM)"PAddr.   VAddr.   File Off. FSize    MSize    Align    Flags Type");
		for (int i = 0; i < gba.elfFile.header.e_phnum; i++) {
			pHeader = &gba.elfFile.programHeaders[i];
			sprintf(text1, "%s", ELFFile_getProgramInfo(&gba.elfFile, pHeader));
			SendDlgItemMessage (appState.hVariablesDlg, IDC_VARIABLELIST, LB_ADDSTRING, 
						(WPARAM)0, (LPARAM)text1);
		}
	}

	///////////////////////////////////////////////
	//If the user wants ELF Data symbols
	///////////////////////////////////////////////
	else  if (strcmp(text1, "Elf Data Symbols") == 0) {
		SendDlgItemMessage (appState.hVariablesDlg, IDC_VARTITLE, WM_SETTEXT, (WPARAM)0, 
			(LPARAM)"LVal     RVal     *RVal    Size      Bind    Type     Name");
		for (int i = 0; i < gba.elfFile.numObjectSymbols; i++) {
			elfData = &gba.elfFile.objectSymbols[i];
			sprintf(text1, "%s %s", ELFFile_getInfo(elfData), ELFFile_getSymbolName(&gba.elfFile, elfData));
			text2 = VariablesGui_getMemoryString(elfData->st_value,  text1);
			
				SendDlgItemMessage (appState.hVariablesDlg, IDC_VARIABLELIST, LB_ADDSTRING, 
						(WPARAM)0, (LPARAM)text2);
		}
	}

	/////////////////////////////////////////////////
	//If the user wants all ELF symbols
	////////////////////////////////////////////////
	else if (strcmp(text1, "All Elf Symbols") == 0) {
		SendDlgItemMessage (appState.hVariablesDlg, IDC_VARTITLE, WM_SETTEXT, (WPARAM)0, (LPARAM)"LVal     RVal     *RVal    Size      Bind    Type     Name");
		for (int i = 0; i < gba.elfFile.numSymbols; i++) {
			elfData = &gba.elfFile.symbolTable[i];
			sprintf(text1, "%s %s", ELFFile_getInfo(elfData), ELFFile_getSymbolName(&gba.elfFile, elfData));
			text2 = VariablesGui_getMemoryString(elfData->st_value,  text1);
				SendDlgItemMessage (appState.hVariablesDlg, IDC_VARIABLELIST, LB_ADDSTRING, 
						(WPARAM)0, (LPARAM)text2);
		}
	}

	/////////////////////////////////////////////////
	//If the user wants all stabs symbols
	////////////////////////////////////////////////
	else if (strcmp(text1, "Stabs Source Lines") == 0) {
		SendDlgItemMessage (appState.hVariablesDlg, IDC_VARTITLE, WM_SETTEXT, (WPARAM)0, 
			(LPARAM)"Address  Line");
		StabSourceLine* sourceLine;
		
		SendDlgItemMessage (appState.hVariablesDlg, IDC_VARIABLESFILENAME, WM_GETTEXT , 
				(WPARAM)255, (LPARAM)text1);		
		sourceFile = (StabSourceFile*)Stabs_findByName((StabListEntry*)&gba.stabs.sourceList, text1);

		if (sourceFile != NULL) {
			//Print out the source lines associated with this file.
			sourceLine = sourceFile->lineList.next;
			while (sourceLine != NULL) {
				sprintf(text1, "%08X %d", sourceLine->entry.n_value, sourceLine->entry.n_desc);	
				SendDlgItemMessage (appState.hVariablesDlg, IDC_VARIABLELIST, LB_ADDSTRING, 
						(WPARAM)0, (LPARAM)text1);
				sourceLine = sourceLine->next;
			}
		}
	}

	//After all these changes, have windows redraw the stuff.
	SendDlgItemMessage (appState.hVariablesDlg, IDC_VARIABLELIST, LB_SETCURSEL, 
				currentSelection, 0);    
	SendDlgItemMessage (appState.hVariablesDlg, IDC_VARIABLELIST, LB_SETTOPINDEX, 
				topIndex, 0);  

	SendDlgItemMessage (appState.hVariablesDlg, IDC_VARIABLELIST, WM_SETREDRAW, 
		TRUE, 0);  
	SendDlgItemMessage (appState.hVariablesDlg, IDC_VARTITLE, WM_SETREDRAW, 
		TRUE, 0);
	SendDlgItemMessage (appState.hVariablesDlg, IDC_SYMBOLADDRESS, WM_SETREDRAW, 
		TRUE, 0);
	InvalidateRect(appState.hVariablesDlg, NULL, FALSE);

}

////////////////////////////////////////////////////////
//VariablesGui_getMemoryString
//This gets a string containing the lvalue, rvalue, and *rvalue
//for the given lvalue.
///////////////////////////////////////////////////////
char* VariablesGui_getMemoryString(u32 address, char* name) {
	static char text[256];
	char lValue[9];
	char rValue[9];
	char rStarValue[9];
	u32 index, offset;
	sprintf(lValue, "%08X", address);
	index = address >> 24;
	offset = address & 0x00FFFFFF;
	if (gbaMem.memorySize[index] > offset) {
		address = *(u32*)(&gbaMem.u8Mem[index][offset]);
		sprintf(rValue, "%08X", address);
	} else sprintf(rValue, "NIL     ", address);

	index = address >> 24;
	offset = address & 0x00FFFFFF;
	if (gbaMem.memorySize[index] > offset) {
		address = *(u32*)(&gbaMem.u8Mem[index][offset]);
		sprintf(rStarValue, "%08X", address);
	} else sprintf(rStarValue, "NIL     ", address);
	
	sprintf(text, "%s %s %s %s", lValue, rValue, rStarValue, Stabs_demangle(name));
	return text;
	
}

 //////////////////////////////////////////////////////
//VariablesGui_variableScroll
//Handles scrolling for variables scroll bar
///////////////////////////////////////////////////////
void VariablesGui_variableScroll(WPARAM wParam, LPARAM lParam) {
	u32 pos = SendDlgItemMessage (appState.hVariablesDlg, IDC_VARIABLESCROLL, SBM_GETPOS, (WPARAM)0, (LPARAM)0);
	
	
	switch (LOWORD(wParam)) /* check what part was pressed */
	{
		case SB_THUMBTRACK:  /* user movded scroll thumb */
			pos = HIWORD(wParam);
	        break;
		
		case SB_THUMBPOSITION:  /* user movded scroll thumb */
			pos = HIWORD(wParam);
			break ;

		case SB_LINEDOWN:       /* user clicked down (right) arrow */
			pos++;
			break ;

		case SB_LINEUP:         /* user clicked up (left) arrow */
			pos--;
			break ;

		case SB_PAGEDOWN:       /* user clicked down (right) area */
			pos+=8;
			break ;

		case SB_PAGEUP:         /* user clicked up (right) area */
			pos-=8;
			break ;

			default:
			break;
		}//end of switch (LOWORD(wParam)
		SendDlgItemMessage (appState.hVariablesDlg, IDC_VARIABLESCROLL, SBM_SETPOS, (WPARAM)pos, (LPARAM)TRUE);				
		VariablesGui_refresh();
}//regScroll()
