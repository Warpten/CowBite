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
#include <stdio.h>
#include <string.h>
#include <richedit.h>
#include "Gui.h"
#include "SourceGui.h"
#include "resource.h"
#include "Console.h"

#include "Constants.h"
//#include "WindowsInterface.h"
#include "Stabs.h"

extern State appState;

//Used for setting the color of lines in the edit control
typedef struct tLineProperty {
	CHARRANGE range;
	u8 isSource;
} LineProperty;

/////////////////////////////////////////////////////////////////////////////////////////
//SourceGui_Proc
//The windows callback for the source.
////////////////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK SourceGui_Proc (HWND hSourceDlg, UINT wMessage, 
                                                        WPARAM wParam, LPARAM lParam)
{
	RECT rect;
	SourceGui* sourceGui;
	s32 widthDif;
	s32 heightDif;
				
	//u32 address;
	u32 incr;

	//sourceGui = &appState.settings.sourceGuis[0];
	 switch (wMessage)       /* process messages */
    {
        case WM_INITDIALOG:
			sourceGui = (SourceGui*)lParam;	//This case is the only place where we can do this.
			//We want to find the difference between the current rect and what
			//is specified in the resource file.
			GetWindowRect(hSourceDlg, &rect);
			sourceGui->initRect = rect;	//This is what the rect as in the beginning.
			GetWindowRect(GetDlgItem(hSourceDlg, IDC_SOURCELIST), &sourceGui->lbInitRect);
			//sourceGui = SourceGui_findByHwnd(hSourceDlg);
			
			if (appState.settings.sourcePos.cx != 0 && appState.settings.sourcePos.cy != 0) {
			//	sourceGui->lastWidth = appState.settings.sourcePos.cx;
			//	sourceGui->lastHeight = appState.settings.sourcePos.cy;
				widthDif = appState.settings.sourcePos.cx - (rect.right - rect.left);				
				heightDif = appState.settings.sourcePos.cy - (rect.bottom - rect.top);

				SetWindowPos( hSourceDlg, NULL, appState.settings.sourcePos.x + 12,
					appState.settings.sourcePos.y+12, appState.settings.sourcePos.cx, 
					appState.settings.sourcePos.cy, SWP_NOZORDER );

				SetWindowPos( GetDlgItem(hSourceDlg, IDC_SOURCELIST), NULL, 0, 0, 
					(sourceGui->lbInitRect.right - sourceGui->lbInitRect.left) + 1 + widthDif,
					(sourceGui->lbInitRect.bottom - sourceGui->lbInitRect.top) + 1 + heightDif, 
					SWP_NOMOVE | SWP_NOZORDER );
				//Gui_compResize(hSourceDlg, IDC_SOURCELIST, 0, 0, widthDif, heightDif);
			} else {
				//sourceGui->lastWidth = rect.right - rect.left + 1;
				//sourceGui->lastHeight = rect.bottom - rect.top + 1;
				
				SetWindowPos( hSourceDlg, NULL, appState.settings.sourcePos.x + 12,
					appState.settings.sourcePos.y+12, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
			}
			
			SendDlgItemMessage (hSourceDlg, IDC_SOURCELIST, LB_SETHORIZONTALEXTENT, (WPARAM)(1024) , (LPARAM)0);		
			ShowWindow(hSourceDlg, SW_SHOW);
			return (TRUE);
		case WM_MOVE:
			{	RECT rect;
				GetWindowRect(hSourceDlg, &rect);
				appState.settings.sourcePos.x = rect.left;
				appState.settings.sourcePos.y = rect.top;
				//sourceGui = SourceGui_findByHwnd(hSourceDlg);
				//if (sourceGui != NULL) {
				//	sourceGui->pos.x = rect.left;//LOWORD(lParam);
				//	sourceGui->pos.y = rect.top;//HIWORD(lParam);
				//}
			}
			break;
		case WM_SIZE:
				if (wParam == SIZE_RESTORED) {
					sourceGui = SourceGui_findByHwnd(hSourceDlg);
					if (sourceGui != NULL) {
						GetWindowRect(hSourceDlg, &rect);
						widthDif = (rect.right - rect.left) - (sourceGui->initRect.right - sourceGui->initRect.left);
						heightDif = (rect.bottom - rect.top) - (sourceGui->initRect.bottom - sourceGui->initRect.top);
						//widthDif = LOWORD(lParam) - (sourceGui->initRect.right - sourceGui->initRect.left);
						//heightDif = HIWORD(lParam) - (sourceGui->initRect.bottom - sourceGui->initRect.top);
						
						SetWindowPos( GetDlgItem(hSourceDlg, IDC_SOURCELIST), NULL, 0, 0, 
							(sourceGui->lbInitRect.right - sourceGui->lbInitRect.left) + widthDif,
							(sourceGui->lbInitRect.bottom - sourceGui->lbInitRect.top) + heightDif, 
							SWP_NOMOVE | SWP_NOZORDER );
						//GetWindowRect(hSourceDlg, &rect);	
						//Gui_compResize(hSourceDlg, IDC_SOURCELIST, 0, 0, widthDif, heightDif);
						
						appState.settings.sourcePos.cx = LOWORD(lParam);
						appState.settings.sourcePos.cy = HIWORD(lParam);
						
					}
				}
				break;
				/*
		case WM_SIZING: {	//Use this command to resize the list box.
				sourceGui = SourceGui_findByHwnd(hSourceDlg);
				if (sourceGui != NULL) {
					RECT* newRect = (LPRECT)lParam;

					//GetWindowRect(hSourceDlg, &rect);
					//widthDif = (newRect->right - newRect->left) - (rect.right - rect.left);
					//heightDif = (newRect->bottom - newRect->top) - (rect.bottom - rect.top);
					
					//This finds the size change from the original (defined in the resource file)
					widthDif = (newRect->right - newRect->left) - (sourceGui->initRect.right - sourceGui->initRect.left);
					heightDif = (newRect->bottom - newRect->top) - (sourceGui->initRect.bottom - sourceGui->initRect.top);
					
					
					//widthDif = (newRect->right - newRect->left+1) - sourceGui->lastWidth;
					//heightDif = (newRect->bottom - newRect->top+1) - sourceGui->lastHeight;
					//if (widthDif > 50)
					
					//	int blah = 0;

					SetWindowPos( GetDlgItem(hSourceDlg, IDC_SOURCELIST), NULL, 0, 0, 
					sourceGui->lbInitRect.right - sourceGui->lbInitRect.left + 1 + widthDif,
					sourceGui->lbInitRect.bottom - sourceGui->lbInitRect.top + 1 + heightDif, 
					SWP_NOMOVE | SWP_NOZORDER );
					//Gui_compResize(hSourceDlg, IDC_SOURCELIST, 0, 0, widthDif, heightDif);
				

					appState.settings.sourcePos.cx = newRect->right - newRect->left+1;
					appState.settings.sourcePos.cy = newRect->bottom - newRect->top+1;
				}
			}
			break;*/
		case WM_SYSCOMMAND:	/*User clicks on close x button*/
			if (wParam == SC_CLOSE) {
				DestroyWindow(hSourceDlg);
				return (TRUE) ;
				
			}
			break;			

        case WM_COMMAND:    /* controls in dialog box activated */
            			
			switch (LOWORD(wParam))
			{
				case IDSOURCECLOSE:
						DestroyWindow(hSourceDlg);
						break;
				/////////////////////////////////////////////////
				//This toggles whether we are going or stopping

				////////////////////////////////////////////////
				case IDC_SOURCEGO:				//Let it run it's course
					appState.armDebugger->saveUndo();	//Save how we were before we did this.
					appState.windowsInterface->setPaused(0);
					//appState.armDebugger->browsingProgram = 0;
					SourceGui_refresh(SourceGui_findByHwnd(hSourceDlg), 0); 
					//Gui_refreshAllDialogs();	//This rather annoyingly will bring other source windows into focus
					break;
				//This just breaks program execution.
				case IDC_SOURCEBREAK:
					appState.windowsInterface->setPaused(1);
					Gui_refreshAllDialogs(1);
					break;
	
				case IDC_SOURCESTEPINTO:	//Step into
					sourceGui = SourceGui_findByHwnd(hSourceDlg);
					if (sourceGui == NULL)
						int blah = 0;
					SourceGui_stepInto();
					break;
				case IDC_SOURCESTEPOVER:	//Step over
					SourceGui_stepOver();
					break;
				case IDC_SOURCESTEPASM:	//Step through Instructions
					appState.windowsInterface->setPaused(1);
					//appState.armDebugger->browsingProgram = 0;
					appState.armDebugger->saveUndo();	//Save how we were before we did this.
					arm.steps = 1;
					appState.windowsInterface->setPaused(0);
					break;

				case IDC_SOURCEUNDO:	//Undo the last instruction
					appState.armDebugger->undo();
					appState.windowsInterface->setPaused(1);
					break;
				case IDC_SHOWASM:
					SourceGui_refresh(SourceGui_findByHwnd(hSourceDlg), 1);
					break;
				case IDC_SOURCELIST:
					switch (HIWORD(wParam)) {
						case LBN_DBLCLK: 
							{
								//Try to add or remove a breakpoint.
								u32 foundBreakpoint;
								u32 startAsm, endAsm;
								
								u32 selectedIndex = SendDlgItemMessage (hSourceDlg, IDC_SOURCELIST, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
								StabListList* sourceListList;
								StabListList* previous;
								StabSourceFile* sourceFile;
								StabSourceLine* sourceLine;
								StabSourceLine* firstLine;
								u32 address;
								sourceGui = SourceGui_findByHwnd(hSourceDlg);
								if ((sourceGui != NULL) && (selectedIndex != LB_ERR)) {
									sourceFile = sourceGui->sourceFile;
									
									//If we're in "source only" mode, hunt to see
									if (IsDlgButtonChecked(sourceGui->hWnd, IDC_SHOWASM) == BST_UNCHECKED) {
										//Remember that this allocates memory, which we must deallocate.
										//So we have to loop through each list entry and delete it no matter
										//what.  (Even if held at gunpoint!)
										sourceListList = Stabs_findAllByDesc((StabListEntry*)&sourceFile->lineList, selectedIndex+1);										
										foundBreakpoint = 0;
										if (sourceListList != NULL) {
											if (arm.thumbMode)
												incr = 2;
											else
												incr = 4;
											firstLine = (StabSourceLine*)sourceListList->list;
											//Now see if there is a breakpoint
											while (sourceListList != NULL) {
												
												sourceLine = (StabSourceLine*) sourceListList->list;
												startAsm = sourceLine->entry.n_value;
												
												//If there's a source line after this one, use that as the ending asm value
												if (sourceLine->next != NULL)
													endAsm = sourceLine->next->entry.n_value;			
												else
													endAsm = sourceFile->fileEnd;	//Otherwise end before the end of the file.
												
												//This checks to see if there are ANY breakpoints set
												//in the asm for this line, and removes them.		
												for (u32 j = startAsm; (startAsm == endAsm) || (j < endAsm); j+=incr) {
													if (ARM_containsBreakpoint(j)) {
														appState.armDebugger->removeBreakpoint(j);
														foundBreakpoint = 1;
													}
													if (startAsm == endAsm)	//If this is the case where start and end are equel,
														break;					//avoid an infinite loop.
												}

												previous = sourceListList;
												sourceListList = sourceListList->next;
												delete previous;	
											}
											if (!foundBreakpoint) {	//If we didn't find a breakpoint, then set one.
												appState.armDebugger->addBreakpoint(firstLine->entry.n_value);
											}//if 
										}//if (sourceListList != NULL)
									} else {	//If we mix asm with source...
										
										for (int i = 0; i < sourceFile->numAsmLines; i++) {
											if (selectedIndex == sourceFile->asmLines[i].lineNumber) {
												address = sourceFile->asmLines[i].address;
												if (ARM_containsBreakpoint (address))
													appState.armDebugger->removeBreakpoint(address);
												else
													appState.armDebugger->addBreakpoint(address);
											}
										}
									}
									SourceGui_refresh(sourceGui, 0);	//refresh + do not set the cursor.
									DebugGui_refresh();
									
								}
							}

						break;
					}

					break;
				default:
					return (FALSE);	//Otherwise we get a stack overflow.
					break;
            }//ENd of switch (LOWORD(wParam);
			
			return (TRUE);
			break;
	//	case WM_VKEYTOITEM:
	//		if (LOWORD(wParam) == VK_F11)
	//			goto EVIL_STEPINTO;	
	//			break;
	
		case WM_DESTROY:
				for (int i = 0; i < MAX_SOURCEWINDOWS; i++) {
					SourceGui* sourceGui = &appState.settings.sourceGuis[i];
					if (sourceGui->hWnd == hSourceDlg) {
						sourceGui->hWnd = NULL;
						//Free memory allocated for the source file.
						Stabs_freeSource(sourceGui->sourceFile);
						sourceGui->hWnd = NULL;
					}
				}
				
				//return (TRUE);
				break;
		
//		}

	}//End of switch(message)
	return (FALSE) ;    /* return FALSE to signify message not processed */
}

///////////////////////////////////////////////////////
//This function handles stepping into.  It may be called from outside this module.
//////////////////////////////////////////////////////
void SourceGui_stepInto() {
	appState.windowsInterface->setPaused(1);
	//appState.armDebugger->browsingProgram = 0;
	appState.armDebugger->saveUndo();	//Save how we were before we did this.
	arm.stepInto = 1;
	appState.windowsInterface->setPaused(0);
}

///////////////////////////////////////////////////////
//This function handles stepping over.  It may be called from outside this module.
//////////////////////////////////////////////////////
void SourceGui_stepOver() {
//	StabFunction* function;
//	u32 address;
	appState.windowsInterface->setPaused(1);
	//appState.armDebugger->browsingProgram = 0;
	appState.armDebugger->saveUndo();	//Save how we were before we did this.
	
	//Problem:  what do you do when you're at the end of a file??  Just step into?
	//Another problem:  What if the next source line compiles to be a greater address
	//than the current one?  Does this mean we should search by the source line number
	//rather than the address?
	//Also, if we're at an if statement, this will try to go to the line of code inside the
	//statement.  Exept if it isnt' reached, it will just run forever, instead of
	//stopping at a point outside of the if. Suck.
	//Visual C++ will only "step over" a line of it is a call to a function.
	//How does it know??
	
	//Idea:  Have step to the next line it reaches within the current fuction?
	arm.stepAddress = 1;
	arm.stepFunction = Stabs_findFunctionByAddress(&gba.stabs, R15CURRENT);
	
	if (arm.stepFunction != NULL) {
		//If we're on the last line of a function, don't step over, step into.  
		if (R15CURRENT == arm.stepFunction->blockEnd) {
			arm.stepInto = 1;
			//arm.stepFunction = NULL;	//Give it a chance
		}
		//Otherwise, if we're in  a function, GO!
		appState.windowsInterface->setPaused(0);
	}
	//otherwise, stepping has no effect.

	/*
	//if (at a function)
		address = Stabs_findNextSourceAddress(&gba.stabs, R15CURRENT);
		if (address != 0xFFFFFFFF) {
			arm.stepAddress = address;
		} else {
			SourceGui_stepInto();	//Otherwise, just step into.
		}
/*	} else {	//Otherwise treat it like a step into.
		arm.stepInto = 1;
	}*/
	
}

////////////////////////////////////////////////////
//This function refreshes the screen based on a pointer passed in.
//If setCursor is 1, it also scrolls the current index into view.
/////////////////////////////////////
void SourceGui_refresh(SourceGui* sourceGui, u32 setCursor) {
	u32 topIndex, currentSelection;
	u32 numSourceLines, numAsmLines, numLines, lastNumLines;;
	u32 incr;
	char* lineAsm;
	char* lineSource;
	u32 startAsm;
	u32 endAsm;
	u32 currentLinenum = 0;
	u32 showAsm;
	u32 isCurrent;

	//return;
	
	if (sourceGui == NULL)
		return;
	
	StabSourceFile* sourceFile = sourceGui->sourceFile;
	if (sourceFile == NULL) {
		DestroyWindow(sourceGui->hWnd);
		return;
	}
	if (!sourceFile->loaded) {
		DestroyWindow(sourceGui->hWnd);
		return;
	}


	int tabSize = 20;
	isCurrent = ((R15CURRENT >= sourceFile->fileStart) && (R15CURRENT < sourceFile->fileEnd));

	//This here completely nullifies the set cursor parameter.  Should I remove it??
	/*
	if (isCurrent)
		setCursor = 1;
	else
		setCursor = 0;*/

	//First turn off redraw
	SendDlgItemMessage (sourceGui->hWnd, IDC_SOURCELIST, WM_SETREDRAW, 
		FALSE, 0);  

	//Get the current index before clearing it out
	topIndex = SendDlgItemMessage (sourceGui->hWnd, IDC_SOURCELIST, LB_GETTOPINDEX , (WPARAM)0, (LPARAM)0);
	currentSelection = SendDlgItemMessage (sourceGui->hWnd, IDC_SOURCELIST, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
	lastNumLines = SendDlgItemMessage (sourceGui->hWnd, IDC_SOURCELIST, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);

	//Clear out the source box so that it can be refilled.
	//SendDlgItemMessage (sourceGui->hWnd, IDC_SOURCEEDIT, WM_SETTEXT, (WPARAM)0, (LPARAM)"");
	SendDlgItemMessage (sourceGui->hWnd, IDC_SOURCELIST, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
	SendDlgItemMessage (sourceGui->hWnd, IDC_SOURCELIST, LB_SETTABSTOPS, (WPARAM)1, (LPARAM)&tabSize);
	
	
	
	//Allocate an array listing the type and range of each line.
	//LineProperty* lineProperties = new LineProperty[0xFFFF];
	char temp[256];
	
	StabListList* sourceListList;
	StabSourceLine* sourceLine;
	StabSourceLine* nextSourceLine;	//We always need a handle to the next line so we can tell where asm stops
	
	//Get the first source line and the second sourceline.
	//sourceLine = (StabSourceLine*)Stabs_findByDesc((StabListEntry*)&sourceFile->lineList, 1);
	//nextSourceLine = sourceLine->next;
	numSourceLines = sourceFile->numSourceLines;
	numAsmLines = 0;
	numLines = 0;
	
	if (arm.thumbMode)
		incr = 2;
	else
		incr = 4;

	//u32* asmLineList = new u32[ maxLines ];
	showAsm = IsDlgButtonChecked(sourceGui->hWnd, IDC_SHOWASM);
	for (u32 i = 0; i < numSourceLines; i++) {// (fgets(lineSource, 256, fp)) {
		lineSource = sourceFile->lines[i];
		
		//Find all stab source lines that match this one.  NOte that this
		//allocates memory, which we must free as we traverse the listlist.
		sourceListList = Stabs_findAllByDesc((StabListEntry*)&sourceFile->lineList, i+1);
		//strcpy(temp, "");
		//if ((i+1) == 74)
		//	int blah = 0;
		u32 containsBreakpoint;
		u32 foundCurrentLine; 
		//Show only the source lines if this is unchecked.
		if (showAsm== BST_UNCHECKED) {
			containsBreakpoint = 0;
			foundCurrentLine = 0;	
			//If we have source entries, let the user know that this is a breakpoint-worthy sourcel ine.
			if (sourceListList == NULL) {
				sprintf(temp, "  %5d:\t", i+1);	//Print a blank space
				
			//For each source line matching the current line...
			} else {
				while (sourceListList != NULL) {
				
					sourceLine = (StabSourceLine*) sourceListList->list;
					startAsm = sourceLine->entry.n_value;
					
					//If there's a source line after this one, use that as the ending asm value
					if (sourceLine->next != NULL)
						endAsm = sourceLine->next->entry.n_value;			
					else
						endAsm = sourceFile->fileEnd;	//Otherwise end before the end of the file.
					
					//if ((sourceLine->entry.n_desc == 78) )
					//	int blah = 0;
					
					//Now check and see if there is a breakpoint on any of the lines following this
					//source line.
					//Note that i do the "startAsm == endAsm" test for the event that this line
					//has the same address as the line after it.
					for (u32 j = startAsm; (startAsm == endAsm) || (j < endAsm); j+=incr) {
						if (ARM_containsBreakpoint(j)) {
							containsBreakpoint = 1;
						} 
						if (R15CURRENT == j) {
							foundCurrentLine = 1;
							//this will result (I think) in the current line being set to the last
							//source line in the list which matches this address.
							currentLinenum = sourceLine->entry.n_desc;
							//currentLinenum = i+1;

						}
						if (startAsm == endAsm)	//If this is the case where start and end are equel,
							break;					//avoid an infinite loop.
					}
										
						
					StabListList* previous = sourceListList;
					sourceListList = sourceListList->next;
					delete previous;
				}
				//Now do some logic to figure out what to print.  Note redundancy.  I hate coding strings.
				if (foundCurrentLine) 
					if (containsBreakpoint)
						sprintf(temp, ">*%5d:\t", i+1);
					else
						sprintf(temp, ">-%5d:\t", i+1);
				else
					if (containsBreakpoint)
						sprintf(temp, " *%5d:\t", i+1);
					else
						sprintf(temp, " -%5d:\t", i+1);
			}
		
			strncat(temp, lineSource, 220);			//Add at most 220 characters
			SendDlgItemMessage (sourceGui->hWnd, IDC_SOURCELIST, LB_INSERTSTRING, (WPARAM)numLines++, (LPARAM)temp);
		}

		//Show asm if the user has the "show asm" checkbox checked
		else if (IsDlgButtonChecked(sourceGui->hWnd, IDC_SHOWASM) == BST_CHECKED) {
			//If we have source entries, let the user know that this is a breakpoint-worthy sourcel ine.
			if (sourceListList != NULL)
				sprintf(temp, " -%5d:\t", i+1);	//Print a -
			else
				sprintf (temp, "  %5d:\t", i+1);		//Print a blank space.
			strncat(temp, lineSource, 220);			//Add at most 220 characters
			SendDlgItemMessage (sourceGui->hWnd, IDC_SOURCELIST, LB_INSERTSTRING, (WPARAM)numLines++, (LPARAM)temp);
	
			
			
			//Iterate once for each entry in the stabs line lists that
			//matches this line number.
			while (sourceListList != NULL) {
				sourceLine = (StabSourceLine*)sourceListList->list;
				SendDlgItemMessage (sourceGui->hWnd, IDC_SOURCELIST, 
					LB_INSERTSTRING, (WPARAM)numLines++, (LPARAM)"------------------------------");
				
				startAsm = sourceLine->entry.n_value;
				nextSourceLine = sourceLine->next;
				
				if (nextSourceLine != NULL)	{//End before the next line.
					endAsm = nextSourceLine->entry.n_value;
				} else
					endAsm = sourceFile->fileEnd;	//Otherwise end before the end of the file.
				
				

				//while (repeatLines--) {
					u32 j = startAsm;
					/*if (startAsm == endAsm) {	//Sometimes two lines of source get the same address.
						if (arm.thumbMode) {
							lineAsm = appState.armDebugger->disassembleThumb(j);
							sourceFile->asmLines[numAsmLines].address = j;
							j+=2;
						} else {
							lineAsm = appState.armDebugger->disassembleARM(j);
							sourceFile->asmLines[numAsmLines].address = j;
							j+=4;
						}
						
						sourceFile->asmLines[numAsmLines].lineNumber = numLines;
						numAsmLines++;
						SendDlgItemMessage (sourceGui->hWnd, IDC_SOURCELIST, LB_INSERTSTRING, (WPARAM)numLines++, (LPARAM)lineAsm);					
					}*/
					while ( (startAsm == endAsm) || (j < endAsm)) {
						if (arm.thumbMode) {
							lineAsm = appState.armDebugger->disassembleThumb(j);
							sourceFile->asmLines[numAsmLines].address = j;
							j+=2;
						} else {
							lineAsm = appState.armDebugger->disassembleARM(j);
							sourceFile->asmLines[numAsmLines].address = j;
							j+=4;
						}
						sourceFile->asmLines[numAsmLines].lineNumber = numLines;
						numAsmLines++;
						SendDlgItemMessage (sourceGui->hWnd, IDC_SOURCELIST, LB_INSERTSTRING, (WPARAM)numLines++, (LPARAM)lineAsm);
						if (startAsm == endAsm)	//If this is the case where start and end are equel,
							break;					//avoid an infinite loop.
					}
				//	souceLine = sourceLine->next;
				//}
				//strcat(lineAsm, "\n");
				
				StabListList* previous = sourceListList;
				sourceListList = sourceListList->next;
				delete previous;	//Delete entries in the list as we go.
				

				//SourceGui_print(lineAsm);
				SendDlgItemMessage (sourceGui->hWnd, IDC_SOURCELIST, 
					LB_INSERTSTRING, (WPARAM)numLines++, (LPARAM)"------------------------------");
			}//while (sourceLine != NULL)
			
		}//If
	}//While

	//Print an error if we counted more asm than the source file can hold.
	if (numAsmLines >= sourceFile->numAsmLines) {
			MessageBox (sourceGui->hWnd,
        "Tried to print more lines of asm than allocated.",
		"CowBite", MB_ICONEXCLAMATION | MB_OK);
		return;
	}
//	int blah = R15CURRENT;
	
	//If requested, put the cursor on the current instruction.
	if (setCursor) {
		if (currentLinenum) {	//If we set "currentLine", it means we must be in "source only" mode
			SendDlgItemMessage (sourceGui->hWnd, IDC_SOURCELIST, LB_SETCURSEL, 
					currentLinenum-1, 0);    

			SendDlgItemMessage (sourceGui->hWnd, IDC_SOURCELIST, LB_SETTOPINDEX, 
					currentLinenum-13, MAKELPARAM(FALSE, 0));  			

		} else if( isCurrent ) {
				for (int i = 0; i < numAsmLines; i++) {
					if (sourceFile->asmLines[i].address == R15CURRENT) {
						//If we found a match, select the index and scroll the list box
						  

						SendDlgItemMessage (sourceGui->hWnd, IDC_SOURCELIST, LB_SETCURSEL, 
							sourceFile->asmLines[i].lineNumber, 0);    

						SendDlgItemMessage (sourceGui->hWnd, IDC_SOURCELIST, LB_SETTOPINDEX, 
							sourceFile->asmLines[i].lineNumber-12, MAKELPARAM(FALSE, 0));    
						//SendDlgItemMessage (sourceGui->hWnd, IDC_SOURCELIST, LB_SETCARETINDEX, 
						//	sourceFile->asmLines[i].lineNumber, MAKELPARAM(FALSE, 0));    
						
						
					}
				
				}
		}
		
	} else {	//Otherwise keep the cursor where we had it.
			//If we have just changed from source/asm mode,
			//we must figure out what line the last position corresponded to, and put the cursor on the new line.
			//if (showAsm != sourceGui->showAsm) {	//
				
				//This is a hack.  It just puts the cursor at the appropriate fraction of the list box
				//size.  The real way to do it is to
				//When switching from asm:  To search for the index of the current selection and top mode
				//in sourceFile->asmLines.  When found, search for this address in the source lines list
				//(need a Stabs_sourceLineContains() function)
				//When switching from source:  search in the list of source lines for this file until
				//we find which "desc" field is closest to the current index and top val.  Then search through
				//the asmLines until we find one that matches that address.  Set the cursor at that line number.
				numLines = SendDlgItemMessage (sourceGui->hWnd, IDC_SOURCELIST, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);
				float fraction = (float)numLines/(float)lastNumLines;
				currentSelection = (float)currentSelection * fraction;
				topIndex = (float)topIndex * fraction;
				
				//if (showAsm = BST_CHECKED)	//if we switched into asm mode...
				//	currentSelection = currentSelection

			//	sourceGui->showAsm = showAsm;
			//}
			SendDlgItemMessage (sourceGui->hWnd, IDC_SOURCELIST, LB_SETCURSEL, 
					currentSelection, 0);    
			SendDlgItemMessage (sourceGui->hWnd, IDC_SOURCELIST, LB_SETTOPINDEX, 
					topIndex, 0);    
			
	}
	//SendDlgItemMessage (sourceGui->hWnd, IDC_SOURCELIST, LB_SETCOLUMNWIDTH, 100, 0);

	//Finally, set this window to the front if it contains the current address and the
	//user has "auto source" checked.
	if (isCurrent) {
		if (GetMenuState(appState.hDebugSourceMenu, IDM_AUTOSOURCE, MF_BYCOMMAND) & MF_CHECKED)
			SetFocus(sourceGui->hWnd);
	}

	//After all these changes, have windows redraw the stuff.
	SendDlgItemMessage (sourceGui->hWnd, IDC_SOURCELIST, WM_SETREDRAW, 
		TRUE, 0);  
	InvalidateRect(sourceGui->hWnd, NULL, FALSE);

}

/////////////////////////////////////////////////////////////
//loadSource
//This loads the source code for a stab source file entry.
//////////////////////////////////////////////////////////////
StabSourceFile* SourceGui_loadSource(StabSourceFile* sourceFile) {
	FILE *fp;
	char fullPath[_MAX_PATH];
	char *buffer;
	u32 start, end, size, bytesRead, numLines, numSourceLines, numAsmLines;
	
	//return NULL;
	
	//First see if we really have this in our stabs
	//StabSourceFile* sourceFile;
	//sourceFile = (StabSourceFile*)Stabs_findByName((StabListEntry*)&stabs->sourceList, filename);	
	if (sourceFile == NULL)
		return NULL;

	//Make sure that we don't do double allocation.
	//Stabs_freeSource(sourceFile);

	//Now create the full path.  Some Elf files will have an absolute path
	//already specified in them.  In these cases we can just copy the path...
	if ( strncmp(&sourceFile->filename[1], ":\\", 2) == 0) {	//If an absolute windows path
		strcpy(fullPath, sourceFile->filename);
	} else if (strncmp(&sourceFile->filename[1], ":/", 2) == 0) {	//If a path with unix separators
		strcpy (fullPath, sourceFile->filename);
		for (int i = 0; i < strlen(fullPath); i++) {
		//	if (fullPath[i] == '/')
		//		fullPath[i] = '\\';
		}
	} else {						//Otherwise it's a relative path
		strcpy(fullPath, gba.romPath);
		strcat(fullPath, sourceFile->filename);
	}
	sourceFile->fullPath = strdup(fullPath);	//strdup allocates memory

	//Next try to open the file.
	if (!(fp = fopen(fullPath, "rt"))) {
		return NULL;
	}

	fseek (fp, 0, SEEK_SET);
	start = ftell (fp);
	fseek (fp, 0, SEEK_END);
	end = ftell (fp);
	
	size = (end - start);	

	fseek (fp, 0, SEEK_SET);

	buffer = new char[size+1];
	
	//note that the number of bytes read is less than the size because it's text mode.
	//Interesting, eh??
	bytesRead = fread (buffer, 1, size, fp);		//Load the file into this memory
	fclose(fp);
	
	sourceFile->data = buffer;			//Save it in our stab file structure.
	
	numSourceLines = 1;					//Count the number of lines.
	for (int i = 0; i < bytesRead; i++) {
		if (buffer[i] == '\n') {
			//buffer[i] = '\0';
			numSourceLines++;
		}
	}
	
	sourceFile->numSourceLines = numSourceLines;
	sourceFile->lines = new char*[numSourceLines];

	//Allocate enough space for all the asm in this file, taking into account
	//the worst case where every address has duplicate lines
	numAsmLines = ((sourceFile->fileEnd - sourceFile->fileStart) ) + 1;
	sourceFile->asmLines = new StabAsmLine[numAsmLines];
	sourceFile->numAsmLines = numAsmLines;
	
	//char* nullString = "";
	
	//sourceFile->lines[0] = buffer;	//Set the first line to the buffer start.
	for (i = 0; i < numSourceLines; i++) {
		sourceFile->lines[i] = "";
	}
	

	numLines = 0;
	char* lineStart = buffer;
	for (i = 1; i <= bytesRead; i++) {
		if (buffer[i-1] == '\n')	{//If we are at the next string,
			buffer[i-1] = '\0';	//Terminate the string with a 0
			sourceFile->lines[numLines] = lineStart;
			lineStart = &buffer[i];
			numLines++;
		}
	}
	sourceFile->lines[numLines] = lineStart;	//Take care of the very last line.
	//char text[256];
	//sprintf(text, "numLines: %d, numSourceLines: %d", numLines, numSourceLines);
	//Console_print(text);
	//return NULL;
	
	buffer[bytesRead] = '\0';	//Since we made the buffer be 'size +1', this is in bounds.
	//Must prep strtok with an initial call with data as the argument.
	/*
	sourceFile->lines[0]= strtok(buffer, "\n");	//Split into tokens divided by \n
	for ( i = 1; i < numSourceLines; i++) {
		//Get the next token.
		if (sourceFile->lines[i] = strtok(NULL, "\n") == NULL) {
			numSourceLines = i;
			break;
		}
	}*/
	
	
	sourceFile->loaded = 1;
	return sourceFile;

	
	//SendDlgItemMessage (hHardwareRegDlg, editId, EM_EXSETSEL, (WPARAM)0, (LPARAM)&charRange);
	
	//delete lineProperties;

	//fclose(fp);
}

//////////////////////////////////////////
//findByHwnd
//Returns the window that has this source gui.
//////////////////////////////////////////
SourceGui* SourceGui_findByHwnd(HWND hWnd) {
	for (int i = 0; i < MAX_SOURCEWINDOWS; i++) {
		if (appState.settings.sourceGuis[i].hWnd == hWnd) {
			return &appState.settings.sourceGuis[i];
		}
	}
	return NULL;
}

///////////////////////////////////////////////
//Gets a source gui for the given address.  If
//the source file has not been loaded, load it.
////////////////////////////////////////
SourceGui* SourceGui_getByAddress(Stabs* stabs, u32 address) {
	SourceGui* sourceGui;
	//First step is to see if we even have the file.
	StabSourceFile* sourceFile;
	sourceFile = Stabs_findSourceByAddress(stabs, address);
	sourceGui = SourceGui_getWindow(sourceFile);
	/*if (sourceFile != NULL) {
		//If it hasn't been loaded...
		if (!sourceFile->loaded) {
			//Load it.
			SourceGui_loadSource(sourceFile);
			//Find a suitable source window.
			for (int i = 0; i < MAX_SOURCEWINDOWS; i++) {
				sourceGui = &appState.settings.sourceGuis[i];
				if (sourceGui->hWnd == NULL) {
					sourceGui->sourceFile = sourceFile;
					sourceGui->hWnd = CreateDialogParam (appState.hInstance,MAKEINTRESOURCE(IDD_SOURCE),
						appState.hWnd,(DLGPROC)SourceGui_Proc, (LPARAM)sourceGui);
					SourceGui_refresh(sourceGui);	//bring it up to speed.
					return sourceGui;
				}
			}
			//If we reach this statement, it means no windows were available.
			MessageBox (appState.hWnd,
				"Maximum limit of 32 source windows.",
				"CowBite", MB_ICONEXCLAMATION | MB_OK);
			return NULL;
		} else {
			//Otherwise, it is already loaded, and we want to find it in our list..
			//Note that closing a window unloads the source.
			for (int i = 0; i < MAX_SOURCEWINDOWS; i++) {
				sourceGui = &appState.settings.sourceGuis[i];
				if (sourceGui->sourceFile == sourceFile) {
					return sourceGui;
				} 
			}
			//If we reach this, we were unable to find source
			return NULL;
		}
	}//if*/
	return NULL;
}

/////////////////////////////////////////////////////
//get the source gui window associated with this filename.
//If it doesn't exist, load data for the file, try to create the window.
/////////////////////////////////////////////////////
SourceGui* SourceGui_getByFilename(Stabs *stabs, char* filename) {
	SourceGui* sourceGui;
	//First step is to see if we even have the file.
	StabSourceFile* sourceFile;
	sourceFile = (StabSourceFile*)Stabs_findByName((StabListEntry*)(&stabs->sourceList), filename);
	sourceGui = SourceGui_getWindow(sourceFile);
	return sourceGui;
}

/////////////////////////////////////////////////////
//get the source gui window associated with this file.
//If it doesn't exist, try to create it.
/////////////////////////////////////////////////////
SourceGui* SourceGui_getWindow(StabSourceFile* sourceFile) {
	SourceGui* sourceGui;
	if (sourceFile != NULL) {
		//If it hasn't been loaded...
		if (!sourceFile->loaded) {
			//Load it.
			SourceGui_loadSource(sourceFile);
			//Find a suitable source window.
			for (int i = 0; i < MAX_SOURCEWINDOWS; i++) {
				sourceGui = &appState.settings.sourceGuis[i];
				if (sourceGui->hWnd == NULL) {
					sourceGui->sourceFile = sourceFile;
					sourceGui->hWnd = CreateDialogParam (appState.hInstance,MAKEINTRESOURCE(IDD_SOURCE),
						appState.hWnd,(DLGPROC)SourceGui_Proc, (LPARAM)sourceGui);
					//Set the text of the window.
					SetWindowText (sourceGui->hWnd, sourceFile->filename);
					SourceGui_refresh(sourceGui, 1);	//bring it up to speed.
					return sourceGui;
				}
			}
			//If we reach this statement, it means no windows were available.
			MessageBox (appState.hWnd,
				"Maximum limit of 32 source windows.",
				"CowBite", MB_ICONEXCLAMATION | MB_OK);
			return NULL;
		} else {
			//Otherwise, it is already loaded, and we want to find it in our list..
			//Note that closing a window unloads the source.
			for (int i = 0; i < MAX_SOURCEWINDOWS; i++) {
				sourceGui = &appState.settings.sourceGuis[i];
				if (sourceGui->sourceFile == sourceFile) {
					return sourceGui;
				} 
			}
			//If we reach this, we were unable to find source
			return NULL;
		}
	} else return NULL;
}

//////////////////////////////////////////
//Use this to "delete" a source gui.
void SourceGui_delete(SourceGui* sourceGui) {


}
