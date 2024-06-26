// (C) 2003 S2 Games

// net_bsdsock.c

// bsd sockets TCP/UDP driver

#include "core.h"

#undef ANAL_NETWORK_DEBUGGING

static int	bytes_received = 0;
static int	bytes_sent = 0;

extern cvar_t net_reliablePackets;
extern cvar_t net_forcedPacketDrop;
extern cvar_t net_debug;

extern cvar_t net_forceIP;

int BSDSock_SetNonblocking(int fd)
{
	int flags;

	/* If they have O_NONBLOCK, use the Posix way to do it */
#if defined(O_NONBLOCK)
	/* Fixme: O_NONBLOCK is defined but broken on SunOS 4.1.x and AIX 3.2.5. */
	if (-1 == (flags = fcntl(fd, F_GETFL, 0)))
		flags = 0;
	return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#else
	/* Otherwise, use the old way of doing it */
	flags = 1;
	return ioctl(fd, FIOBIO, &flags);
#endif
}     


bool	BSDSock_SendRawPacket(SOCKET sock, struct sockaddr *sendaddr, unsigned char *buf, int buf_length, unsigned int lastseq, byte flags);

void	BSDSock_Init()
{
	int n;
	struct hostent *phe;
	int bestip = 0;
	char *ipstring;
	in_addr_t tmp;

	Console_Printf("BSDSock_Init\n");

	if (strcmp(net_forceIP.string, "0.0.0.0") != 0) //they want to force a certain IP
	{
		tmp = inet_addr(net_forceIP.string);
		Mem_Copy(&net_localip, (void *)&tmp, sizeof(struct in_addr));
		strcpy(net_localaddr, inet_ntoa(net_localip));
	}
	else
	{
	
#ifdef _WIN32
		if (WSAStartup(0x202, &ws.wsaData) == SOCKET_ERROR)
		{
			WSACleanup();
			Console_Errorf("WSAStartup failed (%s)\n", WSAGetLastError());
			return;
		}
#endif

#ifdef _WIN32
		if (gethostname(net_localaddr, 256) == SOCKET_ERROR)
		{
			WSACleanup();
			Console_Errorf("gethostname() failed (%s)\n", WSAGetLastError());
			return;
		}
#else
		if (gethostname(net_localaddr, 256) == -1)
		{
			Console_Printf("gethostname() failed (%s)\n",hstrerror(errno));
			Mem_Copy(&net_localip, "\0\0\0\0", 4);
			strcpy(net_localaddr, "0.0.0.0");
			return;
		}
#endif

		Console_DPrintf("Getting IP address for %s\n", net_localaddr);
		phe = gethostbyname(net_localaddr);
		
		if (!phe)
		{
	
			Console_DPrintf("Error trying to get local IP Address, gethostbyname(%s) failed!\n", net_localaddr);
			ipstring = "127.0.0.1";
			tmp = inet_addr(ipstring);
			Mem_Copy(&net_localip, (void *)&tmp, sizeof(struct in_addr));
			strcpy(net_localaddr, inet_ntoa(net_localip));
		}
		else
		{
   		 	for (n = 0; phe->h_addr_list[n] != 0; n++)
			{   
		
				Mem_Copy(&net_localip, phe->h_addr_list[n], sizeof(struct in_addr));
				ipstring = inet_ntoa(net_localip);
				Console_Printf("Local IP #%i: %s\n", n+1, ipstring);
		
				//fixme: this is a hack in case we're shared off the connection 
				//to the internet.  this won't work all the time so find a better way		
				if (strncmp(ipstring,    "192.168.", 8)!=0
					&& strncmp(ipstring, "127.0.0.1", 9)!=0)
					bestip = n;
   			}
	
			Mem_Copy(&net_localip, phe->h_addr_list[bestip], sizeof(struct in_addr));
			strcpy(net_localaddr, inet_ntoa(net_localip));
		}
	}
	
	//	ncLocalClient.driverData = BSDSock_AllocateDriverData();
}	

void	BSDSock_ShutDown()
{
	Console_Printf("BSDSock_ShutDown()\n");
}

bool	BSDSock_SetSendAddr(netconnection_t *nc, char *addr, int clientport)
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
			Console_DPrintf("BSDSock_SetSendAddr: Couldn't resolve %s\n", addr);			
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

