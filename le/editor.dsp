# Microsoft Developer Studio Project File - Name="editor" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=editor - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "editor.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "editor.mak" CFG="editor - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "editor - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "editor - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "editor - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "EDITOR_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "EDITOR_EXPORTS" /D "EDITOR_DLL" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /map /machine:I386 /out:"..\..\clean_build\editor\game.dll"

!ELSEIF  "$(CFG)" == "editor - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "editor___Win32_Debug"
# PROP BASE Intermediate_Dir "editor___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "editor___Win32_Debug"
# PROP Intermediate_Dir "editor___Win32_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "EDITOR_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /Gi /GR /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "EDITOR_EXPORTS" /D "LEVEL_EDITOR" /D "EDITOR_DLL" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /profile /map /debug /machine:I386 /out:"../../clean_build/editor/game_debug.dll"

!ENDIF 

# Begin Target

# Name "editor - Win32 Release"
# Name "editor - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "GUI Widgets"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\gui\gui_brushbutton.c
# End Source File
# Begin Source File

SOURCE=..\gui\gui_brushgrid.c
# End Source File
# Begin Source File

SOURCE=..\gui\gui_button.c
# End Source File
# Begin Source File

SOURCE=..\gui\gui_floater.c
# End Source File
# Begin Source File

SOURCE=..\gui\gui_graphic.c
# End Source File
# Begin Source File

SOURCE=..\gui\gui_label.c
# End Source File
# Begin Source File

SOURCE=..\gui\gui_menu.c
# End Source File
# Begin Source File

SOURCE=..\gui\gui_slider.c
# End Source File
# Begin Source File

SOURCE=..\gui\gui_spinner.c
# End Source File
# Begin Source File

SOURCE=..\gui\gui_swatch.c
# End Source File
# Begin Source File

SOURCE=..\gui\gui_textbox.c
# End Source File
# Begin Source File

SOURCE=..\gui\gui_thumbnailgrid.c
# End Source File
# Begin Source File

SOURCE=..\gui\gui_togglebutton.c
# End Source File
# Begin Source File

SOURCE=..\gui\gui_track.c
# End Source File
# End Group
# Begin Source File

SOURCE=..\Core\camerautils.c
# End Source File
# Begin Source File

SOURCE=.\editor.def
# End Source File
# Begin Source File

SOURCE=.\gui_classes.c
# End Source File
# Begin Source File

SOURCE=.\le_camera.c
# End Source File
# Begin Source File

SOURCE=.\le_draw.c
# End Source File
# Begin Source File

SOURCE=.\le_drawutils.c
# End Source File
# Begin Source File

SOURCE=.\le_main.c
# End Source File
# Begin Source File

SOURCE=.\le_mem.c
# End Source File
# Begin Source File

SOURCE=.\le_misc.c
# End Source File
# Begin Source File

SOURCE=.\le_modelmode.c
# End Source File
# Begin Source File

SOURCE=.\le_objects.c
# End Source File
# Begin Source File

SOURCE=.\le_occluder.c
# End Source File
# Begin Source File

SOURCE=.\le_pool.c
# End Source File
# Begin Source File

SOURCE=.\le_tools.c
# End Source File
# Begin Source File

SOURCE=..\Core\savage_common.c
# End Source File
# Begin Source File

SOURCE=..\Core\savage_mathlib.c
# End Source File
# Begin Source File

SOURCE=..\toplayer\tl_drawutils.c
# End Source File
# Begin Source File

SOURCE=..\toplayer\tl_pool.c
# End Source File
# Begin Source File

SOURCE=..\toplayer\tl_sky.c
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\le_occluder.h
# End Source File
# End Group
# End Target
# End Project
