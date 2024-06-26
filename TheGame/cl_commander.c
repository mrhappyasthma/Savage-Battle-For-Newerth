/*
 * (C) 2002 S2 Games
 *
 * cl_commander.c'
 */

#include "client_game.h"

#define CMDR_MAX_ZOOM	3000
#define CMDR_MIN_ZOOM	150
#define CMDR_FARCLIP	6500

#define OBJECT_PLACEMENT_POS_VALID		1
#define OBJECT_PLACEMENT_POS_INVALID	2
#define OBJECT_PLACEMENT_POS_UNKNOWN	3

#define	MAX_VALID_PLACEHOLDER_OBJECTS 10
#define ANY_ENEMY_OBJECT -2

enum
{
	CMDR_ANY_LOCATION,
	CMDR_UNIT_LOCATION,
	CMDR_ENEMY_LOCATION,
	CMDR_MINE_LOCATION,
	CMDR_BASE_LOCATION,
};

cvar_t	cl_cmdr_scrollaccel =			{ "cl_cmdr_scrollaccel",		"800",	CVAR_SAVECONFIG };
cvar_t	cl_cmdr_scrollspeed =			{ "cl_cmdr_scrollspeed",		"600",	CVAR_SAVECONFIG };
cvar_t	cl_cmdr_zoomspeed =				{ "cl_cmdr_zoomspeed",			"30",	CVAR_SAVECONFIG };
cvar_t	cl_cmdr_rotatespeed =			{ "cl_cmdr_rotatespeed",		"60",	CVAR_SAVECONFIG };
cvar_t	cl_cmdr_scrollup =				{ "cl_cmdr_scrollup",			"0" };
cvar_t	cl_cmdr_scrolldown =			{ "cl_cmdr_scrolldown",			"0" };
cvar_t	cl_cmdr_scrollleft =			{ "cl_cmdr_scrollleft",			"0" };
cvar_t	cl_cmdr_scrollright =			{ "cl_cmdr_scrollright",		"0" };
cvar_t	cl_cmdr_maxheight =				{ "cl_cmdr_maxheight",			"1600" };
cvar_t	cl_cmdr_camConstrain =			{ "cl_cmdr_camConstrain",		"500" };
cvar_t	cl_cmdr_anglelerp =				{ "cl_cmdr_anglelerp",			"2" };
cvar_t	cl_cmdr_camtiltx =				{ "cl_cmdr_camtiltx",			"-64" };
cvar_t	cl_cmdr_camtilty = 				{ "cl_cmdr_camtilty",			"0" };
cvar_t	cl_cmdr_camtiltz = 				{ "cl_cmdr_camtiltz",			"0" };
cvar_t	cl_cmdr_zoom =					{ "cl_cmdr_zoom",				"6000" };
cvar_t	cl_cmdr_zoomsmooth =			{ "cl_cmdr_zoomsmooth",			"8" };
cvar_t	cl_cmdr_swivel =				{ "cl_cmdr_swivel",				"0" };
cvar_t	cl_cmdr_fov =					{ "cl_cmdr_fov",				"90" };
cvar_t	cl_placementLeniency =			{ "cl_placementLeniency",		"20" };
cvar_t	cl_cblclickSelectRange =		{ "cl_cblclickSelectRange",		"800" };

cvar_t	cl_cmdr_minimapRefresh	=		{ "cl_cmdr_minimapRefresh", "200" };

cvar_t	cl_selectRegionRadius	=		{ "cl_selectRegionRadius", "800" };
cvar_t	cl_orderClickRadius	=			{ "cl_orderClickRadius", "30" };

cvar_t	cl_selectionSoundVolume =		{ "cl_selectionSoundVolume", "0.7" };

cvar_t	cl_cmdr_lockCameraToMouse =		{ "cl_cmdr_lockCameraToMouse",	"0" };

extern cvar_t cl_doubleClickTime;
extern cvar_t unit_progress;
extern cvar_t cl_showLobby;

//=============================================================================
bool	CL_TraceGround(traceinfo_t *result, vec3_t start, vec3_t dir)
{
	vec3_t end;
	
	M_MultVec3(dir, 999999, end);
	M_AddVec3(start, end, end);

	return corec.World_TraceBox(result, start, end, zero_vec, zero_vec, ~SURF_TERRAIN);
}

void	CL_GetTerrainPosAtCursor(float *x, float *y)
{
	vec3_t dir;
	traceinfo_t trace;

	Cam_ConstructRay(&cl.camera, cl_mousepos_x.integer, cl_mousepos_y.integer, dir);
	CL_TraceGround(&trace, cl.camera.origin, dir);	

	*x = trace.endpos[0];
	*y = trace.endpos[1];
}

bool	CL_GetTerrainPosAtCursorVec3(vec3_t pos)
{
	traceinfo_t trace;
	vec3_t dir;

	Cam_ConstructRay(&cl.camera, cl_mousepos_x.integer, cl_mousepos_y.integer, dir);
	CL_TraceGround(&trace, cl.camera.origin, dir);	
	
	M_CopyVec3(trace.endpos, pos);	
	if (trace.fraction < 1)
		return true;
	return false;
}
//=============================================================================


//=============================================================================
void	CL_ClearUnitSelection(unitSelection_t *selection)
{
	int n;

	selection->numSelected = 0;
	selection->mobileUnitsSelected = false;
	for (n=0; n<MAX_SELECTABLE_UNITS; n++)
		selection->array[n] = -1;
}

void	CL_ToggleUnitSelection(unitSelection_t *selection, int obj)
{
	int i = 0;

	if (!CL_AddToUnitSelection(selection, obj))
	{
		while (i < selection->numSelected
			 	&& selection->array[i] != obj)
		{
				i++;
		}
		if (i >= selection->numSelected)
			return;

		selection->array[i] = selection->array[selection->numSelected-1];
		selection->numSelected--;

	}
}

bool	CL_UnitIsSelected(unitSelection_t *selection, int obj)
{
	int n;

	for (n=0; n<selection->numSelected; n++)
	{
		if (selection->array[n] == obj)
			return true;
	}

	return false;
}

bool	CL_AddToUnitSelection(unitSelection_t *selection, int obj)
{
	if (selection->numSelected >= MAX_SELECTABLE_UNITS)
		return false;

	if (cl.isCommander && !cl_showLobby.integer)
	{
		if (cl.objects[obj].visual.health <= 0 && CL_ObjectType(cl.objects[obj].base.type)->isVulnerable)
			return false;
	}
		
	if (CL_UnitIsSelected(selection, obj))
		return false;
	
	selection->array[selection->numSelected] = obj;
	selection->numSelected++;

	if (IsMobileType(cl.objects[obj].base.type) && cl.objects[obj].base.team == cl.info->team)
		selection->mobileUnitsSelected = true;

	return true;
}
//=============================================================================


//=============================================================================
void	CL_FinalizeUnitSelection(unitSelection_t *selection)
{
	unitSelection_t final;

	//look at all the units we selected and filter them
	if (selection->numSelected <= 1)
	{		
		return;
	}

	if (selection->mobileUnitsSelected)
	{
		//we have selected mobile units on our own team, so filter out anything else
		int n;

		final.numSelected = 0;
		final.mobileUnitsSelected = true;

		for (n=0; n<selection->numSelected; n++)
		{
			if (IsMobileType(cl.objects[selection->array[n]].base.type) && cl.objects[selection->array[n]].base.team == cl.info->team)
			{
				final.array[final.numSelected] = selection->array[n];
				final.numSelected++;
			}
		}

		*selection = final;
	}
	else
		selection->numSelected = 1;		//not allowed to do multiple selections of non mobile units
}

void	CL_MessageSelected(char *msg)
{
	corec.Client_SendMessageToServer(fmt("%s %s", CLIENT_CHAT_SELECTED_MSG, msg));
}
//=============================================================================


