CowBite Source Distribution
by Tom Happ

This is the source distribution for CowBite.  I'm making all of my source
available under GPL; you can read more about it in the file gpl.txt.  This
program also makes use of Kreed's 2xsai algorithm, which is public domain.



COMMENTS
---------

- I'm releasing the source because I no longer have enough time to devote
to this project.  I will likely take it up again at some future point, 
but for the time being, I just can't afford to spend any more time on it.

- If anyone wishes to contribute bugfixes or implement additional features,
I will be able to offer a little advice, but I don't have time to help you
debug pointer errors or to coordinate a "CowBite team"; my hands are
already full with too many other things.:(

- I am not claiming that this is the best, most stable code out there.
It began as a hobby project and thus there is a certain irreverance to it.
However, I do think there is great potential for this to become a very
powerful development tool if there is enough interest.



HOW TO COMPILE
--------------

This program was written using MSVC++ 6.0.  These instructions assume you
are using that compiler.

Unzip and untar the contents.  They should unzip to a directory
called "cbsrc" by default.

Double click on "CowBite.dsw".  The workspace should open in Visual Studio.
The files are listed under the "files" tab on the left.

You may have to set up your library paths.  CowBite uses an old version of
microsoft's Direct Sound (the version included in Visual Studio; 3, I believe).
It is likely that you will need to make changes if you want to get it
to compile and link with a different version of the DirectX libraries.

You need to have CBBIOS.bin in the same folder as Cowbite to run.  Because
VS will put CowBite.exe in the "Debug" or "Release" folder, you will need
to copy this file to those directories before running.  

"CowBite Core" files are those which comprise the main part of the engine, i.e.
the internals of the GBA emulation.

"Gui" files mostly contain windows callbacks, plus some auxilliary functions
to help with various taks.  WinMain is also located here.

"Kreed's 2xSai" gets its own folder.  I did not write most of this code, though
I may have changed some variable names to make them mesh better with my
naming preferences.  Thanks to Kreed for coming up with such great filter algorithms.



HOW TO SEND ME UPDATES
----------------------
If you want to contribute to *this* source distribution (cowbite.emuunlim.com),
I would prefer you did the following:


- Download the source onto your own machine; get it to compile on its own and
make sure it runs normally.

- Make whatever changes or additions you see fit, keeping note of what
function and what files you modified, and commenting your changes

- Make sure that your correction/addition compiles and runs as expected.

- Send the update to me with explicit instructions on what needs to go where
in order for the update to work.  If it's easier to just replace a whole 
source file, let me know.  Otherwise make sure it's obvious where a correction
goes.

- I'll look at the code and if it seems like a good addition to me, I'll add it
and give you credit.:)

- CowBite is a debugger first.  I have no interest in adding any features
that I feel will facilitate the playing of illegal ROMS or that serve no other
purpose than to be used with such ROMS.  If you do this with your own copy, I 
can't stop you, but I won't add it to this distro.

- I am very busy . . . please be understanding if I don't get around to it
right away.  Chances are I won't look at it until the weekend.

- Remember that this is under the GPL.  You are free to make your own distributions
as long as you abide by the license.


TODO
----
Now, here's a list of things that need updating:


- "Debug Mode" toolbar buttons (they are supposed to work like those in
VC++ and insight).  I've already implemented a couple of them.  The rest
should be fairly easy to add (I think).

- The disassembler, register viewer, and memory viewer all need to
be implemented (I may do this myself, actually, so you can probably survive
with the version VII debugger)

- There is some bug (possibly related to interrupts CPSR/SPSR mode
switching) that I introduced recently and makes a lot of games behave
strangely.

- Various source debugging flaws.

- Anything in the CowBite Spec that says "Unimplemented in CowBite" needs
to be implemented at some point.

- IO Registers Viewer needs to be updated with the most recent feedback
from the CowBite specification doc.

- An option to turn off sound :)

- The FM synthesis channels (1-4) need to be implemented

- A clicking noise is sometimes apparent in games that use the directsound
channels.

- The "Moo" button

- Currently the emulator runs a lot slower in debug mode.  It would be
good to turn up breakpoint efficiency (perhaps with a better searching alg)

- I wanted to add an emulator-specific IO register meant to perform
various debugging tasks, but haven't gotten to adding it.  Some things I wanted
it to do:
	- Determine if the code is running in CowBite
	- Have the ability to shut down CowBite with an error message
	(for commercial roms)
	- Set a breakpoint
	- Print a string to the console or to a log file
	- Dump memory to a set file (with imposed limits for security reasons)
	- Enable or disable certain features in cowbite
	- Do your taxes

- There are lots of old bugs that I just never got around to fixing.  


CONTACT
-------
Tom Happ
SorcererXIII@yahoo.com
http://cowbite.emuunlim.com
