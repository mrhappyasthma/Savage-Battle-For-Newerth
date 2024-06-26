// (C) 2001 S2 Games

// tl_objects.c

#include "tl_shared.h"

//for use with object anim_* variables
//anim_* vars are specified in the following way:
//
//startframe endframe loop(0 or 1) loopbackframe fps
//ex: 0 10 30 1 5   //plays an animation at 30fps from frame 0 to frame 10, loops back to frame 5, and loops from frame 5-10
void TL_ParseAnimString(const char *string, anim_t *anim)
{
	int end = 0;

	anim->start = 0;
	anim->fps = 30;
	anim->looping = false;
	anim->loopbackframe = -1;
	anim->numframes = 0;

	sscanf(string, "%i %i %i %i %i", &anim->start, &end, &anim->fps, &anim->looping, &anim->loopbackframe);

	anim->numframes = (end - anim->start) + 1;

	if (anim->looping)
	{
		if (anim->loopbackframe == -1)
			anim->loopbackframe = anim->start;

		anim->numloopframes = (end - anim->loopbackframe) + 1;
	}

	if (!anim->numframes)
	{
		int i;
		i = 0;
	}

	corec.Console_Printf("parsing anim %s: start=%i numframes=%i looping=%s loopbackframe=%i fps=%i\n",string,anim->start,anim->numframes,anim->looping ? "YES" : "NO", anim->loopbackframe, anim->fps);
}