//=============================================================================
int	CL_FindNextIdleWorker()
{
	static int lastIdleWorker = MAX_CLIENTS;
	int i = lastIdleWorker + 1;

	do
	{
		if (cl.objects[i].base.active
			&& cl.objects[i].base.team == cl.info->team
			&& IsWorkerType(cl.objects[i].base.type)
			&& cl.objects[i].visual.animState == AS_IDLE
			&& cl.objects[i].animStateTime >= 1000)
		{
			lastIdleWorker = i;
			return i;
		}
		i++;

		if (i >= MAX_OBJECTS)
		{
			i = MAX_CLIENTS;
		}
	}
	while (i != lastIdleWorker);

	return -1;
}

void	CL_SelectNextIdleWorker_cmd(int argc, char *argv[])
{
	int worker;
	
	if (!cl.isCommander)
		return;
	
	worker = CL_FindNextIdleWorker();

	if (worker != -1)
	{
		CL_ClearUnitSelection(&cl.potentialSelection);
		CL_AddToUnitSelection(&cl.potentialSelection, worker);
		CL_ProcessPotentialSelection(&cl.potentialSelection);
		cl.cmdrLookAtPoint[X] = cl.objects[worker].visual.pos[0];
		cl.cmdrLookAtPoint[Y] = cl.objects[worker].visual.pos[1];
	}
}
//=============================================================================


//=============================================================================
int		CL_GetClosestObjectInRegion(vec2_t pos, float radius, bool allyOnly)
{
	float radiussq, dist;
	float closest = FAR_AWAY;
	int i, object_id = -1;
	
	radiussq = radius * radius;

	for (i = 0; i < MAX_OBJECTS; i++)
	{
		if (!cl.objects[i].base.active
			|| (allyOnly && cl.objects[i].base.team != cl.info->team))
			continue;
		
		if ((dist = M_GetDistanceSqVec2(cl.objects[i].base.pos, pos)) < radiussq
			&& dist < closest)
		{
			closest = dist;
			object_id = i;
		}
	}
	return object_id;
}

int	CL_GetObjectUnderCursor()
{
	vec3_t dir, endpos;
	vec2_t pos;
	traceinfo_t trace;
	int obj;

	//determine what's exactly under the mouse cursor
	Cam_ConstructRay(&cl.camera, cl_mousepos_x.integer, cl_mousepos_y.integer, dir);
	M_PointOnLine(cl.camera.origin, dir, 99999, endpos);
	corec.World_TraceBox(&trace, cl.camera.origin, endpos, zero_vec, zero_vec, SURF_FOLIAGE | SURF_NOT_SOLID);

	if (trace.index >= 0 && trace.index < MAX_OBJECTS)
	{
		if (!(cl.objects[trace.index].base.flags & BASEOBJ_NO_RENDER)
			&& !(cl.objects[trace.index].inFogOfWar)
			&& (IsBuildingType(cl.objects[trace.index].base.type) || (IsMobileType(cl.objects[trace.index].base.type))))
			return trace.index;		//only return units (structures / mobile units), not world objects or projectiles or items
	}

	CL_GetTerrainPosAtCursor(&pos[0], &pos[1]);
	obj = CL_GetClosestObjectInRegion(pos, cl_orderClickRadius.value, false);

	return obj;
}

//fill in a selection array based on the current selection rectangle / mouse position
void CL_GetUnitSelection(unitSelection_t *selection)
{
	int obj;

	if (!corec.Input_IsKeyDown(KEY_SHIFT))
		CL_ClearUnitSelection(selection);

	obj = CL_GetObjectUnderCursor();
	if (obj >= 0)
	{
		CL_AddToUnitSelection(selection, obj);
	}

	if (cl.showSelectionRect && abs(cl.selectionRect_dim[X]) > 6 && abs(cl.selectionRect_dim[Y]) > 6)
	{
		int numobjs;
		int sel[64];

		numobjs = corec.Scene_SelectObjects(&cl.camera, 
							 cl.selectionRect_tl[X], cl.selectionRect_tl[Y], 
							 cl.selectionRect_dim[X], cl.selectionRect_dim[Y], sel, 64);
		if (numobjs)
		{
			int n;

			for (n=0; n<numobjs; n++)
			{
				CL_AddToUnitSelection(selection, sel[n]);
			}
		}
	}
}
	
void	CL_ResetSelectionRectangle()
{
	cl.selectionRect_tl[X] = cl_mousepos_x.value;
	cl.selectionRect_tl[Y] = cl_mousepos_y.value;
	cl.selectionRect_dim[X] = 1;
	cl.selectionRect_dim[Y] = 1;
}

void	CL_SelectingObjectsFrame()
{
	if (cl.showSelectionRect)
	{					
		cl.selectionRect_dim[X] = cl_mousepos_x.value - cl.selectionRect_tl[X];
		cl.selectionRect_dim[Y] = cl_mousepos_y.value - cl.selectionRect_tl[Y];
	}

	CL_ClearUnitSelection(&cl.potentialSelection);
	CL_GetUnitSelection(&cl.potentialSelection);		//perform the selection

}

bool	CL_CommanderInitializeCamera()
{
	vec3_t pos;
	
	if (cl.info->team == TEAM_UNDECIDED)
	{
		corec.Console_DPrintf("Can't initialize commander camera... player hasn't chosen a team yet.\n");
		return false;
	}

	if (!cl.objects[cl.teams[cl.info->team].command_center].base.active)
		return false;

	M_CopyVec3(cl.objects[cl.teams[cl.info->team].command_center].base.pos, pos);
	
	cl.cmdrLookAtPoint[X] = pos[X];
	cl.cmdrLookAtPoint[Y] = pos[Y];	
	
	corec.Cvar_SetVarValue(&cl_cmdr_camtiltx, -64);
	corec.Cvar_SetVarValue(&cl_cmdr_camtilty, 0);
	corec.Cvar_SetVarValue(&cl_cmdr_camtiltz, 0);
	//set the zoom cvar and cl.cmdrZoom differently for a subtle zooming effect at the start of the game
	corec.Cvar_SetVarValue(&cl_cmdr_zoom, 6000);
	SET_VEC3(cl.cmdrAngles, -60, 10, 25);
	cl.cmdrZoom = 1500;

	return true;
}

void	CL_CommanderSetupCamera()
{
	vec3_t start,end;
	traceinfo_t trace;

	CL_CommanderMoveCamera();

	cl.camera.fovx = cl_cmdr_fov.integer;
	Cam_CalcFovy(&cl.camera);

	cl.camera.x = 0;
	cl.camera.y = 0;

	cl.cmdrLookAtPoint[2]=0;

	//trace a ray from the sky to the desired position in the view direction so we can detect intersection with terrain and objects

	M_PointOnLine(cl.cmdrLookAtPoint, cl.camera.viewaxis[FORWARD], -cl.cmdrZoom, start);
	M_PointOnLine(cl.cmdrLookAtPoint, cl.camera.viewaxis[FORWARD], 10000, end);

	corec.World_TraceBox(&trace, start, end, zero_vec, zero_vec, 0);

	if (trace.fraction < 1)
	{
		vec3_t point,seg;
		float len;

		//push away a little
		M_PointOnLine(trace.endpos, cl.camera.viewaxis[FORWARD], -CMDR_MIN_ZOOM, point);
		
		M_SubVec3(point, cl.cmdrLookAtPoint, seg);

		len = M_GetVec3Length(seg);
		if (cl_cmdr_zoom.value < len)
		{
			//set zoom cvar to reflect this new point
			corec.Cvar_SetVarValue(&cl_cmdr_zoom, len);
		}
	}

	
	//smooth zooming effect
	cl.cmdrZoom = M_ClampLerp(cl_cmdr_zoomsmooth.value * cl.frametime, cl.cmdrZoom, cl_cmdr_zoom.value);
	if (cl.cmdrZoom > cl_cmdr_maxheight.value)
	{
		cl.cmdrZoom = cl_cmdr_maxheight.value;
		corec.Cvar_SetVarValue(&cl_cmdr_zoom, cl.cmdrZoom);
	}
	//smooth rotating effect
	corec.Cvar_SetVarValue(&cl_cmdr_camtiltz, cl_cmdr_camtiltz.value + cl_cmdr_swivel.value);
	cl.cmdrAngles[X] = M_ClampLerp(cl_cmdr_anglelerp.value * cl.frametime, cl.cmdrAngles[X], cl_cmdr_camtiltx.value);
	cl.cmdrAngles[Y] = M_ClampLerp(cl_cmdr_anglelerp.value * cl.frametime, cl.cmdrAngles[Y], cl_cmdr_camtilty.value);
	cl.cmdrAngles[Z] = M_ClampLerp(cl_cmdr_anglelerp.value * cl.frametime, cl.cmdrAngles[Z], cl_cmdr_camtiltz.value);
	
	Cam_SetTarget(&cl.camera, cl.cmdrLookAtPoint);
	Cam_SetAngles(&cl.camera, cl.cmdrAngles);
	Cam_SetDistance(&cl.camera, cl.cmdrZoom);
	
	//set listener position
	M_PointOnLine(cl.camera.origin, cl.camera.viewaxis[FORWARD], 99999, end);

	corec.World_TraceBox(&trace, cl.camera.origin, end, zero_vec, zero_vec, 0);

	corec.Sound_SetListenerPosition(trace.endpos, cl.camera.viewaxis[FORWARD], cl.camera.viewaxis[UP], true);
}

