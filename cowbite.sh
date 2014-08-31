#!/bin/sh
######################################################################
#cowbite.sh
#by Tom Happ
#
#Use this script in Cygwin to start up CowBite.
#Because CowBite is a win32 application, writing to stdout is tricky -
#by default Windows grabs it from you, even if you run your app in
#a console window (I'm still trying to find a way around this, 
#but it's looking bleak).  However, Cygwin's pipes can grab the output
#back again and put it where you need it.  Use this simple script with
#cygwin to get more intuitive console functionality out of CowBite.
######################################################################

#Uncomment and adjust the line below if you would like to set
#the full path to CowBite in this script (rather than using
#your PATH environment variable)

#FULLCBPATH="/cygdrive/c/Emulators/CowBite/Debug/"


CBEXECUTABLE=CowBite.exe
CB="$FULLCBPATH$CBEXECUTABLE $*"

$CB | while read line
do
	echo "$line"
done