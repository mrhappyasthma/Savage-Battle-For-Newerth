// (C) 2003 S2 Games

// net_winsock.c

// winsock TCP/UDP driver

#include "core.h"

typedef int socklen_t;

typedef struct
{	
	WSADATA wsaData;
	unsigned int		nonblock;
} winsockServer_t;

winsockServer_t ws;

extern cvar_t net_reliablePackets;
extern cvar_t net_forcedPacketDrop;

extern cvar_t net_forceIP;

extern cvar_t net_debug;

#define NET_PRINTF if (net_debug.integer) Console_Printf

/*
static winsockDriverData_t wsdata[64];

winsockDriverData_t *WSock_AllocateDriverData()
{
	int n;

	for (n=0; n<64; n++)
	{
		if (!wsdata[n].active)
		{
			wsdata[n].active = true;
			return &wsdata[n];
		}
	}

	return NULL;
}
*/

bool	_WSock_SendPacket(SOCKET sock, struct sockaddr *sendaddr, unsigned char *buf, int buf_length, unsigned int lastseq, byte flags);

void	WSock_Init()
{
	int n;
	struct hostent *phe;
	int bestip = 0;

	Console_DPrintf("WSock_Init\n");

	if (strcmp(net_forceIP.string, "0.0.0.0") != 0)
	{
		unsigned long tmp;

		tmp = inet_addr(net_forceIP.string);
		Mem_Copy(&net_localip, (void *)&tmp, sizeof(struct in_addr));
		strcpy(net_localaddr, inet_ntoa(net_localip));
	}
	else
	{
		if (WSAStartup(0x202, &ws.wsaData) == SOCKET_ERROR)
		{
			WSACleanup();
			Console_Errorf("WSAStartup failed (%s)\n", WSAGetLastError());		
			return;
		}
	
		if (gethostname(net_localaddr, 256) == SOCKET_ERROR)
		{
			WSACleanup();
			Console_Errorf("gethostname() failed (%s)\n", WSAGetLastError());
			return;
		}
	
		phe = gethostbyname(net_localaddr);
		
	    for (n = 0; phe->h_addr_list[n] != 0; n++)
		{   
			char *ipstring;
	
	        Mem_Copy(&net_localip, phe->h_addr_list[n], sizeof(struct in_addr));
			ipstring = inet_ntoa(net_localip);
			Console_DPrintf("Local IP #%i: %s\n", n+1, ipstring);
	
			//fixme: this is a hack in case we're shared off the connection 
			//to the internet.  this won't work all the time so find a better way		
			if (strncmp(ipstring, "192.168.", 8)!=0)
				bestip = n;
	    }
	
	    Mem_Copy(&net_localip, phe->h_addr_list[bestip], sizeof(struct in_addr));
	    strcpy(net_localaddr, inet_ntoa(net_localip));
	}

//	ncLocalClient.driverData = WSock_AllocateDriverData();

	ws.nonblock = 1;
}

void	WSock_ShutDown()
{
	Console_DPrintf("WSock_ShutDown()\n");
	WSACleanup();	
}

bool	WSock_SetSendAddr(netconnection_t *nc, char *addr, int clientport)
{	
	struct hostent *hp;
	char *colon;

	if (!addr)  //just change the port
	{
		nc->sendAddr.sin_port = htons((unsigned short)clientport);
		return true;
	}

	//get rid of the colon used to specify a port
	colon = strchr(addr, ':');
	if (colon)
	{
		if (strlen(addr) > (unsigned)(colon-addr))
			clientport = atoi(colon+1);
		*colon = 0;		
	}

	if (isdigit(addr[strlen(addr)-1]))
	{
		unsigned int uint_addr;
		//hp = gethostbyaddr(inet_addr(addr), 4, AF_INET);
		memset(&nc->sendAddr,0,sizeof(struct sockaddr_in));
		uint_addr = inet_addr(addr);
		Mem_Copy(&nc->sendAddr.sin_addr, &uint_addr, 4);
		nc->sendAddr.sin_family = AF_INET;
	}
	else
	{
		hp = gethostbyname(addr);

		if (!hp)
		{
			int error = 0;
		
			#ifdef _WIN32
			error = WSAGetLastError();
			#endif
			NET_PRINTF("WSock_SetSendAddr: Couldn't resolve %s (Error #%i)\n", addr, error);
			return false;
		}

		memset(&nc->sendAddr,0,sizeof(struct sockaddr_in));
		Mem_Copy(&nc->sendAddr.sin_addr, hp->h_addr, hp->h_length);
		nc->sendAddr.sin_family = hp->h_addrtype;
	}

	if (!clientport)
	{
		clientport = Net_GetPortFromAddress(addr);
	}
	
	nc->sendAddr.sin_port = htons((unsigned short)clientport);

	strcpy(nc->sendAddrName, fmt("%s:%i", addr, clientport));

	return true;

}