//adjust the lookat point
void	CL_CommanderMoveCamera()
{
	static float xspeed = 0.0;
	static float yspeed = 0.0;
	vec3_t bmin;
	vec3_t bmax;
	
	if (cl.winStatus)
		return;
	
	if (!cl.isCommander)
		return;

	if (!cl_cmdr_lockCameraToMouse.integer)
	{
		vec3_t velocity, diff;
		bool moved = false;

		corec.Input_SetMouseMode(MOUSE_FREE);
		
		if (cl_cmdr_scrollup.integer || cl_mousepos_y.value < 15)
		{
			moved = true;
			yspeed -= corec.FrameSeconds() * cl_cmdr_scrollaccel.value;
		}
		if (cl_cmdr_scrolldown.integer || cl_mousepos_y.value > corec.Vid_GetScreenH() - 15)
		{
			moved = true;
			yspeed += corec.FrameSeconds() * cl_cmdr_scrollaccel.value;
		}
		if (cl_cmdr_scrollleft.integer || cl_mousepos_x.value < 15)
		{
			moved = true;
			xspeed -= corec.FrameSeconds() * cl_cmdr_scrollaccel.value;
		}
		if (cl_cmdr_scrollright.integer || cl_mousepos_x.value > corec.Vid_GetScreenW() - 15)
		{
			moved = true;
			xspeed += corec.FrameSeconds() * cl_cmdr_scrollaccel.value;
		}
	
		if (!moved)
		{
			if (xspeed < 0)
			{
				xspeed += corec.FrameSeconds() * cl_cmdr_scrollaccel.value;
				if (xspeed > 0)
					xspeed = 0;
			}
			if (xspeed > 0)
			{
				xspeed -= corec.FrameSeconds() * cl_cmdr_scrollaccel.value;
				if (xspeed < 0)
					xspeed = 0;
			}
			if (yspeed < 0)
			{
				yspeed += corec.FrameSeconds() * cl_cmdr_scrollaccel.value;
				if (yspeed > 0)
					yspeed = 0;
			}
			if (yspeed > 0)
			{
				yspeed -= corec.FrameSeconds() * cl_cmdr_scrollaccel.value;
				if (yspeed < 0)
					yspeed = 0;
			}
		}
			
		if (xspeed > cl_cmdr_scrollspeed.value) xspeed = cl_cmdr_scrollspeed.value;
		if (yspeed > cl_cmdr_scrollspeed.value) yspeed = cl_cmdr_scrollspeed.value;
		if (xspeed < -cl_cmdr_scrollspeed.value) xspeed = -cl_cmdr_scrollspeed.value;
		if (yspeed < -cl_cmdr_scrollspeed.value) yspeed = -cl_cmdr_scrollspeed.value;
		
		M_MultVec3(cl.camera.viewaxis[RIGHT], xspeed * corec.FrameSeconds(), velocity);
		M_AddVec3(cl.cmdrLookAtPoint, velocity, cl.cmdrLookAtPoint);

		M_CopyVec2(cl.camera.viewaxis[UP], velocity);
		M_NormalizeVec2(velocity);
		M_MultVec3(velocity, -yspeed * corec.FrameSeconds(), diff);
		M_AddVec3(cl.cmdrLookAtPoint, diff, cl.cmdrLookAtPoint);
	
		/*if (cl.cmdrZoom >= cl_cmdr_maxheight.value)
		{
			corec.Cvar_SetVarValue(&cl_cmdr_zoom, cl_cmdr_maxheight.value - 0.01);
		}*/

		cl.cmdrJustGrabbedCamera = true;
	}
	else
	{
		corec.Input_SetMouseMode(MOUSE_RECENTER);

		if (cl.cmdrJustGrabbedCamera)
		{
			cl.inputstate.pitch = 0;
			cl.cmdrLastYaw = cl.inputstate.yaw;
			cl.cmdrJustGrabbedCamera = false;
			return;
		}

		cl.cmdrLookAtPoint[X] -= (cl.inputstate.yaw - cl.cmdrLastYaw);
		cl.cmdrLookAtPoint[Y] += cl.inputstate.pitch;

		cl.cmdrLastYaw = cl.inputstate.yaw;
	}

	//tidy up the camera motion
	core.Cvar_SetValue("gfx_nearclip", cl.camera.origin[Z]/10);
	core.Cvar_SetValue("gfx_farclip", CMDR_FARCLIP);

	corec.World_GetBounds(bmin, bmax);
	bmin[0] += cl_cmdr_camConstrain.value;
	bmin[1] += cl_cmdr_camConstrain.value;
	bmax[0] -= cl_cmdr_camConstrain.value;
	bmax[1] -= cl_cmdr_camConstrain.value;
	if (cl.cmdrLookAtPoint[X] < bmin[X])
		cl.cmdrLookAtPoint[X] = bmin[X];
	if (cl.cmdrLookAtPoint[X] > bmax[X])
		cl.cmdrLookAtPoint[X] = bmax[X];
	if (cl.cmdrLookAtPoint[Y] < bmin[Y])
		cl.cmdrLookAtPoint[Y] = bmin[Y];
	if (cl.cmdrLookAtPoint[Y] > bmax[Y])
		cl.cmdrLookAtPoint[Y] = bmax[Y];
}

void	CL_ClearObjectTargets(int object)
{
	if (object >= 0 
		&& object < MAX_OBJECTS
		&& cl.objects[object].base.active)
	{
		corec.Client_SendMessageToServer(fmt("%s %i", CLIENT_COMMANDER_CLEAR_CLIENT_TARGETS_MSG, object));
	}
}

void	CL_ClearTargets_Cmd(int argc, char *argv[])
{
	int i;
	for (i = 0; i < cl.selection.numSelected; i++)
		CL_ClearObjectTargets(cl.selection.array[i]);
}

void	CL_CommanderExitMode()
{
	if (cl.cmdrPlaceObjectTwin)
		corec.Client_SendMessageToServer(fmt("%s", CLIENT_COMMANDER_CANCEL_LINK_MSG));

	cl.cmdrMode = 0;
	cl.showSelectionRect = false;
	cl.cmdrPlaceObjectTwin = false;
	Rend_SetMouseCursor(&res.mainCursor);
}

//=============================================================================
// Object Placement
//=============================================================================
float	cmdr_objectPlacementAngleOffset;

bool	CL_ValidObjectPosition(byte structure_type, vec3_t pos)
{
	objectData_t *bld = CL_ObjectType(structure_type);
	pointinfo_t pi;
	float basez, z1, z2, z3, z4;
	
	corec.World_SampleGround(pos[X], pos[Y], &pi);
	basez = pi.z;
	corec.World_SampleGround(pos[X]+bld->bmin[X], pos[Y]+bld->bmin[Y], &pi);
	z1 = pi.z;
	corec.World_SampleGround(pos[X]+bld->bmin[X], pos[Y]+bld->bmax[Y], &pi);
	z2 = pi.z;
	corec.World_SampleGround(pos[X]+bld->bmax[X], pos[Y]+bld->bmin[Y], &pi);
	z3 = pi.z; 
	corec.World_SampleGround(pos[X]+bld->bmax[X], pos[Y]+bld->bmax[Y], &pi);
	z4 = pi.z;
	if (ABS(basez - z1) > cl_placementLeniency.value
		|| ABS(basez - z2) > cl_placementLeniency.value
		|| ABS(basez - z3) > cl_placementLeniency.value
		|| ABS(basez - z4) > cl_placementLeniency.value)
		return false;
	
	return true;
}

