#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>

#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <net/if.h>
#include <arpa/inet.h>

#include "sharedcommon.h"

#define true 1
#define false 0
#define bool int
#define byte unsigned char

//packet types
#define CPROTO_HEARTBEAT			0xCA
#define	CPROTO_SERVER_SHUTDOWN  	0xCB
#define CPROTO_PLAYER_DISCONNECT    0xCC

#ifndef MAX
#define MAX(x,y) ((x)<(y)?(y):(x))
#endif

#define MAX_QUEUE_LENGTH    10000

#define NUM_RACES 3
#define MAX_PLAYERS 64

#define	MAX_SERVERS 10000 //max servers sent to a client asking for the master list
#define	MAX_USERS 100 //max servers sent to a client asking for the master list

#define MASTERSERVER_LISTEN_PORT 11236 

#define FULL_GAME_LIST_MSG	"FULL_LIST"
#define USER_SEARCH_MSG		"USER_LIST"
#define FIREWALL_TEST_MSG	"FIREWALL_TEST"
#define USERINFO_COOKIE_MSG	"USERCOOKIE"

#define FLAG_FULL_GAMELIST	0x001000
#define FLAG_USER_SEARCH	0x002000
#define FLAG_FIREWALL_TEST	0x004000
#define	FLAG_USERINFO_COOKIE 0x008000

