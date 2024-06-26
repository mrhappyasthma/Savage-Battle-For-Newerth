.KEEP_STATE:
.SUFFIXESS: .C .c .o .ln .lint .src
#.SUFFIXES: .C $(SUFFIXES)

CC=gcc

OUTPUT_OPTION = -o $*.o

OUR_CFLAGS=#-fPIC -D_REENTRANT 
CFLAGS=$(CMDLINE_CFLAGS) $(OUR_CFLAGS) -Wall -Winline

OUR_SHARED_LIBS=-lc $(SHARED_LIBS)
		
.c.o:
	$(CC) $(INCLUDE) $(CFLAGS) -c $< -o $*.o

LIBS=../../libs/

INCLUDE=\
        -I$(LIBS)nvidia/include \
        -I$(LIBS)nvidia/include/glh \
        -I$(LIBS)opengl \
        -I../toplayer \
        -I../Core \
        -I. 

OBJS= \
    gui_classes.c \
    le_camera.c \
    le_draw.c \
    le_drawutils.c \
    le_main.c \
    le_mem.c \
    le_menus.c \
    le_modelmode.c \
    le_objects.c \
    le_pool.c \
    le_tools.c \
	../Core/savage_mathlib.c \
	../Core/savage_common.c \
	../Core/camerautils.c \
    ../toplayer/tl_camera.c \
    ../toplayer/tl_drawutils.c \
    ../toplayer/tl_pool.c \
    ../toplayer/tl_sky.c \
	../gui/gui_brushbutton.c \
	../gui/gui_brushgrid.c \
	../gui/gui_button.c \
	../gui/gui_floater.c \
	../gui/gui_graphic.c \
	../gui/gui_label.c \
	../gui/gui_menu.c \
	../gui/gui_slider.c \
	../gui/gui_spinner.c \
	../gui/gui_swatch.c \
	../gui/gui_textbox.c \
	../gui/gui_thumbnailgrid.c \
	../gui/gui_togglebutton.c \
	../gui/gui_track.c 
#	../gui/gui_chatbox.c \
#    le_brushfuncs.c 
#	../gui/gui_brushpanel.c 
#	../gui/gui_textblock.c \

all: debug #clean-core-files

#clean-core-files:
#	rm -f ../Core/savage_mathlib.o ../Core/savage_common.o ../Core/camerautils.o

debug:
	CMDLINE_CFLAGS=-g make ../../Clean_Build/editor/game.so

debug-efence:
	SHARED_LIBS="-g -lefence" make ../../Clean_Build/editor/game.so

debug-dmalloc:
	SHARED_LIBS="-g -ldmalloc" make ../../Clean_Build/editor/game.so
#	SHARED_LIBS="-g /usr/local/lib/libdmallocth.a" make game.so

release:
	CMDLINE_CFLAGS="-O -finline-functions -ffast-math -mno-ieee-fp -fstrength-reduce -funroll-loops" make ../../build/editor/game.so

release-profile:
	CMDLINE_CFLAGS="-O2 -finline-functions -ffast-math -mno-ieee-fp -fstrength-reduce -funroll-loops -pg" make ../../build/editor/game.so

../../Clean_Build/editor/game.so: $(OBJS:.c=.o)
	$(CC) $(CFLAGS) $(OUR_CFLAGS) -shared -Wl,-soname,game.so.1 \
	    -o ../../Clean_Build/editor/game.so $(OBJS:.c=.o) $(OUR_SHARED_LIBS)

