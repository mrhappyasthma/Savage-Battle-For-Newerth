// (C) 2003 S2 Games

// client_main.c

// Core client functions (interface to client game functions)

#include "core.h"

#define NET_DEBUG

localClientInfo_t lc;
clientDemoInfo_t demo;

cvar_t cl_name =				{ "name", "UnnamedNewbie", CVAR_SAVECONFIG | CVAR_NETSETTING };
cvar_t cl_maxPacketSize =		{ "maxPacketSize", "1300", CVAR_SAVECONFIG | CVAR_NETSETTING };
cvar_t cl_bps =					{ "netBPS", "20000", CVAR_SAVECONFIG | CVAR_NETSETTING };
cvar_t cl_netFPS =				{ "netFrames", "20", CVAR_SAVECONFIG | CVAR_NETSETTING };
cvar_t cl_debug =				{ "cl_debug", "0" };
cvar_t cl_password =			{ "cl_password", "" };
cvar_t cl_adminPassword =		{ "cl_adminPassword",	"" };
cvar_t cl_ambientSound =		{ "cl_ambientSound", "", CVAR_WORLDCONFIG };
cvar_t cl_ambientVolume =		{ "cl_ambientVolume", "1.0", CVAR_WORLDCONFIG };
cvar_t cl_showUnordered =		{ "cl_showUnordered", "0" };
cvar_t cl_music1 =				{ "cl_music1", "music/lost_hills.ogg", CVAR_WORLDCONFIG };
cvar_t cl_music2 =				{ "cl_music2", "music/the_savage_age.ogg", CVAR_WORLDCONFIG };
cvar_t cl_music3 =				{ "cl_music3", "music/the_savage_age.ogg", CVAR_WORLDCONFIG };
cvar_t cl_music4 =				{ "cl_music4", "music/the_savage_age.ogg", CVAR_WORLDCONFIG };
cvar_t cl_music5 =				{ "cl_music5", "music/the_savage_age.ogg", CVAR_WORLDCONFIG };
cvar_t cl_music6 =				{ "cl_music6", "music/the_savage_age.ogg", CVAR_WORLDCONFIG };
cvar_t cl_debugTimer =			{ "cl_debugTimer", "0" };
cvar_t cl_lagFudgeTime =		{ "cl_lagFudgeTime", "0" };
cvar_t cl_ignoreBadPackets =	{ "cl_ignoreBadPackets", "0", CVAR_SAVECONFIG };
cvar_t cl_randomPacketLoss =	{ "cl_randomPacketLoss", "0" };
cvar_t cl_showPackets =			{ "cl_showSend", "0" };
cvar_t cl_delayFrames =			{ "cl_delayFrames", "0" };
cvar_t cl_packetSendFPS =		{ "cl_packetSendFPS", "30", CVAR_SAVECONFIG | CVAR_VALUERANGE, 1, 85 };
cvar_t cl_debugInputs =			{ "cl_debugInputs", "0" };
cvar_t cl_noDelta =				{ "cl_noDelta", "0" };

cvar_t cl_currentContentDownload =			{ "cl_currentContentDownload", "", CVAR_READONLY };
cvar_t cl_currentContentDownloadProgress =	{ "cl_currentContentDownloadProgress", "", CVAR_READONLY };


extern cvar_t gfx;
extern cvar_t net_connectTimeout;
extern cvar_t net_problemIconTime;
extern cvar_t net_connectionProblemTimeThreshhold;

inputState_t nullinput;

void	Sound_StopAllSounds();

#define DOWNLOAD_KEEPALIVE_INTERVAL 5000


bool	Client_RestartWorld(bool softrestart);

bool	Client_SNET_MESSAGE(packet_t *pkt)
{
	int msgtype;
	char clientnum;
	char buf[1024];
	bool handled = false;

	if (cl_debug.integer) 
	{
		Console_DPrintf("Server: SNET_MESSAGE: ");	
	}

	msgtype = Pkt_ReadByte(pkt);
	clientnum = (char)Pkt_ReadByte(pkt);

	Pkt_ReadString(pkt, buf, 1023);

	if (cl_debug.integer)
	{
		Console_DPrintf("%i %i %s\n", msgtype, clientnum, buf);
	}

	if (strncmp(buf, "echo ", 5)==0)
	{
		//echo to console
		Console_Printf(&buf[5]);

		handled = true;
	}
	else if (strcmp(buf, "softrestart")==0)
	{
		//we're doing a "soft" restart		

		if (cl_debug.integer)
		{
			Console_Printf("SOFT RESTART\n");
		}

		Pkt_Clear(&ncLocalClient.reliableSend);
		lc.softRestart = true;
		lc.cstate = CCS_AWAITING_STATE_STRINGS;
		Pkt_WriteCmd(&ncLocalClient.reliableSend, CNET_REQUEST_STATE_STRINGS);
		cl_api.Shutdown();

		return true;
	}
	else if (strcmp(buf, "fullrestart")==0)
	{
		//we're doing a full restart

		if (cl_debug.integer)
		{
			Console_Printf("FULL RESTART\n");
		}

		Pkt_Clear(&ncLocalClient.reliableSend);
		lc.softRestart = false;
		lc.cstate = CCS_AWAITING_STATE_STRINGS;
		Pkt_WriteCmd(&ncLocalClient.reliableSend, CNET_REQUEST_STATE_STRINGS);
		cl_api.Shutdown();

		return true;
	}

	if (lc.cstate >= CCS_AWAITING_FIRST_FRAME)
	{
		if (cl_api.ServerMessage(clientnum, buf))
			return true;
	}

	if (cl_debug.integer) {
		Console_DPrintf("msgtype: %i  clientnum: %i\nbuf: %s\n", msgtype, clientnum, buf);
	}
	

	if (!handled)
	{
		Console_Printf("Invalid message: %s\n", buf);
		return false;
	}

	return true;
}

bool	Client_SNET_DISCONNECTING(packet_t *pkt)
{
	if (cl_debug.integer) {
		Console_DPrintf("Server: SNET_DISCONNECTING\n");
	}

	//Console_Errorf("***Server disconnected***\n");
	//Client_Disconnect();		//fixme: do we want the client to send a disconnect packet to the server? 

	Game_Error("*** Server disconnected ***\n");

	return true;
}

bool	Client_SNET_KICK(packet_t *pkt)
{
	char reason[256];

	if (cl_debug.integer) {
		Console_DPrintf("Server: SNET_KICK\n");
	}

	Pkt_ReadString(pkt, reason, 255);
	
	if (reason[0])
	{
		Game_Error(fmt("%s\n", reason));
	}
	else
	{
		Game_Error("Disconnected for unknown reason\n");
	}

	return true;
}



void	Client_GameObjectPointer(void *base, int stride, int num_objects)
{
	int n, offset;

	offset = 0;
	if (!stride)
		System_Error("Client_GameObjectPointer: stride was 0\n");

	if (num_objects > MAX_OBJECTS)
		System_Error("Client_GameObjectPointer: exceeded MAX_OBJECTS\n");

	for (n=0; n<num_objects; n++)
	{
		(char *)lc.gameobjs[n] = (char *)base + offset;

		memset(lc.gameobjs[n], 0, stride);

		offset += stride;
	}

}

bool	Client_ConnectionProblems()
{
	return lc.connectionProblems;
}


//temporary data used by Client_ReadObjectUpdate and Client_ReadFrameUpdate
static int removedObjects[MAX_OBJECTS];
static baseObject_t *changedObjects[MAX_OBJECTS];
static baseObject_t *unchangedObjects[MAX_OBJECTS];
static int removedCount;
static int changedCount;
static int unchangedCount;
static int fromMarker = 0;


/*==========================

  Client_CopyBaseObject

  copy an object over from an old frame into the new frame

 ==========================*/

void	Client_CopyBaseObject(const baseObject_t *oldobj, baseObject_t *newobj)
{
	*newobj = *oldobj;

	//events only exist during the frame they happened on
	newobj->numEvents = 0;

	newobj->active = true;
}


/*==========================

  Client_ReadBaseObjectDiff

  read a baseobject delta update from a packet

 ==========================*/

void	Client_ReadBaseObjectDiff(packet_t *pkt, baseObject_t *base, bool hasEvents)
{
	int n;

	Net_ReadStructDiff(pkt, base, baseObjectDesc);		//read in the diff
	
	base->active = true;

	//read in events

	if (!hasEvents)
	{
		base->numEvents = 0;
		return;
	}

	base->numEvents = Pkt_ReadByte(pkt);
	for (n=0; n<base->numEvents; n++)
	{
		objEvent_t *event = &base->events[n];

		event->type = Pkt_ReadByte(pkt);
		if (event->type & EVENT_SENT_PARAM1)
		{
			event->param = Pkt_ReadByte(pkt);
		}
		else
		{
			event->param = 0;
		}
		if (event->type & EVENT_SENT_PARAM2)
		{
			event->param2 = Pkt_ReadByte(pkt);
		}
		else
		{
			event->param2 = 0;
		}
		event->type &= ~(EVENT_SENT_PARAM1 | EVENT_SENT_PARAM2);
	}
}



/*==========================

  Client_ReadObjectUpdate

  Read all object updates and stick them into cl.curFrame

 ==========================*/