void	WSock_CloseNetConnection(netconnection_t *nc)
{
	closesocket(nc->sock);		
}

bool	WSock_NewSocket(netconnection_t *nc)
{
	SOCKET sock;

	if (nc->sock)
	{
		WSock_CloseNetConnection(nc);
	}

	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); // UDP socket
	nc->sock = sock;

	if (sock == INVALID_SOCKET)
	{
		WSock_CloseNetConnection(nc);
		NET_PRINTF("socket() failure (%d)\n", WSAGetLastError());
		return false;
	}

	if (ioctlsocket(nc->sock, FIONBIO, &ws.nonblock) == SOCKET_ERROR)
	{
		WSock_CloseNetConnection(nc);
		NET_PRINTF("ioctlsocket() failure (%d)\n", WSAGetLastError());
		return false;
	}
	return true;
}

void	WSock_Converse(netconnection_t *nc)
{
	Mem_Copy(&nc->sendAddr, &nc->recvAddr, sizeof(struct sockaddr_in));
	strcpy(nc->sendAddrName, nc->recvAddrName);
}

//open a socket with the specified port
bool WSock_OpenPort(netconnection_t *nc, int port)
{
	unsigned long addr;
	NET_PRINTF("WSock_OpenPort(%i)\n", port);

	nc->addr.sin_family = AF_INET;
	if (strcmp(net_forceIP.string, "0.0.0.0") != 0)
    {
		addr = inet_addr(net_forceIP.string);
		memcpy(&nc->addr.sin_addr, &addr, sizeof(unsigned long));
	}
	else
	{
		nc->addr.sin_addr.s_addr = INADDR_ANY; 
	}
	nc->addr.sin_port = htons((unsigned short)port);

	if (!WSock_NewSocket(nc))
	{
		NET_PRINTF("Socket creation failed\n");
		return false;
	}

	if (bind(nc->sock, (void*)&(nc->addr), sizeof(struct sockaddr_in)) == SOCKET_ERROR) 
	{
		WSock_CloseNetConnection(nc);
		NET_PRINTF("bind() failed (%d)\n", WSAGetLastError());		
		return false;
	}

	if (!nc->port)
	{
		socklen_t namelen = sizeof(struct sockaddr_in);
		getsockname(nc->sock, (void*)(&nc->addr), &namelen);
		nc->port = ntohs(nc->addr.sin_port);
		Console_Printf("Port %i opened\n", nc->port);
	}

	return true;
}

extern int client_id;

