//os.h

#ifdef _WIN32
	#pragma warning (disable: 4305)		//conversion to smaller type
	#pragma warning (disable: 4244)		//type conversion
	#pragma warning (disable: 4018)		//signed/unsigned mismatch
	#define WIN32_LEAN_AND_MEAN
	#include <objbase.h>
	#include <windows.h>
	#include <winuser.h>
	#include <winsock2.h>
	#include <direct.h>
	#include <Ws2tcpip.h>
	#include <wchar.h>
int wmemcmp(const wchar_t *_S1, const wchar_t *_S2, size_t _N);
	#define snprintf _snprintf
	#define STRUCT_STAT	struct stat
	//#include "CrashRpt.h"
	#define RAD_NO_LOWERCASE_TYPES
	#include "bink.h"
	#include "rad3d.h"
	HWND	System_Win32_GetHWnd();
	#define USE_NVIDIA_EXTENSIONS
	#define USE_VBO
#else //not _WIN32

	#include <sys/socket.h>
	#include <sys/time.h>
	#include <signal.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <unistd.h>
	#include <fcntl.h>
	#include <netdb.h>
	#define STRUCT_STAT	struct stat
	#define SOCKET int

	#ifdef __APPLE__
	typedef int socklen_t;
	typedef void* __sighandler_t;
	#endif //no socklen_t

	#ifdef __APPLE__
	#define glVertexArrayRangeARB glVertexArrayRangeAPPLE
	#define glVertexArrayRangeNV glVertexArrayRangeAPPLE
	#define glFlushVertexArrayRangeARB glFlushVertexArrayRangeAPP
	#define glVertexArrayParameteriARB glVertexArrayParameteriAPPLE
	#define glGenFencesNV glGenFencesAPPLE
	#define glDeleteFencesNV glDeleteFencesAPPLE
	#define glSetFenceNV glSetFenceAPPLE
	#define glFinishFenceNV glFinishFenceAPPLE
	#define GL_VERTEX_ARRAY_RANGE_NV GL_VERTEX_ARRAY_RANGE_APPLE
	#endif

	#define USE_NVIDIA_EXTENSIONS

	#ifndef __APPLE__
	#define USE_VBO
	#endif

#endif //not _WIN32