bool	Client_ReadObjectUpdate(packet_t *pkt)
{/*
	while (1)
	{

		short index_flags = Pkt_ReadShort(pkt);

	}
	*/

	baseObject_t *oldobj, *newobj;
	int fromNumObjects;	
	int lastIndex = -1;
	bool terminated = false;
	byte cmd = Pkt_ReadCmd(pkt);

	if (cmd != SNET_UPDATE_OBJECTS)
	{
		Console_Printf("cmd != SNET_UPDATE_OBJECTS\n");
		return false;
	}

	if (lc.fromFrame)
		fromNumObjects = lc.fromFrame->num_objects;
	else
		fromNumObjects = 0;


	while (pkt->curpos < pkt->length)
	{			
		baseObject_t *oldobj;
		baseObject_t *newobj;
		short index;
		bool updateWasRead = false;
		short flags = Pkt_ReadShort(pkt);
		

		if (flags == OBJECT_CONTINUED)
		{
			//the rest of the update will continue on the next frame			
			return true;
		}
		else if (flags == OBJECT_END_UPDATE)
		{
			lc.finishedFrame = true;
			terminated = true;			
			break;
		}
		
		index = flags & 1023;		

		if (lastIndex >= index)
		{
			//index is always incremented during a packet update
			Console_Printf("lastIndex >= index");
			return false;
		}		

		if (fromMarker < fromNumObjects)
		{
			while (fromMarker < fromNumObjects)
			{			
				oldobj = &lc.net_objects[(lc.fromFrame->start_object + fromMarker) % NUM_CL_NET_OBJECTS];
				newobj = &lc.net_objects[lc.cur_net_object % NUM_CL_NET_OBJECTS];

				if (oldobj->index < index)
				{
					//old object is unchanged in the new update					
					Client_CopyBaseObject(oldobj, newobj);
					lc.cur_net_object++;
					lc.curFrame.num_objects++;
					unchangedObjects[unchangedCount++] = newobj;
				}
				else if (oldobj->index == index)
				{
					//the object has changed in some way
					if (flags & OBJECT_FREE || flags & OBJECT_NOT_VISIBLE)
					{
						//object was removed
						removedObjects[removedCount++] = flags;						
					}					
					else
					{
						*newobj = *oldobj;		//start with oldobj as the base
						Client_ReadBaseObjectDiff(pkt, newobj, flags & OBJECT_HAS_EVENTS);						
						lc.cur_net_object++;
						lc.curFrame.num_objects++;
						changedObjects[changedCount++] = newobj;
					}

					updateWasRead = true;
				}
				else
				{
					break;
				}

				fromMarker++;
			}
		}
		
		if (!updateWasRead)
		{
			//reading in data for a new object
			if (flags & OBJECT_FREE || flags & OBJECT_NOT_VISIBLE)
			{
				//the server is telling us to remove an object we never knew existed				
				//FIXME: i don't know why this is happening!
				Console_Printf("Unexpected baseObject removal for index %i", index);
				lc.lastValidFrame = 0;
				lc.validFrame = false;
				lc.invalidFrameCount++;
				//something's fucked up so clear the frame history
				memset(&lc.frames, 0, sizeof(lc.frames));
				Pkt_Clear(pkt);
				return true;
			}
			else
			{

				newobj = &lc.net_objects[lc.cur_net_object % NUM_CL_NET_OBJECTS];
				
				memset(newobj, 0, sizeof(*newobj));
				Client_ReadBaseObjectDiff(pkt, newobj, flags & OBJECT_HAS_EVENTS);			
				//this is the only place index gets explicitly set.  Extremely Important!
				//the entire system relies on this...
				newobj->index = index;		
				lc.cur_net_object++;
				lc.curFrame.num_objects++;
				changedObjects[changedCount++] = newobj;
			}
		}

		lastIndex = index;
	}

	//copy any remaining objects over from the old frame
	while (fromMarker < fromNumObjects)
	{
		oldobj = &lc.net_objects[(lc.fromFrame->start_object + fromMarker) % NUM_CL_NET_OBJECTS];
		newobj = &lc.net_objects[lc.cur_net_object % NUM_CL_NET_OBJECTS];

		Client_CopyBaseObject(oldobj, newobj);

		lc.cur_net_object++;
		lc.curFrame.num_objects++;
		fromMarker++;
		unchangedObjects[unchangedCount++] = newobj;
	}
	
	if (!terminated)		//the update didn't terminate correctly
	{
		Game_Error("Net error: Update not terminated\n");
		return false;
	}

	return true;
}


/*==========================

  Client_ReadPlayerState

  ==========================*/

bool	Client_ReadPlayerState(packet_t *pkt)
{
	int n;
	byte cmd = Pkt_ReadCmd(pkt);
	
	playerState_t *ps = &lc.curFrame.playerstate;

	if (cmd != SNET_UPDATE_PLAYERSTATE)
	{
		Console_Printf("cmd != SNET_UPDATE_PLAYERSTATE");
		return false;
	}

	if (lc.fromFrame)
	{
		//start from the old frame's playerstate
		*ps = lc.fromFrame->playerstate;		
	}
	else
	{
		//start with a blank playerstate
		memset(ps, 0, sizeof(*ps));		
	}

	//incorporate the new fields
	Net_ReadStructDiff(pkt, ps, playerStateDesc);
	
	//read in events
	ps->numEvents = Pkt_ReadByte(pkt);
	for (n=0; n<ps->numEvents; n++)
	{
		ps->events[n].type = Pkt_ReadByte(pkt);

		if (ps->events[n].type & EVENT_SENT_PARAM1)
			ps->events[n].param = Pkt_ReadByte(pkt);
		else
			ps->events[n].param = 0;

		if (ps->events[n].type & EVENT_SENT_PARAM2)
			ps->events[n].param2 = Pkt_ReadByte(pkt);
		else
			ps->events[n].param2 = 0;

		ps->events[n].type &= ~(EVENT_SENT_PARAM1 | EVENT_SENT_PARAM2);
	}

	lc.objectUpdateFlag = true;
	lc.objectUpdateTime = lc.server_timestamp;
	
	return true;
}




/*==========================

  Client_ReadFrameUpdate

  

 ==========================*/

bool _Client_ReadFrameUpdate(packet_t *pkt, bool continuing)
{
	lc.finishedFrame = false;

	if (!continuing)				//the update is a continuation of the last frame
	{
		if (!Client_ReadPlayerState(pkt))
			return false;
	}
	if (!Client_ReadObjectUpdate(pkt))
		return false;
	
	return true;
}



/*==========================

  Client_ReadFrameUpdate

  

 ==========================*/

void	Client_EnterGame();

bool	Client_ReadFrameUpdate(packet_t *pkt, bool continuing)
{
	bool ret;
	
	if (!lc.validFrame)
	{
		lc.invalidFrameCount++;

		Console_Printf("Invalid sv frame %i\n", lc.curFrame.framenum);

		if (lc.invalidFrameCount > 15)
		{
			//we haven't gotten a valid frame for the last second or so
			//(our connection == craptastic)
			//request a full update			
			lc.connectionProblems = true;
			lc.connectionProblemsStartTime = Host_Milliseconds();
			lc.lastValidFrame = 0;
		}

		Pkt_Clear(pkt);

		return true;
	}

	ret = _Client_ReadFrameUpdate(pkt, continuing);

	if (!ret)
		return false;

	//the frame has valid data and the update is completely finished
	if (lc.validFrame)
	{		
		if (lc.finishedFrame)
		{
			int n;

			if (cl_delayFrames.integer > 0)
			{
				//artificially delay our "last valid frame" to simulate a high latency connection

				int n;
				int best = 0;
				int lowest = 9999;

				//search for the frame in the circular buffer
				//we can't index into it by subtracting cl_delayFrames from the current frame, because
				//that frame might not exist in our history
				for (n=0; n<MAX_CL_UPDATE_HISTORY; n++)
				{
					if (lc.frames[n].framenum == lc.curFrame.framenum - cl_delayFrames.integer)
					{
						best = n;
						break;
					}
					else
					{
						int dif = ABS((int)lc.frames[n].framenum - ((int)lc.curFrame.framenum - cl_delayFrames.integer));
						if (dif < lowest)
						{
							lowest = dif;
							best = n;
						}
					}
				}

				lc.lastValidFrame = lc.frames[best].framenum;
			}
			else
			{
				lc.lastValidFrame = lc.curFrame.framenum;		//tell the server we want to diff from this frame
			}

			lc.playerState = lc.curFrame.playerstate;

			if (!world.cl_loaded)
			  return true;

			//
			//give all the new data to the game code
			//

			cl_api.BeginServerFrame(lc.curFrame.time, lc.server_timestamp);

			if (!lc.fromFrame)
			{	
				//we received a full update from the server
				//explicitly free any client objects that we didn't receive an update for
				//and call ObjectUpdated() on everything else

				int idx = 0;

				for (n=0; n<MAX_OBJECTS; n++)
				{
					if (n == lc.clientnum)
						continue;

					if (idx < changedCount)
					{
						if (n < changedObjects[idx]->index)
						{
							cl_api.ObjectFreed(n);
						}
						else if (n == changedObjects[idx]->index)
						{
							cl_api.ObjectUpdated(changedObjects[idx], true);
							idx++;
						}
					}
					else
					{
						cl_api.ObjectFreed(n);
					}
				}
			}
			else
			{
				//we diffed from an older frame
				//removedObjects[] and unchangedObjects[] arrays are only relevant here
				for (n=0; n<removedCount; n++)
				{
					int index = removedObjects[n] & 1023;

					if (removedObjects[n] & OBJECT_FREE)
						cl_api.ObjectFreed(index);
					else if (removedObjects[n] & OBJECT_NOT_VISIBLE)
						cl_api.ObjectNotVisible(index);
					else
						Game_Error("Invalid removal list item: [%i]%i (index %i)", n, removedObjects[n], index);
				}

				for (n=0; n<unchangedCount; n++)
				{
					cl_api.ObjectUpdated(unchangedObjects[n], false);
				}

				//changed or new objects
				for (n=0; n<changedCount; n++)
				{
					cl_api.ObjectUpdated(changedObjects[n], true);
				}
			}	

			cl_api.EndServerFrame();

			//copy this frame into our frame history
			//the frame can then be used for future delta updates from the server
			lc.frames[lc.curFrame.framenum % MAX_CL_UPDATE_HISTORY] = lc.curFrame;

			if (cl_debug.integer)
				Console_Printf("Reconstructed sv frame %i from %i\n", lc.curFrame.framenum, lc.fromFrame ? lc.fromFrame->framenum : 0);

			lc.invalidFrameCount = 0;
			lc.validFrame = false;
			lc.finishedFrame = false;

			if (lc.cstate == CCS_AWAITING_FIRST_FRAME && ret)
			{
				//this is our first frame
				Client_EnterGame();		
			}
		}
	}

	return true;
}


/*==========================

  Client_SNET_FRAME

 ==========================*/