int	CL_GetObjectPlacementPos(vec3_t pos, vec3_t angle)
{
	CL_GetTerrainPosAtCursorVec3(pos);	
	SET_VEC3(angle, 0, 0, /*-cl_cmdr_camtiltz.value +*/ cmdr_objectPlacementAngleOffset);

	if ( cl.cmdrPlaceObjectDirty || !M_CompareVec3(pos, cl.cmdrPlaceObjectPos) || !M_CompareVec3(angle, cl.cmdrPlaceObjectAngle) )
	{
		if ( !cl.cmdrPlaceObjectDirty )
		{
			cl.cmdrPlaceObjectDirty = true;
			cl.cmdrPlaceObjectValid = false;
			cl.cmdrPlaceObjectReqId++;
			M_CopyVec3(pos, cl.cmdrPlaceObjectPos);
			M_CopyVec3(angle, cl.cmdrPlaceObjectAngle);
		}

		{
			traceinfo_t trace;
			vec3_t bmin, bmax, bmin2, bmax2;
			vec3_t axis[3];
			bool invalidPos = true;

			//collision test here
			corec.Res_GetModelSurfaceBounds(CL_Model(cl.cmdrPlaceObjectType), bmin, bmax);
			
			M_MultVec3(bmin, STRUCTURE_SCALE, bmin);
			M_MultVec3(bmax, STRUCTURE_SCALE, bmax);
			M_GetAxis(angle[0], angle[1], angle[2], axis);
			M_TransformPoint(bmin, vec3(0,0,0), axis, bmin2);
			M_TransformPoint(bmax, vec3(0,0,0), axis, bmax2);

			bmin[0] = MIN(bmin2[0], bmax2[0]);
			bmin[1] = MIN(bmin2[1], bmax2[1]);
			bmax[0] = MAX(bmin2[0], bmax2[0]);
			bmax[1] = MAX(bmin2[1], bmax2[1]);

			corec.World_TraceBox(&trace, pos, pos, bmin, bmax, SURF_TERRAIN);
			invalidPos = trace.fraction < 1;

			if (!invalidPos)
			{
				if (!CL_ValidObjectPosition(cl.cmdrPlaceObjectType, pos))
					invalidPos = true;
			}
			
			return invalidPos ? OBJECT_PLACEMENT_POS_INVALID : OBJECT_PLACEMENT_POS_VALID;
		}
	}
	else
	{
		return cl.cmdrPlaceObjectValid ? OBJECT_PLACEMENT_POS_VALID : OBJECT_PLACEMENT_POS_INVALID;
	}
}

bool	CL_PlaceObjectFrame()
{
	vec3_t pos, angle;

	switch ( CL_GetObjectPlacementPos(pos, angle) )
	{
		case OBJECT_PLACEMENT_POS_VALID:
			Rend_SetMouseCursor(&res.crosshairCursor);
			break;
		case OBJECT_PLACEMENT_POS_INVALID:
			Rend_SetMouseCursor(&res.errorCursor);
			break;
		case OBJECT_PLACEMENT_POS_UNKNOWN:
			Rend_SetMouseCursor(&res.unknownCursor);
			break;
	}

	Rend_DrawObjectPreview(CL_Model(cl.cmdrPlaceObjectType), 
						   CL_Skin(cl.cmdrPlaceObjectType, cl.info->team), 
						   pos, angle, 0.6, STRUCTURE_SCALE, SCENEOBJ_SHOW_BOUNDS, 0, false);

	return true;	
}

//perform the actual placement
void	CL_PlaceDownObject()
{
	vec3_t pos, angle;

	if ( OBJECT_PLACEMENT_POS_VALID != CL_GetObjectPlacementPos(pos, angle) )
	{
		CL_Play2d(Snd("place_error"), 1.0, CHANNEL_GUI);
		return;
	}
		
	CL_Play2d(Snd("building_place"), 1.0, CHANNEL_GUI);
	
	//placing the second stage of a link
	if (cl.cmdrPlaceObjectTwin)
	{
		corec.Client_SendMessageToServer(fmt("%s %i %.0f %.0f %.0f %.0f %.0f %.0f",
										CLIENT_COMMANDER_SPAWN_BUILDING_LINK_MSG,
										cl.cmdrPlaceObjectType,
										pos[0], pos[1], pos[2],
										angle[0], angle[1], angle[2]));

		cl.cmdrPlaceObjectTwin = false;
		CL_CommanderExitMode();
		Rend_SetMouseCursor(&res.mainCursor);
		return;
	}

	corec.Client_SendMessageToServer(fmt("%s %i %.0f %.0f %.0f %.0f %.0f %.0f",
									CLIENT_COMMANDER_SPAWN_BUILDING_MSG,
									cl.cmdrPlaceObjectType,
									pos[0], pos[1], pos[2],
									angle[0], angle[1], angle[2]));

	if (CL_ObjectType(cl.cmdrPlaceObjectType)->linked)
		cl.cmdrPlaceObjectTwin = true;
	else
		CL_CommanderExitMode();
}

void	CL_StartPlaceObject(int objectType)
{
	cl.cmdrMode = CMDR_PLACING_OBJECT;
	cl.cmdrPlaceObjectType = objectType;
	cmdr_objectPlacementAngleOffset = 0;
}
//=============================================================================
//=============================================================================


/*==========================

  CMDR_RotateBuilding_Cmd

  Allows the commander to change the orientation of the building he is
  currently attempting to place.
  Syntax: rotateBuilding <degrees>
  (negative = ccw)

 ==========================*/

void	CMDR_RotateBuilding_Cmd(int argc, char *argv[])
{
	float	delta;

	//safety checks
	if (cl.cmdrMode != CMDR_PLACING_OBJECT)
		return;

	if (argc < 1)
		return;

	//get the change
	delta = atof(argv[0]);
	if (delta > 0)
		delta = 90;
	else if (delta < 0)
		delta = -90;

	//adjust the angle offset
	cmdr_objectPlacementAngleOffset += delta;
	if (cmdr_objectPlacementAngleOffset > 360.0)
		cmdr_objectPlacementAngleOffset -= 360.0;
	if (cmdr_objectPlacementAngleOffset < 0.0)
		cmdr_objectPlacementAngleOffset += 360.0;
}
//=============================================================================

//syntax: pickLocation <goal>		-- pick a location, then send the location along with 'goal' as a unit command to the current selection of units
void	CL_PickLocation_Cmd(int argc, char *argv[])
{
	if (argc < 1)
		return;

	if (!cl.selection.numSelected)
		return;

	cl.cmdrGoal = CL_GetGoalFromString(argv[0]);
	if (!cl.cmdrGoal)
		return;

	cl.cmdrMode = CMDR_PICKING_LOCATION;
	switch (cl.cmdrGoal)
	{
		case GOAL_MINE:
			cl.cmdrLocationType = CMDR_MINE_LOCATION;
			break;
		case GOAL_ATTACK_OBJECT:
			cl.cmdrLocationType = CMDR_ANY_LOCATION;
			break;
		default:
			cl.cmdrLocationType = CMDR_ANY_LOCATION;	
	}
}

