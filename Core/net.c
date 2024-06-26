// (C) 2003 S2 Games

// net.c

// network interface layer


#include "core.h"

static bool net_initialized = false;

int client_id;

netStats_t netStats;

netconnection_t	ncListen;			//port for accepting new connections
netconnection_t	ncLocalClient;		//port for handling data from the local client to the server
netconnection_t	ncLocalBrowser;		//port for requesting game info from servers
netconnection_t ncMasterServer;		//port for handling OUTGOING data to the master server

cvar_t	net_reliablePackets = { "net_reliablePackets", "1" };
cvar_t	net_forcedPacketDrop = { "net_forcedPacketDrop", "0" };

cvar_t	net_forceIP = {"net_forceIP", "0.0.0.0" };

cvar_t	net_showStats = { "net_showStats", "0" };

cvar_t	net_debug = { "net_debug", "0" };
cvar_t	net_connectTimeout = { "net_connectTimeout", "60000" };
cvar_t	net_problemIconTime = { "net_problemIconTime", "2000" };
cvar_t	net_connectionProblemTimeThreshhold = {"net_connectionProblemTimeThreshhold", "1000" };

/****  Connection Protocol info  ****


	To connect:

	Client sends:
		CPROTO_CONNECT_ME
			"S2Connect"			//connection string
			CPROTO_VERSION		//connection protocol version
	Server responds with:
		CPROTO_OK				//accept connection
			ushort dataport		//port client should now be talking with
		or
		CPROTO_NO				//reject connection
			char msg[32]		//reason for rejection

	If CPROTO_OK, Client responds with:
		
			server's last packet ID
	else
	    


	
 ************************************/


#define	CPROTO_VERSION		1

//client commands
#define	CPROTO_ACK					0xC1		//general acknowledgement
#define	CPROTO_CONNECT_ME			0xC2

//server commands
#define	CPROTO_OK			0xC3
#define	CPROTO_NO			0xC4

//keyserver commands
#define CPROTO_SEND_HEARTBEAT	0xC5

char net_localaddr[256];
struct in_addr net_localip;

extern cvar_t svr_maxclients;
extern cvar_t svr_firewalled;

typedef struct
{
	bool active;
	char cookie[COOKIE_SIZE+1];
	int  clan;
	int  user_id;
	int  guid;
} clientCookie_t;

//if we're the server, the list of acceptable cookies
clientCookie_t acceptable_cookies[MAX_CLIENTS];

bool	Net_InitNetConnection(netconnection_t *nc, int driver, int port)
{
	memset(nc, 0, sizeof(netconnection_t));

	nc->driver = driver;

	return _netdrivers[driver].OpenPort(nc, port);
}

//important side effect - if it matches, it erases it from the acceptable_cookies array!
bool	Net_ClientCookie_OK(char *cookie, unsigned int *clan, unsigned int *user_id, unsigned int *guid)
{
	int i = 0;

	if (svr_firewalled.integer)
		return true;
	
	while (i < MAX_CLIENTS)
	{
		if (acceptable_cookies[i].active && strncmp(acceptable_cookies[i].cookie, cookie, COOKIE_SIZE) == 0)
		{
			acceptable_cookies[i].active = false;
			*clan = acceptable_cookies[i].clan;
			*user_id = acceptable_cookies[i].user_id;
			*guid = acceptable_cookies[i].guid;
			return true;
		}
		i++;
	}
//#ifdef SAVAGE_DEMO
	return true;
//#else
//	return false;
//#endif
}

bool	Net_AddAcceptableCookie(char *cookie, unsigned int clan, unsigned int user_id, unsigned int guid)
{
	int i = 0;

	while (true)
	{
		if (!acceptable_cookies[i].active || i >= MAX_CLIENTS)
		{
			//this is probably an old slot that the person failed to connect with
			if (i >= MAX_CLIENTS)
			{
				i = 0;
			}
			strncpy(acceptable_cookies[i].cookie, cookie, COOKIE_SIZE);
			acceptable_cookies[i].clan = clan;
			acceptable_cookies[i].guid = guid;
			acceptable_cookies[i].user_id = user_id;
			acceptable_cookies[i].active = true;
			//each string is acceptable_cookies has a NULL at the [COOKIE_SIZE] pos, so no worries there
			return true;
		}
		i++;
	}
	return false;
}

