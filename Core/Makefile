.KEEP_STATE:
.SUFFIXESS: .C .c .o .ln .lint .src
#.SUFFIXES: .C $(SUFFIXES)

MAKE=colormake
CC=gcc-3.3
DEPEND=gccmakedep

OUTPUT_OPTION = -o $*.o

.c.o:
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $*.o

OURLIBS_PATH=../../libs/
CFLAGS=$(CMDLINE_CFLAGS) -Wall -Winline -Wpointer-arith #-Wunreachable-code #-D_REENTRANT
THEORA_LIBS=#-logg -lvorbis ../../libs/theora/lib/libtheora.a
SERVER_LIBS=-lz

#OUR_SHARED_LIBS=-lpng -lssl -lSDL -lGL -lGLU -lXext -lcurl -logg -lvorbis ../../libs/theora/lib/libtheora.a $(SHARED_LIBS)
OUR_SHARED_LIBS=-lssl -lcurl $(SHARED_LIBS)
OPENGL_LIBS=-lGL -lGLU -lXext
CLIENT_LIBS=-lpng -ljpeg \
	`freetype-config --libs` 
#OUR_SHARED_LIBS=-lpng -lssl -ldb -lSDL -lGL -lGLU -lXext $(SHARED_LIBS)

#LIBS=-L$(OURLIBS_PATH)openal-dev/openal/linux/src \
#
LIBS=-L$(OURLIBS_PATH)fmod/api/ \
     -L$(OURLIBS_PATH)theora/lib/ \
     -L/usr/lib/debug/ \
     -L/usr/X11R6/lib/ \
	 `sdl-config --libs` \
	 `pkg-config --libs glib-2.0`
     #-L$(OURLIBS_PATH)libpng/zlib/ \
     #-L$(OURLIBS_PATH)libpng/lpng109/ 

INCLUDE= \
        -I. \
        -I$(OURLIBS_PATH)fmod/api/inc \
        -I$(OURLIBS_PATH)theora/include \
        -I$(OURLIBS_PATH)nvidia/include \
        -I$(OURLIBS_PATH)nvidia/include/glh \
        -I$(OURLIBS_PATH)opengl \
        -I/usr/include/speech_tools \
        -I/usr/include/festival \
        -I../toplayer \
        -I../keyserver \
	    `sdl-config --cflags` \
	    `freetype-config --cflags` \
		`pkg-config --cflags glib-2.0`
        #-I$(OURLIBS_PATH)libpng/zlib \
        #-I$(OURLIBS_PATH)openal-dev/openal/linux/include \
        #-I$(OURLIBS_PATH)openal-dev/openal/include/AL \
        #-I$(OURLIBS_PATH)openal-dev/openal/include 
		#$(OURLIBS_PATH)libpng/lpng109 

CLIENTOBJS=\
     gl_terrain.c \
     gl_model.c \
     gl_scene.c \
     gl_sdl.c \
     gl_scene_builder.c 

OBJS= \
     allocator.c \
	 bans.c \
	 bink_unix.c \
     bitmap.c \
	 bsp.c \
     camerautils.c \
     client_le.c \
     cmd.c \
     colorutils.c \
	 cookie.c \
     console.c \
     cvar.c \
     cvar_container.c \
     drawutils.c \
     eval.c \
     file.c \
     geom.c \
     gl_console.c \
     gl_main.c \
	 buddies.c \
     font.c \
     gui.c \
     gui_drawutils.c \
     gui_panel.c \
	 hash.c \
	 heap.c \
     host.c \
     host_getcoreapi.c \
	 http.c \
     input.c \
     input_sdl.c \
     intersection.c \
	 jpeg.c \
     main_sdl.c \
     main_linux.c \
     mem.c \
     misc_cmds.c \
	 net_deltastructs.c \
     net.c \
     net_bsdsock.c \
     net_unix.c \
     net_server.c \
     net_tcp.c \
     net_irc.c \
	 navpoly.c \
	 navrep.c \
	 navmesh.c \
     packet.c \
	 parsestats.c \
	 quadtree.c \
     res.c \
     savage_common.c \
     savage_mathlib.c \
	 serverlist.c \
	 set.c \
     scene.c \
     server_le.c \
	 sound_fmod.c \
	 stringtable.c \
	 theora_win32.c \
	 timeofday.c \
	 unzip.c \
     vid.c \
     vid_sdl.c \
     world.c \
     world_objects.c \
     world_objectgrid.c \
     world_lights.c \
     world_tree2.c \
	 keyclient.c \
	 zip.c \
	 ../keyserver/ssl-utils.c
#	gl_model.c \
#     client_main.c \
#     server_main.c 
#     openal.c \
#     wav.c \
#     vorbis.c \
#     sound.c \
#     modelstats.c \
#     sound_input.c \
#     sound_output.c 

TESTOBJS=\
	test.c \
	cookie.c \
	net_server.c

all: debug ../../Clean_Build/update ../../Clean_Build/savage #clean-core-files

clean-core-files:
	rm -f savage_mathlib.o savage_common.o camerautils.o

pot:
	xgettext --keyword=fmt --keyword=_ --keyword=Console_Printf --keyword=Console_DPrintf --keyword=Console_Errorf --keyword=corec.Console_Printf --keyword=corec.Console_DPrintf --keyword=cores.Console_Printf --keyword=cores.Console_DPrintf --keyword=core.Console_Printf --keyword=core.Console_DPrintf --keyword=cores.Console_Errorf --keyword=corec.Console_Errorf --keyword=core.Console_Errorf -o strings.pot *.[ch]

