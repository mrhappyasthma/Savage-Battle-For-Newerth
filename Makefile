SHELL=/bin/sh

SRC=\
./Core/allocator.c \
./Core/bans.c \
./Core/bink_unix.c \
./Core/bitmap.c \
./Core/bsp.c \
./Core/buddies.c \
./Core/camerautils.c \
./Core/client_le.c \
./Core/cmd.c \
./Core/colorutils.c \
./Core/console.c \
./Core/cookie.c \
./Core/cvar.c \
./Core/cvar_container.c \
./Core/drawutils.c \
./Core/eval.c \
./Core/file.c \
./Core/font.c \
./Core/geom.c \
./Core/gl_console.c \
./Core/gl_main.c \
./Core/gl_model.c \
./Core/gl_scene.c \
./Core/gl_scene_builder.c \
./Core/gl_sdl.c \
./Core/gl_terrain.c \
./Core/gui.c \
./Core/gui_drawutils.c \
./Core/gui_panel.c \
./Core/hash.c \
./Core/heap.c \
./Core/host.c \
./Core/host_getcoreapi.c \
./Core/http.c \
./Core/input.c \
./Core/input_sdl.c \
./Core/intersection.c \
./Core/jpeg.c \
./Core/keyclient.c \
./Core/main_sdl.c \
./Core/main_linux.c \
./Core/mem.c \
./Core/misc_cmds.c \
./Core/navmesh.c \
./Core/navpoly.c \
./Core/navrep.c \
./Core/net.c \
./Core/net_bsdsock.c \
./Core/net_deltastructs.c \
./Core/net_irc.c \
./Core/net_server.c \
./Core/net_tcp.c \
./Core/net_unix.c \
./Core/packet.c \
./Core/parsestats.c \
./Core/quadtree.c \
./Core/res.c \
./Core/savage_common.c \
./Core/savage_mathlib.c \
./Core/scene.c \
./Core/server_le.c \
./Core/serverlist.c \
./Core/set.c \
./Core/sound_fmod.c \
./Core/stringtable.c \
./Core/theora_win32.c \
./Core/timeofday.c \
./Core/unzip.c \
./Core/vid.c \
./Core/vid_sdl.c \
./Core/world.c \
./Core/world_lights.c \
./Core/world_objectgrid.c \
./Core/world_objects.c \
./Core/world_tree2.c \
./Core/zip.c \
./keyserver/ssl-utils.c

OBJS=$(SRC:.c=.o)

NAME=./silverback.bin

INCLUDE=\
-I keyserver \
`pkg-config --cflags glib-2.0 libpng` \
`sdl-config --cflags` \
`freetype-config --cflags`

CFLAGS=\
-O3 -march=i686 -mfpmath=sse -mmmx -msse -m3dnow -ffast-math -mno-ieee-fp -funroll-loops -fomit-frame-pointer -Wall

LFLAGS=\
`pkg-config --libs glib-2.0 libpng` \
-Wl,-allow-multiple-definition \
-lm \
`sdl-config --libs` \
-lz \
-lGL -lGLU \
-ljpeg \
`freetype-config --libs` \
`curl-config --libs` \
-lfmod

default: debug

debug: $(OBJS) $(NAME)
debug: override CFLAGS = -g3 -Wall

silverback: $(OBJS) $(NAME)

$(NAME): $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o $@

clean:
	@rm -f ./Core/*.o ./keyserver/*.o silverback.bin

.c.o:
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $*.o