void	Net_RegisterDeltaStructs();

void	Net_RegisterVars()
{
	Cvar_Register(&net_reliablePackets);
	Cvar_Register(&net_forcedPacketDrop);
	Cvar_Register(&net_forceIP);
	Cvar_Register(&net_debug);
	Cvar_Register(&net_connectTimeout);
	Cvar_Register(&net_problemIconTime);
	Cvar_Register(&net_connectionProblemTimeThreshhold);
}

void	Net_Init()
{
	int n;

	for (n=0; n<_num_netdrivers; n++)
	{
		_netdrivers[n].Init();		
	}		

	Net_RegisterDeltaStructs();

	memset(&ncListen, 0, sizeof(netconnection_t));
	memset(&ncLocalClient, 0, sizeof(netconnection_t));
	memset(&ncLocalBrowser, 0, sizeof(netconnection_t));
	memset(&ncMasterServer, 0, sizeof(netconnection_t));	

#ifndef DEDICATED_SERVER
	if (!dedicated_server.integer)
	{
		Net_InitNetConnection(&ncLocalBrowser, 0, 0);
	}
#endif

	Net_InitNetConnection(&ncMasterServer, 0, 0);

	for (n=0; n < MAX_CLIENTS; n++)
	{
		acceptable_cookies[n].active = false; //make them empty strings
		acceptable_cookies[n].cookie[COOKIE_SIZE] = 0; //make sure they're terminated for when we fill them in
	}

	//get a random number for our unique id
	client_id = System_GetTicks() & 0xfffe;
	
	Cvar_Register(&net_showStats);

	net_initialized = true;
}

void	Net_ShutDown()
{
	int n;

	if (!net_initialized)
		return;

	Net_CloseNetConnection(&ncLocalClient);
	Net_CloseNetConnection(&ncLocalBrowser);
	Net_CloseNetConnection(&ncMasterServer);
	Net_CloseNetConnection(&ncListen);
	
	for (n=0; n<_num_netdrivers; n++)
	{
		_netdrivers[n].ShutDown();
	}
}

void    Net_NewSocket(netconnection_t *nc)
{
	_netdrivers[nc->driver].NewSocket(nc);
}

void Client_StopPlayingDemo();

extern cvar_t demo_speedTest;

int		Net_ReceivePacket(netconnection_t *nc)
{
	int ret;
	OVERHEAD_INIT;

	if (demo.playing && nc == &ncLocalClient)
	{
		int cmdstart = File_Tell(demo.file);

		byte democmd = File_ReadByte(demo.file);		

		switch(democmd)
		{
			case DEMOCMD_PACKET:
			{				
				bool delay = false;
				int count,len,packettime;

				count = File_ReadInt(demo.file);				
				packettime = File_ReadInt(demo.file);

				if (demo.time < packettime)
					delay = true;

				if (delay)
				{
					//rewind to the start of the packet command
					File_Seek(demo.file, cmdstart, SEEK_SET);
					return 0;
				}

				len = File_ReadInt(demo.file);

				if (len < 0 || len >= MAX_PACKET_SIZE)
				{					
					Game_Error("Invalid demo packet length");
				}
				
				File_Read(nc->recv.__buf, len, 1, demo.file);
				nc->recv.length = len - HEADER_SIZE;
				nc->recv.curpos = 0;
				return len;
			}
			case DEMOCMD_EOF:
			{
				int starttime = demo.startTime;

				if (demo_speedTest.integer)
				{
					float timecompleted = (System_Milliseconds() - starttime) / 1000.0;

					Game_Error(fmt("Completed in %.2f seconds\nRendered %i frames\nAverage FPS: %.2f\n", 
						timecompleted,
						demo.framecount,
						(demo.framecount / timecompleted)));
				}
				else
				{
					Game_Error("End of demo\n");
				}
				break;
			}
			default:
				Game_Error("Invalid demo command\n");
				break;
		}

		return 0;
	}

	if (!net_initialized)
		return 0;

	ret = _netdrivers[nc->driver].ReceivePacket(nc);

	OVERHEAD_COUNT(OVERHEAD_NETWORK);
	return ret;

#if 0
	if (lastnc != nc)
	{
		lastpos = -1;
		lastnc = nc;
		return _netdrivers[nc->driver].ReceivePacket(nc);
	}

	if (nc->recv.curpos < nc->recv.length)  //not done reading previous packet
	{
		/*if (lastpos == nc->recv.curpos && nc == lastnc)  //must not have received expected packet data, since we're reading the same packet twice in a row
		{
			Console_Printf("Net_ReceivePacket: illegal packet data from %s\n", nc->recvAddr);
			Pkt_Clear(&nc->send);
			lastpos = -1;
			lastnc = NULL;
			return _netdrivers[nc->driver].ReceivePacket(nc);
		}*/
		lastpos = nc->recv.curpos;
		lastnc = nc;
		return true;	//still dealing with current packet
	}

	return _netdrivers[nc->driver].ReceivePacket(nc);
#endif
}

