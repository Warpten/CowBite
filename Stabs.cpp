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

#include "Stabs.h"
#include "Console.h"
#include <stdlib.h>

/////////////////////////////////
//Initialize stabs.  We get them from an elf file.
////////////////////////////////
void Stabs_init(Stabs* stabs, ELFFile* elfFile) {
	char* filename;
//	char* name;
	char text[256];
	u32 lineNum;

	if (elfFile->stabTable == NULL)
		return;

	Stabs_delete(stabs);	//Free any used memory.

	//Print out stab info
	stabs->stabStringTable = elfFile->stabStringTable;
	Elf32_Shdr* stabHeader = stabs->stabHeader = elfFile->stabHeader;
	StabEntry* stabTable = stabs->stabTable = elfFile->stabTable;
	u32 stabEntries = stabHeader->sh_size / stabHeader->sh_entsize;

	lineNum = 0;
	StabEntry* stab;
	//Count how many line entries we have.
/*	for(u32 i = 0; i < stabEntries; i ++) {
		stab = &stabs->stabTable[i];
		if ( (stab->n_type == N_SLINE) || (stab->n_type == N_DSLINE) || (stab->n_type == N_BSLINE))
			lineNum++;
	}
	//Allocate memory for an array of source lines.
	stabs->numSourceLines = lineNum;
	stabs->lineList; = new StabSourceLine[lineNum];
	
*/	//Initialize the file name to nothing:
	
	filename = &stabs->stabStringTable[0];	//First entry is always ""
	//Set pointers to our lists.
	StabSourceFile* currentSourceFile = &stabs->sourceList;
	currentSourceFile->filename = "";
	StabSourceLine* currentSourceLine =	&stabs->sourceList.lineList;
	currentSourceLine->filename = "";
	StabFunction* currentFunction = &stabs->functionList;
	currentFunction->name = "";
	StabData* currentLocalVar = &stabs->functionList.localVarList;
	currentLocalVar->name = "";
	StabData* currentStaticVar = &stabs->functionList.staticVarList;
	currentStaticVar->name = "";
	StabData* currentParam = &stabs->functionList.paramList;
	currentParam->name = "";
	StabData* currentGlobalVar = &stabs->sourceList.globalList;
	currentGlobalVar->name = "";
	

	u32 inFunction = 0;	//Flag to let it know we're currently
						//looking at a function description.

	//Now found out what file each source line appears in.
	for( int i = 0; i < stabEntries; i ++) {
		stab = &stabs->stabTable[i];
		//if (strcmp( &stabs->stabStringTable[stab->n_strx], "functions\\loadlevel.c") == 0)
		//	int blah = 0;
	/*	sprintf(text, "\nType: %08X, Desc:  %08X, Value: %08X, Other: %08X, Name:",
		stab->n_type, stab->n_desc, stab->n_value, stab->n_other); 
		Console_print(text);
		Console_print(&stabs->stabStringTable[stab->n_strx]);
	*/	
		switch (stab->n_type) {
			case N_SOL:	//If it's an include file (note that I haven't thoroughly tested this!)
			case N_SO:	//If it's the name of a source file...
				filename = &stabs->stabStringTable[stab->n_strx];
				currentSourceFile->fileEnd = stab->n_value;	//Tell it where the source ends
				currentSourceFile->next = new StabSourceFile;	//Start a new source
				currentSourceFile = currentSourceFile->next;
				memset(currentSourceFile, 0, sizeof(StabSourceFile));
				currentSourceFile->filename = filename;
				currentSourceFile->entry = *stab;
				currentSourceFile->fileStart = stab->n_value;
				currentSourceLine = &currentSourceFile->lineList;
				currentGlobalVar = &currentSourceFile->globalList;
				inFunction = 0;	//We are outside of a function for sure.
				break;
			//This might break if nested functions (yuck) are specified.
			case N_FUN:	//If it's a function
				
				//Some ELF files out there havea  different stabs notation, where instead of
				//having an end block, there is a second "function" entry with an invalid address
				//value.  This value is actually the size of the function, so it can be used
				//to determine where the function ends (instead of LBRAC and RBRAC).
				//Best to support both.
				if (stab->n_value >= 0x01000000) {	//If it's the function
					inFunction = 1;
					currentFunction->next = new StabFunction;
					currentFunction = currentFunction->next;
					memset(currentFunction,0,sizeof(StabFunction));
					currentFunction->name = &stabs->stabStringTable[stab->n_strx];
					currentFunction->functionBegin = stab->n_value & 0xFFFFFFFE;
					currentFunction->filename = filename;
					currentFunction->entry = *stab;
					currentLocalVar = &currentFunction->localVarList;
					currentStaticVar = &currentFunction->staticVarList;
					currentParam = &currentFunction->paramList;
					currentSourceFile->numFunctions++;
				} else {	//Othewise, it is a function size (only used in some elf files)
					currentFunction->blockEnd = currentFunction->functionBegin + stab->n_value;
				}
				break;	
			case N_LBRAC:
				currentFunction->blockBegin = stab->n_value;
				break;
			case N_RBRAC:
				currentFunction->blockEnd = stab->n_value;
				inFunction = 0;	//We are done with the function description.
				break;
			case N_GSYM:
				currentGlobalVar->next = new StabData;
				currentGlobalVar = currentGlobalVar->next;
				currentGlobalVar->entry = *stab;
				currentGlobalVar->name = &stabs->stabStringTable[stab->n_strx];
				currentGlobalVar->next = NULL;
				currentSourceFile->numGlobalVars++;
				break;
			case N_LSYM:
				if (inFunction)	{	//If we're in a function, it's a stack variable.
					currentLocalVar->next = new StabData;
					currentLocalVar = currentLocalVar->next;
					currentLocalVar->entry =*stab;
					currentLocalVar->name = &stabs->stabStringTable[stab->n_strx];
					currentLocalVar->next = NULL;
					currentFunction->numLocalVars++;
				} else {}	//Otherwise it's a type and I haven't implemented that yet.
				break;
			case N_STSYM:
				currentStaticVar->next = new StabData;
				currentStaticVar = currentStaticVar->next;
				currentStaticVar->entry = *stab;
				currentStaticVar->name = &stabs->stabStringTable[stab->n_strx];
				currentStaticVar->next = NULL;
				currentFunction->numStaticVars++;
				break;
			case N_PSYM:	//If it's a parameter.
				currentParam->next = new StabData;
				currentParam = currentParam->next;
				currentParam->entry = *stab;
				currentParam->name = &stabs->stabStringTable[stab->n_strx];
				currentParam->next = NULL;
				currentFunction->numParams++;
				break;

			
			case N_SLINE:
			case N_DSLINE:
			case N_BSLINE:
				//Add an entry to our source list.
				currentSourceLine->next = new StabSourceLine;
				currentSourceLine = currentSourceLine->next;
				currentSourceLine->filename = filename;
				currentSourceLine->entry = *stab;
				currentSourceLine->next = NULL;
				break;
		default:
				break;
		}//Switch.
	}
	//For the very last bit of source, there is no way to tell what the last line was
	//(what the hell is up with that???), but we can tell the last line of the last function,
	//or the last source code line entry, so we'll take the greater of the two.
	//(+2 Because the "end" field is actually the "next" field for comparisons)
	if (currentSourceLine->entry.n_value > currentFunction->blockEnd ) {
		currentSourceFile->fileEnd = currentSourceLine->entry.n_value+2;	
	} else {
		currentSourceFile->fileEnd = currentFunction->blockEnd+2;		
	}

	stabs->loaded = 1;
	VariablesGui_reload();	//Fill up that variables gui.

	int blah = 0;
/*
	//Print out information about the stabs table.
	Console_print("\n\n#######################################");
	Console_print("\nSTABS functions");
	Console_print("\n#######################################");
		
	currentFunction = &stabs->functionList;
	while (currentFunction != NULL) {
		Console_print("\nFunction: ");
		Console_print(currentFunction->name);
		Console_print("\nLocal Variables:");
		currentLocalVar = &currentFunction->localVarList;
		while (currentLocalVar != NULL) {
			Console_print("\nName: ");
			if (currentLocalVar->name)
				Console_print(currentLocalVar->name);
			currentLocalVar = currentLocalVar->next;
		}
		Console_print("\nStatic Variables:");
		currentStaticVar = &currentFunction->staticVarList;
		while (currentStaticVar != NULL) {
			Console_print("\nName: ");
			if (currentStaticVar->name)
				Console_print(currentStaticVar->name);
			currentStaticVar = currentStaticVar->next;
		}
		Console_print("\nParameters:");
		currentParam = &currentFunction->paramList;
		while (currentParam != NULL) {
			Console_print("\nName: ");
			if (currentParam->name)
				Console_print(currentParam->name);
			currentParam = currentParam->next;
		}
		Console_print("\n-----------------------");
		currentFunction = currentFunction->next;
	}
*/
/*	//Print out information about the stabs table.
	Console_print("\n\n#######################################");
	Console_print("\nSTABS source");
	Console_print("\n#######################################");
	currentSourceFile = &stabs->sourceList;
	while (currentSourceFile != NULL) {
		Console_print("\nSourceFile: ");
		Console_print(currentSourceFile->filename);
		Console_print("\nGlobal Variables:");
		currentGlobalVar = &currentSourceFile->globalList;
		while (currentGlobalVar != NULL) {
			Console_print("\nName: ");
			if (currentGlobalVar->name)
				Console_print(currentGlobalVar->name);
			currentGlobalVar = currentGlobalVar->next;
		}
		Console_print("\n-----------------------");
		currentSourceFile = currentSourceFile->next;
	}*/
/*	
	u32 i = 0; i < stabEntries; i ++) {
		stab = &stabs->stabTable[i];
		name = &stabs->stabStringTable[stab->n_strx];
	
		Console_print("\nName: ");
		Console_print(name);
		sprintf(text, "Type: 0x%X; Other: 0x%X;  Desc: 0x%X; Value: 0x%X",
			stab->n_type,stab->n_other,stab->n_desc,stab->n_value);
		Console_print(text);
		Console_print("\n-----------------------");
	
	}
*/
	
}

