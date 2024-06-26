// (C) 2003 S2 Games

// net.h

// network driver layer

#define NET_PROTOCOL_VERSION 12 //7 == RC1.  8 == RC2
#define NET_PROTOCOL_VERSION_STRING "12"


#define	MAX_ADDRESS_LENGTH	64
#define MAX_NETDRIVERS		16
#define	DEFAULT_SERVER_PORT	11235
#define	MAX_DRIVERDATA_SIZE	256		
#define	DEFAULT_SERVER_PORT_ASCII "11235"
#define MAX_NAME_LENGTH		64

#define MAX_PACKETS_RESENT_PER_FRAME	4
#define MAX_PACKET_SEQUENCES_AHEAD		3

//number of sequence numbers ahead to check in the case of a reliable packet with
// a sequence number that doesn't match the next expected one.  We don't want to make this
// too big or people could DOS clients by sending a lot of invalid packet sequences.
#define MAX_NUM_MISSED_PACKETS		20
#define	PACKET_TIMEOUT_MILLIS		500

#define HEADER_FLAG_LOC		4

#define PACKET_NORMAL			0x00
#define PACKET_RELIABLE			0x01
#define PACKET_ACK				0x02
//#define PACKET_REQUEST_RESEND	0x04  //not used anymore

#include "packet.h"
#include "../masterserver/sharedcommon.h" 

#define NONRELIABLE_SEQUENCE 2313374u //2 31337 4u d00d!!!  # warning: this is also defined in the keyserver code and must be synchronized

#define CPROTO_VERSION      1

//client commands
#define CPROTO_ACK                  0xC1        //general acknowledgement
#define CPROTO_CONNECT_ME           0xC2

//server commands
#define CPROTO_OK           0xC3
#define CPROTO_NO           0xC4

//keyserver commands
#define CPROTO_SEND_HEARTBEAT   0xC5

//pings
#define	CPROTO_PING				0xC6
#define	CPROTO_PING_ACK			0xC7

//server info
#define	CPROTO_INFO_REQUEST		0xC8
#define	CPROTO_INFO				0xC9

//heartbeat packet
#define	CPROTO_HEARTBEAT		0xCA
#define	CPROTO_SERVER_SHUTDOWN	0xCB
#define	CPROTO_PLAYER_DISCONNECT	0xCC

//client-server for re-learn my source port
#define CPROTO_LEARNPORT		0xCD

//server extended info
#define CPROTO_EXT_INFO_REQUEST	0xCE
#define CPROTO_EXT_INFO			0xCF

//this structure holds a "reliable packet" until we get an ACK
typedef struct reliablePacket_s
{
	float				timesent;
	unsigned char		*packet;
	int					data_length;
	unsigned int		seq;

	struct	reliablePacket_s	*next;
} reliablePacket_t;

typedef struct sockaddr_in netaddr_t;

typedef struct
{
	int			driver;
	bool		tcp;
	unsigned int sock;		//used as a socket identifier for the driver layer
	int			port;
	netaddr_t	addr;

	unsigned int reliableRecv_lastseq;
	packet_t	recv;
	char		recvAddrName[MAX_ADDRESS_LENGTH];
	netaddr_t	recvAddr;

	reliablePacket_t *unack_packets;
	
	packet_t	send;
	packet_t	reliableSend;
	unsigned int reliableSend_lastseq;
	char		sendAddrName[MAX_ADDRESS_LENGTH];	//address to send packet to
	netaddr_t	sendAddr;	

	char		cookie[COOKIE_SIZE+1];
	unsigned int clan_id;				//stored here because we get it before we have a client slot to put it in
	unsigned int user_id;				//stored here because we get it before we have a client slot to put it in
	unsigned int guid;					//stored here because we get it before we have a client slot to put it in
} netconnection_t;

//this structure holds pointers to all fundamental network functions
typedef struct
{
	char				*drivername;

	void				(*Init)();
	void				(*ShutDown)();

	bool				(*OpenPort)(netconnection_t *nc, int port);
	bool				(*SetSendAddr)(netconnection_t *nc, char *addr, int clientport);
	bool    			(*NewSocket)(netconnection_t *nc);
	
	int					(*ReceivePacket)(netconnection_t *nc);
	bool				(*PreProcessPacket)(netconnection_t *nc);
	bool				(*SendPacket)(netconnection_t *nc);
	bool				(*SendReliablePacket)(netconnection_t *nc);
	void				(*CloseNetConnection)(netconnection_t *nc);
	void				(*Converse)(netconnection_t *nc);

	bool				(*BroadcastPacket)(netconnection_t *nc, int port);
	
	void				(*CheckPacketTimeouts)(netconnection_t *nc);
	void				(*ClearReliablePackets)(netconnection_t *nc);
	//client functions
	bool				(*Connect)(char *addr);
	void				(*Disconnect)();	
} _net_driver_t;