/*=======================

  Net_PreProcessPacket

  filter out bad packets and ACKs

 =======================*/

bool	Net_PreProcessPacket(netconnection_t *nc)
{
	int ret;
	OVERHEAD_INIT;

	if (demo.playing && nc == &ncLocalClient)
		return true;

	ret = _netdrivers[nc->driver].PreProcessPacket(nc);
	OVERHEAD_COUNT(OVERHEAD_NETWORK);
	return ret;
}

void	Net_WriteClientID(netconnection_t *nc, packet_t *pkt)
{
	packet_t copy;
	if (nc == &ncLocalClient || nc == &ncLocalBrowser)
	{
		//write a unique identifier at the top of the packet to fix routers changing our udp port.
		//this code sucks.
		Pkt_Clear(&copy);
		Pkt_WriteShort(&copy, nc == &ncLocalBrowser ? 0xffff : client_id);
		Pkt_Write(&copy, &pkt->__buf[HEADER_SIZE], pkt->length);
		Pkt_Copy(&copy, pkt);
	}
}


bool	Net_SendPacket(netconnection_t *nc)
{
	bool ret;
	OVERHEAD_INIT;

	if (!net_initialized)
		return false;

	if (nc == &ncLocalClient && demo.playing)
		return true;

	Net_WriteClientID(nc, &nc->send);

	ret = _netdrivers[nc->driver].SendPacket(nc);

	if (nc->send.length + HEADER_SIZE < ret)
		Console_DPrintf("WARNING: tried to write %i bytes to the socket, but only %i bytes were sent\n", nc->send.length+HEADER_SIZE, ret);
	Pkt_Clear(&nc->send);
	
	OVERHEAD_COUNT(OVERHEAD_NETWORK);
	return ret;
}


bool	Net_SendReliablePacket(netconnection_t *nc)
{
	bool ret;
	OVERHEAD_INIT;

	if (!net_initialized)
		return false;

	if (nc == &ncLocalBrowser)
	{
		System_Error("Don't send reliable packets through the ncLocalBrowser connection!!");
	}

	Net_WriteClientID(nc, &nc->reliableSend);

	ret = _netdrivers[nc->driver].SendReliablePacket(nc);

	Pkt_Clear(&nc->reliableSend);
	
	OVERHEAD_COUNT(OVERHEAD_NETWORK);
	return ret;
}

void	Net_CheckPacketTimeouts(netconnection_t *nc)
{
	OVERHEAD_INIT;
	if (!net_initialized)
		return;

	_netdrivers[(&ncLocalClient)->driver].CheckPacketTimeouts(nc);
	OVERHEAD_COUNT(OVERHEAD_NETWORK);
}



void	Net_CloseNetConnection(netconnection_t *nc)
{
	if (!net_initialized)
		return;

	_netdrivers[nc->driver].CloseNetConnection(nc);

	memset(nc, 0, sizeof(netconnection_t));
}

//server functions
bool	Net_OpenListenPort(netconnection_t *nc, int port)
{
	if (!net_initialized)
		return false;

	Console_DPrintf("Net_OpenListenPort()\n");

	Net_CloseNetConnection(nc);

	if (!_netdrivers[localServer.netdriver].OpenPort(nc, port))	
	{
		Console_DPrintf("_netdrivers[%i].OpenPort(%i) failed\n", localServer.netdriver, port);
		return false;
	}

	return true;
}