////////////////////////////////////
//Finds a stab entry by the name string
//Note that this will give erroneous results
//if a variable name includes another variable name.:(
/////////////////////////////////////
void* Stabs_findByName(StabListEntry* list, char* name) {
	list = list->next;	//The first entry is always ignored.
	s32 length = strlen(name);
	if (length < 1)
		return NULL;
	//Compare the first LENGTH characters
	while (list != NULL) {
		if (strncmp(name, list->name, length) == 0)
			return list;
		list = list->next;
	}
	return NULL;
}

////////////////////////////////////
//Finds a stab entry by a name string
//which is has been demangled.
/////////////////////////////////////
void* Stabs_findByDemangledName(StabListEntry* list, char* name) {
	list = list->next;	//The first entry is always ignored.
	//Compare the first LENGTH characters
	while (list != NULL) {
		if (strcmp(name, Stabs_demangle(list->name)) == 0)
			return list;
		list = list->next;
	}
	return NULL;
}

////////////////////////////////////
//Finds a stab entry by the address
/////////////////////////////////////
void* Stabs_findByAddress(StabListEntry* list, u32 value) {
	list = list->next;	//The first entry is always ignored.
	while (list != NULL) {
		//Strip off the last bit, since stabs will record a switch to thumb
		//as being an address with bit 0 set (which we don't want).
		if ( (list->entry.n_value & 0xFFFFFFFE) == (value&0xFFFFFFFE))
			return list;
		list = list->next;
	}
	return NULL;
}

