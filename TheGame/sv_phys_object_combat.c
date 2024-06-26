// (C) 2002 S2 Games

// sv_phys_object_combat.c

#include "server_game.h"


extern physicsParams_t p;
extern phys_out_t *p_out;

void	SV_Phys_TraceThroughCrosshair(serverObject_t *obj, traceinfo_t *trace, float range)
{
	//start at the viewheight and trace a ray in the view direction
	vec3_t start = { obj->base.pos[0], obj->base.pos[1], obj->base.pos[2] + objData[obj->base.type].viewheight };
	vec3_t end;
	M_MultVec3(p.forward, range, end);
	M_AddVec3(start, end, end);

	p.tracefunc(trace, start, end, zero_vec, zero_vec, SURF_TERRAIN);	//ignore terrain?
}

bool	SV_Phys_CheckForImpact(serverObject_t *obj, traceinfo_t *trace)
{
	SV_Phys_TraceThroughCrosshair(obj, trace, 30);		//we'll use 30 as the default weapon range for now

	if (trace->fraction < 1)
	{
		return true;
	}
	return false;
}