../../Clean_Build/update: updater.c
	$(CC) $(CFLAGS) -o ../../Clean_Build/update updater.c

../../Clean_Build/savage.bin: savage_shell.c
	$(CC) $(CFLAGS) -g -o ../../Clean_Build/savage.bin savage_shell.c

binary: ../../Clean_Build/silverback.bin

dedicated_server: 
	CMDLINE_CFLAGS="$(CMDLINE_CFLAGS) -DDEDICATED_SERVER -D_S2_DONT_INCLUDE_GL -D_S2_EXCLUDE_PNG -D_S2_EXCLUDE_FREETYPE" $(MAKE) ../../Clean_Build/dedicated_server.bin

demo-server: 
	CMDLINE_CFLAGS="$(CMDLINE_CFLAGS) -DDEDICATED_SERVER -DSAVAGE_DEMO -D_S2_DONT_INCLUDE_GL -D_S2_EXCLUDE_PNG -D_S2_EXCLUDE_FREETYPE" $(MAKE) ../../Clean_Build/savagedemo_server.bin

binary-static: ../../Clean_Build/silverback-static.bin

../../Clean_Build/dedicated_server.bin: $(OBJS:.c=.o)
	sync
	#$(CC) $(CFLAGS) $(LIBS) $(OUR_SHARED_LIBS) -o ../../Clean_Build/silverback.bin $(OBJS:.c=.o) ../../libs/theora/lib/libtheora.a
	$(CC) $(CFLAGS) $(LIBS) $(OUR_SHARED_LIBS) $(SERVER_LIBS) -o ../../Clean_Build/dedicated_server.bin $(OBJS:.c=.o) $(THEORA_LIBS)

../../Clean_Build/savagedemo_server.bin: $(OBJS:.c=.o)
	sync
	$(CC) $(CFLAGS) $(LIBS) $(OUR_SHARED_LIBS) $(SERVER_LIBS) -o ../../Clean_Build/savagedemo_server.bin $(OBJS:.c=.o) $(THEORA_LIBS)

../../Clean_Build/silverback.bin: $(OBJS:.c=.o) $(CLIENTOBJS:.c=.o)
	sync
	$(CC) $(CFLAGS) $(LIBS) $(OPENGL_LIBS) $(CLIENT_LIBS) $(OUR_SHARED_LIBS) -o ../../Clean_Build/silverback.bin $(OBJS:.c=.o) $(CLIENTOBJS:.c=.o) $(THEORA_LIBS)

