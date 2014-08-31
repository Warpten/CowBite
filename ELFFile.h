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

#ifndef ELFFILE_H
#define ELFFILE_H

#include "Constants.h"

typedef u32 Elf32_Addr;
typedef u32 Elf32_Off;
typedef u16 Elf32_Half;
typedef u32 Elf32_Word;
typedef s32 Elf32_Sword;


///////////////////////
//Most of this information was obtained from
//the web.  There seem to be standardized
//type conventions for ELF and I am sticking
//with those.
///////////////////////
#define EI_NIDENT 16

//////////////////////////////
//The ELF header
//////////////////////////////
typedef struct tElf32_Ehdr{
        unsigned char   e_ident[EI_NIDENT];
        Elf32_Half      e_type;
        Elf32_Half      e_machine;
        Elf32_Word      e_version;
        Elf32_Addr      e_entry;	//process start point
        Elf32_Off       e_phoff;	//program header table offset
        Elf32_Off       e_shoff;	//section header table offset
        Elf32_Word      e_flags;	//processor specific (seems to be 0 for GBA)
        Elf32_Half      e_ehsize;	//Size of this header (52 bytes)
        Elf32_Half      e_phentsize;//Size of program header table entry
        Elf32_Half      e_phnum;	//Number of entries in table (so total size = phensize*phnum)
        Elf32_Half      e_shentsize;//Size of a section header in bytes
        Elf32_Half      e_shnum;	//Number of entries in section header table
        Elf32_Half      e_shtrndx;	//section header table index of the section name string table
} Elf32_Ehdr;

///////////////////////////////////
//Section header
//////////////////////////////////
typedef struct tElf32_Shdr { 
	Elf32_Word sh_name; //Index of name in the section header string table
	Elf32_Word sh_type; 
	Elf32_Word sh_flags; 
	Elf32_Addr sh_addr; //If this section goes into memory, the address of the first byte
	Elf32_Off sh_offset; //Offset to section (from beginning of file)
	Elf32_Word sh_size; //Section size in bytes
	Elf32_Word sh_link; //section header table index link
	Elf32_Word sh_info; 
	Elf32_Word sh_addralign; 
	Elf32_Word sh_entsize; //Size of entries in the section (only if a fixed size)
} Elf32_Shdr;


///////////////////////////////
//Symbol Table Entry
//////////////////////////////
typedef struct tElf32_Sym{
	Elf32_Word      st_name;	//index of name in symbol string table,
	Elf32_Addr      st_value;	//Value of the symbol (in an executable, this is a virtual address)
	Elf32_Word      st_size;	//Size (in bytes) of the symbol
	unsigned char   st_info;	//Type and binding
	unsigned char   st_other;	//has no meaning
	Elf32_Half      st_shndx;	//relevant section header table index
} Elf32_Sym;


//////////////////////////
//Relocation entry without explicit addend
//////////////////////////
typedef struct Elf32_Rel{
       Elf32_Addr      r_offset;
       Elf32_Word      r_info;
} Elf32_Rel;

///////////////////////////
//Relocation entry with explicit addend
///////////////////////////
typedef struct tElf32_Rela{
       Elf32_Addr      r_offset;
       Elf32_Word      r_info;
       Elf32_Sword     r_addend;
} Elf32_Rela;

////////////////////////////////
//Program header
////////////////////////////////
typedef struct tElf32_Phdr {
       Elf32_Word      p_type;
       Elf32_Off       p_offset;	//Offset of first byte (from beginning of file)
       Elf32_Addr      p_vaddr;		//Address of this byte in virtual memory
       Elf32_Addr      p_paddr;		//Address of this byte in physical memory
       Elf32_Word      p_filesz;	//Number of bytes in file of this image
       Elf32_Word      p_memsz;		//Number of bytes in memory usage of segment
       Elf32_Word      p_flags;		//Flags 
       Elf32_Word      p_align;		//Alignment
} Elf32_Phdr;

//////////////////////////////
//Stabs entry
//////////////////////////////
typedef struct tStabEntry{
	u32 n_strx;	//offset of string
    u8 n_type;	//Type
	u8 n_other;		//Misc info (usually empty)
	u16 n_desc;		//Description field
    u32 n_value;	//Value of symbol
} StabEntry;