void	BSDSock_Converse(netconnection_t *nc)
{
	Mem_Copy(&nc->sendAddr, &nc->recvAddr, sizeof(struct sockaddr_in));
	strcpy(nc->sendAddrName, nc->recvAddrName);
}

bool	BSDSock_NewSocket(netconnection_t *nc)
{
	nc->sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); // UDP socket
	if (nc->sock < 0)
	{
		Console_Errorf("Error creating a socket!\n");
		return false;
	}
	
	if (BSDSock_SetNonblocking(nc->sock) == -1)
	{
		Console_Errorf("Error!  Couldn't set new socket to nonblocking mode!\n");
		return false;
	}
	return true;
}
	

void	BSDSock_CloseNetConnection(netconnection_t *nc)
{
	close(nc->sock);		
}


//open a socket with the specified port
bool BSDSock_OpenPort(netconnection_t *nc, int port)
{
	in_addr_t addr;
	Console_DPrintf("BSDSock_OpenPort(%i)\n", port);

	nc->addr.sin_family = AF_INET;
	if (strcmp(net_forceIP.string, "0.0.0.0") != 0)
	{
		addr = inet_addr(net_forceIP.string);
		memcpy(&nc->addr.sin_addr, &addr, sizeof(in_addr_t));
	}
	else
	{
		nc->addr.sin_addr.s_addr = INADDR_ANY;
	}
	nc->addr.sin_port = htons((unsigned short)port);

	if (!BSDSock_NewSocket(nc))
	{
		Console_Printf("Socket creation failed\n");
		return false;
	}

	if (bind(nc->sock, (void*)&nc->addr, sizeof(struct sockaddr_in)) == -1) 
	{
		BSDSock_CloseNetConnection(nc);
		Console_Errorf("bind() failed (%s)\n", hstrerror(errno));		
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

bool	BSDSock_ProcessReliablePacket(netconnection_t *nc, unsigned char *buf, struct sockaddr_in *from)
{
	packet_t ackpacket;
	unsigned int buf_seq;
	bool process = true;

	Mem_Copy(&buf_seq, buf, sizeof(unsigned int));
	buf_seq = LittleInt(buf_seq);

	if (net_debug.integer)
		Console_DPrintf("Received packet with sequence %u\n", buf_seq);
	
	if (buf_seq > nc->reliableRecv_lastseq+1)
	{
		//drop it, out of order - wait for the next packet before jumping ahead, to maintain transfer order
		//  don't even ACK it, so they think we never got it and resend it after resending the earlier one(s) we missed
		if (net_debug.integer)
			Console_DPrintf("Received out of order sequence %u, waiting for %u\n", buf_seq, nc->reliableRecv_lastseq+1);
		return false;
	}
	else if (buf_seq == nc->reliableRecv_lastseq+1)
	{
		//this is a new reliable packet that we've waiting for.  Increment the incoming sequence, and use the packet
		nc->reliableRecv_lastseq++;
		if (net_debug.integer)
			Console_DPrintf("Packet matches what we were looking for, incrementing lastseq to %u\n", nc->reliableRecv_lastseq);
	}
	else //buf_seq < nc->reliableRecv_lastseq+1
	{
		//this is a resend for a packet we've gotten already.  ACK it, but don't process it again
		if (net_debug.integer)
			Console_DPrintf("Received resend of reliable packet sequence %u, not using it\n", buf_seq);
		process = false;
	}
		
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
		Console_DPrintf("Sending ack for packet sequence %u\n", buf_seq);
		BSDSock_SendRawPacket(nc->sock, (struct sockaddr *)&nc->sendAddr, ackpacket.__buf, ackpacket.length + HEADER_SIZE, NONRELIABLE_SEQUENCE, PACKET_NORMAL | PACKET_ACK);
	}

	return process;
}

void	BSDSock_ProcessPacketAck(netconnection_t *nc)
{
	reliablePacket_t *packet = nc->unack_packets;
	reliablePacket_t *tmp_packet = nc->unack_packets;
	unsigned int seq;

	seq = Pkt_ReadInt(&nc->recv);

	if (net_debug.integer)
		Console_DPrintf("Got ACK for sequence %u\n", seq);
	if (!packet)
	{
		if (net_debug.integer)
			Console_DPrintf("Got ACK for seq %u, but we have no unACK'd packets!\n", seq);
		return;
	}
	
	if (net_debug.integer)
    	Console_DPrintf("Comparing seq %u with unack_packet_seq %u\n", seq, packet->seq);
    while (packet
            && packet->seq != seq)
    {
        packet = packet->next;
        if (packet)
			if (net_debug.integer)
            	Console_DPrintf("Comparing seq %u with unack_packet_seq %u\n", seq, packet->seq);
    }
    if (!packet)
    {
		if (net_debug.integer)
        	Console_DPrintf("Got ACK for non-existant sequence %u\n", seq);
        return;
    }
	
	if (net_debug.integer)
		Console_DPrintf("Successfully ACK'd sequence %u\n", seq);

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

		if (net_debug.integer)
		{
			if (seq != packet->seq)
        	    Console_DPrintf("They must have already gotten packet %u then, I'll assume they did.\n", packet->seq);
		}

        tmp_packet = tmp_packet->next;
        Tag_Free(packet->packet);
        Tag_Free(packet);
    }
	if (nc->unack_packets && net_debug.integer)
		Console_DPrintf("The first unack'd packet # is now %u\n", nc->unack_packets->seq);
}

