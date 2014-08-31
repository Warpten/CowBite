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

#ifndef STABS_H
#define STABS_H

#include "ELFFile.h"
#include <list>	//So that I can have lists of stuff.

#define STABS_MAX_DUPS	8	//This is the maximum number of duplicate elements
							//I allow to be in a list
#define STABS_MAX_SOURCELINES 0xFFFF

//////////////////////////////
//A generic entry for stabs.
////////////////////////////
typedef struct tStabListEntry {
	struct tStabListEntry* next;
	char* name;
	StabEntry entry;
} StabListEntry;

//////////////////////////////
//An array of pointers to stab lists
//In the event that we search for a stab item
//and there are multiple matches.
///////////////////////////////////
typedef struct tStabListList {
	struct tStabListList* next;
	StabListEntry* list;
} StabListList;

////////////////////////////
//tSourceLine
//Uses stab notation to represent a line of source code.
////////////////////////////
typedef struct tStabSourceLine {
	struct tStabSourceLine* next;
	char* filename;
	StabEntry entry;
	//u32 startAddress;	//Address this source line corresponds to
	//u32 nextAddress;	//Address of beginning of next source line
} StabSourceLine;

////////////////////////////
//This represents a line of asm.
//Contains the line number at which
//the asm should be inserted into the output
//(in this case, a list box)
////////////////////////////
typedef struct tStabAsmLine {
	u32 address;
	u32 lineNumber;
} StabAsmLine;

typedef struct tStabData {
	struct tStabData* next;
	char* name;
	StabEntry entry;
} StabData;

typedef struct tStabSourceFile {
	struct tStabSourceFile* next;
	char* filename;
	StabEntry entry;
	StabSourceLine lineList;	//Lines found in this source.
	StabData globalList;	//Globals found in this source
	u32 numGlobalVars;
	u32 fileStart;
	u32 fileEnd;	//Because as far as i know, there is no other way to tell.

	char* data;		//Pointer to the data in the file
	char** lines;	//An array of pointers to the lines in the data (above)
	u32 numSourceLines;	//Total number of lines in the file.
	StabAsmLine* asmLines;
	u32 numAsmLines;
	u32 numFunctions;

	char* fullPath;	//Full path to the file
	u32 loaded;		//Flag to tell if the file has been loaded.
	//u32 display;	//Flag to tell whether to show this file in the source browser
					//whenver an address contained in this file is reached.
} StabSourceFile;

typedef struct tStabFunction {
	struct tStabFunction* next;
	char* name;
	StabEntry entry;
	char* filename;
	u32 functionBegin;
	u32 blockBegin;
	u32 blockEnd;
	u32 numLocalVars;
	u32 numStaticVars;
	u32 numParams;
	StabData localVarList;
	StabData staticVarList;
	StabData paramList;
} StabFunction;

typedef struct tStabs {
	Elf32_Shdr* stabHeader;
	char* stabStringTable;		//For .stab debug strings
	StabEntry* stabTable;		//For .stab debug data.

	u32 numSourceLines;
	u32 loaded;					//if it's loaded.
	StabSourceFile sourceList;
	StabFunction functionList;
	//StabData globalList;
} Stabs;


void Stabs_init(Stabs* stabs, ELFFile* elfFile);
void* Stabs_findByName(StabListEntry* list, char* name);
void* Stabs_findByDemangledName(StabListEntry* list, char* name);
void* Stabs_findByAddress(StabListEntry* list, u32 value);
void* Stabs_findByValue(StabListEntry* list, u32 value);
void* Stabs_findByDesc(StabListEntry* list, u16 desc);
StabListList* Stabs_findAllByDesc(StabListEntry* list, u16 desc);
StabSourceFile* Stabs_findSourceByAddress(Stabs* stabs, u32 address);
StabFunction* Stabs_findFunctionByAddress(Stabs* stabs, u32 address);
u32 Stabs_findNextSourceAddress(Stabs* stabs, u32 address);
char* Stabs_demangle(char* name);
void Stabs_freeList(StabListEntry* list);
void Stabs_freeSource(StabSourceFile* sourceFile);
void Stabs_delete(Stabs* stabs);