void	CL_PickingLocationFrame()
{
	int object;
	
	if (cl.cmdrLocationType == CMDR_UNIT_LOCATION)
	{
		if (CL_GetObjectUnderCursor() >= 0)
			Rend_SetMouseCursor(&res.crosshairCursor);
		else
			Rend_SetMouseCursor(&res.mainCursor);
	}
	else if (cl.cmdrLocationType == CMDR_ENEMY_LOCATION)
	{
		if ((object = CL_GetObjectUnderCursor()) >= 0
			&& cl.objects[object].base.team != cl.info->team)
			Rend_SetMouseCursor(&res.crosshairCursor);
		else
			Rend_SetMouseCursor(&res.mainCursor);
	}
	else if (cl.cmdrLocationType == CMDR_MINE_LOCATION)
	{
		if ((object = CL_GetObjectUnderCursor()) >= 0
			&& CL_ObjectType(cl.objects[object].base.type)->isMine)
			Rend_SetMouseCursor(&res.crosshairCursor);
		else
			Rend_SetMouseCursor(&res.mainCursor);
	}
	else if (cl.cmdrLocationType == CMDR_BASE_LOCATION)
	{
		if ((object = CL_GetObjectUnderCursor()) >= 0
			&& CL_ObjectType(cl.objects[object].base.type)->spawnFrom)
			Rend_SetMouseCursor(&res.crosshairCursor);
		else
			Rend_SetMouseCursor(&res.mainCursor);
	}
	else
		Rend_SetMouseCursor(&res.crosshairCursor);
}

//send a locational goal (a goal associated with a location or an object)
void	CL_SendLocationGoalToSelection(int goal, vec3_t pos)
{	
	char *snd;

	corec.Client_SendMessageToServer(fmt("%s %i %.0f %.0f %.0f", 
										CLIENT_COMMANDER_LOCATION_GOAL_MSG,
										goal,
										pos[0], pos[1], pos[2]));

	snd = Snd(fmt("%s_confirm", CL_ObjectType(cl.objects[cl.selection.array[0]].base.type)->name));
	if (snd[0])
		CL_Play2d(snd, 1.0, CHANNEL_CMDR_UNIT_SELECT);

	//client side blinking effect
	//CL_BlinkLocation(pos);
}

void	CL_SendObjectGoalToSelection(int goal, int objidx)
{	
	char *snd;

	corec.Client_SendMessageToServer(fmt("%s %i %i",
										CLIENT_COMMANDER_OBJECT_GOAL_MSG,
										goal, objidx));

	snd = Snd(fmt("%s_confirm", CL_ObjectType(cl.objects[cl.selection.array[0]].base.type)->name));
	if (snd[0])
		CL_Play2d(snd, 1.0, CHANNEL_CMDR_UNIT_SELECT);

	//client side blinking effect
	cl.objects[objidx].blink = true;
	cl.objects[objidx].blinkStartTime = cl.gametime;	
}

void	CL_SelectRegion(vec3_t pos)
{
	float radiussq;
	int i;
	unitSelection_t final;
	
	radiussq = cl_selectRegionRadius.value*cl_selectRegionRadius.value;

	for (i = 0; i < MAX_OBJECTS; i++)
	{
		if (!cl.objects[i].base.active
			|| cl.objects[i].base.team != cl.info->team)
			continue;
		
		if (M_GetDistanceSqVec2(cl.objects[i].base.pos, pos) < radiussq)
		{
			CL_AddToUnitSelection(&cl.potentialSelection, i);
		}
	}
	final.numSelected = 0;
	for (i = 0; i < cl.selection.numSelected; i++)
	{
		if (cl.selection.array[i] < MAX_CLIENTS)
			final.array[final.numSelected++] = cl.selection.array[i];
	}
	cl.potentialSelection = final;
	CL_ProcessPotentialSelection();
}

void	CL_SelectRegionAtCursor()
{
	vec3_t pos;
	CL_GetTerrainPosAtCursor(&pos[0], &pos[1]);
	CL_SelectRegion(pos);
}

void	CL_PickLocation(bool useRange)
{
	int objidx;
	vec3_t pos;

	CL_GetTerrainPosAtCursorVec3(pos);

	if (cl.cmdrLocationType == CMDR_UNIT_LOCATION
		|| cl.cmdrLocationType == CMDR_MINE_LOCATION
		|| cl.cmdrLocationType == CMDR_ENEMY_LOCATION)
	{
		if (useRange)
		{
			CL_GetTerrainPosAtCursor(&pos[0], &pos[1]);
			objidx = CL_GetClosestObjectInRegion(pos, cl_selectRegionRadius.value, false);
		}
		else
		{
			objidx = CL_GetObjectUnderCursor();
		}

		if (objidx >= 0)
		{
			CL_SendObjectGoalToSelection(cl.cmdrGoal, objidx);
		}
	}
	else
	{
		objidx = CL_GetObjectUnderCursor();
		if (objidx >= 0)
			CL_SendObjectGoalToSelection(cl.cmdrGoal, objidx);
		else
		{
			if (cl.cmdrGoal == GOAL_ATTACK_OBJECT)
				cl.cmdrGoal = GOAL_DEFEND;
			CL_SendLocationGoalToSelection(cl.cmdrGoal, pos);
		}
	}
	CL_CommanderExitMode();
}

//syntax: pickUnit <goal>			-- pick a unit, then send the unit index along with 'goal' as a unit command to the current selection of units
void	CL_PickUnit_Cmd(int argc, char *argv[])
{
	if (argc < 1)
		return;

	if (!cl.selection.numSelected)
		return;

	cl.cmdrGoal = CL_GetGoalFromString(argv[0]);
	if (!cl.cmdrGoal)
		return;

	cl.cmdrMode = CMDR_PICKING_LOCATION;

	switch (cl.cmdrGoal)
	{
		case GOAL_ATTACK_OBJECT:
			cl.cmdrLocationType = CMDR_ANY_LOCATION;
			break;
		case GOAL_ENTER_BUILDING:
			cl.cmdrLocationType = CMDR_BASE_LOCATION;
		default:
			cl.cmdrLocationType = CMDR_UNIT_LOCATION;
	}
}

//syntax: radiusTask <goal> - send a task to all nearby team units
void	CL_RadiusTask_Cmd(int argc, char *argv[])
{
	int goal, target, numSelected;
	int currentSelection[MAX_SELECTABLE_UNITS];
		
	if (argc < 1)
		return;

	if (!cl.selection.numSelected)
		return;

	goal = CL_GetGoalFromString(argv[0]);
	if (!goal)
		return;

	target = cl.selection.array[0];

	//save their current selection array
	numSelected = cl.selection.numSelected;
	memcpy(currentSelection, cl.selection.array, sizeof(int) * MAX_SELECTABLE_UNITS);
	
	//find all nearby units and overwrite the current selection
	CL_SelectRegion(cl.objects[target].base.pos);
	CL_SendSelectionToServer();

	//send out the goal string to these nearby units
	switch (goal)
	{
		case GOAL_ATTACK_OBJECT:
		case GOAL_DEFEND:
			CL_SendObjectGoalToSelection(goal, target);
			break;
	}

	//copy their old selection array back
	memcpy(cl.selection.array, currentSelection, sizeof(int) * MAX_SELECTABLE_UNITS);
	cl.selection.numSelected = numSelected;
	CL_SendSelectionToServer();
}

void	CL_SelectNearbyUnitsByType()
{
	int	target, type, n;

	if (!cl.isCommander)
		return;

	target = CL_GetObjectUnderCursor();

	//did we hit something?
	if (target < 0)
		return;

	//only works for team units
	if (cl.objects[target].visual.team != cl.info->team)
		return;

	type = cl.objects[target].visual.type;

	if (!IsUnitType(type))
		return;

	for (n = 0; n < MAX_OBJECTS; n++)
	{
		if (cl.objects[n].visual.team != cl.info->team ||
			IsWorkerType(cl.objects[n].visual.type) != IsWorkerType(type) ||
			IsCharacterType(cl.objects[n].visual.type) != IsCharacterType(type))
			continue;

		if (abs(cl.objects[target].visual.pos[X] - cl.objects[n].visual.pos[X]) < cl_cblclickSelectRange.value &&
			abs(cl.objects[target].visual.pos[Y] - cl.objects[n].visual.pos[Y]) < cl_cblclickSelectRange.value)
			CL_AddToUnitSelection(&cl.selection, n);
	}

	//let the server know which units we have selected
	CL_SendSelectionToServer();
		
	//update the grid menu with the selection
	CMDR_RefreshGrid();
	CL_InterfaceEvent(IEVENT_SELECTION);
	
	CMDR_RefreshSelectionIcons();
}