bool	Client_SNET_FRAME(packet_t *pkt)
{
	bool ret;
	unsigned int fromFrameNum;
	int oldpacketpos = pkt->curpos-1;		//save out rewind position for demo recording

if (cl_debug.integer) {
	Console_DPrintf("Server: SNET_FRAME\n");
}

	if (lc.cstate < CCS_AWAITING_FIRST_FRAME)
		return false;

	if (cl_randomPacketLoss.value > 0 && !demo.playing)
	{
		if (M_Randnum(0, 1) < cl_randomPacketLoss.value)
		{
			Pkt_Clear(pkt);
			return true;
		}
	}

	lc.last_timestamp = lc.server_timestamp;	

	lc.server_timestamp = Pkt_ReadInt(pkt);					//get the server timestamp (milliseconds)
	lc.lastServerPacketTime = Host_Milliseconds();
	fromFrameNum = Pkt_ReadInt(pkt);
	lc.totaldelta = 0;

	if (fromFrameNum == 0 && demo.waiting == 2)
	{
		//we are waiting to record the demo
		//this is the message we've been waiting for
		//rewind the packet and return so we'll write it to the file
		Console_Printf("Received full update.  Demo recording started!\n");
		pkt->curpos = oldpacketpos;
		demo.waiting = 1;
		return true;
	}

	lc.continuedCount = 0;
	removedCount = 0;
	changedCount = 0;
	unchangedCount = 0;
	fromMarker = 0;	

	if (fromFrameNum == 0)
	{
		//server is sending a full update
		lc.validFrame = true;
		lc.fromFrame = NULL;
		memset(&lc.curFrame, 0, sizeof(lc.curFrame));
	}
	else
	{
		//see if the from frame exists in our history of frame updates
		//if not, we can't reliably update our data
		lc.fromFrame = &lc.frames[fromFrameNum % MAX_CL_UPDATE_HISTORY];
		if (lc.fromFrame->framenum != fromFrameNum)
		{
			//this frame doesn't exist in our update history (it got pushed out)
			if (cl_debug.integer)
				Console_Printf("(from %i) ", fromFrameNum);
			lc.validFrame = false;			
		}
		else if (lc.cur_net_object - lc.fromFrame->start_object >= NUM_CL_NET_OBJECTS - MAX_OBJECTS)
		{
			//the object data in the from frame got pushed out of the net_objects array,
			//or could possibly get pushed out
			if (cl_debug.integer)
				Console_Printf("(buffer out) ");
			lc.validFrame = false;			
		}
		else
		{
			lc.validFrame = true;
		}
	}

	//fill in information for this frame
	lc.curFrame.framenum = Pkt_ReadInt(pkt);
	lc.curFrame.start_object = lc.cur_net_object;
	lc.curFrame.num_objects = 0;				//this number will get incremented as we read in the object update message
	lc.curFrame.time = lc.server_timestamp;

	
	if (lc.gametime < lc.last_timestamp - net_connectionProblemTimeThreshhold.integer)
	{
		lc.connectionProblems = true;
		lc.connectionProblemsStartTime = Host_Milliseconds();
	}


	if (lc.curFrame.framenum < lc.lastServerFrame)
	{
		if (cl_debug.integer || cl_showUnordered.integer)
			Console_Printf("Out of order frame received from server: %i\n", lc.curFrame.framenum);
		netStats.clUnordered++;
		//hmm, should we mark the frame as invalid in this case?
	}

	lc.lastServerFrame = lc.curFrame.framenum;

	ret = Client_ReadFrameUpdate(pkt, false);

	return ret;
}





/*==========================

  Client_SNET_FRAME_CONTINUED

  Server is continuing a frame update from the previous frame

 ==========================*/

bool	Client_SNET_FRAME_CONTINUED(packet_t *pkt)
{	
	int framenum;
	int continuedCount;

	if (cl_debug.integer)
	{
		Console_DPrintf("Server: SNET_FRAME_CONTINUED\n");
	}

	framenum = Pkt_ReadInt(pkt);
	continuedCount = Pkt_ReadByte(pkt);
/*
	if (!lc.fromFrame)
	{
		Console_Printf("No from frame\n");
	}
*/

	//expect that the frame num is identical to the one we're currently updating
	if (framenum != lc.curFrame.framenum)
	{
		lc.validFrame = false;		
		netStats.clUnordered++;
		if (cl_debug.integer)
		{
			Console_Printf("Wrong framenum in FRAME_CONTINUED\n");
		}
	}

	//expect that the continued count sequence is incremented from the previous one
	//fixme: we could be a bit smarter about this so out of order packets don't get discarded...
	if (continuedCount != lc.continuedCount + 1)
	{
		lc.validFrame = false;
		netStats.clUnordered++;
		if (cl_debug.integer)
		{
			Console_Printf("Out of order count sequence (%i, should have been %i)\n", continuedCount, lc.continuedCount + 1);
		}
	}
	else
	{
		lc.continuedCount = continuedCount;
	}

	return Client_ReadFrameUpdate(pkt, true);
}



/*==========================

  Client_SNET_CVAR

 ==========================*/

bool	Client_SNET_CVAR(packet_t *pkt)
{
	char name[1024];
	char value[1024];
if (cl_debug.integer) {
	Console_DPrintf("Server: SNET_CVAR\n");
}
	Pkt_ReadString(pkt, name, 1024); //get the cvar name
	Pkt_ReadString(pkt, value, 1024); //get the new cvar value

	//set it
	Cvar_Set(name, value);
	
	return true;
}



/*==========================

  Client_SNET_UPDATE_OBJECTS

  

 ==========================*/

bool	Client_SNET_UPDATE_OBJECTS(packet_t *pkt)
{
	if (cl_debug.integer) {
		Console_DPrintf("Server: SNET_UPDATE_OBJECTS\n");
	}

	Game_Error("Got unexpected SNET_UPDATE_OBJECTS\n");

	return false;
}



/*==========================

  Client_SNET_UPDATE_PLAYERSTATE
  
  

 ==========================*/

bool	Client_SNET_UPDATE_PLAYERSTATE(packet_t *pkt)
{
if (cl_debug.integer) {
	Console_DPrintf("Server: SNET_UPDATE_PLAYERSTATE\n");
}
	
	Game_Error("Got unexpected SNET_UPDATE_PLAYERSTATE\n");

	return false;
}

/*==========================

  Client_ClearStateStrings

  done on a restart or SNET_AWAITING_STATE_STRINGS message

 ==========================*/

void	Client_ClearStateStrings()
{
	int n;

	//dealloc all state strings
	for (n=0; n<MAX_STATE_STRINGS; n++)
	{
		if (lc.stateStrings[n].string)
			Tag_Free(lc.stateStrings[n].string);
	}

	if (lc.bigstringBuffer)
		Tag_Free(lc.bigstringBuffer);

	memset(lc.stateStrings, 0, MAX_STATE_STRINGS * sizeof(stateString_t));
	lc.bigstringBuffer = NULL;
	lc.bigstringIndex = 0;
	lc.bigstringLength = 0;
	lc.bigstringBufferSize = 0;
}


/*==========================

  Client_GetStateString

  API function

 ==========================*/

void	Client_GetStateString(int id, char *buf, int size)
{
	stateString_t *state;

	if (id < 0 || id >= MAX_STATE_STRINGS)
		return;

	state = &lc.stateStrings[id];

	if (state->string)
		strncpySafe(buf, state->string, size);
	else
		buf[0] = 0;
}


/*==========================

  Client_SetStateString

  this is only for updating state strings from the server internally,
  it should never be used otherwise

 ==========================*/

void	Client_SetStateString(int id, const char *string)
{
	int len = strlen(string);
	stateString_t *state = &lc.stateStrings[id];

	if (state->string)
	{
		if (len+1 > state->memsize)
		{
			state->string = Tag_Realloc(state->string, len+1, MEM_CLIENT);
			state->memsize = len+1;
		}
	}
	else
	{
		state->string = Tag_Malloc(len+1, MEM_CLIENT);
		state->memsize = len+1;
	}

	strcpy(state->string, string);
	state->modifyCount++;
	state->length = len;

	if (id == ST_CVAR_SETTINGS)
	{
		//server transmitted some new cvar settings to us, so let's sync them up
		Cvar_SetLocalTransmitVars(state->string);
	}
	else
	{
		//if we're in the game, call the StateStringModified() function
		if (lc.cstate >= CCS_AWAITING_FIRST_FRAME)
		{
			if (cl_api.StateStringModified)			//don't require that this function be defined
				cl_api.StateStringModified(id);		//notify the client game code that the string was modified
		}
	}
}


/*==========================

  Client_SNET_SMALL_STRING

 ==========================*/

bool	Client_SNET_SMALL_STRING(packet_t *pkt)
{
	int id;
	char string[MAX_PACKET_SIZE];
	stateString_t *state;

	if (cl_debug.integer)
	{
		Console_DPrintf("SNET_SMALL_STRING\n");
	}

	id = Pkt_ReadShort(pkt);
	if (id < 0 || id >= MAX_STATE_STRINGS)
	{
		Console_Printf("SNET_SMALL_STRING: bad id %i\n", id);
		return false;
	}

	if (cl_debug.integer)
	{
		Console_DPrintf("Id = %i\n", id);
	}

	state = &lc.stateStrings[id];	
	
	Pkt_ReadString(pkt, string, MAX_PACKET_SIZE);
	Client_SetStateString(id, string);

	return true;
}


/*==========================

  Client_SNET_BIG_STRING_START

 ==========================*/

bool	Client_SNET_BIG_STRING_START(packet_t *pkt)
{
	int id;

	if (cl_debug.integer)
	{
		Console_DPrintf("SNET_BIG_STRING_START\n");
	}

	id = Pkt_ReadShort(pkt);
	if (id < 0 || id >= MAX_STATE_STRINGS)
	{
		Console_Printf("SNET_BIG_STRING_START: bad id %i\n", id);
		return false;
	}

	if (cl_debug.integer)
	{
		Console_DPrintf("Id = %i\n", id);
	}

	lc.bigstringIndex = id;
	lc.bigstringLength = Pkt_ReadInt(pkt);
	if (lc.bigstringBuffer)
	{
		if (lc.bigstringLength + 1 > lc.bigstringBufferSize)
		{
			lc.bigstringBuffer = Tag_Realloc(lc.bigstringBuffer, lc.bigstringLength + 1, MEM_CLIENT);
			lc.bigstringBufferSize = lc.bigstringLength + 1;
		}
	}
	else
	{
		lc.bigstringBuffer = Tag_Malloc(lc.bigstringLength + 1, MEM_CLIENT);
		lc.bigstringBufferSize = lc.bigstringLength + 1;
	}

	return true;
}


/*==========================

  Client_SNET_BIG_STRING_CONTINUE

 ==========================*/