bool	Net_SetSendAddr(netconnection_t *nc, char *addr, int clientport)
{
	if (!net_initialized)
		return false;

	return _netdrivers[nc->driver].SetSendAddr(nc, addr, clientport);
}

bool    Net_BroadcastPacket(netconnection_t *nc, int port)
{
	if (!net_initialized)
		return false;

	Net_WriteClientID(nc, &nc->send);

	return _netdrivers[nc->driver].BroadcastPacket(nc, port);
}
void    Net_ClearReliablePackets(netconnection_t *nc)
{
	if (!net_initialized)
		return;

	_netdrivers[nc->driver].ClearReliablePackets(nc);
}

void	Net_Converse(netconnection_t *nc)
{
	if (!net_initialized)
		return;

	_netdrivers[nc->driver].Converse(nc);
}

void	Net_CopyNetConnection(netconnection_t *from, netconnection_t *to)
{
	Mem_Copy(to, from, sizeof(netconnection_t));
}





int		Net_GetPortFromAddress(char *addr)
{
	char *s = addr;
	char port[6];
	int n = 0;

	while(*s)
	{
		if (*s==':' || *s==' ')
		{
			s++;
			goto extractPort;
		}
		s++;
	}

	return DEFAULT_SERVER_PORT;

extractPort:
	
	while (*s && n<6)
	{
		port[n] = *s;
		n++; s++;
	}

	return atoi(s);
}

bool	Net_IsSameAddress(netaddr_t *addr1, netaddr_t *addr2)
{
	if (memcmp(addr1, addr2, sizeof(netaddr_t) - 8)==0)		//hack: subtract 8 bytes from the comparison as this is the sin_zero field of sockaddr_in
		return true;

	return false;
}

bool	Net_IsSameIP(netaddr_t *addr1, netaddr_t *addr2)
{
	if (memcmp(&addr1->sin_addr, &addr2->sin_addr, sizeof(addr1->sin_addr))==0)
		return true;

	return false;
}


int Net_ReadStructDiff(packet_t *pkt, void *newstruct, deltaUpdateStruct_t desc[])
{
	structDiff_t diff;
	deltaUpdateStruct_t *field = desc;
	int i = 0;
	int lastupdate;
	int length = 0;

	//read in the update flags
	for (i=0; i<MAX_UPDATE_FLAGS; i++)
	{
		diff.updateflags[i] = Pkt_ReadByte(pkt);
		if (diff.updateflags[i] & 0x80)
			continue;
		else
			break;
	}
	length = pkt->curpos;

	lastupdate = i;

	i = 0;

	while(field->type)
	{
		div_t d;
		int flagnum;

		d = div(i, 7);
		flagnum = d.quot;
		if (flagnum > lastupdate)
			break;

		if (diff.updateflags[flagnum] & (1 << d.rem))
		{
			switch(field->type)
			{
				case T_COORD:
					CAST_FLOAT((char *)newstruct + field->offset) = Pkt_ReadCoord(pkt);
					break;
				case T_FLOAT:
					CAST_FLOAT((char *)newstruct + field->offset) = Pkt_ReadFloat(pkt);
					break;
				case T_BYTE_ANGLE:
					CAST_FLOAT((char *)newstruct + field->offset) = Pkt_ReadByteAngle(pkt);
					break;
				case T_WORD_ANGLE:
					CAST_FLOAT((char *)newstruct + field->offset) = Pkt_ReadWordAngle(pkt);
					break;
				case T_INT:
					CAST_INT((char *)newstruct + field->offset) = Pkt_ReadInt(pkt);
					break;
				case T_SHORT:
					CAST_SHORT((char *)newstruct + field->offset) = Pkt_ReadShort(pkt);
					break;
				case T_BYTE:
					CAST_BYTE((char *)newstruct + field->offset) = Pkt_ReadByte(pkt);
					break;
				case T_STRING:				
					Pkt_ReadString(pkt, ((char *)newstruct + field->offset), 255);
					break;
				case T_SHORT15:
				{
					short b1,b2;

					b1 = Pkt_ReadByte(pkt);
					if (b1 & 0x80)
					{					
						//short transmitted in big endian order
						b1 &= ~0x80;
						b2 = Pkt_ReadByte(pkt);


						CAST_SHORT((char *)newstruct + field->offset) = ((b1 << 8) + b2) & 0xffff;
					}
					else
					{
						CAST_SHORT((char *)newstruct + field->offset) = b1;
					}
					
					break;
				}
			}
		}

		i++;
		field++;
	}

	length = pkt->curpos - length;
	//Console_DPrintf("object diff was %i bytes\n", length);
	return length;
}