void	CL_SendSelectionToServer()
{
	int n;
	char msg[256];

	BPrintf(msg, 255, "%s %i", CLIENT_COMMANDER_SELECTION_MSG, cl.selection.numSelected);

	for (n=0; n<cl.selection.numSelected && strlen(msg) < 252; n++)
	{
		strcat(msg, fmt(" %i", cl.selection.array[n]));
	}

	corec.Client_SendMessageToServer(msg);
}


/*==========================

  CL_PlayUnitSelectionSound

  Plays the seelction sound associated with a unit

 ==========================*/

void	CL_PlayUnitSelectionSound(int objidx)
{
	int	objtype = cl.objects[objidx].visual.type;
	bool freeslot = false;
	static int lastTime = 0;
	int chan;

	if (CL_ObjectType(objidx)->objclass == OBJCLASS_BUILDING)
	{
		//if (cl.gametime < cl.cmdrNextSelectBldSoundTime)
		//	return;

		//cl.cmdrNextSelectBldSoundTime = cl.gametime + 1000;
		chan = CHANNEL_CMDR_BUILDING_SELECT;
	}
	else
	{
		if (cl.gametime < cl.cmdrNextSelectUnitSoundTime)
			return;

		cl.cmdrNextSelectUnitSoundTime = cl.gametime + 500;
		chan = CHANNEL_CMDR_UNIT_SELECT;
	}

	if (cl.info->team != cl.objects[objidx].base.team)
		return;
		
	if (cl.objects[objidx].visual.flags & BASEOBJ_UNDER_CONSTRUCTION)
		CL_Play2d(Snd("under_construction"), cl_selectionSoundVolume.value, chan);
	else if (objtype)
	{
		if (CL_ObjectType(objtype)->selectionSound[0])
			CL_Play2d(CL_ObjectType(objtype)->selectionSound, cl_selectionSoundVolume.value, chan);
		else
		{
			char *stringentry = Snd(fmt("%s_select", CL_ObjectType(objtype)->name));
			if (stringentry[0])
				CL_Play2d(stringentry, 1.0, chan);
		}
	}
}

void	CL_UpdateInterfaceWithNewSelection()
{
	CL_PlayUnitSelectionSound(cl.selection.array[0]);
}

void	CL_RemoveFromUnitSelection(unitSelection_t *selection, int slotnum)
{
	int n;

	if (!selection)
		return;

	if (selection->numSelected < slotnum ||
		selection->numSelected < 1 ||
		slotnum < 0 ||
		slotnum >= MAX_SELECTABLE_UNITS)
		return;

	for (n = slotnum; n < (selection->numSelected - 1); n++)
		selection->array[n] = selection->array[n + 1];

	selection->array[n] = -1;
	selection->numSelected = n;
	selection->mobileUnitsSelected = false;

	for (n = 0; n < selection->numSelected; n++)
	{
		if (IsMobileType(cl.objects[selection->array[n]].base.type))
			selection->mobileUnitsSelected = true;
	}
}

void	CL_ProcessPotentialSelection()
{
	bool playsound = true;

	CL_FinalizeUnitSelection(&cl.potentialSelection);

	if (cl.potentialSelection.numSelected)
	{
		//adding to selection
		if (corec.Input_IsKeyDown(KEY_SHIFT))
		{
			int n;

			//can only add mobile units, and only if mobiles is what we have selected
			if (cl.potentialSelection.mobileUnitsSelected)
			{
				for (n = 0; n < cl.selection.numSelected; n++)
				{
					if (!IsMobileType(cl.objects[cl.selection.array[n]].base.type) ||
						cl.objects[cl.selection.array[n]].base.team != cl.info->team)
						CL_RemoveFromUnitSelection(&cl.selection, n);
				}

				for (n = 0; n < cl.potentialSelection.numSelected; n++)
					CL_AddToUnitSelection(&cl.selection, cl.potentialSelection.array[n]);
			}
			else
			{
				cl.selection = cl.potentialSelection;
			}
		}
		//removing from selection
		else if (corec.Input_IsKeyDown(KEY_CTRL))
		{
			int i, n;

			playsound = false;

			for (n = 0; n < cl.selection.numSelected; n++)
			{
				for (i = 0; i < cl.potentialSelection.numSelected; i++)
				{
					if (cl.selection.array[n] == cl.potentialSelection.array[i])
						CL_RemoveFromUnitSelection(&cl.selection, n);
				}
			}
		}
		else
		{
			cl.selection = cl.potentialSelection;
		}

		if (cl.selection.numSelected && playsound)
		{
			CL_PlayUnitSelectionSound(cl.selection.array[0]);
		}
	}

	if (cl.isCommander)
	{
		//let the server know which units we have selected
		CL_SendSelectionToServer();
		
		//update the grid menu with the selection
		CMDR_RefreshGrid();
		CL_InterfaceEvent(IEVENT_SELECTION);
	
		CMDR_RefreshSelectionIcons();
	}
}

void	CL_ActivatingTechFrame()
{
}


//=============================================================================
// Left Click
//=============================================================================
void	CL_CommanderMouseDown1()
{
	//start selecting units
	switch(cl.cmdrMode)
	{
		case CMDR_PLACING_OBJECT:
		case CMDR_PLACING_LINK:
			CL_PlaceDownObject();
			break;

		case CMDR_PICKING_LOCATION:
			if (cl.gametime - cl.lastLeftMouseClick < cl_doubleClickTime.value)
				CL_PickLocation(true);
			else
				CL_PickLocation(false);
			break;

		case CMDR_ACTIVATING_TECH:
			if (CMDR_ActivateTech())
				CL_CommanderExitMode();
			break;

		default:
			//select units
			if (cl.gametime - cl.lastLeftMouseClick < cl_doubleClickTime.value)
			{
				CL_SelectNearbyUnitsByType();
			}
			else
			{
				CL_ResetSelectionRectangle();
				cl.showSelectionRect = true;
				cl.cmdrMode = CMDR_SELECTING_UNITS;
			}
			break;
	}
}

void	CL_CommanderMouseUp1()
{
	switch(cl.cmdrMode)
	{
		case CMDR_SELECTING_UNITS:
			//select the units
			if (cl.showSelectionRect)
			{
				CL_ProcessPotentialSelection();

				cl.showSelectionRect = false;
				CL_CommanderExitMode();
			}
			break;
		default:
			break;
	}
	cl.lastLeftMouseClick = cl.gametime;
}
//=============================================================================
//=============================================================================


//=============================================================================
//Right Click
//=============================================================================
void	CL_CommanderMouseDown2()
{
	int goal;
		
	if (cl.cmdrMode)
	{
		//right click cancels things like selection rectangle, placing a building, etc
		CL_CommanderExitMode();
		return;
	}

	if (cl.selection.numSelected && cl.selection.mobileUnitsSelected)
	{
		//perform context sensitive unit command		
		int objidx = CL_GetObjectUnderCursor();

		if (objidx >= 0)
		{
			goal = CL_DetermineGoalForObject(objidx);
			CL_SendObjectGoalToSelection(goal, objidx);
		}
		else
		{
			vec3_t pos;
			
			CL_GetTerrainPosAtCursorVec3(pos);
			CL_SendLocationGoalToSelection(GOAL_REACH_WAYPOINT, pos);			
		}
	}
}

void	CL_CommanderMouseUp2()
{
}
//=============================================================================
//=============================================================================


bool	CL_CommanderEscapeKey()
{
	//escape just cancels whatever mode we were in
	if (cl.cmdrMode)
	{		
		CL_CommanderExitMode();
		return true;
	}
	else if (cl.selection.numSelected > 0)
	{
		CL_ClearUnitSelection(&cl.selection);
		CL_SendSelectionToServer();
		CMDR_RefreshGrid();
		CL_InterfaceEvent(IEVENT_SELECTION);
		CMDR_RefreshSelectionIcons();
	}

	return false;
}