////////////////////////////////////
//Finds a stab entry strictly by value
/////////////////////////////////////
void* Stabs_findByValue(StabListEntry* list, u32 value) {
	list = list->next;	//The first entry is always ignored.
	while (list != NULL) {
		if (list->entry.n_value == value)
			return list;
		list = list->next;
	}
	return NULL;
}

////////////////////////////////////
//Finds a stab entry strictly by dexc
/////////////////////////////////////
void* Stabs_findByDesc(StabListEntry* list, u16 desc) {
	list = list->next;	//The first entry is always ignored.
	while (list != NULL) {
		if (list->entry.n_desc == desc)
			return list;
		list = list->next;
	}
	return NULL;
}


////////////////////////////////////
//Finds ALL stab entries of this desc.
/////////////////////////////////////
StabListList* Stabs_findAllByDesc(StabListEntry* list, u16 desc) {
	int numEntries = 0;
	StabListList* startListList = NULL;
	StabListList* listList = NULL;	//A list of all the things we find.

	list = list->next;	//The first entry in the search list is always ignored.
	while (list != NULL) {
		if (list->entry.n_desc == desc) {
			//if (desc == 78 && list->entry.n_value == 0x8001532)
			//	int blah = 0;

			if (startListList == NULL) {
				startListList = listList = new StabListList;	//We don't ignore the first entry of listlists.
			} else listList = listList->next = new StabListList;
			listList->list = list;
			listList->next = NULL;
		}
		list = list->next;
	}
	return startListList;
	return NULL;
}

