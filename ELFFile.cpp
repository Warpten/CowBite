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

#include "ELFFile.h"
#include "Console.h"
#include <stdio.h>
#include <string.h>

///////////////////////////////////////////////////////
//ELFFile_load
//Loads an elf file into our elf structure.
///////////////////////////////////////////////////////
ELFFile* ELFFile_load(ELFFile* elfFile, char* filename) {
	FILE *fp;
	Elf32_Ehdr* header;
	Elf32_Shdr* sectionStringTableHeader;
	u32 start, end, fileSize, index;
//	char* name;
	char text[255];

	if (!(fp = fopen(filename, "rb"))) {
		elfFile->loaded = 0;
		return NULL;
	}
	ELFFile_delete(elfFile);	//Free any used memory, zero the structure.

	fseek (fp, 0, SEEK_SET);
	start = ftell (fp);
	fseek (fp, 0, SEEK_END);
	end = ftell (fp);
	
	elfFile->fileSize = fileSize = (end - start)+1;

	fseek (fp, 0, SEEK_SET);

	fread (&elfFile->header, 1, sizeof(Elf32_Ehdr), fp);		//Load the file into this memory
	header = &elfFile->header;
	
	//Make sure it's an elf file.
	if ((header->e_ident[0] != 0x7F) || (strncmp( (const char*)&(header->e_ident[1]), "ELF", 3) != 0)) {
		elfFile->loaded = 0;
		return NULL;
	}
	
	//Allocate memory for the header tables.
	//Though the size of the headers is specified in the elf header,
	//i am aoing to assume that they are the size of the predefined struct.
	elfFile->programHeaders = new Elf32_Phdr[header->e_phnum];
	elfFile->sectionHeaders = new Elf32_Shdr[header->e_shnum];

	//Copy the program headers into memory
	fseek (fp, header->e_phoff, SEEK_SET);	//Seek to offset of program headers.
	fread (elfFile->programHeaders, header->e_phnum, sizeof(Elf32_Phdr), fp);

	//Copy the section headers into memory
	fseek (fp, header->e_shoff, SEEK_SET);	//Seek to offset of program headers.
	fread (elfFile->sectionHeaders, header->e_shnum, sizeof(Elf32_Shdr), fp);

	//Get the section header string table
	//(don't confuse with the section string table header!)
	index = header->e_shtrndx;	//First find the address of the header
	sectionStringTableHeader = &elfFile->sectionHeaders[index];	//Get the header
	//Allocate memory for the strings
	elfFile->sectionHeaderStringTable = new char [sectionStringTableHeader->sh_size];
	//Allocate memory for a table of these strings -- one for each entry.
	elfFile->sectionStrings = new char*[header->e_shnum];
	fseek(fp, sectionStringTableHeader->sh_offset, SEEK_SET);	//Get the file offset
	//Copy it over to our array
	fread (elfFile->sectionHeaderStringTable, sectionStringTableHeader->sh_size, sizeof(char), fp);
	//Now go through each section, find it's index into the array, and get a poniter to it
	//It looks complicated right?  Yeah.  But it makes sense if you look at it right.
	for (u32 i = 0; i < header->e_shnum; i++) {
		elfFile->sectionStrings[i] = & elfFile->sectionHeaderStringTable [elfFile->sectionHeaders[i].sh_name];
	}
	u32 romSize = 0;
	
	Console_print("CowBite:  Loading ELF File.\n");
	for (i = 0; i < header->e_shnum; i++) {
		Elf32_Shdr* sHeader = &elfFile->sectionHeaders[i];
	switch (sHeader->sh_type) {
			case SHT_NULL:
				break;
			case SHT_PROGBITS:	
				//If this is the stab table
				if (strcmp(elfFile->sectionStrings[i], ".stab") == 0) {
					elfFile->stabHeader = sHeader;
					elfFile->stabTable = new StabEntry [sHeader->sh_size/sizeof(StabEntry)];
					fseek(fp, sHeader->sh_offset, SEEK_SET);	//Get the file offset
					//Copy it over to our array.
					fread (elfFile->stabTable, sHeader->sh_size, sizeof(u8), fp);
				} else if (strcmp(elfFile->sectionStrings[i], ".text") == 0) {
					//If this is the "text" section, use it as the start address.
					arm.programStart = sHeader->sh_addr;
				}
				
				//Try to figure out the size of ROM.
				index = sHeader->sh_addr >> 24;
				//If it is a part of rom . . . 
				//(This will break if the memory goes into different sections
				//of rom :(
				if ( (index == 0x8) || (index == 0x9)) {
					//The rom size will be the size of the largest address used.
					romSize = __max(romSize, sHeader->sh_size + sHeader->sh_addr );
				}
				break;
			case SHT_SYMTAB: 
				//If this is the primary symbol table
				if (strcmp(elfFile->sectionStrings[i], ".symtab") == 0) {
					elfFile->symbolHeader = sHeader;
					elfFile->symbolTable = new Elf32_Sym [sHeader->sh_size/sizeof(Elf32_Sym)];
					fseek(fp, sHeader->sh_offset, SEEK_SET);	//Get the file offset
					//Copy it over to our array.
					fread (elfFile->symbolTable, sHeader->sh_size, sizeof(u8), fp);
				} 
				break;
			
			case SHT_STRTAB:
				//If this is the string table (and not the section header string table)
				if (strcmp(elfFile->sectionStrings[i], ".strtab") == 0) {
					elfFile->stringTable = new char [sHeader->sh_size];
					fseek(fp, sHeader->sh_offset, SEEK_SET);	//Get the file offset
					//Copy it over to our array.
					fread (elfFile->stringTable, sHeader->sh_size, sizeof(char), fp);
				} 
				//Otherwise, if this is the stabs string table...
				else if (strcmp(elfFile->sectionStrings[i], ".stabstr") == 0) {
					elfFile->stabStringTable = new char [sHeader->sh_size];
					fseek(fp, sHeader->sh_offset, SEEK_SET);	//Get the file offset
					//Copy it over to our array.
					fread (elfFile->stabStringTable, sHeader->sh_size, sizeof(char), fp);
				}
				break;
			case SHT_RELA:
				break;
			case SHT_HASH:
				break;
			case SHT_DYNAMIC:
				break;
			case SHT_NOTE:
				break;
			case SHT_NOBITS:
				break;
			case SHT_REL:
				break;
			case SHT_DYNSYM:
				break;
			default:
				break;
		}
	}
	

	//Ok, now this part may seem to be braindead, since I already calculated the ROM size
	//using the section headers, but it seems that not all size information gets included
	//in those headers.  So I also take into account the program sizes listed in the pheader
	//table, and use the greatest size of all of these as my ROM size.
	for (i = 0; i < header->e_phnum; i++) {
		Elf32_Phdr* pHeader = &elfFile->programHeaders[i];
		//Find out if the physical address is a part of ROM
		u32 index = pHeader->p_paddr >> 24;
		
		//If it is a part of rom . . . 
		//(This will break if the memory goes into different sections
		//of rom :(
		if ( (index == 0x8) || (index == 0x9)) {
			//The rom size will be the size of the largest address used.
			romSize = __max(romSize, pHeader->p_paddr + pHeader->p_memsz);
		}
	}
	////////////////////////////////////////////////
	//Allocate ROM
	////////////////////////////////////////////////
	//elfFile->programData = new u8* [header->e_phnum];	//I don't use this here.
/*	u32 romSize = 0;
	for (i = 0; i < header->e_phnum; i++) {
		Elf32_Phdr* pHeader = &elfFile->programHeaders[i];
		//Find out if the physical address is a part of ROM
		u32 index = pHeader->p_paddr >> 24;
		//If it is a part of rom . . . 
		//(This will break if the memory goes into different sections
		//of rom :(
		if ( (index == 0x8) || (index == 0x9)) {
			//The rom size will be the size of the largest address used.
			romSize = __max(romSize, pHeader->p_paddr + pHeader->p_memsz);
		}
	}*/
	//romSize = 0x8022D0C;	//DEBUG
	if (romSize != 0) {
		romSize -= 0x8000000;	//Get the size relative to 0x8000000.
		elfFile->romSize = romSize;
		elfFile->romData = new u8[romSize];		//Allocate the memory.
		memset(elfFile->romData, 0, romSize);	//Zero it out
		gbaMem.u8ROM = elfFile->romData;

		//Set up our memory indexes.  Also set up mirror ROM.
		gbaMem.u8Mem[8] = gbaMem.u8Mem[0xA] = gbaMem.u8Mem[0xC] = gbaMem.u8ROM;
		//Now because CowBite divides memory up into 15 segments so as to allocate
		//only what it needs, we end up needing to split roms greater than 16 mb
		//accross two segments.
		gbaMem.u8Mem[9] = gbaMem.u8Mem[0xB] = gbaMem.u8Mem[0xD] = gbaMem.u8ROM + 0x1000000;

		//Record the size of each segment of rom.
		//Should I be using "romSize" instead of "fileSize"?  For now I use
		//fileSize (will make keeping tracks of illegal rom accesses more accurate)
		if (romSize <= 0x1000000) {
			gbaMem.memorySize[8] = gbaMem.memorySize[0xA] = gbaMem.memorySize[0xC] = romSize;
			gbaMem.memorySize[9] = gbaMem.memorySize[0xB] = gbaMem.memorySize[0xD] = 0;
		} else {
			//Note that I set the first half of rom to be the filsize.  This is
			//perfectly alright and will even facilitate dumping/loading.
			gbaMem.memorySize[8] = gbaMem.memorySize[0xA] = gbaMem.memorySize[0xC] = romSize;
			gbaMem.memorySize[9] = gbaMem.memorySize[0xB] = gbaMem.memorySize[0xD] = romSize - 0x1000000;
		} 
	} else {
		//Otherwise allocate some dummy ROM so that cowbite don't crash.
		elfFile->romSize = 0;
		elfFile->romData = NULL;//new u8[elfFile->romSize];
		//memset(elfFile->romData, 0, elfFile->romSize);
	}
	fclose(fp);
	if (!(fp = fopen(filename, "rb"))) {
		elfFile->loaded = 0;
		return NULL;
	}
	int blah;
	
	//Copy the data listed in the section headers over.
	for (i = 0; i < header->e_shnum; i++) {
		Elf32_Shdr* sHeader = &elfFile->sectionHeaders[i];
		
		if (sHeader->sh_type == SHT_PROGBITS) {
		//if ( (strcmp(&elfFile->sectionHeaderStringTable[sHeader->sh_name], ".text") == 0)
		//	|| (strcmp(&elfFile->sectionHeaderStringTable[sHeader->sh_name], ".rodata") == 0)) {
			if (sHeader->sh_size > 0) {
				u32 index = sHeader->sh_addr >> 24;
				u32 offset = sHeader->sh_addr & 0xFFFFFF;
				if (index > 0) {	//Don't copy memory to the BIOS.  That would suck.
					//If there's enough room to copy it over...
					if ( (offset + sHeader->sh_size) <= gbaMem.memorySize[index]) {
						blah = fseek(fp, sHeader->sh_offset, SEEK_SET);	//Get the file offset
						//Copy it over to our array.  But only the bytes specified in the file.
						blah = fread (&(gbaMem.u8Mem[index][offset]), sizeof(u8), sHeader->sh_size,fp);
					}
				}

			}
		}
	
	}

	//////////////////////////////////
	//Copy data to memory AGAIN, because the section headers won't find all the data
	//we want.  However, we can't just do one or the other, because otherwise multiboot
	//files will crash.
	//////////////////////////////////
	for (i = 0; i < header->e_phnum; i++) {
		Elf32_Phdr* pHeader = &elfFile->programHeaders[i];
		if (pHeader->p_memsz > 0) {
				u32 index = pHeader->p_paddr >> 24;
				u32 offset = pHeader->p_paddr & 0xFFFFFF;
				//If there's enough room to copy it over...
				if ( (offset + pHeader->p_filesz) <= gbaMem.memorySize[index]) {
					blah = fseek(fp, pHeader->p_offset, SEEK_SET);	//Get the file offset
					//Copy it over to our array.  But only the bytes specified in the file.
					blah = fread (&(gbaMem.u8Mem[index][offset]), sizeof(u8), pHeader->p_filesz,fp);
				}
		}
	}

	//gbaMem.u8ROM = elfFile->romData;
	elfFile->loaded = 1;
									//Close the file.

	

//	char* text;
	Elf32_Shdr* symbolHeader = elfFile->symbolHeader;
	Elf32_Sym* symbolTable = elfFile->symbolTable;
	Elf32_Sym* symbol;
	u32 symbolEntries = symbolHeader->sh_size / sizeof(Elf32_Sym);
	elfFile->numSymbols = symbolEntries;
	char* name;	

	//Determine the number of each kind of symbol
	if (elfFile->symbolTable != NULL) {
		for(i = 0; i < symbolEntries; i++) {
			switch (ELF32_ST_TYPE(symbolTable[i].st_info)) {
				case STT_NOTYPE:  elfFile->numNoTypeSymbols++;
					break;
				case STT_OBJECT: elfFile->numObjectSymbols++;
					break;
				case STT_GBAFUNC:
				case STT_FUNC: elfFile->numFuncSymbols++;
					break;
				case STT_SECTION: elfFile->numSectSymbols++;
					break;
				case STT_FILE: elfFile->numFileSymbols++;
					break;
				case STT_GBALABEL: elfFile->numLabelSymbols++;
					break;
				default:  elfFile->numOtherSymbols++;
					break;
			}
		}
	
		//Allocate memory for these symbols.
		if (elfFile->numNoTypeSymbols)
			elfFile->noTypeSymbols = new Elf32_Sym[elfFile->numNoTypeSymbols];
		if (elfFile->numObjectSymbols)
			elfFile->objectSymbols = new Elf32_Sym[elfFile->numObjectSymbols];
		if (elfFile->numFuncSymbols)
			elfFile->funcSymbols = new Elf32_Sym[elfFile->numFuncSymbols];
		if (elfFile->numSectSymbols)
			elfFile->sectSymbols = new Elf32_Sym[elfFile->numSectSymbols];
		if (elfFile->numFileSymbols)
			elfFile->fileSymbols = new Elf32_Sym[elfFile->numFileSymbols];
		if (elfFile->numLabelSymbols)
			elfFile->labelSymbols = new Elf32_Sym[elfFile->numLabelSymbols];
		if (elfFile->numOtherSymbols)
			elfFile->otherSymbols = new Elf32_Sym[elfFile->numOtherSymbols];
		
		u32 currentNoType = 0;
		u32 currentObject = 0;
		u32 currentFunc = 0;
		u32 currentSect = 0;
		u32 currentFile = 0;
		u32 currentLabel = 0;
		u32 currentOther = 0;

		//Copy the symbols over to our arrays.
		for(i = 0; i < symbolEntries; i++) {
			switch (ELF32_ST_TYPE(symbolTable[i].st_info)) {
				case STT_NOTYPE:  
					elfFile->noTypeSymbols[currentNoType] = symbolTable[i];	//This does a shallow copy.
					currentNoType++;
					break;
				case STT_OBJECT: 
					elfFile->objectSymbols[currentObject] = symbolTable[i];	//This does a shallow copy.
					currentObject++;
					break;
				case STT_GBAFUNC:
				case STT_FUNC:
					elfFile->funcSymbols[currentFunc] = symbolTable[i];	//This does a shallow copy.
					currentFunc++;
					break;
				case STT_SECTION:
					elfFile->sectSymbols[currentSect] = symbolTable[i];	//This does a shallow copy.
					currentSect++;
					break;
				case STT_FILE: 
					elfFile->fileSymbols[currentFile] = symbolTable[i];	//This does a shallow copy.
					currentFile++;
					break;
				case STT_GBALABEL:
					elfFile->labelSymbols[currentLabel] = symbolTable[i];	//This does a shallow copy.
					currentLabel++;
					break;
				default:
					elfFile->otherSymbols[currentOther] = symbolTable[i];	//This does a shallow copy.
					currentOther++;
					break;
			}
		}
		
	}//End of if

	//Now that we have all the symbols save away . . .
	//delete [] elfFile->symbolTable;
	//elfFile->symbolTable = NULL;

	return elfFile;
}

