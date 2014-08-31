# Microsoft Developer Studio Project File - Name="CowBite" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=CowBite - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "CowBite.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "CowBite.mak" CFG="CowBite - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "CowBite - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "CowBite - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "CowBite - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /Gr /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib dsound.lib dxguid.lib comctl32.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "CowBite - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /Gr /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib dsound.lib dxguid.lib comctl32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "CowBite - Win32 Release"
# Name "CowBite - Win32 Debug"
# Begin Group "CowBite Core"

# PROP Default_Filter "*.cpp"
# Begin Group "Header Files"

# PROP Default_Filter "*.h"
# Begin Source File

SOURCE=.\ALU.h
# End Source File
# Begin Source File

SOURCE=.\ARM.h
# End Source File
# Begin Source File

SOURCE=.\ARMBranches.h
# End Source File
# Begin Source File

SOURCE=.\ARMDataOp.h
# End Source File
# Begin Source File

SOURCE=.\ARMDebugger.h
# End Source File
# Begin Source File

SOURCE=.\ARMLoadStore.h
# End Source File
# Begin Source File

SOURCE=.\ARMThumb.h
# End Source File
# Begin Source File

SOURCE=.\Audio.h
# End Source File
# Begin Source File

SOURCE=.\BIOS.h
# End Source File
# Begin Source File

SOURCE=.\Constants.h
# End Source File
# Begin Source File

SOURCE=.\ELFFile.h
# End Source File
# Begin Source File

SOURCE=.\GBA.h
# End Source File
# Begin Source File

SOURCE=.\GBADebugger.h
# End Source File
# Begin Source File

SOURCE=.\Graphics.h
# End Source File
# Begin Source File

SOURCE=.\IO.h
# End Source File
# Begin Source File

SOURCE=.\Memory.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\Stabs.h
# End Source File
# Begin Source File

SOURCE=.\Support.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\ALU.cpp
# End Source File
# Begin Source File

SOURCE=.\ARM.cpp
# End Source File
# Begin Source File

SOURCE=.\ARMBranches.cpp
# End Source File
# Begin Source File

SOURCE=.\ARMDataOp.cpp
# End Source File
# Begin Source File

SOURCE=.\ARMDebugger.cpp
# End Source File
# Begin Source File

SOURCE=.\ARMLoadStore.cpp
# End Source File
# Begin Source File

SOURCE=.\ARMThumb.cpp
# End Source File
# Begin Source File

SOURCE=.\Audio.cpp
# End Source File
# Begin Source File

SOURCE=.\ELFFile.cpp
# End Source File
# Begin Source File

SOURCE=.\GBA.cpp
# End Source File
# Begin Source File

SOURCE=.\GBADebugger.cpp
# End Source File
# Begin Source File

SOURCE=.\Graphics.cpp
# End Source File
# Begin Source File

SOURCE=.\IO.cpp
# End Source File
# Begin Source File

SOURCE=.\Memory.cpp
# End Source File
# Begin Source File

SOURCE=.\Stabs.cpp
# End Source File
# Begin Source File

SOURCE=.\StatsGui.cpp

!IF  "$(CFG)" == "CowBite - Win32 Release"

# ADD CPP /O2

!ELSEIF  "$(CFG)" == "CowBite - Win32 Debug"

# SUBTRACT CPP /Fr

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Support.cpp
# End Source File
# End Group
# Begin Group "Kreed's 2xSai"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\2xsai.cpp
# End Source File
# Begin Source File

SOURCE=.\2xsai.h
# End Source File
# End Group
# Begin Group "Gui"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\BreakpointGui.cpp
# End Source File
# Begin Source File

SOURCE=.\Console.cpp
# End Source File
# Begin Source File

SOURCE=.\Console.h
# End Source File
# Begin Source File

SOURCE=.\ConsoleGui.cpp
# End Source File
# Begin Source File

SOURCE=.\DebugGui.cpp
# End Source File
# Begin Source File

SOURCE=.\DisplayGui.cpp
# End Source File
# Begin Source File

SOURCE=.\DumpGui.cpp
# End Source File
# Begin Source File

SOURCE=.\Gui.cpp
# End Source File
# Begin Source File

SOURCE=.\Gui.h
# End Source File
# Begin Source File

SOURCE=.\Gui.rc
# End Source File
# Begin Source File

SOURCE=.\HardwareRegGui.cpp
# End Source File
# Begin Source File

SOURCE=.\InputGui.cpp
# End Source File
# Begin Source File

SOURCE=.\MapGui.cpp
# End Source File
# Begin Source File

SOURCE=.\ObjGui.cpp
# End Source File
# Begin Source File

SOURCE=.\PaletteGui.cpp
# End Source File
# Begin Source File

SOURCE=.\RegistersGui.cpp
# End Source File
# Begin Source File

SOURCE=.\SourceGui.cpp
# End Source File
# Begin Source File

SOURCE=.\SourceGui.h
# End Source File
# Begin Source File

SOURCE=.\TileGui.cpp
# End Source File
# Begin Source File

SOURCE=.\VariablesGui.cpp
# End Source File
# Begin Source File

SOURCE=.\WindowsInterface.cpp
# End Source File
# Begin Source File

SOURCE=.\WindowsInterface.h
# End Source File
# Begin Source File

SOURCE=.\WinMain.cpp
# End Source File
# End Group
# End Target
# End Project
