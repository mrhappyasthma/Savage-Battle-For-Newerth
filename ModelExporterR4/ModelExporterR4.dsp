# Microsoft Developer Studio Project File - Name="ModelExporterR4" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=ModelExporterR4 - Win32 Hybrid
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ModelExporterR4.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ModelExporterR4.mak" CFG="ModelExporterR4 - Win32 Hybrid"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ModelExporterR4 - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ModelExporterR4 - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ModelExporterR4 - Win32 Hybrid" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ModelExporterR4 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /YX /FD /c
# ADD CPP /nologo /G6 /MD /W3 /O2 /I "C:\3dsmax42\maxsdk\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /YX /FD /LD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib bmm.lib core.lib geom.lib gfx.lib mesh.lib maxutil.lib maxscrpt.lib gup.lib paramblk2.lib /nologo /base:"0x62e40000" /subsystem:windows /dll /machine:I386 /out:"C:\SavageRealms\ModelExporterR4.dlu" /libpath:"C:\3dsmax42\maxsdk\lib" /release

!ELSEIF  "$(CFG)" == "ModelExporterR4 - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /YX /FD /GZ /c
# ADD CPP /nologo /G6 /MDd /W3 /Gm /ZI /Od /I "C:\3dsmax42\maxsdk\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /FR /YX /FD /LD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib bmm.lib core.lib geom.lib gfx.lib mesh.lib maxutil.lib maxscrpt.lib gup.lib paramblk2.lib /nologo /base:"0x62e40000" /subsystem:windows /dll /debug /machine:I386 /out:"c:\savagerealms\ModelExporterR4.dlu" /pdbtype:sept /libpath:"C:\3dsmax42\maxsdk\lib"

!ELSEIF  "$(CFG)" == "ModelExporterR4 - Win32 Hybrid"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "ModelExporterR4___Win32_Hybrid"
# PROP BASE Intermediate_Dir "ModelExporterR4___Win32_Hybrid"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "ModelExporterR4___Win32_Hybrid"
# PROP Intermediate_Dir "ModelExporterR4___Win32_Hybrid"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /G6 /MD /W3 /Gm /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_S2_EXPORTER" /D "_S2_DONT_INCLUDE_OS" /D "_S2_DONT_INCLUDE_GL" /FR /YX /FD /LD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib bmm.lib core.lib geom.lib gfx.lib mesh.lib maxutil.lib maxscrpt.lib gup.lib paramblk2.lib /nologo /base:"0x62e40000" /subsystem:windows /dll /debug /machine:I386 /out:"C:\Savage\plugins\ModelExporterR4.dlu" /pdbtype:sept /libpath:"C:\3dsmax42\maxsdk\lib"

!ENDIF 

# Begin Target

# Name "ModelExporterR4 - Win32 Release"
# Name "ModelExporterR4 - Win32 Debug"
# Name "ModelExporterR4 - Win32 Hybrid"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\DllEntry.cpp
# End Source File
# Begin Source File

SOURCE=.\mem.c
# End Source File
# Begin Source File

SOURCE=..\3dsMaxShared\meshtools.cpp
# End Source File
# Begin Source File

SOURCE=.\ModelExporterR4.cpp
# End Source File
# Begin Source File

SOURCE=.\ModelExporterR4.def
# End Source File
# Begin Source File

SOURCE=.\ModelExporterR4.rc
# End Source File
# Begin Source File

SOURCE=..\Core\savage_common.c
# End Source File
# Begin Source File

SOURCE=..\Core\savage_mathlib.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\ModelExporterR4.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Source File

SOURCE=..\..\..\3dsmax42\maxsdk\Include\baseinterface.h
# End Source File
# Begin Source File

SOURCE=..\..\..\3dsmax42\maxsdk\Include\ptrvec.h
# End Source File
# End Target
# End Project
