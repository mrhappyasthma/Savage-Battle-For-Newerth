// (C) 2001 S2 Games

// sv_phys_object.c

// physics which affect an npc/object 

#include "server_game.h"

//#define DEBUG_MOVEMENT

extern vec3_t up;
extern cvar_t p_stepheight;

void    SV_Phys_AddEvent(serverObject_t *obj, byte event, byte param, byte param2)
{
	if (obj->client && sl.teams[obj->client->info.team].commander != obj->base.index)
	{
		playerState_t *ps = &sl.clients[obj->base.index].ps;

		if (ps->numEvents >= MAX_OBJECT_EVENTS)
		{
			//fixme: let's use a circular buffer instead.  newer events are more important than old ones
			core.Console_DPrintf("Warning: too many events for player %i\n", obj->base.index);
			return;
		}

		ps->events[ps->numEvents].type = event;
		ps->events[ps->numEvents].param = param;
		ps->events[ps->numEvents].param2 = param2;
		ps->numEvents++;
	}
	else
	{
		if (obj->base.numEvents >= MAX_OBJECT_EVENTS)
		{
			//fixme: let's use a circular buffer instead.  newer events are more important than old ones
			core.Console_DPrintf("Warning: too many events for object %i\n", obj->base.index);
			return;
		}

		obj->base.events[obj->base.numEvents].type = event;
		obj->base.events[obj->base.numEvents].param = param;
		obj->base.events[obj->base.numEvents].param2 = param2;
		obj->base.numEvents++;
	}

}



extern cvar_t p_minslope;
extern cvar_t sv_aiIgnoreSlope;

bool    SV_Phys_ObjectOnGround(serverObject_t *obj, traceinfo_t *trace)
{
	vec3_t targetpos = { obj->base.pos[0], obj->base.pos[1], obj->base.pos[2] - 0.5 };
	
	cores.World_TraceBox(trace, obj->base.pos,  targetpos, obj->base.bmin, obj->base.bmax, TRACE_PLAYER_MOVEMENT);
	
	if (trace->fraction < 1 /*&& trace->normal[2] >= p_minslope.value*/)
	{
		return true;
	}
	
	return false;
}