bool	Client_SNET_BIG_STRING_CONTINUE(packet_t *pkt)
{
	int id;
	int offset, size;
	int numread;

	if (cl_debug.integer)
	{
		Console_DPrintf("Server: SNET_BIG_STRING_CONTINUE\n");
	}

	id = Pkt_ReadShort(pkt);
	if (id < 0 || id >= MAX_STATE_STRINGS)
	{
		Console_Printf("SNET_BIG_STRING_CONTINUE: bad id %i\n", id);
		return false;
	}

	if (cl_debug.integer)
	{
		Console_DPrintf("Id = %i\n", id);
	}

	if (id != lc.bigstringIndex)
	{
		Console_Printf("Received unordered SNET_BIG_STRING_CONTINUE message\n");
		return false;		//we weren't expecting to receive this message
	}

	offset = Pkt_ReadInt(pkt);
	size = Pkt_ReadShort(pkt);
	numread = Pkt_Read(pkt, &lc.bigstringBuffer[offset], size);
	if (numread != size)
	{
		Console_Printf("Big string: numread != size\n");
		return false;
	}

	if (lc.bigstringBuffer[offset + numread - 1] == 0)
	{
		//string was null terminated, so the update has ended

		Client_SetStateString(id, lc.bigstringBuffer);
	}

	return true;
}

/*==========================

  Client_GetRealPlayerState

 ==========================*/

void	Client_GetRealPlayerState(playerState_t *ps)
{
	if (!ps)
		return;

	//copy only the communicated portion of the playerstate over, so that the client can preserve locally maintained fields
	Net_CopyDeltaStruct(ps, &lc.playerState, playerStateDesc);
	//.memcpy(ps, &lc.playerState, player
	//memcpy(ps, &lc.playerState, sizeof(playerState_t));
	memcpy(ps->events, lc.playerState.events, sizeof(lc.playerState.events));
	ps->numEvents = lc.playerState.numEvents;
}

void	Client_SendNetSettings()
{
	char settings[768] = "";
	
	if (!Cvar_GetNetSettings(settings, 768))
	{
		Game_Error("Net settings overflowed send buffer");
	}

	Pkt_WriteCmd(&ncLocalClient.reliableSend, CNET_NETSETTINGS);
	Pkt_WriteString(&ncLocalClient.reliableSend, settings);

	cvar_netSettingsModified = false;
}



/*==========================

  Client_SNET_STARTING_STATE_STRINGS

  The server is sending us all its current state strings and forcing us to wait for them until we can do anything else

  When it has sent them all to us, we'll get an SNET_FINISHED_STATE_STRINGS message.
  At this point, we can look at the ST_SERVER_INFO string to determine what
  world to load.  

 ==========================*/

bool	Client_SNET_STARTING_STATE_STRINGS(packet_t *pkt)
{
	if (cl_debug.integer) {
		Console_DPrintf("Server: SNET_STARTING_STATE_STRINGS\n");
	}

	if (lc.cstate != CCS_AWAITING_STATE_STRINGS)
	{
		Game_Error("Got unexpected SNET_STARTING_STATE_STRINGS message\n");
	}

	//clear any previous state strings
	Client_ClearStateStrings();

	return true;
}

/*==========================

  Client_SNET_FINISHED_STATE_STRINGS

  the server has finished sending all its state string data on an initial connect (or restart)

  load the world specified in the server info state string, then send a message indicating that we're loaded
  

 ==========================*/

bool	Client_SNET_FINISHED_STATE_STRINGS(packet_t *pkt)
{
	char *worldname;
	char *serverinfo = lc.stateStrings[ST_SERVER_INFO].string;

	if (cl_debug.integer) {
		Console_DPrintf("Server: SNET_FINISHED_STATE_STRINGS\n");
	}

	if (lc.cstate != CCS_AWAITING_STATE_STRINGS)
	{
		Game_Error("Got unexpected SNET_FINISHED_STATE_STRINGS message\n");
	}

	if (!serverinfo)
		Game_Error("Didn't get a server info string!\n");

	lc.clientnum = Pkt_ReadByte(pkt);

	worldname = ST_GetState(serverinfo, "svr_world");
	localClient.xorbits = Pkt_ReadInt(pkt);
	Console_DPrintf("xorbits are %i\n", localClient.xorbits);

	if (!worldname[0])
		Game_Error("Didn't get a valid world name from server (string was: %s)\n", serverinfo);

	//if (!World_Exists(worldname))
	//{

	lc.worldPending = true;		//world is about to be loaded

	lc.cstate = CCS_LOADING;
	/*}
	else
	{
   		localClient.cstate = CCS_HASHCOMPARE;

		File_GetHashStrings(hashstring, 4096, localClient.xorbits);
		    
		Pkt_WriteCmd(&ncLocalClient.reliableSend, CNET_HASHCOMPARE);
		Pkt_WriteString(&ncLocalClient.reliableSend, hashstring);
	}
	*/

	return true;
}
/*
bool	Client_SNET_FINISHED_HASHSTRINGS(packet_t *pkt)
{
   	lc.cstate = CCS_LOADING;	

	if (Client_RestartWorld(lc.softRestart))
	{
		//tell the server we're ready, along with a hash value for the world (to help prevent cheating)
		Pkt_WriteCmd(&ncLocalClient.reliableSend, CNET_OK_TO_START);
	}

	return true;
}
*/



/*==========================

  Client_EnterGame

  called right after the first SNET_FRAME message

 ==========================*/

void	Client_EnterGame()
{
	if (lc.clientnum < 0 || lc.clientnum > MAX_CLIENTS)
	{
		Game_Error("We have an invalid client number\n");
	}

	Sound_StopAllSounds();
	Input_ClearKeyStates();

	if (demo.playing && demo.startTime == -1)
		demo.startTime = System_Milliseconds();		//set performance test start time

	//cl_api.EnterGame();

	lc.cstate = CCS_IN_GAME;	
}

/*==========================

  Client_SNET_GOOD_TO_GO

  we can now safely enter the game

  this is also where the server tells us our client number

  client number will remain the same for as long as we are connected to a server,
  even through restarts

 ==========================*/
/*
bool	Client_SNET_GOOD_TO_GO(packet_t *pkt)
{
	if (lc.cstate != CCS_LOADING)
		Game_Error("Got unexpected SNET_GOOD_TO_GO message\n");		

	Client_EnterGame();

	return true;
}
*/



unsigned int Client_GetCurrentInputStateNum()
{
	return lc.inputStateNum-1;
}

void	Client_GetInputState(unsigned int num, inputState_t *is)
{
	inputState_t *inp = &lc.inputStates[num % INPUTSTATE_BUFFER_SIZE];

	Mem_Copy(is, inp, sizeof(inputState_t));
}

void	Client_SendCvarPtr(cvar_t *var)
{
	Pkt_WriteCmd(&ncLocalClient.send, SNET_CVAR);
	Pkt_WriteString(&ncLocalClient.send, var->name);
	Pkt_WriteString(&ncLocalClient.send, var->string);
}



void	Client_StartRecording(const char *filename)
{
	int id;	

	if (localClient.cstate < CCS_IN_GAME)
	{
		Console_Printf("Can't record demo unless connected to a game\n");
		return;
	}

	demo.file = File_Open(filename, "wb");

	if (!demo.file)
	{
		Console_Printf("Couldn't open %s for demo recording\n", filename);
		return;
	}


	Console_Printf("Opened %s\n", filename);	

	//write demo header

	File_Write("s2demo", 6, 1, demo.file);
	//write net protocol and game version
	File_WriteInt(demo.file, NET_PROTOCOL_VERSION);
	File_WriteInt(demo.file, strlen(GAME_VERSION)+1);
	File_Write(GAME_VERSION, strlen(GAME_VERSION)+1, 1, demo.file);
	File_WriteInt(demo.file, localClient.clientnum);

	//write all state strings
	for (id=0; id<MAX_STATE_STRINGS; id++)
	{
		stateString_t *state;

		state = &lc.stateStrings[id];

		if (state->string)
		{
			File_WriteInt(demo.file, id);
			File_WriteInt(demo.file, state->length+1);
			File_Write(state->string, strlen(state->string)+1, 1, demo.file);
		}
	}

	File_WriteInt(demo.file, -1);	

	demo.recording = true;
	demo.framecount = 0;
	demo.startTime = Host_Milliseconds();		//NOTE: playback uses System_Milliseconds for performance testing, while recording uses host time
	demo.waiting = 2;

	Console_Printf("Wrote demo header.  Waiting for update...\n");
	Console_Printf("(Type \"demostop\" to stop the recording)\n");

	//now set our last valid frame to 0
	//this will tell the server to send over a full update
	//once we receive the full update we can begin recording
	
	//also clear out our frame history, because we don't want
	//to be sent a delta update for a frame the demo file
	//won't have
	lc.lastValidFrame = 0;
	memset(&lc.frames, 0, sizeof(lc.frames));
}


void	Client_StopPlayingDemo()
{
	if (!demo.file)
		return;

	if (demo.playing)
	{
		File_Close(demo.file);
		Console_Printf("Finished demo\n");
		memset(&demo, 0, sizeof(demo));
	}
}
void	Client_StopRecording()
{
	if (!demo.file)
		return;

	if (demo.recording)
	{		
		File_WriteByte(demo.file, DEMOCMD_EOF);
		File_Close(demo.file);
		Console_Printf("Stopped recording demo\n");
		memset(&demo, 0, sizeof(demo));
	}
}

void	Client_DemoStop_Cmd(int argc, char *argv[])
{	
	if (demo.recording)
		Client_StopRecording();
	else if (demo.playing)
	{
		Client_Disconnect();
	}
}

void	Client_DemoRecord_Cmd(int argc, char *argv[])
{
	char filename[256];	

	if (demo.file)
	{
		Console_Printf("Demo already recording!\nType \"demostop\" to stop recording\n");
		return;
	}	

	if (!argc)
	{
		File_GetNextFileIncrement(4, "demos/demo", "demo", filename, 255);
	}
	else
	{
		strncpy(filename, fmt("demos/%s.demo", argv[0]), 255);
	}

	//create the demo directory if it doesn't exist
	System_CreateDir(Filename_GetDir(filename));

	Client_StartRecording(filename);
}

void	Client_Prep();

extern bool host_developer_mode;