//////////////////////////////////////////////////////
//This returns a source file based on memory address
////////////////////////////////////////////////////
StabSourceFile* Stabs_findSourceByAddress(Stabs* stabs, u32 address) {
	StabSourceFile* list;
	list = &stabs->sourceList;
	list = list->next;	//The first entry is always ignored.
	while (list != NULL) {
		if ((address >= list->fileStart) && (address < list->fileEnd) )
			return list;
		list = list->next;
	}
	return NULL;
}

//////////////////////////////////////////////////////
//This returns a function based on memory address
////////////////////////////////////////////////////
StabFunction* Stabs_findFunctionByAddress(Stabs* stabs, u32 address) {
	StabFunction* list;
	list = &stabs->functionList;
	list = list->next;	//The first entry is always ignored.
	while (list != NULL) {
		if ((address >= list->functionBegin) && (address <= list->blockEnd) ) {
			return list;
		}
		
		list = list->next;
	}
	return NULL;
}

///////////////////////////////////////////////////
//This finds the source line which is nearest to this address
///////////////////////////////////////////////////
u32 Stabs_findNextSourceAddress(Stabs* stabs, u32 address) {
	StabSourceFile* sourceFile;
	StabSourceLine* list;
	sourceFile = Stabs_findSourceByAddress(stabs, address);
	if (sourceFile == NULL) 
		return 0xFFFFFFFF;
	 
	 list = sourceFile->lineList.next;
	 //Hopefully they are listed in order from greatest to least (haven't found a situation
	 //where they weren't)
	while (list != NULL) {
		if (address < list->entry.n_value)
			return list->entry.n_value;

		list = list->next;
	}
	//Otherwise, we found no next line.  So use the last line of this file
	return sourceFile->fileEnd;
}

////////////////////////////////////
//Frees a stabs list.
/////////////////////////////////////
void Stabs_freeList(StabListEntry* list) {
	StabListEntry* previous;
	previous = list;
	list = list->next;	//The first entry is always ignored.
	previous->next = NULL;
	while (list != NULL) {
		previous = list;
		list = list->next;
		delete previous;
	}
}

////////////////////////////////////
//Demangles a stabs variable or function name
////////////////////////////////////
char* Stabs_demangle(char* name) {
	static char text[256];	//Buffer for damangline suitably large.
	char* colon;
	if ((name != NULL) && (strcmp(name, "") != 0)) {	//If passed a valid string
		strncpy(text, name, 255);
		colon = strrchr(text, ':');
		if (colon != NULL)
			colon[0] = '\0';		//Replace the lst : with a \0
	} else strcpy(text, "");
	return text;
}

/////////////////////////////////////////////////
//This frees the source code for a stab source file.
//It does NOT free the stab source entry itself, just
//the source and asm lines.
//Do not confuse free source with open source.
////////////////////////////////////////////////
void Stabs_freeSource(StabSourceFile* sourceFile) {
	if (sourceFile->data != NULL)
		delete [] sourceFile->data;
	sourceFile->data = NULL;
	sourceFile->numSourceLines = 0;
	if (sourceFile->fullPath != NULL)	//Fullpath is allocated by strdup
		delete [] sourceFile->fullPath;
	sourceFile->fullPath = NULL;
	if (sourceFile->asmLines != NULL)
		delete [] sourceFile->asmLines;
	sourceFile->asmLines = NULL;
	sourceFile->numAsmLines = 0;
	sourceFile->loaded = 0;
	
}

///////////////////////////////////////
//Frees all resources used by the stab.
///////////////////////////////////////
void Stabs_delete(Stabs* stabs) {
	StabSourceFile* previousFile;
	StabSourceFile* sourceList;
	if (stabs->stabStringTable != NULL)
		delete [] stabs->stabStringTable;
	//if (elfFile->stabHeader != NULL)//Included with sectionHeaders.
	//	delete [] elfFile->stabHeader;
	if (stabs->stabTable != NULL)
		delete [] stabs->stabTable;
	Stabs_freeList((StabListEntry*)&stabs->functionList);
	sourceList = stabs->sourceList.next;
	while (sourceList != NULL) {
		previousFile = sourceList;
		sourceList = sourceList->next;
		Stabs_freeSource(previousFile);
		Stabs_freeList((StabListEntry*)&previousFile->lineList);
		Stabs_freeList((StabListEntry*)&previousFile->globalList);
	}
	VariablesGui_reload();	//IN theory causes the variables gui to divulge itself
							//of its crap.
	
	memset(stabs, 0, sizeof(Stabs));
}

