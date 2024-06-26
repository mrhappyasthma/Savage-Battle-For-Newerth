# Microsoft Developer Studio Project File - Name="TheGame" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=TheGame - Win32 Demo Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "TheGame.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "TheGame.mak" CFG="TheGame - Win32 Demo Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "TheGame - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TheGame - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TheGame - Win32 Demo Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TheGame - Win32 Demo Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "TheGame - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "THEGAME_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /Zi /Ot /Og /Oi /Op /Oy /Ob1 /Gf /Gy /I "../gui" /I "../TheGame" /I "../toplayer" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "THEGAME_EXPORTS" /D "RAD_NO_LOWERCASE_TYPES" /FR /YX /FD /Gs /Gs /c
# SUBTRACT CPP /Ox /Oa
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib /nologo /dll /profile /map:"../../clean_build/game/game_release.map" /machine:I386 /def:".\TheGame.def" /out:"../../clean_build/game/game.dll" /MAPINFO:LINES /MAPINFO:EXPORTS
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "TheGame - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "THEGAME_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /Gi /vmg /GR /GX /ZI /Od /I "../gui" /I "../TheGame" /I "../toplayer" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "THEGAME_EXPORTS" /FAs /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /dll /map:"../../clean_build/game/game_debug.map" /debug /machine:I386 /def:".\TheGame.def" /out:"..\..\clean_build\game\game_debug.dll" /MAPINFO:LINES /MAPINFO:EXPORTS
# SUBTRACT LINK32 /profile

!ELSEIF  "$(CFG)" == "TheGame - Win32 Demo Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "TheGame___Win32_Demo_Release"
# PROP BASE Intermediate_Dir "TheGame___Win32_Demo_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "TheGame___Win32_Demo_Release"
# PROP Intermediate_Dir "TheGame___Win32_Demo_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /Zi /Ot /Og /Oi /Op /Oy /Ob1 /Gf /Gy /I "../gui" /I "../TheGame" /I "../toplayer" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "THEGAME_EXPORTS" /D "RAD_NO_LOWERCASE_TYPES" /YX /FD /Gs /Gs /c
# SUBTRACT BASE CPP /Ox /Oa
# ADD CPP /nologo /MT /W3 /GX /Zi /Ot /Og /Oi /Op /Oy /Ob1 /Gf /Gy /I "../gui" /I "../TheGame" /I "../toplayer" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "THEGAME_EXPORTS" /D "RAD_NO_LOWERCASE_TYPES" /D "SAVAGE_DEMO" /YX /FD /Gs /Gs /c
# SUBTRACT CPP /Ox /Oa
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib /nologo /dll /pdb:none /map:"../../clean_build/game/game_release.map" /debug /machine:I386 /def:".\TheGame.def" /out:"../../clean_build/game/game.dll" /MAPINFO:LINES /MAPINFO:EXPORTS
# ADD LINK32 kernel32.lib /nologo /dll /pdb:none /machine:I386 /def:".\TheGame.def" /out:"C:\savage\final_builds\demo 2.0\game\game_demo.dll" /MAPINFO:LINES /MAPINFO:EXPORTS
# SUBTRACT LINK32 /map /debug

!ELSEIF  "$(CFG)" == "TheGame - Win32 Demo Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "TheGame___Win32_Demo_Debug"
# PROP BASE Intermediate_Dir "TheGame___Win32_Demo_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "TheGame___Win32_Demo_Debug"
# PROP Intermediate_Dir "TheGame___Win32_Demo_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /Gi /vmg /GR /GX /ZI /Od /I "../gui" /I "../TheGame" /I "../toplayer" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "THEGAME_EXPORTS" /FAs /FR /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /Gi /vmg /GR /GX /ZI /Od /I "../gui" /I "../TheGame" /I "../toplayer" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "THEGAME_EXPORTS" /D "SAVAGE_DEMO" /FAs /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /dll /map:"../../clean_build/game/game_debug.map" /debug /machine:I386 /def:".\TheGame.def" /out:"../../clean_build/game/game_debug.dll" /MAPINFO:LINES /MAPINFO:EXPORTS
# SUBTRACT BASE LINK32 /profile
# ADD LINK32 /nologo /dll /map:"../../clean_build/game/game_debug.map" /debug /machine:I386 /def:".\TheGame.def" /out:"C:\SavageDemo2\game\game_demo_debug.dll" /MAPINFO:LINES /MAPINFO:EXPORTS
# SUBTRACT LINK32 /profile

!ENDIF 

# Begin Target