//types you can use in a delta update structure
//add more below if a type you want is not supported (requires editing Net_*StructDiff, also)
typedef enum
{
	T_COORD = 1,
	T_BYTE_ANGLE,			//low precision angle (0.0 to (360.0 - 1/255) maps to 0 to 255)
	T_WORD_ANGLE,			//high precision angle (0.0 to (360.0 - 1/65535) maps to 0 to 65535)
	T_FLOAT,
	T_INT,
	T_SHORT,
	T_SHORT15,
	T_BYTE,
	T_STRING
} deltaStructTypes_enum;

#define CAST_COORD(ptr) (*(float *)(ptr))
#define CAST_VEC(ptr) (*(float *)(ptr))
#define CAST_FLOAT(ptr) (*(float *)(ptr))
#define CAST_INT(ptr) (*(int *)(ptr))
#define CAST_SHORT(ptr) (*(short *)(ptr))
#define CAST_BYTE(ptr) (*(byte *)(ptr))
#define CAST_STRING(ptr) ((char *)(ptr))

typedef struct
{
	int	type;
	int	offset;
} deltaUpdateStruct_t;

//increase this field to specify more fields in a deltaUpdateStruct
#define MAX_UPDATE_FLAGS	17

typedef struct
{
	byte updateflags[MAX_UPDATE_FLAGS];
} structDiff_t;


extern _net_driver_t	_netdrivers[MAX_NETDRIVERS];
extern int				_num_netdrivers;
extern deltaUpdateStruct_t baseObjectDesc[];
extern deltaUpdateStruct_t playerStateDesc[];
extern deltaUpdateStruct_t netClientInfoDesc[];
extern deltaUpdateStruct_t inputStateDesc[];

//shared ports
extern	netconnection_t	ncLocalClient;
extern	netconnection_t	ncLocalBrowser;
extern	netconnection_t	ncMasterServer;
extern	netconnection_t	ncListen;

extern	char			net_localaddr[256];
extern  struct in_addr	net_localip;

void    Net_RegisterVars();
void	Net_Init();
void	Net_ShutDown();

bool	Net_InitNetConnection(netconnection_t *nc, int driver, int port);

void	Net_Connect(char *addr);
void	Net_Disconnect();
char	*Net_GetDenyReason();

bool    Net_GetNewSocket(netconnection_t *nc);
int		Net_ReceivePacket(netconnection_t *nc);
bool	Net_PreProcessPacket(netconnection_t *nc);
bool	Net_SendPacket(netconnection_t *nc);

bool	Net_SendReliablePacket(netconnection_t *nc);

void	Net_CloseNetConnection(netconnection_t *nc);

bool	Net_OpenListenPort(netconnection_t *nc, int port);
bool	Net_SetSendAddr(netconnection_t *nc, char *addr, int clientport);
void	Net_Converse(netconnection_t *nc);
void	Net_CopyNetConnection(netconnection_t *from, netconnection_t *to);

void    Net_ClearReliablePackets(netconnection_t *nc);
bool    Net_BroadcastPacket(netconnection_t *nc, int port);

int		Net_GetPortFromAddress(char *addr);

void	Net_CheckPacketTimeouts(netconnection_t *nc);

bool	Net_IsSameAddress(netaddr_t *addr1, netaddr_t *addr2);
bool	Net_IsSameIP(netaddr_t *addr1, netaddr_t *addr2);

void	Net_RegisterDeltaStruct(deltaUpdateStruct_t desc[]);
int		Net_ReadStructDiff(packet_t *pkt, void *newstruct, deltaUpdateStruct_t desc[]);
void	Net_WriteStructDiff(packet_t *pkt, void *newstruct, deltaUpdateStruct_t desc[], structDiff_t *diff);
int		Net_GetStructDiff(void *oldstruct, void *newstruct, deltaUpdateStruct_t desc[], structDiff_t *diff);
void	Net_CopyDeltaStruct(void *dst, void *src, deltaUpdateStruct_t desc[]);

bool    Net_AddAcceptableCookie(char *cookie, unsigned int clan, unsigned int user_id, unsigned int guid);
bool    Net_ClientCookie_OK(char *cookie, unsigned int *clan, unsigned int *user_id, unsigned int *guid);

void	Net_StatsFrame();
void	Net_PrintStats();

#ifdef _WIN32
char		*WSock_GetErrorString(int err);
#endif

typedef struct
{
	int clBytesInPerSecond;
	int clBytesOutPerSecond;
	int clBytesInAccum;
	int clBytesOutAccum;
	int clWastedBytesIn;
	int clWastedBytesInPerSecond;
	int clUnordered;
	int svBytesInPerSecond;
	int svBytesOutPerSecond;
	int svBytesInAccum;
	int svBytesOutAccum;
} netStats_t;

extern netStats_t netStats;