////////////////////////////////////////
//Free up memory taken by elf.
////////////////////////////////////////
void ELFFile_delete(ELFFile* elfFile) {
	if (elfFile->sectionHeaders != NULL)
		delete [] elfFile->sectionHeaders;
	if (elfFile->programHeaders != NULL)
		delete [] elfFile->programHeaders;
	if (elfFile->sectionHeaderStringTable != NULL)	//All of the strings in one big array
		delete [] elfFile->sectionHeaderStringTable;
	if (elfFile->sectionStrings != NULL)
		delete [] elfFile->sectionStrings;	//This references the above, but with indices.
	if (elfFile->stringTable != NULL)
		delete [] elfFile->stringTable;
	//if (elfFile->symbolHeader != NULL)	//Included with sectionHeaders.
	//	delete [] elfFile->symbolHeader;	
	if (elfFile->symbolTable != NULL)
		delete [] elfFile->symbolTable;
	
	if (elfFile->noTypeSymbols != NULL)
		delete [] elfFile->noTypeSymbols;
	if (elfFile->objectSymbols != NULL)
		delete [] elfFile->objectSymbols;
	if (elfFile->funcSymbols != NULL)
		delete [] elfFile->funcSymbols;
	if (elfFile->sectSymbols != NULL)
		delete [] elfFile->sectSymbols;
	if (elfFile->labelSymbols != NULL)
		delete [] elfFile->labelSymbols;
	if (elfFile->otherSymbols != NULL)
		delete [] elfFile->otherSymbols;
	
	memset(elfFile, 0, sizeof(ELFFile));
	//Don't bother deleting the rom data since GBA_delete will do this already.
	//u8* romData;			//All of the data that goes into ROM.
}