void	Client_DemoPlay_Cmd(int argc, char *argv[])
{
	char *filename;
	char header[5];
	int ver, clientnum;
	char gamever[256];
	int gamever_len;

	if (localClient.cstate)
	{
		Console_Printf("You must be disconnected before playing a demo\n");
		return;
	}

	if (!argc)
	{
		Console_Printf("syntax: demoplay filename.demo\n\n");
		Cmd_Exec("dir demos\n");
		return;
	}

	filename = fmt("demos/%s", argv[0]);
	demo.file = File_Open(filename, "rb");

	if (!demo.file && !(demo.file = File_Open(fmt("demos/%s.demo", argv[0]), "rb")))
	{
		Console_Printf("%s not found\n", filename);
		return;
	}

	//check demo header
	File_Read(header, 6, 1, demo.file);
	ver = File_ReadInt(demo.file);

	if (memcmp(header, "s2demo", 6)!=0)
	{
		Console_Printf("Invalid demo file\n");
		goto bad_demo;
	}
	if (ver != NET_PROTOCOL_VERSION)
	{
		Console_Printf("Sorry, demo version %i not supported by %s\n", ver, GAME_VERSION);
		goto bad_demo;
	}

	gamever_len = File_ReadInt(demo.file);
	if (gamever_len < 0 || gamever_len > 256)
	{
		Console_Printf("Invalid game version in demo file\n");
		goto bad_demo;
	}

	File_Read(gamever, gamever_len, 1, demo.file);
	Console_Printf("Playing demo recorded with %s\n", gamever);

	clientnum = File_ReadInt(demo.file);

	demo.playing = true;

	//prime the client state
	host_developer_mode = true;
	Cvar_AllowCheats();
	Client_Reset();

	localClient.cstate = CCS_LOADING;

	//set state strings
	while(1)
	{		
		char *s;
		int id = File_ReadInt(demo.file);
		int len;

		if (id == -1)
			break;

		len = File_ReadInt(demo.file);

		if (id < 0 || id >= MAX_STATE_STRINGS || len < 0 || len > 65536)
		{
			Console_Printf("Bad string data\n");
			Client_Disconnect();
			goto bad_demo;
		}

		s = Tag_Malloc(len, MEM_CLIENT);		
		File_Read(s, len, 1, demo.file);
		Client_SetStateString(id, s);
		Tag_Free(s);		
	}

	lc.clientnum = clientnum;

	Client_Prep();

	demo.startTime = -1;
	demo.time = 0;
	demo.framecount = 0;
	demo.speed = argc > 1 ? atof(argv[1]) : 1;

	return;

bad_demo:
	File_Close(demo.file);
	demo.file = NULL;
	return;
}


bool	Client_IsPlayingDemo()
{
	return demo.playing;
}


/*==========================

  Client_ReadOutOfBandPackets

  Read packets coming from outside our server connection (master browser)

 ==========================*/

void	Client_ReadOutOfBandPackets()
{
	while (Net_ReceivePacket(&ncLocalBrowser) > 0
			&& Net_PreProcessPacket(&ncLocalBrowser))
	{
		byte cmd;

		while (!Pkt_DoneReading(&ncLocalBrowser.recv))
		{
			cmd = Pkt_ReadCmd(&ncLocalBrowser.recv);

			switch(cmd)
			{
				case CPROTO_PING_ACK:
					Console_Printf("ping from %s is %i\n", ncLocalBrowser.recvAddrName, (System_Milliseconds() - Pkt_ReadInt(&ncLocalBrowser.recv)));
					break;
				case CPROTO_INFO:
					Console_Printf("info from %s is %s\n", ncLocalBrowser.recvAddrName, Host_ParseServerInfo(ncLocalBrowser.recvAddrName, ntohs(ncLocalBrowser.recvAddr.sin_port), &ncLocalBrowser.recv));
					break;
				case CPROTO_EXT_INFO:
					Console_Printf("extended info from %s is %s\n", ncLocalBrowser.recvAddrName, Host_ParseExtServerInfo(ncLocalBrowser.recvAddrName, ntohs(ncLocalBrowser.recvAddr.sin_port), &ncLocalBrowser.recv));
					break;
				default:
					Console_DPrintf("Unknown packet type on the browser socket\n");
					break;
			}
		}
	}
}



static char denyReason[1024];
//the local client cookie
char clientcookie[COOKIE_SIZE+1];


/*==========================

  Client_CheckConnectionResponse

  //returns:
	//0 == waiting for connection
	//1 == connection accepted
	//-1 == connection denied


 ==========================*/

extern int client_id;

int	Client_CheckConnectionResponse()
{
	while (Net_ReceivePacket(&ncLocalClient) > 0)
	{
		byte cmd;

		if (Net_PreProcessPacket(&ncLocalClient))
		{
			cmd = Pkt_ReadCmd(&ncLocalClient.recv);
			if (cmd == CPROTO_OK)
			{				
				int oldclientid = client_id;		//hack to maintain compatibility with old protocol
				client_id = -1;
				
				Pkt_Clear(&ncLocalClient.send);
				Pkt_WriteCmd(&ncLocalClient.send, CPROTO_LEARNPORT);
				Pkt_WriteString(&ncLocalClient.send, clientcookie);
				Net_SendPacket(&ncLocalClient);

				client_id = oldclientid;

				return 1;
			}
			else if (cmd == CPROTO_NO)
			{
				Pkt_ReadString(&ncLocalClient.recv, denyReason, 1024);

				return -1;
			}
		}
	}

	return 0;
}


/*==========================

  Client_Connecting

  We're establishing a connection to the server

 ==========================*/

void	Client_Connecting()
{
	int response = Client_CheckConnectionResponse();

	switch(response)
	{
		case 1:		//connection accepted
		{
			//prep us for loading

			//clear any previous data and initialize client game code
			Client_Reset();
			//tell the server our network settings (this includes our name)
			Client_SendNetSettings();
			Net_SendPacket(&ncLocalClient);

			//tell the server to send us state strings
			lc.cstate = CCS_AWAITING_STATE_STRINGS;
			Pkt_WriteCmd(&ncLocalClient.reliableSend, CNET_REQUEST_STATE_STRINGS);
			Net_SendPacket(&ncLocalClient);

			break;
		}		
		case -1:	//connection denied
		{
			Console_Errorf("Game server rejected access - %s\n", denyReason);
			Host_Disconnect();
			break;
		}			
		case 0:		//no response yet
		{
			if (System_Milliseconds() - lc.conn_start > net_connectTimeout.value)
			{
				Console_Errorf("Connect: giving up after %.0f seconds\n", net_connectTimeout.value / 1000.0);
				Host_Disconnect();
			}
			break;
		}
	}
}





/*==========================

  Client_ReadGameData

  Handle all game commands

 ==========================*/

void	Client_ReadGameData()
{
	int numFrames = 0;

	while (Net_ReceivePacket(&ncLocalClient) > 0
			&& Net_PreProcessPacket(&ncLocalClient))
	{
		byte cmd;
		bool ret = false;

		netStats.clBytesInAccum += ncLocalClient.recv.length;

demo_record:
		if (demo.recording && !demo.waiting)
		{			
			int len = ncLocalClient.recv.length + HEADER_SIZE;
			File_WriteByte(demo.file, DEMOCMD_PACKET);
			File_WriteInt(demo.file, demo.framecount);
			File_WriteInt(demo.file, Host_Milliseconds() - demo.startTime);
			File_WriteInt(demo.file, len);
			File_Write(&ncLocalClient.recv.__buf, ncLocalClient.recv.length + HEADER_SIZE, 1, demo.file);			
		}
		
		while (!Pkt_DoneReading(&ncLocalClient.recv))
		{
			cmd = Pkt_ReadCmd(&ncLocalClient.recv);

			switch(cmd)
			{
				case SNET_FRAME:
				{
					numFrames++;

					ret = Client_SNET_FRAME(&ncLocalClient.recv);
					if (demo.waiting == 1 && demo.recording)
					{
						demo.waiting = 0;
						goto demo_record;
					}
					break;
				}
				case SNET_FRAME_CONTINUED:					
					ret = Client_SNET_FRAME_CONTINUED(&ncLocalClient.recv);
					break;
				case SNET_CVAR:
					ret = Client_SNET_CVAR(&ncLocalClient.recv);
					break;
				case SNET_MESSAGE:
					ret = Client_SNET_MESSAGE(&ncLocalClient.recv);
					break;
				case SNET_DISCONNECTING:
					ret = Client_SNET_DISCONNECTING(&ncLocalClient.recv);
					break;
				case SNET_KICK:
					ret = Client_SNET_KICK(&ncLocalClient.recv);
					break;
				case SNET_UPDATE_OBJECTS:
					ret = Client_SNET_UPDATE_OBJECTS(&ncLocalClient.recv);
					break;
				case SNET_UPDATE_PLAYERSTATE:
					ret = Client_SNET_UPDATE_PLAYERSTATE(&ncLocalClient.recv);
					break;
				case SNET_SMALL_STRING:
					ret = Client_SNET_SMALL_STRING(&ncLocalClient.recv);
					break;
				case SNET_BIG_STRING_START:
					ret = Client_SNET_BIG_STRING_START(&ncLocalClient.recv);
					break;
				case SNET_BIG_STRING_CONTINUE:
					ret = Client_SNET_BIG_STRING_CONTINUE(&ncLocalClient.recv);
					break;
				case SNET_STARTING_STATE_STRINGS:
					ret = Client_SNET_STARTING_STATE_STRINGS(&ncLocalClient.recv);
					break;
				case SNET_FINISHED_STATE_STRINGS:
					ret = Client_SNET_FINISHED_STATE_STRINGS(&ncLocalClient.recv);
					break;/*
				case SNET_FINISHED_HASHCOMPARE:
					ret = Client_SNET_FINISHED_HASHSTRINGS(&ncLocalClient.recv);
					break;*//*
				case SNET_GOOD_TO_GO:
					ret = Client_SNET_GOOD_TO_GO(&ncLocalClient.recv);
					break;*/
				default:
					ret = false;
					break;
			}

			if (!ret)
			{				
				FILE *debugpacket = fopen("badpacket.dat", "wb");
				fwrite(ncLocalClient.recv.__buf, ncLocalClient.recv.length + HEADER_SIZE, 1, debugpacket);
				fclose(debugpacket);

				if (cl_ignoreBadPackets.integer)
				{
					Console_Printf("*** WARNING: BAD PACKET DATA RECEIVED FROM SERVER (cmd %i) ***\n", cmd);
					//we can't reliably parse the rest of the packet, so clear it
					Pkt_Clear(&ncLocalClient.recv);
					break;
				}
				else
				{
					Game_Error(fmt("Invalid packet data received from server (cmd was %i)\n\nPacket dumped to game\\badpacket.dat\n", cmd));
				}				
				//Pkt_Clear(&ncLocalClient.recv);
			}
		}
	}

	if (demo.recording || demo.playing)
		demo.framecount++;

	if (numFrames > 1)
	{
		Console_DPrintf("%i SNET_FRAME packets received from server in one client frame\n", numFrames);
	}
}