int		BSDSock_ReceivePacket(netconnection_t *nc)
{
	int ret;
	socklen_t fromlen;
	netaddr_t from;
	//char buf[MAX_PACKET_SIZE + HEADER_SIZE];
	
	fromlen = sizeof(struct sockaddr);

	ret = recvfrom(nc->sock, nc->recv.__buf, MAX_PACKET_SIZE + HEADER_SIZE, 0, (struct sockaddr *)&from, &fromlen);
		
	if (ret < 0)
	{
		if (errno != EAGAIN)
		{
			Console_DPrintf("recvfrom() failed: error %s\n", hstrerror(errno));	
		}
	}
	else if (ret > 0)
	{
		bytes_received += ret;

#ifdef ANAL_NETWORK_DEBUGGING
		Console_DPrintf("Net: receiving %i bytes from %s:%i\n", nc->recv.length, inet_ntoa(from.sin_addr), ntohs(from.sin_port));
#endif

		Mem_Copy(&nc->recvAddr, &from, sizeof(struct sockaddr_in));
		strncpy(nc->recvAddrName, inet_ntoa(from.sin_addr), MAX_ADDRESS_LENGTH);
	
		nc->recv.length = ret - HEADER_SIZE;
		nc->recv.curpos = 0;
	}

	return ret;
}

bool 	BSDSock_PreProcessPacket(netconnection_t *nc)
{
	byte flags;
	unsigned int seq;
	
	//extract sequence number and flags from the packet header
	Mem_Copy(&seq, nc->recv.__buf, sizeof(unsigned int));
	seq = LittleInt(seq);
	flags = nc->recv.__buf[HEADER_FLAG_LOC];

	if (flags & PACKET_RELIABLE)
	{
		if (BSDSock_ProcessReliablePacket(nc, nc->recv.__buf, &nc->recvAddr))
			return true;
		else
			return false;
	}
	//okay, it's not a reliable packet, so the sequence should match the generic nonreliable sequence number
	// if not, it's probably a fragmented packet and we should throw it out
	if (seq != NONRELIABLE_SEQUENCE)
	{
		Console_DPrintf("Got invalid packet data - sequence %u\n", seq);
		return false;
	}
	if (flags & PACKET_ACK)
	{	    
		BSDSock_ProcessPacketAck(nc);
		return false;
	}
	return true;
}

void	BSDSock_AddHeader(unsigned char fullPacket[], byte flags, unsigned int seq)
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