void	Net_WriteStructDiff(packet_t *pkt, void *newstruct, deltaUpdateStruct_t desc[], structDiff_t *diff)
{
	deltaUpdateStruct_t *field = desc;
	int i;
	int lastupdate;
//	static structDiff_t updateall;

	//write the update flags
	for (i=0; i<MAX_UPDATE_FLAGS; i++)
	{
		Pkt_WriteByte(pkt, diff->updateflags[i]);
		if (diff->updateflags[i] & 0x80)
			continue;
		else
			break;
	}

	lastupdate = i;

	i = 0;

	while (field->type)
	{
		div_t d;
		int flagnum;

		d = div(i,7);
		flagnum = d.quot;
		if (flagnum > lastupdate)
			break;

	
		if (diff->updateflags[flagnum] & (1 << d.rem))
		{
			switch(field->type)
			{
				case T_COORD:
					Pkt_WriteCoord(pkt, CAST_FLOAT((char *)newstruct + field->offset));
					break;
				case T_FLOAT:
					Pkt_WriteFloat(pkt, CAST_FLOAT((char *)newstruct + field->offset));
					break;
				case T_BYTE_ANGLE:
					Pkt_WriteByteAngle(pkt, CAST_FLOAT((char *)newstruct + field->offset));
					break;
				case T_WORD_ANGLE:
					Pkt_WriteWordAngle(pkt, CAST_FLOAT((char *)newstruct + field->offset));
					break;
				case T_INT:
					Pkt_WriteInt(pkt, CAST_INT((char *)newstruct + field->offset));
					break;
				case T_SHORT:
					Pkt_WriteShort(pkt, CAST_SHORT((char *)newstruct + field->offset));
					break;
				case T_SHORT15:
				{
					byte hi,lo;
					unsigned short s = CAST_SHORT((char *)newstruct + field->offset);					

					if (s > 127)
					{
						hi = (s & 0xff00) >> 8;
						hi |= 0x80;
						lo = s & 0x00ff;					
						//transmit in big endian order, with the highest bit set on the high byte
						Pkt_WriteByte(pkt, hi);
						Pkt_WriteByte(pkt, lo);
					}
					else
					{
						Pkt_WriteByte(pkt, (byte)s);
					}

					break;
				}				
				case T_BYTE:
					Pkt_WriteByte(pkt, CAST_BYTE((char *)newstruct + field->offset));
					break;
				case T_STRING:
					Pkt_WriteString(pkt, ((char *)newstruct + field->offset));
					break;
			}
		}
	

		i++;
		field++;
	}
}

void	Net_RegisterDeltaStruct(deltaUpdateStruct_t desc[])
{
	//i could see this doing more, but right now we just use it to count the number of fields in a struct to make sure there won't be any access violations
	deltaUpdateStruct_t *field = desc;
	int i=0;

	while (field->type)
	{
		i++;
		field++;
	}

	if (i > MAX_UPDATE_FLAGS * 7)
		System_Error("Delta update struct contains too many fields, either reduce the fields or change the code in net.c\n");
}

void	Net_CopyDeltaStruct(void *dst, void *src, deltaUpdateStruct_t desc[])
{
	deltaUpdateStruct_t *field = desc;

	while (field->type)
	{
		switch(field->type)
		{
			case T_COORD:
				CAST_FLOAT((char *)dst + field->offset) = CAST_FLOAT((char *)src + field->offset);
				break;
			case T_FLOAT:
				CAST_FLOAT((char *)dst + field->offset) = CAST_FLOAT((char *)src + field->offset);
				break;
			case T_BYTE_ANGLE:
				CAST_FLOAT((char *)dst + field->offset) = CAST_FLOAT((char *)src + field->offset);
				break;
			case T_WORD_ANGLE:
				CAST_FLOAT((char *)dst + field->offset) = CAST_FLOAT((char *)src + field->offset);
				break;
			case T_INT:
				CAST_INT((char *)dst + field->offset) = CAST_INT((char *)src + field->offset);
				break;
			case T_SHORT:
			case T_SHORT15:
				CAST_SHORT((char *)dst + field->offset) = CAST_SHORT((char *)src + field->offset);
				break;				
			case T_BYTE:
				CAST_BYTE((char *)dst + field->offset) = CAST_BYTE((char *)src + field->offset);
				break;
			case T_STRING:		
				memcpy((char *)dst + field->offset, (char *)src + field->offset, 255);
				break;				
		}

		field++;
	}
}