void	CL_RemoveDeadObjectsFromSelection()
{
	int i, j;
	bool changed = false;
	unitSelection_t tmp;

	tmp.numSelected = 0;
	j = 0;

	for (i = 0; i < cl.selection.numSelected; i++)
	{
		if (cl.objects[cl.selection.array[i]].visual.health >= 0 ||
			(cl.objects[cl.selection.array[i]].visual.health < 0 && !CL_ObjectType(cl.selection.array[i])))
		{
			tmp.array[j] = cl.selection.array[i];
			tmp.numSelected++;
			j++;
		}
		else
			changed = true;
	}

	if (changed)
	{
		CL_FinalizeUnitSelection(&tmp);
		cl.selection = tmp;
		CL_SendSelectionToServer();

		//update the grid menu with the selection
		CMDR_RefreshGrid();
		CL_InterfaceEvent(IEVENT_SELECTION);
		
		CMDR_RefreshSelectionIcons();
	}
}

/*==========================

  CL_CommanderFrame

 ==========================*/

void	CL_CommanderFrame()
{		
	switch (cl.cmdrMode)
	{
		case CMDR_PLACING_OBJECT:
		case CMDR_PLACING_LINK:
			CL_PlaceObjectFrame();
			break;
		case CMDR_PICKING_LOCATION:
			CL_PickingLocationFrame();
			CL_SelectingObjectsFrame();
			break;
		case CMDR_ACTIVATING_TECH:
			CL_ActivatingTechFrame();
			break;
		case CMDR_SELECTING_UNITS:
		default:
			CL_SelectingObjectsFrame();
			break;
	}

	if (cl.cmdrPendingInput & CMDR_MOUSEDOWN1)
		CL_CommanderMouseDown1();
	if (cl.cmdrPendingInput & CMDR_MOUSEUP1)
		CL_CommanderMouseUp1();
	if (cl.cmdrPendingInput & CMDR_MOUSEDOWN2)
		CL_CommanderMouseDown2();
	if (cl.cmdrPendingInput & CMDR_MOUSEUP2)
		CL_CommanderMouseUp2();

	cl.cmdrPendingInput = 0;
	
	CL_CommanderMoveCamera();

	CMDR_RefreshSelectedView();

	//if nothing is selected, we're done
	if (cl.selection.numSelected)	
	{
		clientObject_t *obj;
		obj = &cl.objects[cl.selection.array[0]];

		CL_RemoveDeadObjectsFromSelection();

		//update progress bar of selected object (if any)
		if (obj->visual.percentToComplete)
		{
			corec.Cvar_SetVarValue(&unit_progress, 100 - obj->visual.percentToComplete);
			corec.Cmd_Exec("show commander_unit_panel:progress_slider");
		}
		else
			corec.Cmd_Exec("hide commander_unit_panel:progress_slider");
	}
}

//purchase an item in the techtree.  for buildings, we'll call PlaceObject
void    CL_CommanderPurchase_Cmd(int argc, char *argv[])
{
	int	type = 0, builder1type, builder2type;

	if (!argc)
		return;
	
	SetInt(&type, FIRST_OBJECT_TYPE, LAST_OBJECT_TYPE, cl.objNames, argv[0]);
	
	if (!type)
	{
		corec.Console_Printf("Invalid tech type\n");
		return;
	}

	builder1type = CL_GetObjectTypeByName(CL_ObjectType(type)->builder[0]);
	builder2type = CL_GetObjectTypeByName(CL_ObjectType(type)->builder[1]);

	if (IsBuildingType(type))
	{
		//self building structure, with no builder specified, place anywhere
		if (!builder1type && !builder2type && CL_ObjectType(type)->selfBuild)
		{
			CL_StartPlaceObject(type);
			return;
		}

		if (cl.objects[cl.selection.array[0]].base.type == builder1type ||
			cl.objects[cl.selection.array[0]].base.type == builder2type ||
			cl.selection.array[0] < MAX_CLIENTS)
		{
			if (CL_ObjectType(type)->selfBuild || IsBuildingType(cl.objects[cl.selection.array[0]].base.type))
				CL_SendPurchaseRequest(type, cl.selection.array[0]);
			else
				CL_StartPlaceObject(type);
		}
	}
	else
	{
		if (Tech_IsBuilder(type, cl.objects[cl.selection.array[0]].base.type))
			CL_SendPurchaseRequest(type, cl.selection.array[0]);
	}
}

void	CL_SelectUnits_Cmd(int argc, char *argv[])
{
	int i;

	CL_ClearUnitSelection(&cl.potentialSelection);
	for (i = 0; i < argc; i++)
	{
		CL_AddToUnitSelection(&cl.potentialSelection, atoi(argv[i]));
	}

	CL_ProcessPotentialSelection();
}

void	CL_CancelGoal_Cmd(int argc, char *argv[])
{
	CL_SendLocationGoalToSelection(GOAL_NONE, zero_vec);		
}

void	CL_CommanderGotoEvent_Cmd(int argc, char *argv[])
{
	if (!cl.isCommander)
		return;

	if (cl.noticeCount < 1)
		return;

	cl.cmdrLookAtPoint[X] = cl.objects[cl.noticeQueue[cl.noticeIndex].objnum].visual.pos[X];
	cl.cmdrLookAtPoint[Y] = cl.objects[cl.noticeQueue[cl.noticeIndex].objnum].visual.pos[Y];
	CL_ClearUnitSelection(&cl.selection);
	CL_AddToUnitSelection(&cl.selection, cl.noticeQueue[cl.noticeIndex].objnum);
	CMDR_RefreshGrid();
	
	cl.noticeIndex--;
	if (cl.noticeIndex < 0)
		cl.noticeIndex = cl.noticeCount -1;
}


//=============================================================================
//Unit Grouping
//=============================================================================
void	CL_UnitGroup_Cmd(int argc, char *argv[])
{
	int groupnum;

	if (argc < 1)
		return;

	groupnum = atoi(argv[0]);

	if (groupnum < 0 || groupnum > 9)
	{
		corec.Console_DPrintf("Invalid group number: %i\n", groupnum);
		return;
	}

	//ctrl sets a group
	if (corec.Input_IsKeyDown(KEY_CTRL))
	{
		CL_ClearUnitSelection(&cl.grouplist[groupnum]);
		memcpy(&cl.grouplist[groupnum], &cl.selection, sizeof(unitSelection_t));
		return;
	}

	//shift adds to the group
	if (corec.Input_IsKeyDown(KEY_SHIFT))
	{
		int i;

		for (i = 0; i < cl.selection.numSelected; i++)
			CL_AddToUnitSelection(&cl.grouplist[groupnum], cl.selection.array[i]);

		return;
	}

	//alt does... something?
	if (corec.Input_IsKeyDown(KEY_ALT))
	{
		return;
	}

	//twice in a row centers on the group
	if (cl.gametime - cl.lastGroupCmdTime < 500 && groupnum == cl.selectedGroup)
	{
		int n;
		float	xmean = 0, ymean = 0;

		if (cl.grouplist[cl.selectedGroup].numSelected < 1)
			return;

		for (n = 0; n < cl.grouplist[cl.selectedGroup].numSelected; n++)
		{
			xmean += cl.objects[cl.grouplist[cl.selectedGroup].array[n]].base.pos[0];
			ymean += cl.objects[cl.grouplist[cl.selectedGroup].array[n]].base.pos[1];
		}
		
		xmean /= cl.grouplist[cl.selectedGroup].numSelected;
		ymean /= cl.grouplist[cl.selectedGroup].numSelected;

		cl.cmdrLookAtPoint[X] = xmean;
		cl.cmdrLookAtPoint[Y] = ymean;
		
		return;
	}

	//just select the group
	cl.selectedGroup = groupnum;
	cl.lastGroupCmdTime = cl.gametime;

	if (cl.grouplist[cl.selectedGroup].numSelected > 0)
	{
		CL_ClearUnitSelection(&cl.selection);
		memcpy(&cl.selection, &cl.grouplist[groupnum], sizeof(unitSelection_t));
		CL_SendSelectionToServer();
		CMDR_RefreshSelectionIcons();
		CMDR_RefreshGrid();
	}
}
//=============================================================================
//=============================================================================

