.KEEP_STATE:
.SUFFIXESS: .C .c .o .ln .lint .src
#.SUFFIXES: .C $(SUFFIXES)

#CC=/opt/intel/compiler60/ia32/bin/icc # gcc-3.0
CC=gcc-3.3

OUTPUT_OPTION = -o $*.o

#intel compiler flags
#OUR_CFLAGS=-Kpic -no_cpprt -i_dynamic#-fPIC #-D_REENTRANT 
#CFLAGS=$(CMDLINE_CFLAGS) $(OUR_CFLAGS) #-Wall -Winline -Wpointer-arith
OUR_CFLAGS=-fpic #-D_REENTRANT 
CFLAGS=$(CMDLINE_CFLAGS) $(OUR_CFLAGS) -Wall -Winline -Wpointer-arith #`pkg-config --cflags glib-2.0`

OUR_SHARED_LIBS=-lc $(SHARED_LIBS) #`pkg-config --libs gthread-2.0` `pkg-config --libs glib-2.0`
		
.c.o:
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $*.o

LIBS=../../libs/

INCLUDE=\
        -I$(LIBS)nvidia/include \
        -I$(LIBS)nvidia/include/glh \
        -I$(LIBS)opengl \
        -I../toplayer \
        -I../Core \
        -I../gui \
	    -I. 
        #-I$(LIBS)openal-dev/openal/linux/include \
        #-I$(LIBS)openal-dev/openal/include/AL \
        #-I$(LIBS)openal-dev/openal/include \
        #-I$(LIBS)libpng/zlib 

OBJS= \
	../Core/savage_mathlib.c \
	../Core/savage_common.c \
	../Core/camerautils.c \
    ../toplayer/tl_camera.c \
    ../toplayer/tl_drawutils.c \
    ../toplayer/tl_pool.c \
    ../toplayer/tl_sky.c \
	../gui/gui_button.c \
	../gui/gui_chatbox.c \
	../gui/gui_floater.c \
	../gui/gui_graphic.c \
	../gui/gui_label.c \
	../gui/gui_keygrab.c \
	../gui/gui_menu.c \
	../gui/gui_scrollbuffer.c \
	../gui/gui_orderedscrollbuffer.c \
	../gui/gui_slider.c \
	../gui/gui_spinner.c \
	../gui/gui_swatch.c \
	../gui/gui_textbuffer.c \
	../gui/gui_textbox.c \
	../gui/gui_thumbnailgrid.c \
	../gui/gui_togglebutton.c \
	../gui/gui_map.c \
	../gui/gui_playermap.c \
	../gui/gui_userlist_ping.c \
	builddate.c \
	cl_animation.c \
	cl_ask.c \
	cl_cmdr_interface.c \
	cl_cmdr_requests.c \
	cl_cmdr_gridmenu.c \
	cl_cmdr_officer.c \
	cl_cmdr_powerups.c \
	cl_main.c \
	cl_events.c \
	cl_effects.c \
	cl_render.c \
	cl_environment.c \
	cl_resources.c \
	cl_playerstate.c \
	cl_commander.c \
	cl_objects.c \
	cl_camera.c \
	cl_drawutils.c \
	cl_simpleparticles.c \
	cl_vote.c \
	cl_world.c \
	effects.c \
	expTable.c \
	items.c \
	objdefs.c \
	object_config.c \
	misc_shared.c \
	upgrades.c \
	sv_ai.c \
	sv_aigoal.c \
	sv_ainav.c \
	sv_aistate.c \
	sv_aiutil.c \
	sv_ai_unit.c \
	sv_buildings.c \
	sv_experience.c \
	sv_gamescript.c \
	sv_main.c \
	sv_world.c \
	sv_objects.c \
	sv_clients.c \
	sv_teams.c \
	sv_player.c \
	sv_commandcenter.c \
	sv_events.c \
	sv_weapons.c \
	sv_items.c \
	sv_phys_object.c \
	sv_vote.c \
	sv_vclient.c \
	phys_player_combat.c \
	phys_player.c \
	phys_shared.c \
	gui_classes.c \
	states.c \
	techtree.c \
	todo.c \
	weapons.c \
	units.c \
	dllentry.c \
	dynstrings.c \
	metaconfig.c \
	namelist.c \
	trajectory.c \
	interface.c \
	weather.c
#	../gui/gui_teammenu.c \
#	cl_contextmenu.c \
#	cl_objedit.c \
	

all: debug #clean-core-files

#clean-core-files:
	#rm -f ../Core/savage_mathlib.o ../Core/savage_common.o ../Core/camerautils.o

