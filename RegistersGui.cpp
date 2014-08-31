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
#include <commctrl.h>
#include "Gui.h"
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////////////////
//Map_Proc
//The windows callback for the map drawing window.
////////////////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK RegistersGui_Proc (HWND hRegistersDlg, UINT wMessage, 
                                                        WPARAM wParam, LPARAM lParam)
{
	ARMDebugger* myDebugger = appState.armDebugger;
	 switch (wMessage)       /* process messages */
    {
        case WM_INITDIALOG:
			appState.hRegistersDlg = hRegistersDlg;
			ShowWindow(hRegistersDlg, SW_SHOW);
			
		
			//If we have it set to view banked registers,
			if (appState.settings.debugSettings.viewBanked == BST_CHECKED) {
				CheckDlgButton(appState.hRegistersDlg, IDC_VIEWBANKED, BST_CHECKED);	//check box
				//If we have it set to view banked registers,
				SetWindowPos( appState.hRegistersDlg, NULL, appState.settings.regPos.x, appState.settings.regPos.y, 
					460, 380, SWP_NOZORDER );
	
			} else {
				CheckDlgButton(appState.hRegistersDlg, IDC_VIEWBANKED, BST_UNCHECKED);	//check box
				SetWindowPos( appState.hRegistersDlg, NULL, appState.settings.regPos.x, appState.settings.regPos.y, 
					148, 380, SWP_NOZORDER );
			}

			RegistersGui_refresh();
			return (TRUE);

		case WM_SYSCOMMAND:	/*User clicks on close x button*/
			if (wParam == SC_CLOSE) {
				DestroyWindow(hRegistersDlg);
				appState.hRegistersDlg = NULL;
				return (TRUE) ;
			}
			break;			
		//case WM_MOVE:
		case WM_PAINT:
			RegistersGui_refresh();
			break;

		case WM_MOVE:
			{	RECT rect;
				GetWindowRect(hRegistersDlg, &rect);
				appState.settings.regPos.x = rect.left;//LOWORD(lParam);
				appState.settings.regPos.y = rect.top;//HIWORD(lParam);
			}
			break;
		case WM_COMMAND:    /* controls in dialog box activated */
            			
			switch (LOWORD(wParam))
            {
				case IDC_VIEWBANKED:
					appState.settings.debugSettings.viewBanked = IsDlgButtonChecked(hRegistersDlg, IDC_VIEWBANKED);
					//If we have it set to view banked registers,
					if (appState.settings.debugSettings.viewBanked == BST_CHECKED) {
						CheckDlgButton(appState.hRegistersDlg, IDC_VIEWBANKED, BST_CHECKED);	//check box
						//If we have it set to view banked registers,
						SetWindowPos( appState.hRegistersDlg, NULL, appState.settings.regPos.x, appState.settings.regPos.y, 
							460, 380, SWP_NOZORDER );
			
					} else {
						CheckDlgButton(appState.hRegistersDlg, IDC_VIEWBANKED, BST_UNCHECKED);	//check box
						SetWindowPos( appState.hRegistersDlg, NULL, appState.settings.regPos.x, appState.settings.regPos.y, 
							148, 380, SWP_NOZORDER );
					}
					RegistersGui_refresh();
					break;
				
				
				case IDC_REGREFRESH:	
					RegistersGui_refresh();
					return DefWindowProc (hRegistersDlg, wMessage, wParam, lParam);
					break;
			
				default:
					break;
			}//ENd of switch (LOWORD(wParam);
			break;
		
		}//End of switch(message)
	return (FALSE) ;    /* return FALSE to signify message not processed */
}//procodile dundee

void RegistersDlg_setRegVal(u32 id, u32 val) {
	char text[64];
	sprintf(text, "%08X", val);
	SendDlgItemMessage (appState.hRegistersDlg, id, WM_SETTEXT, (WPARAM)0 , (LPARAM)text);
}