////////////////////////////////////////////////
//Get the name of a function based on the address, 
//or NULL if the function is non-existent.
////////////////////////////////////////////////
char* ELFFile_getFunctionName(ELFFile* elfFile, u32 address) {
	Elf32_Sym* symbol;
	for (int i = 0; i < elfFile->numFuncSymbols; i++) {
		symbol = &elfFile->funcSymbols[i];
		if (symbol->st_value == address) {
			return &elfFile->stringTable[symbol->st_name];
		}
	}
	return NULL;
}

/////////////////////////////////////////
//Get the name of a symbol passed in as a parameter
/////////////////////////////////////////
char* ELFFile_getSymbolName(ELFFile* elfFile, Elf32_Sym* symbol) {
	return &elfFile->stringTable[symbol->st_name];
}

////////////////////////////////////////////////
//Get the name of a symbol (any kind) based on the address.
//or NULL if the function is non-existent.
////////////////////////////////////////////////
char* ELFFile_getSymbolName(ELFFile* elfFile, u32 address) {
	Elf32_Sym* symbol;
	for (int i = 0; i < elfFile->numObjectSymbols; i++) {
		symbol = &elfFile->objectSymbols[i];
		if (symbol->st_value == address) {
			return &elfFile->stringTable[symbol->st_name];
		}
	}
	for (i = 0; i < elfFile->numFuncSymbols; i++) {
		symbol = &elfFile->funcSymbols[i];
		if (symbol->st_value == address) {
			return &elfFile->stringTable[symbol->st_name];
		}
	}
	for (i = 0; i < elfFile->numLabelSymbols; i++) {
		symbol = &elfFile->labelSymbols[i];
		if (symbol->st_value == address) {
			return &elfFile->stringTable[symbol->st_name];
		}
	}
	for (i = 0; i < elfFile->numNoTypeSymbols; i++) {
		symbol = &elfFile->noTypeSymbols[i];
		if (symbol->st_value == address) {
			return &elfFile->stringTable[symbol->st_name];
		}
	}
	//
	for (i = 0; i < elfFile->numFileSymbols; i++) {
		symbol = &elfFile->fileSymbols[i];
		if (symbol->st_value == address) {
			return &elfFile->stringTable[symbol->st_name];
		}
	}
	
	for (i = 0; i < elfFile->numOtherSymbols; i++) {
		symbol = &elfFile->otherSymbols[i];
		if (symbol->st_value == address) {
			return &elfFile->stringTable[symbol->st_name];
		}
	}
	//Not sure if I want this...
	for (i = 0; i < elfFile->numSectSymbols; i++) {
		symbol = &elfFile->sectSymbols[i];
		if (symbol->st_value == address) {
			return &elfFile->stringTable[symbol->st_name];
		}
	}
	return NULL;
}

