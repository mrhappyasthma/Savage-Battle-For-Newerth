// (C) 2001 S2 Games

// sound_manager.c


#include "core.h"

#define MAX_SOUND_OBJECTS			2048
#define MAX_SOUNDOBJECT_CHANNELS	4

#define SC_VALID_ID(_id)			((_id) < MAX_SOUND_OBJECTS && (_id) >= 0)

typedef struct
{
	sound_handle_t channels[MAX_SOUNDOBJECT_CHANNELS];
	
	vec3_t	pos;
	vec3_t	velocity;

	byte	c;		//use this to cycle through object channels for now
} soundObject_t;

soundObject_t soundobjs[MAX_SOUND_OBJECTS];

void	SC_Init()
{
	int i, j;
	memset(soundobjs, 0, sizeof(soundobjs));
	for (i = 0;  i < MAX_SOUND_OBJECTS; i++)
		for (j = 0; j < MAX_SOUNDOBJECT_CHANNELS; j++)
			soundobjs[i].channels[j] = -1;
}

void	SC_DoMove(int id, vec3_t pos)
{
	int n;

	for (n=0; n<MAX_SOUNDOBJECT_CHANNELS; n++)
	{
		if (soundobjs[id].channels[n] > -1)
		{			
			if (!Sound_Handle_IsActive(soundobjs[id].channels[n]))
			{
				soundobjs[id].channels[n] = -1;
				continue;
			}

			Sound_SetPosition(soundobjs[id].channels[n], pos);							
			Sound_SetVelocity(soundobjs[id].channels[n], soundobjs[id].velocity);
		}
	}
}

void	SC_MoveObject(int id, vec3_t pos)
{
	if (!SC_VALID_ID(id))
		return;

	M_SubVec3(pos, soundobjs[id].pos, soundobjs[id].velocity);
	M_CopyVec3(pos, soundobjs[id].pos);

	SC_DoMove(id, pos);
}

void	SC_ResetObjectPosition(int id, vec3_t pos)
{
	if (!SC_VALID_ID(id))
		return;

	M_CopyVec3(pos, soundobjs[id].pos);
	M_ClearVec3(soundobjs[id].velocity);

	SC_DoMove(id, pos);
}

void	SC_PlayObjectSound(int id, residx_t sound, float volume, int channel, bool loop)
{
	sound_handle_t *handle;

	if (!SC_VALID_ID(id))
		return;

	if (channel < -1 || channel >= MAX_SOUNDOBJECT_CHANNELS)
		return;
	
	if (channel == SC_AUTOCHANNEL)
	{
		channel = soundobjs[id].c;

		soundobjs[id].c++;
		if (soundobjs[id].c >= MAX_SOUNDOBJECT_CHANNELS)
			soundobjs[id].c = 0;
	}
	
	handle = &soundobjs[id].channels[channel];

	if (*handle > -1)
	{
		Console_DPrintf("Stopping sound at pos %i due to intentional channel overlap\n", *handle);
		Sound_Stop(*handle);			//stop the old sound if one was playing
	}

	if (loop)
		*handle = Sound_PlayLooped(sound, volume, soundobjs[id].pos, false);
	else
		*handle = Sound_Play(sound, volume, soundobjs[id].pos, false);

	if (*handle > -1)
	{
		Console_DPrintf("SM: Playing sound at pos %i\n", *handle);

		Sound_SetVelocity(*handle, soundobjs[id].velocity);
	}
	else
	{
		Console_DPrintf("error trying to play sample %i\n", sound);
	}
}

void	SC_StopObjectChannel(int id, int channel)
{
	sound_handle_t *handle;

	if (!SC_VALID_ID(id))
		return;

	if (channel == SC_AUTOCHANNEL || channel < -1 || channel >= MAX_SOUNDOBJECT_CHANNELS)
		return;

	handle = &soundobjs[id].channels[channel];

	if (*handle > -1)
	{
		Sound_Stop(*handle);
		*handle = -1;
	}
}