///////////////////////////////////////////////////////
//Refresh the data in the window
////////////////////////////////////////////////////////
void RegistersGui_refresh () {
	HWND hDlg = appState.hRegistersDlg;
	

	

	//char* nullString = "";

	//if (!gba.loaded)
	//	return;

	//This gets the contents of the registers into a list of strings
	//Blah.  Was this really a good idea?
	appState.armDebugger->parseRegisters();
	
		
	//Now update register controls
	for (int i = 0; i < 17; i++) {
		SendDlgItemMessage (hDlg, IDC_R0 + i, WM_SETTEXT, (WPARAM)0 , (LPARAM)appState.armDebugger->regArray[i]);
	}
	
	//Update the CPSR flags
	if (arm.N)
		CheckDlgButton(hDlg, IDC_NCHECK, BST_CHECKED);
	else 
		CheckDlgButton(hDlg, IDC_NCHECK, BST_UNCHECKED);

	if (arm.Z)
		CheckDlgButton(hDlg, IDC_ZCHECK, BST_CHECKED);
	else 
		CheckDlgButton(hDlg, IDC_ZCHECK, BST_UNCHECKED);

	if (arm.C)
		CheckDlgButton(hDlg, IDC_CCHECK, BST_CHECKED);
	else 
		CheckDlgButton(hDlg, IDC_CCHECK, BST_UNCHECKED);

	if (arm.V)
		CheckDlgButton(hDlg, IDC_VCHECK, BST_CHECKED);
	else 
		CheckDlgButton(hDlg, IDC_VCHECK, BST_UNCHECKED);



	if (arm.CPSR & 0x80)
		CheckDlgButton(hDlg, IDC_ICHECK, BST_CHECKED);
	else 
		CheckDlgButton(hDlg, IDC_ICHECK, BST_UNCHECKED);



	if (arm.CPSR & 0x40)
		CheckDlgButton(hDlg, IDC_FCHECK, BST_CHECKED);
	else 
		CheckDlgButton(hDlg, IDC_FCHECK, BST_UNCHECKED);



	if (arm.CPSR & 0x20)
		CheckDlgButton(hDlg, IDC_TCHECK, BST_CHECKED);
	else 
		CheckDlgButton(hDlg, IDC_TCHECK, BST_UNCHECKED);

	//Now, depending on the mode, draw the banked registers (I use a copying rather than a pointer
	//method for them so they don't come from a consistent location)
	if (((arm.CPSR & 0x0F) == 0) || ((arm.CPSR & 0xF) == 0xF)) {	//If it's the current mode, get them from the current regs.
		RegistersDlg_setRegVal(IDC_R8_usr, arm.r[8]);
		RegistersDlg_setRegVal(IDC_R9_usr, arm.r[9]);
		RegistersDlg_setRegVal(IDC_R10_usr, arm.r[10]);
		RegistersDlg_setRegVal(IDC_R11_usr, arm.r[11]);
		RegistersDlg_setRegVal(IDC_R12_usr, arm.r[12]);
		RegistersDlg_setRegVal(IDC_R13_usr, arm.r[13]);
		RegistersDlg_setRegVal(IDC_R14_usr, arm.r[14]);
	} else {	//Otherwise we get it from banked regs
		RegistersDlg_setRegVal(IDC_R8_usr, *arm.r_usrPtr[8]);
		RegistersDlg_setRegVal(IDC_R9_usr, *arm.r_usrPtr[9]);
		RegistersDlg_setRegVal(IDC_R10_usr, *arm.r_usrPtr[10]);
		RegistersDlg_setRegVal(IDC_R11_usr, *arm.r_usrPtr[11]);
		RegistersDlg_setRegVal(IDC_R12_usr, *arm.r_usrPtr[12]);
		RegistersDlg_setRegVal(IDC_R13_usr, *arm.r_usrPtr[13]);
		RegistersDlg_setRegVal(IDC_R14_usr, *arm.r_usrPtr[14]);
	}

	if ((arm.CPSR & 0x0F) == 1) {	//If it's the current mode, get them from the current regs.
		RegistersDlg_setRegVal(IDC_R8_fiq, arm.r[8]);
		RegistersDlg_setRegVal(IDC_R9_fiq, arm.r[9]);
		RegistersDlg_setRegVal(IDC_R10_fiq, arm.r[10]);
		RegistersDlg_setRegVal(IDC_R11_fiq, arm.r[11]);
		RegistersDlg_setRegVal(IDC_R12_fiq, arm.r[12]);
		RegistersDlg_setRegVal(IDC_R13_fiq, arm.r[13]);
		RegistersDlg_setRegVal(IDC_R14_fiq, arm.r[14]);
	} else {	//Otherwise we get it from banked regs
		RegistersDlg_setRegVal(IDC_R8_fiq, *arm.r_fiqPtr[8]);
		RegistersDlg_setRegVal(IDC_R9_fiq, *arm.r_fiqPtr[9]);
		RegistersDlg_setRegVal(IDC_R10_fiq, *arm.r_fiqPtr[10]);
		RegistersDlg_setRegVal(IDC_R11_fiq, *arm.r_fiqPtr[11]);
		RegistersDlg_setRegVal(IDC_R12_fiq, *arm.r_fiqPtr[12]);
		RegistersDlg_setRegVal(IDC_R13_fiq, *arm.r_fiqPtr[13]);
		RegistersDlg_setRegVal(IDC_R14_fiq, *arm.r_fiqPtr[14]);
	}
	RegistersDlg_setRegVal(IDC_R16_fiq, arm.SPSR_fiq);
	
	if ((arm.CPSR & 0x0F) == 2) {	//If it's the current mode, get them from the current regs.
		RegistersDlg_setRegVal(IDC_R13_irq, arm.r[13]);
		RegistersDlg_setRegVal(IDC_R14_irq, arm.r[14]);
	} else {	//Otherwise we get it from banked regs
		RegistersDlg_setRegVal(IDC_R13_irq, *arm.r_irqPtr[13]);
		RegistersDlg_setRegVal(IDC_R14_irq, *arm.r_irqPtr[14]);
	}
	RegistersDlg_setRegVal(IDC_R16_irq, arm.SPSR_irq);

	if ((arm.CPSR & 0x0F) == 3) {	//If it's the current mode, get them from the current regs.
		RegistersDlg_setRegVal(IDC_R13_svc, arm.r[13]);
		RegistersDlg_setRegVal(IDC_R14_svc, arm.r[14]);
	} else {	//Otherwise we get it from banked regs
		RegistersDlg_setRegVal(IDC_R13_svc, *arm.r_svcPtr[13]);
		RegistersDlg_setRegVal(IDC_R14_svc, *arm.r_svcPtr[14]);
	}
	RegistersDlg_setRegVal(IDC_R16_svc, arm.SPSR_svc);


	if ((arm.CPSR & 0x0F) == 7) {	//If it's the current mode, get them from the current regs.
		RegistersDlg_setRegVal(IDC_R13_abt, arm.r[13]);
		RegistersDlg_setRegVal(IDC_R14_abt, arm.r[14]);
	} else {	//Otherwise we get it from banked regs
		RegistersDlg_setRegVal(IDC_R13_abt, *arm.r_abtPtr[13]);
		RegistersDlg_setRegVal(IDC_R14_abt, *arm.r_abtPtr[14]);
	}
	RegistersDlg_setRegVal(IDC_R16_abt, arm.SPSR_abt);

	if ((arm.CPSR & 0x0F) == 0xD) {	//If it's the current mode, get them from the current regs.
		RegistersDlg_setRegVal(IDC_R13_und, arm.r[13]);
		RegistersDlg_setRegVal(IDC_R14_und, arm.r[14]);
	} else {	//Otherwise we get it from banked regs
		RegistersDlg_setRegVal(IDC_R13_und, *arm.r_undPtr[13]);
		RegistersDlg_setRegVal(IDC_R14_und, *arm.r_undPtr[14]);
	}
	RegistersDlg_setRegVal(IDC_R16_und, arm.SPSR_und);

	//This logic is redundant, but I don't feel like doing the typing required to remove it.
	//Display the current mode
	switch (arm.CPSR & 0x0F) {
		case 0x0:	SendDlgItemMessage (hDlg, IDC_DEBUGMODE, WM_SETTEXT, (WPARAM)0 , (LPARAM)"User");
			break;
		case 0x1:	SendDlgItemMessage (hDlg, IDC_DEBUGMODE, WM_SETTEXT, (WPARAM)0 , (LPARAM)"FIQ");
			break;
		case 0x2:	SendDlgItemMessage (hDlg, IDC_DEBUGMODE, WM_SETTEXT, (WPARAM)0 , (LPARAM)"IRQ");
			break;
		case 0x3:	SendDlgItemMessage (hDlg, IDC_DEBUGMODE, WM_SETTEXT, (WPARAM)0 , (LPARAM)"Super");
			break;
		case 0x7:	SendDlgItemMessage (hDlg, IDC_DEBUGMODE, WM_SETTEXT, (WPARAM)0 , (LPARAM)"Abort");
			break;
		case 0xB:	SendDlgItemMessage (hDlg, IDC_DEBUGMODE, WM_SETTEXT, (WPARAM)0 , (LPARAM)"Undef.");
			break;
		case 0xF:	SendDlgItemMessage (hDlg, IDC_DEBUGMODE, WM_SETTEXT, (WPARAM)0 , (LPARAM)"System");
			break;
		default:	SendDlgItemMessage (hDlg, IDC_DEBUGMODE, WM_SETTEXT, (WPARAM)0 , (LPARAM)"Invalid");
			break;

	}

	
	//Force windows to redraw the dialog (and the list boxes, which have since changed).
	//ivalidateRect(appState.hDlg, NULL, FALSE);

	//if (sourceGui != NULL)
	//	SourceGui_refres


}//refresh