void	CL_CommanderLookAtUnit(int unitnum)
{
	if (unitnum >= 0 && unitnum < MAX_CLIENTS 
		&& cl.clients[unitnum].info.active 
		&& cl.clients[unitnum].info.team == cl.info->team)
	{
		cl.cmdrLookAtPoint[X] = cl.objects[unitnum].base.pos[X];
		cl.cmdrLookAtPoint[Y] = cl.objects[unitnum].base.pos[Y];

		CL_ClearUnitSelection(&cl.potentialSelection);
		CL_AddToUnitSelection(&cl.potentialSelection, unitnum);
		CL_ProcessPotentialSelection(&cl.potentialSelection);
	}

}

void	CL_CommanderLookAtUnit_Cmd(int argc, char *argv[])
{
	int unitnum;
	
	if (!argc)
	{
		corec.Console_Printf("lookAtUnit <unitnumber>");
		return;
	}

	unitnum = atoi(argv[0]);
	CL_CommanderLookAtUnit(unitnum);
}

void	CL_CommanderLookAtTeamUnit_Cmd(int argc, char *argv[])
{
	int unitnum;
	
	if (!argc)
	{
		corec.Console_Printf("lookAtTeamUnit <unitnumber>");
		return;
	}

	unitnum = CL_TeamUnitToGlobalUnitNum(atoi(argv[0]));
	CL_CommanderLookAtUnit(unitnum);
}


/*==========================

  CL_GotoBase_Cmd

 ==========================*/

void	CL_GotoBase_Cmd(int argc, char *argv[])
{
	if (!cl.isCommander)
		return;

	cl.cmdrLookAtPoint[X] = cl.objects[cl.teams[cl.info->team].command_center].base.pos[X];
	cl.cmdrLookAtPoint[Y] = cl.objects[cl.teams[cl.info->team].command_center].base.pos[Y];
}


/*==========================

  CL_Demolish_Cmd

 ==========================*/

void	CL_Demolish_Cmd(int arc, char* argv[])
{
	if (cl.isCommander && cl.selection.numSelected == 1)
	{
		if (cl.objects[cl.selection.array[0]].base.team == cl.info->team &&
			IsBuildingType(cl.objects[cl.selection.array[0]].visual.type))
			corec.Client_SendMessageToServer(fmt("%s %i", CLIENT_COMMANDER_DEMOLISH_MSG, cl.selection.array[0]));
	}
}


/*==========================

  CL_Suicide_Cmd

 ==========================*/

void	CL_Suicide_Cmd(int argc, char *argv[])
{
	int index;

	for (index = 0; index < cl.selection.numSelected; index++)
	{
		if (cl.objects[cl.selection.array[index]].base.team == cl.info->team &&
			(IsWorkerType(cl.objects[cl.selection.array[index]].visual.type) || IsItemType(cl.objects[cl.selection.array[index]].visual.type)))
			corec.Client_SendMessageToServer(fmt("%s %i", CLIENT_COMMANDER_SUICIDE_MSG, cl.selection.array[index]));
	}
}


/*==========================

  CL_StopResearch_Cmd

 ==========================*/

void	CL_StopResearch_Cmd(int argc, char *argv[])
{
	if (cl.isCommander && cl.selection.numSelected == 1)
	{
		if (cl.objects[cl.selection.array[0]].base.team == cl.info->team &&
			IsBuildingType(cl.objects[cl.selection.array[0]].visual.type))
			corec.Client_SendMessageToServer(fmt("%s %i", CLIENT_COMMANDER_STOP_RESEARCH_MSG, cl.selection.array[0]));
	}
}



/*==========================

  CL_AddNotification

 ==========================*/

void	CL_AddNotification(int index, int type, bool playSound)
{
	if (!cl.isCommander)
		return;

	if (cl.noticeCount >= MAX_CMDR_NOTICES)
	{
		cl.noticeCount--;
		memcpy(cl.noticeQueue, &cl.noticeQueue[1], sizeof(commanderNotice_t) * cl.noticeCount);
	}

	cl.noticeQueue[cl.noticeCount].expireTime = cl.gametime + cl_cmdrNoticePersistTime.integer;
	cl.noticeQueue[cl.noticeCount].objnum = index;
	cl.noticeQueue[cl.noticeCount].noticeType = type;
	cl.noticeIndex = cl.noticeCount;
	cl.noticeCount++;

	if (playSound)
		CL_Play2d(Snd("commander_notice"), 1.0, CHANNEL_AUTO);
}


//=============================================================================
// called every world load
void	CL_CommanderReset()
{
	int n;

	for (n = 0; n < 10; n++)
		CL_ClearUnitSelection(&cl.grouplist[n]);
	cl.selectedGroup = -1;
	cl.lastGroupCmdTime = cl.gametime;
}

//called only at startup
void	CL_CommanderInit()
{
	corec.Cvar_Register(&cl_cmdr_camtiltx);
	corec.Cvar_Register(&cl_cmdr_camtilty);
	corec.Cvar_Register(&cl_cmdr_camtiltz);
	corec.Cvar_Register(&cl_cmdr_zoom);
	corec.Cvar_Register(&cl_cmdr_zoomsmooth);
	corec.Cvar_Register(&cl_cmdr_swivel);
	corec.Cvar_Register(&cl_cmdr_camConstrain);
	corec.Cvar_Register(&cl_cmdr_anglelerp);

    corec.Cvar_Register(&cl_cmdr_scrollaccel);
	corec.Cvar_Register(&cl_cmdr_scrollspeed);
	corec.Cvar_Register(&cl_cmdr_zoomspeed);
	corec.Cvar_Register(&cl_cmdr_rotatespeed);
	corec.Cvar_Register(&cl_cmdr_scrollup);
	corec.Cvar_Register(&cl_cmdr_scrolldown);
	corec.Cvar_Register(&cl_cmdr_scrollleft);
	corec.Cvar_Register(&cl_cmdr_scrollright);
	corec.Cvar_Register(&cl_cmdr_maxheight);	
	corec.Cvar_Register(&cl_cmdr_fov);

	corec.Cvar_Register(&cl_cmdr_minimapRefresh);
	
	corec.Cvar_Register(&cl_selectRegionRadius);
	corec.Cvar_Register(&cl_orderClickRadius);

	corec.Cvar_Register(&cl_placementLeniency);

	corec.Cvar_Register(&cl_selectionSoundVolume);

	corec.Cvar_Register(&cl_cmdr_lockCameraToMouse);

	corec.Cvar_Register(&cl_cblclickSelectRange);

	corec.Cmd_Register("purchase",				CL_CommanderPurchase_Cmd);
	corec.Cmd_Register("rotateBuilding",		CMDR_RotateBuilding_Cmd);
	corec.Cmd_Register("clearTargets",			CL_ClearTargets_Cmd);	
	corec.Cmd_Register("pickLocation",			CL_PickLocation_Cmd);
	corec.Cmd_Register("pickUnit",				CL_PickUnit_Cmd);
	corec.Cmd_Register("radiusTask",			CL_RadiusTask_Cmd);
	corec.Cmd_Register("commanderGotoEvent",	CL_CommanderGotoEvent_Cmd);
	corec.Cmd_Register("commanderGotoBase",		CL_GotoBase_Cmd);
	corec.Cmd_Register("selectUnits",			CL_SelectUnits_Cmd);
	corec.Cmd_Register("cancelGoal",			CL_CancelGoal_Cmd);

	corec.Cmd_Register("unitgroup",				CL_UnitGroup_Cmd);
	corec.Cmd_Register("lookatunit", 			CL_CommanderLookAtUnit_Cmd);
	corec.Cmd_Register("lookatteamunit", 		CL_CommanderLookAtTeamUnit_Cmd);
	corec.Cmd_Register("demolish",				CL_Demolish_Cmd);
	corec.Cmd_Register("suicide",				CL_Suicide_Cmd);
	corec.Cmd_Register("stopResearch",			CL_StopResearch_Cmd);
	corec.Cmd_Register("findidle",				CL_SelectNextIdleWorker_cmd);

	CL_CommanderReset();

	CMDR_InitInterface();
	CMDR_InitRequests();
	CMDR_InitOfficers();
	CMDR_InitGridMenu();
	CMDR_InitUpgrades();
}