bool	WSock_ProcessReliablePacket(netconnection_t *nc, unsigned char *buf, struct sockaddr_in *from)
{
	packet_t ackpacket;
	//static char ackbuf[HEADER_SIZE + sizeof(unsigned int)];
    unsigned int buf_seq;
    bool process = true;

    Mem_Copy(&buf_seq, buf, sizeof(unsigned int));
	buf_seq = LittleInt(buf_seq);
	NET_PRINTF("got reliable packet with sequence %u, waiting for %u\n", buf_seq, nc->reliableRecv_lastseq+1);

    if (buf_seq > nc->reliableRecv_lastseq+1)
    {
        //drop it, out of order - wait for the next packet before jumping ahead, to maintain transfer order
        //  don't even ACK it, so they think we never got it and resend it after resending the earlier one(s) we missed
		
        NET_PRINTF("Received out of order sequence %u, waiting for %u\n", buf_seq, nc->reliableRecv_lastseq+1);
        return false;
    }
    else if (buf_seq == nc->reliableRecv_lastseq+1)
    {
        //this is a new reliable packet that we've waiting for.  Increment the incoming sequence, and use the packet
        nc->reliableRecv_lastseq++;
		NET_PRINTF("reliable packet with sequence %u is valid, incrementing our lastseq to %u\n", buf_seq, nc->reliableRecv_lastseq);
    }
    else //buf_seq < nc->reliableRecv_lastseq+1
    {
        //this is a resend for a packet we've gotten already.  ACK it, but don't process it again
        NET_PRINTF("Received resend of reliable packet sequence %u, not using it\n", buf_seq);
        process = false;
    }
	
	//send an ACK for this reliable packet
	//HACK: write the client ID if we're a client
	Pkt_Clear(&ackpacket);
	if (nc == &ncLocalClient)
	{
		Pkt_WriteShort(&ackpacket, (short)client_id);
		Pkt_WriteInt(&ackpacket, buf_seq);
	}
	else
	{
		Pkt_WriteInt(&ackpacket, buf_seq);		
	}

	//hack to stress-test the reliable packet code
	if (!net_forcedPacketDrop.value || M_Randnum(0,1) <= 0.5)
	{
		NET_PRINTF("Sending ack for reliable packet sequence %u\n", buf_seq);
	 	_WSock_SendPacket(nc->sock, (struct sockaddr *)(&nc->sendAddr), ackpacket.__buf, ackpacket.length + HEADER_SIZE, NONRELIABLE_SEQUENCE, PACKET_NORMAL | PACKET_ACK);
	}
	
	return process;
}

void	WSock_ProcessPacketAck(netconnection_t *nc)
{
	reliablePacket_t *packet = nc->unack_packets;
	reliablePacket_t *tmp_packet = nc->unack_packets;
	unsigned int seq;

	seq = Pkt_ReadInt(&nc->recv);

	NET_PRINTF("Got ACK for sequence %u\n", seq);

	if (!packet)
	{
		NET_PRINTF("Got ACK for seq %u, but we have no unACK'd packets.\n", seq);
		return;
	}

    NET_PRINTF("Comparing seq %u with unack_packet_seq %u\n", seq, packet->seq);
    while (packet
            && packet->seq != seq)
    {
        packet = packet->next;
        if (packet)
            NET_PRINTF("Comparing seq %u with unack_packet_seq %u\n", seq, packet->seq);
    }
    if (!packet)
    {
        NET_PRINTF("Got ACK for non-existant sequence %u\n", seq);
        return;
    }
	
	NET_PRINTF("Successfully ACK'd sequence %u\n", seq);
	
    //now drop all unack packets with a sequence <= to this one
    //point tmp_packet to the old head of the list
    tmp_packet = nc->unack_packets;
    //meanwhile, point the new unack_packets to the first unACKed packet
    nc->unack_packets = packet->next;

    //now walk down tmp_packet and free them all
    while (tmp_packet
           && tmp_packet != nc->unack_packets)
    {
        packet = tmp_packet;

		if (seq != packet->seq)
            NET_PRINTF("They must have already gotten packet %u then, I'll assume they did.\n", packet->seq);

        tmp_packet = tmp_packet->next;
        Tag_Free(packet->packet);
        Tag_Free(packet);
    }
	if (nc->unack_packets)
		NET_PRINTF("The first unack'd packet # is now %u\n", nc->unack_packets->seq);
}