typedef struct tELFFile {
	Elf32_Ehdr header;
	Elf32_Shdr* sectionHeaders;
	Elf32_Phdr* programHeaders;
	char* sectionHeaderStringTable;	//All of the strings in one big array
	char** sectionStrings;	//This references the above, but with indices.

	Elf32_Shdr* symbolHeader;
	char* stringTable;
	Elf32_Sym* symbolTable;


	//Stab stuff
	Elf32_Shdr* stabHeader;
	StabEntry* stabTable;		//For .stab debug data.
	char* stabStringTable;		//For .stab debug strings

	//The following all relate to Elf symbols
	u32 numSymbols;	//Total symbols.
	u32 numNoTypeSymbols;
	u32 numObjectSymbols;
	u32 numFuncSymbols;
	u32 numSectSymbols;
	u32 numFileSymbols;
	u32 numLabelSymbols;
	u32 numOtherSymbols;
	//Arrays of symbols organized by type
	Elf32_Sym* noTypeSymbols;
	Elf32_Sym* objectSymbols;
	Elf32_Sym* funcSymbols;
	Elf32_Sym* sectSymbols;
	Elf32_Sym* fileSymbols;
	Elf32_Sym* labelSymbols;
	Elf32_Sym* otherSymbols;



	//u8** programData;		//Array of pointers to program sections
	u8* romData;			//All of the data that goes into ROM.
	u32 romSize;
	u32 fileSize;
	u32 loaded;
} ELFFile;

ELFFile* ELFFile_load(ELFFile* elfFile, char* filename);
char* ELFFile_getFunctionName(ELFFile* elfFile, u32 address);
char* ELFFile_getSymbolName(ELFFile* elfFile, Elf32_Sym* symbol);
char* ELFFile_getSymbolName(ELFFile* elfFile, u32 address);
char* ELFFile_getInfo(Elf32_Sym* symbol);
char* ELFFile_getSectionInfo(ELFFile* elfFile, Elf32_Shdr* sHeader);
char* ELFFile_getProgramInfo(ELFFile* elfFile, Elf32_Phdr* pHeader);
Elf32_Sym* ELFFile_findByName(ELFFile* elfFile, char* name);
void ELFFile_delete(ELFFile* elfFile);


///////////////////////////////////////
//The following are indexes into e_ident.
///////////////////////////////////////
#define EI_MAG0 0 //'0x7F'
#define EI_MAG1 1 //'E'
#define EI_MAG2 2 //'L'
#define EI_MAG3 3 //'F'
#define EI_CLASS 4 //File class
#define EI_DATA 5 //Data encoding
#define EI_VERSION 6 //File version
#define EI_OSABI 7 //Operating system/ABI identification
#define EI_ABIVERSION 8 //ABI version 
#define EI_PAD 9 //Start of padding bytes 

////////////////////////////////////
//THe following are values for the EI_CLASS entry
//////////////////////////////
#define ELFCLASSNONE 0 //Invalid class 
#define ELFCLASS32 1 //32-bit objects 
#define ELFCLASS64 2 //64-bit objects

/////////////////////////////////
//Data encoding type (for EI_DATA)
/////////////////////
#define ELFDATANONE 0 //Invalid data encoding 
#define ELFDATA2LSB 1 //
#define ELFDATA2MSB 2 //

//ELFOSABI fields (next to useless)
#define ELFOSABI_SYSV 0 // UNIX System V ABI
#define ELFOSABI_HPUX 1 //HP-UX operating system 
#define ELFOSABI_STANDALONE 255 //Standalone (embedded) application

////////////////////////////////////////
//The following are values of the e_type field
////////////////////////////////////////
#define ET_NONE 0 //No file type
#define ET_REL 1 //Relocatable file
#define ET_EXEC 2 //Executable file
#define ET_DYN 3 //Shared object file
#define ET_CORE 4 //Core file 
#define ET_LOOS 0xfe00 //Operating system-specific
#define ET_HIOS 0xfeff //Operating system-specific 
#define ET_LOPROC 0xff00 //Processor-specific 
#define ET_HIPROC 0xffff //Processor-specific

