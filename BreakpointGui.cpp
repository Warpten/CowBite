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

extern State appState;



/////////////////////////////////////////////////////////////////////////////////////////
//BreakpointGui_Proc
//The windows callback for dumpinfo dialog.
//Presents the user with options for dumping files.
////////////////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK BreakpointGui_Proc (HWND hBreakpointDlg, UINT wMessage, 
                                                        WPARAM wParam, LPARAM lParam)
{
	char text[255];
	u32 address;
	char *nullString = "";
		
	 switch (wMessage)       /* process messages */
    {
        case WM_INITDIALOG:
			appState.hBreakpointDlg = hBreakpointDlg;
			appState.settings.debugSettings.lastCondBreakpoint.value1.name = "";
			appState.settings.debugSettings.lastCondBreakpoint.value2.name = "";
			//Determine the type of value
			SendDlgItemMessage (hBreakpointDlg, IDC_VALUE1LIST, CB_ADDSTRING, (WPARAM)0, (LPARAM)"*Address (u8)");
			SendDlgItemMessage (hBreakpointDlg, IDC_VALUE1LIST, CB_ADDSTRING, (WPARAM)0, (LPARAM)"*Address (u16)");
			SendDlgItemMessage (hBreakpointDlg, IDC_VALUE1LIST, CB_ADDSTRING, (WPARAM)0, (LPARAM)"*Address (u32)");
			SendDlgItemMessage (hBreakpointDlg, IDC_VALUE1LIST, CB_ADDSTRING, (WPARAM)0, (LPARAM)"Hardware Reg");
			SendDlgItemMessage (hBreakpointDlg, IDC_VALUE1LIST, CB_ADDSTRING, (WPARAM)0, (LPARAM)"Register");
			SendDlgItemMessage (hBreakpointDlg, IDC_VALUE1LIST, CB_ADDSTRING, (WPARAM)0, (LPARAM)"Constant");
			SendDlgItemMessage (hBreakpointDlg, IDC_VALUE2LIST, CB_ADDSTRING, (WPARAM)0, (LPARAM)"*Address (u8)");
			SendDlgItemMessage (hBreakpointDlg, IDC_VALUE2LIST, CB_ADDSTRING, (WPARAM)0, (LPARAM)"*Address (u16)");
			SendDlgItemMessage (hBreakpointDlg, IDC_VALUE2LIST, CB_ADDSTRING, (WPARAM)0, (LPARAM)"*Address (u32)");
			SendDlgItemMessage (hBreakpointDlg, IDC_VALUE2LIST, CB_ADDSTRING, (WPARAM)0, (LPARAM)"Hardware Reg");
			SendDlgItemMessage (hBreakpointDlg, IDC_VALUE2LIST, CB_ADDSTRING, (WPARAM)0, (LPARAM)"Register");
			SendDlgItemMessage (hBreakpointDlg, IDC_VALUE2LIST, CB_ADDSTRING, (WPARAM)0, (LPARAM)"Constant");
	
			SendDlgItemMessage (hBreakpointDlg, IDC_VALUE1LIST, 
			CB_SETCURSEL, (WPARAM)appState.settings.debugSettings.lastCondBreakpoint.value1.type, (LPARAM)0);
			SendDlgItemMessage (hBreakpointDlg, IDC_VALUE2LIST, 
			CB_SETCURSEL, (WPARAM)appState.settings.debugSettings.lastCondBreakpoint.value2.type, (LPARAM)0);

			//Fill up the condition list box
			SendDlgItemMessage (hBreakpointDlg, IDC_CONDITION, CB_ADDSTRING, (WPARAM)0, (LPARAM)"==");
			SendDlgItemMessage (hBreakpointDlg, IDC_CONDITION, CB_ADDSTRING, (WPARAM)0, (LPARAM)"!=");
			SendDlgItemMessage (hBreakpointDlg, IDC_CONDITION, CB_ADDSTRING, (WPARAM)0, (LPARAM)"&&");
			SendDlgItemMessage (hBreakpointDlg, IDC_CONDITION, CB_ADDSTRING, (WPARAM)0, (LPARAM)"&");
			SendDlgItemMessage (hBreakpointDlg, IDC_CONDITION, CB_ADDSTRING, (WPARAM)0, (LPARAM)"||");
			SendDlgItemMessage (hBreakpointDlg, IDC_CONDITION, CB_ADDSTRING, (WPARAM)0, (LPARAM)"|");
			SendDlgItemMessage (hBreakpointDlg, IDC_CONDITION, CB_ADDSTRING, (WPARAM)0, (LPARAM)"!");
			SendDlgItemMessage (hBreakpointDlg, IDC_CONDITION, CB_ADDSTRING, (WPARAM)0, (LPARAM)"== ~");
			SendDlgItemMessage (hBreakpointDlg, IDC_CONDITION, CB_ADDSTRING, (WPARAM)0, (LPARAM)"accessed");

			//Fill up the PC address box]
			SendDlgItemMessage (hBreakpointDlg, IDC_PCVALUE, CB_ADDSTRING, (WPARAM)0, (LPARAM)"ANY");
			SendDlgItemMessage (hBreakpointDlg, IDC_PCVALUE, CB_ADDSTRING, (WPARAM)0, (LPARAM)"02000000");
			SendDlgItemMessage (hBreakpointDlg, IDC_PCVALUE, CB_ADDSTRING, (WPARAM)0, (LPARAM)"03000000");
			SendDlgItemMessage (hBreakpointDlg, IDC_PCVALUE, CB_ADDSTRING, (WPARAM)0, (LPARAM)"08000000");

			
			BreakpointGui_refresh();

			SetWindowPos( hBreakpointDlg, NULL, appState.settings.breakpointPos.x,
						appState.settings.breakpointPos.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER );


			//appState.settings.dumpSetting = IDC_RSTATE;
			//CheckRadioButton( hBreakpointDlg, IDC_RSTATE, IDC_RTHUMBLOCATION, IDC_RSTATE);
			//SendDlgItemMessage (hBreakpointDlg, IDC_DUMPSTART, WM_SETTEXT, (WPARAM)0 , (LPARAM)"8000000");
			//SendDlgItemMessage (hBreakpointDlg, IDC_DUMPEND, WM_SETTEXT, (WPARAM)0 , (LPARAM)"8000000");
			ShowWindow(hBreakpointDlg, SW_SHOW);
			return (TRUE);

		case WM_MOVE:
			{	RECT rect;
				GetWindowRect(hBreakpointDlg, &rect);
				appState.settings.breakpointPos.x = rect.left;//LOWORD(lParam);
				appState.settings.breakpointPos.y = rect.top;//HIWORD(lParam);
			}
			break;

		case WM_SYSCOMMAND:	/*User clicks on close x button*/
			if (wParam == SC_CLOSE) {
				DestroyWindow(hBreakpointDlg);
				return (TRUE) ;
				
			}
			break;			

        case WM_COMMAND:    /* controls in dialog box activated */
            			
			switch (LOWORD(wParam))
			{
				case IDC_DEBUGADDBREAKPOINT:
					//Get the text form of the address
					SendDlgItemMessage (hBreakpointDlg, IDC_DEBUGADDRESS, WM_GETTEXT, (WPARAM)(255) , (LPARAM)text);
					strcpy(appState.settings.debugSettings.lastBreakpointText, text);
					address = strtoul(text, &((char*)nullString), 16);

					if (ARM_containsBreakpoint(address))
						appState.armDebugger->removeBreakpoint(address);
					else
						appState.armDebugger->addBreakpoint(address);

					//Refresh all to reflect the changes
					Gui_refreshAllDialogs(1);
					break;	
				case IDADD: {
						int retVal;
						retVal = BreakpointGui_getValue(IDC_VALUE1, &appState.settings.debugSettings.lastCondBreakpoint.value1);
						retVal &= BreakpointGui_getValue(IDC_VALUE2, &appState.settings.debugSettings.lastCondBreakpoint.value2);
						if (retVal) {
							//Get the address, if any						
							SendDlgItemMessage (hBreakpointDlg, IDC_DEBUGADDRESS, WM_GETTEXT, (WPARAM)(255) , (LPARAM)text);
							if ( (strcmp(text, "Any") == 0) || (strcmp(text, "") == 0) ) {
								address = 0xFFFFFFFF;	//This means "do the check at all breakpoints"
							} else {
								address = strtoul(text, &((char*)nullString), 16);
							}
							appState.settings.debugSettings.lastCondBreakpoint.breakpoint = address;
							appState.armDebugger->addConditionalBreakpoint(&appState.settings.debugSettings.lastCondBreakpoint);
						}
						BreakpointGui_refresh();
					}
						break;	
				case IDREMOVE: {
						//Get the current selection from the list box, remove it.
						int i = SendDlgItemMessage (hBreakpointDlg, IDC_CONDITIONALBREAKPOINTS, CB_GETCURSEL, (WPARAM)0 , (LPARAM)0);		
						appState.armDebugger->removeConditionalBreakpoint(&arm.condBreakpoints[i]);
						BreakpointGui_refresh();
						}
						break;	
				case ID_BPCLOSE:
						DestroyWindow(hBreakpointDlg);
						break;
				case IDC_CONDITION:
					if (HIWORD(wParam) == CBN_SELENDOK) {
						int i = SendDlgItemMessage (hBreakpointDlg, IDC_CONDITION, CB_GETCURSEL, (WPARAM)0 , (LPARAM)0);		
						//Note that we don't just give it the index, we shift *by* the index in order
						//to get an or-able value
						appState.settings.debugSettings.lastCondBreakpoint.condition = 1 << i;
						if ( (1 << i) == BPCOND_NOT ) {
							EnableWindow( GetDlgItem (appState.hBreakpointDlg, IDC_VALUE1), FALSE);
						} else EnableWindow( GetDlgItem (appState.hBreakpointDlg, IDC_VALUE1), TRUE);
						if ( (1 << i) == BPCOND_ACC ) {
							EnableWindow( GetDlgItem (appState.hBreakpointDlg, IDC_VALUE2), FALSE);
						} else EnableWindow( GetDlgItem (appState.hBreakpointDlg, IDC_VALUE2), TRUE);

						//BreakpointGui_refresh();
					}

					break;
				case IDC_VALUE1LIST:
					if (HIWORD(wParam) == CBN_SELCHANGE) {
						int i = SendDlgItemMessage (hBreakpointDlg, IDC_VALUE1LIST, CB_GETCURSEL, (WPARAM)0 , (LPARAM)0);		
						appState.settings.debugSettings.lastCondBreakpoint.value1.type = i;
						BreakpointGui_fillValueBox(IDC_VALUE1, &appState.settings.debugSettings.lastCondBreakpoint.value1);
						//BreakpointGui_refresh();
					}

					break;
				case IDC_VALUE2LIST:
					if (HIWORD(wParam) == CBN_SELCHANGE) {
						int i = SendDlgItemMessage (hBreakpointDlg, IDC_VALUE2LIST, CB_GETCURSEL, (WPARAM)0 , (LPARAM)0);		
						appState.settings.debugSettings.lastCondBreakpoint.value2.type = i;
						BreakpointGui_fillValueBox(IDC_VALUE2, &appState.settings.debugSettings.lastCondBreakpoint.value2);
						//BreakpointGui_refresh();
					}
					break;
				//Depending on the type the user has selected in the
				//value 1 list box, get a pointer to the value
				case IDC_VALUE1:
					if (HIWORD(wParam) == CBN_SELCHANGE) {
					
						//BreakpointGui_refresh();
					}
					break;
				case IDC_VALUE2:
					if (HIWORD(wParam) == CBN_SELCHANGE) {
						
						//BreakpointGui_refresh();
					}
					break;
				default:
					return (FALSE);	//Otherwise we get a stack overflow.
					break;
            }//ENd of switch (LOWORD(wParam);
			return (TRUE);
			break;

	
		case WM_DESTROY:
				appState.hBreakpointDlg = NULL;
				//return (TRUE);
				break;
		
	}//End of switch(message)
	return (FALSE) ;    /* return FALSE to signify message not processed */
}