char	*WSock_GetErrorString(int err)
{
	switch (err)
	{
		case WSAEINTR: 
			return "WSAEINTR";
		case WSAEBADF: 
			return "WSAEBADF";
		case WSAEACCES: 
			return "WSAEACCES";
		case WSAEDISCON:
			return "WSAEDISCON";
		case WSAEFAULT: 
			return "WSAEFAULT";
		case WSAEINVAL:
			return "WSAEINVAL";
		case WSAEMFILE:
			return "WSAEMFILE";
		case WSAEWOULDBLOCK: 
			return "WSAEWOULDBLOCK";
		case WSAEINPROGRESS: 
			return "WSAEINPROGRESS";
		case WSAEALREADY: 
			return "WSAEALREADY";
		case WSAENOTSOCK: 
			return "WSAENOTSOCK";
		case WSAEDESTADDRREQ: 
			return "WSAEDESTADDRREQ";
		case WSAEMSGSIZE: 
			return "WSAEMSGSIZE";
		case WSAEPROTOTYPE: 
			return "WSAEPROTOTYPE";
		case WSAENOPROTOOPT: 
			return "WSAENOPROTOOPT";
		case WSAEPROTONOSUPPORT:
			return "WSAEPROTONOSUPPORT";
		case WSAESOCKTNOSUPPORT: 
			return "WSAESOCKTNOSUPPORT";
		case WSAEOPNOTSUPP: 
			return "WSAEOPNOTSUPP";
		case WSAEPFNOSUPPORT: 
			return "WSAEPFNOSUPPORT";
		case WSAEAFNOSUPPORT: 
			return "WSAEAFNOSUPPORT";
		case WSAEADDRINUSE: 
			return "WSAEADDRINUSE";
		case WSAEADDRNOTAVAIL: 
			return "WSAEADDRNOTAVAIL";
		case WSAENETDOWN: 
			return "WSAENETDOWN";
		case WSAENETUNREACH: 
			return "WSAENETUNREACH";
		case WSAENETRESET: 
			return "WSAENETRESET";
		case WSAECONNABORTED: 
			return "WSWSAECONNABORTEDAEINTR";
		case WSAECONNRESET: 
			return "WSAECONNRESET";
		case WSAENOBUFS: 
			return "WSAENOBUFS";
		case WSAEISCONN: 
			return "WSAEISCONN";
		case WSAENOTCONN: 
			return "WSAENOTCONN";
		case WSAESHUTDOWN: 
			return "WSAESHUTDOWN";
		case WSAETOOMANYREFS: 
			return "WSAETOOMANYREFS";
		case WSAETIMEDOUT: 
			return "WSAETIMEDOUT";
		case WSAECONNREFUSED: 
			return "WSAECONNREFUSED";
		case WSAELOOP: 
			return "WSAELOOP";
		case WSAENAMETOOLONG: 
			return "WSAENAMETOOLONG";
		case WSAEHOSTDOWN: 
			return "WSAEHOSTDOWN";
		case WSASYSNOTREADY: 
			return "WSASYSNOTREADY";
		case WSAVERNOTSUPPORTED: 
			return "WSAVERNOTSUPPORTED";
		case WSANOTINITIALISED: 
			return "WSANOTINITIALISED";
		case WSAHOST_NOT_FOUND: 
			return "WSAHOST_NOT_FOUND";
		case WSATRY_AGAIN: 
			return "WSATRY_AGAIN";
		case WSANO_RECOVERY: 
			return "WSANO_RECOVERY";
		case WSANO_DATA: 
			return "WSANO_DATA";
		default: 
			return "Unknown";
	}
}

int		WSock_ReceivePacket(netconnection_t *nc)
{
	int ret;
	socklen_t fromlen;
	netaddr_t from;
	
	fromlen = sizeof(struct sockaddr);

	ret = recvfrom(nc->sock, nc->recv.__buf, MAX_PACKET_SIZE + HEADER_SIZE, 0, (struct sockaddr *)&from, &fromlen);
		
	if (ret < 0)
	{
		int err = WSAGetLastError();
		if (err != WSAEWOULDBLOCK)
		{
			NET_PRINTF("recvfrom() failed: error %s from %s\n", WSock_GetErrorString(err), inet_ntoa(from.sin_addr));
		}
	}
	else if (ret > 0)
	{
	
#ifdef ANAL_NETWORK_DEBUGGING
		NET_PRINTF("Net: receiving %i bytes with seq %u from port %i - flags %s %s\n", nc->recv.length, seq,
									ntohs(from->sin_port),
									flags & PACKET_RELIABLE ? "RELIABLE" : "",
									flags & PACKET_ACK ? "ACK" : "");
#endif
		Mem_Copy(&nc->recvAddr, &from, sizeof(struct sockaddr_in));
		strncpy(nc->recvAddrName, inet_ntoa(from.sin_addr), MAX_ADDRESS_LENGTH);
		nc->recv.length = ret - HEADER_SIZE;
		nc->recv.curpos = 0;
	}

	return ret;
}