//////////////////////////////////////////
//The following are values for the e_machine field
//////////////////////////////////////////
#define EM_NONE 0 //No machine
#define EM_M32 1 //AT&T WE 32100 
#define EM_SPARC 2 //SPARC 
#define EM_386 3 //Intel 80386 
#define EM_68K 4 //Motorola 68000 
#define EM_88K 5 //Motorola 88000 
#define EM_860 7 //Intel 80860 
#define EM_MIPS 8 //MIPS I Architecture 
#define EM_MIPS_RS3_LE 10 //MIPS RS3000 Little-endian 
#define EM_PARISC 15 //Hewlett-Packard PA-RISC 
#define EM_VPP500 17 //Fujitsu VPP500 
#define EM_SPARC32PLUS 18 //Enhanced instruction set 
#define SPARC EM_960 19 //Intel 80960 EM_PPC 20 Power PC 
#define EM_V800 36 //NEC V800 
#define EM_FR20 37 //Fujitsu FR20  
#define EM_RH32 38 //TRW RH-32 
#define EM_RCE 39 //Motorola RCE 
#define EM_ARM 40 //Advanced RISC Machines ARM 
#define EM_ALPHA 41 //Digital Alpha 
#define EM_SH 42 //Hitachi SH 
#define EM_SPARCV9 43 //SPARC Version 9 
#define EM_TRICORE 44 //Siemens Tricore embedded processor 
#define EM_ARC 45 //Argonaut RISC Core, Argonaut Technologies Inc. 
#define EM_H8_300 46 //Hitachi H8/300 
#define EM_H8_300H 47 //Hitachi H8/300H 
#define EM_H8S 48 //Hitachi H8S 
#define EM_H8_500 49 //Hitachi H8/500 
#define EM_IA_64 50 //Intel MercedTM Processor 
#define EM_MIPS_X 51 //Stanford MIPS-X 
#define EM_COLDFIRE 52 //Motorola Coldfire 
#define EM_68HC12 53 //Motorola M68HC12

////////////////////////////////////////
//The following are values for the e_version field
////////////////////////////////////////
#define EV_NONE 0
#define EV_CURRENT 1

///////////////////////////////////////////
//Section types (for sh_type)
//////////////////////////////////////////
#define SHT_NULL 0		//Section header is inactive
#define SHT_PROGBITS 1	//Program specific data
#define SHT_SYMTAB 2	//Info used for linking
#define SHT_STRTAB 3	//A string table
#define SHT_RELA 4		//Relocation entries with explicit addends
#define SHT_HASH 5		//A symbol hash table
#define SHT_DYNAMIC 6	//Symbol table used for dynamic linking
#define SHT_NOTE 7		//A note.
#define SHT_NOBITS 8	//A section that contains no bytes
#define SHT_REL 9		//Relocation entries without explicit addends
#define SHT_SHLIB 10	//Reserved
#define SHT_DYNSYM 11	//Symbol table used for dynamic linking
#define SHT_LOOS 0x60000000		//System specific start
#define SHT_HIOS 0x6fffffff //System specific end
#define SHT_LOPROC 0x70000000 //System specific start
#define SHT_HIPROC 0x7fffffff //System specific end
#define SHT_LOUSER 0x80000000 //Application specific start
#define SHT_HIUSER 0xffffffff //Application specific end

//Notes
//for SHT_DYNAMIC sh_link = The section header index of the string table used by entries in the section.
//for SHT_HASH, sh_link = The section header index of the symbol table to which the hash table applies.
//for SH_REL and SH_REL, sh_link = The section header index of the associated symbol table,
//	sh_info = The section header index of the section to which the relocation applies.
//for SH_SYMTAB and SH_DYNSYM, sh_link = The section header index of the associated string table.
//	sh_info = One greater than the symbol table index of the last local symbol (binding STB_LOCAL).

////////////////////////
//Values for sh_flags
//////////////////////
#define SHF_WRITE 0x1	//Section data should be writable by program
#define SHF_ALLOC 0x2	//If this section resides in memory
#define SHF_EXECINSTR ox4	//If the section contains executable machine instructions
#define SHF_MASKOS 0x0f000000 //Reserved
#define SHF_MASKPROC 0xf0000000	//Reserved