../../Clean_Build/silverback-static.bin: $(OBJS:.c=.o) $(CLIENTOBJS:.c=.o) ../TheGame/*.o
	sync
	$(CC) $(CFLAGS) $(LIBS) $(OPENGL_LIBS) $(CLIENT_LIBS) $(OUR_SHARED_LIBS) -o ../../Clean_Build/silverback-static.bin $(OBJS:.c=.o) $(CLIENTOBJS:.c=.o) ../TheGame/*.o ../toplayer/*.o ../gui/*.o $(THEORA_LIBS)

release:
	#CMDLINE_CFLAGS="-march=athlon-xp -mcpu=i686 -O3 -finline-functions -mno-ieee-fp -ffast-math -fstrength-reduce -funroll-loops $(CMDLINE_CFLAGS)" $(MAKE) ../../build/Savage
	CMDLINE_CFLAGS="-O3 -march=i686 -mfpmath=sse -mmmx -msse -m3dnow -ffast-math -mno-ieee-fp -funroll-loops -fomit-frame-pointer $(CMDLINE_CFLAGS)" $(MAKE) binary

demo-release:
	#CMDLINE_CFLAGS="-march=athlon-xp -mcpu=i686 -O3 -finline-functions -mno-ieee-fp -ffast-math -fstrength-reduce -funroll-loops $(CMDLINE_CFLAGS)" $(MAKE) ../../build/Savage
	CMDLINE_CFLAGS="-O3 -march=i686 -mfpmath=sse -mmmx -msse -m3dnow -ffast-math -mno-ieee-fp -funroll-loops -fomit-frame-pointer -DSAVAGE_DEMO $(CMDLINE_CFLAGS)" $(MAKE) binary

release-game:
	CMDLINE_CFLAGS="$(CMDLINE_CFLAGS) -DFINAL_GAME" $(MAKE) release
	
release-static:
	#CMDLINE_CFLAGS="-march=athlon-xp -mcpu=i686 -O3 -finline-functions -mno-ieee-fp -ffast-math -fstrength-reduce -funroll-loops $(CMDLINE_CFLAGS)" $(MAKE) ../../build/Savage
	CMDLINE_CFLAGS="-march=i686 -mfpmath=sse -mmmx -msse -m3dnow -minline-all-stringops -O3 -ffast-math $(CMDLINE_CFLAGS)" $(MAKE) binary-static
	
release-profile:
    #removed -fprofile-arcs because of the bbfunc.o crap
	#CMDLINE_CFLAGS="-march=athlon-xp -mcpu=i686 -O3 -finline-functions -mno-ieee-fp -ffast-math -fstrength-reduce -funroll-loops -pg $(CMDLINE_CFLAGS)" $(MAKE) ../../build/Savage
	CMDLINE_CFLAGS="-march=i686 -mfpmath=sse -mmmx -msse -m3dnow -minline-all-stringops -O3 -ffast-math -pg -DSTATIC_BUILD -g $(CMDLINE_CFLAGS)" $(MAKE) binary-static
	
depend:
	$(DEPEND) $(INCLUDE) $(CFLAGS) $(OBJS) $(CLIENTOBJS)

debug: 
	CMDLINE_CFLAGS="-g $(CMDLINE_CFLAGS)" $(MAKE) binary
	#CMDLINE_CFLAGS="-g $(CMDLINE_CFLAGS)" $(MAKE) ../../build/servertest
	
debug-static:
	CMDLINE_CFLAGS="-g $(CMDLINE_CFLAGS)" $(MAKE) binary-static
	
server-optimized:
	CMDLINE_CFLAGS="-g -O2 $(CMDLINE_CFLAGS)" $(MAKE) dedicated_server
	
debug-optimized:
	CMDLINE_CFLAGS="-g -O2 $(CMDLINE_CFLAGS)" $(MAKE) binary
	
debug-efence:
	CMDLINE_CFLAGS="-g $(CMDLINE_CFLAGS)" SHARED_LIBS=-lefence $(MAKE) binary
	
debug-njamd:
	CMDLINE_CFLAGS="-g $(CMDLINE_CFLAGS)" SHARED_LIBS=-lnjamd $(MAKE) binary
#	CFLAGS="-g -DDMALLOC $(CFLAGS)" SHARED_LIBS=/usr/local/lib/libdmallocth.a $(MAKE) Savage
	
debug-dmalloc:
	CMDLINE_CFLAGS="-g -DDMALLOC $(CMDLINE_CFLAGS)" SHARED_LIBS=-ldmalloc $(MAKE) binary
#	CFLAGS="-g -DDMALLOC $(CFLAGS)" SHARED_LIBS=/usr/local/lib/libdmallocth.a $(MAKE) Savage
	
debug-profile:
    #removed -fprofile-arcs because of the bbfunc.o crap
	CMDLINE_CFLAGS="-g -pg $(CMDLINE_CFLAGS)" $(MAKE) binary-static
#	CFLAGS="-g -DDMALLOC $(CFLAGS)" SHARED_LIBS=/usr/local/lib/libdmallocth.a $(MAKE) Savage
	
profile:
    #removed -fprofile-arcs because of the bbfunc.o crap
	CMDLINE_CFLAGS="-pg $(CMDLINE_CFLAGS)" $(MAKE) binary
#	CFLAGS="-g -DDMALLOC $(CFLAGS)" SHARED_LIBS=/usr/local/lib/libdmallocth.a $(MAKE) Savage

clean:
	rm -f *.o *.da

# DO NOT DELETE
allocator.o: allocator.c /usr/include/stdlib.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h core.h \
  /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h \
  /usr/include/wchar.h /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h
bans.o: bans.c core.h /usr/include/stdlib.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h bans.h
bink_unix.o: bink_unix.c core.h /usr/include/stdlib.h \
  /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h
bitmap.o: bitmap.c core.h /usr/include/stdlib.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h /usr/include/png.h /usr/include/zlib.h \
  /usr/include/zconf.h /usr/include/pngconf.h /usr/include/setjmp.h \
  /usr/include/bits/setjmp.h
bsp.o: bsp.c core.h /usr/include/stdlib.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h navpoly.h bsp.h list.h
camerautils.o: camerautils.c savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/stdlib.h \
  /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/string.h /usr/include/ctype.h savage_mathlib.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h
client_le.o: client_le.c core.h /usr/include/stdlib.h \
  /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h
cmd.o: cmd.c core.h /usr/include/stdlib.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h
colorutils.o: colorutils.c core.h /usr/include/stdlib.h \
  /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h
cookie.o: cookie.c core.h /usr/include/stdlib.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h
console.o: console.c core.h /usr/include/stdlib.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h ../gui/gui_textbuffer.h
cvar.o: cvar.c core.h /usr/include/stdlib.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h zip.h /usr/include/zlib.h /usr/include/zconf.h
cvar_container.o: cvar_container.c core.h /usr/include/stdlib.h \
  /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h
drawutils.o: drawutils.c core.h /usr/include/stdlib.h \
  /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h
eval.o: eval.c core.h /usr/include/stdlib.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h
file.o: file.c core.h /usr/include/stdlib.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h unzip.h /usr/include/zlib.h /usr/include/zconf.h
geom.o: geom.c core.h /usr/include/stdlib.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h
gl_console.o: gl_console.c core.h /usr/include/stdlib.h \
  /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h resource.h /usr/local/include/SDL/SDL.h \
  /usr/local/include/SDL/SDL_main.h /usr/local/include/SDL/SDL_types.h \
  /usr/local/include/SDL/SDL_getenv.h /usr/local/include/SDL/SDL_error.h \
  /usr/local/include/SDL/begin_code.h /usr/local/include/SDL/close_code.h \
  /usr/local/include/SDL/SDL_rwops.h /usr/local/include/SDL/SDL_timer.h \
  /usr/local/include/SDL/SDL_audio.h \
  /usr/local/include/SDL/SDL_byteorder.h \
  /usr/local/include/SDL/SDL_cdrom.h \
  /usr/local/include/SDL/SDL_joystick.h \
  /usr/local/include/SDL/SDL_events.h /usr/local/include/SDL/SDL_active.h \
  /usr/local/include/SDL/SDL_keyboard.h \
  /usr/local/include/SDL/SDL_keysym.h /usr/local/include/SDL/SDL_mouse.h \
  /usr/local/include/SDL/SDL_video.h /usr/local/include/SDL/SDL_mutex.h \
  /usr/local/include/SDL/SDL_quit.h /usr/local/include/SDL/SDL_version.h
gl_main.o: gl_main.c core.h /usr/include/stdlib.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h
buddies.o: buddies.c core.h /usr/include/stdlib.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h
font.o: font.c core.h /usr/include/stdlib.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h /usr/include/ft2build.h \
  /usr/include/freetype2/freetype/config/ftheader.h \
  /usr/include/freetype2/freetype/internal/internal.h \
  /usr/include/freetype2/freetype/freetype.h \
  /usr/include/freetype2/freetype/config/ftconfig.h \
  /usr/include/freetype2/freetype/config/ftoption.h \
  /usr/include/freetype2/freetype/config/ftstdlib.h /usr/include/setjmp.h \
  /usr/include/bits/setjmp.h /usr/include/freetype2/freetype/fterrors.h \
  /usr/include/freetype2/freetype/ftmoderr.h \
  /usr/include/freetype2/freetype/fterrdef.h \
  /usr/include/freetype2/freetype/fttypes.h \
  /usr/include/freetype2/freetype/ftsystem.h \
  /usr/include/freetype2/freetype/ftimage.h
gui.o: gui.c core.h /usr/include/stdlib.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h
gui_drawutils.o: gui_drawutils.c core.h /usr/include/stdlib.h \
  /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h
gui_panel.o: gui_panel.c core.h /usr/include/stdlib.h \
  /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h
hash.o: hash.c core.h /usr/include/stdlib.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h
heap.o: heap.c savage_common.h /usr/include/stdlib.h \
  /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/string.h /usr/include/ctype.h savage_mathlib.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h heap.h mem.h
host.o: host.c core.h /usr/include/stdlib.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h /usr/include/setjmp.h /usr/include/bits/setjmp.h \
  navrep.h navmesh.h quadtree.h navpoly.h bans.h
host_getcoreapi.o: host_getcoreapi.c core.h /usr/include/stdlib.h \
  /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h navrep.h navmesh.h quadtree.h navpoly.h
http.o: http.c core.h /usr/include/stdlib.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h /usr/include/curl/curl.h /usr/include/curl/types.h \
  /usr/include/curl/easy.h /usr/include/curl/multi.h
input.o: input.c core.h /usr/include/stdlib.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h zip.h /usr/include/zlib.h /usr/include/zconf.h
input_sdl.o: input_sdl.c /usr/local/include/SDL/SDL.h \
  /usr/local/include/SDL/SDL_main.h /usr/local/include/SDL/SDL_types.h \
  /usr/local/include/SDL/SDL_getenv.h /usr/local/include/SDL/SDL_error.h \
  /usr/local/include/SDL/begin_code.h /usr/local/include/SDL/close_code.h \
  /usr/local/include/SDL/SDL_rwops.h /usr/include/stdio.h \
  /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/wordsize.h \
  /usr/include/bits/typesizes.h /usr/include/libio.h \
  /usr/include/_G_config.h /usr/include/wchar.h /usr/include/bits/wchar.h \
  /usr/include/gconv.h /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/local/include/SDL/SDL_timer.h /usr/local/include/SDL/SDL_audio.h \
  /usr/local/include/SDL/SDL_byteorder.h \
  /usr/local/include/SDL/SDL_cdrom.h \
  /usr/local/include/SDL/SDL_joystick.h \
  /usr/local/include/SDL/SDL_events.h /usr/local/include/SDL/SDL_active.h \
  /usr/local/include/SDL/SDL_keyboard.h \
  /usr/local/include/SDL/SDL_keysym.h /usr/local/include/SDL/SDL_mouse.h \
  /usr/local/include/SDL/SDL_video.h /usr/local/include/SDL/SDL_mutex.h \
  /usr/local/include/SDL/SDL_quit.h /usr/local/include/SDL/SDL_version.h \
  core.h /usr/include/stdlib.h /usr/include/sys/types.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/math.h \
  /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h \
  /usr/include/bits/mathcalls.h /usr/include/ctype.h /usr/include/errno.h \
  /usr/include/bits/errno.h /usr/include/linux/errno.h \
  /usr/include/asm/errno.h /usr/include/asm-generic/errno.h \
  /usr/include/asm-generic/errno-base.h /usr/include/libintl.h \
  /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h
intersection.o: intersection.c core.h /usr/include/stdlib.h \
  /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h
jpeg.o: jpeg.c core.h /usr/include/stdlib.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h /usr/include/png.h /usr/include/zlib.h \
  /usr/include/zconf.h /usr/include/pngconf.h /usr/include/setjmp.h \
  /usr/include/bits/setjmp.h /usr/include/jpeglib.h \
  /usr/include/jconfig.h /usr/include/jmorecfg.h /usr/include/jerror.h
main_sdl.o: main_sdl.c core.h /usr/include/stdlib.h \
  /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h /usr/local/include/SDL/SDL.h \
  /usr/local/include/SDL/SDL_main.h /usr/local/include/SDL/SDL_types.h \
  /usr/local/include/SDL/SDL_getenv.h /usr/local/include/SDL/SDL_error.h \
  /usr/local/include/SDL/begin_code.h /usr/local/include/SDL/close_code.h \
  /usr/local/include/SDL/SDL_rwops.h /usr/local/include/SDL/SDL_timer.h \
  /usr/local/include/SDL/SDL_audio.h \
  /usr/local/include/SDL/SDL_byteorder.h \
  /usr/local/include/SDL/SDL_cdrom.h \
  /usr/local/include/SDL/SDL_joystick.h \
  /usr/local/include/SDL/SDL_events.h /usr/local/include/SDL/SDL_active.h \
  /usr/local/include/SDL/SDL_keyboard.h \
  /usr/local/include/SDL/SDL_keysym.h /usr/local/include/SDL/SDL_mouse.h \
  /usr/local/include/SDL/SDL_video.h /usr/local/include/SDL/SDL_mutex.h \
  /usr/local/include/SDL/SDL_quit.h /usr/local/include/SDL/SDL_version.h \
  /usr/include/execinfo.h /usr/include/sys/wait.h \
  /usr/include/sys/resource.h /usr/include/bits/resource.h \
  /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
main_linux.o: main_linux.c core.h /usr/include/stdlib.h \
  /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h /usr/include/dirent.h /usr/include/bits/dirent.h \
  /usr/include/glob.h /usr/include/dlfcn.h /usr/include/bits/dlfcn.h
mem.o: mem.c core.h /usr/include/stdlib.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h
misc_cmds.o: misc_cmds.c core.h /usr/include/stdlib.h \
  /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h
net_deltastructs.o: net_deltastructs.c core.h /usr/include/stdlib.h \
  /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h
net.o: net.c core.h /usr/include/stdlib.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h
net_bsdsock.o: net_bsdsock.c core.h /usr/include/stdlib.h \
  /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h
net_unix.o: net_unix.c core.h /usr/include/stdlib.h \
  /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h net_bsdsock.h
net_server.o: net_server.c core.h /usr/include/stdlib.h \
  /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h ../keyserver/ssl-utils.h ../keyserver/auth_common.h \
  ../Core/savage_common.h /usr/include/sys/wait.h \
  /usr/include/sys/resource.h /usr/include/bits/resource.h \
  /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h \
  /usr/include/sys/ioctl.h /usr/include/bits/ioctls.h \
  /usr/include/asm/ioctls.h /usr/include/asm/ioctl.h \
  /usr/include/bits/ioctl-types.h /usr/include/sys/ttydefaults.h \
  /usr/include/net/if.h cookie.h
net_tcp.o: net_tcp.c core.h /usr/include/stdlib.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h ../keyserver/ssl-utils.h ../keyserver/auth_common.h \
  ../Core/savage_common.h /usr/include/sys/wait.h \
  /usr/include/sys/resource.h /usr/include/bits/resource.h \
  /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h \
  /usr/include/sys/ioctl.h /usr/include/bits/ioctls.h \
  /usr/include/asm/ioctls.h /usr/include/asm/ioctl.h \
  /usr/include/bits/ioctl-types.h /usr/include/sys/ttydefaults.h \
  /usr/include/net/if.h cookie.h
net_irc.o: net_irc.c core.h /usr/include/stdlib.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h ../keyserver/auth_common.h ../Core/savage_common.h \
  ../keyserver/ssl-utils.h /usr/include/sys/wait.h \
  /usr/include/sys/resource.h /usr/include/bits/resource.h \
  /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h \
  /usr/include/sys/ioctl.h /usr/include/bits/ioctls.h \
  /usr/include/asm/ioctls.h /usr/include/asm/ioctl.h \
  /usr/include/bits/ioctl-types.h /usr/include/sys/ttydefaults.h \
  /usr/include/net/if.h
navpoly.o: navpoly.c core.h /usr/include/stdlib.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h navpoly.h navmesh.h quadtree.h list.h bsp.h
navrep.o: navrep.c core.h /usr/include/stdlib.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h list.h heap.h set.h navrep.h navmesh.h quadtree.h \
  navpoly.h bsp.h
navmesh.o: navmesh.c core.h /usr/include/stdlib.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h navmesh.h quadtree.h navpoly.h bsp.h set.h list.h
packet.o: packet.c core.h /usr/include/stdlib.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h
parsestats.o: parsestats.c core.h /usr/include/stdlib.h \
  /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h
quadtree.o: quadtree.c core.h /usr/include/stdlib.h \
  /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h quadtree.h list.h
res.o: res.c core.h /usr/include/stdlib.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h
savage_common.o: savage_common.c savage_common.h /usr/include/stdlib.h \
  /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/string.h /usr/include/ctype.h savage_mathlib.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h
savage_mathlib.o: savage_mathlib.c savage_common.h /usr/include/stdlib.h \
  /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/string.h /usr/include/ctype.h savage_mathlib.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h
serverlist.o: serverlist.c core.h /usr/include/stdlib.h \
  /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h ../keyserver/ssl-utils.h ../keyserver/auth_common.h \
  ../Core/savage_common.h /usr/include/sys/wait.h \
  /usr/include/sys/resource.h /usr/include/bits/resource.h \
  /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h \
  /usr/include/sys/ioctl.h /usr/include/bits/ioctls.h \
  /usr/include/asm/ioctls.h /usr/include/asm/ioctl.h \
  /usr/include/bits/ioctl-types.h /usr/include/sys/ttydefaults.h \
  /usr/include/net/if.h
set.o: set.c savage_common.h /usr/include/stdlib.h \
  /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/string.h /usr/include/ctype.h savage_mathlib.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h set.h
scene.o: scene.c core.h /usr/include/stdlib.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h
server_le.o: server_le.c core.h /usr/include/stdlib.h \
  /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h /usr/include/zlib.h /usr/include/zconf.h bans.h \
  ../keyserver/auth_common.h ../Core/savage_common.h \
  ../keyserver/ssl-utils.h /usr/include/sys/wait.h \
  /usr/include/sys/resource.h /usr/include/bits/resource.h \
  /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h \
  /usr/include/sys/ioctl.h /usr/include/bits/ioctls.h \
  /usr/include/asm/ioctls.h /usr/include/asm/ioctl.h \
  /usr/include/bits/ioctl-types.h /usr/include/sys/ttydefaults.h \
  /usr/include/net/if.h
sound_fmod.o: sound_fmod.c core.h /usr/include/stdlib.h \
  /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h ../../libs/fmod/api/inc/wincompat.h \
  /usr/include/termios.h /usr/include/bits/termios.h \
  /usr/include/sys/ttydefaults.h ../../libs/fmod/api/inc/fmoddyn.h \
  /usr/include/dlfcn.h /usr/include/bits/dlfcn.h \
  ../../libs/fmod/api/inc/fmod.h ../../libs/fmod/api/inc/fmod_errors.h
sound_manager.o: sound_manager.c core.h /usr/include/stdlib.h \
  /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h
stringtable.o: stringtable.c core.h /usr/include/stdlib.h \
  /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h
theora.o: theora.c ../../libs/theora/include/theora/theora.h \
  /usr/local/include/ogg/ogg.h /usr/local/include/ogg/os_types.h \
  /usr/include/sys/types.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h \
  /usr/include/bits/types.h /usr/include/bits/wordsize.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/bits/typesizes.h /usr/include/time.h /usr/include/endian.h \
  /usr/include/bits/endian.h /usr/include/sys/select.h \
  /usr/include/bits/select.h /usr/include/bits/sigset.h \
  /usr/include/bits/time.h /usr/include/sys/sysmacros.h \
  /usr/include/bits/pthreadtypes.h /usr/include/bits/sched.h \
  /usr/local/include/ogg/config_types.h /usr/include/vorbis/codec.h \
  core.h /usr/include/stdlib.h /usr/include/bits/waitflags.h \
  /usr/include/bits/waitstatus.h /usr/include/xlocale.h \
  /usr/include/alloca.h /usr/include/stdio.h /usr/include/libio.h \
  /usr/include/_G_config.h /usr/include/wchar.h /usr/include/bits/wchar.h \
  /usr/include/gconv.h /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/nan.h /usr/include/bits/mathdef.h \
  /usr/include/bits/mathcalls.h /usr/include/ctype.h /usr/include/errno.h \
  /usr/include/bits/errno.h /usr/include/linux/errno.h \
  /usr/include/asm/errno.h /usr/include/asm-generic/errno.h \
  /usr/include/asm-generic/errno-base.h /usr/include/libintl.h \
  /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/xopen_lim.h \
  /usr/include/bits/sockaddr.h /usr/include/asm/socket.h \
  /usr/include/asm/sockios.h /usr/include/sys/time.h \
  /usr/include/signal.h /usr/include/bits/signum.h \
  /usr/include/bits/siginfo.h /usr/include/bits/sigaction.h \
  /usr/include/bits/sigcontext.h /usr/include/asm/sigcontext.h \
  /usr/include/linux/compiler.h /usr/include/bits/sigstack.h \
  /usr/include/sys/ucontext.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/environments.h /usr/include/bits/confname.h \
  /usr/include/getopt.h /usr/include/fcntl.h /usr/include/bits/fcntl.h \
  /usr/include/netdb.h /usr/include/rpc/netdb.h /usr/include/bits/netdb.h \
  /usr/include/glib-2.0/glib.h /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h /usr/local/include/SDL/SDL.h \
  /usr/local/include/SDL/SDL_main.h /usr/local/include/SDL/SDL_types.h \
  /usr/local/include/SDL/SDL_getenv.h /usr/local/include/SDL/SDL_error.h \
  /usr/local/include/SDL/begin_code.h /usr/local/include/SDL/close_code.h \
  /usr/local/include/SDL/SDL_rwops.h /usr/local/include/SDL/SDL_timer.h \
  /usr/local/include/SDL/SDL_audio.h \
  /usr/local/include/SDL/SDL_byteorder.h \
  /usr/local/include/SDL/SDL_cdrom.h \
  /usr/local/include/SDL/SDL_joystick.h \
  /usr/local/include/SDL/SDL_events.h /usr/local/include/SDL/SDL_active.h \
  /usr/local/include/SDL/SDL_keyboard.h \
  /usr/local/include/SDL/SDL_keysym.h /usr/local/include/SDL/SDL_mouse.h \
  /usr/local/include/SDL/SDL_video.h /usr/local/include/SDL/SDL_mutex.h \
  /usr/local/include/SDL/SDL_quit.h /usr/local/include/SDL/SDL_version.h \
  /usr/include/sys/soundcard.h /usr/include/linux/soundcard.h \
  /usr/include/linux/ioctl.h /usr/include/asm/ioctl.h \
  /usr/include/sys/ioctl.h /usr/include/bits/ioctls.h \
  /usr/include/asm/ioctls.h /usr/include/bits/ioctl-types.h \
  /usr/include/sys/ttydefaults.h
timeofday.o: timeofday.c core.h /usr/include/stdlib.h \
  /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h zip.h /usr/include/zlib.h /usr/include/zconf.h
unzip.o: unzip.c unzip.h core.h /usr/include/stdlib.h \
  /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h /usr/include/zlib.h /usr/include/zconf.h \
  /usr/include/utime.h
vid.o: vid.c core.h /usr/include/stdlib.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h
vid_sdl.o: vid_sdl.c core.h /usr/include/stdlib.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h gl_sdl.h gl_console.h /usr/local/include/SDL/SDL.h \
  /usr/local/include/SDL/SDL_main.h /usr/local/include/SDL/SDL_types.h \
  /usr/local/include/SDL/SDL_getenv.h /usr/local/include/SDL/SDL_error.h \
  /usr/local/include/SDL/begin_code.h /usr/local/include/SDL/close_code.h \
  /usr/local/include/SDL/SDL_rwops.h /usr/local/include/SDL/SDL_timer.h \
  /usr/local/include/SDL/SDL_audio.h \
  /usr/local/include/SDL/SDL_byteorder.h \
  /usr/local/include/SDL/SDL_cdrom.h \
  /usr/local/include/SDL/SDL_joystick.h \
  /usr/local/include/SDL/SDL_events.h /usr/local/include/SDL/SDL_active.h \
  /usr/local/include/SDL/SDL_keyboard.h \
  /usr/local/include/SDL/SDL_keysym.h /usr/local/include/SDL/SDL_mouse.h \
  /usr/local/include/SDL/SDL_video.h /usr/local/include/SDL/SDL_mutex.h \
  /usr/local/include/SDL/SDL_quit.h /usr/local/include/SDL/SDL_version.h
world.o: world.c core.h /usr/include/stdlib.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h zip.h /usr/include/zlib.h /usr/include/zconf.h
world_objects.o: world_objects.c core.h /usr/include/stdlib.h \
  /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h navrep.h navmesh.h quadtree.h navpoly.h zip.h \
  /usr/include/zlib.h /usr/include/zconf.h
world_objectgrid.o: world_objectgrid.c core.h /usr/include/stdlib.h \
  /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h
world_lights.o: world_lights.c core.h /usr/include/stdlib.h \
  /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h
world_tree2.o: world_tree2.c core.h /usr/include/stdlib.h \
  /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h
keyclient.o: keyclient.c core.h /usr/include/stdlib.h \
  /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h ../keyserver/auth_common.h ../Core/savage_common.h \
  ../keyserver/ssl-utils.h /usr/include/sys/wait.h \
  /usr/include/sys/resource.h /usr/include/bits/resource.h \
  /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h \
  /usr/include/sys/ioctl.h /usr/include/bits/ioctls.h \
  /usr/include/asm/ioctls.h /usr/include/asm/ioctl.h \
  /usr/include/bits/ioctl-types.h /usr/include/sys/ttydefaults.h \
  /usr/include/net/if.h
zip.o: zip.c unzip.h core.h /usr/include/stdlib.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h /usr/include/zlib.h /usr/include/zconf.h zip.h
ssl-utils.o: ../keyserver/ssl-utils.c core.h /usr/include/stdlib.h \
  /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h
gl_terrain.o: gl_terrain.c core.h /usr/include/stdlib.h \
  /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h
gl_model.o: gl_model.c core.h /usr/include/stdlib.h \
  /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h
gl_scene.o: gl_scene.c core.h /usr/include/stdlib.h \
  /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h
gl_sdl.o: gl_sdl.c core.h /usr/include/stdlib.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h resource.h /usr/local/include/SDL/SDL.h \
  /usr/local/include/SDL/SDL_main.h /usr/local/include/SDL/SDL_types.h \
  /usr/local/include/SDL/SDL_getenv.h /usr/local/include/SDL/SDL_error.h \
  /usr/local/include/SDL/begin_code.h /usr/local/include/SDL/close_code.h \
  /usr/local/include/SDL/SDL_rwops.h /usr/local/include/SDL/SDL_timer.h \
  /usr/local/include/SDL/SDL_audio.h \
  /usr/local/include/SDL/SDL_byteorder.h \
  /usr/local/include/SDL/SDL_cdrom.h \
  /usr/local/include/SDL/SDL_joystick.h \
  /usr/local/include/SDL/SDL_events.h /usr/local/include/SDL/SDL_active.h \
  /usr/local/include/SDL/SDL_keyboard.h \
  /usr/local/include/SDL/SDL_keysym.h /usr/local/include/SDL/SDL_mouse.h \
  /usr/local/include/SDL/SDL_video.h /usr/local/include/SDL/SDL_mutex.h \
  /usr/local/include/SDL/SDL_quit.h /usr/local/include/SDL/SDL_version.h
gl_scene_builder.o: gl_scene_builder.c core.h /usr/include/stdlib.h \
  /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stddef.h \
  /usr/include/sys/types.h /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h \
  /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/select.h /usr/include/bits/select.h \
  /usr/include/bits/sigset.h /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h /usr/include/alloca.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/include/bits/wchar.h /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/include/libintl.h /usr/include/locale.h /usr/include/bits/locale.h \
  /usr/include/sys/stat.h /usr/include/bits/stat.h os.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/sys/time.h /usr/include/signal.h \
  /usr/include/bits/signum.h /usr/include/bits/siginfo.h \
  /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h \
  /usr/include/asm/sigcontext.h /usr/include/linux/compiler.h \
  /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h \
  /usr/include/netinet/in.h /usr/include/stdint.h /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h /usr/include/arpa/inet.h \
  /usr/include/unistd.h /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h /usr/include/getopt.h /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h /usr/include/netdb.h /usr/include/rpc/netdb.h \
  /usr/include/bits/netdb.h /usr/include/glib-2.0/glib.h \
  /usr/include/glib-2.0/glib/galloca.h \
  /usr/include/glib-2.0/glib/gtypes.h \
  /usr/lib/glib-2.0/include/glibconfig.h \
  /usr/include/glib-2.0/glib/gmacros.h \
  /usr/lib/gcc-lib/i486-linux/3.3.2/include/float.h \
  /usr/include/glib-2.0/glib/garray.h \
  /usr/include/glib-2.0/glib/gasyncqueue.h \
  /usr/include/glib-2.0/glib/gthread.h \
  /usr/include/glib-2.0/glib/gerror.h /usr/include/glib-2.0/glib/gquark.h \
  /usr/include/glib-2.0/glib/gbacktrace.h \
  /usr/include/glib-2.0/glib/gcache.h /usr/include/glib-2.0/glib/glist.h \
  /usr/include/glib-2.0/glib/gmem.h \
  /usr/include/glib-2.0/glib/gcompletion.h \
  /usr/include/glib-2.0/glib/gconvert.h \
  /usr/include/glib-2.0/glib/gdataset.h \
  /usr/include/glib-2.0/glib/gdate.h /usr/include/glib-2.0/glib/gdir.h \
  /usr/include/glib-2.0/glib/gfileutils.h \
  /usr/include/glib-2.0/glib/ghash.h /usr/include/glib-2.0/glib/ghook.h \
  /usr/include/glib-2.0/glib/giochannel.h \
  /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gslist.h \
  /usr/include/glib-2.0/glib/gstring.h \
  /usr/include/glib-2.0/glib/gunicode.h \
  /usr/include/glib-2.0/glib/gmarkup.h \
  /usr/include/glib-2.0/glib/gmessages.h \
  /usr/include/glib-2.0/glib/gnode.h \
  /usr/include/glib-2.0/glib/gpattern.h \
  /usr/include/glib-2.0/glib/gprimes.h \
  /usr/include/glib-2.0/glib/gqsort.h /usr/include/glib-2.0/glib/gqueue.h \
  /usr/include/glib-2.0/glib/grand.h /usr/include/glib-2.0/glib/grel.h \
  /usr/include/glib-2.0/glib/gscanner.h \
  /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gspawn.h \
  /usr/include/glib-2.0/glib/gstrfuncs.h \
  /usr/include/glib-2.0/glib/gthreadpool.h \
  /usr/include/glib-2.0/glib/gtimer.h /usr/include/glib-2.0/glib/gtree.h \
  /usr/include/glib-2.0/glib/gutils.h savage_types.h \
  ../masterserver/sharedcommon.h savage_common.h /usr/include/string.h \
  savage_mathlib.h allocator.h core_api.h geom.h intersection.h mem.h \
  hash.h /usr/include/openssl/ssl.h /usr/include/openssl/e_os2.h \
  /usr/include/openssl/opensslconf.h /usr/include/openssl/comp.h \
  /usr/include/openssl/crypto.h /usr/include/openssl/stack.h \
  /usr/include/openssl/safestack.h /usr/include/openssl/opensslv.h \
  /usr/include/openssl/symhacks.h /usr/include/openssl/bio.h \
  /usr/include/openssl/x509.h /usr/include/openssl/buffer.h \
  /usr/include/openssl/evp.h /usr/include/openssl/ossl_typ.h \
  /usr/include/openssl/md2.h /usr/include/openssl/md4.h \
  /usr/include/openssl/md5.h /usr/include/openssl/sha.h \
  /usr/include/openssl/ripemd.h /usr/include/openssl/des.h \
  /usr/include/openssl/rc4.h /usr/include/openssl/rc2.h \
  /usr/include/openssl/blowfish.h /usr/include/openssl/cast.h \
  /usr/include/openssl/aes.h /usr/include/openssl/rsa.h \
  /usr/include/openssl/asn1.h /usr/include/openssl/bn.h \
  /usr/include/openssl/dsa.h /usr/include/openssl/dh.h \
  /usr/include/openssl/objects.h /usr/include/openssl/obj_mac.h \
  /usr/include/openssl/x509_vfy.h /usr/include/openssl/lhash.h \
  /usr/include/openssl/pkcs7.h /usr/include/openssl/kssl.h \
  /usr/include/openssl/pem.h /usr/include/openssl/pem2.h \
  /usr/include/openssl/ssl2.h /usr/include/openssl/ssl3.h \
  /usr/include/openssl/tls1.h /usr/include/openssl/ssl23.h host.h \
  system.h file.h cvar.h cvar_container.h cmd.h console.h bitmap.h \
  sound.h sound_manager.h font.h gui.h gui_panel.h gui_drawutils.h \
  keyclient.h serverlist.h /usr/include/GL/gl.h /usr/include/GL/glu.h \
  /usr/include/GL/glext.h /usr/include/GL/glx.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
  /usr/include/X11/Xmd.h /usr/include/GL/glxtokens.h world_objectgrid.h \
  shader.h stringtable.h res.h world.h world_tree2.h world_objects.h \
  world_lights.h timeofday.h scene.h camerautils.h gl_main.h gl_scene.h \
  gl_terrain.h vid.h input.h net.h packet.h server.h client.h drawutils.h \
  colorutils.h net_server.h bink_unix.h theora.h http.h net_tcp.h \
  net_irc.h buddies.h