/*==========================

  Client_ReadServerPackets

  Read incoming data from the server

 ==========================*/

void	Client_ReadServerPackets()
{
	if (!lc.cstate)
		return;					//not connected

	if (lc.cstate == CCS_CONNECTING)	
	{
		Client_Connecting();
		return;
	}

	Client_ReadGameData();
}




/*==========================

  Client_AddInputState

  add the inputstate into a circular buffer  

 ==========================*/

void	Client_AddInputState()
{
	inputState_t *inp = &lc.inputStates[lc.inputStateNum % INPUTSTATE_BUFFER_SIZE];
	int deltatime = Host_FrameMilliseconds();
	
	if (deltatime > 255)	//running at < 4fps
	{
		Console_DPrintf("deltatime > 255\n");
		deltatime = 255;
	}
	
	Input_GetInputState(inp);

	inp->sequence = lc.inputStateNum;	
	//inp->gametime = (int)lc.gametime;		//this allows the server to advance the playerstate the correct amount
	inp->delta_msec = (byte)deltatime;
	inp->gametime = lc.server_timestamp + lc.totaldelta;

	lc.totaldelta += deltatime;

	if (cl_debugInputs.integer)
	{
		Console_Printf("LOCAL INPUT: pitch: %i  yaw: %i  mvmt: %i  gametime: %i  delta: %i\n", 						
						inp->pitch,
						inp->yaw,
						inp->movement,
						inp->gametime,
						inp->delta_msec);
	}

	lc.inputStateNum++;
}

void	Client_EditorFrame()
{
	/*if (!world.cl_loaded)
		return;
		*/

	lc.gametime += Host_FrameMilliseconds();

	cl_api.Frame((int)lc.gametime);
}


/*==========================

  Client_WriteInputCommand

 ==========================*/

void	Client_WriteInputCommand()
{
	int n;
	structDiff_t diff;
	inputState_t *firstInput = &lc.inputStates[lc.firstInputNum % INPUTSTATE_BUFFER_SIZE];
	inputState_t *prevInp;	
	packet_t *pkt = &ncLocalClient.send;
	unsigned int numInputs;

	numInputs = lc.inputStateNum - lc.firstInputNum;

	if (numInputs <= 0)
	{
		return;
	}

	Pkt_WriteCmd(pkt, CNET_INPUTSTATE);
	Pkt_WriteInt(pkt, (demo.waiting == 2 || cl_noDelta.integer) ? 0 : lc.lastValidFrame);

	//send num inputs this frame
	Pkt_WriteByte(pkt, (byte)numInputs);
	//send inputstate beginning sequence
	Pkt_WriteInt(pkt, firstInput->sequence);
	//send inputstate beginning gametime
	Pkt_WriteInt(pkt, firstInput->gametime);

	//send all the inputs we have buffered up to this point
	prevInp = &nullinput;

	for (n=lc.firstInputNum; n<lc.inputStateNum; n++)
	{
		inputState_t *inp = &lc.inputStates[n % INPUTSTATE_BUFFER_SIZE];

		Net_GetStructDiff(prevInp, inp, inputStateDesc, &diff);
		Net_WriteStructDiff(pkt, inp, inputStateDesc, &diff);

		prevInp = inp;
	}

	lc.firstInputNum = lc.inputStateNum;
}


/*==========================

  Client_SendPackets

 ==========================*/

void	Client_SendPackets()
{
	if (!lc.cstate)
		return;

	if (Host_Milliseconds() - lc.lastSendTime < 1000 / cl_packetSendFPS.value)
		return;

	Net_CheckPacketTimeouts(&ncLocalClient);

	if (lc.cstate >= CCS_AWAITING_FIRST_FRAME)
	{
		Client_WriteInputCommand();
	}

	netStats.clBytesOutAccum += ncLocalClient.send.length;
	netStats.clBytesOutAccum += ncLocalClient.reliableSend.length;

	if (ncLocalClient.send.length)
	{
		if (lc.cstate == CCS_DISCONNECTED
			|| lc.cstate == CCS_AUTHENTICATING)
		{
			Console_DPrintf("Not connected to server\n");
			Pkt_Clear(&ncLocalClient.send);
		}
		else
		{		  
			if (cl_showPackets.integer)
				Console_Printf("%i ", ncLocalClient.send.length);
			Net_SendPacket(&ncLocalClient);
			lc.lastSendTime = Host_Milliseconds();
		}
	}
	if (ncLocalClient.reliableSend.length)
	{
		if (lc.cstate == CCS_DISCONNECTED
			|| lc.cstate == CCS_AUTHENTICATING)
		{
			Console_DPrintf("Not connected to server\n");
			Pkt_Clear(&ncLocalClient.reliableSend);
		}
		else
		{
			if (cl_showPackets.integer)
				Console_Printf("RELIABLE: (%i) ", ncLocalClient.reliableSend.length);

		 	Net_SendReliablePacket(&ncLocalClient);
			lc.lastSendTime = Host_Milliseconds();
		}
	}

	//clear playerstate events, assume game code has handled them
//	lc.playerState.numEvents = 0;
}

void	Client_ModifyInputStateItem(byte item)		//right now 'item' is the only thing we need to modify
{
	inputState_t *inp = &lc.inputStates[(lc.inputStateNum-1) % INPUTSTATE_BUFFER_SIZE];

	inp->item = item;
}

void	Client_ModifyInputStateAngles(short pitch, short yaw)
{
	inputState_t *inp = &lc.inputStates[(lc.inputStateNum-1) % INPUTSTATE_BUFFER_SIZE];

	inp->pitch = pitch;
	inp->yaw = yaw;
}


/*==========================

  Client_GameFrame

 ==========================*/

void	Client_GameFrame()
{
	static int lastgametime;		//for debug

	if (!lc.cstate)
		return;
	if (!world.cl_loaded)		//this can happen on a restart of the local server
		return;

	if (lc.cstate >= CCS_AWAITING_FIRST_FRAME)
	{
		//let the client code know about the current input state

		Client_AddInputState();
	}

	if (lc.cstate == CCS_IN_GAME)
	{			
		//handle client gametime
		//we fudge a bit beyond the server time window so the client can keep a smooth timer going on laggy servers

		if (lc.last_timestamp && lc.server_timestamp)		//make sure we have two consecutive server frames before clamping the time
		{
			lastgametime = lc.gametime;

			if (lc.gametime && !(demo.playing && demo.paused))
				lc.gametime += Host_FrameMilliseconds();

			if (lc.gametime > lc.server_timestamp+cl_lagFudgeTime.integer)		//don't let gametime get too far beyond the server time
			{
				if (cl_debugTimer.integer)
					Console_Printf("gametime > server_timestamp (%i)\n", lc.gametime - lc.server_timestamp);
				lc.gametime = lc.server_timestamp+cl_lagFudgeTime.integer;
			}
			else if (lc.gametime < lc.last_timestamp)
			{
				if (cl_debugTimer.integer)
					Console_Printf("gametime < last_timestamp (%i)\n", lc.last_timestamp - lc.gametime);
				lc.gametime = lc.last_timestamp;
			}

			if (cl_debugTimer.integer)
			{
				if (lastgametime > lc.gametime)
				{
					Console_Printf("backwards gametime (%i)\n", lastgametime - lc.gametime);
				}
			}
		}
	
		if (lc.connectionProblemsStartTime + net_problemIconTime.integer < Host_Milliseconds())
		{
			lc.connectionProblems = false;
		}

		//run game code frame

		cl_api.Frame((int)lc.gametime);		
	}

	if (cvar_netSettingsModified)
	{
		//send over any changes in the CVAR_NETSETTING cvars

		Client_SendNetSettings();
		cvar_netSettingsModified = false;
	}
}


/*=========================

  Client_PrepareForFirstFrame

  prepare for the first SNET_FRAME message

  this will happen during an initial connect or a restart

 =========================*/

void	Client_PrepareForFirstFrame()
{
	int id;
	char *serverinfo = lc.stateStrings[ST_SERVER_INFO].string;

	if (!serverinfo)		//sanity check
		Game_Error("No server info\n");		
	
	//clear playerstate and inputstates, since we'll be getting a full update
	memset(&lc.playerState, 0, sizeof(lc.playerState));
	memset(&lc.inputStates, 0, sizeof(lc.inputStates));

	lc.inputStateNum = 0;
	lc.firstInputNum = 0;
	lc.lastValidFrame = 0;
	lc.fromFrame = NULL;
	lc.framenum = 0;

	lc.gametime = 0;
	lc.server_timestamp = 0;
	lc.last_timestamp = 0;
		
	//prime the game code state
	cl_api.Restart();

	//call StateStringModified for all the server state strings so the client game code is all synced up
	for (id=0; id<MAX_STATE_STRINGS; id++)
	{
		if (lc.stateStrings[id].string)
		{
			if (cl_api.StateStringModified)
				cl_api.StateStringModified(id);
		}
	}

	cl_api.PrecacheResources();		//this function may take a while to complete

	lc.cstate = CCS_AWAITING_FIRST_FRAME;
}


/*==========================

  Client_Prep

  called right after the content downloading phase completes

 ==========================*/

void	Client_Prep()
{
	char hashstring[4096] = "";
	char *serverinfo = localClient.stateStrings[ST_SERVER_INFO].string;
	char *worldName = ST_GetState(serverinfo, "svr_world");

	//loading music here?

	if (lc.softRestart)
	{			
		//do a few cleanups but don't reload the whole world

		int n;

		//delete static surfaces that were allocated during the game
		if (!localServer.active)
		{
			for (n=0; n<MAX_OBJECTS; n++)
			{
				WO_DeleteObject(n);
			}
		}

		//clear dynamic objects
		WT_ResetDynamic(true);	
	}
	else
	{
		if (!World_Load(worldName, true))
			Game_Error("Error loading world\n");

		//on the initial connection to the server, reset GUI and call CL_Init()
		if (lc.initialConnect)
		{
			GUI_Reset();
			int_api.Init();
			int_api.Restart();
			cl_api.Init();
	
			lc.initialConnect = false;
		}
	}



	localClient.worldPending = false;

	//server expects a hash now	
  	
	Pkt_WriteCmd(&ncLocalClient.reliableSend, CNET_OK_TO_START);
	//write world hash
	Pkt_WriteArray(&ncLocalClient.reliableSend, world.s2z_hash, world.s2z_hashlen);
	//write archive hashes
	File_GetHashStrings(hashstring, 4096, localClient.xorbits);
	Pkt_WriteString(&ncLocalClient.reliableSend, hashstring);

	Client_PrepareForFirstFrame();
}




