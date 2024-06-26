// (C) 2003 S2 Games

// colorutils.c

// color utility functions

#include "core.h"

#define	COLOR_SCALE_DONT_CLAMP

void	Color_Clamp(vec4_t color)
{
#ifdef COLOR_SCALE_DONT_CLAMP
	//divide color components by the largest component

	float temp, scale;
	
	temp = color[0];
	if (color[1] > color[0]) temp = color[1];
	if (color[2] > temp) temp = color[2];

	scale = 1 / temp;

	color[0] *= scale;
	color[1] *= scale;
	color[2] *= scale;

#else
	if (color[0] > 1) color[0] = 1;
	if (color[1] > 1) color[1] = 1;
	if (color[2] > 1) color[2] = 1;
	if (color[0] < 0) color[0] = 0;
	if (color[1] < 0) color[1] = 0;
	if (color[2] < 0) color[2] = 0;
#endif
}

void	Color_ToFloat(bvec4_t color, vec4_t out)
{
	out[0] = color[0] / 255.0;
	out[1] = color[1] / 255.0;
	out[2] = color[2] / 255.0;
	out[3] = color[3] / 255.0;
}

void	Color_ToByte(vec4_t color, bvec4_t out)
{
	out[0] = color[0] * 0xff;
	out[1] = color[1] * 0xff;
	out[2] = color[2] * 0xff;
	out[3] = color[3] * 0xff;
}