////////////////////////////////////////////////////////////
//This finds a symbol by the name.
/////////////////////////////////////////////////////
Elf32_Sym* ELFFile_findByName(ELFFile* elfFile, char* name) {
	Elf32_Sym* symbol;
	for (int i = 0; i < elfFile->numObjectSymbols; i++) {
		symbol = &elfFile->objectSymbols[i];
		if (strcmp(name, &elfFile->stringTable[symbol->st_name]) == 0)
			return symbol;
	}
	for (i = 0; i < elfFile->numFuncSymbols; i++) {
		symbol = &elfFile->funcSymbols[i];
		if (strcmp(name, &elfFile->stringTable[symbol->st_name]) == 0)
			return symbol;
	}
	for (i = 0; i < elfFile->numLabelSymbols; i++) {
		symbol = &elfFile->labelSymbols[i];
		if (strcmp(name, &elfFile->stringTable[symbol->st_name]) == 0)
			return symbol;
	}
	for (i = 0; i < elfFile->numNoTypeSymbols; i++) {
		symbol = &elfFile->noTypeSymbols[i];
		if (strcmp(name, &elfFile->stringTable[symbol->st_name]) == 0)
			return symbol;
	}
	//
	for (i = 0; i < elfFile->numFileSymbols; i++) {
		symbol = &elfFile->fileSymbols[i];
		if (strcmp(name, &elfFile->stringTable[symbol->st_name]) == 0)
			return symbol;
	}
	
	for (i = 0; i < elfFile->numOtherSymbols; i++) {
		symbol = &elfFile->otherSymbols[i];
		if (strcmp(name, &elfFile->stringTable[symbol->st_name]) == 0)
			return symbol;
	}
	//Not sure if I want this...
	for (i = 0; i < elfFile->numSectSymbols; i++) {
		symbol = &elfFile->sectSymbols[i];
		if (strcmp(name, &elfFile->stringTable[symbol->st_name]) == 0)
			return symbol;
	}
	return NULL;
}

