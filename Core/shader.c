// (C) 2003 S2 Games

// shader.c

// shader implementation

#include "core.h"

float	uvSawtooth(shader_t *me, int paramidx, float time, float tc)
{
	return (float)(((time*me->frequency[paramidx])-floor(time*me->frequency[paramidx]))*me->amplitude[paramidx]) + tc;
}

float	uvSin(shader_t *me, int paramidx, float time, float tc)
{
//	float tmp = tc - floor(tc);
	float tmp = tc;
	return (float)(sin((time*me->frequency[paramidx]) * M_PI) * me->amplitude[paramidx]) + tmp;
}


float	uvNull(shader_t *me, int paramidx, float time, float tc)
{
	return tc;
}


float	uvSquare(shader_t *me, int paramidx, float time, float tc)
{
	return tc;
}


float	uvTriangle(shader_t *me, int paramidx, float time, float tc)
{
	return tc;
}

float	uvRandom(shader_t *me, int paramidx, float time, float tc)
{
	return ((float)(rand()%1000) / (me->frequency[paramidx]*1000.0)) * me->amplitude[paramidx] + tc;
}
