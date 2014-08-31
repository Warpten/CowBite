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
#include <richedit.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
//#include "shlwapi.h"
//#include "commctrl.h"
#include "Gui.h"
#include "resource.h"

#include "Constants.h"
#include "WindowsInterface.h"
#include "GBADebugger.h"

extern State appState;



/////////////////////////////////////////////////////////////////////////////////////////
//HardwareRegGui_Proc
//The windows callback for hardware info dialog.
//Fills the windows with info about the hardware.
////////////////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK HardwareRegGui_Proc (HWND hHardwareRegDlg, UINT wMessage, 
                                                        WPARAM wParam, LPARAM lParam)
{	
	//static int avoidRegUpdate;
	NMHDR *nmhdr;
	MSGFILTER *msgFilter;
	CHARRANGE charRange;	//Used for finding the current selection in the edit control
	POINTL pointl;
	char text[255];
	char* nullString = "";
	u32 index;
	u32 offset;
	switch (wMessage)       /* process messages */
    {
        case WM_INITDIALOG:
//			appState.avoidRegUpdate = 0;
			//LoadLibrary(TEXT("comctl32.dll"));
			//LoadLibrary(TEXT("riched32.dll"));
			appState.hHardwareRegDlg = hHardwareRegDlg;
			
			//Initialize our static array of regIDs.
			appState.regIds[0].bits = IDC_BITS1;	appState.regIds[1].bits = IDC_BITS2;
			appState.regIds[2].bits = IDC_BITS3;	appState.regIds[3].bits = IDC_BITS4;
			appState.regIds[4].bits = IDC_BITS5;	appState.regIds[5].bits = IDC_BITS6;
			appState.regIds[6].bits = IDC_BITS7;	appState.regIds[7].bits = IDC_BITS8;

			appState.regIds[0].hex = IDC_HEXVAL1;	appState.regIds[1].hex = IDC_HEXVAL2;
			appState.regIds[2].hex = IDC_HEXVAL3;	appState.regIds[3].hex = IDC_HEXVAL4;
			appState.regIds[4].hex = IDC_HEXVAL5;	appState.regIds[5].hex = IDC_HEXVAL6;
			appState.regIds[6].hex = IDC_HEXVAL7;	appState.regIds[7].hex = IDC_HEXVAL8;

			appState.regIds[0].bin = IDC_BINVAL1;	appState.regIds[1].bin = IDC_BINVAL2;
			appState.regIds[2].bin = IDC_BINVAL3;	appState.regIds[3].bin = IDC_BINVAL4;
			appState.regIds[4].bin = IDC_BINVAL5;	appState.regIds[5].bin = IDC_BINVAL6;
			appState.regIds[6].bin = IDC_BINVAL7;	appState.regIds[7].bin = IDC_BINVAL8;

			appState.regIds[0].name = IDC_NAMEVAL1;	appState.regIds[1].name = IDC_NAMEVAL2;
			appState.regIds[2].name = IDC_NAMEVAL3;	appState.regIds[3].name = IDC_NAMEVAL4;
			appState.regIds[4].name = IDC_NAMEVAL5;	appState.regIds[5].name = IDC_NAMEVAL6;
			appState.regIds[6].name = IDC_NAMEVAL7;	appState.regIds[7].name = IDC_NAMEVAL8;


			//Tell the rich edit control to send us notification messages.
			SendDlgItemMessage (hHardwareRegDlg, IDC_REGBINVALUE, EM_SETEVENTMASK, (WPARAM)0 , (LPARAM)(ENM_KEYEVENTS | ENM_MOUSEEVENTS));
			SendDlgItemMessage (hHardwareRegDlg, IDC_REGHEXVALUE, EM_SETEVENTMASK, (WPARAM)0 , (LPARAM)(ENM_KEYEVENTS));
			{
				for (int i = 0; i < 8; i++) {
					SendDlgItemMessage (hHardwareRegDlg, appState.regIds[i].hex, EM_SETLIMITTEXT, (WPARAM)4 , (LPARAM)0);
					SendDlgItemMessage (hHardwareRegDlg, appState.regIds[i].hex, EM_SETEVENTMASK, (WPARAM)0 , (LPARAM)(ENM_KEYEVENTS));
					SendDlgItemMessage (hHardwareRegDlg, appState.regIds[i].bin, EM_SETEVENTMASK, (WPARAM)0 , (LPARAM)(ENM_KEYEVENTS | ENM_MOUSEEVENTS));
					SendDlgItemMessage (hHardwareRegDlg, appState.regIds[i].bin, EM_SETLIMITTEXT, (WPARAM)0 , (LPARAM)0);
					SendDlgItemMessage (hHardwareRegDlg, appState.regIds[i].bin, WM_SETTEXT, (WPARAM)0 , (LPARAM)"");
				}
			}
			
			//Set the text limit and initial value for the primary register viewer thingy
			SendDlgItemMessage (hHardwareRegDlg, IDC_REGBINVALUE, EM_SETLIMITTEXT, (WPARAM)16 , (LPARAM)0);
			SendDlgItemMessage (hHardwareRegDlg, IDC_REGBINVALUE, WM_SETTEXT, (WPARAM)0 , (LPARAM)"0000000000000000");
			SendDlgItemMessage (hHardwareRegDlg, IDC_REGHEXVALUE, EM_SETLIMITTEXT, (WPARAM)4 , (LPARAM)0);
			SendDlgItemMessage (hHardwareRegDlg, IDC_REGADDRESS, EM_SETLIMITTEXT, (WPARAM)3 , (LPARAM)0);

			//Set the scroll range for the scroll bars
			//SendDlgItemMessage (hHardwareRegDlg, IDC_REGSCROLL, SBM_SETRANGE, (WPARAM)0, (LPARAM)NUMHARDWAREREGS-16);
			SendDlgItemMessage (hHardwareRegDlg, IDC_BITSCROLL, SBM_SETRANGE, (WPARAM)0, (LPARAM)8);
			//SendDlgItemMessage (hHardwareRegDlg, IDC_REGSCROLL, SBM_SETPOS, (WPARAM)0, (LPARAM)TRUE);

			gbaDebugger.currentReg = *GBADebugger_getRegEntry((int)0);

/*			INITCOMMONCONTROLSEX commonControls;
			commonControls.dwSize = sizeof(INITCOMMONCONTROLSEX);
			commonControls.dwICC = ICC_TAB_CLASSES ;
			InitCommonControlsEx(&commonControls);
*/			
			//Add the tabs.
/*			memset (&tab1Item, 0, sizeof(TCITEM));
			tab1Item.mask = TCIF_TEXT;
			tab1Item.pszText = "Graphics";
			tab1Item.cchTextMax  = 9;
			SendDlgItemMessage (hHardwareRegDlg, IDC_HARDWARETAB, TCM_INSERTITEM, (WPARAM)0 , (LPARAM)&tab1Item);
			tab1Item.pszText = "Registers";
			tab1Item.cchTextMax  = 10;
			SendDlgItemMessage (hHardwareRegDlg, IDC_HARDWARETAB, TCM_INSERTITEM, (WPARAM)1 , (LPARAM)&tab1Item);
			tab1Item.pszText = "Statistics";
			tab1Item.cchTextMax  = 11;
			SendDlgItemMessage (hHardwareRegDlg, IDC_HARDWARETAB, TCM_INSERTITEM, (WPARAM)2 , (LPARAM)&tab1Item);
*/
		
			HardwareRegGui_refresh();
			SetWindowPos( hHardwareRegDlg, NULL, appState.settings.hardwareRegPos.x,
				appState.settings.hardwareRegPos.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER );

			//hDC = GetDC(hDlg);
			ShowWindow(hHardwareRegDlg, SW_SHOW);
			return (TRUE);
		case WM_MOVE:
			{	RECT rect;
				GetWindowRect(hHardwareRegDlg, &rect);
				appState.settings.hardwareRegPos.x = rect.left;//LOWORD(lParam);
				appState.settings.hardwareRegPos.y = rect.top;//HIWORD(lParam);
			}
			break;

		case WM_SYSCOMMAND:	/*User clicks on close x button*/
			if (wParam == SC_CLOSE) {
				DestroyWindow(hHardwareRegDlg);
				return (TRUE);
				
			}
			break;			

      case WM_COMMAND:    /* controls in dialog box activated */
            			
			switch (LOWORD(wParam))
			{
				case IDC_REGBINVALUE:
						SendDlgItemMessage (hHardwareRegDlg, IDC_REGBINVALUE, EM_EXGETSEL, (WPARAM)0 , (LPARAM)&charRange);
						return (TRUE) ;
						break;
				case IDHARDWAREAPPLY:
						*((u16*)(&(gbaMem.u8IORAM[gbaDebugger.currentReg.offset]))) = gbaDebugger.currentReg.value;
						
				case IDHARDWAREREFRESH:
						HardwareRegGui_refresh();
						return (TRUE) ;
						break;
				case IDHARDWARECLOSE:
						DestroyWindow(hHardwareRegDlg);
						return (TRUE) ;
						break;

		
				default:
				//	return (FALSE);	//Otherwise we get a stack overflow.
					break;
            }//ENd of switch (LOWORD(wParam);

			switch (HIWORD(wParam)) {
				case LBN_SELCHANGE:
					//Current selection is the selected index of the list box plus
					index = SendDlgItemMessage (appState.hHardwareRegDlg, IDC_REGLIST, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
					//Get the text.
					SendDlgItemMessage (appState.hHardwareRegDlg, IDC_REGLIST, LB_GETTEXT, (WPARAM)index, (LPARAM)text);
					text[8] = '\0';	//Chop off the address by adding a null terminator
					offset = strtoul(text, &((char*)nullString), 16);	//Get the address
					offset = offset & 0x00FFFFFF;

					//index += SendDlgItemMessage (appState.hHardwareRegDlg, IDC_REGSCROLL, SBM_GETPOS, (WPARAM)0, (LPARAM)0);
					//Get most data from our register list
					memcpy(&gbaDebugger.currentReg, GBADebugger_getRegEntry(offset), sizeof(RegEntry));
					//but get the value from the actual IO_RAM
					gbaDebugger.currentReg.value = *((u16*)(&(gbaMem.u8IORAM[gbaDebugger.currentReg.offset])));
					
					HardwareRegGui_refresh();
					return (TRUE);
					break;
				default:
					return (FALSE);
					break;
			}

			return (TRUE);
			break;

		///////////////////////////////////////////////////////
		//The user clicked on the scroll bar for the memory
		//////////////////////////////////////////////////////
		case WM_VSCROLL:    /* scroll bar was activated */
			
			//Find out if the message came from the reg scroll bar or the
			//bits scroll bar.
			if (GetDlgCtrlID((HWND)lParam) == IDC_BITSCROLL) {
				HardwareRegGui_bitScroll(wParam, lParam);
			}
		return (TRUE);
		break;// End of case (WM_HSCROLL)


		///////////////////////////////////////////////
		//The user presses a key in the rich edit control.
		//Since the edit controls have additional functionality not found in standard
		//windows controls, this process is kind of complex, or at least long.
		///////////////////////////////////////////////////////
		case WM_NOTIFY:
			nmhdr = ((LPNMHDR)lParam);
			
			if (nmhdr->code == EN_MSGFILTER) {
				u32 code;
				u32 editId = wParam;
				u32 maxWidth;
				u32 updateReg = 0;
				u32 base = 2;
				//Note that the MSGFILTER is a larger structure containging 
				//NMHDR
				msgFilter = (MSGFILTER*)nmhdr;
				code = msgFilter->wParam;
				
				switch (editId) {
					case IDC_REGHEXVALUE:
					case IDC_HEXVAL1:
					case IDC_HEXVAL2:	
					case IDC_HEXVAL3:
					case IDC_HEXVAL4:
					case IDC_HEXVAL5:
					case IDC_HEXVAL6:
					case IDC_HEXVAL7:
					case IDC_HEXVAL8:
						base = 16;
					
					case IDC_BINVAL1:
					case IDC_BINVAL2:
					case IDC_BINVAL3:
					case IDC_BINVAL4:
					case IDC_BINVAL5:
					case IDC_BINVAL6:
					case IDC_BINVAL7:
					case IDC_BINVAL8:
					case IDC_REGBINVALUE:
						

						//Get the width
						maxWidth = SendDlgItemMessage (hHardwareRegDlg, editId, EM_GETLIMITTEXT, (WPARAM)0 , (LPARAM)0);
						switch (msgFilter->msg) {
							
							case WM_KEYDOWN:
								//Get the current selection.
								SendDlgItemMessage (hHardwareRegDlg, editId, EM_EXGETSEL, (WPARAM)0 , (LPARAM)&charRange);
								if (charRange.cpMax > maxWidth)	//For some reason it lets you select beyond range
									charRange.cpMax = maxWidth;		//This corrects it
								
								if ((code == VK_DELETE) ||(code == VK_BACK)){
									
									SendDlgItemMessage (hHardwareRegDlg, editId, WM_GETTEXT, (WPARAM)(255) , (LPARAM)text);
									//If only deleting a single character
									if (charRange.cpMin == charRange.cpMax) {
										//Different behavior for backspace
										if (code == VK_BACK) {
											charRange.cpMin = charRange.cpMax = __max (charRange.cpMin-1,0);
										} 
										text[charRange.cpMin] = '0';
										
									}
									else for (int i = charRange.cpMin; i < charRange.cpMax; i++)
										text[i] = '0';

									SendDlgItemMessage (hHardwareRegDlg, editId, WM_SETTEXT, (WPARAM)0 , (LPARAM)text);
									
									//if (editId != IDC_REGBINVALUE)
									HardwareRegGui_updateRegField(editId);
									
									//SETTEXT resets the cursor.  set it back with this.
									SendDlgItemMessage (hHardwareRegDlg, editId, EM_EXSETSEL, (WPARAM)0, (LPARAM)&charRange);
									
									msgFilter->wParam = 0;
								//If it is is base 2 AND 0 or 1, or if it's base 16 AND 0 - F
								} else 	if ( ((base == 2) && ((code == '0') || (code == '1')) )
											|| (  ( (base == 16) && ( ((code >= '0') && (code <= '9'))  
																  ||  ((code >= 'A') && (code <= 'Z')) )  ))){
									SendDlgItemMessage (hHardwareRegDlg, editId, WM_GETTEXT, (WPARAM)(255) , (LPARAM)text);
									//msgFilter->wParam = '1';
									//If the selection is zero, inser character and move to the left.
									if (charRange.cpMin == charRange.cpMax) 
										charRange.cpMin = __max(charRange.cpMin-1,0);
										
									for (int i = charRange.cpMin; i < charRange.cpMax; i++)
										text[i] = code;
									SendDlgItemMessage (hHardwareRegDlg, editId, WM_SETTEXT, (WPARAM)0 , (LPARAM)text);
								
									//if (editId != IDC_REGBINVALUE)
									HardwareRegGui_updateRegField(editId);

									//SETTEXT resets the cursor.  set it back with this.  
									charRange.cpMax = charRange.cpMin;
									SendDlgItemMessage (hHardwareRegDlg, editId, EM_EXSETSEL, (WPARAM)0, (LPARAM)&charRange);
									//Tell it not to do the default character processing.
										
									 msgFilter->wParam = 0;
								} else if (code == VK_LEFT || code == VK_RIGHT) {
									//Do nothing.
								} else {
									//Otherwise the user entered an invalid number.  We went to set the range to nothing.
									SendDlgItemMessage (hHardwareRegDlg, editId, EM_EXGETSEL, (WPARAM)0 , (LPARAM)&charRange);
									charRange.cpMax = charRange.cpMin;
									SendDlgItemMessage (hHardwareRegDlg, editId, EM_EXSETSEL, (WPARAM)0, (LPARAM)&charRange);
									msgFilter->wParam=0;

								}
								//if (editId != IDC_REGBINVALUE)
								//	HardwareRegGui_updateRegField(editId);
								
								break;

							case WM_RBUTTONDOWN:
							case WM_LBUTTONDOWN:
								{
									pointl.x = LOWORD(msgFilter->lParam);
									pointl.y = HIWORD(msgFilter->lParam);
									int index = LOWORD(SendDlgItemMessage (hHardwareRegDlg, editId, EM_CHARFROMPOS, (WPARAM)0 , (LPARAM)&pointl));
								}		
								
								break;

						}//switch
						break;
				}//switch
				
				
				return (TRUE);


			} /*else if (nmhdr->code == EN_CHANGE) {
				SendDlgItemMessage (hHardwareRegDlg, IDC_REGBINVALUE, EM_EXGETSEL, (WPARAM)0 , (LPARAM)&charRange);		
				return (TRUE);
			}*/

			
			
			break;
		case WM_DESTROY:
				appState.hHardwareRegDlg = NULL;
				//return (TRUE);
				break;
		
	}//End of switch(message)
	return (FALSE) ;    /* return FALSE to signify message not processed */
}



///////////////////////////////////
//refresh()
//Updates the information displayed.
//Does not modify any internal GBA data.
//Will reset the contents of the reg displayed
//to the value currently in memory
///////////////////////////////////
void HardwareRegGui_refresh() {
	int listSel, topIndex;

	//DOn't let windows redraw it until we're done.
	SendDlgItemMessage (appState.hHardwareRegDlg, IDC_REGLIST, WM_SETREDRAW, 
		FALSE, 0);  

	//Get the current list box status so that we can set it back later.
	listSel = SendDlgItemMessage (appState.hHardwareRegDlg, IDC_REGLIST, LB_GETCURSEL, 0, 0);    
	topIndex = SendDlgItemMessage (appState.hHardwareRegDlg, IDC_REGLIST, LB_GETTOPINDEX, 0, 0);

	//Clear out the list boxes so that they can be refilled
	SendDlgItemMessage (appState.hHardwareRegDlg, IDC_REGLIST, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);

	
	//Loop through entries in the register list, print them
	for (int i = 0; i < NUMHARDWAREREGS; i++) {
		
		//Input a string containing the register info
		SendDlgItemMessage (appState.hHardwareRegDlg, IDC_REGLIST, LB_ADDSTRING, (WPARAM)0, (LPARAM)GBADebugger_regToString(i));
	}


	//Resets the data in the current reg!
	gbaDebugger.currentReg.value = *((u16*)(&(gbaMem.u8IORAM[gbaDebugger.currentReg.offset])));	
	HardwareRegGui_refreshCurrentReg();

	//Put the list back where it was before we redrew it
	SendDlgItemMessage (appState.hHardwareRegDlg, IDC_REGLIST, LB_SETCURSEL, 
		listSel, 0);    
	SendDlgItemMessage (appState.hHardwareRegDlg, IDC_REGLIST, LB_SETTOPINDEX, 
		topIndex, 0);    

	//After all these changes, have windows redraw the stuff.
	SendDlgItemMessage (appState.hHardwareRegDlg, IDC_REGLIST, WM_SETREDRAW, 
		TRUE, 0);  
	InvalidateRect(appState.hHardwareRegDlg, NULL, FALSE);

	
}

//////////////////////////////////////////////////////
//HardwareRegGui_refreshCurrentReg
//Display contents of current register
//Does not reset itself
//////////////////////////////////////////////////////
void HardwareRegGui_refreshCurrentReg () {
	char bitRange[255];
	char binVal[33];
	char hexVal[9];
	char offsetVal[9];
	u32 fieldVal;
	int fieldSize;	//Size of the range of bits.
	char* bitName;

	//Find out the current scroll bar position
	u32 bitIndex = SendDlgItemMessage (appState.hHardwareRegDlg, IDC_BITSCROLL, SBM_GETPOS, (WPARAM)0, (LPARAM)0);

	//Print the name of the register
	SendDlgItemMessage (appState.hHardwareRegDlg, IDC_REGNAME, WM_SETTEXT, (WPARAM)0, (LPARAM)gbaDebugger.currentReg.name);

	strcpy(binVal, "");
	u32 bitMask = 0x1;
	
	//Print out the value of this register in hex
	//Print out the binary value of this field.
	for (int l = 0; l < 16; l++) {
		if (gbaDebugger.currentReg.value & bitMask)
			strcat(binVal, "1");
		else
			strcat(binVal, "0");
		bitMask = bitMask << 1;
	}
	strrev(binVal);

	//Get the hex value of this register.
	sprintf(hexVal, "%04X", gbaDebugger.currentReg.value);
	sprintf(offsetVal, "%03X", gbaDebugger.currentReg.offset);

	SendDlgItemMessage (appState.hHardwareRegDlg, IDC_REGBINVALUE, WM_SETTEXT, (WPARAM)0, (LPARAM)binVal);
	
//	if (!appState.avoidRegUpdate) {	//If we don't want recursive errors
		SendDlgItemMessage (appState.hHardwareRegDlg, IDC_REGHEXVALUE, WM_SETTEXT, (WPARAM)0, (LPARAM)hexVal);
//	} else appState.avoidRegUpdate = 0;

	SendDlgItemMessage (appState.hHardwareRegDlg, IDC_REGADDRESS, WM_SETTEXT, (WPARAM)0, (LPARAM)offsetVal);
			


	//Get the entry list for the currently selected register
	//Loop through entries in the bit list of each register
	//Skip the first one, which is the name and address of the register
	u32 numEntries = gbaDebugger.currentReg.numEntries;
	for (int j = 0; j < 8; j++) {
		if (bitIndex < numEntries) {	//If we're within range...
			EnableWindow( GetDlgItem (appState.hHardwareRegDlg, appState.regIds[j].bin), TRUE);
			EnableWindow( GetDlgItem (appState.hHardwareRegDlg, appState.regIds[j].hex), TRUE);
			
			//Format the bit range into something we can understand
			int begin = -1;
			int end = -1;
			bitMask = gbaDebugger.currentReg.bitEntries[bitIndex].mask;
			
			for (int k = 0; k < 17; k++) {
				
				if (end == -1) {	//If we're not already at the end
					//See if bit 0 is 1
					if ( bitMask & 0x1) {
						if (begin == -1)
							begin = k;	//We found the beginning bit
					} else if (begin != -1) {	//If we have already past the beginning, then we're at the end
						end = k-1;	
					}
				}
				bitMask = bitMask >> 1;
			}

			if (begin == end) {
				sprintf(bitRange, "%X", begin);
			} else {
				sprintf(bitRange, "%X-%X", begin, end);
			}

			//Size of the field
			fieldSize = end - begin + 1; 
			u32 hexFieldSize = ceil ( (double)(fieldSize/(double)4));
			fieldVal = (gbaDebugger.currentReg.value & gbaDebugger.currentReg.bitEntries[bitIndex].mask) >> begin;
			switch(hexFieldSize) {
				case 1: sprintf(hexVal, "%01X", fieldVal);
					break;
				case 2:	sprintf(hexVal, "%02X", fieldVal);
					break;
				case 3:	sprintf(hexVal, "%03X", fieldVal);
					break;
				default:
					sprintf(hexVal, "%04X", fieldVal);
					break;
			}
			bitMask = 1;
			strcpy(binVal,"");
			
			//Print out the binary value of this field.
			for (int l = 0; l < fieldSize; l++) {
				if (fieldVal & bitMask)
					strcat(binVal, "1");
				else
					strcat(binVal, "0");
				bitMask = bitMask << 1;
			}
			strrev(binVal);

			bitName = gbaDebugger.currentReg.bitEntries[bitIndex].name;
			SendDlgItemMessage (appState.hHardwareRegDlg, appState.regIds[j].bits, WM_SETTEXT, (WPARAM)0, (LPARAM)bitRange);
			SendDlgItemMessage (appState.hHardwareRegDlg, appState.regIds[j].bin, EM_SETLIMITTEXT, (WPARAM)fieldSize, (LPARAM)0);
			//if (!appState.avoidRegUpdate ) {
				SendDlgItemMessage (appState.hHardwareRegDlg, appState.regIds[j].hex, EM_SETLIMITTEXT, (WPARAM)hexFieldSize , (LPARAM)0);
				SendDlgItemMessage (appState.hHardwareRegDlg, appState.regIds[j].hex, WM_SETTEXT, (WPARAM)0, (LPARAM)hexVal);
			//} 
			
			SendDlgItemMessage (appState.hHardwareRegDlg, appState.regIds[j].bin, WM_SETTEXT, (WPARAM)0, (LPARAM)binVal);
			
			SendDlgItemMessage (appState.hHardwareRegDlg, appState.regIds[j].name, WM_SETTEXT, (WPARAM)0, (LPARAM)bitName);
		} else {
			//Otherwise disable the edit boxes
			SendDlgItemMessage (appState.hHardwareRegDlg, appState.regIds[j].bits, WM_SETTEXT, (WPARAM)0, (LPARAM)"");
			SendDlgItemMessage (appState.hHardwareRegDlg, appState.regIds[j].hex, WM_SETTEXT, (WPARAM)0, (LPARAM)"");
			SendDlgItemMessage (appState.hHardwareRegDlg, appState.regIds[j].bin, WM_SETTEXT, (WPARAM)0, (LPARAM)"");
			SendDlgItemMessage (appState.hHardwareRegDlg, appState.regIds[j].name, WM_SETTEXT, (WPARAM)0, (LPARAM)"");
			
			//EnableWindow( GetDlgItem (hHardwareRegDlg, appState.regIds[j].bits), FALSE);
			EnableWindow( GetDlgItem (appState.hHardwareRegDlg, appState.regIds[j].bin), FALSE);
			EnableWindow( GetDlgItem (appState.hHardwareRegDlg, appState.regIds[j].hex), FALSE);
		}
		bitIndex++;
		//SendDlgItemMessage (appState.hHardwareRegDlg, appState.regIds[j].name, WM_SETTEXT, (WPARAM)(255) name, (LPARAM)"");
	}
	SendDlgItemMessage (appState.hHardwareRegDlg, IDC_REGINFO, WM_SETTEXT, (WPARAM)0, (LPARAM)GBADebugger_getRegInfo());

	//appState.avoidRegUpdate = 0;
}

//////////////////////////////////////////////////////
//HardwareRegGui_updateRegField
//Takes data out of the field input boxes and updates the register.
//Then refreshes.
//////////////////////////////////////////////////////
void HardwareRegGui_updateRegField (u32 controlID) {
	char text[255];
	char *nullString = "";
	u32 value;
	u32 bitMask = 1;
	
	//Found out which of the binary fields this is by taking the control ID
	//and looking it up in reverse
	u32 index ;
	u32 base = 2;
	for (index = 0; (index < 8) && (appState.regIds[index].bin != controlID); index++) {}
		
	//Maybe it was a hex value.
	if (index >= 8)	{
		base = 16;
		for (index = 0; (index < 8) && (appState.regIds[index].hex != controlID); index++) {}
	}
	//Otherwise it was one of the main registers.
	if (index >= 8) {
		if (controlID == IDC_REGBINVALUE) {
			SendDlgItemMessage (appState.hHardwareRegDlg, controlID, WM_GETTEXT, (WPARAM)(255), (LPARAM)text);		
			gbaDebugger.currentReg.value = strtoul(text, &((char*)nullString), 2);
		} else {
			SendDlgItemMessage (appState.hHardwareRegDlg, controlID, WM_GETTEXT, (WPARAM)(255), (LPARAM)text);		
			gbaDebugger.currentReg.value = strtoul(text, &((char*)nullString), 16);
		}
	} else  {	//We're okay.

		//Don't need any bounds checking because if the field wasn't in range,
		//it wouldn't let the user edit it.
		u32 bitIndex = index+SendDlgItemMessage (appState.hHardwareRegDlg, IDC_BITSCROLL, SBM_GETPOS, (WPARAM)0, (LPARAM)0);

		//Convert the text in this field to a number, bitwise and with current register value.
		SendDlgItemMessage (appState.hHardwareRegDlg, controlID, WM_GETTEXT, (WPARAM)(255), (LPARAM)text);		
		value = strtoul(text, &((char*)nullString), base);
		
		while ( !(bitMask & gbaDebugger.currentReg.bitEntries[bitIndex].mask) ) {
			value = value << 1;
			bitMask = bitMask << 1;
		}

		bitMask = gbaDebugger.currentReg.bitEntries[bitIndex].mask;
		//Make sure it's properly masked out.
		value = value & bitMask;

		gbaDebugger.currentReg.value = (gbaDebugger.currentReg.value & (~bitMask)) | value;
	}
	HardwareRegGui_refreshCurrentReg ();
}


 //////////////////////////////////////////////////////
//HardwareRegGui_bitScroll
//Handles scrolling for the bit field scroll bar
///////////////////////////////////////////////////////
void HardwareRegGui_bitScroll(WPARAM wParam, LPARAM lParam) {
	u32 pos = SendDlgItemMessage (appState.hHardwareRegDlg, IDC_BITSCROLL, SBM_GETPOS, (WPARAM)0, (LPARAM)0);
	
	
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
		SendDlgItemMessage (appState.hHardwareRegDlg, IDC_BITSCROLL, SBM_SETPOS, (WPARAM)pos, (LPARAM)TRUE);				
		HardwareRegGui_refresh();
}//regScroll()