clean:
	rm *.o ../gui/*.o ../toplayer/*.o
# DO NOT DELETE

le_camera.o: ../le/le.h ../toplayer/tl_shared.h ../toplayer/savage.h
le_camera.o: /usr/include/stdlib.h /usr/include/features.h
le_camera.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
le_camera.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/stddef.h
le_camera.o: /usr/include/stdio.h /usr/include/bits/types.h
le_camera.o: /usr/include/libio.h /usr/include/_G_config.h
le_camera.o: /usr/include/wchar.h /usr/include/bits/wchar.h
le_camera.o: /usr/include/gconv.h
le_camera.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/stdarg.h
le_camera.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
le_camera.o: /usr/include/string.h le_drawutils.h le_draw.h le_camera.h
le_camera.o: le_tools.h le_main.h le_ui.h le_modelmode.h le_texturemode.h
le_camera.o: le_objects.h le_pool.h
le_draw.o: ../le/le.h ../toplayer/tl_shared.h ../toplayer/savage.h
le_draw.o: /usr/include/stdlib.h /usr/include/features.h
le_draw.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
le_draw.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/stddef.h
le_draw.o: /usr/include/stdio.h /usr/include/bits/types.h
le_draw.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
le_draw.o: /usr/include/bits/wchar.h /usr/include/gconv.h
le_draw.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/stdarg.h
le_draw.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
le_draw.o: /usr/include/string.h le_drawutils.h le_draw.h le_camera.h
le_draw.o: le_tools.h le_main.h le_ui.h le_modelmode.h le_texturemode.h
le_draw.o: le_objects.h le_pool.h
le_drawutils.o: ../le/le.h ../toplayer/tl_shared.h ../toplayer/savage.h
le_drawutils.o: /usr/include/stdlib.h /usr/include/features.h
le_drawutils.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
le_drawutils.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/stddef.h
le_drawutils.o: /usr/include/stdio.h /usr/include/bits/types.h
le_drawutils.o: /usr/include/libio.h /usr/include/_G_config.h
le_drawutils.o: /usr/include/wchar.h /usr/include/bits/wchar.h
le_drawutils.o: /usr/include/gconv.h
le_drawutils.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/stdarg.h
le_drawutils.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
le_drawutils.o: /usr/include/string.h le_drawutils.h le_draw.h le_camera.h
le_drawutils.o: le_tools.h le_main.h le_ui.h le_modelmode.h le_texturemode.h
le_drawutils.o: le_objects.h le_pool.h
le_main.o: ../le/le.h ../toplayer/tl_shared.h ../toplayer/savage.h
le_main.o: /usr/include/stdlib.h /usr/include/features.h
le_main.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
le_main.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/stddef.h
le_main.o: /usr/include/stdio.h /usr/include/bits/types.h
le_main.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
le_main.o: /usr/include/bits/wchar.h /usr/include/gconv.h
le_main.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/stdarg.h
le_main.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
le_main.o: /usr/include/string.h le_drawutils.h le_draw.h le_camera.h
le_main.o: le_tools.h le_main.h le_ui.h le_modelmode.h le_texturemode.h
le_main.o: le_objects.h le_pool.h
le_menus.o: ../le/le.h ../toplayer/tl_shared.h ../toplayer/savage.h
le_menus.o: /usr/include/stdlib.h /usr/include/features.h
le_menus.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
le_menus.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/stddef.h
le_menus.o: /usr/include/stdio.h /usr/include/bits/types.h
le_menus.o: /usr/include/libio.h /usr/include/_G_config.h
le_menus.o: /usr/include/wchar.h /usr/include/bits/wchar.h
le_menus.o: /usr/include/gconv.h
le_menus.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/stdarg.h
le_menus.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
le_menus.o: /usr/include/string.h le_drawutils.h le_draw.h le_camera.h
le_menus.o: le_tools.h le_main.h le_ui.h le_modelmode.h le_texturemode.h
le_menus.o: le_objects.h le_pool.h
le_modelmode.o: ../le/le.h ../toplayer/tl_shared.h ../toplayer/savage.h
le_modelmode.o: /usr/include/stdlib.h /usr/include/features.h
le_modelmode.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
le_modelmode.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/stddef.h
le_modelmode.o: /usr/include/stdio.h /usr/include/bits/types.h
le_modelmode.o: /usr/include/libio.h /usr/include/_G_config.h
le_modelmode.o: /usr/include/wchar.h /usr/include/bits/wchar.h
le_modelmode.o: /usr/include/gconv.h
le_modelmode.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/stdarg.h
le_modelmode.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
le_modelmode.o: /usr/include/string.h le_drawutils.h le_draw.h le_camera.h
le_modelmode.o: le_tools.h le_main.h le_ui.h le_modelmode.h le_texturemode.h
le_modelmode.o: le_objects.h le_pool.h
le_objects.o: ../le/le.h ../toplayer/tl_shared.h ../toplayer/savage.h
le_objects.o: /usr/include/stdlib.h /usr/include/features.h
le_objects.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
le_objects.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/stddef.h
le_objects.o: /usr/include/stdio.h /usr/include/bits/types.h
le_objects.o: /usr/include/libio.h /usr/include/_G_config.h
le_objects.o: /usr/include/wchar.h /usr/include/bits/wchar.h
le_objects.o: /usr/include/gconv.h
le_objects.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/stdarg.h
le_objects.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
le_objects.o: /usr/include/string.h le_drawutils.h le_draw.h le_camera.h
le_objects.o: le_tools.h le_main.h le_ui.h le_modelmode.h le_texturemode.h
le_objects.o: le_objects.h le_pool.h ../gui/gui_menu.h
le_pool.o: ../le/le.h ../toplayer/tl_shared.h ../toplayer/savage.h
le_pool.o: /usr/include/stdlib.h /usr/include/features.h
le_pool.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
le_pool.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/stddef.h
le_pool.o: /usr/include/stdio.h /usr/include/bits/types.h
le_pool.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
le_pool.o: /usr/include/bits/wchar.h /usr/include/gconv.h
le_pool.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/stdarg.h
le_pool.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
le_pool.o: /usr/include/string.h le_drawutils.h le_draw.h le_camera.h
le_pool.o: le_tools.h le_main.h le_ui.h le_modelmode.h le_texturemode.h
le_pool.o: le_objects.h le_pool.h
le_tools.o: ../le/le.h ../toplayer/tl_shared.h ../toplayer/savage.h
le_tools.o: /usr/include/stdlib.h /usr/include/features.h
le_tools.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
le_tools.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/stddef.h
le_tools.o: /usr/include/stdio.h /usr/include/bits/types.h
le_tools.o: /usr/include/libio.h /usr/include/_G_config.h
le_tools.o: /usr/include/wchar.h /usr/include/bits/wchar.h
le_tools.o: /usr/include/gconv.h
le_tools.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/stdarg.h
le_tools.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
le_tools.o: /usr/include/string.h le_drawutils.h le_draw.h le_camera.h
le_tools.o: le_tools.h le_main.h le_ui.h le_modelmode.h le_texturemode.h
le_tools.o: le_objects.h le_pool.h