bool	WSock_PreProcessPacket(netconnection_t *nc)
{
	byte flags;
	unsigned int seq;

	//extract sequence number and flags from the packet header
	Mem_Copy(&seq, nc->recv.__buf, sizeof(unsigned int));
	seq = LittleInt(seq);
	flags = nc->recv.__buf[HEADER_FLAG_LOC];

	if (flags & PACKET_RELIABLE)
	{
		if (WSock_ProcessReliablePacket(nc, nc->recv.__buf, &nc->recvAddr))
			return true;
		else
			return false;
	}
	//okay, it's not a reliable packet, so the sequence should match the generic nonreliable sequence number
	// if not, it's probably a fragmented packet and we should throw it out
	if (seq != NONRELIABLE_SEQUENCE)
	{
		NET_PRINTF("Got invalid packet data - sequence %u\n", seq);
		return false;
	}
	if (flags & PACKET_ACK)
	{	  	
		WSock_ProcessPacketAck(nc);
		return false;
	}

	return true;
}

void	WSock_AddHeader(unsigned char fullPacket[], byte flags, unsigned int seq)
{
	union
	{
		unsigned int i;
		byte b[4];
	} u;

	u.i = LittleInt(seq);

	fullPacket[0] = u.b[0];
	fullPacket[1] = u.b[1];
	fullPacket[2] = u.b[2];
	fullPacket[3] = u.b[3];
	fullPacket[4] = flags;
}

bool    WSock_BroadcastPacket(netconnection_t *nc, int port)
{
	int on = 1, ret = 0;
	SOCKET sock;
	struct sockaddr_in sin;
	
	sock = nc->sock;
	if (sock < 0)
	{
		Console_DPrintf("Error opening local client socket.\n");
		return false;
	}
	
	if(setsockopt (sock, SOL_SOCKET, SO_BROADCAST, (char *)&on, sizeof(on)) < 0)
	{
		Console_DPrintf("Error setsockopt(SO_BROADCAST), errno: %d, desc: %s", errno, strerror(errno));
		return false;
	}
	
	memset(&sin, 0, sizeof(sin));
	sin.sin_family  = AF_INET;
	sin.sin_port    = htons((unsigned short)port);
	sin.sin_addr.s_addr = htonl(INADDR_BROADCAST);
	
	ret = sendto(sock, nc->send.__buf, nc->send.length + HEADER_SIZE, 0, (const struct sockaddr *)&sin, sizeof(sin));
	if (ret == -1)
	{
		Console_DPrintf("error sending packet - %s\n", strerror(errno));
		return false;
	}
	else
		Console_DPrintf("%i bytes sent\n", ret);
	
	on = 0;
	if(setsockopt (sock, SOL_SOCKET, SO_BROADCAST, (char *)&on, sizeof(on)) < 0)
	{
		Console_DPrintf("Error setsockopt(SO_BROADCAST), errno: %d, desc: %s", errno, strerror(errno));
		return false;
	}
	
	return true;
}

bool	_WSock_SendPacket(SOCKET sock, struct sockaddr *sendaddr, unsigned char *buf, int buf_length, unsigned int seq, byte flags)
{
	int ret;

	WSock_AddHeader(buf, flags, seq);

#ifdef ANAL_NETWORK_DEBUGGING
	NET_PRINTF("Net: Sending %i bytes with seq %u to port %i - flags %s %s\n", buf_length - HEADER_SIZE, seq,
									((struct sockaddr_in *)sendaddr)->sin_port,
									flags & PACKET_RELIABLE ? "RELIABLE" : "",
									flags & PACKET_ACK ? "ACK" : "");
#endif
	
	ret = sendto(sock, buf, buf_length, 0, sendaddr, sizeof(struct sockaddr_in));

	if (ret == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		if (err != WSAEWOULDBLOCK)
		{
			NET_PRINTF("sendto() failed: error %d\n", err);
		}
		return false;
	}

	return true;
}

