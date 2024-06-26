/*
 * (C) 2002 S2 Games
 * Client world functions
 */

#include "client_game.h"

float CL_SampleBrightness(vec3_t pos)
{
	float brightness;
	bvec4_t col;
	corec.WR_GetColormap((int)corec.WorldToGrid(pos[0]), (int)corec.WorldToGrid(pos[1]), col);

	brightness = (col[0] + col[1] + col[2]) / 3.0;
	brightness *= BYTE_TO_FLOAT;

	return brightness;
}

float   CL_GetHighestSolidPoint(float x, float y)
{
	vec3_t wayUpThere = { x, y, 99999 };
	vec3_t wayDownHere = { x, y, -99999 };
	traceinfo_t trace;
	
	corec.World_TraceBox(&trace, wayUpThere, wayDownHere, zero_vec, zero_vec, 0);
	
	return trace.endpos[Z];
}