////////////////////////////////////////////
//Fills in the value list box with values depending
//on the type the user has selected.
////////////////////////////////////////////
void BreakpointGui_fillValueBox(u32 iD, BreakpointValue* value) {
	HWND hBreakpointDlg = appState.hBreakpointDlg;
	char text[255];
	u32 valueType;
	u32 breakpointMasks[16];
//	u32 curSel;	
	memset(breakpointMasks, 0xFFFFFFFF, sizeof(breakpointMasks));
	breakpointMasks[BPVALUE_U8LOCATION] = 0x000000FF;
	breakpointMasks[BPVALUE_U16LOCATION] = 0x0000FFFF;
	value->mask = breakpointMasks[value->type];
	
	valueType = value->type;
	
	//Save the last selection
	//curSel = SendDlgItemMessage (appState.hBreakpointDlg, iD, CB_GETCURSEL, (WPARAM)0 , (LPARAM)0);

	//Clear out the list box of previous values.
	SendDlgItemMessage (appState.hBreakpointDlg, iD, CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
	switch (valueType) {
		case BPVALUE_CONSTANT:
		case BPVALUE_U8LOCATION:
		case BPVALUE_U16LOCATION:
		case BPVALUE_U32LOCATION: {
			//Fill it up with values from any breakpoints already in our list
			for (u32 i = 0; i < arm.numCondBreakpoints; i++) {
				if (arm.condBreakpoints[i].value1.type == valueType) {
					sprintf(text, "%08X", arm.condBreakpoints[i].value1.lValue);
					SendDlgItemMessage (appState.hBreakpointDlg, iD, CB_ADDSTRING, (WPARAM)0, (LPARAM)text);
				}
				if (arm.condBreakpoints[i].value2.type == valueType) {
					sprintf(text, "%08X", arm.condBreakpoints[i].value2.lValue);
					SendDlgItemMessage (hBreakpointDlg, iD, CB_ADDSTRING, (WPARAM)0, (LPARAM)text);
				}
			}

			SendDlgItemMessage (hBreakpointDlg, iD, CB_ADDSTRING, (WPARAM)0, (LPARAM)"02000000");
			SendDlgItemMessage (hBreakpointDlg, iD, CB_ADDSTRING, (WPARAM)0, (LPARAM)"03000000");
			SendDlgItemMessage (hBreakpointDlg, iD, CB_ADDSTRING, (WPARAM)0, (LPARAM)"08000000");
			break;
		}
		case BPVALUE_HARDWAREREG: {
			//Fill it up with values from the gba debugger register list
			for (int i = 0; i < NUMHARDWAREREGS; i++) {
				sprintf(text, "%03X %s", gbaDebugger.regEntries[i].offset, gbaDebugger.regEntries[i].name);
				SendDlgItemMessage (hBreakpointDlg, iD, CB_ADDSTRING, 
					(WPARAM)0, (LPARAM)text);
			}
			break;
		}
		case BPVALUE_REG: {
			//Fill it up with register names.  For now does not include
			//banked registers.
			for (int i = 0; i < 16; i++) {
				sprintf(text, "r%02d", i);
				SendDlgItemMessage (hBreakpointDlg, iD, CB_ADDSTRING, 
					(WPARAM)0, (LPARAM)text);
			}
			SendDlgItemMessage (hBreakpointDlg, iD, CB_ADDSTRING, 
					(WPARAM)0, (LPARAM)"CPSR");
			SendDlgItemMessage (hBreakpointDlg, iD, CB_ADDSTRING, 
					(WPARAM)0, (LPARAM)"SPSR");
			break;
		}
	}//case

	//Set the selectionb ack
	//SendDlgItemMessage (appState.hVariablesDlg, IDC_SYMBOLTYPE, CB_SELECTSTRING , 
	//			(WPARAM)0, (LPARAM)"Local Variables");
	SendDlgItemMessage (appState.hBreakpointDlg, iD, CB_SELECTSTRING, 
		(WPARAM)0 , (LPARAM)appState.armDebugger->breakpointValueToString(value));
}//fillValueBox


///////////////////////////////////////////////////
//refresh
//Fills up the list of conditional breakpoints, sets it to the currently
//seleted breakpoint, fills up all other lists boxes accordingly
///////////////////////////////////////////////////
void BreakpointGui_refresh() {
//	char* text;
	
	//Reset list of conditional breakpoints
	SendDlgItemMessage (appState.hBreakpointDlg, IDC_CONDITIONALBREAKPOINTS, CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
	//Fill up list of conditional breakpoints
	for (int i = 0; i < arm.numCondBreakpoints; i++) {
		//switch (arm.condBreakpoitns[i].value1->type)
		//strcpy(text, arm.condBreakpoints[i].text);
		SendDlgItemMessage (appState.hBreakpointDlg, IDC_CONDITIONALBREAKPOINTS, 
			CB_ADDSTRING, (WPARAM)0, (LPARAM)appState.armDebugger->condBreakpointToString(&arm.condBreakpoints[i]));
	}
	//select the last breakpoint added to the list.
	SendDlgItemMessage (appState.hBreakpointDlg, IDC_CONDITIONALBREAKPOINTS, 
			CB_SETCURSEL, (WPARAM)arm.numCondBreakpoints-1, (LPARAM)0);

	//Fill up the value boxes
	BreakpointGui_fillValueBox(IDC_VALUE1, &appState.settings.debugSettings.lastCondBreakpoint.value1);
	BreakpointGui_fillValueBox(IDC_VALUE2, &appState.settings.debugSettings.lastCondBreakpoint.value2);
}

////////////////////////////////////////////////////////
//getValue
//This gets a pointer to the value requested by the user
//I guess there's really no point in returning the pointer
//since it's hardcoded in here anyway.
////////////////////////////////////////////////////////
int BreakpointGui_getValue(u32 iD, BreakpointValue* value) {
	u32 address, index, offset;
	char* nullString = "";
	char text[256];
	RegEntry* regEntry;
//	u32 currentSel;

	//Before doign anything, save the current position for thsi list.
	index = SendDlgItemMessage (appState.hBreakpointDlg, iD, CB_GETCURSEL, (WPARAM)0 , (LPARAM)0);

	//Switch on the value num.  Set the pointer to the constant
	//by default, so we are guaranteed no errors.
	value->value = &value->constant;
	value->name = "";	//Set the naem field to something safe
	switch (value->type) {
		case BPVALUE_U8LOCATION:
		case BPVALUE_U16LOCATION:
		case BPVALUE_U32LOCATION:

			SendDlgItemMessage (appState.hBreakpointDlg, iD, WM_GETTEXT, (WPARAM)(32) , (LPARAM)text);		
			address = strtoul(text, &((char*)nullString), 16);
			
			index = address >> 24;
			offset = address & 0x00FFFFFF;
			
			if ( (index >= MEMORY_NUMSECTIONS) || (gbaMem.memorySize[index] == 0) ) {
				sprintf(text, "Address: %08X - This is an invalid region of memory.", address);
				MessageBox (appState.hBreakpointDlg, text,
				"Invalid Range", MB_ICONEXCLAMATION | MB_OK);
				//Set it to some constant
				value->type = BPVALUE_CONSTANT;
				value->lValue = value->constant;
				return 0;
			} else {
				value->lValue = address;
				value->value = (u32*)(gbaMem.u8Mem[index]+offset);
				//value->value += offset;
			}
			break;
		case BPVALUE_HARDWAREREG:
			//If it's a reg, get the index of the register and find the offset
			SendDlgItemMessage (appState.hBreakpointDlg, iD, WM_GETTEXT, (WPARAM)(32) , (LPARAM)text);
			//Because we waste 4 characters for the offset
			regEntry = GBADebugger_getRegEntry((char*)&(text[4]));
			if (regEntry == NULL)
				return 0;
			offset =  regEntry->offset;
			value->lValue = 0x4000000 + offset;	//so that later we can print out the name.
			value->name = regEntry->name;
			value->value = (u32*)(&(gbaMem.u8IORAM[offset]));
			value->mask = 0xFFFF;	//Values are 16 bit.
			break;
		case BPVALUE_REG:
			//If it's a reg, get the index of the register and find the offset
			index = SendDlgItemMessage (appState.hBreakpointDlg, iD, CB_GETCURSEL, (WPARAM)0 , (LPARAM)0);
			value->lValue = index-1;
			if ((index > 0) && (index < 17))	//If it's a normal register
				value->value = &arm.r[index-1];	//Remember that the the CPSR is first in list.
			else if (index == 0) {
				value->value = &arm.CPSR;
				value->lValue = 16;
			}
			else {
				value->value = arm.SPSR_current;
				value->lValue = 17;
			}
			break;
		case BPVALUE_CONSTANT:
			SendDlgItemMessage (appState.hBreakpointDlg, iD, WM_GETTEXT, (WPARAM)(32) , (LPARAM)text);		
			address = strtoul(text, &((char*)nullString), 16);
			value->lValue = address;
			*value->value = address;	//Our value->value points to a constant by default
			break;
		
	}//switch
	return 1;

	
}