int		Net_FieldSize(int type)
{
	switch(type)
	{
		case T_COORD:
			return 2;
		case T_FLOAT:
			return 4;
		case T_BYTE_ANGLE:			//low precision angle (0.0 to (360.0 - 1/255) maps to 0 to 255)
			return 1;
		case T_WORD_ANGLE:			//high precision angle (0.0 to (360.0 - 1/65535) maps to 0 to 65535)
			return 2;
		case T_INT:
			return 4;
		case T_SHORT:
			return 2;
		case T_BYTE:
			return 1;
		case T_STRING:
			Game_Error("Net_FieldSize() on string");
		case T_SHORT15:
			Game_Error("Net_FieldSize() on short15");
		default:
			return 1;
	}
}

//fills up a packet with up to 3 bytes with update flags describing which fields are being updated, followed by the struct data
//in the update flags
int		Net_GetStructDiff(void *oldstruct, void *newstruct, deltaUpdateStruct_t desc[], structDiff_t *diff)
{
	int i = 0;
	int flagnum;
	div_t d;
	deltaUpdateStruct_t *field = desc;
	bool changed = false;

	memset(diff, 0, sizeof(structDiff_t));

	if (!oldstruct)
	{
		while (field->type)
		{
			d = div(i, 7);
			flagnum = d.quot;

			diff->updateflags[flagnum] |= 1 << d.rem;

			i++;
			field++;
			changed += Net_FieldSize(field->type);
		}
	}
	else
	{
		while(field->type)
		{
			d = div(i, 7);
			flagnum = d.quot;

			switch (field->type)
			{
				case T_COORD:
					if (COORD2SHORT(CAST_FLOAT((char *)oldstruct + field->offset)) != COORD2SHORT(CAST_FLOAT((char *)newstruct + field->offset)))
					{
						diff->updateflags[flagnum] |= 1 << d.rem;
						changed += Net_FieldSize(T_COORD);
					}
					break;
				case T_FLOAT:
					if (CAST_FLOAT((char *)oldstruct + field->offset) != CAST_FLOAT((char *)newstruct + field->offset))
					{
						diff->updateflags[flagnum] |= 1 << d.rem;
						changed += Net_FieldSize(T_FLOAT);
					}
					break;
				case T_BYTE_ANGLE:
					if (ANGLE2BYTE(CAST_FLOAT((char *)oldstruct + field->offset)) != ANGLE2BYTE(CAST_FLOAT((char *)newstruct + field->offset)))
					{
						changed += Net_FieldSize(T_BYTE_ANGLE);
						diff->updateflags[flagnum] |= 1 << d.rem;
					}
					break;
				case T_WORD_ANGLE:
					if (ANGLE2WORD(CAST_FLOAT((char *)oldstruct + field->offset)) != ANGLE2WORD(CAST_FLOAT((char *)newstruct + field->offset)))
					{
						changed += Net_FieldSize(T_WORD_ANGLE);
						diff->updateflags[flagnum] |= 1 << d.rem;
					}
					break;
				case T_INT:
					if (CAST_INT((char *)oldstruct + field->offset) != CAST_INT((char *)newstruct + field->offset))
					{
						changed += Net_FieldSize(T_INT);
						diff->updateflags[flagnum] |= 1 << d.rem;
					}
					break;
				case T_SHORT:
					if (CAST_SHORT((char *)oldstruct + field->offset) != CAST_SHORT((char *)newstruct + field->offset))
					{
						changed += Net_FieldSize(T_SHORT);
						diff->updateflags[flagnum] |= 1 << d.rem;
					}
					break;
				case T_SHORT15:
				{
					short s = CAST_SHORT((char *)newstruct + field->offset);
					if (CAST_SHORT((char *)oldstruct + field->offset) != s)
					{
						if (s > 127)
							changed += 2;
						else
							changed += 1;
						diff->updateflags[flagnum] |= 1 << d.rem;
					}
					break;
				}
				case T_BYTE:
					if (CAST_BYTE((char *)oldstruct + field->offset) != CAST_BYTE((char *)newstruct + field->offset))
					{
						changed += Net_FieldSize(T_BYTE);
						diff->updateflags[flagnum] |= 1 << d.rem;
					}
					break;
				case T_STRING:
					if (strcmp((char *)oldstruct + field->offset, (char *)newstruct + field->offset)!=0)
					{
						changed += strlen((char *)oldstruct + field->offset) + 1;
						diff->updateflags[flagnum] |= 1 << d.rem;
					}
					break;
			}

			i++;
			field++;
		}

	}

	for (i=1; i<MAX_UPDATE_FLAGS; i++)
	{
		int f;

		if (diff->updateflags[i])
			for (f=0; f<i; f++)
				diff->updateflags[f] |= 0x80;		//set the "more bits" bit
	}

	return changed;
}