# Name "TheGame - Win32 Release"
# Name "TheGame - Win32 Debug"
# Name "TheGame - Win32 Demo Release"
# Name "TheGame - Win32 Demo Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "GUI Widgets"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\gui\gui_button.c
# End Source File
# Begin Source File

SOURCE=..\gui\gui_chatbox.c
# End Source File
# Begin Source File

SOURCE=.\gui_classes.c
# End Source File
# Begin Source File

SOURCE=..\gui\gui_floater.c
# End Source File
# Begin Source File

SOURCE=..\gui\gui_graphic.c
# End Source File
# Begin Source File

SOURCE=..\gui\gui_keygrab.c
# End Source File
# Begin Source File

SOURCE=..\gui\gui_label.c
# End Source File
# Begin Source File

SOURCE=..\gui\gui_map.c
# End Source File
# Begin Source File

SOURCE=..\gui\gui_menu.c
# End Source File
# Begin Source File

SOURCE=..\gui\gui_orderedscrollbuffer.c
# End Source File
# Begin Source File

SOURCE=..\gui\gui_playermap.c
# End Source File
# Begin Source File

SOURCE=..\gui\gui_scrollbuffer.c
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

SOURCE=..\gui\gui_textbuffer.c
# End Source File
# Begin Source File

SOURCE=..\gui\gui_thumbnailgrid.c
# End Source File
# Begin Source File

SOURCE=..\gui\gui_togglebutton.c
# End Source File
# Begin Source File

SOURCE=..\gui\gui_userlist_ping.c
# End Source File
# End Group
# Begin Group "Client Game"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Core\camerautils.c
# End Source File
# Begin Source File

SOURCE=.\cl_animation.c
# End Source File
# Begin Source File

SOURCE=.\cl_ask.c
# End Source File
# Begin Source File

SOURCE=.\cl_camera.c
# End Source File
# Begin Source File

SOURCE=.\cl_cmdr_gridmenu.c
# End Source File
# Begin Source File

SOURCE=.\cl_cmdr_interface.c
# End Source File
# Begin Source File

SOURCE=.\cl_cmdr_officer.c
# End Source File
# Begin Source File

SOURCE=.\cl_cmdr_powerups.c
# End Source File
# Begin Source File

SOURCE=.\cl_cmdr_requests.c
# End Source File
# Begin Source File

SOURCE=.\cl_commander.c
# End Source File
# Begin Source File

SOURCE=.\cl_drawutils.c
# End Source File
# Begin Source File

SOURCE=.\cl_effects.c
# End Source File
# Begin Source File

SOURCE=.\cl_environment.c
# End Source File
# Begin Source File

SOURCE=.\cl_events.c
# End Source File
# Begin Source File

SOURCE=.\cl_main.c
# End Source File
# Begin Source File

SOURCE=.\cl_objects.c
# End Source File
# Begin Source File

SOURCE=.\cl_playerstate.c
# End Source File
# Begin Source File

SOURCE=.\cl_render.c
# End Source File
# Begin Source File

SOURCE=.\cl_resources.c
# End Source File
# Begin Source File

SOURCE=.\cl_simpleparticles.c
# End Source File
# Begin Source File

SOURCE=.\cl_vote.c
# End Source File
# Begin Source File

SOURCE=.\cl_world.c
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
# Begin Group "Server Game"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\sv_ai.c
# End Source File
# Begin Source File

SOURCE=.\sv_ai_unit.c
# End Source File
# Begin Source File

SOURCE=.\sv_aigoal.c
# End Source File
# Begin Source File

SOURCE=.\sv_ainav.c
# End Source File
# Begin Source File

SOURCE=.\sv_aistate.c
# End Source File
# Begin Source File

SOURCE=.\sv_aiutil.c
# End Source File
# Begin Source File

SOURCE=.\sv_buildings.c
# End Source File
# Begin Source File

SOURCE=.\sv_clients.c
# End Source File
# Begin Source File

SOURCE=.\sv_events.c
# End Source File
# Begin Source File

SOURCE=.\sv_experience.c
# End Source File
# Begin Source File

SOURCE=.\sv_gamescript.c
# End Source File
# Begin Source File

SOURCE=.\sv_items.c
# End Source File
# Begin Source File

SOURCE=.\sv_main.c
# End Source File
# Begin Source File

SOURCE=.\sv_objects.c
# End Source File
# Begin Source File

SOURCE=.\sv_phys_object.c
# End Source File
# Begin Source File

SOURCE=.\sv_teams.c
# End Source File
# Begin Source File

