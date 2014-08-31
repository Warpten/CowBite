/*
    CowBite VIIIpa2, GBA Emulator/Debugger 
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

Requirements:
-------------
- Windows 98/NT/2K/XP (untested on other windows)
- 16 bit (high color) display
- PII/PIII/Athlon CPU

Recommended:
- Windows 2K
- Resolution of 1024 x 768 or higher
- 800 mhz or more

ALSO NOTE:  
As you update CowBite, you may want to remove the old "CowBite.cfg" file
created by older versions of CowBite.  Forget my earlier message that this
would be fixed for CowBite VIIa.  It isn't, yet.  Patience.:)


Usage:
------

CowBite [romname]
cowbite.sh [romname]	#For cygwin users


Instructions:
-------------
THE BASICS
The following were written for CowBite VII.  CowBite VIII will operate slightly
different once I get it going.  Until I complete the interface and write it up,
I leave it to the user to figure out what has changed.

To play a ROM, click the "load" button and choose a file.  Unless CowBite
is paused or in Debug mode, the ROM will begin playing.  You can set your
keyboard and joystick controls using the input menu.  There are a variety of 
other basic features like screen size (I, II, or III), interpolation
(using Kreed's Super Eagle or 2xSaI), frameskip, and save/load states.
Probably you can figure all of these out just by clicking around.  

ENTER DEBUG MODE (MOSTLY UNIMPLEMENTED)
If you have this selected, you're in debug mode.  CowBite will ignore
breakpoints if not in debug mode.

DYNAMIC REFRESH
Choose whether and how often you want your debug views to refresh automatically.
Be sure to give this option a try.  You'd be surprised at how good your eye
is at catching problems as they flash past in a debug window.  
That is why this is here.

OBJ TILES
Lets you view obj tiles as they appear in memory.

OBJS
Lets you view objs or sprites.

BG TILES
This lets you view tiles as they appear in memory.

BG MAPS
The map viewer allows you to view maps either statically (with 0,0 located
in the upper left) or to render them using the graphics engine.  This
option is especially useful when Dynamic Refresh is enabled, allowing you
to view how tiles are being loaded in outside the visible area of the screen.

SOURCE
If you open an ELF file that was compiled with the -gstabs option, CowBite
can load your source, allowing you to view it normally or mixed with Asm.
CowBite looks for the source relative to the directory the ELF file is in.
Let me know if you have issues with CowBite being unable to find or open source
files.

VARIABLES
This allows you to browse local, static, and global variables, functions,
and any other ELF or stabs symbols embedded in your executable.

CONSOLE
This is a new feature.  You can view console output (dprint is now supported)
by choosing "View Console" from the debug menu.  Alternately, if you start
CowBite in cygwin using the "cowbite.sh" script, the output will also go to
stdout, which you can view in your cygwin window or pipe into other cygwin tools.

CONDITIONAL BREAKPOINTS
In the Advanced Breakpoints dialog, you can set breakpoints on several
simple conditions like "register == register" or "*address == constant".

VIEW HARDWARE REGISTERS
This option lets you view the GBA registers in a fashion similar to Mappy's
register viewer.  In the future it will also provide additional information
specific to each individual register (i.e. the meaning of the various bit
flags, etc.)

VIEW STATISTICS
As CowBite runs it collects statistics on interrupts, memory accesses,
DMA transfers, sound, etc.  This window is designed to present this information
in a concise format.

VERSION VII DEBUGGER
In version VII, all of the ASM debugging features were part of one window.
In later versions they will be separated into individual windows.  However,
for those who want everything in one place, the old debugger will be left in
for backwards compatibility.


About ELF and stabs
-------------------
If you're using gcc, you can compile programs with the -gstabs option in order
to get stabs symbol support.  Probably you have gcc set up to create
ELF files by default and then convert it to raw binary format using "objcopy",
but you may need to edit your makefile so as not to delete the original ELF
after the conversion has been made.  ELF files made with -gstabs contain extra
information that allows CowBite to perform source level debugging.

Issues with ELF and stabs
-------------------------
- I'm trying my best to make CowBite as compatible as possible with the ELF files produced
by different devkits, but it is obviously biased towards my own test platform.  For
reference, I usually write my test programs using Emanuel's HAM devkit, simply because it
is easy to install and the compiler and makefiles come set up and ready to go.

-I have also gotten CowBite to work with devkit advance using the -mthumb compiler option.
Other things I have tried produced some very strange and unusual results, though I am
still working on it.  If you can't get source debugging to work, PLEASE e-mail me at
SorcererXIII@yahoo.com.  If you zip an example project directory together and mail the
whole thing to me it makes it a bit easier (you have my word that I will NOT steal your
source or your ideas).

-You will likely get better debug output it you disable optomiziations.  But don't forget
to turn them on (and to disable -gstabs) for your final build.

-There may be a problem with ELF files compiled for multiboot support.  Let me know if
you encounter issues with this.



Unimplemented:
-----------------------
- Many BIOS calls
- Sound 1 - 4
- Keyboard interrupts
- Anything to do with link cables
- Disallowing ROM writes
- EEPROM
- Correct CPU cycle timings


Known Bugs:
-----------------------
- Problems loading anything with a space in the path
- Sprite and background bugs (i.e. in some demos they update strangely
or just looked messed up.)  If you think you know why this is, let me know.
- FIXED - Source windows sporadically stop working.  I have no clue why this is.

Suspected Bugs:
-----------------------
Flaws in DMA implementation?
Flaws in CPUSet and CPUFastSet



I might have forgotten something or just missed it altogether.
Let me know if you find other things that don't work.


Notice:
-------
Piracy is illegal.  I don't think CowBite can even run any commercial games,
but even so, don't bother trying.

Do NOT ask me for commercial ROMs or where you can find them.  If you mention anything
about these I will just ignore your e-mail.





Special Thanks (in no pariticular order):
---------------
- Kreed (Derek Liauw) for his SuperEagle and Super2xSaI engines
- Agent Q
- All those GBA demo authors. You rock!
- Every other emu author
- www.gbadev.org and everyone involved (Simon especially)
- The gbadev list on yahoo
- Dovoto and the PERN Project
- Jeff Frohwein and his Devrs.com site
- Nocturn and his tutorial
- Uze from BeLogic for all the great information on the GBA's sound!
- Nintendo 
- Robert Gebis (Lots of useful feedback and bug reports)
- Markus (Provided very useful info on LZ77 compression)
- Costis (A variety of new info/corrections)
- Grauw (Info on forced blanking, hblank lenghths, and on the BIOS wait function.)
- Max
- Mike/gbcft (LOTS of info on interrupts and on windowing)
- Otaku
- Yarpen (Almost all the information on the timer registers and the keyboard control register. Thanks!)
- Eloist and the EloGBA emulator, for having open source.  Being able to compare
the EloGBA source to CowBite was invaluable in the beginning stages.
- CoRE (Comptuers, Robotics, and Engineering) at Binghamton U. for a billion things
- Jim Heliotis for having such a cool computer architecture course