////////////////////////////////////////
//This prints out the size, binding, and
//type of a symbol.
////////////////////////////////////////
char* ELFFile_getInfo(Elf32_Sym* symbol) {
	static char text[64];
	char text2[16];
	strcpy (text, "");
	sprintf(text2, "%08X ", symbol->st_size);
	strcat (text, text2);
	//Get the binding
	switch (ELF32_ST_BIND(symbol->st_info)) {
		case STB_LOCAL:	sprintf(text2, " Local  ");
			break;
		case STB_GLOBAL: sprintf(text2, " Global ");
			break;
		case STB_WEAK: sprintf(text2, " Weak   ");
			break;
		default:  sprintf(text2, " Unknown");
			break;
	}
	strcat (text, text2);
	//Get the type.
	switch (ELF32_ST_TYPE(symbol->st_info)) {
		case STT_NOTYPE:	sprintf(text2, " None    ");
				break;
		case STT_OBJECT: sprintf(text2," Data    ");
				break;
		case STT_GBAFUNC:
		case STT_FUNC: sprintf(text2," Function");
				break;
		case STT_SECTION: sprintf(text2," Section ");
				break;
		case STT_FILE: sprintf(text2," File    ");
				break;
		case STT_GBALABEL: sprintf(text2," Label   ");
				break;
		default:  sprintf(text2," Unknown ");
				break;
	}
	strcat (text, text2);
	return text;
}

