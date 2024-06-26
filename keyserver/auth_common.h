#ifndef AUTH_COMMONH
#define AUTH_COMMONH

#include "../Core/savage_common.h"
#include "ssl-utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>

#ifndef _WIN32
#include <unistd.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <net/if.h>
#include <arpa/inet.h>
#endif

#include <string.h>
#include <fcntl.h>
#include <time.h>

#include <sys/types.h>

#define true 1
#define false 0
#define bool int

#define MAX_AUTH_PACKET_SIZE 512

#define USE_SSL

#define KEYSERVER_IP "keyserver.savage.s2games.com"
#define KEYSERVER_LISTEN_PORT 11237

#ifdef __linux
#define PLATFORM "Linux"
#elif defined(_WIN32)
#define PLATFORM "Windows"
#elif __APPLE__
#define PLATFORM "Mac"
#else
#define PLATFORM "Unknown"
#endif

#define SERVER_KEY      "SERVER"
#define PORT_KEY      	"PORT"
#define BUILD_KEY       "BUILD"
#define PLATFORM_KEY    "PLATFORM"
#define USER_KEY        "USER"
#define PASSWORD_KEY    "PASS"
#define MD5PASSWORD_KEY "MD5PASS"
#define CDKEY_KEY		"CDKEY"
#define CREATEUSER_KEY	"CREATEUSER"

#define AUTH_SUCCESS "ok: "
#define AUTH_FAILURE "access denied"

#define KS_HI_CMD    		"KS_HI"
#define GS_HI_CMD    		"SV_HI"
#define ALLOW_COOKIE_CMD    "KS_ALLOWCOOKIE"
#define STATESTRING_CMD    	"KS_STATESTRING"
#define NEWPATCH_CMD    	"KS_NEWPATCH"
#define NOTFIREWALLED_CMD  	"KS_NOTFIREWALLED"

#define SLOTHYS_COOKIE		"31337!_______"
#define SLOTHYS_USERID		1
#define JESSES_COOKIE		"jesses2______"
#define JESSES_USERID		2
#define SAMMYS_COOKIE		"sammys2______"
#define SAMMYS_USERID		3
#define JASONS_COOKIE		"jasons2______"
#define JASONS_USERID		4
#define TRAVISS_COOKIE		"traviss2_____"
#define TRAVISS_USERID		5
#define TREVORS_COOKIE		"trevors2_____"
#define TREVORS_USERID		6
#define MARCS_COOKIE		"marcs2_______"
#define MARCS_USERID		12
#define RICKS_COOKIE		"ricks2_______"
#define RICKS_USERID		21
#define WILLIES_COOKIE		"willes2______"
#define WILLIES_USERID		22

#endif