SOURCE=.\sv_vclient.c
# End Source File
# Begin Source File

SOURCE=.\sv_vote.c
# End Source File
# Begin Source File

SOURCE=.\sv_weapons.c
# End Source File
# Begin Source File

SOURCE=.\sv_world.c
# End Source File
# End Group
# Begin Group "Shared Code"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\builddate.c
# End Source File
# Begin Source File

SOURCE=.\dynstrings.c
# End Source File
# Begin Source File

SOURCE=.\effects.c
# End Source File
# Begin Source File

SOURCE=.\expTable.c
# End Source File
# Begin Source File

SOURCE=.\metaconfig.c
# End Source File
# Begin Source File

SOURCE=.\misc_shared.c
# End Source File
# Begin Source File

SOURCE=.\namelist.c
# End Source File
# Begin Source File

SOURCE=.\objdefs.c
# End Source File
# Begin Source File

SOURCE=.\object_config.c
# End Source File
# Begin Source File

SOURCE=.\phys_player.c
# End Source File
# Begin Source File

SOURCE=.\phys_player_combat.c
# End Source File
# Begin Source File

SOURCE=.\phys_shared.c
# End Source File
# Begin Source File

SOURCE=..\Core\savage_common.c
# End Source File
# Begin Source File

SOURCE=..\Core\savage_mathlib.c
# End Source File
# Begin Source File

SOURCE=.\states.c
# End Source File
# Begin Source File

SOURCE=.\techtree.c
# End Source File
# Begin Source File

SOURCE=.\trajectory.c
# End Source File
# Begin Source File

SOURCE=.\units.c
# End Source File
# Begin Source File

SOURCE=.\upgrades.c
# End Source File
# Begin Source File

SOURCE=.\weapons.c
# End Source File
# Begin Source File

SOURCE=.\weather.c
# End Source File
# End Group
# Begin Group "Main Interface"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\interface.c
# End Source File
# End Group
# Begin Source File

SOURCE=.\dllentry.c
# End Source File
# Begin Source File

SOURCE=.\TheGame.def

!IF  "$(CFG)" == "TheGame - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TheGame - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TheGame - Win32 Demo Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TheGame - Win32 Demo Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "GUI Widget Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\gui\gui_button.h
# End Source File
# Begin Source File

SOURCE=..\gui\gui_chatbox.h
# End Source File
# Begin Source File

SOURCE=..\gui\gui_floater.h
# End Source File
# Begin Source File

SOURCE=..\gui\gui_graphic.h
# End Source File
# Begin Source File

SOURCE=..\gui\gui_hud.h
# End Source File
# Begin Source File

SOURCE=..\gui\gui_keygrab.h
# End Source File
# Begin Source File

SOURCE=..\gui\gui_label.h
# End Source File
# Begin Source File

SOURCE=..\gui\gui_map.h
# End Source File
# Begin Source File

SOURCE=..\gui\gui_menu.h
# End Source File
# Begin Source File

SOURCE=..\gui\gui_scrollbuffer.h
# End Source File
# Begin Source File

SOURCE=..\gui\gui_slider.h
# End Source File
# Begin Source File

SOURCE=..\gui\gui_spinner.h
# End Source File
# Begin Source File

SOURCE=..\gui\gui_swatch.h
# End Source File
# Begin Source File

SOURCE=..\gui\gui_textbox.h
# End Source File
# Begin Source File

SOURCE=..\gui\gui_textbuffer.h
# End Source File
# Begin Source File

SOURCE=..\gui\gui_thumbnailgrid.h
# End Source File
# Begin Source File

SOURCE=..\gui\gui_togglebutton.h
# End Source File
# Begin Source File

SOURCE=..\gui\gui_userlist_ping.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\client_game.h
# End Source File
# Begin Source File

SOURCE=.\game.h
# End Source File
# Begin Source File

SOURCE=..\gui\gui_orderedscrollbuffer.h
# End Source File
# Begin Source File

SOURCE=..\gui\gui_playermap.h
# End Source File
# Begin Source File

SOURCE=.\server_game.h
# End Source File
# Begin Source File

SOURCE=.\sv_ai.h
# End Source File
# Begin Source File

SOURCE=.\sv_aigoal.h
# End Source File
# Begin Source File

SOURCE=.\sv_ainav.h
# End Source File
# Begin Source File

SOURCE=.\sv_aistate.h
# End Source File
# Begin Source File

SOURCE=.\sv_aiutil.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Source File

SOURCE=.\hashstring
# End Source File
# End Target
# End Project
