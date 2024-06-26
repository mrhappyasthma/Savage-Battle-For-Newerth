// (C) 2003 S2 Games

// core.h

// Functions and definitions available to the core engine

#ifndef _S2CORE_H
#define _S2CORE_H

//#define INTERNATIONAL

/*#ifdef INTERNATIONAL
#pragma message( "(INTERNATIONAL build)")
#define	_(a)	gettext(a)
#else
#define _(a)	(a)
#endif*/
#ifdef unix
#define _(a)	(a)
#else
#define _(a)	getstring(a)
#include "../../libs/lokalizator_lib/lokalizator.h"
#pragma comment (lib, "../../libs/lokalizator_lib/lokalizator.lib")
#endif

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <stddef.h>
#include <errno.h>
#include <time.h>

/*#ifdef INTERNATIONAL
#include <libintl.h>
#include <locale.h>
#endif*/

#include <sys/types.h>
#include <sys/stat.h>

#ifndef _S2_DONT_INCLUDE_OS
	#include "os.h"
	#include <glib.h>
#endif

struct media_s;		//forward declare media_s

//#include "savage_common.h"
#include "savage_types.h"
#include "savage_mathlib.h"
#include "allocator.h"
#include "core_api.h"
#include "geom.h"
#include "intersection.h"
#include "mem.h"
#include "hash.h"
#include "host.h"
#include "system.h"
#include "file.h"
#include "cvar.h"
#include "cvar_container.h"
#include "cmd.h"
#include "console.h"
#include "bitmap.h"
//#include "sound.h"
#include "sound.h"
#include "sound_manager.h"
//#include "track.h"
//#include "paths.h"
//#include "dynamicpaths.h"
//#include "time_changes.h"
#include "font.h"
#include "gui.h"
#include "keyclient.h"
#include "serverlist.h"

#ifndef _S2_DONT_INCLUDE_GL	
	#ifdef __APPLE__
		#include <OpenGL/gl.h>
		#include <OpenGL/glu.h>
		#include <OpenGL/glext.h>
		#include <GL/glext.h>
	#else
		#include <GL/gl.h>
		#include <GL/glu.h>
		#include "GL/glext.h"
	#endif
	#ifdef _WIN32
		#include "gl/wglext.h"
	#else
		#ifdef __APPLE__
			#include <OpenGL/OpenGL.h>
		#else
			#include <GL/glx.h>
		#endif
		//#undef ClientMessage
	#endif
#endif

#include "world_objectgrid.h"
//#include "s2model.h"
//#ifdef TEMP_GEOM_FILE
//#include "geom.h.working"
//#else
//#endif
#include "shader.h"
#include "stringtable.h"
#include "res.h"
#include "world.h"
#include "timeofday.h"
#include "scene.h"
#include "camerautils.h"
#ifndef _S2_DONT_INCLUDE_GL
	#include "gl_main.h"
	#include "gl_scene.h"
	#include "gl_terrain.h"
#endif

#include "vid.h"
#include "input.h"
#include "net.h"
#include "server.h"
#include "client.h"
#include "drawutils.h"
#include "colorutils.h"
#include "net_server.h"
#include "theora.h"
#include "http.h"
#include "net_tcp.h"
#include "net_irc.h"
#include "buddies.h"
#ifdef _WIN32
	#include "bink_win32.h"
#else
	#include "bink_unix.h"
#endif

#endif
#ifdef ClientMessage //for whatever reason this is a numerical #define in unix, and that breaks host.c
#undef ClientMessage
#endif

extern	file_t *DEBUG_LOG_FILE;

//for mem debugging in linux
#ifdef DMALLOC
#include "dmalloc.h"
#endif