//THese are used for the type and binding of symbol table entries (st_info)
#define ELF32_ST_BIND(i)   ((i)>>4)
#define ELF32_ST_TYPE(i)   ((i)&0xf)
#define ELF32_ST_INFO(b,t) (((b)<<4)+((t)&0xf))

//These are binding values for st_info - use macros above
#define STB_LOCAL 0		//symbols not visible outside the obj containing the definition
#define STB_GLOBAL 1	//Symbols visible to all object files
#define STB_WEAK 2		//same as global but with lower precedence
#define STB_LOOS 10		//operating system specific start
#define STB_HIOS 12		//operating specific end
#define STB_LOPROC 13	//processor specific start
#define STB_HIPROC 15	//processor specific end

//These are the type values (for st_info) - use macros above
#define STT_NOTYPE 0	//NOt specified
#define STT_OBJECT 1	//Symbol is a data object (variable, array, etc.)
#define STT_FUNC 2		//Symbol is a function
#define STT_SECTION 3	//Associated with a section (usually STB_LOCAL binding)
#define STT_FILE 4		//The name of the source file associated with the object file
#define STT_LOOS 10		//reserved (operating system specific
#define STT_HIOS 12		//reserved
#define STT_LOPROC 13	//reserved (hardware specific)
#define STT_HIPROC 15	//reserved
#define STT_GBAFUNC 13	//Seems to be what the GBA uses to specify functions
#define STT_GBALABEL 15	//What the GBA uses for labels

///////////////////////////////////////
//Segment types
////////////////////////////////////////
#define PT_NULL 0	//Means ignore this entry
#define PT_LOAD 1	//Load this entry into memory, padding with zeros where needed
#define PT_DYNAMIC 2	 //Dynamic linking info
#define PT_INTERP 3		//Means this array element specifies location of an interpereter for the program
#define PT_NOTE 4		//This array specifies locatio and size of aux. information
#define PT_SHLIB 5	//Reserved
#define PT_PHDR 6	//If the program header table appears in the memory
					//of the image, this is the location and size of the
					//program header table.
#define PT_LOOS 0x60000000		//reserved
#define PT_HIOS 0x6fffffff		//reserved
#define PT_LOPROC 0x70000000	//reserved
#define PT_HIPROC 0x7fffffff	//reserved

//Values of p_flags in the program header
//These may be loosely interpereted (see ELF docs)
#define PF_X 0x1 //Execute 
#define PF_W 0x2 //Write 
#define PF_R 0x4 //Read 
#define PF_MASKOS 0x0ff00000 //Unspecified 
#define PF_MASKPROC 0xf0000000 //Unspecified


//Values for the n_type entry of stabs.

//NON STAB TYPES:
#define N_UNDF	0x0 //Undefined symbol
#define N_ABS	0x2 //File scope absolute symbol
#define N_EXT	0x3
#define N_TEXT	0x4 //File scope text symbol
#define N_EXT2	0x4 //External text symbol
#define N_DATA  0x6 //File scope data symbol
#define N_EXT3	0x7 //    External data symbol
#define N_BSS	0x8 //    File scope BSS symbol
#define N_EXT4	0x9	//    External BSS symbol
#define N_FN_SEQ 0xC //    Same as N_FN, for Sequent compilers
#define N_INDR 0xA	//    Symbol is indirected to another symbol
#define N_COMM	0x12	//Common--visible after shared library dynamic link
#define N_SETA	0x14	//Absolute set element
#define N_SETT	0x16	//Text segment set element
#define N_SETD	0x18	//Data segment set element
#define N_SETB	0x1a	//BSS segment set element
#define N_SETV  0x1c	//Pointer to set vector
#define N_WARNING 0x1E	//Print a warning message during linking
#define N_FN	0x1F	//File name of a `.o' file

////////////////////////
//Here is a summary of the different sections defined in the linkscript,
//which someone posted in gbadev, which I thought was easier to understand
//than docs on the web.
/////////////////////////
/*
.data --> initialized globals... initialization data resides in ROM and same
amount of RAM is reserved. Data is copied to RAM by crt0.s.
.rodata --> ROM data (const data).
.bss --> globals... no init data, just reserve RAM
.text --> program... resides in ROM
*/


#endif