void	Net_StatsFrame()
{
	static int nextBandwidthCalc = 0;

	if (!net_showStats.integer)
		return;

	if (Host_Milliseconds() >= nextBandwidthCalc)
	{
		nextBandwidthCalc = Host_Milliseconds() + 1000;

		netStats.clBytesInPerSecond = netStats.clBytesInAccum;
		netStats.clBytesInAccum = 0;
		netStats.clBytesOutPerSecond = netStats.clBytesOutAccum;
		netStats.clBytesOutAccum = 0;
		netStats.clWastedBytesInPerSecond = netStats.clWastedBytesIn;
		netStats.clWastedBytesIn = 0;
		
		netStats.svBytesInPerSecond = netStats.svBytesInAccum;
		netStats.svBytesInAccum = 0;		
		netStats.svBytesOutPerSecond = netStats.svBytesOutAccum;
		netStats.svBytesOutAccum = 0;		
	}
	
	Net_PrintStats();
}

void	Net_PrintStats()
{	
	Host_PrintOverhead("============================\n");
	Host_PrintOverhead("Net stats\n\n");

	if (localClient.cstate)
	{
		Host_PrintOverhead("CLIENT\n");
		Host_PrintOverhead("  Bytes in / sec:       %i\n", netStats.clBytesInPerSecond);
		Host_PrintOverhead("  Bytes out / sec:      %i\n", netStats.clBytesOutPerSecond);
		Host_PrintOverhead("  Bytes total / sec:    %i\n", netStats.clBytesInPerSecond + netStats.clBytesOutPerSecond);
		Host_PrintOverhead("  Out of order packets: %i\n\n", netStats.clUnordered);
		//Host_PrintOverhead("  Total wasted bytes due to server diff bug / sec:    %i\n\n", netStats.clWastedBytesInPerSecond);
	}

	if (localServer.active)
	{
		Host_PrintOverhead("SERVER\n");
		Host_PrintOverhead("  Bytes in / sec:       %i\n", netStats.svBytesInPerSecond);
		Host_PrintOverhead("  Bytes out / sec:      %i\n", netStats.svBytesOutPerSecond);
		Host_PrintOverhead("  Bytes total / sec:    %i\n\n", netStats.svBytesInPerSecond + netStats.svBytesOutPerSecond);		
	}

	if (localClient.cstate && localServer.active)
	{
		Host_PrintOverhead("TOTAL\n");
		Host_PrintOverhead("  Bytes in / sec:       %i\n", netStats.svBytesInPerSecond + netStats.clBytesInPerSecond);
		Host_PrintOverhead("  Bytes out / sec:      %i\n", netStats.svBytesOutPerSecond + netStats.clBytesOutPerSecond);
		Host_PrintOverhead("  Bytes total / sec:    %i\n", netStats.svBytesInPerSecond + netStats.svBytesOutPerSecond +
														 netStats.clBytesInPerSecond + netStats.clBytesOutPerSecond);
	}
	
	Host_PrintOverhead("\n");
}