//STAB SYMBOL TYPES
//The following symbol types indicate that this is a stab. 
//This is the full list of stab numbers, including stab types 
//that are used in languages other than C.

#define N_GSYM 0x20 //Global symbol
#define N_FNAME	0x22	//Function name (for BSD Fortran)
#define N_FUN	0x24	//Function name (see section Procedures) or text segment variable (see section Static Variables).
#define N_STSYM	0x26	//Data segment file-scope variable; see section Static Variables.
//#define N_STSYM	0x26	//Static variable
#define N_LCSYM	0x28	//BSS segment file-scope variable; see section Static Variables.
#define N_MAIN	0x2A	//Name of main routine; see section Main Program.
#define N_ROSYM	0x2C	//Variable in .rodata section; see section Static Variables.
#define N_PC	0x30	//Global symbol (for Pascal); see section N_PC.
#define N_NSYMS	0x32	//Number of symbols (according to Ultrix V4.0); see section N_NSYMS.
#define N_NOMAP	0x34	//No DST map; see section N_NOMAP.
#define N_OBJ	0x38	//Object file (Solaris2).
#define N_OPT	0x3C	//Debugger options (Solaris2).
#define N_RSYM	0x40	//Register variable; see section Register Variables.
#define N_M2C	0x42	//Modula-2 compilation unit; see section N_M2C.
#define N_SLINE	0x44	//Line number in text segment; see section Line Numbers.
#define N_DSLINE	0x46	//Line number in data segment; see section Line Numbers.
#define N_BSLINE	0x48	//Line number in bss segment; see section Line Numbers.
#define N_BROWS		0x48	//Sun source code browser, path to `.cb' file; see section N_BROWS.
#define N_DEFD	0x4A		//GNU Modula2 definition module dependency; see section N_DEFD.
#define N_FLINE	0x4C		//Function start/body/end line numbers (Solaris2).
#define N_EHDECL	0x50	//GNU C++ exception variable; see section N_EHDECL.
#define N_MOD2	0x50	//Modula2 info "for imc" (according to Ultrix V4.0); see section N_MOD2.
#define N_CATCH 0x54	//GNU C++ catch clause; see section N_CATCH.
#define N_SSYM	0x60	//Structure of union element; see section N_SSYM.
#define N_ENDM	0x62	//Last stab for module (Solaris2).
#define N_SO	0x64	//Path and name of source file; see section Paths and Names of the Source Files.
#define N_LSYM	0x80	//Stack variable (see section Automatic Variables Allocated on the Stack) or type (see section Giving a Type a Name).
#define N_BINCL	0x82	//Beginning of an include file (Sun only); see section Names of Include Files.
#define N_SOL	0x84	//Name of include file; see section Names of Include Files.
#define N_PSYM	0xA0	//Parameter variable; see section Parameters.
#define N_EINCL	0xA2	//End of an include file; see section Names of Include Files.
#define N_ENTRY	0xA4	//Alternate entry point; see section N_ENTRY.
#define N_LBRAC	0xC0	//Beginning of a lexical block; see section Block Structure.
#define N_EXCL	0xC2	//Place holder for a deleted include file; see section Names of Include Files.
#define N_SCOPE	0xC4	//Modula2 scope information (Sun linker); see section N_SCOPE.
#define N_RBRAC	0xE0	//End of a lexical block; see section Block Structure.
#define N_BCOMM	0xE2	//Begin named common block; see section Common Blocks.
#define N_ECOMM	0xE4	//End named common block; see section Common Blocks.
#define N_ECOML	0xE8	//Member of a common block; see section Common Blocks.
#define N_WITH	0xEA	//Pascal with statement: type,,0,0,offset (Solaris2).
#define N_NBTEXT	0xF0	//Gould non-base registers; see section Non-base registers on Gould systems.
#define N_NBDATA	0xF2	//Gould non-base registers; see section Non-base registers on Gould systems.
#define N_NBBSS	0xF4	//Gould non-base registers; see section Non-base registers on Gould systems.
#define N_NBSTS	0xF6	//Gould non-base registers; see section Non-base registers on Gould systems.
#define N_NBLCS	0xF8	//Gould non-base registers; see section Non-base registers on Gould systems.


#endif