/*==========================

  Client_DownloadContent

  returns true if more content needs to be downloaded, false if no more content needs to be downloaded

 ==========================*/

bool	Client_DownloadContent(int *size, int *bytesTransferred, int *time)
{
	char *serverinfo = localClient.stateStrings[ST_SERVER_INFO].string;
	char *worldName = NULL;
	char *currentFile = NULL;
	char *tmp, *end, *slash, *requiredArchives;
	char currentLocalFilename[512] = {0};
	int res;
	bool haveAllRequiredArchives = false;
	float progress;
	char url[1024];

	*size = 0;
	*bytesTransferred = 0;
	*time = 0;
	
	worldName = ST_GetState(serverinfo, "svr_world");
	strncpySafe(url, ST_GetState(serverinfo, "svr_mapurl"), sizeof(url)-1);
	if (url[strlen(url)] != '/')
		strcat(url, "/");

	if (!url[0])
	{				
		//don't download anything, just attempt to load the files
		return false;
	}
	
	if (url[0])
	{		
		//download the overhead map?

		if (!cl_currentContentDownload.string[0] || strcmp(cl_currentContentDownload.string, "overhead_map") == 0)
		{			
			currentFile = fmt("%s%s_overhead.jpg", url, worldName);
			if ((!File_Exists(fmt("world/%s_overhead.tga", worldName))
				&& !File_Exists(fmt("world/%s_overhead.jpg", worldName))
				&& !File_Exists(fmt("world/%s_overhead.png", worldName)))
				|| lc.gettingFile
				)
			{
				lc.gettingFile = true;
				res = HTTP_GetFileNonBlocking(currentFile, fmt("world/%s_overhead.jpg", worldName));				
				if (res == 1)
				{
					Console_Printf("We have the overhead map for world %s\n", worldName);
					Cvar_SetVar(&cl_currentContentDownload, fmt("map (%s)", worldName));
					lc.gettingFile = false;

					return true;
				}
				else if (res == 0)
				{
					Console_Printf("Failed to get overhead map for the %s world from the Map URL\n", worldName);
					Cvar_SetVar(&cl_currentContentDownload, fmt("map (%s)", worldName));
					lc.gettingFile = false;

					return true;		//keep downloading, the overhead map isn't required to play
				}
				else
				{
					progress = HTTP_GetProgress(currentFile, size, bytesTransferred, time);
					Cvar_SetVarValue(&cl_currentContentDownloadProgress, progress);

					return true;
				}
			}
			else
			{
				Console_Printf("We already have the overhead map for world %s\n", worldName);
				Cvar_SetVar(&cl_currentContentDownload, fmt("map (%s)", worldName));
				
				//just continue immediately to the map downloading
			}
		}

		//download the s2z file?
		if (strcmp(cl_currentContentDownload.string, fmt("map (%s)", worldName)) == 0)
		{
			currentFile = fmt("%s%s.s2z", url, worldName);
			strncpySafe(currentLocalFilename, fmt("world/%s.s2z", worldName), 512);
			if (!File_Exists(currentLocalFilename) || lc.gettingFile)
			{				
				lc.gettingFile = true;
				res = HTTP_GetFileNonBlocking(currentFile, currentLocalFilename);
				if (res == 1)
				{
					//we're done downloading
					Cvar_SetVar(&cl_currentContentDownload, "");
					Cvar_SetVarValue(&cl_currentContentDownloadProgress, 1);					
					lc.gettingFile = false;

					return true;
				}
				else if (res == 0)
				{
					//download failed
					Cvar_SetVar(&cl_currentContentDownload, "");
					Cvar_SetVarValue(&cl_currentContentDownloadProgress, 0);					
					lc.gettingFile = false;

					//can't get the map, so disconnect
					Game_Error(fmt("Failed to download map %s from the server map URL (%s)", worldName, url));
				}
				else
				{
					progress = HTTP_GetProgress(currentFile, size, bytesTransferred, time);
					Cvar_SetVarValue(&cl_currentContentDownloadProgress, progress);

					return true;
				}
			}
			else
			{ 
				//we have this map already
				Cvar_SetVar(&cl_currentContentDownload, "");

				Cvar_SetVarValue(&cl_currentContentDownloadProgress, 1);		

				//just continue on to the archive downloading
			}
		}
	}
	
	/* now to request any required archives that we don't have */

	requiredArchives = ST_GetState(serverinfo, "svr_requiredArchives");
	if (requiredArchives[0] && url[0])
	{
		//set it to true, and if we don't have one set it to false
		haveAllRequiredArchives = true;
			
		tmp = requiredArchives;
		while (tmp)
		{
			slash = strchr(tmp, '/');
			end = strchr(tmp, ' ');
			if (!end)
				end = tmp + strlen(tmp);

			//rid ourselves of subdirs
			if (slash && slash < end)
				tmp = slash;
		
			strncpySafe(currentLocalFilename, fmt("/%s",tmp), MIN(512, end - tmp + 2));
	
			if (!strstr(currentLocalFilename, ".s2z")
				|| Archive_IsOfficialArchiveName(currentLocalFilename))
			{
				Console_Printf("Invalid server requiredArchive %s\n", currentLocalFilename);
			}
			else
			{
				if (strcmp(currentLocalFilename, cl_currentContentDownload.string) != 0
					&& File_Exists(currentLocalFilename))
				{
					Archive_RegisterArchive(currentLocalFilename, ARCHIVE_NORMAL | ARCHIVE_THIS_CONNECTION_ONLY);
				}
					
				if (!File_Exists(currentLocalFilename)
					|| !Archive_GetArchive(currentLocalFilename) || lc.gettingFile)
				{
					haveAllRequiredArchives = false;
					lc.gettingFile = true;
		
					currentFile = fmt("%s%s", url, currentLocalFilename);
					res = HTTP_GetFileNonBlocking(currentFile, currentLocalFilename);
					if (res == 1)
					{
						//we're done downloading
						Cvar_SetVar(&cl_currentContentDownload, "");
						Cvar_SetVarValue(&cl_currentContentDownloadProgress, 1);
						lc.gettingFile = false;
		
						//register this archive as an archive just for this connection
						if (!Archive_GetArchive(currentLocalFilename))
						{
							if (!Archive_RegisterArchive(currentLocalFilename, ARCHIVE_NORMAL | ARCHIVE_THIS_CONNECTION_ONLY))
							{
								Game_Error(fmt("Failed loading server-required archive %s\n", currentLocalFilename));
							}
						}

						return true;
					}
					else if (res == 0)
					{
						lc.gettingFile = false;
						//abort the connection
						Game_Error(fmt("Error trying to download required archive %s and save it to %s\n", currentFile, currentLocalFilename));
					}
					else
					{
						//just show the current progress
						progress = HTTP_GetProgress(currentFile, size, bytesTransferred, time);
						Cvar_SetVarValue(&cl_currentContentDownloadProgress, progress);
						Cvar_SetVar(&cl_currentContentDownload, currentLocalFilename);

						return true;
					}					
				}
				/*
				else
				{
					//in case we already had it, try registering it if it's not registered already
					if (!Archive_GetArchive(currentLocalFilename))
						Archive_RegisterArchive(currentLocalFilename, ARCHIVE_NORMAL | ARCHIVE_THIS_CONNECTION_ONLY);
				}
				*/
			}
	
			if (end && end[0])
				tmp = end + 1;
			else
				break;
		}
	}

	return false;
}

void	Client_Frame()
{
//	if (localClient.gametime > server_timestamp)
//		localClient.gametime = server_timestamp - 1;	

	OVERHEAD_INIT;

	localClient.framenum++;

	Input_Frame();

	if (lc.lastServerPacketTime + net_connectionProblemTimeThreshhold.value < Host_Milliseconds() && !demo.playing)
	{
		lc.connectionProblems = true;
		lc.connectionProblemsStartTime = Host_Milliseconds();
	}
	
	//process packets outside of our connection to the server
	Client_ReadOutOfBandPackets();
	
	Client_ReadServerPackets();

	if (localClient.worldPending)
	{
		int time, size, bytesTransferred;

		if (Client_DownloadContent(&size, &bytesTransferred, &time))
		{
			int_api.ShowProgressMeter(cl_currentContentDownload.string, cl_currentContentDownloadProgress.value, size, bytesTransferred, time); //show the progress
		}
		else
		{
			Cvar_SetVar(&cl_currentContentDownload, "");
			int_api.ShowProgressMeter(NULL, 0, 0, 0, 0); //aka hide it

			Client_Prep();
		}

		if (localClient.lastKeepalive + DOWNLOAD_KEEPALIVE_INTERVAL < System_Milliseconds())
		{
			Pkt_WriteCmd(&ncLocalClient.send, CNET_KEEPALIVE);
			localClient.lastKeepalive = System_Milliseconds();
		}
	}
	
	//run the "main interface" code all the time, even if we're disconnected
	int_api.Frame();

	//run game code
	if (DLLTYPE == DLLTYPE_EDITOR)
		Client_EditorFrame();
	else
		Client_GameFrame();

	//send packets to server
	Client_SendPackets();

	OVERHEAD_COUNT(OVERHEAD_CLIENT_FRAME);
}

int	Client_GetConnectionState()
{
	return lc.cstate;
}

void	Client_DrawForeground()
{
	OVERHEAD_INIT;
			
	if (lc.cstate == CCS_IN_GAME
		|| DLLTYPE == DLLTYPE_EDITOR)
		cl_api.DrawForeground();

	int_api.DrawForeground();

	OVERHEAD_COUNT(OVERHEAD_DRAWFOREGROUND);
}




extern bool net_initialized;


/*==========================

  Client_Connect

  happens after we have successfully authenticated

 ==========================*/