bool	BSDSock_BroadcastPacket(netconnection_t *nc, int port)
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
		close(sock);
		return false;
	}
	
	memset(&sin, 0, sizeof(sin));
	sin.sin_family  = AF_INET;  
	sin.sin_port    = htons(port); 
	sin.sin_addr.s_addr = htonl(INADDR_BROADCAST);
	
	ret = sendto(sock, nc->send.__buf, nc->send.length + HEADER_SIZE, 0, (const struct sockaddr *)&sin, sizeof(sin));
	if (ret == -1)
	{
		Console_DPrintf("error broadcasting packet - %s\n", strerror(errno));
		close(sock);
		return false;
	}
	else
		Console_DPrintf("%i bytes broadcast\n", ret);
	
	//turn off broadcast
	on = 0;
	
	if(setsockopt (sock, SOL_SOCKET, SO_BROADCAST, (char *)&on, sizeof(on)) < 0)
	{
		Console_DPrintf("Error setsockopt(SO_BROADCAST), errno: %d, desc: %s", errno, strerror(errno));
		close(sock);
		return false;
	}
	
	return true;
}

bool	BSDSock_SendRawPacket(int sock, struct sockaddr *sendaddr, unsigned char *buf, int buf_length, unsigned int seq, byte flags)
{
	int ret;

	BSDSock_AddHeader(buf, flags, seq);

	if (seq == 0)
		Console_DPrintf("ERROR! Invalid sequence!\n");
	
#ifdef ANAL_NETWORK_DEBUGGING
	Console_DPrintf("Net: Sending %i bytes over socket %i with seq %u to port %i - flags %s %s\n", buf_length - HEADER_SIZE, seq, 
					sock,
					ntohs(((struct sockaddr_in *)sendaddr)->sin_port),
					flags & PACKET_RELIABLE ? "RELIABLE" : "", 
					flags & PACKET_ACK ? "ACK" : "");
#endif
	
	ret = sendto(sock, buf, buf_length, 0, sendaddr, sizeof(struct sockaddr_in));

	if (ret < 0)
	{
		if (errno != EAGAIN)
		{
			Console_DPrintf("sendto() failed: error %s\n", hstrerror(errno));
		}
		return false;
	}

	bytes_sent += ret;

	return true;
}

bool	BSDSock_SendPacket(netconnection_t *nc)
{
	return BSDSock_SendRawPacket(nc->sock, (struct sockaddr *)(&nc->sendAddr), nc->send.__buf, nc->send.length + HEADER_SIZE, NONRELIABLE_SEQUENCE, PACKET_NORMAL);
}

bool	BSDSock_SendReliablePacket(netconnection_t *nc)
{
	reliablePacket_t *unack_packets_tail = nc->unack_packets;
  	unsigned int newseq = nc->reliableSend_lastseq+1;
	bool success;

  	//Console_DPrintf("sending %i byte reliable packet with sequence %u, last sequence was %u\n", nc->reliableSend.length + HEADER_SIZE, newseq, nc->reliableSend_lastseq);

	if (!net_reliablePackets.value)
	{
		//disable reliable packet sending
		return BSDSock_SendRawPacket(nc->sock, (struct sockaddr *)(&nc->sendAddr), nc->reliableSend.__buf, nc->reliableSend.length + HEADER_SIZE, NONRELIABLE_SEQUENCE, PACKET_NORMAL);
	}

	if (net_forcedPacketDrop.value
		&& M_Randnum(0,1) <= 0.5)
	{
		//hack to try breaking the reliable packet stuff
		success = true;
	}
	else
	{
		success = BSDSock_SendRawPacket(nc->sock, (struct sockaddr *)(&nc->sendAddr), nc->reliableSend.__buf, nc->reliableSend.length + HEADER_SIZE, newseq, PACKET_NORMAL | PACKET_RELIABLE);
	}

//	if (success)
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
  		if (net_debug.integer)
			Console_Printf("Failure to send reliable packet with sequence %u, will retry\n", newseq);
		return false;
	}
}

void		BSDSock_CheckPacketTimeouts(netconnection_t *nc)
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
			//Console_DPrintf("Resending reliable packet sequence %u, since we didn't get an ACK in %f ms\n", packet->seq, time - packet->timesent);
			BSDSock_SendRawPacket(nc->sock, (struct sockaddr *)(&nc->sendAddr), packet->packet, packet->data_length + HEADER_SIZE, packet->seq, PACKET_NORMAL | PACKET_RELIABLE);
			packet->timesent = time;
			numPacketsSent++;
		}
		packet = packet->next;
	}
}

void    BSDSock_ClearReliablePackets(netconnection_t *nc)
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