pot:
	xgettext --keyword=fmt --keyword=_ --keyword=Console_Printf --keyword=Console_DPrintf --keyword=Console_Errorf --keyword=corec.Console_Printf --keyword=corec.Console_DPrintf --keyword=cores.Console_Printf --keyword=cores.Console_DPrintf --keyword=core.Console_Printf --keyword=core.Console_DPrintf --keyword=cores.Console_Errorf --keyword=corec.Console_Errorf --keyword=core.Console_Errorf -o strings.pot *.[ch] ../toplayer/*.[ch] ../gui/*.[ch]

debug:
	makedepend $(INCLUDE) $(OBJS)
	cd ../Core/ && colormake
	CMDLINE_CFLAGS="$(CMDLINE_CFLAGS) -g" colormake binary

debug-efence:
	SHARED_LIBS=-lefence colormake debug

debug-dmalloc:
	SHARED_LIBS=-ldmalloc CMDLINE_CFLAGS="-DDMALLOC" colormake debug

debug-profile:
	CMDLINE_CFLAGS="-pg $(CMDLINE_CFLAGS)" colormake debug

debug-optimized:
	CMDLINE_CFLAGS="-O2 $(CMDLINE_CFLAGS)" colormake debug

threads:
	CMDLINE_CFLAGS="-DUSE_THREADS $(CMDLINE_CLFAGS)" colormake debug

demo-release:
	# intel compiler flags
	#CMDLINE_CFLAGS="-O3 -tpp6 -rcd -axM -unroll -parallel -openmp" colormake game.so
	#CMDLINE_CFLAGS="$(CMDLINE_CFLAGS) -O6 -march=pentium -mcpu=i686 -finline-functions -mno-ieee-fp -ffast-math -fstrength-reduce -funroll-loops" colormake game.so
	CMDLINE_CFLAGS="$(CMDLINE_CFLAGS) -march=i686 -mfpmath=sse -mmmx -msse -m3dnow -minline-all-stringops -O3 -ffast-math -mno-ieee-fp -fomit-frame-pointer -DSAVAGE_DEMO" colormake binary

release:
	# intel compiler flags
	#CMDLINE_CFLAGS="-O3 -tpp6 -rcd -axM -unroll -parallel -openmp" colormake game.so
	#CMDLINE_CFLAGS="$(CMDLINE_CFLAGS) -O6 -march=pentium -mcpu=i686 -finline-functions -mno-ieee-fp -ffast-math -fstrength-reduce -funroll-loops" colormake game.so
	CMDLINE_CFLAGS="$(CMDLINE_CFLAGS) -march=i686 -mfpmath=sse -mmmx -msse -m3dnow -minline-all-stringops -O3 -ffast-math -mno-ieee-fp -fomit-frame-pointer" colormake binary

release-game:
	CMDLINE_CFLAGS="$(CMDLINE_CFLAGS) -DFINAL_GAME" make release

release-profile: 
	#CMDLINE_CFLAGS="$(CMDLINE_CFLAGS) -O6 -march=pentium -mcpu=i686 -finline-functions -mno-ieee-fp -ffast-math -fstrength-reduce -funroll-loops -pg" colormake game.so
	CMDLINE_CFLAGS="$(CMDLINE_CFLAGS) -pg -march=i686 -mfpmath=sse -mmmx -msse -m3dnow -minline-all-stringops -O3 -ffast-math -mno-ieee-fp" colormake binary

profile: 
	CMDLINE_CFLAGS="-pg $(CMDLINE_CFLAGS)" colormake binary

binary: ../../Clean_Build/game/game.so

../../build/game/game.so: $(OBJS:.c=.o)
	sync
	$(CC) $(OUR_CFLAGS) $(CFLAGS) -shared -Wl,-soname,game.so.1 \
	    -o ../../build/game/game.so $(OBJS:.c=.o) $(OUR_SHARED_LIBS)

../../Clean_Build/game/game.so: $(OBJS:.c=.o)
	sync
	$(CC) $(OUR_CFLAGS) $(CFLAGS) -shared -Wl,-soname,game.so.1 \
	    -o ../../Clean_Build/game/game.so $(OBJS:.c=.o) $(OUR_SHARED_LIBS)

builddate.o: $(OBJS) ../Core/*.c

clean:
	rm -f *.o ../gui/*.o ../toplayer/*.o
# DO NOT DELETE

../Core/savage_mathlib.o: ../Core/savage_common.h /usr/include/stdlib.h
../Core/savage_mathlib.o: /usr/include/features.h /usr/include/sys/cdefs.h
../Core/savage_mathlib.o: /usr/include/gnu/stubs.h
../Core/savage_mathlib.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
../Core/savage_mathlib.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
../Core/savage_mathlib.o: /usr/include/stdio.h /usr/include/bits/types.h
../Core/savage_mathlib.o: /usr/include/bits/wordsize.h
../Core/savage_mathlib.o: /usr/include/bits/typesizes.h /usr/include/libio.h
../Core/savage_mathlib.o: /usr/include/_G_config.h /usr/include/wchar.h
../Core/savage_mathlib.o: /usr/include/bits/wchar.h /usr/include/gconv.h
../Core/savage_mathlib.o: /usr/include/bits/stdio_lim.h
../Core/savage_mathlib.o: /usr/include/bits/sys_errlist.h
../Core/savage_mathlib.o: /usr/include/string.h ../Core/savage_mathlib.h
../Core/savage_mathlib.o: /usr/include/math.h /usr/include/bits/huge_val.h
../Core/savage_mathlib.o: /usr/include/bits/mathdef.h
../Core/savage_mathlib.o: /usr/include/bits/mathcalls.h
../Core/savage_mathlib.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h
../Core/savage_common.o: ../Core/savage_common.h /usr/include/stdlib.h
../Core/savage_common.o: /usr/include/features.h /usr/include/sys/cdefs.h
../Core/savage_common.o: /usr/include/gnu/stubs.h
../Core/savage_common.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
../Core/savage_common.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
../Core/savage_common.o: /usr/include/stdio.h /usr/include/bits/types.h
../Core/savage_common.o: /usr/include/bits/wordsize.h
../Core/savage_common.o: /usr/include/bits/typesizes.h /usr/include/libio.h
../Core/savage_common.o: /usr/include/_G_config.h /usr/include/wchar.h
../Core/savage_common.o: /usr/include/bits/wchar.h /usr/include/gconv.h
../Core/savage_common.o: /usr/include/bits/stdio_lim.h
../Core/savage_common.o: /usr/include/bits/sys_errlist.h
../Core/savage_common.o: /usr/include/string.h ../Core/savage_mathlib.h
../Core/savage_common.o: /usr/include/math.h /usr/include/bits/huge_val.h
../Core/savage_common.o: /usr/include/bits/mathdef.h
../Core/savage_common.o: /usr/include/bits/mathcalls.h
../Core/camerautils.o: ../Core/savage_types.h ../masterserver/sharedcommon.h
../Core/camerautils.o: ../Core/savage_common.h /usr/include/stdlib.h
../Core/camerautils.o: /usr/include/features.h /usr/include/sys/cdefs.h
../Core/camerautils.o: /usr/include/gnu/stubs.h
../Core/camerautils.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
../Core/camerautils.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
../Core/camerautils.o: /usr/include/stdio.h /usr/include/bits/types.h
../Core/camerautils.o: /usr/include/bits/wordsize.h
../Core/camerautils.o: /usr/include/bits/typesizes.h /usr/include/libio.h
../Core/camerautils.o: /usr/include/_G_config.h /usr/include/wchar.h
../Core/camerautils.o: /usr/include/bits/wchar.h /usr/include/gconv.h
../Core/camerautils.o: /usr/include/bits/stdio_lim.h
../Core/camerautils.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
../Core/camerautils.o: ../Core/savage_mathlib.h /usr/include/math.h
../Core/camerautils.o: /usr/include/bits/huge_val.h
../Core/camerautils.o: /usr/include/bits/mathdef.h
../Core/camerautils.o: /usr/include/bits/mathcalls.h
../toplayer/tl_camera.o: ../toplayer/tl_shared.h ../toplayer/savage.h
../toplayer/tl_camera.o: ../Core/savage_types.h
../toplayer/tl_camera.o: ../masterserver/sharedcommon.h
../toplayer/tl_camera.o: ../Core/savage_common.h /usr/include/stdlib.h
../toplayer/tl_camera.o: /usr/include/features.h /usr/include/sys/cdefs.h
../toplayer/tl_camera.o: /usr/include/gnu/stubs.h
../toplayer/tl_camera.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
../toplayer/tl_camera.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
../toplayer/tl_camera.o: /usr/include/stdio.h /usr/include/bits/types.h
../toplayer/tl_camera.o: /usr/include/bits/wordsize.h
../toplayer/tl_camera.o: /usr/include/bits/typesizes.h /usr/include/libio.h
../toplayer/tl_camera.o: /usr/include/_G_config.h /usr/include/wchar.h
../toplayer/tl_camera.o: /usr/include/bits/wchar.h /usr/include/gconv.h
../toplayer/tl_camera.o: /usr/include/bits/stdio_lim.h
../toplayer/tl_camera.o: /usr/include/bits/sys_errlist.h
../toplayer/tl_camera.o: /usr/include/string.h ../Core/savage_mathlib.h
../toplayer/tl_camera.o: /usr/include/math.h /usr/include/bits/huge_val.h
../toplayer/tl_camera.o: /usr/include/bits/mathdef.h
../toplayer/tl_camera.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
../toplayer/tl_camera.o: /usr/include/time.h /usr/include/bits/time.h
../toplayer/tl_camera.o: ../Core/core_api.h
../toplayer/tl_drawutils.o: ../toplayer/tl_shared.h ../toplayer/savage.h
../toplayer/tl_drawutils.o: ../Core/savage_types.h
../toplayer/tl_drawutils.o: ../masterserver/sharedcommon.h
../toplayer/tl_drawutils.o: ../Core/savage_common.h /usr/include/stdlib.h
../toplayer/tl_drawutils.o: /usr/include/features.h /usr/include/sys/cdefs.h
../toplayer/tl_drawutils.o: /usr/include/gnu/stubs.h
../toplayer/tl_drawutils.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
../toplayer/tl_drawutils.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
../toplayer/tl_drawutils.o: /usr/include/stdio.h /usr/include/bits/types.h
../toplayer/tl_drawutils.o: /usr/include/bits/wordsize.h
../toplayer/tl_drawutils.o: /usr/include/bits/typesizes.h
../toplayer/tl_drawutils.o: /usr/include/libio.h /usr/include/_G_config.h
../toplayer/tl_drawutils.o: /usr/include/wchar.h /usr/include/bits/wchar.h
../toplayer/tl_drawutils.o: /usr/include/gconv.h
../toplayer/tl_drawutils.o: /usr/include/bits/stdio_lim.h
../toplayer/tl_drawutils.o: /usr/include/bits/sys_errlist.h
../toplayer/tl_drawutils.o: /usr/include/string.h ../Core/savage_mathlib.h
../toplayer/tl_drawutils.o: /usr/include/math.h /usr/include/bits/huge_val.h
../toplayer/tl_drawutils.o: /usr/include/bits/mathdef.h
../toplayer/tl_drawutils.o: /usr/include/bits/mathcalls.h
../toplayer/tl_drawutils.o: ../Core/camerautils.h /usr/include/time.h
../toplayer/tl_drawutils.o: /usr/include/bits/time.h ../Core/core_api.h
../toplayer/tl_pool.o: ../toplayer/tl_shared.h ../toplayer/savage.h
../toplayer/tl_pool.o: ../Core/savage_types.h ../masterserver/sharedcommon.h
../toplayer/tl_pool.o: ../Core/savage_common.h /usr/include/stdlib.h
../toplayer/tl_pool.o: /usr/include/features.h /usr/include/sys/cdefs.h
../toplayer/tl_pool.o: /usr/include/gnu/stubs.h
../toplayer/tl_pool.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
../toplayer/tl_pool.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
../toplayer/tl_pool.o: /usr/include/stdio.h /usr/include/bits/types.h
../toplayer/tl_pool.o: /usr/include/bits/wordsize.h
../toplayer/tl_pool.o: /usr/include/bits/typesizes.h /usr/include/libio.h
../toplayer/tl_pool.o: /usr/include/_G_config.h /usr/include/wchar.h
../toplayer/tl_pool.o: /usr/include/bits/wchar.h /usr/include/gconv.h
../toplayer/tl_pool.o: /usr/include/bits/stdio_lim.h
../toplayer/tl_pool.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
../toplayer/tl_pool.o: ../Core/savage_mathlib.h /usr/include/math.h
../toplayer/tl_pool.o: /usr/include/bits/huge_val.h
../toplayer/tl_pool.o: /usr/include/bits/mathdef.h
../toplayer/tl_pool.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
../toplayer/tl_pool.o: /usr/include/time.h /usr/include/bits/time.h
../toplayer/tl_pool.o: ../Core/core_api.h
../toplayer/tl_sky.o: ../toplayer/tl_shared.h ../toplayer/savage.h
../toplayer/tl_sky.o: ../Core/savage_types.h ../masterserver/sharedcommon.h
../toplayer/tl_sky.o: ../Core/savage_common.h /usr/include/stdlib.h
../toplayer/tl_sky.o: /usr/include/features.h /usr/include/sys/cdefs.h
../toplayer/tl_sky.o: /usr/include/gnu/stubs.h
../toplayer/tl_sky.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
../toplayer/tl_sky.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
../toplayer/tl_sky.o: /usr/include/stdio.h /usr/include/bits/types.h
../toplayer/tl_sky.o: /usr/include/bits/wordsize.h
../toplayer/tl_sky.o: /usr/include/bits/typesizes.h /usr/include/libio.h
../toplayer/tl_sky.o: /usr/include/_G_config.h /usr/include/wchar.h
../toplayer/tl_sky.o: /usr/include/bits/wchar.h /usr/include/gconv.h
../toplayer/tl_sky.o: /usr/include/bits/stdio_lim.h
../toplayer/tl_sky.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
../toplayer/tl_sky.o: ../Core/savage_mathlib.h /usr/include/math.h
../toplayer/tl_sky.o: /usr/include/bits/huge_val.h
../toplayer/tl_sky.o: /usr/include/bits/mathdef.h
../toplayer/tl_sky.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
../toplayer/tl_sky.o: /usr/include/time.h /usr/include/bits/time.h
../toplayer/tl_sky.o: ../Core/core_api.h
../gui/gui_button.o: ../toplayer/tl_shared.h ../toplayer/savage.h
../gui/gui_button.o: ../Core/savage_types.h ../masterserver/sharedcommon.h
../gui/gui_button.o: ../Core/savage_common.h /usr/include/stdlib.h
../gui/gui_button.o: /usr/include/features.h /usr/include/sys/cdefs.h
../gui/gui_button.o: /usr/include/gnu/stubs.h
../gui/gui_button.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
../gui/gui_button.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
../gui/gui_button.o: /usr/include/stdio.h /usr/include/bits/types.h
../gui/gui_button.o: /usr/include/bits/wordsize.h
../gui/gui_button.o: /usr/include/bits/typesizes.h /usr/include/libio.h
../gui/gui_button.o: /usr/include/_G_config.h /usr/include/wchar.h
../gui/gui_button.o: /usr/include/bits/wchar.h /usr/include/gconv.h
../gui/gui_button.o: /usr/include/bits/stdio_lim.h
../gui/gui_button.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
../gui/gui_button.o: ../Core/savage_mathlib.h /usr/include/math.h
../gui/gui_button.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
../gui/gui_button.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
../gui/gui_button.o: /usr/include/time.h /usr/include/bits/time.h
../gui/gui_button.o: ../Core/core_api.h ../gui/gui_button.h
../gui/gui_chatbox.o: client_game.h game.h ../toplayer/savage.h
../gui/gui_chatbox.o: ../Core/savage_types.h ../masterserver/sharedcommon.h
../gui/gui_chatbox.o: ../Core/savage_common.h /usr/include/stdlib.h
../gui/gui_chatbox.o: /usr/include/features.h /usr/include/sys/cdefs.h
../gui/gui_chatbox.o: /usr/include/gnu/stubs.h
../gui/gui_chatbox.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
../gui/gui_chatbox.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
../gui/gui_chatbox.o: /usr/include/stdio.h /usr/include/bits/types.h
../gui/gui_chatbox.o: /usr/include/bits/wordsize.h
../gui/gui_chatbox.o: /usr/include/bits/typesizes.h /usr/include/libio.h
../gui/gui_chatbox.o: /usr/include/_G_config.h /usr/include/wchar.h
../gui/gui_chatbox.o: /usr/include/bits/wchar.h /usr/include/gconv.h
../gui/gui_chatbox.o: /usr/include/bits/stdio_lim.h
../gui/gui_chatbox.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
../gui/gui_chatbox.o: ../Core/savage_mathlib.h /usr/include/math.h
../gui/gui_chatbox.o: /usr/include/bits/huge_val.h
../gui/gui_chatbox.o: /usr/include/bits/mathdef.h
../gui/gui_chatbox.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
../gui/gui_chatbox.o: /usr/include/time.h /usr/include/bits/time.h
../gui/gui_chatbox.o: ../Core/core_api.h ../toplayer/tl_shared.h interface.h
../gui/gui_chatbox.o: ../gui/gui_scrollbuffer.h
../gui/gui_chatbox.o: ../gui/gui_orderedscrollbuffer.h
../gui/gui_chatbox.o: ../gui/gui_textbuffer.h ../gui/gui_button.h
../gui/gui_chatbox.o: ../gui/gui_graphic.h ../gui/gui_label.h
../gui/gui_chatbox.o: ../gui/gui_chatbox.h
../gui/gui_floater.o: ../toplayer/tl_shared.h ../toplayer/savage.h
../gui/gui_floater.o: ../Core/savage_types.h ../masterserver/sharedcommon.h
../gui/gui_floater.o: ../Core/savage_common.h /usr/include/stdlib.h
../gui/gui_floater.o: /usr/include/features.h /usr/include/sys/cdefs.h
../gui/gui_floater.o: /usr/include/gnu/stubs.h
../gui/gui_floater.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
../gui/gui_floater.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
../gui/gui_floater.o: /usr/include/stdio.h /usr/include/bits/types.h
../gui/gui_floater.o: /usr/include/bits/wordsize.h
../gui/gui_floater.o: /usr/include/bits/typesizes.h /usr/include/libio.h
../gui/gui_floater.o: /usr/include/_G_config.h /usr/include/wchar.h
../gui/gui_floater.o: /usr/include/bits/wchar.h /usr/include/gconv.h
../gui/gui_floater.o: /usr/include/bits/stdio_lim.h
../gui/gui_floater.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
../gui/gui_floater.o: ../Core/savage_mathlib.h /usr/include/math.h
../gui/gui_floater.o: /usr/include/bits/huge_val.h
../gui/gui_floater.o: /usr/include/bits/mathdef.h
../gui/gui_floater.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
../gui/gui_floater.o: /usr/include/time.h /usr/include/bits/time.h
../gui/gui_floater.o: ../Core/core_api.h ../gui/gui_floater.h
../gui/gui_floater.o: ../gui/gui_graphic.h
../gui/gui_graphic.o: ../toplayer/tl_shared.h ../toplayer/savage.h
../gui/gui_graphic.o: ../Core/savage_types.h ../masterserver/sharedcommon.h
../gui/gui_graphic.o: ../Core/savage_common.h /usr/include/stdlib.h
../gui/gui_graphic.o: /usr/include/features.h /usr/include/sys/cdefs.h
../gui/gui_graphic.o: /usr/include/gnu/stubs.h
../gui/gui_graphic.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
../gui/gui_graphic.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
../gui/gui_graphic.o: /usr/include/stdio.h /usr/include/bits/types.h
../gui/gui_graphic.o: /usr/include/bits/wordsize.h
../gui/gui_graphic.o: /usr/include/bits/typesizes.h /usr/include/libio.h
../gui/gui_graphic.o: /usr/include/_G_config.h /usr/include/wchar.h
../gui/gui_graphic.o: /usr/include/bits/wchar.h /usr/include/gconv.h
../gui/gui_graphic.o: /usr/include/bits/stdio_lim.h
../gui/gui_graphic.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
../gui/gui_graphic.o: ../Core/savage_mathlib.h /usr/include/math.h
../gui/gui_graphic.o: /usr/include/bits/huge_val.h
../gui/gui_graphic.o: /usr/include/bits/mathdef.h
../gui/gui_graphic.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
../gui/gui_graphic.o: /usr/include/time.h /usr/include/bits/time.h
../gui/gui_graphic.o: ../Core/core_api.h ../gui/gui_graphic.h
../gui/gui_label.o: ../toplayer/tl_shared.h ../toplayer/savage.h
../gui/gui_label.o: ../Core/savage_types.h ../masterserver/sharedcommon.h
../gui/gui_label.o: ../Core/savage_common.h /usr/include/stdlib.h
../gui/gui_label.o: /usr/include/features.h /usr/include/sys/cdefs.h
../gui/gui_label.o: /usr/include/gnu/stubs.h
../gui/gui_label.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
../gui/gui_label.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
../gui/gui_label.o: /usr/include/stdio.h /usr/include/bits/types.h
../gui/gui_label.o: /usr/include/bits/wordsize.h
../gui/gui_label.o: /usr/include/bits/typesizes.h /usr/include/libio.h
../gui/gui_label.o: /usr/include/_G_config.h /usr/include/wchar.h
../gui/gui_label.o: /usr/include/bits/wchar.h /usr/include/gconv.h
../gui/gui_label.o: /usr/include/bits/stdio_lim.h
../gui/gui_label.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
../gui/gui_label.o: ../Core/savage_mathlib.h /usr/include/math.h
../gui/gui_label.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
../gui/gui_label.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
../gui/gui_label.o: /usr/include/time.h /usr/include/bits/time.h
../gui/gui_label.o: ../Core/core_api.h ../gui/gui_label.h
../gui/gui_keygrab.o: ../toplayer/tl_shared.h ../toplayer/savage.h
../gui/gui_keygrab.o: ../Core/savage_types.h ../masterserver/sharedcommon.h
../gui/gui_keygrab.o: ../Core/savage_common.h /usr/include/stdlib.h
../gui/gui_keygrab.o: /usr/include/features.h /usr/include/sys/cdefs.h
../gui/gui_keygrab.o: /usr/include/gnu/stubs.h
../gui/gui_keygrab.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
../gui/gui_keygrab.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
../gui/gui_keygrab.o: /usr/include/stdio.h /usr/include/bits/types.h
../gui/gui_keygrab.o: /usr/include/bits/wordsize.h
../gui/gui_keygrab.o: /usr/include/bits/typesizes.h /usr/include/libio.h
../gui/gui_keygrab.o: /usr/include/_G_config.h /usr/include/wchar.h
../gui/gui_keygrab.o: /usr/include/bits/wchar.h /usr/include/gconv.h
../gui/gui_keygrab.o: /usr/include/bits/stdio_lim.h
../gui/gui_keygrab.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
../gui/gui_keygrab.o: ../Core/savage_mathlib.h /usr/include/math.h
../gui/gui_keygrab.o: /usr/include/bits/huge_val.h
../gui/gui_keygrab.o: /usr/include/bits/mathdef.h
../gui/gui_keygrab.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
../gui/gui_keygrab.o: /usr/include/time.h /usr/include/bits/time.h
../gui/gui_keygrab.o: ../Core/core_api.h ../gui/gui_keygrab.h
../gui/gui_menu.o: ../toplayer/tl_shared.h ../toplayer/savage.h
../gui/gui_menu.o: ../Core/savage_types.h ../masterserver/sharedcommon.h
../gui/gui_menu.o: ../Core/savage_common.h /usr/include/stdlib.h
../gui/gui_menu.o: /usr/include/features.h /usr/include/sys/cdefs.h
../gui/gui_menu.o: /usr/include/gnu/stubs.h
../gui/gui_menu.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
../gui/gui_menu.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
../gui/gui_menu.o: /usr/include/stdio.h /usr/include/bits/types.h
../gui/gui_menu.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
../gui/gui_menu.o: /usr/include/libio.h /usr/include/_G_config.h
../gui/gui_menu.o: /usr/include/wchar.h /usr/include/bits/wchar.h
../gui/gui_menu.o: /usr/include/gconv.h /usr/include/bits/stdio_lim.h
../gui/gui_menu.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
../gui/gui_menu.o: ../Core/savage_mathlib.h /usr/include/math.h
../gui/gui_menu.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
../gui/gui_menu.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
../gui/gui_menu.o: /usr/include/time.h /usr/include/bits/time.h
../gui/gui_menu.o: ../Core/core_api.h ../gui/gui_menu.h
../gui/gui_scrollbuffer.o: ../toplayer/tl_shared.h ../toplayer/savage.h
../gui/gui_scrollbuffer.o: ../Core/savage_types.h
../gui/gui_scrollbuffer.o: ../masterserver/sharedcommon.h
../gui/gui_scrollbuffer.o: ../Core/savage_common.h /usr/include/stdlib.h
../gui/gui_scrollbuffer.o: /usr/include/features.h /usr/include/sys/cdefs.h
../gui/gui_scrollbuffer.o: /usr/include/gnu/stubs.h
../gui/gui_scrollbuffer.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
../gui/gui_scrollbuffer.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
../gui/gui_scrollbuffer.o: /usr/include/stdio.h /usr/include/bits/types.h
../gui/gui_scrollbuffer.o: /usr/include/bits/wordsize.h
../gui/gui_scrollbuffer.o: /usr/include/bits/typesizes.h /usr/include/libio.h
../gui/gui_scrollbuffer.o: /usr/include/_G_config.h /usr/include/wchar.h
../gui/gui_scrollbuffer.o: /usr/include/bits/wchar.h /usr/include/gconv.h
../gui/gui_scrollbuffer.o: /usr/include/bits/stdio_lim.h
../gui/gui_scrollbuffer.o: /usr/include/bits/sys_errlist.h
../gui/gui_scrollbuffer.o: /usr/include/string.h ../Core/savage_mathlib.h
../gui/gui_scrollbuffer.o: /usr/include/math.h /usr/include/bits/huge_val.h
../gui/gui_scrollbuffer.o: /usr/include/bits/mathdef.h
../gui/gui_scrollbuffer.o: /usr/include/bits/mathcalls.h
../gui/gui_scrollbuffer.o: ../Core/camerautils.h /usr/include/time.h
../gui/gui_scrollbuffer.o: /usr/include/bits/time.h ../Core/core_api.h
../gui/gui_scrollbuffer.o: ../gui/gui_scrollbuffer.h
../gui/gui_orderedscrollbuffer.o: ../toplayer/tl_shared.h
../gui/gui_orderedscrollbuffer.o: ../toplayer/savage.h ../Core/savage_types.h
../gui/gui_orderedscrollbuffer.o: ../masterserver/sharedcommon.h
../gui/gui_orderedscrollbuffer.o: ../Core/savage_common.h
../gui/gui_orderedscrollbuffer.o: /usr/include/stdlib.h
../gui/gui_orderedscrollbuffer.o: /usr/include/features.h
../gui/gui_orderedscrollbuffer.o: /usr/include/sys/cdefs.h
../gui/gui_orderedscrollbuffer.o: /usr/include/gnu/stubs.h
../gui/gui_orderedscrollbuffer.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
../gui/gui_orderedscrollbuffer.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
../gui/gui_orderedscrollbuffer.o: /usr/include/stdio.h
../gui/gui_orderedscrollbuffer.o: /usr/include/bits/types.h
../gui/gui_orderedscrollbuffer.o: /usr/include/bits/wordsize.h
../gui/gui_orderedscrollbuffer.o: /usr/include/bits/typesizes.h
../gui/gui_orderedscrollbuffer.o: /usr/include/libio.h
../gui/gui_orderedscrollbuffer.o: /usr/include/_G_config.h
../gui/gui_orderedscrollbuffer.o: /usr/include/wchar.h
../gui/gui_orderedscrollbuffer.o: /usr/include/bits/wchar.h
../gui/gui_orderedscrollbuffer.o: /usr/include/gconv.h
../gui/gui_orderedscrollbuffer.o: /usr/include/bits/stdio_lim.h
../gui/gui_orderedscrollbuffer.o: /usr/include/bits/sys_errlist.h
../gui/gui_orderedscrollbuffer.o: /usr/include/string.h
../gui/gui_orderedscrollbuffer.o: ../Core/savage_mathlib.h
../gui/gui_orderedscrollbuffer.o: /usr/include/math.h
../gui/gui_orderedscrollbuffer.o: /usr/include/bits/huge_val.h
../gui/gui_orderedscrollbuffer.o: /usr/include/bits/mathdef.h
../gui/gui_orderedscrollbuffer.o: /usr/include/bits/mathcalls.h
../gui/gui_orderedscrollbuffer.o: ../Core/camerautils.h /usr/include/time.h
../gui/gui_orderedscrollbuffer.o: /usr/include/bits/time.h ../Core/core_api.h
../gui/gui_orderedscrollbuffer.o: ../gui/gui_orderedscrollbuffer.h
../gui/gui_slider.o: ../toplayer/tl_shared.h ../toplayer/savage.h
../gui/gui_slider.o: ../Core/savage_types.h ../masterserver/sharedcommon.h
../gui/gui_slider.o: ../Core/savage_common.h /usr/include/stdlib.h
../gui/gui_slider.o: /usr/include/features.h /usr/include/sys/cdefs.h
../gui/gui_slider.o: /usr/include/gnu/stubs.h
../gui/gui_slider.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
../gui/gui_slider.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
../gui/gui_slider.o: /usr/include/stdio.h /usr/include/bits/types.h
../gui/gui_slider.o: /usr/include/bits/wordsize.h
../gui/gui_slider.o: /usr/include/bits/typesizes.h /usr/include/libio.h
../gui/gui_slider.o: /usr/include/_G_config.h /usr/include/wchar.h
../gui/gui_slider.o: /usr/include/bits/wchar.h /usr/include/gconv.h
../gui/gui_slider.o: /usr/include/bits/stdio_lim.h
../gui/gui_slider.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
../gui/gui_slider.o: ../Core/savage_mathlib.h /usr/include/math.h
../gui/gui_slider.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
../gui/gui_slider.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
../gui/gui_slider.o: /usr/include/time.h /usr/include/bits/time.h
../gui/gui_slider.o: ../Core/core_api.h ../gui/gui_slider.h
../gui/gui_spinner.o: ../toplayer/tl_shared.h ../toplayer/savage.h
../gui/gui_spinner.o: ../Core/savage_types.h ../masterserver/sharedcommon.h
../gui/gui_spinner.o: ../Core/savage_common.h /usr/include/stdlib.h
../gui/gui_spinner.o: /usr/include/features.h /usr/include/sys/cdefs.h
../gui/gui_spinner.o: /usr/include/gnu/stubs.h
../gui/gui_spinner.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
../gui/gui_spinner.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
../gui/gui_spinner.o: /usr/include/stdio.h /usr/include/bits/types.h
../gui/gui_spinner.o: /usr/include/bits/wordsize.h
../gui/gui_spinner.o: /usr/include/bits/typesizes.h /usr/include/libio.h
../gui/gui_spinner.o: /usr/include/_G_config.h /usr/include/wchar.h
../gui/gui_spinner.o: /usr/include/bits/wchar.h /usr/include/gconv.h
../gui/gui_spinner.o: /usr/include/bits/stdio_lim.h
../gui/gui_spinner.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
../gui/gui_spinner.o: ../Core/savage_mathlib.h /usr/include/math.h
../gui/gui_spinner.o: /usr/include/bits/huge_val.h
../gui/gui_spinner.o: /usr/include/bits/mathdef.h
../gui/gui_spinner.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
../gui/gui_spinner.o: /usr/include/time.h /usr/include/bits/time.h
../gui/gui_spinner.o: ../Core/core_api.h ../Core/system.h
../gui/gui_spinner.o: ../gui/gui_spinner.h ../gui/gui_button.h
../gui/gui_swatch.o: ../toplayer/tl_shared.h ../toplayer/savage.h
../gui/gui_swatch.o: ../Core/savage_types.h ../masterserver/sharedcommon.h
../gui/gui_swatch.o: ../Core/savage_common.h /usr/include/stdlib.h
../gui/gui_swatch.o: /usr/include/features.h /usr/include/sys/cdefs.h
../gui/gui_swatch.o: /usr/include/gnu/stubs.h
../gui/gui_swatch.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
../gui/gui_swatch.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
../gui/gui_swatch.o: /usr/include/stdio.h /usr/include/bits/types.h
../gui/gui_swatch.o: /usr/include/bits/wordsize.h
../gui/gui_swatch.o: /usr/include/bits/typesizes.h /usr/include/libio.h
../gui/gui_swatch.o: /usr/include/_G_config.h /usr/include/wchar.h
../gui/gui_swatch.o: /usr/include/bits/wchar.h /usr/include/gconv.h
../gui/gui_swatch.o: /usr/include/bits/stdio_lim.h
../gui/gui_swatch.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
../gui/gui_swatch.o: ../Core/savage_mathlib.h /usr/include/math.h
../gui/gui_swatch.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
../gui/gui_swatch.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
../gui/gui_swatch.o: /usr/include/time.h /usr/include/bits/time.h
../gui/gui_swatch.o: ../Core/core_api.h ../gui/gui_swatch.h
../gui/gui_textbuffer.o: ../toplayer/tl_shared.h ../toplayer/savage.h
../gui/gui_textbuffer.o: ../Core/savage_types.h
../gui/gui_textbuffer.o: ../masterserver/sharedcommon.h
../gui/gui_textbuffer.o: ../Core/savage_common.h /usr/include/stdlib.h
../gui/gui_textbuffer.o: /usr/include/features.h /usr/include/sys/cdefs.h
../gui/gui_textbuffer.o: /usr/include/gnu/stubs.h
../gui/gui_textbuffer.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
../gui/gui_textbuffer.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
../gui/gui_textbuffer.o: /usr/include/stdio.h /usr/include/bits/types.h
../gui/gui_textbuffer.o: /usr/include/bits/wordsize.h
../gui/gui_textbuffer.o: /usr/include/bits/typesizes.h /usr/include/libio.h
../gui/gui_textbuffer.o: /usr/include/_G_config.h /usr/include/wchar.h
../gui/gui_textbuffer.o: /usr/include/bits/wchar.h /usr/include/gconv.h
../gui/gui_textbuffer.o: /usr/include/bits/stdio_lim.h
../gui/gui_textbuffer.o: /usr/include/bits/sys_errlist.h
../gui/gui_textbuffer.o: /usr/include/string.h ../Core/savage_mathlib.h
../gui/gui_textbuffer.o: /usr/include/math.h /usr/include/bits/huge_val.h
../gui/gui_textbuffer.o: /usr/include/bits/mathdef.h
../gui/gui_textbuffer.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
../gui/gui_textbuffer.o: /usr/include/time.h /usr/include/bits/time.h
../gui/gui_textbuffer.o: ../Core/core_api.h ../gui/gui_textbuffer.h
../gui/gui_textbox.o: ../toplayer/tl_shared.h ../toplayer/savage.h
../gui/gui_textbox.o: ../Core/savage_types.h ../masterserver/sharedcommon.h
../gui/gui_textbox.o: ../Core/savage_common.h /usr/include/stdlib.h
../gui/gui_textbox.o: /usr/include/features.h /usr/include/sys/cdefs.h
../gui/gui_textbox.o: /usr/include/gnu/stubs.h
../gui/gui_textbox.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
../gui/gui_textbox.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
../gui/gui_textbox.o: /usr/include/stdio.h /usr/include/bits/types.h
../gui/gui_textbox.o: /usr/include/bits/wordsize.h
../gui/gui_textbox.o: /usr/include/bits/typesizes.h /usr/include/libio.h
../gui/gui_textbox.o: /usr/include/_G_config.h /usr/include/wchar.h
../gui/gui_textbox.o: /usr/include/bits/wchar.h /usr/include/gconv.h
../gui/gui_textbox.o: /usr/include/bits/stdio_lim.h
../gui/gui_textbox.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
../gui/gui_textbox.o: ../Core/savage_mathlib.h /usr/include/math.h
../gui/gui_textbox.o: /usr/include/bits/huge_val.h
../gui/gui_textbox.o: /usr/include/bits/mathdef.h
../gui/gui_textbox.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
../gui/gui_textbox.o: /usr/include/time.h /usr/include/bits/time.h
../gui/gui_textbox.o: ../Core/core_api.h ../gui/gui_textbox.h
../gui/gui_thumbnailgrid.o: ../toplayer/tl_shared.h ../toplayer/savage.h
../gui/gui_thumbnailgrid.o: ../Core/savage_types.h
../gui/gui_thumbnailgrid.o: ../masterserver/sharedcommon.h
../gui/gui_thumbnailgrid.o: ../Core/savage_common.h /usr/include/stdlib.h
../gui/gui_thumbnailgrid.o: /usr/include/features.h /usr/include/sys/cdefs.h
../gui/gui_thumbnailgrid.o: /usr/include/gnu/stubs.h
../gui/gui_thumbnailgrid.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
../gui/gui_thumbnailgrid.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
../gui/gui_thumbnailgrid.o: /usr/include/stdio.h /usr/include/bits/types.h
../gui/gui_thumbnailgrid.o: /usr/include/bits/wordsize.h
../gui/gui_thumbnailgrid.o: /usr/include/bits/typesizes.h
../gui/gui_thumbnailgrid.o: /usr/include/libio.h /usr/include/_G_config.h
../gui/gui_thumbnailgrid.o: /usr/include/wchar.h /usr/include/bits/wchar.h
../gui/gui_thumbnailgrid.o: /usr/include/gconv.h
../gui/gui_thumbnailgrid.o: /usr/include/bits/stdio_lim.h
../gui/gui_thumbnailgrid.o: /usr/include/bits/sys_errlist.h
../gui/gui_thumbnailgrid.o: /usr/include/string.h ../Core/savage_mathlib.h
../gui/gui_thumbnailgrid.o: /usr/include/math.h /usr/include/bits/huge_val.h
../gui/gui_thumbnailgrid.o: /usr/include/bits/mathdef.h
../gui/gui_thumbnailgrid.o: /usr/include/bits/mathcalls.h
../gui/gui_thumbnailgrid.o: ../Core/camerautils.h /usr/include/time.h
../gui/gui_thumbnailgrid.o: /usr/include/bits/time.h ../Core/core_api.h
../gui/gui_thumbnailgrid.o: ../gui/gui_thumbnailgrid.h
../gui/gui_togglebutton.o: ../toplayer/tl_shared.h ../toplayer/savage.h
../gui/gui_togglebutton.o: ../Core/savage_types.h
../gui/gui_togglebutton.o: ../masterserver/sharedcommon.h
../gui/gui_togglebutton.o: ../Core/savage_common.h /usr/include/stdlib.h
../gui/gui_togglebutton.o: /usr/include/features.h /usr/include/sys/cdefs.h
../gui/gui_togglebutton.o: /usr/include/gnu/stubs.h
../gui/gui_togglebutton.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
../gui/gui_togglebutton.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
../gui/gui_togglebutton.o: /usr/include/stdio.h /usr/include/bits/types.h
../gui/gui_togglebutton.o: /usr/include/bits/wordsize.h
../gui/gui_togglebutton.o: /usr/include/bits/typesizes.h /usr/include/libio.h
../gui/gui_togglebutton.o: /usr/include/_G_config.h /usr/include/wchar.h
../gui/gui_togglebutton.o: /usr/include/bits/wchar.h /usr/include/gconv.h
../gui/gui_togglebutton.o: /usr/include/bits/stdio_lim.h
../gui/gui_togglebutton.o: /usr/include/bits/sys_errlist.h
../gui/gui_togglebutton.o: /usr/include/string.h ../Core/savage_mathlib.h
../gui/gui_togglebutton.o: /usr/include/math.h /usr/include/bits/huge_val.h
../gui/gui_togglebutton.o: /usr/include/bits/mathdef.h
../gui/gui_togglebutton.o: /usr/include/bits/mathcalls.h
../gui/gui_togglebutton.o: ../Core/camerautils.h /usr/include/time.h
../gui/gui_togglebutton.o: /usr/include/bits/time.h ../Core/core_api.h
../gui/gui_togglebutton.o: ../gui/gui_togglebutton.h ../gui/gui_button.h
../gui/gui_map.o: ../TheGame/client_game.h game.h ../toplayer/savage.h
../gui/gui_map.o: ../Core/savage_types.h ../masterserver/sharedcommon.h
../gui/gui_map.o: ../Core/savage_common.h /usr/include/stdlib.h
../gui/gui_map.o: /usr/include/features.h /usr/include/sys/cdefs.h
../gui/gui_map.o: /usr/include/gnu/stubs.h
../gui/gui_map.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
../gui/gui_map.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
../gui/gui_map.o: /usr/include/stdio.h /usr/include/bits/types.h
../gui/gui_map.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
../gui/gui_map.o: /usr/include/libio.h /usr/include/_G_config.h
../gui/gui_map.o: /usr/include/wchar.h /usr/include/bits/wchar.h
../gui/gui_map.o: /usr/include/gconv.h /usr/include/bits/stdio_lim.h
../gui/gui_map.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
../gui/gui_map.o: ../Core/savage_mathlib.h /usr/include/math.h
../gui/gui_map.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
../gui/gui_map.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
../gui/gui_map.o: /usr/include/time.h /usr/include/bits/time.h
../gui/gui_map.o: ../Core/core_api.h ../toplayer/tl_shared.h interface.h
../gui/gui_map.o: ../gui/gui_scrollbuffer.h ../gui/gui_orderedscrollbuffer.h
../gui/gui_map.o: ../gui/gui_textbuffer.h ../gui/gui_button.h
../gui/gui_map.o: ../gui/gui_graphic.h ../gui/gui_label.h ../gui/gui_map.h
../gui/gui_playermap.o: ../TheGame/client_game.h game.h ../toplayer/savage.h
../gui/gui_playermap.o: ../Core/savage_types.h ../masterserver/sharedcommon.h
../gui/gui_playermap.o: ../Core/savage_common.h /usr/include/stdlib.h
../gui/gui_playermap.o: /usr/include/features.h /usr/include/sys/cdefs.h
../gui/gui_playermap.o: /usr/include/gnu/stubs.h
../gui/gui_playermap.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
../gui/gui_playermap.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
../gui/gui_playermap.o: /usr/include/stdio.h /usr/include/bits/types.h
../gui/gui_playermap.o: /usr/include/bits/wordsize.h
../gui/gui_playermap.o: /usr/include/bits/typesizes.h /usr/include/libio.h
../gui/gui_playermap.o: /usr/include/_G_config.h /usr/include/wchar.h
../gui/gui_playermap.o: /usr/include/bits/wchar.h /usr/include/gconv.h
../gui/gui_playermap.o: /usr/include/bits/stdio_lim.h
../gui/gui_playermap.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
../gui/gui_playermap.o: ../Core/savage_mathlib.h /usr/include/math.h
../gui/gui_playermap.o: /usr/include/bits/huge_val.h
../gui/gui_playermap.o: /usr/include/bits/mathdef.h
../gui/gui_playermap.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
../gui/gui_playermap.o: /usr/include/time.h /usr/include/bits/time.h
../gui/gui_playermap.o: ../Core/core_api.h ../toplayer/tl_shared.h
../gui/gui_playermap.o: interface.h ../gui/gui_scrollbuffer.h
../gui/gui_playermap.o: ../gui/gui_orderedscrollbuffer.h
../gui/gui_playermap.o: ../gui/gui_textbuffer.h ../gui/gui_button.h
../gui/gui_playermap.o: ../gui/gui_graphic.h ../gui/gui_label.h
../gui/gui_playermap.o: ../gui/gui_playermap.h
../gui/gui_userlist_ping.o: ../gui/gui_userlist_ping.h client_game.h game.h
../gui/gui_userlist_ping.o: ../toplayer/savage.h ../Core/savage_types.h
../gui/gui_userlist_ping.o: ../masterserver/sharedcommon.h
../gui/gui_userlist_ping.o: ../Core/savage_common.h /usr/include/stdlib.h
../gui/gui_userlist_ping.o: /usr/include/features.h /usr/include/sys/cdefs.h
../gui/gui_userlist_ping.o: /usr/include/gnu/stubs.h
../gui/gui_userlist_ping.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
../gui/gui_userlist_ping.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
../gui/gui_userlist_ping.o: /usr/include/stdio.h /usr/include/bits/types.h
../gui/gui_userlist_ping.o: /usr/include/bits/wordsize.h
../gui/gui_userlist_ping.o: /usr/include/bits/typesizes.h
../gui/gui_userlist_ping.o: /usr/include/libio.h /usr/include/_G_config.h
../gui/gui_userlist_ping.o: /usr/include/wchar.h /usr/include/bits/wchar.h
../gui/gui_userlist_ping.o: /usr/include/gconv.h
../gui/gui_userlist_ping.o: /usr/include/bits/stdio_lim.h
../gui/gui_userlist_ping.o: /usr/include/bits/sys_errlist.h
../gui/gui_userlist_ping.o: /usr/include/string.h ../Core/savage_mathlib.h
../gui/gui_userlist_ping.o: /usr/include/math.h /usr/include/bits/huge_val.h
../gui/gui_userlist_ping.o: /usr/include/bits/mathdef.h
../gui/gui_userlist_ping.o: /usr/include/bits/mathcalls.h
../gui/gui_userlist_ping.o: ../Core/camerautils.h /usr/include/time.h
../gui/gui_userlist_ping.o: /usr/include/bits/time.h ../Core/core_api.h
../gui/gui_userlist_ping.o: ../toplayer/tl_shared.h interface.h
../gui/gui_userlist_ping.o: ../gui/gui_scrollbuffer.h
../gui/gui_userlist_ping.o: ../gui/gui_orderedscrollbuffer.h
../gui/gui_userlist_ping.o: ../gui/gui_textbuffer.h ../gui/gui_button.h
../gui/gui_userlist_ping.o: ../gui/gui_graphic.h ../gui/gui_label.h
cl_animation.o: client_game.h game.h ../toplayer/savage.h
cl_animation.o: ../Core/savage_types.h ../masterserver/sharedcommon.h
cl_animation.o: ../Core/savage_common.h /usr/include/stdlib.h
cl_animation.o: /usr/include/features.h /usr/include/sys/cdefs.h
cl_animation.o: /usr/include/gnu/stubs.h
cl_animation.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
cl_animation.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
cl_animation.o: /usr/include/stdio.h /usr/include/bits/types.h
cl_animation.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
cl_animation.o: /usr/include/libio.h /usr/include/_G_config.h
cl_animation.o: /usr/include/wchar.h /usr/include/bits/wchar.h
cl_animation.o: /usr/include/gconv.h /usr/include/bits/stdio_lim.h
cl_animation.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
cl_animation.o: ../Core/savage_mathlib.h /usr/include/math.h
cl_animation.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
cl_animation.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
cl_animation.o: /usr/include/time.h /usr/include/bits/time.h
cl_animation.o: ../Core/core_api.h ../toplayer/tl_shared.h interface.h
cl_animation.o: ../gui/gui_scrollbuffer.h ../gui/gui_orderedscrollbuffer.h
cl_animation.o: ../gui/gui_textbuffer.h ../gui/gui_button.h
cl_animation.o: ../gui/gui_graphic.h ../gui/gui_label.h
cl_animation.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h
cl_ask.o: client_game.h game.h ../toplayer/savage.h ../Core/savage_types.h
cl_ask.o: ../masterserver/sharedcommon.h ../Core/savage_common.h
cl_ask.o: /usr/include/stdlib.h /usr/include/features.h
cl_ask.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
cl_ask.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
cl_ask.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
cl_ask.o: /usr/include/stdio.h /usr/include/bits/types.h
cl_ask.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
cl_ask.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
cl_ask.o: /usr/include/bits/wchar.h /usr/include/gconv.h
cl_ask.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
cl_ask.o: /usr/include/string.h ../Core/savage_mathlib.h /usr/include/math.h
cl_ask.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
cl_ask.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
cl_ask.o: /usr/include/time.h /usr/include/bits/time.h ../Core/core_api.h
cl_ask.o: ../toplayer/tl_shared.h interface.h ../gui/gui_scrollbuffer.h
cl_ask.o: ../gui/gui_orderedscrollbuffer.h ../gui/gui_textbuffer.h
cl_ask.o: ../gui/gui_button.h ../gui/gui_graphic.h ../gui/gui_label.h
cl_cmdr_interface.o: client_game.h game.h ../toplayer/savage.h
cl_cmdr_interface.o: ../Core/savage_types.h ../masterserver/sharedcommon.h
cl_cmdr_interface.o: ../Core/savage_common.h /usr/include/stdlib.h
cl_cmdr_interface.o: /usr/include/features.h /usr/include/sys/cdefs.h
cl_cmdr_interface.o: /usr/include/gnu/stubs.h
cl_cmdr_interface.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
cl_cmdr_interface.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
cl_cmdr_interface.o: /usr/include/stdio.h /usr/include/bits/types.h
cl_cmdr_interface.o: /usr/include/bits/wordsize.h
cl_cmdr_interface.o: /usr/include/bits/typesizes.h /usr/include/libio.h
cl_cmdr_interface.o: /usr/include/_G_config.h /usr/include/wchar.h
cl_cmdr_interface.o: /usr/include/bits/wchar.h /usr/include/gconv.h
cl_cmdr_interface.o: /usr/include/bits/stdio_lim.h
cl_cmdr_interface.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
cl_cmdr_interface.o: ../Core/savage_mathlib.h /usr/include/math.h
cl_cmdr_interface.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
cl_cmdr_interface.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
cl_cmdr_interface.o: /usr/include/time.h /usr/include/bits/time.h
cl_cmdr_interface.o: ../Core/core_api.h ../toplayer/tl_shared.h interface.h
cl_cmdr_interface.o: ../gui/gui_scrollbuffer.h
cl_cmdr_interface.o: ../gui/gui_orderedscrollbuffer.h ../gui/gui_textbuffer.h
cl_cmdr_interface.o: ../gui/gui_button.h ../gui/gui_graphic.h
cl_cmdr_interface.o: ../gui/gui_label.h ../gui/gui_slider.h
cl_cmdr_requests.o: client_game.h game.h ../toplayer/savage.h
cl_cmdr_requests.o: ../Core/savage_types.h ../masterserver/sharedcommon.h
cl_cmdr_requests.o: ../Core/savage_common.h /usr/include/stdlib.h
cl_cmdr_requests.o: /usr/include/features.h /usr/include/sys/cdefs.h
cl_cmdr_requests.o: /usr/include/gnu/stubs.h
cl_cmdr_requests.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
cl_cmdr_requests.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
cl_cmdr_requests.o: /usr/include/stdio.h /usr/include/bits/types.h
cl_cmdr_requests.o: /usr/include/bits/wordsize.h
cl_cmdr_requests.o: /usr/include/bits/typesizes.h /usr/include/libio.h
cl_cmdr_requests.o: /usr/include/_G_config.h /usr/include/wchar.h
cl_cmdr_requests.o: /usr/include/bits/wchar.h /usr/include/gconv.h
cl_cmdr_requests.o: /usr/include/bits/stdio_lim.h
cl_cmdr_requests.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
cl_cmdr_requests.o: ../Core/savage_mathlib.h /usr/include/math.h
cl_cmdr_requests.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
cl_cmdr_requests.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
cl_cmdr_requests.o: /usr/include/time.h /usr/include/bits/time.h
cl_cmdr_requests.o: ../Core/core_api.h ../toplayer/tl_shared.h interface.h
cl_cmdr_requests.o: ../gui/gui_scrollbuffer.h
cl_cmdr_requests.o: ../gui/gui_orderedscrollbuffer.h ../gui/gui_textbuffer.h
cl_cmdr_requests.o: ../gui/gui_button.h ../gui/gui_graphic.h
cl_cmdr_requests.o: ../gui/gui_label.h
cl_cmdr_gridmenu.o: client_game.h game.h ../toplayer/savage.h
cl_cmdr_gridmenu.o: ../Core/savage_types.h ../masterserver/sharedcommon.h
cl_cmdr_gridmenu.o: ../Core/savage_common.h /usr/include/stdlib.h
cl_cmdr_gridmenu.o: /usr/include/features.h /usr/include/sys/cdefs.h
cl_cmdr_gridmenu.o: /usr/include/gnu/stubs.h
cl_cmdr_gridmenu.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
cl_cmdr_gridmenu.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
cl_cmdr_gridmenu.o: /usr/include/stdio.h /usr/include/bits/types.h
cl_cmdr_gridmenu.o: /usr/include/bits/wordsize.h
cl_cmdr_gridmenu.o: /usr/include/bits/typesizes.h /usr/include/libio.h
cl_cmdr_gridmenu.o: /usr/include/_G_config.h /usr/include/wchar.h
cl_cmdr_gridmenu.o: /usr/include/bits/wchar.h /usr/include/gconv.h
cl_cmdr_gridmenu.o: /usr/include/bits/stdio_lim.h
cl_cmdr_gridmenu.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
cl_cmdr_gridmenu.o: ../Core/savage_mathlib.h /usr/include/math.h
cl_cmdr_gridmenu.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
cl_cmdr_gridmenu.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
cl_cmdr_gridmenu.o: /usr/include/time.h /usr/include/bits/time.h
cl_cmdr_gridmenu.o: ../Core/core_api.h ../toplayer/tl_shared.h interface.h
cl_cmdr_gridmenu.o: ../gui/gui_scrollbuffer.h
cl_cmdr_gridmenu.o: ../gui/gui_orderedscrollbuffer.h ../gui/gui_textbuffer.h
cl_cmdr_gridmenu.o: ../gui/gui_button.h ../gui/gui_graphic.h
cl_cmdr_gridmenu.o: ../gui/gui_label.h
cl_cmdr_officer.o: client_game.h game.h ../toplayer/savage.h
cl_cmdr_officer.o: ../Core/savage_types.h ../masterserver/sharedcommon.h
cl_cmdr_officer.o: ../Core/savage_common.h /usr/include/stdlib.h
cl_cmdr_officer.o: /usr/include/features.h /usr/include/sys/cdefs.h
cl_cmdr_officer.o: /usr/include/gnu/stubs.h
cl_cmdr_officer.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
cl_cmdr_officer.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
cl_cmdr_officer.o: /usr/include/stdio.h /usr/include/bits/types.h
cl_cmdr_officer.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
cl_cmdr_officer.o: /usr/include/libio.h /usr/include/_G_config.h
cl_cmdr_officer.o: /usr/include/wchar.h /usr/include/bits/wchar.h
cl_cmdr_officer.o: /usr/include/gconv.h /usr/include/bits/stdio_lim.h
cl_cmdr_officer.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
cl_cmdr_officer.o: ../Core/savage_mathlib.h /usr/include/math.h
cl_cmdr_officer.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
cl_cmdr_officer.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
cl_cmdr_officer.o: /usr/include/time.h /usr/include/bits/time.h
cl_cmdr_officer.o: ../Core/core_api.h ../toplayer/tl_shared.h interface.h
cl_cmdr_officer.o: ../gui/gui_scrollbuffer.h ../gui/gui_orderedscrollbuffer.h
cl_cmdr_officer.o: ../gui/gui_textbuffer.h ../gui/gui_button.h
cl_cmdr_officer.o: ../gui/gui_graphic.h ../gui/gui_label.h
cl_cmdr_powerups.o: client_game.h game.h ../toplayer/savage.h
cl_cmdr_powerups.o: ../Core/savage_types.h ../masterserver/sharedcommon.h
cl_cmdr_powerups.o: ../Core/savage_common.h /usr/include/stdlib.h
cl_cmdr_powerups.o: /usr/include/features.h /usr/include/sys/cdefs.h
cl_cmdr_powerups.o: /usr/include/gnu/stubs.h
cl_cmdr_powerups.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
cl_cmdr_powerups.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
cl_cmdr_powerups.o: /usr/include/stdio.h /usr/include/bits/types.h
cl_cmdr_powerups.o: /usr/include/bits/wordsize.h
cl_cmdr_powerups.o: /usr/include/bits/typesizes.h /usr/include/libio.h
cl_cmdr_powerups.o: /usr/include/_G_config.h /usr/include/wchar.h
cl_cmdr_powerups.o: /usr/include/bits/wchar.h /usr/include/gconv.h
cl_cmdr_powerups.o: /usr/include/bits/stdio_lim.h
cl_cmdr_powerups.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
cl_cmdr_powerups.o: ../Core/savage_mathlib.h /usr/include/math.h
cl_cmdr_powerups.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
cl_cmdr_powerups.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
cl_cmdr_powerups.o: /usr/include/time.h /usr/include/bits/time.h
cl_cmdr_powerups.o: ../Core/core_api.h ../toplayer/tl_shared.h interface.h
cl_cmdr_powerups.o: ../gui/gui_scrollbuffer.h
cl_cmdr_powerups.o: ../gui/gui_orderedscrollbuffer.h ../gui/gui_textbuffer.h
cl_cmdr_powerups.o: ../gui/gui_button.h ../gui/gui_graphic.h
cl_cmdr_powerups.o: ../gui/gui_label.h
cl_main.o: client_game.h game.h ../toplayer/savage.h ../Core/savage_types.h
cl_main.o: ../masterserver/sharedcommon.h ../Core/savage_common.h
cl_main.o: /usr/include/stdlib.h /usr/include/features.h
cl_main.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
cl_main.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
cl_main.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
cl_main.o: /usr/include/stdio.h /usr/include/bits/types.h
cl_main.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
cl_main.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
cl_main.o: /usr/include/bits/wchar.h /usr/include/gconv.h
cl_main.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
cl_main.o: /usr/include/string.h ../Core/savage_mathlib.h /usr/include/math.h
cl_main.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
cl_main.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
cl_main.o: /usr/include/time.h /usr/include/bits/time.h ../Core/core_api.h
cl_main.o: ../toplayer/tl_shared.h interface.h ../gui/gui_scrollbuffer.h
cl_main.o: ../gui/gui_orderedscrollbuffer.h ../gui/gui_textbuffer.h
cl_main.o: ../gui/gui_button.h ../gui/gui_graphic.h ../gui/gui_label.h
cl_events.o: client_game.h game.h ../toplayer/savage.h ../Core/savage_types.h
cl_events.o: ../masterserver/sharedcommon.h ../Core/savage_common.h
cl_events.o: /usr/include/stdlib.h /usr/include/features.h
cl_events.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
cl_events.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
cl_events.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
cl_events.o: /usr/include/stdio.h /usr/include/bits/types.h
cl_events.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
cl_events.o: /usr/include/libio.h /usr/include/_G_config.h
cl_events.o: /usr/include/wchar.h /usr/include/bits/wchar.h
cl_events.o: /usr/include/gconv.h /usr/include/bits/stdio_lim.h
cl_events.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
cl_events.o: ../Core/savage_mathlib.h /usr/include/math.h
cl_events.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
cl_events.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
cl_events.o: /usr/include/time.h /usr/include/bits/time.h ../Core/core_api.h
cl_events.o: ../toplayer/tl_shared.h interface.h ../gui/gui_scrollbuffer.h
cl_events.o: ../gui/gui_orderedscrollbuffer.h ../gui/gui_textbuffer.h
cl_events.o: ../gui/gui_button.h ../gui/gui_graphic.h ../gui/gui_label.h
cl_effects.o: client_game.h game.h ../toplayer/savage.h
cl_effects.o: ../Core/savage_types.h ../masterserver/sharedcommon.h
cl_effects.o: ../Core/savage_common.h /usr/include/stdlib.h
cl_effects.o: /usr/include/features.h /usr/include/sys/cdefs.h
cl_effects.o: /usr/include/gnu/stubs.h
cl_effects.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
cl_effects.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
cl_effects.o: /usr/include/stdio.h /usr/include/bits/types.h
cl_effects.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
cl_effects.o: /usr/include/libio.h /usr/include/_G_config.h
cl_effects.o: /usr/include/wchar.h /usr/include/bits/wchar.h
cl_effects.o: /usr/include/gconv.h /usr/include/bits/stdio_lim.h
cl_effects.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
cl_effects.o: ../Core/savage_mathlib.h /usr/include/math.h
cl_effects.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
cl_effects.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
cl_effects.o: /usr/include/time.h /usr/include/bits/time.h ../Core/core_api.h
cl_effects.o: ../toplayer/tl_shared.h interface.h ../gui/gui_scrollbuffer.h
cl_effects.o: ../gui/gui_orderedscrollbuffer.h ../gui/gui_textbuffer.h
cl_effects.o: ../gui/gui_button.h ../gui/gui_graphic.h ../gui/gui_label.h
cl_render.o: client_game.h game.h ../toplayer/savage.h ../Core/savage_types.h
cl_render.o: ../masterserver/sharedcommon.h ../Core/savage_common.h
cl_render.o: /usr/include/stdlib.h /usr/include/features.h
cl_render.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
cl_render.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
cl_render.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
cl_render.o: /usr/include/stdio.h /usr/include/bits/types.h
cl_render.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
cl_render.o: /usr/include/libio.h /usr/include/_G_config.h
cl_render.o: /usr/include/wchar.h /usr/include/bits/wchar.h
cl_render.o: /usr/include/gconv.h /usr/include/bits/stdio_lim.h
cl_render.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
cl_render.o: ../Core/savage_mathlib.h /usr/include/math.h
cl_render.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
cl_render.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
cl_render.o: /usr/include/time.h /usr/include/bits/time.h ../Core/core_api.h
cl_render.o: ../toplayer/tl_shared.h interface.h ../gui/gui_scrollbuffer.h
cl_render.o: ../gui/gui_orderedscrollbuffer.h ../gui/gui_textbuffer.h
cl_render.o: ../gui/gui_button.h ../gui/gui_graphic.h ../gui/gui_label.h
cl_render.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h
cl_environment.o: client_game.h game.h ../toplayer/savage.h
cl_environment.o: ../Core/savage_types.h ../masterserver/sharedcommon.h
cl_environment.o: ../Core/savage_common.h /usr/include/stdlib.h
cl_environment.o: /usr/include/features.h /usr/include/sys/cdefs.h
cl_environment.o: /usr/include/gnu/stubs.h
cl_environment.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
cl_environment.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
cl_environment.o: /usr/include/stdio.h /usr/include/bits/types.h
cl_environment.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
cl_environment.o: /usr/include/libio.h /usr/include/_G_config.h
cl_environment.o: /usr/include/wchar.h /usr/include/bits/wchar.h
cl_environment.o: /usr/include/gconv.h /usr/include/bits/stdio_lim.h
cl_environment.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
cl_environment.o: ../Core/savage_mathlib.h /usr/include/math.h
cl_environment.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
cl_environment.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
cl_environment.o: /usr/include/time.h /usr/include/bits/time.h
cl_environment.o: ../Core/core_api.h ../toplayer/tl_shared.h interface.h
cl_environment.o: ../gui/gui_scrollbuffer.h ../gui/gui_orderedscrollbuffer.h
cl_environment.o: ../gui/gui_textbuffer.h ../gui/gui_button.h
cl_environment.o: ../gui/gui_graphic.h ../gui/gui_label.h
cl_resources.o: client_game.h game.h ../toplayer/savage.h
cl_resources.o: ../Core/savage_types.h ../masterserver/sharedcommon.h
cl_resources.o: ../Core/savage_common.h /usr/include/stdlib.h
cl_resources.o: /usr/include/features.h /usr/include/sys/cdefs.h
cl_resources.o: /usr/include/gnu/stubs.h
cl_resources.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
cl_resources.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
cl_resources.o: /usr/include/stdio.h /usr/include/bits/types.h
cl_resources.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
cl_resources.o: /usr/include/libio.h /usr/include/_G_config.h
cl_resources.o: /usr/include/wchar.h /usr/include/bits/wchar.h
cl_resources.o: /usr/include/gconv.h /usr/include/bits/stdio_lim.h
cl_resources.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
cl_resources.o: ../Core/savage_mathlib.h /usr/include/math.h
cl_resources.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
cl_resources.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
cl_resources.o: /usr/include/time.h /usr/include/bits/time.h
cl_resources.o: ../Core/core_api.h ../toplayer/tl_shared.h interface.h
cl_resources.o: ../gui/gui_scrollbuffer.h ../gui/gui_orderedscrollbuffer.h
cl_resources.o: ../gui/gui_textbuffer.h ../gui/gui_button.h
cl_resources.o: ../gui/gui_graphic.h ../gui/gui_label.h
cl_playerstate.o: client_game.h game.h ../toplayer/savage.h
cl_playerstate.o: ../Core/savage_types.h ../masterserver/sharedcommon.h
cl_playerstate.o: ../Core/savage_common.h /usr/include/stdlib.h
cl_playerstate.o: /usr/include/features.h /usr/include/sys/cdefs.h
cl_playerstate.o: /usr/include/gnu/stubs.h
cl_playerstate.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
cl_playerstate.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
cl_playerstate.o: /usr/include/stdio.h /usr/include/bits/types.h
cl_playerstate.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
cl_playerstate.o: /usr/include/libio.h /usr/include/_G_config.h
cl_playerstate.o: /usr/include/wchar.h /usr/include/bits/wchar.h
cl_playerstate.o: /usr/include/gconv.h /usr/include/bits/stdio_lim.h
cl_playerstate.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
cl_playerstate.o: ../Core/savage_mathlib.h /usr/include/math.h
cl_playerstate.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
cl_playerstate.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
cl_playerstate.o: /usr/include/time.h /usr/include/bits/time.h
cl_playerstate.o: ../Core/core_api.h ../toplayer/tl_shared.h interface.h
cl_playerstate.o: ../gui/gui_scrollbuffer.h ../gui/gui_orderedscrollbuffer.h
cl_playerstate.o: ../gui/gui_textbuffer.h ../gui/gui_button.h
cl_playerstate.o: ../gui/gui_graphic.h ../gui/gui_label.h
cl_commander.o: client_game.h game.h ../toplayer/savage.h
cl_commander.o: ../Core/savage_types.h ../masterserver/sharedcommon.h
cl_commander.o: ../Core/savage_common.h /usr/include/stdlib.h
cl_commander.o: /usr/include/features.h /usr/include/sys/cdefs.h
cl_commander.o: /usr/include/gnu/stubs.h
cl_commander.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
cl_commander.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
cl_commander.o: /usr/include/stdio.h /usr/include/bits/types.h
cl_commander.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
cl_commander.o: /usr/include/libio.h /usr/include/_G_config.h
cl_commander.o: /usr/include/wchar.h /usr/include/bits/wchar.h
cl_commander.o: /usr/include/gconv.h /usr/include/bits/stdio_lim.h
cl_commander.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
cl_commander.o: ../Core/savage_mathlib.h /usr/include/math.h
cl_commander.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
cl_commander.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
cl_commander.o: /usr/include/time.h /usr/include/bits/time.h
cl_commander.o: ../Core/core_api.h ../toplayer/tl_shared.h interface.h
cl_commander.o: ../gui/gui_scrollbuffer.h ../gui/gui_orderedscrollbuffer.h
cl_commander.o: ../gui/gui_textbuffer.h ../gui/gui_button.h
cl_commander.o: ../gui/gui_graphic.h ../gui/gui_label.h
cl_objects.o: client_game.h game.h ../toplayer/savage.h
cl_objects.o: ../Core/savage_types.h ../masterserver/sharedcommon.h
cl_objects.o: ../Core/savage_common.h /usr/include/stdlib.h
cl_objects.o: /usr/include/features.h /usr/include/sys/cdefs.h
cl_objects.o: /usr/include/gnu/stubs.h
cl_objects.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
cl_objects.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
cl_objects.o: /usr/include/stdio.h /usr/include/bits/types.h
cl_objects.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
cl_objects.o: /usr/include/libio.h /usr/include/_G_config.h
cl_objects.o: /usr/include/wchar.h /usr/include/bits/wchar.h
cl_objects.o: /usr/include/gconv.h /usr/include/bits/stdio_lim.h
cl_objects.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
cl_objects.o: ../Core/savage_mathlib.h /usr/include/math.h
cl_objects.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
cl_objects.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
cl_objects.o: /usr/include/time.h /usr/include/bits/time.h ../Core/core_api.h
cl_objects.o: ../toplayer/tl_shared.h interface.h ../gui/gui_scrollbuffer.h
cl_objects.o: ../gui/gui_orderedscrollbuffer.h ../gui/gui_textbuffer.h
cl_objects.o: ../gui/gui_button.h ../gui/gui_graphic.h ../gui/gui_label.h
cl_camera.o: client_game.h game.h ../toplayer/savage.h ../Core/savage_types.h
cl_camera.o: ../masterserver/sharedcommon.h ../Core/savage_common.h
cl_camera.o: /usr/include/stdlib.h /usr/include/features.h
cl_camera.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
cl_camera.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
cl_camera.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
cl_camera.o: /usr/include/stdio.h /usr/include/bits/types.h
cl_camera.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
cl_camera.o: /usr/include/libio.h /usr/include/_G_config.h
cl_camera.o: /usr/include/wchar.h /usr/include/bits/wchar.h
cl_camera.o: /usr/include/gconv.h /usr/include/bits/stdio_lim.h
cl_camera.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
cl_camera.o: ../Core/savage_mathlib.h /usr/include/math.h
cl_camera.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
cl_camera.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
cl_camera.o: /usr/include/time.h /usr/include/bits/time.h ../Core/core_api.h
cl_camera.o: ../toplayer/tl_shared.h interface.h ../gui/gui_scrollbuffer.h
cl_camera.o: ../gui/gui_orderedscrollbuffer.h ../gui/gui_textbuffer.h
cl_camera.o: ../gui/gui_button.h ../gui/gui_graphic.h ../gui/gui_label.h
cl_drawutils.o: client_game.h game.h ../toplayer/savage.h
cl_drawutils.o: ../Core/savage_types.h ../masterserver/sharedcommon.h
cl_drawutils.o: ../Core/savage_common.h /usr/include/stdlib.h
cl_drawutils.o: /usr/include/features.h /usr/include/sys/cdefs.h
cl_drawutils.o: /usr/include/gnu/stubs.h
cl_drawutils.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
cl_drawutils.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
cl_drawutils.o: /usr/include/stdio.h /usr/include/bits/types.h
cl_drawutils.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
cl_drawutils.o: /usr/include/libio.h /usr/include/_G_config.h
cl_drawutils.o: /usr/include/wchar.h /usr/include/bits/wchar.h
cl_drawutils.o: /usr/include/gconv.h /usr/include/bits/stdio_lim.h
cl_drawutils.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
cl_drawutils.o: ../Core/savage_mathlib.h /usr/include/math.h
cl_drawutils.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
cl_drawutils.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
cl_drawutils.o: /usr/include/time.h /usr/include/bits/time.h
cl_drawutils.o: ../Core/core_api.h ../toplayer/tl_shared.h interface.h
cl_drawutils.o: ../gui/gui_scrollbuffer.h ../gui/gui_orderedscrollbuffer.h
cl_drawutils.o: ../gui/gui_textbuffer.h ../gui/gui_button.h
cl_drawutils.o: ../gui/gui_graphic.h ../gui/gui_label.h
cl_simpleparticles.o: client_game.h game.h ../toplayer/savage.h
cl_simpleparticles.o: ../Core/savage_types.h ../masterserver/sharedcommon.h
cl_simpleparticles.o: ../Core/savage_common.h /usr/include/stdlib.h
cl_simpleparticles.o: /usr/include/features.h /usr/include/sys/cdefs.h
cl_simpleparticles.o: /usr/include/gnu/stubs.h
cl_simpleparticles.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
cl_simpleparticles.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
cl_simpleparticles.o: /usr/include/stdio.h /usr/include/bits/types.h
cl_simpleparticles.o: /usr/include/bits/wordsize.h
cl_simpleparticles.o: /usr/include/bits/typesizes.h /usr/include/libio.h
cl_simpleparticles.o: /usr/include/_G_config.h /usr/include/wchar.h
cl_simpleparticles.o: /usr/include/bits/wchar.h /usr/include/gconv.h
cl_simpleparticles.o: /usr/include/bits/stdio_lim.h
cl_simpleparticles.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
cl_simpleparticles.o: ../Core/savage_mathlib.h /usr/include/math.h
cl_simpleparticles.o: /usr/include/bits/huge_val.h
cl_simpleparticles.o: /usr/include/bits/mathdef.h
cl_simpleparticles.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
cl_simpleparticles.o: /usr/include/time.h /usr/include/bits/time.h
cl_simpleparticles.o: ../Core/core_api.h ../toplayer/tl_shared.h interface.h
cl_simpleparticles.o: ../gui/gui_scrollbuffer.h
cl_simpleparticles.o: ../gui/gui_orderedscrollbuffer.h
cl_simpleparticles.o: ../gui/gui_textbuffer.h ../gui/gui_button.h
cl_simpleparticles.o: ../gui/gui_graphic.h ../gui/gui_label.h
cl_vote.o: client_game.h game.h ../toplayer/savage.h ../Core/savage_types.h
cl_vote.o: ../masterserver/sharedcommon.h ../Core/savage_common.h
cl_vote.o: /usr/include/stdlib.h /usr/include/features.h
cl_vote.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
cl_vote.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
cl_vote.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
cl_vote.o: /usr/include/stdio.h /usr/include/bits/types.h
cl_vote.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
cl_vote.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
cl_vote.o: /usr/include/bits/wchar.h /usr/include/gconv.h
cl_vote.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
cl_vote.o: /usr/include/string.h ../Core/savage_mathlib.h /usr/include/math.h
cl_vote.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
cl_vote.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
cl_vote.o: /usr/include/time.h /usr/include/bits/time.h ../Core/core_api.h
cl_vote.o: ../toplayer/tl_shared.h interface.h ../gui/gui_scrollbuffer.h
cl_vote.o: ../gui/gui_orderedscrollbuffer.h ../gui/gui_textbuffer.h
cl_vote.o: ../gui/gui_button.h ../gui/gui_graphic.h ../gui/gui_label.h
cl_world.o: client_game.h game.h ../toplayer/savage.h ../Core/savage_types.h
cl_world.o: ../masterserver/sharedcommon.h ../Core/savage_common.h
cl_world.o: /usr/include/stdlib.h /usr/include/features.h
cl_world.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
cl_world.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
cl_world.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
cl_world.o: /usr/include/stdio.h /usr/include/bits/types.h
cl_world.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
cl_world.o: /usr/include/libio.h /usr/include/_G_config.h
cl_world.o: /usr/include/wchar.h /usr/include/bits/wchar.h
cl_world.o: /usr/include/gconv.h /usr/include/bits/stdio_lim.h
cl_world.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
cl_world.o: ../Core/savage_mathlib.h /usr/include/math.h
cl_world.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
cl_world.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
cl_world.o: /usr/include/time.h /usr/include/bits/time.h ../Core/core_api.h
cl_world.o: ../toplayer/tl_shared.h interface.h ../gui/gui_scrollbuffer.h
cl_world.o: ../gui/gui_orderedscrollbuffer.h ../gui/gui_textbuffer.h
cl_world.o: ../gui/gui_button.h ../gui/gui_graphic.h ../gui/gui_label.h
effects.o: game.h ../toplayer/savage.h ../Core/savage_types.h
effects.o: ../masterserver/sharedcommon.h ../Core/savage_common.h
effects.o: /usr/include/stdlib.h /usr/include/features.h
effects.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
effects.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
effects.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
effects.o: /usr/include/stdio.h /usr/include/bits/types.h
effects.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
effects.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
effects.o: /usr/include/bits/wchar.h /usr/include/gconv.h
effects.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
effects.o: /usr/include/string.h ../Core/savage_mathlib.h /usr/include/math.h
effects.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
effects.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
effects.o: /usr/include/time.h /usr/include/bits/time.h ../Core/core_api.h
expTable.o: game.h ../toplayer/savage.h ../Core/savage_types.h
expTable.o: ../masterserver/sharedcommon.h ../Core/savage_common.h
expTable.o: /usr/include/stdlib.h /usr/include/features.h
expTable.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
expTable.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
expTable.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
expTable.o: /usr/include/stdio.h /usr/include/bits/types.h
expTable.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
expTable.o: /usr/include/libio.h /usr/include/_G_config.h
expTable.o: /usr/include/wchar.h /usr/include/bits/wchar.h
expTable.o: /usr/include/gconv.h /usr/include/bits/stdio_lim.h
expTable.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
expTable.o: ../Core/savage_mathlib.h /usr/include/math.h
expTable.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
expTable.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
expTable.o: /usr/include/time.h /usr/include/bits/time.h ../Core/core_api.h
items.o: game.h ../toplayer/savage.h ../Core/savage_types.h
items.o: ../masterserver/sharedcommon.h ../Core/savage_common.h
items.o: /usr/include/stdlib.h /usr/include/features.h
items.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
items.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
items.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
items.o: /usr/include/stdio.h /usr/include/bits/types.h
items.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
items.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
items.o: /usr/include/bits/wchar.h /usr/include/gconv.h
items.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
items.o: /usr/include/string.h ../Core/savage_mathlib.h /usr/include/math.h
items.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
items.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
items.o: /usr/include/time.h /usr/include/bits/time.h ../Core/core_api.h
objdefs.o: game.h ../toplayer/savage.h ../Core/savage_types.h
objdefs.o: ../masterserver/sharedcommon.h ../Core/savage_common.h
objdefs.o: /usr/include/stdlib.h /usr/include/features.h
objdefs.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
objdefs.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
objdefs.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
objdefs.o: /usr/include/stdio.h /usr/include/bits/types.h
objdefs.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
objdefs.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
objdefs.o: /usr/include/bits/wchar.h /usr/include/gconv.h
objdefs.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
objdefs.o: /usr/include/string.h ../Core/savage_mathlib.h /usr/include/math.h
objdefs.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
objdefs.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
objdefs.o: /usr/include/time.h /usr/include/bits/time.h ../Core/core_api.h
object_config.o: game.h ../toplayer/savage.h ../Core/savage_types.h
object_config.o: ../masterserver/sharedcommon.h ../Core/savage_common.h
object_config.o: /usr/include/stdlib.h /usr/include/features.h
object_config.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
object_config.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
object_config.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
object_config.o: /usr/include/stdio.h /usr/include/bits/types.h
object_config.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
object_config.o: /usr/include/libio.h /usr/include/_G_config.h
object_config.o: /usr/include/wchar.h /usr/include/bits/wchar.h
object_config.o: /usr/include/gconv.h /usr/include/bits/stdio_lim.h
object_config.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
object_config.o: ../Core/savage_mathlib.h /usr/include/math.h
object_config.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
object_config.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
object_config.o: /usr/include/time.h /usr/include/bits/time.h
object_config.o: ../Core/core_api.h
misc_shared.o: game.h ../toplayer/savage.h ../Core/savage_types.h
misc_shared.o: ../masterserver/sharedcommon.h ../Core/savage_common.h
misc_shared.o: /usr/include/stdlib.h /usr/include/features.h
misc_shared.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
misc_shared.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
misc_shared.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
misc_shared.o: /usr/include/stdio.h /usr/include/bits/types.h
misc_shared.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
misc_shared.o: /usr/include/libio.h /usr/include/_G_config.h
misc_shared.o: /usr/include/wchar.h /usr/include/bits/wchar.h
misc_shared.o: /usr/include/gconv.h /usr/include/bits/stdio_lim.h
misc_shared.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
misc_shared.o: ../Core/savage_mathlib.h /usr/include/math.h
misc_shared.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
misc_shared.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
misc_shared.o: /usr/include/time.h /usr/include/bits/time.h
misc_shared.o: ../Core/core_api.h
upgrades.o: game.h ../toplayer/savage.h ../Core/savage_types.h
upgrades.o: ../masterserver/sharedcommon.h ../Core/savage_common.h
upgrades.o: /usr/include/stdlib.h /usr/include/features.h
upgrades.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
upgrades.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
upgrades.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
upgrades.o: /usr/include/stdio.h /usr/include/bits/types.h
upgrades.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
upgrades.o: /usr/include/libio.h /usr/include/_G_config.h
upgrades.o: /usr/include/wchar.h /usr/include/bits/wchar.h
upgrades.o: /usr/include/gconv.h /usr/include/bits/stdio_lim.h
upgrades.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
upgrades.o: ../Core/savage_mathlib.h /usr/include/math.h
upgrades.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
upgrades.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
upgrades.o: /usr/include/time.h /usr/include/bits/time.h ../Core/core_api.h
sv_ai.o: server_game.h game.h ../toplayer/savage.h ../Core/savage_types.h
sv_ai.o: ../masterserver/sharedcommon.h ../Core/savage_common.h
sv_ai.o: /usr/include/stdlib.h /usr/include/features.h
sv_ai.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
sv_ai.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
sv_ai.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
sv_ai.o: /usr/include/stdio.h /usr/include/bits/types.h
sv_ai.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
sv_ai.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
sv_ai.o: /usr/include/bits/wchar.h /usr/include/gconv.h
sv_ai.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
sv_ai.o: /usr/include/string.h ../Core/savage_mathlib.h /usr/include/math.h
sv_ai.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
sv_ai.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
sv_ai.o: /usr/include/time.h /usr/include/bits/time.h ../Core/core_api.h
sv_ai.o: sv_aiutil.h sv_ai.h sv_ainav.h sv_aigoal.h
sv_aigoal.o: server_game.h game.h ../toplayer/savage.h ../Core/savage_types.h
sv_aigoal.o: ../masterserver/sharedcommon.h ../Core/savage_common.h
sv_aigoal.o: /usr/include/stdlib.h /usr/include/features.h
sv_aigoal.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
sv_aigoal.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
sv_aigoal.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
sv_aigoal.o: /usr/include/stdio.h /usr/include/bits/types.h
sv_aigoal.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
sv_aigoal.o: /usr/include/libio.h /usr/include/_G_config.h
sv_aigoal.o: /usr/include/wchar.h /usr/include/bits/wchar.h
sv_aigoal.o: /usr/include/gconv.h /usr/include/bits/stdio_lim.h
sv_aigoal.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
sv_aigoal.o: ../Core/savage_mathlib.h /usr/include/math.h
sv_aigoal.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
sv_aigoal.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
sv_aigoal.o: /usr/include/time.h /usr/include/bits/time.h ../Core/core_api.h
sv_aigoal.o: sv_aiutil.h sv_aistate.h sv_aigoal.h sv_ainav.h sv_ai.h
sv_ainav.o: server_game.h game.h ../toplayer/savage.h ../Core/savage_types.h
sv_ainav.o: ../masterserver/sharedcommon.h ../Core/savage_common.h
sv_ainav.o: /usr/include/stdlib.h /usr/include/features.h
sv_ainav.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
sv_ainav.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
sv_ainav.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
sv_ainav.o: /usr/include/stdio.h /usr/include/bits/types.h
sv_ainav.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
sv_ainav.o: /usr/include/libio.h /usr/include/_G_config.h
sv_ainav.o: /usr/include/wchar.h /usr/include/bits/wchar.h
sv_ainav.o: /usr/include/gconv.h /usr/include/bits/stdio_lim.h
sv_ainav.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
sv_ainav.o: ../Core/savage_mathlib.h /usr/include/math.h
sv_ainav.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
sv_ainav.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
sv_ainav.o: /usr/include/time.h /usr/include/bits/time.h ../Core/core_api.h
sv_ainav.o: sv_aiutil.h sv_ainav.h sv_ai.h sv_aigoal.h
sv_aistate.o: server_game.h game.h ../toplayer/savage.h
sv_aistate.o: ../Core/savage_types.h ../masterserver/sharedcommon.h
sv_aistate.o: ../Core/savage_common.h /usr/include/stdlib.h
sv_aistate.o: /usr/include/features.h /usr/include/sys/cdefs.h
sv_aistate.o: /usr/include/gnu/stubs.h
sv_aistate.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
sv_aistate.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
sv_aistate.o: /usr/include/stdio.h /usr/include/bits/types.h
sv_aistate.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
sv_aistate.o: /usr/include/libio.h /usr/include/_G_config.h
sv_aistate.o: /usr/include/wchar.h /usr/include/bits/wchar.h
sv_aistate.o: /usr/include/gconv.h /usr/include/bits/stdio_lim.h
sv_aistate.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
sv_aistate.o: ../Core/savage_mathlib.h /usr/include/math.h
sv_aistate.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
sv_aistate.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
sv_aistate.o: /usr/include/time.h /usr/include/bits/time.h ../Core/core_api.h
sv_aistate.o: sv_aiutil.h sv_ai.h sv_ainav.h sv_aigoal.h sv_aistate.h
sv_aiutil.o: server_game.h game.h ../toplayer/savage.h ../Core/savage_types.h
sv_aiutil.o: ../masterserver/sharedcommon.h ../Core/savage_common.h
sv_aiutil.o: /usr/include/stdlib.h /usr/include/features.h
sv_aiutil.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
sv_aiutil.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
sv_aiutil.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
sv_aiutil.o: /usr/include/stdio.h /usr/include/bits/types.h
sv_aiutil.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
sv_aiutil.o: /usr/include/libio.h /usr/include/_G_config.h
sv_aiutil.o: /usr/include/wchar.h /usr/include/bits/wchar.h
sv_aiutil.o: /usr/include/gconv.h /usr/include/bits/stdio_lim.h
sv_aiutil.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
sv_aiutil.o: ../Core/savage_mathlib.h /usr/include/math.h
sv_aiutil.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
sv_aiutil.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
sv_aiutil.o: /usr/include/time.h /usr/include/bits/time.h ../Core/core_api.h
sv_aiutil.o: sv_aiutil.h
sv_ai_unit.o: server_game.h game.h ../toplayer/savage.h
sv_ai_unit.o: ../Core/savage_types.h ../masterserver/sharedcommon.h
sv_ai_unit.o: ../Core/savage_common.h /usr/include/stdlib.h
sv_ai_unit.o: /usr/include/features.h /usr/include/sys/cdefs.h
sv_ai_unit.o: /usr/include/gnu/stubs.h
sv_ai_unit.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
sv_ai_unit.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
sv_ai_unit.o: /usr/include/stdio.h /usr/include/bits/types.h
sv_ai_unit.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
sv_ai_unit.o: /usr/include/libio.h /usr/include/_G_config.h
sv_ai_unit.o: /usr/include/wchar.h /usr/include/bits/wchar.h
sv_ai_unit.o: /usr/include/gconv.h /usr/include/bits/stdio_lim.h
sv_ai_unit.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
sv_ai_unit.o: ../Core/savage_mathlib.h /usr/include/math.h
sv_ai_unit.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
sv_ai_unit.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
sv_ai_unit.o: /usr/include/time.h /usr/include/bits/time.h ../Core/core_api.h
sv_ai_unit.o: sv_aiutil.h sv_ai.h sv_ainav.h sv_aigoal.h
sv_buildings.o: server_game.h game.h ../toplayer/savage.h
sv_buildings.o: ../Core/savage_types.h ../masterserver/sharedcommon.h
sv_buildings.o: ../Core/savage_common.h /usr/include/stdlib.h
sv_buildings.o: /usr/include/features.h /usr/include/sys/cdefs.h
sv_buildings.o: /usr/include/gnu/stubs.h
sv_buildings.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
sv_buildings.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
sv_buildings.o: /usr/include/stdio.h /usr/include/bits/types.h
sv_buildings.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
sv_buildings.o: /usr/include/libio.h /usr/include/_G_config.h
sv_buildings.o: /usr/include/wchar.h /usr/include/bits/wchar.h
sv_buildings.o: /usr/include/gconv.h /usr/include/bits/stdio_lim.h
sv_buildings.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
sv_buildings.o: ../Core/savage_mathlib.h /usr/include/math.h
sv_buildings.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
sv_buildings.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
sv_buildings.o: /usr/include/time.h /usr/include/bits/time.h
sv_buildings.o: ../Core/core_api.h
sv_experience.o: server_game.h game.h ../toplayer/savage.h
sv_experience.o: ../Core/savage_types.h ../masterserver/sharedcommon.h
sv_experience.o: ../Core/savage_common.h /usr/include/stdlib.h
sv_experience.o: /usr/include/features.h /usr/include/sys/cdefs.h
sv_experience.o: /usr/include/gnu/stubs.h
sv_experience.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
sv_experience.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
sv_experience.o: /usr/include/stdio.h /usr/include/bits/types.h
sv_experience.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
sv_experience.o: /usr/include/libio.h /usr/include/_G_config.h
sv_experience.o: /usr/include/wchar.h /usr/include/bits/wchar.h
sv_experience.o: /usr/include/gconv.h /usr/include/bits/stdio_lim.h
sv_experience.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
sv_experience.o: ../Core/savage_mathlib.h /usr/include/math.h
sv_experience.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
sv_experience.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
sv_experience.o: /usr/include/time.h /usr/include/bits/time.h
sv_experience.o: ../Core/core_api.h
sv_gamescript.o: server_game.h game.h ../toplayer/savage.h
sv_gamescript.o: ../Core/savage_types.h ../masterserver/sharedcommon.h
sv_gamescript.o: ../Core/savage_common.h /usr/include/stdlib.h
sv_gamescript.o: /usr/include/features.h /usr/include/sys/cdefs.h
sv_gamescript.o: /usr/include/gnu/stubs.h
sv_gamescript.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
sv_gamescript.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
sv_gamescript.o: /usr/include/stdio.h /usr/include/bits/types.h
sv_gamescript.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
sv_gamescript.o: /usr/include/libio.h /usr/include/_G_config.h
sv_gamescript.o: /usr/include/wchar.h /usr/include/bits/wchar.h
sv_gamescript.o: /usr/include/gconv.h /usr/include/bits/stdio_lim.h
sv_gamescript.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
sv_gamescript.o: ../Core/savage_mathlib.h /usr/include/math.h
sv_gamescript.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
sv_gamescript.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
sv_gamescript.o: /usr/include/time.h /usr/include/bits/time.h
sv_gamescript.o: ../Core/core_api.h
sv_main.o: server_game.h game.h ../toplayer/savage.h ../Core/savage_types.h
sv_main.o: ../masterserver/sharedcommon.h ../Core/savage_common.h
sv_main.o: /usr/include/stdlib.h /usr/include/features.h
sv_main.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
sv_main.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
sv_main.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
sv_main.o: /usr/include/stdio.h /usr/include/bits/types.h
sv_main.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
sv_main.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
sv_main.o: /usr/include/bits/wchar.h /usr/include/gconv.h
sv_main.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
sv_main.o: /usr/include/string.h ../Core/savage_mathlib.h /usr/include/math.h
sv_main.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
sv_main.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
sv_main.o: /usr/include/time.h /usr/include/bits/time.h ../Core/core_api.h
sv_world.o: server_game.h game.h ../toplayer/savage.h ../Core/savage_types.h
sv_world.o: ../masterserver/sharedcommon.h ../Core/savage_common.h
sv_world.o: /usr/include/stdlib.h /usr/include/features.h
sv_world.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
sv_world.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
sv_world.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
sv_world.o: /usr/include/stdio.h /usr/include/bits/types.h
sv_world.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
sv_world.o: /usr/include/libio.h /usr/include/_G_config.h
sv_world.o: /usr/include/wchar.h /usr/include/bits/wchar.h
sv_world.o: /usr/include/gconv.h /usr/include/bits/stdio_lim.h
sv_world.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
sv_world.o: ../Core/savage_mathlib.h /usr/include/math.h
sv_world.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
sv_world.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
sv_world.o: /usr/include/time.h /usr/include/bits/time.h ../Core/core_api.h
sv_objects.o: server_game.h game.h ../toplayer/savage.h
sv_objects.o: ../Core/savage_types.h ../masterserver/sharedcommon.h
sv_objects.o: ../Core/savage_common.h /usr/include/stdlib.h
sv_objects.o: /usr/include/features.h /usr/include/sys/cdefs.h
sv_objects.o: /usr/include/gnu/stubs.h
sv_objects.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
sv_objects.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
sv_objects.o: /usr/include/stdio.h /usr/include/bits/types.h
sv_objects.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
sv_objects.o: /usr/include/libio.h /usr/include/_G_config.h
sv_objects.o: /usr/include/wchar.h /usr/include/bits/wchar.h
sv_objects.o: /usr/include/gconv.h /usr/include/bits/stdio_lim.h
sv_objects.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
sv_objects.o: ../Core/savage_mathlib.h /usr/include/math.h
sv_objects.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
sv_objects.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
sv_objects.o: /usr/include/time.h /usr/include/bits/time.h ../Core/core_api.h
sv_clients.o: server_game.h game.h ../toplayer/savage.h
sv_clients.o: ../Core/savage_types.h ../masterserver/sharedcommon.h
sv_clients.o: ../Core/savage_common.h /usr/include/stdlib.h
sv_clients.o: /usr/include/features.h /usr/include/sys/cdefs.h
sv_clients.o: /usr/include/gnu/stubs.h
sv_clients.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
sv_clients.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
sv_clients.o: /usr/include/stdio.h /usr/include/bits/types.h
sv_clients.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
sv_clients.o: /usr/include/libio.h /usr/include/_G_config.h
sv_clients.o: /usr/include/wchar.h /usr/include/bits/wchar.h
sv_clients.o: /usr/include/gconv.h /usr/include/bits/stdio_lim.h
sv_clients.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
sv_clients.o: ../Core/savage_mathlib.h /usr/include/math.h
sv_clients.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
sv_clients.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
sv_clients.o: /usr/include/time.h /usr/include/bits/time.h ../Core/core_api.h
sv_teams.o: server_game.h game.h ../toplayer/savage.h ../Core/savage_types.h
sv_teams.o: ../masterserver/sharedcommon.h ../Core/savage_common.h
sv_teams.o: /usr/include/stdlib.h /usr/include/features.h
sv_teams.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
sv_teams.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
sv_teams.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
sv_teams.o: /usr/include/stdio.h /usr/include/bits/types.h
sv_teams.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
sv_teams.o: /usr/include/libio.h /usr/include/_G_config.h
sv_teams.o: /usr/include/wchar.h /usr/include/bits/wchar.h
sv_teams.o: /usr/include/gconv.h /usr/include/bits/stdio_lim.h
sv_teams.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
sv_teams.o: ../Core/savage_mathlib.h /usr/include/math.h
sv_teams.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
sv_teams.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
sv_teams.o: /usr/include/time.h /usr/include/bits/time.h ../Core/core_api.h
sv_player.o: server_game.h game.h ../toplayer/savage.h ../Core/savage_types.h
sv_player.o: ../masterserver/sharedcommon.h ../Core/savage_common.h
sv_player.o: /usr/include/stdlib.h /usr/include/features.h
sv_player.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
sv_player.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
sv_player.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
sv_player.o: /usr/include/stdio.h /usr/include/bits/types.h
sv_player.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
sv_player.o: /usr/include/libio.h /usr/include/_G_config.h
sv_player.o: /usr/include/wchar.h /usr/include/bits/wchar.h
sv_player.o: /usr/include/gconv.h /usr/include/bits/stdio_lim.h
sv_player.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
sv_player.o: ../Core/savage_mathlib.h /usr/include/math.h
sv_player.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
sv_player.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
sv_player.o: /usr/include/time.h /usr/include/bits/time.h ../Core/core_api.h
sv_commandcenter.o: server_game.h game.h ../toplayer/savage.h
sv_commandcenter.o: ../Core/savage_types.h ../masterserver/sharedcommon.h
sv_commandcenter.o: ../Core/savage_common.h /usr/include/stdlib.h
sv_commandcenter.o: /usr/include/features.h /usr/include/sys/cdefs.h
sv_commandcenter.o: /usr/include/gnu/stubs.h
sv_commandcenter.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
sv_commandcenter.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
sv_commandcenter.o: /usr/include/stdio.h /usr/include/bits/types.h
sv_commandcenter.o: /usr/include/bits/wordsize.h
sv_commandcenter.o: /usr/include/bits/typesizes.h /usr/include/libio.h
sv_commandcenter.o: /usr/include/_G_config.h /usr/include/wchar.h
sv_commandcenter.o: /usr/include/bits/wchar.h /usr/include/gconv.h
sv_commandcenter.o: /usr/include/bits/stdio_lim.h
sv_commandcenter.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
sv_commandcenter.o: ../Core/savage_mathlib.h /usr/include/math.h
sv_commandcenter.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
sv_commandcenter.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
sv_commandcenter.o: /usr/include/time.h /usr/include/bits/time.h
sv_commandcenter.o: ../Core/core_api.h
sv_events.o: server_game.h game.h ../toplayer/savage.h ../Core/savage_types.h
sv_events.o: ../masterserver/sharedcommon.h ../Core/savage_common.h
sv_events.o: /usr/include/stdlib.h /usr/include/features.h
sv_events.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
sv_events.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
sv_events.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
sv_events.o: /usr/include/stdio.h /usr/include/bits/types.h
sv_events.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
sv_events.o: /usr/include/libio.h /usr/include/_G_config.h
sv_events.o: /usr/include/wchar.h /usr/include/bits/wchar.h
sv_events.o: /usr/include/gconv.h /usr/include/bits/stdio_lim.h
sv_events.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
sv_events.o: ../Core/savage_mathlib.h /usr/include/math.h
sv_events.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
sv_events.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
sv_events.o: /usr/include/time.h /usr/include/bits/time.h ../Core/core_api.h
sv_weapons.o: server_game.h game.h ../toplayer/savage.h
sv_weapons.o: ../Core/savage_types.h ../masterserver/sharedcommon.h
sv_weapons.o: ../Core/savage_common.h /usr/include/stdlib.h
sv_weapons.o: /usr/include/features.h /usr/include/sys/cdefs.h
sv_weapons.o: /usr/include/gnu/stubs.h
sv_weapons.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
sv_weapons.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
sv_weapons.o: /usr/include/stdio.h /usr/include/bits/types.h
sv_weapons.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
sv_weapons.o: /usr/include/libio.h /usr/include/_G_config.h
sv_weapons.o: /usr/include/wchar.h /usr/include/bits/wchar.h
sv_weapons.o: /usr/include/gconv.h /usr/include/bits/stdio_lim.h
sv_weapons.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
sv_weapons.o: ../Core/savage_mathlib.h /usr/include/math.h
sv_weapons.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
sv_weapons.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
sv_weapons.o: /usr/include/time.h /usr/include/bits/time.h ../Core/core_api.h
sv_weapons.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h
sv_items.o: server_game.h game.h ../toplayer/savage.h ../Core/savage_types.h
sv_items.o: ../masterserver/sharedcommon.h ../Core/savage_common.h
sv_items.o: /usr/include/stdlib.h /usr/include/features.h
sv_items.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
sv_items.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
sv_items.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
sv_items.o: /usr/include/stdio.h /usr/include/bits/types.h
sv_items.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
sv_items.o: /usr/include/libio.h /usr/include/_G_config.h
sv_items.o: /usr/include/wchar.h /usr/include/bits/wchar.h
sv_items.o: /usr/include/gconv.h /usr/include/bits/stdio_lim.h
sv_items.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
sv_items.o: ../Core/savage_mathlib.h /usr/include/math.h
sv_items.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
sv_items.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
sv_items.o: /usr/include/time.h /usr/include/bits/time.h ../Core/core_api.h
sv_phys_object.o: server_game.h game.h ../toplayer/savage.h
sv_phys_object.o: ../Core/savage_types.h ../masterserver/sharedcommon.h
sv_phys_object.o: ../Core/savage_common.h /usr/include/stdlib.h
sv_phys_object.o: /usr/include/features.h /usr/include/sys/cdefs.h
sv_phys_object.o: /usr/include/gnu/stubs.h
sv_phys_object.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
sv_phys_object.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
sv_phys_object.o: /usr/include/stdio.h /usr/include/bits/types.h
sv_phys_object.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
sv_phys_object.o: /usr/include/libio.h /usr/include/_G_config.h
sv_phys_object.o: /usr/include/wchar.h /usr/include/bits/wchar.h
sv_phys_object.o: /usr/include/gconv.h /usr/include/bits/stdio_lim.h
sv_phys_object.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
sv_phys_object.o: ../Core/savage_mathlib.h /usr/include/math.h
sv_phys_object.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
sv_phys_object.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
sv_phys_object.o: /usr/include/time.h /usr/include/bits/time.h
sv_phys_object.o: ../Core/core_api.h
sv_vote.o: server_game.h game.h ../toplayer/savage.h ../Core/savage_types.h
sv_vote.o: ../masterserver/sharedcommon.h ../Core/savage_common.h
sv_vote.o: /usr/include/stdlib.h /usr/include/features.h
sv_vote.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
sv_vote.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
sv_vote.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
sv_vote.o: /usr/include/stdio.h /usr/include/bits/types.h
sv_vote.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
sv_vote.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
sv_vote.o: /usr/include/bits/wchar.h /usr/include/gconv.h
sv_vote.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
sv_vote.o: /usr/include/string.h ../Core/savage_mathlib.h /usr/include/math.h
sv_vote.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
sv_vote.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
sv_vote.o: /usr/include/time.h /usr/include/bits/time.h ../Core/core_api.h
sv_vclient.o: server_game.h game.h ../toplayer/savage.h
sv_vclient.o: ../Core/savage_types.h ../masterserver/sharedcommon.h
sv_vclient.o: ../Core/savage_common.h /usr/include/stdlib.h
sv_vclient.o: /usr/include/features.h /usr/include/sys/cdefs.h
sv_vclient.o: /usr/include/gnu/stubs.h
sv_vclient.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
sv_vclient.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
sv_vclient.o: /usr/include/stdio.h /usr/include/bits/types.h
sv_vclient.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
sv_vclient.o: /usr/include/libio.h /usr/include/_G_config.h
sv_vclient.o: /usr/include/wchar.h /usr/include/bits/wchar.h
sv_vclient.o: /usr/include/gconv.h /usr/include/bits/stdio_lim.h
sv_vclient.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
sv_vclient.o: ../Core/savage_mathlib.h /usr/include/math.h
sv_vclient.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
sv_vclient.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
sv_vclient.o: /usr/include/time.h /usr/include/bits/time.h ../Core/core_api.h
phys_player_combat.o: game.h ../toplayer/savage.h ../Core/savage_types.h
phys_player_combat.o: ../masterserver/sharedcommon.h ../Core/savage_common.h
phys_player_combat.o: /usr/include/stdlib.h /usr/include/features.h
phys_player_combat.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
phys_player_combat.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
phys_player_combat.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
phys_player_combat.o: /usr/include/stdio.h /usr/include/bits/types.h
phys_player_combat.o: /usr/include/bits/wordsize.h
phys_player_combat.o: /usr/include/bits/typesizes.h /usr/include/libio.h
phys_player_combat.o: /usr/include/_G_config.h /usr/include/wchar.h
phys_player_combat.o: /usr/include/bits/wchar.h /usr/include/gconv.h
phys_player_combat.o: /usr/include/bits/stdio_lim.h
phys_player_combat.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
phys_player_combat.o: ../Core/savage_mathlib.h /usr/include/math.h
phys_player_combat.o: /usr/include/bits/huge_val.h
phys_player_combat.o: /usr/include/bits/mathdef.h
phys_player_combat.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
phys_player_combat.o: /usr/include/time.h /usr/include/bits/time.h
phys_player_combat.o: ../Core/core_api.h
phys_player.o: game.h ../toplayer/savage.h ../Core/savage_types.h
phys_player.o: ../masterserver/sharedcommon.h ../Core/savage_common.h
phys_player.o: /usr/include/stdlib.h /usr/include/features.h
phys_player.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
phys_player.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
phys_player.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
phys_player.o: /usr/include/stdio.h /usr/include/bits/types.h
phys_player.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
phys_player.o: /usr/include/libio.h /usr/include/_G_config.h
phys_player.o: /usr/include/wchar.h /usr/include/bits/wchar.h
phys_player.o: /usr/include/gconv.h /usr/include/bits/stdio_lim.h
phys_player.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
phys_player.o: ../Core/savage_mathlib.h /usr/include/math.h
phys_player.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
phys_player.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
phys_player.o: /usr/include/time.h /usr/include/bits/time.h
phys_player.o: ../Core/core_api.h
phys_shared.o: game.h ../toplayer/savage.h ../Core/savage_types.h
phys_shared.o: ../masterserver/sharedcommon.h ../Core/savage_common.h
phys_shared.o: /usr/include/stdlib.h /usr/include/features.h
phys_shared.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
phys_shared.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
phys_shared.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
phys_shared.o: /usr/include/stdio.h /usr/include/bits/types.h
phys_shared.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
phys_shared.o: /usr/include/libio.h /usr/include/_G_config.h
phys_shared.o: /usr/include/wchar.h /usr/include/bits/wchar.h
phys_shared.o: /usr/include/gconv.h /usr/include/bits/stdio_lim.h
phys_shared.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
phys_shared.o: ../Core/savage_mathlib.h /usr/include/math.h
phys_shared.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
phys_shared.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
phys_shared.o: /usr/include/time.h /usr/include/bits/time.h
phys_shared.o: ../Core/core_api.h
states.o: game.h ../toplayer/savage.h ../Core/savage_types.h
states.o: ../masterserver/sharedcommon.h ../Core/savage_common.h
states.o: /usr/include/stdlib.h /usr/include/features.h
states.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
states.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
states.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
states.o: /usr/include/stdio.h /usr/include/bits/types.h
states.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
states.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
states.o: /usr/include/bits/wchar.h /usr/include/gconv.h
states.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
states.o: /usr/include/string.h ../Core/savage_mathlib.h /usr/include/math.h
states.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
states.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
states.o: /usr/include/time.h /usr/include/bits/time.h ../Core/core_api.h
techtree.o: game.h ../toplayer/savage.h ../Core/savage_types.h
techtree.o: ../masterserver/sharedcommon.h ../Core/savage_common.h
techtree.o: /usr/include/stdlib.h /usr/include/features.h
techtree.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
techtree.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
techtree.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
techtree.o: /usr/include/stdio.h /usr/include/bits/types.h
techtree.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
techtree.o: /usr/include/libio.h /usr/include/_G_config.h
techtree.o: /usr/include/wchar.h /usr/include/bits/wchar.h
techtree.o: /usr/include/gconv.h /usr/include/bits/stdio_lim.h
techtree.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
techtree.o: ../Core/savage_mathlib.h /usr/include/math.h
techtree.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
techtree.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
techtree.o: /usr/include/time.h /usr/include/bits/time.h ../Core/core_api.h
weapons.o: game.h ../toplayer/savage.h ../Core/savage_types.h
weapons.o: ../masterserver/sharedcommon.h ../Core/savage_common.h
weapons.o: /usr/include/stdlib.h /usr/include/features.h
weapons.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
weapons.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
weapons.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
weapons.o: /usr/include/stdio.h /usr/include/bits/types.h
weapons.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
weapons.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
weapons.o: /usr/include/bits/wchar.h /usr/include/gconv.h
weapons.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
weapons.o: /usr/include/string.h ../Core/savage_mathlib.h /usr/include/math.h
weapons.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
weapons.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
weapons.o: /usr/include/time.h /usr/include/bits/time.h ../Core/core_api.h
units.o: game.h ../toplayer/savage.h ../Core/savage_types.h
units.o: ../masterserver/sharedcommon.h ../Core/savage_common.h
units.o: /usr/include/stdlib.h /usr/include/features.h
units.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
units.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
units.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
units.o: /usr/include/stdio.h /usr/include/bits/types.h
units.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
units.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
units.o: /usr/include/bits/wchar.h /usr/include/gconv.h
units.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
units.o: /usr/include/string.h ../Core/savage_mathlib.h /usr/include/math.h
units.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
units.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
units.o: /usr/include/time.h /usr/include/bits/time.h ../Core/core_api.h
dllentry.o: game.h ../toplayer/savage.h ../Core/savage_types.h
dllentry.o: ../masterserver/sharedcommon.h ../Core/savage_common.h
dllentry.o: /usr/include/stdlib.h /usr/include/features.h
dllentry.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
dllentry.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
dllentry.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
dllentry.o: /usr/include/stdio.h /usr/include/bits/types.h
dllentry.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
dllentry.o: /usr/include/libio.h /usr/include/_G_config.h
dllentry.o: /usr/include/wchar.h /usr/include/bits/wchar.h
dllentry.o: /usr/include/gconv.h /usr/include/bits/stdio_lim.h
dllentry.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
dllentry.o: ../Core/savage_mathlib.h /usr/include/math.h
dllentry.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
dllentry.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
dllentry.o: /usr/include/time.h /usr/include/bits/time.h ../Core/core_api.h
dynstrings.o: game.h ../toplayer/savage.h ../Core/savage_types.h
dynstrings.o: ../masterserver/sharedcommon.h ../Core/savage_common.h
dynstrings.o: /usr/include/stdlib.h /usr/include/features.h
dynstrings.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
dynstrings.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
dynstrings.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
dynstrings.o: /usr/include/stdio.h /usr/include/bits/types.h
dynstrings.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
dynstrings.o: /usr/include/libio.h /usr/include/_G_config.h
dynstrings.o: /usr/include/wchar.h /usr/include/bits/wchar.h
dynstrings.o: /usr/include/gconv.h /usr/include/bits/stdio_lim.h
dynstrings.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
dynstrings.o: ../Core/savage_mathlib.h /usr/include/math.h
dynstrings.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
dynstrings.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
dynstrings.o: /usr/include/time.h /usr/include/bits/time.h ../Core/core_api.h
metaconfig.o: game.h ../toplayer/savage.h ../Core/savage_types.h
metaconfig.o: ../masterserver/sharedcommon.h ../Core/savage_common.h
metaconfig.o: /usr/include/stdlib.h /usr/include/features.h
metaconfig.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
metaconfig.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
metaconfig.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
metaconfig.o: /usr/include/stdio.h /usr/include/bits/types.h
metaconfig.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
metaconfig.o: /usr/include/libio.h /usr/include/_G_config.h
metaconfig.o: /usr/include/wchar.h /usr/include/bits/wchar.h
metaconfig.o: /usr/include/gconv.h /usr/include/bits/stdio_lim.h
metaconfig.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
metaconfig.o: ../Core/savage_mathlib.h /usr/include/math.h
metaconfig.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
metaconfig.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
metaconfig.o: /usr/include/time.h /usr/include/bits/time.h ../Core/core_api.h
trajectory.o: game.h ../toplayer/savage.h ../Core/savage_types.h
trajectory.o: ../masterserver/sharedcommon.h ../Core/savage_common.h
trajectory.o: /usr/include/stdlib.h /usr/include/features.h
trajectory.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
trajectory.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
trajectory.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
trajectory.o: /usr/include/stdio.h /usr/include/bits/types.h
trajectory.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
trajectory.o: /usr/include/libio.h /usr/include/_G_config.h
trajectory.o: /usr/include/wchar.h /usr/include/bits/wchar.h
trajectory.o: /usr/include/gconv.h /usr/include/bits/stdio_lim.h
trajectory.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
trajectory.o: ../Core/savage_mathlib.h /usr/include/math.h
trajectory.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
trajectory.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
trajectory.o: /usr/include/time.h /usr/include/bits/time.h ../Core/core_api.h
interface.o: interface.h ../toplayer/savage.h ../Core/savage_types.h
interface.o: ../masterserver/sharedcommon.h ../Core/savage_common.h
interface.o: /usr/include/stdlib.h /usr/include/features.h
interface.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
interface.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
interface.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
interface.o: /usr/include/stdio.h /usr/include/bits/types.h
interface.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
interface.o: /usr/include/libio.h /usr/include/_G_config.h
interface.o: /usr/include/wchar.h /usr/include/bits/wchar.h
interface.o: /usr/include/gconv.h /usr/include/bits/stdio_lim.h
interface.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
interface.o: ../Core/savage_mathlib.h /usr/include/math.h
interface.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
interface.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
interface.o: /usr/include/time.h /usr/include/bits/time.h ../Core/core_api.h
interface.o: ../gui/gui_scrollbuffer.h ../gui/gui_orderedscrollbuffer.h
interface.o: ../gui/gui_textbuffer.h ../gui/gui_button.h ../gui/gui_graphic.h
interface.o: ../gui/gui_label.h
weather.o: game.h ../toplayer/savage.h ../Core/savage_types.h
weather.o: ../masterserver/sharedcommon.h ../Core/savage_common.h
weather.o: /usr/include/stdlib.h /usr/include/features.h
weather.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
weather.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h
weather.o: /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h
weather.o: /usr/include/stdio.h /usr/include/bits/types.h
weather.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
weather.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
weather.o: /usr/include/bits/wchar.h /usr/include/gconv.h
weather.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
weather.o: /usr/include/string.h ../Core/savage_mathlib.h /usr/include/math.h
weather.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
weather.o: /usr/include/bits/mathcalls.h ../Core/camerautils.h
weather.o: /usr/include/time.h /usr/include/bits/time.h ../Core/core_api.h
