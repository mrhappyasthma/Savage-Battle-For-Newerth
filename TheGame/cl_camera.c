// (C) 2001 S2 Games

// cl_camera.c


#include "client_game.h"

#define DEATH_ROLL	40

#define POSITIONREF cl.predictedState.pos

extern cvar_t cl_cameraDistance;
extern cvar_t cl_cameraLerp;
extern cvar_t cl_cameraAngleLerp;
extern cvar_t cl_cameraPosLerp;
extern cvar_t cl_cameraDistLerp;
extern cvar_t cl_offsetz;
extern cvar_t cl_fov;
extern cvar_t cl_zoomLerp;
extern cvar_t cl_chaseDistance;
extern cvar_t cl_usePlayerSoundPosition;
extern cvar_t cl_thirdPerson;

void	Cam_DetermineViewpoint()
{
	objectData_t *unit = CL_ObjectType(cl.predictedState.unittype);

	corec.Cvar_SetValue("wo_translucent", 0 );

	if (!cl.draw_world)
		return;

	cl.camera.time = Rend_SceneTime();

	cl.camera.width = corec.Vid_GetScreenW();
	cl.camera.height = corec.Vid_GetScreenH();

	
	if (cl.serverStatus == GAME_STATUS_ENDED)
	{		
		vec3_t angles = { WORD2ANGLE(cl.inputstate.pitch) + cl.effects.angleOffset[0], cl.effects.angleOffset[1], WORD2ANGLE(cl.inputstate.yaw) + cl.effects.angleOffset[2]};
 		
		cl.thirdperson = true;		
		
		//use playerstate.pos here because we want to make sure it's exactly what was sent over from the server
		cl.camera.origin[0] = cl.playerstate.pos[0] + cl.effects.posOffset[0];
		cl.camera.origin[1] = cl.playerstate.pos[1] + cl.effects.posOffset[1];
		cl.camera.origin[2] = cl.playerstate.pos[2] + cl.effects.posOffset[2];
		cl.camera.fovx = 90; //cl_fov.value;

		corec.Sound_SetListenerPosition(cl.camera.origin, cl.camera.viewaxis[FORWARD], cl.camera.viewaxis[UP], true);

		if (angles[PITCH] > -30)
			angles[PITCH] = -30;

		Cam_SetAngles(&cl.camera, angles);
		Cam_SetDistance(&cl.camera, /*cl.isCommander ? 1500 : */1000);
	}
	else if (cl.isCommander)
	{
		corec.Cvar_SetValue("wo_translucent", 1);

		CL_CommanderSetupCamera();

		return;
	}
	else if (cl.predictedState.phys_mode == PHYSMODE_FREEFLY)
	{
		vec3_t angles;
			
		angles[0] = cl.predictedState.angle[PITCH];
		angles[1] = cl.predictedState.angle[ROLL];
		angles[2] = cl.predictedState.angle[YAW];

		cl.camera.origin[0] = POSITIONREF[0] + cl.effects.posOffset[0];
		cl.camera.origin[1] = POSITIONREF[1] + cl.effects.posOffset[1];
		cl.camera.origin[2] = POSITIONREF[2] + 15 + cl.effects.posOffset[2];
		cl.camera.fovx = cl_fov.integer;

		Cam_SetAngles(&cl.camera, angles);

		Cam_CalcFovy(&cl.camera);

		corec.Sound_SetListenerPosition(cl.camera.origin, cl.camera.viewaxis[FORWARD], cl.camera.viewaxis[UP], false);
	}
	else
	{
		//set third or first person view
		static float fovx = 90;
		playerState_t   *ps = &cl.predictedState;
		objectData_t *obj = CL_ObjectType(ps->inventory[ps->item]);

		if (cl.predictedState.item == 0)
			corec.Cvar_SetVarValue(&cl_fov, 90);

		if (cl_fov.value > 100)
			corec.Cvar_SetVarValue(&cl_fov, 100);
		
		fovx = M_ClampLerp(cl.frametime * cl_zoomLerp.value, fovx, cl_fov.value);
		if (cl.predictedState.item > 0)
		{
			if (obj && obj->minfov > fovx)
				fovx = obj->minfov;
		}

		//should we be in third person?
		if (cl_alwaysThirdPerson.integer)
		{
			cl.thirdperson = true;
		}
		else
		{			
			if ((IsWeaponType(cl.predictedState.inventory[cl.predictedState.item]) || IsItemType(cl.predictedState.inventory[cl.predictedState.item])) && 
				!unit->isVehicle && 
				!(cl.predictedState.flags & PS_CHASECAM) &&
				unit->allowFirstPerson)
			{
				cl.thirdperson = false;
			}
			else
			{
				cl.thirdperson = true;
			}
		}

		if (cl.thirdperson)
		{
			traceinfo_t trace;
			vec3_t head = { POSITIONREF[0], POSITIONREF[1], POSITIONREF[2] + unit->viewheight };
			vec3_t pushout_min = { 0,0,0 };
			vec3_t pushout_max = { 0,0,0 };			
			vec3_t newangles;
			vec3_t newpos;			
			float lerp;
			bool warp = false;
			
			objectData_t *unit = CL_ObjectType(cl.predictedState.unittype);
			
			if (cl.predictedState.flags & PS_CHASECAM)
			{
				newpos[0] = cl.predictedState.chasePos[0];
				newpos[1] = cl.predictedState.chasePos[1];
				newpos[2] = cl.predictedState.chasePos[2];

				newangles[0] = cl.predictedState.angle[PITCH] + cl.effects.angleOffset[0];
				newangles[1] = cl.effects.angleOffset[1];
				newangles[2] = cl.predictedState.angle[YAW] + cl.effects.angleOffset[2];

				cl.camera.fovx = 90;				
			}
			else if (unit->isVehicle)
			{
				newpos[0] = POSITIONREF[0];
				newpos[1] = POSITIONREF[1];
				newpos[2] = POSITIONREF[2] + unit->viewheight;

				newangles[0] = cl.predictedState.angle[PITCH] + cl.effects.angleOffset[0];
				newangles[1] = (cl.predictedState.angle[ROLL] + cl.effects.angleOffset[1]) * 0.4; //dampen the roll a little
				newangles[2] = cl.predictedState.angle[YAW];

				cl.camera.fovx = 90;
			}
			else
			{
				newpos[0] = POSITIONREF[0];
				newpos[1] = POSITIONREF[1];
				newpos[2] = POSITIONREF[2] + unit->viewheight + cl_offsetz.value;

				newangles[0] = cl.predictedState.angle[PITCH] + cl.effects.angleOffset[0];
				newangles[1] = cl.predictedState.angle[ROLL] + cl.effects.angleOffset[1];
				newangles[2] = cl.predictedState.angle[YAW];

				cl.camera.fovx = fovx;
				if (cl.camera.fovx > 179)
					cl.camera.fovx = 179;
				if (cl.camera.fovx < 20)
					cl.camera.fovx = 20;
			}

			if (cl.tpLastFrame != cl.frame-1 || 
				cl.predictedState.chaseIndex != cl.oldPlayerState.chaseIndex || 
				(cl.predictedState.flags & PS_CHASECAM) != (cl.oldPlayerState.flags & PS_CHASECAM))
			{
				warp = true;				
			}
			
			
			lerp = warp ? 1 : cl_cameraAngleLerp.value * cl.frametime;

			if (lerp > 1)
				lerp = 1;
			if (lerp < 0)
				lerp = 0;			
			
			M_LerpAngleVec3(lerp, cl.tpAngles, newangles, cl.tpAngles);

			lerp = warp ? 1 : cl_cameraPosLerp.value * cl.frametime;
			if (lerp > 1)
				lerp = 1;
			if (lerp < 0)
				lerp = 0;
			
			M_LerpVec3(lerp, cl.tpPos, newpos, cl.tpPos);
			M_CopyVec3(cl.tpPos, cl.camera.origin);
			//set listener position here, before we zoom out
			corec.Sound_SetListenerPosition(cl.camera.origin, cl.camera.viewaxis[FORWARD], cl.camera.viewaxis[UP], warp);

			lerp = warp ? 1 : cl_cameraDistLerp.value * cl.frametime;
			if (lerp > 1)
				lerp = 1;
			if (lerp < 0)
				lerp = 0;			

			if (cl.predictedState.flags & PS_CHASECAM)
			{
				if (cl.predictedState.flags & PS_INSIDE_TRANSPORT)
					cl.tpNewDistance = 150;
				else
					cl.tpNewDistance = cl_chaseDistance.value;
			}
			else
			{
				if (unit->isVehicle && !(cl.predictedState.flags & PS_NOT_IN_WORLD))
					cl.tpNewDistance = LERP(lerp, cl.tpNewDistance, unit->distOffset);
				else
					cl.tpNewDistance = LERP(lerp, cl.tpNewDistance, cl_cameraDistance.value + unit->distOffset);
			}
			
			if (cl_cameraDistance.value > 150)
			{
				corec.Cvar_SetVarValue(&cl_cameraDistance, 150);
			}
			else if (cl_cameraDistance.value < 20)
			{
				corec.Cvar_SetVarValue(&cl_cameraDistance, 20);
			}

			Cam_SetAngles(&cl.camera, cl.tpAngles);

			Cam_CalcFovy(&cl.camera);

			Cam_SetDistance(&cl.camera, cl.tpNewDistance);		
	
			//don't have the camera disappear through objects
			corec.World_TraceBox(&trace, cl.tpPos, cl.camera.origin, pushout_min, pushout_max, TRACE_PLAYER_MOVEMENT);

			if (trace.fraction < 1)
			{					
				M_PointOnLine(trace.endpos, cl.camera.viewaxis[FORWARD], 1, cl.camera.origin);
			}

			//add in effect offset after final position has been computed
			M_AddVec3(cl.camera.origin, cl.effects.posOffset, cl.camera.origin);

			cl.tpLastFrame = cl.frame;
		}
		else
		{
			vec3_t angles;
			
			angles[0] = cl.predictedState.angle[PITCH] + cl.effects.angleOffset[0];
			angles[1] = cl.predictedState.angle[ROLL] + cl.effects.angleOffset[1];
			angles[2] = cl.predictedState.angle[YAW] + cl.effects.angleOffset[2];

			cl.camera.origin[0] = POSITIONREF[0] + cl.effects.posOffset[0];
			cl.camera.origin[1] = POSITIONREF[1] + cl.effects.posOffset[1];
			cl.camera.origin[2] = POSITIONREF[2] + cl.effects.posOffset[2] + unit->viewheight;
			cl.camera.fovx = fovx;

			Cam_SetAngles(&cl.camera, angles);

			Cam_CalcFovy(&cl.camera);

			corec.Sound_SetListenerPosition(cl.camera.origin, cl.camera.viewaxis[FORWARD], cl.camera.viewaxis[UP], false);
		}
	}

	corec.Cvar_SetVarValue(&cl_thirdPerson, cl.thirdperson);
}