bool	WSock_SendPacket(netconnection_t *nc)
{
	return _WSock_SendPacket(nc->sock, (struct sockaddr *)(&nc->sendAddr), nc->send.__buf, nc->send.length + HEADER_SIZE, NONRELIABLE_SEQUENCE, PACKET_NORMAL);
}

bool	WSock_SendReliablePacket(netconnection_t *nc)
{
	reliablePacket_t *unack_packets_tail = nc->unack_packets;
  	unsigned int newseq = nc->reliableSend_lastseq+1;
	bool success;

	NET_PRINTF("sending %i byte reliable packet with sequence %u\n", nc->reliableSend.length + HEADER_SIZE, newseq);
	
	if (!net_reliablePackets.value)
	{
		//don't use reliable packet sending
		return _WSock_SendPacket(nc->sock, (struct sockaddr *)(&nc->sendAddr), nc->reliableSend.__buf, nc->reliableSend.length + HEADER_SIZE, NONRELIABLE_SEQUENCE, PACKET_NORMAL);
	}

	//NET_PRINTF("sending reliable packet with sequence %u, last seq was %u, nc is %p\n", newseq, nc->reliableSend_lastseq, nc);
	NET_PRINTF("Sending reliable packet with sequence %u, last sequence was %u\n", newseq, nc->reliableSend_lastseq);


	if (net_forcedPacketDrop.value
		&& M_Randnum(0,1) <= 0.5)
	{
		//hack to try breaking the reliable packet stuff
		success = true;
	}
	else
		success = _WSock_SendPacket(nc->sock, (struct sockaddr *)(&nc->sendAddr), nc->reliableSend.__buf, nc->reliableSend.length + HEADER_SIZE, newseq, PACKET_NORMAL | PACKET_RELIABLE);

	//if (success)
	{
		//add this packet to the list of sent reliable packets waiting for ACKs
		reliablePacket_t *newPacket = Tag_Malloc(sizeof (reliablePacket_t), MEM_NET);
		newPacket->packet = Tag_Malloc(sizeof (byte) * (nc->reliableSend.length + HEADER_SIZE), MEM_NET);
		Mem_Copy(newPacket->packet, nc->reliableSend.__buf, nc->reliableSend.length + HEADER_SIZE);
		newPacket->data_length = nc->reliableSend.length;
		newPacket->timesent = Host_Milliseconds();
		newPacket->seq = newseq;
		newPacket->next = NULL;

		//we need to add this packet to the end of the unack_packets list
		if (unack_packets_tail)
		{
			while (unack_packets_tail->next)
				unack_packets_tail = unack_packets_tail->next;
			unack_packets_tail->next = newPacket;
		}
		else
		{
			nc->unack_packets = newPacket;
		}

		//increment the sequence number
		nc->reliableSend_lastseq = newseq;
		
		return true;
	} 
	
	if (!success)
	{
	  	NET_PRINTF("Failure to send reliable packet with sequence %u, will retry\n", newseq);
		return false;
	}
}




void		WSock_CheckPacketTimeouts(netconnection_t *nc)
{
	reliablePacket_t *packet = nc->unack_packets;
	float time = Host_Milliseconds();
	int numPacketsSent = 0, startSequence;

	if (!packet)
		return;

	startSequence = packet->seq;
	
	while (packet && numPacketsSent < MAX_PACKETS_RESENT_PER_FRAME)
	{
		if (//packet->seq < startSequence + MAX_PACKET_SEQUENCES_AHEAD && 
			time - packet->timesent > PACKET_TIMEOUT_MILLIS)
		{
			_WSock_SendPacket(nc->sock, (struct sockaddr *)(&nc->sendAddr), packet->packet, packet->data_length + HEADER_SIZE, packet->seq, PACKET_NORMAL | PACKET_RELIABLE);
			packet->timesent = time;
			numPacketsSent++;
		}
		packet = packet->next;
	}
}

void	WSock_ClearReliablePackets(netconnection_t *nc)
{
	reliablePacket_t *packet = nc->unack_packets;
	reliablePacket_t *tmp_packet;

	while (packet)
	{
		tmp_packet = packet->next;
		Tag_Free(packet->packet);
		Tag_Free(packet);
		packet = tmp_packet;
	}
	nc->unack_packets = NULL;
}