////////////////////////////////////////////////////
//This returns section information
////////////////////////////////////////////////////
char* ELFFile_getSectionInfo(ELFFile* elfFile, Elf32_Shdr* sHeader) {
	static char text[256];
	char text2[16];	
	char* name;
	
	name = &elfFile->sectionHeaderStringTable[sHeader->sh_name];
	
		switch (sHeader->sh_type) {
			case SHT_NULL:	sprintf(text2,"SHT_NULL    ");
				break;
			case SHT_PROGBITS:	sprintf(text2,"SHT_PROGBITS");
				break;
			case SHT_SYMTAB: sprintf(text2,"SHT_SYMTAB  ");
				break;
			case SHT_STRTAB:	sprintf(text2,"SHT_STRTAB  ");
				break;
			case SHT_RELA:  sprintf(text2,"SHT_RELA    ");
				break;
			case SHT_HASH:	sprintf(text2, "SHT_HASH    ");
				break;
			case SHT_DYNAMIC:	sprintf(text2,"SHT_DYNAMIC ");
				break;
			case SHT_NOTE:	sprintf(text2,"SHT_NOTE    ");
				break;
			case SHT_NOBITS:  sprintf(text2,"SHT_NOBITS  ");
				break;
			case SHT_REL:	Console_print ("SHT_REL     ");
				break;
			case SHT_DYNSYM:	Console_print ("SHT_DYNSYM  ");
				break;
			default:	sprintf(text2,"UNKNOWN     ");
				break;
		}
		sprintf(text, "%08X %08X  %08X %s %s", sHeader->sh_addr, sHeader->sh_offset, sHeader->sh_size, text2, name);	
	return text;
}

