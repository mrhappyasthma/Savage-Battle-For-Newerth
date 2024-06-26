// (C) 2002 S2 Games

// phys_shared.c

#include "game.h"

extern cvar_t p_stepheight;
extern cvar_t p_minslope;


/*==========================

  Phys_Slide

  attempt to move forward in a straight line.  if we impact something, slide along the plane

 ==========================*/

int	Phys_Slide(physicsParams_t *in, phys_out_t *out)
{
	float	remaining = in->frametime;
	vec3_t targetpos;
	int i = 0, j;
	vec3_t starting_vel;
	vec3_t last_nml = { 0,0,0 };
	bool	hitSomething = false;
	traceinfo_t traceinfo;
//	int			unlinked[255], numunlinked = 0;

	if (in->velocity[0]==0.0 && in->velocity[1]==0.0 && in->velocity[2]==0.0)
		return SLIDE_OK;

	//in->velocity[0]=0;in->velocity[1]=0;//in->velocity[2]=1;
	M_CopyVec3(in->velocity, starting_vel);
	
	
	while (1)
	{
		//set up and perform trace
		M_MultVec3(in->velocity, remaining, targetpos);
		M_AddVec3(targetpos, in->pos, targetpos);

		in->tracefunc(&traceinfo, in->pos, targetpos, in->bmin, in->bmax, TRACE_PLAYER_MOVEMENT);

		if (traceinfo.embedded)
		{	
			return SLIDE_STUCK;
		}

		if (traceinfo.fraction >= 0)
		{
			//made it some or none of the way
			if (traceinfo.fraction < 1)
			{
				hitSomething = true;
				if (traceinfo.index > -1
					&& out->num_collisions < MAX_SLIDE_COLLISIONS)
				{
				  	for (j = 0; j < out->num_collisions; j++)
					{
						if (out->collisions[j].index == traceinfo.index)
							break;
					}
					if (j == out->num_collisions)
					{
						memcpy(&out->collisions[out->num_collisions], &traceinfo, sizeof(traceinfo_t));
						out->num_collisions++;
					}
				}
			}
			M_CopyVec3(traceinfo.endpos, in->pos);
		}

		if (traceinfo.fraction == 1)
		{
			//made it all the way
			if (hitSomething)
			{	
				return SLIDE_CLIPPED;
			}
			else
			{	
				return SLIDE_OK;
			}
		}

		if (M_DotProduct(traceinfo.normal, last_nml) > 0.99)	//naughty plane (hit the same plane twice)
		{
			//push us out from the plane a little
			in->velocity[0]+=traceinfo.normal[0];//*0.5;
			in->velocity[1]+=traceinfo.normal[1];//*0.5;
			in->velocity[2]+=traceinfo.normal[2];//*0.5;
		}
		else
		{
			//project our velocity onto this plane
			Phys_ParallelPlane(in->velocity, traceinfo.normal, in->velocity);
			if (M_DotProduct(in->velocity, starting_vel) < 0)
			{
				//obstructed
				M_CopyVec3(zero_vec, in->velocity);
	
//				M_CrossProduct(in->velocity, starting_vel, in->velocity);
				return SLIDE_OBSTRUCTED;
			}

		}

		remaining -= remaining * traceinfo.fraction;
		M_CopyVec3(traceinfo.normal, last_nml);

		i++;
		if (i > 10)
		{
			//get out of stuck places hackity hack
			in->velocity[2] = 100;
			return SLIDE_OVERTRACED;
		}
	}
}


/*==========================

  Phys_SmartSlide

  walk over bumps in the ground

  ==========================*/

int		Phys_SmartSlide(physicsParams_t *in, phys_out_t *out)
{
	int result;
	vec3_t orig_vel, orig_pos;

	//save the current velocity and position
	M_CopyVec3(in->velocity, orig_vel);
	M_CopyVec3(in->pos, orig_pos);

	out->num_collisions = 0;

	M_MultVec2(in->velocity, 0.5, in->velocity);
	result = Phys_Slide(in, out);

	//stepping code
//	if (result==SLIDE_OBSTRUCTED)	//this might be a bump we can step over	
	{
		vec3_t steptarget;
		traceinfo_t steptrace;
		vec3_t stepup = { 0,0,in->stepheight };
		vec3_t stepdown = { 0,0,-in->stepheight*2 };

		M_AddVec3(in->pos, stepup, steptarget);

		//trace up by stepheight to see if we can try another slide above the current position

		in->tracefunc(&steptrace, steptarget, steptarget, in->bmin, in->bmax, TRACE_PLAYER_MOVEMENT);

		if (steptrace.fraction > 0)
		{
			//slide above the current position
			M_CopyVec3(steptrace.endpos, in->pos);
		}
		else
		{
			M_CopyVec3(orig_vel, in->velocity);
			return result;
		}

		//slide above the step using our original velocity		
		M_CopyVec3(orig_vel, in->velocity);

		M_MultVec2(in->velocity, 0.5, in->velocity);
		result = Phys_Slide(in, out);
		if (result == SLIDE_OBSTRUCTED)
		{
			M_CopyVec3(orig_vel, in->velocity);
			M_CopyVec3(orig_pos, in->pos);
			return result;
		}

		M_CopyVec3(orig_vel, in->velocity);
		
		//now trace down again to see if we're still on the ground
		//add in an extra step distance to properly move down slopes
		M_AddVec3(in->pos, stepdown, steptarget);
		in->tracefunc(&steptrace, in->pos, steptarget, in->bmin, in->bmax, TRACE_PLAYER_MOVEMENT);

		if (steptrace.fraction == 1)
		{
			steptrace.endpos[2] += in->stepheight;
			M_CopyVec3(steptrace.endpos, in->pos);

			return result;
		}
		if (steptrace.fraction > 0/* && steptrace.normal[2] >= p_minslope.value*/)
		{			
			//finished smart slide
			M_CopyVec3(steptrace.endpos, in->pos);
			return result;
		}
	}
	return result;
}