void	Client_Connect(char *addr)
{	
	int oldclientid;
	/*
	if (lc.cstate > CCS_DISCONNECTED)
	{
		Client_Disconnect();
	}
	*/

	if (!Net_InitNetConnection(&ncLocalClient, 0, 0))
		Game_Error("Couldn't initialize network");
	
	if (!Net_SetSendAddr(&ncLocalClient, addr, 0))
		Game_Error("Couldn't resolve server address");

	//start connecting to server
	lc.conn_start = System_Milliseconds();

 	lc.cstate = CCS_CONNECTING;

	Host_GetCookie(clientcookie, COOKIE_SIZE);
	clientcookie[COOKIE_SIZE] = 0;
				
	Pkt_Clear(&ncLocalClient.send);
	Pkt_WriteCmd(&ncLocalClient.send, CPROTO_CONNECT_ME);
	Pkt_WriteString(&ncLocalClient.send, "S2Connect");
	//write protocol version
	Pkt_WriteShort(&ncLocalClient.send, NET_PROTOCOL_VERSION);
	//write password
	Pkt_WriteString(&ncLocalClient.send, cl_password.string);
	//write cookie
	Pkt_WriteString(&ncLocalClient.send, clientcookie);
	//write client id
	Pkt_WriteShort(&ncLocalClient.send, client_id);
#ifdef SAVAGE_DEMO
	Pkt_WriteByte(&ncLocalClient.send, 1);
#else
	Pkt_WriteByte(&ncLocalClient.send, 0);
#endif

	ncLocalClient.reliableSend_lastseq = 0;
	ncLocalClient.reliableRecv_lastseq = 0;

	ncLocalClient.unack_packets = NULL;
	
	//hack to maintain compatibility with old protocol
	//(the server expects all CPROTO packets to have a client id of 0xffff)
	oldclientid = client_id;
	client_id = -1;	

	Net_SendPacket(&ncLocalClient);	

	client_id = oldclientid;
}



/*==========================

  Client_Disconnect

 ==========================*/

void	Client_Disconnect()
{
	extern cvar_t nextSessionCommand;

	if (DLLTYPE == DLLTYPE_EDITOR)	
		return;

	if (demo.recording)
		Client_StopRecording();		//stop recording demo
	else if (demo.playing)
		Client_StopPlayingDemo();	//stop playing demo	

	//notify the server that we're disconnecting
	if (lc.cstate >= CCS_CONNECTING)
	{
		//flush any incoming data from the net connection
		while (Net_ReceivePacket(&ncLocalClient) > 0 && Net_PreProcessPacket(&ncLocalClient));

		Pkt_Clear(&ncLocalClient.send);
		Pkt_Clear(&ncLocalClient.reliableSend);
		Pkt_Clear(&ncLocalClient.recv);

		Net_ClearReliablePackets(&ncLocalClient);	


		//try a few times
		Pkt_WriteCmd(&ncLocalClient.send, CNET_DISCONNECT);
		Net_SendPacket(&ncLocalClient);
		System_Sleep(5);
		Pkt_WriteCmd(&ncLocalClient.send, CNET_DISCONNECT);
		Net_SendPacket(&ncLocalClient);
		System_Sleep(5);
		Pkt_WriteCmd(&ncLocalClient.send, CNET_DISCONNECT);
		Net_SendPacket(&ncLocalClient);
		System_Sleep(5);

		Net_CloseNetConnection(&ncLocalClient);

		cl_api.Shutdown();

		World_Destroy();
		Sound_StopAllSounds();

		//int_api.Restart();
	}	

	int_api.Restart();

	//clear all data
	memset(&lc, 0, sizeof(lc));

	//free mem
	Tag_FreeAll(MEM_CLIENT);

	//make sure content downloads starts fresh next time we connect
	Cvar_SetVar(&cl_currentContentDownload, "");

	Archive_UnregisterUnusedArchives();

	Input_ClearKeyStates();

/*	if (nextSessionCommand.string[0])
	{
		Cmd_BufPrintf("%s\n", nextSessionCommand.string);
	}*/
}

char	*Client_StatusString()
{
	switch(lc.cstate)
	{
		case CCS_DISCONNECTED:
			return "CCS_DISCONNECTED";
		case CCS_AUTHENTICATING:
			return "CCS_AUTHENTICATING";
		case CCS_CONNECTING:
			return "CCS_CONNECTING";
		case CCS_LOADING:
			return "CCS_LOADING";
		case CCS_IN_GAME:
			return "CCS_IN_GAME";
		default:
			return "Unknown!!";
	}
}

 


/*==========================

  Client_SendMessageToServer

 ==========================*/

void	Client_SendMessageToServer(char *buf)
{
	Pkt_WriteCmd(&ncLocalClient.send, CNET_MESSAGE);
	Pkt_WriteString(&ncLocalClient.send, buf);
}


/*==========================

  Client_RequestStateString

  Request a string from the server that has its STATESTRING_REQUEST_ONLY flag set

 ==========================*/

void	Client_RequestStateString(int id, int num)
{
	if (id < 0 || id >= MAX_STATE_STRINGS)
		return;
	if (num <= 0)
		return;

	Pkt_WriteCmd(&ncLocalClient.send, CNET_MESSAGE);
	Pkt_WriteString(&ncLocalClient.send, fmt("ssrq %i %i", id, num));
}



/*void	Client_TeamDataPointer(void *base, int stride, int num_objects)
{
	int n, offset;

	offset = 0;
	if (!stride)
		System_Error("Client_TeamDataPointer: stride not specified\n");

	if (num_objects != MAX_TEAMS)
		System_Error("Client_TeamDataPointer: num_objects must be %i\n", MAX_TEAMS);

	for (n=0; n<num_objects; n++)
	{
		(char *)lc.teams[n] = (char *)base + offset;

		memset(lc.teams[n], 0, stride);

		offset += stride;
	}

	lc.num_teams = 0;
}
*/
int	Client_GetOwnClientNum()
{
	return lc.clientnum;
}



/*==========================

  Client_Reset

  Reinitialize the client on an initial connect

 ==========================*/

void	Client_Reset()
{
	baseObject_t		*objects[MAX_OBJECTS];
	int index;
	int oldstate;
	
	Console_Printf("Client_Reset()\n");

	//reset the out-of-order packet count
	netStats.clUnordered = 0;

	for (index = 0; index < MAX_OBJECTS; index++)
		objects[index] = lc.gameobjs[index];

	oldstate = lc.cstate;

	Client_ClearStateStrings();
	
	memset(&lc, 0, sizeof(localClientInfo_t));
	lc.cstate = oldstate;
	
	for (index = 0; index < MAX_OBJECTS; index++)
		lc.gameobjs[index] = objects[index];

	lc.initialConnect = true;
}

void	Client_StateStrings_Cmd(int argc, char *argv[])
{
	int n;

	for (n=0; n<MAX_STATE_STRINGS; n++)
	{
		if (lc.stateStrings[n].string)
			Console_Printf("%i: %s\n", n, lc.stateStrings[n].string);
	}
}

void	_EchoServerInfo(const char *key, const char *value)
{
	Console_Printf("%s     \"%s\"\n", key, value);
}

void	Client_ServerInfo_Cmd(int argc, char *argv[])
{
	if (!lc.stateStrings[ST_SERVER_INFO].string)
		return;

	Console_Printf("Server Info\n===========\n\n");

	ST_ForeachState(lc.stateStrings[ST_SERVER_INFO].string, _EchoServerInfo);
}

/*==========================

  Client_SvCmd_Cmd

  Execute a command on a remote server
  Syntax: svcmd <command>
  cl_adminPassword must be set to match the servers svr_adminPassword
  
 ==========================*/

void	Client_SvCmd_Cmd(int argc, char *argv[])
{
	int n = 0;
	int size = 0;
	char msg[CMD_MAX_LENGTH];	//use CMD_MAX_LENGTH so it holds any possible console line

	if (!argc)
		return;

	memset(msg, 0, CMD_MAX_LENGTH);
	ConcatArgs(argv, argc, msg);
	msg[1024] = 0;	// cut it off at whatever length we want

	Client_SendMessageToServer(fmt("svcmd %s %s", cl_adminPassword.string, msg));
}


void	Client_Init()
{
	Cvar_Register(&cl_name);
	Cvar_Register(&cl_bps);
	Cvar_Register(&cl_netFPS);
	Cvar_Register(&cl_maxPacketSize);
	Cvar_Register(&cl_debug);
	Cvar_Register(&cl_password);
	Cvar_Register(&cl_adminPassword);
	Cvar_Register(&cl_ambientSound);
	Cvar_Register(&cl_ambientVolume);
	Cvar_Register(&cl_music1);
	Cvar_Register(&cl_music2);
	Cvar_Register(&cl_music3);
	Cvar_Register(&cl_music4);
	Cvar_Register(&cl_music5);
	Cvar_Register(&cl_music6);
	Cvar_Register(&cl_showUnordered);
	Cvar_Register(&cl_debugTimer);
	Cvar_Register(&cl_lagFudgeTime);
	Cvar_Register(&cl_ignoreBadPackets);
	Cvar_Register(&cl_currentContentDownload);
	Cvar_Register(&cl_currentContentDownloadProgress);
	Cvar_Register(&cl_randomPacketLoss);
	Cvar_Register(&cl_showPackets);	
	Cvar_Register(&cl_delayFrames);
	Cvar_Register(&cl_packetSendFPS);
	Cvar_Register(&cl_debugInputs);
	Cvar_Register(&cl_noDelta);
	//Cvar_Register(&cl_getFull);


	Cmd_Register("demorecord",	Client_DemoRecord_Cmd);	
	Cmd_Register("demostop",	Client_DemoStop_Cmd);
	Cmd_Register("demoplay",	Client_DemoPlay_Cmd);	
	Cmd_Register("demo",		Client_DemoPlay_Cmd);		//alias

	Cmd_Register("statestrings",	Client_StateStrings_Cmd);
	Cmd_Register("serverinfo",		Client_ServerInfo_Cmd);
	Cmd_Register("svcmd",			Client_SvCmd_Cmd);

	lc.inputStateNum = 2;

	//initialize memory
	memset(&nullinput, 0, sizeof(nullinput));
	memset(&lc, 0, sizeof(lc));
	memset(&demo, 0, sizeof(demo));

//	Client_Reset();
}