////////////////////////////////////////////////////
//This returns program entry information
////////////////////////////////////////////////////
char* ELFFile_getProgramInfo(ELFFile* elfFile, Elf32_Phdr* pHeader) {
	static char text[256];
	char text2[16];	
	char* name;
	
		switch(pHeader->p_type) {
			case PT_NULL:	sprintf(text2,"PT_NULL   ");
				break;
			case PT_LOAD:	sprintf(text2,"PT_LOAD   ");
				break;
			case PT_DYNAMIC: sprintf(text2,"PT_DYNAMIC");
				break;
			case PT_INTERP:	sprintf(text2,"PT_INTERP ");
				break;
			case PT_NOTE:  sprintf(text2,"PT_NOTE   ");
				break;
			case PT_SHLIB:	sprintf(text2, "PT_SHLIB  ");
				break;
			case PT_PHDR:	sprintf(text2, "PT_PHDR   ");
				break;
			default:	sprintf(text2,"UNKNOWN   ");
				break;
		}
		sprintf(text, "%08X %08X %08X  %08X %08X %08X ",
						pHeader->p_paddr, pHeader->p_vaddr, pHeader->p_offset, pHeader->p_filesz,
						pHeader->p_memsz, pHeader->p_align);
	
	if (pHeader->p_flags & PF_R)
		strcat(text,"R");
	else 
		strcat(text," ");
	if (pHeader->p_flags & PF_W)
		strcat(text, "W");
	else 
		strcat(text," ");
	if (pHeader->p_flags & PF_X)
		strcat(text, "X");
	else 
		strcat(text," ");
	strcat(text, "   ");
	//Type goes last.
	strcat(text, text2);	
	return text;
}
