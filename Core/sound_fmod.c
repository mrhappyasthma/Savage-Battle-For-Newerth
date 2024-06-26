// (C) 2003 S2 Games

// sound_fmod.c


#include "core.h"

#ifndef _WIN32
	#include "wincompat.h"
#endif

#include "fmoddyn.h"
#include "fmod_errors.h"	// optional


//
// cvars
//

cvar_t	snd_disable = {"snd_disable", "0", CVAR_SAVECONFIG };
cvar_t	sound_debug = { "sound_debug", "0", CVAR_DONTSAVE };
cvar_t	sound_masterVolume = {"sound_masterVolume", "1.0", CVAR_SAVECONFIG };
//cvar_t	sound_distanceFactor = { "sound_distanceFactor", "1", CVAR_DONTSAVE };
//cvar_t	sound_rolloff = { "sound_rolloff", "1.0", CVAR_DONTSAVE };
cvar_t	sound_musicVolume = { "sound_musicVolume", "0.75", CVAR_SAVECONFIG };
cvar_t	sound_sfxVolume = { "sound_sfxVolume", "1.0", CVAR_SAVECONFIG };
cvar_t	sound_dopplerFactor = { "sound_dopplerFactor", "0", CVAR_DONTSAVE };
cvar_t	sound_defaultMinFalloff = { "sound_defaultMinFalloff", "100", CVAR_DONTSAVE };
cvar_t	sound_defaultMaxFalloff = { "sound_defaultMaxFalloff", "1000", CVAR_DONTSAVE };
cvar_t	sound_mixrate = { "sound_mixrate", "44100", CVAR_SAVECONFIG };
cvar_t	sound_noMusic = { "sound_noMusic", "0", CVAR_SAVECONFIG };
cvar_t	sound_numChannels = { "sound_numChannels", "32", CVAR_VALUERANGE | CVAR_SAVECONFIG, 8, 256 };
cvar_t	sound_noCullLooping = { "sound_noCullLooping", "0" };
cvar_t	sound_driver = { "sound_driver", "", CVAR_SAVECONFIG };
cvar_t	sound_numDrivers = { "sound_numDrivers", "0" };
cvar_t	sound_softwareMode = { "sound_softwareMode", "0", CVAR_SAVECONFIG };
//cvar_t	sound_softwareMixing = { "sound_softwareMixing", "0", CVAR_SAVECONFIG };

//
// structs
//

FMOD_INSTANCE  *fmodapi = NULL;

#define	MAX_SOUND_SOURCES	8192
#define MAX_LOOPING_SOUNDS	256
#define MAX_CHANNELS 256

typedef struct
{
	FSOUND_STREAM	*stream;
	int				channel;	
	bool			queueStop;
	char			filename[1024];
} musicStatus_t;

typedef struct
{
	vec3_t			pos;
	vec3_t			vel;
	bool			useVel;
} soundSource_t;

static musicStatus_t	music;

bool	sound_initialized = false;

typedef struct
{
	int				sourcenum;
	residx_t		sound;
	int				priority;
	float			volume;
} loopingSound_t;

typedef struct
{
	bool			inUse;
	int				index;
	int				sourcenum;
	unsigned int	handle;
	bool			looping;
	int				loopframe;
	residx_t		sound;
	int				priority;
	float			volume;
	vec3_t			pos;
	bool			usePos;
} soundChannel_t;

typedef struct
{
	vec3_t			pos;
} listener_t;

static int numChannels;

static loopingSound_t loopingSounds[MAX_LOOPING_SOUNDS];
static int numLoopingSounds = 0;
static soundChannel_t channels[MAX_CHANNELS];
static unsigned int handleCounter = 0;
static soundSource_t sources[MAX_SOUND_SOURCES];
static listener_t	listener;


//
// functions
//




/*==========================

  Sound_CalcChannelVolume

  necessary because of the broken FMOD attenuation

 ==========================*/

float	Sound_CalcChannelVolume(soundChannel_t *chan)
{
	float *pos;
	sample_t *sample;
	float dist;
	float attenmin, attenmax;
	float vol;
	float div;
	float between;
	float range;
	float chanvol;

	sample = Res_GetSample(chan->sound);
	if (!sample)
		return chan->volume * 255;		//this will be the case with streams

	chanvol = sample->volume * chan->volume;

	if (chan->sourcenum == -1)
	{
		if (!chan->usePos)
		{
			return chanvol * 255;
		}
		else
		{
			pos = chan->pos;			
		}
	}
	else
	{
		pos = sources[chan->sourcenum].pos;
	}


	dist = sqrt(M_GetDistanceSq(listener.pos, pos));
	attenmin = sample->falloff_min;
	attenmax = sample->falloff_max;

	if (dist <= attenmin)
		return chanvol * 255;
	if (dist >= attenmax)
		return 0.0;

	between = dist - attenmin;
	range = attenmax - attenmin;	

	div = 1 - between/range;
	if (!sample->falloff_linear)
		div *= div;

	vol = chanvol * div * 255;

	if (sound_debug.integer)
	{
		Console_DPrintf("atten(%i): %f\n", chan->sourcenum, vol);
	}

	return vol;
}

/*==========================

  Sound_Play

  if pos is NULL, the sound will follow the sourcenum specified

  a handle to the sound is returned.  in most cases this is not needed, but may be occasionally useful
  if tracking of a specific sound effect is needed

  if channel is -1 (CHANNEL_AUTO), then a free channel will be found to play the sound, assuming a
  channel with an equal or lower priority can be found


 ==========================*/

unsigned int Sound_Play(residx_t sampleidx, int sourcenum, const vec3_t pos, float volume, int channel, int priority)
{
	int useChannel = -1;
	sample_t *sample = Res_GetSample(sampleidx);
	float *soundPos = NULL;

	if (!sound_initialized)
		return 0;

	if (!sample)
		return 0;

	if (sourcenum < 0 || sourcenum >= MAX_SOUND_SOURCES)
		sourcenum = -1;

	if (sourcenum >= 0)
		soundPos = sources[sourcenum].pos;
	else if (pos)
		soundPos = (float *)pos;

	if (soundPos)
	{
		//cull out the sound if it's out of range		
		float distsq = M_GetDistanceSq(soundPos, listener.pos);
		if (distsq > ((sample->falloff_max + 100) * (sample->falloff_max + 100)))
			return 0;
	}

	if (channel == CHANNEL_AUTO)
	{
		int n;	

		for (n=CHANNEL_FIRST_AUTO; n<numChannels; n++)
		{
			if (!channels[n].inUse)
			{
				useChannel = n;
				break;
			}			
		}

		if (useChannel == -1)
		{
			//all channels are in use
			//find one to override, if any

			for (n=CHANNEL_FIRST_AUTO; n<numChannels; n++)
			{
				if (channels[n].priority <= priority)
				{
					useChannel = n;
					break;
				}
			}			
		}
	}
	else
	{
		useChannel = channel;
	}

	if (useChannel >= 0 && useChannel < numChannels)
	{
		soundChannel_t *chan = &channels[useChannel];

		//found a channel to use			
		fmodapi->FSOUND_PlaySound(useChannel, sample->data);

		chan->inUse = true;
		chan->handle = ++handleCounter;
		chan->sourcenum = sourcenum;
		if (pos)
		{
			M_CopyVec3(pos, chan->pos);
			chan->usePos = true;
		}
		else
		{			
			chan->usePos = false;
		}
		chan->priority = priority;
		chan->sound = sampleidx;
		chan->looping = false;
		chan->volume = volume;

		if (sourcenum >= 0 && sourcenum < MAX_SOUND_SOURCES)
		{
			//ignore position given.  use the source position instead
			fmodapi->FSOUND_3D_SetAttributes(
				useChannel,
				sources[sourcenum].pos,
				sources[sourcenum].useVel ? sources[sourcenum].vel : NULL);
		}
		else
		{
			fmodapi->FSOUND_3D_SetAttributes(
				useChannel,
				(float *)pos,
				NULL);
		}

		fmodapi->FSOUND_SetLoopMode(useChannel, FSOUND_LOOP_OFF);

		fmodapi->FSOUND_SetVolume(useChannel, Sound_CalcChannelVolume(chan) * sound_sfxVolume.value);

		if (sound_debug.integer)
		{
			Console_Printf("Sound_Play(): sample %i, chan %i, source %i\n", sampleidx, useChannel, sourcenum);
		}

		return chan->handle;
	}

	return 0;
}



/*==========================

  Sound_ClearChannelData

  done on initialization and in Sound_StopAllSounds()

 ==========================*/

void	Sound_ClearChannelData()
{
	int n;

	memset(channels, 0, sizeof(channels));

	for (n=0; n<MAX_CHANNELS; n++)
	{
		channels[n].sourcenum = -1;
		channels[n].index = n;
	}
}


/*==========================

  Sound_StopSource

  stops all sounds emitting from the given source

 ==========================*/

void	Sound_StopSource(int sourcenum)
{
	int n;

	if (!sound_initialized)
		return;

	for (n=0; n<numChannels; n++)
	{
		if (!channels[n].inUse)
			continue;

		if (channels[n].sourcenum == sourcenum)
		{
			fmodapi->FSOUND_StopSound(n);
			channels[n].inUse = false;
		}
	}
}



/*==========================

  Sound_StopHandle

  stops the given sound handle

 ==========================*/

void	Sound_StopHandle(unsigned int handle)
{
	int n;

	if (!sound_initialized)
		return;
	if (!handle)
		return;

	for (n=0; n<numChannels; n++)
	{
		if (!channels[n].inUse)
			continue;

		if (channels[n].handle == handle)
		{
			fmodapi->FSOUND_StopSound(n);
			channels[n].inUse = false;

			break;		//handles are unique to one channel
		}
	}
}


/*==========================

  Sound_StopChannel

  stop the given sound channel

 ==========================*/

void	Sound_StopChannel(int channel)
{
	if (!sound_initialized)
		return;

	if (channel < 0 || channel >= numChannels)
		return;

	fmodapi->FSOUND_StopSound(channel);
	channels[channel].inUse = false;
}


/*==========================

  Sound_MoveSource

  move a sound source to a new location

  velocity can be NULL

 ==========================*/

void	Sound_MoveSource(int sourcenum, vec3_t pos, vec3_t velocity)
{
	if (!sound_initialized)
		return;

	if (sourcenum < 0 || sourcenum >= MAX_SOUND_SOURCES)
		return;

	M_CopyVec3(pos, sources[sourcenum].pos);
	if (velocity)
	{
		M_CopyVec3(velocity, sources[sourcenum].vel);
		sources[sourcenum].useVel = true;
	}
	else
	{
		sources[sourcenum].useVel = false;
	}
}



/*==========================

  Sound_AddLoopingSound

  each sound source can have one looping sound attached to it

  to eliminate cases where a looping sound can be "forgotten about"
  and left playing, these sounds are cleared every frame and
  re-added.  a second advantage to doing things this way is that
  looping sound channels will not get permanently stolen by other
  sounds.

 ==========================*/

void	Sound_AddLoopingSound(residx_t sampleidx, int sourcenum, int priority)
{
	if (numLoopingSounds >= MAX_LOOPING_SOUNDS)
		return;	

	loopingSounds[numLoopingSounds].sound = sampleidx;
	loopingSounds[numLoopingSounds].sourcenum = sourcenum;
	loopingSounds[numLoopingSounds].priority = priority;
	loopingSounds[numLoopingSounds].volume = 1.0;

	numLoopingSounds++;
}


/*==========================

  Sound_AddLoopingSoundEx

  adds a volume option

 ==========================*/

void	Sound_AddLoopingSoundEx(residx_t sampleidx, int sourcenum, float volume, int priority)
{
	if (numLoopingSounds >= MAX_LOOPING_SOUNDS)
		return;	

	loopingSounds[numLoopingSounds].sound = sampleidx;
	loopingSounds[numLoopingSounds].sourcenum = sourcenum;
	loopingSounds[numLoopingSounds].priority = priority;
	loopingSounds[numLoopingSounds].volume = volume;

	numLoopingSounds++;
}




/*==========================

  Sound_ClearLoopingSounds

  called once per frame by the game code

 ==========================*/

void	Sound_ClearLoopingSounds()
{
	numLoopingSounds = 0;
}



/*==========================

  Sound_PlayMusic

  streams a sound file for background music

  the stream specified must be external to the archives

 ==========================*/

void	Sound_PlayMusic(const char *streamFile)
{
	char full_system_path[1024];	
	int flags = FSOUND_HW2D | FSOUND_NONBLOCKING | FSOUND_LOOP_NORMAL;

	if (!sound_initialized)
		return;

	//fixme: there is a chance that stopmusic will fail to close
	//the previous stream if the stream has not finished opening yet.
	//this could be fixed by repeatedly calling Sound_StopMusic
	//until it succeeds, but i don't want to risk an infinite loop
	//due to any possible bugs in FMOD.  plus hitching is never good...

	File_FixPath(streamFile, full_system_path, true);	

	if (music.channel > -1 && strcmp(full_system_path, music.filename)==0)
		return;

	Sound_StopMusic();

	music.stream = fmodapi->FSOUND_Stream_Open(full_system_path,
								flags,
								0,
								0);

	if (!music.stream)
		Console_Printf("Couldn't open stream %s\n", full_system_path);

	music.channel = -1;
	music.queueStop = false;
	strncpySafe(music.filename, full_system_path, sizeof(music.filename));
}


/*==========================

  Sound_StopMusic

 ==========================*/

void	Sound_StopMusic()
{
	if (!sound_initialized)
		return;

	if (music.stream)
	{
		if (music.channel)
		{
			fmodapi->FSOUND_Stream_Stop(music.stream);
			fmodapi->FSOUND_Stream_Close(music.stream);
			music.queueStop = false;
			music.channel = -1;
			music.stream = NULL;
		}
		else		
		{
			//music was opened but the stream isn't ready,
			//so we have to keep calling Sound_StopMusic()
			//until it's done
			music.queueStop = true;
		}
	}
}



/*==========================

  Sound_PlayMusic_Cmd

 ==========================*/

void	Sound_PlayMusic_Cmd(int argc, char *argv[])
{
	if (argc < 1)
		return;

	Sound_PlayMusic(argv[0]);
}



/*==========================

  Sound_StopMusic_Cmd

 ==========================*/

void	Sound_StopMusic_Cmd(int argc, char *argv[])
{
	Sound_StopMusic();
}


/*==========================

  Sound_StopAllSounds

 ==========================*/

void	Sound_StopAllSounds()
{
	if (!sound_initialized)
		return;

	if (sound_debug.integer)
	{
		Console_Printf("StopAllSounds()\n");
	}

	Sound_StopMusic();

	fmodapi->FSOUND_StopSound(FSOUND_ALL);

	Sound_ClearLoopingSounds();
	Sound_ClearChannelData();	
}





/*==========================

  Sound_RefreshSample
 
  updates FMOD with new sample settings

 ==========================*/

void	Sound_RefreshSample(sample_t *sample)
{
	int flags;

	if (sample->looping)
		flags = FSOUND_LOOP_NORMAL;
	else
		flags = FSOUND_LOOP_OFF;

	fmodapi->FSOUND_Sample_SetMinMaxDistance(sample->data, 
											 100000000, 
											 100000000);
	fmodapi->FSOUND_Sample_SetMode(sample->data,
								   flags);
}



static sample_t	*sndmat_sample = NULL;

/*==========================

  Sound_SetLooping

 ==========================*/

void	Sound_SetLooping_Cmd(int argc, char *argv[])
{
	sndmat_sample->looping = atoi(argv[0]);
}

/*==========================

  Sound_SetFalloff

  sets min/max falloff for a sample

 ==========================*/

void	Sound_SetFalloff_Cmd(int argc, char *argv[])
{
	sndmat_sample->falloff_min = atof(argv[0]);	
	sndmat_sample->falloff_max = atof(argv[1]);
}


/*==========================

  Sound_SetVolume

  sets volume for a sample

 ==========================*/

void	Sound_SetVolume_Cmd(int argc, char *argv[])
{
	sndmat_sample->volume = atof(argv[0]);
}


/*==========================

  Sound_Set_Cmd

 ==========================*/

extern cvar_t svr_allowCheats;

void	Sound_Set_Cmd(int argc, char *argv[])
{
	if (!sndmat_sample)
		return;

	if (argc < 2)
	{
		Console_Printf( "================================\n"
			            "Properties for %s\n"
						"================================\n"
						"Volume: %f\n"
						"Falloff: %f to %f\n"						
						"Looping: %i\n",
						sndmat_sample->media->name,
						sndmat_sample->volume,
						sndmat_sample->falloff_min, sndmat_sample->falloff_max,
						sndmat_sample->looping);
		return;
	}
	if (!svr_allowCheats.integer)
		return;

	if (stricmp(argv[0], "volume")==0)
		Sound_SetVolume_Cmd(argc-1, &argv[1]);
	else if (stricmp(argv[0], "falloff")==0)
		Sound_SetFalloff_Cmd(argc-1, &argv[1]);
	else if (stricmp(argv[0], "looping")==0)
		Sound_SetLooping_Cmd(argc-1, &argv[1]);

	Sound_RefreshSample(sndmat_sample);
}



/*==========================

  Sound_Edit_Cmd

  for changing sample settings on the fly (testing purposes only)

 ==========================*/

void	Sound_Edit_Cmd(int argc, char *argv[])
{
	sample_t *sample;

	if (argc < 1)
	{
		Console_Printf("syntax: sndEdit <index>\n");
		Console_Printf("Type \"soundlist\" to see currently loaded sounds\n");
		return;
	}

	sample = Res_GetSample(atoi(argv[0]));
	if (!sample)
		return;

	sndmat_sample = sample;
}


/*==========================

  Sound_Load_Cmd

 ==========================*/

void	Sound_Load_Cmd(int argc, char *argv[])
{
	if (!argc)
		return;

	//touch the sample
	Res_LoadSound(argv[0]);
}

/*==========================

  Sound_Save_Cmd

  saves out a .sndmat file

 ==========================*/

void	Sound_Save_Cmd(int argc, char *argv[])
{
	file_t *file;
	char basefname[1024];
	char *outname;

	if (!sndmat_sample)
		return;

	Filename_StripExtension(sndmat_sample->media->name, basefname);
	outname = fmt("%s.sndmat", basefname);

	file = File_Open(outname, "w");
	if (!file)
	{
		Console_Printf("Couldn't open %s for writing\n", outname);
		return;
	}

	File_Printf(file, "sndSet volume %f\n", sndmat_sample->volume);
	File_Printf(file, "sndSet falloff %f %f\n", sndmat_sample->falloff_min, sndmat_sample->falloff_max);
	File_Printf(file, "sndSet looping %i\n", sndmat_sample->looping);

	Console_Printf("Wrote %s\n", outname);

	File_Close(file);
}


/*==========================

  Sound_LoadSample

  main sample loading function

 ==========================*/

bool Sound_LoadSample(const char *filename, sample_t *sample)
{	
	int flags = FSOUND_HW3D | FSOUND_LOADMEMORY | FSOUND_16BITS | FSOUND_SIGNED;
	char *sampleData;
	int sampleLen;
	char basefname[1024];
	char *sndmatfile;

	if (!sound_initialized)
		return true;

	memset(sample, 0, sizeof(*sample));

	sampleData = File_LoadIntoBuffer(filename, &sampleLen, MEM_SOUND);

	if (!sampleData)
	{
		Console_DPrintf("Failed to load sample %s\n", filename);
		return false;
	}

	//load it into fmod
	sample->data = fmodapi->FSOUND_Sample_Load(FSOUND_UNMANAGED,
										sampleData,
										flags,
										0,
										sampleLen);

	Tag_Free(sampleData);

	if (!sample->data)
		return false;
 
	//set up default properties
	sample->volume = 1.0;
	sample->falloff_linear = true;

	//set min max defaults
	sample->falloff_min = sound_defaultMinFalloff.value;
	sample->falloff_max = sound_defaultMaxFalloff.value;

	sample->looping = false;

	//set looping mode automatically if there's an "l_" in the filename
	if ((strncmp(Filename_GetFilename((char *)filename), "l_", 2) == 0) || strstr(Filename_GetFilename((char *)filename), "_l_"))
		sample->looping = true;

	//see if we have a sndmat file to load
	Filename_StripExtension(filename, basefname);

	sndmatfile = fmt("%s.sndmat", basefname);
	if (File_Exists(sndmatfile))
	{
		sndmat_sample = sample;
		Cmd_ReadConfigFile(sndmatfile, false);
		sndmat_sample = NULL;
	}

	Sound_RefreshSample(sample);

	return true;
}


/*==========================

  Sound_FreeSample

  frees the sample data from memory

 ==========================*/

void Sound_FreeSample(sample_t *sample)
{
	fmodapi->FSOUND_Sample_Free(sample->data);

}


/*==========================

  Sound_SetListenerPosition

  set the listener position

  derives velocity automatically, or ignores any doppler effects if warp == true

 ==========================*/

void	Sound_SetListenerPosition(const vec3_t pos, const vec3_t forward, const vec3_t up, bool warp)
{
	static vec3_t oldpos = { 0,0,0 };
	vec3_t vel;

	if (!sound_initialized)
		return;

	if (warp)
	{
		M_ClearVec3(vel);
		M_CopyVec3(pos, oldpos);
	}
	else
	{
		M_SubVec3(pos, oldpos, vel);
		//multiply by 1 / frameseconds to convert into units per second
		M_MultVec3(vel, 1 / Host_FrameSeconds(), vel);
		M_MultVec3(vel, sound_dopplerFactor.value, vel);
		M_CopyVec3(pos, oldpos);
	}

	//set global params
	fmodapi->FSOUND_3D_Listener_SetAttributes((float *)pos, (float *)vel, forward[0], forward[1], -forward[2], up[0], up[1], -up[2]);	

	M_CopyVec3(pos, listener.pos);
}



/*==========================

  Sound_SetGlobals

  set master volume and any other global sound settings

 ==========================*/

void	Sound_SetGlobals()
{
	fmodapi->FSOUND_SetSFXMasterVolume(sound_masterVolume.value * 255);
	fmodapi->FSOUND_3D_SetDopplerFactor(sound_dopplerFactor.value);
	fmodapi->FSOUND_3D_SetRolloffFactor(0);
	fmodapi->FSOUND_3D_SetDistanceFactor(1.0);
}




/*==========================

  Sound_UpdateLoopingSounds

  updates all looping sounds added to the looping sound list

 ==========================*/

void	Sound_UpdateLoopingSounds()
{
	int i,n;
	soundChannel_t *freeChannels[MAX_CHANNELS];
	soundChannel_t *loopingChannels[MAX_CHANNELS];
	int numFree = 0;
	int numLoopingChannels = 0;
	static int loopframe = 0;

	loopframe++;

	for (n=numChannels; n>=CHANNEL_FIRST_AUTO; n--)
	{
		//mark as free channels anything that's not in use, or playing a looping sound
		if (!channels[n].inUse)
		{
			freeChannels[numFree] = &channels[n];
			numFree++;
		}
		else if (channels[n].looping)
		{
			loopingChannels[numLoopingChannels] = &channels[n];
			numLoopingChannels++;
		}
	}

	for (n=0; n<numLoopingSounds; n++)
	{		
		soundChannel_t *useChan = NULL;
		bool audible = true;
		bool wasAlreadyPlaying = false;
		loopingSound_t *loopsound = &loopingSounds[n];
		float *soundPos = sources[loopsound->sourcenum].pos;
		sample_t *sample = Res_GetSample(loopsound->sound);

		if (!sample)
			continue;

		//determine if the sound is audible
		if (!sound_noCullLooping.integer)
		{
			float distsq = M_GetDistanceSq(soundPos, listener.pos);
			//Console_Printf("dist: %f\n", sqrt(distsq));
			if (distsq > (sample->falloff_max * sample->falloff_max))
				audible = false;			
		}

		//determine if the sound is already playing
		for (i=0; i<numLoopingChannels; i++)
		{
			if (loopingChannels[i]->sourcenum == loopsound->sourcenum)
			{
				if (loopingChannels[i]->sound == loopsound->sound)
				{
					if (audible)
					{
						//update the loopframe so we don't stop playing the sound
						loopingChannels[i]->loopframe = loopframe;			
					}
					else
					{
						if (sound_debug.integer)
						{
							Console_Printf("Stopped Looping: chan %i\n", loopingChannels[i]->index);
						}

						//stop it
						Sound_StopChannel(loopingChannels[i]->index);

						//add to the free channels list
						freeChannels[numFree++] = loopingChannels[i];
					}

					wasAlreadyPlaying = true;
				}
				else
				{
					//we want to play a different sound from the same source
					//use the same channel
					useChan = loopingChannels[i];
				}				

				break;
			}
		}

		if (wasAlreadyPlaying || !audible)
			continue;

		if (!useChan && numFree)
		{
			//use a free channel
			useChan = freeChannels[numFree-1];
			numFree--;
		}
		
		//if no free channel is available, don't play the sound

		//this differs slightly from non looping sounds, which will
		//steal a channel of equal or lesser priority if available

		if (!useChan)
			continue;

		//play the looping sound
		if (Sound_Play(loopsound->sound, loopsound->sourcenum, soundPos, loopsound->volume, useChan->index, loopsound->priority))
		{
			useChan->looping = true;
			useChan->loopframe = loopframe;
			fmodapi->FSOUND_SetLoopMode(useChan->index, FSOUND_LOOP_NORMAL);

			if (sound_debug.integer)
				Console_Printf("Looping: idx %i, source %i, chan %i\n", loopsound->sound, loopsound->sourcenum, useChan->index);
		}
	}


	//stop any looping sounds that weren't added this frame
	for (n=CHANNEL_FIRST_AUTO; n<numChannels; n++)
	{
		if (!channels[n].inUse)
			continue;

		if (channels[n].looping && channels[n].loopframe != loopframe)
			Sound_StopChannel(n);
	}
}


/*==========================

  Sound_UpdateChannels

  update the "inUse" field of all channels,
  and refresh sound source positions

 ==========================*/

void	Sound_UpdateChannels()
{
	int n;	

	for (n=0; n<numChannels; n++)
	{		
		channels[n].inUse = fmodapi->FSOUND_IsPlaying(n);

		if (!channels[n].inUse)
			continue;

		if (n == music.channel)
		{
			fmodapi->FSOUND_SetVolume(n, Sound_CalcChannelVolume(&channels[n]) * sound_musicVolume.value);
			continue;
		}

		if (channels[n].sourcenum >= 0)
		{
			fmodapi->FSOUND_3D_SetAttributes(
				n,
				sources[channels[n].sourcenum].pos,
				sources[channels[n].sourcenum].useVel ? sources[channels[n].sourcenum].vel : NULL);
		}
		else
		{
			if (!channels[n].usePos)
			{
				//move the sound with the listener
				fmodapi->FSOUND_3D_SetAttributes(
					n,
					listener.pos,
					NULL);
			}
		}

		//refresh volume - necessary because of the broken FMOD attenuation
		fmodapi->FSOUND_SetVolume(n, Sound_CalcChannelVolume(&channels[n]) * sound_sfxVolume.value);
	}
}



/*==========================

  Sound_UpdateMusic

  check status of music stream

 ==========================*/

void	Sound_UpdateMusic()
{
	if (music.stream && music.channel == -1)
	{
		if (music.queueStop)
			Sound_StopMusic();
		else
		{
			music.channel = fmodapi->FSOUND_Stream_Play(CHANNEL_MUSIC,
											  music.stream);

			if (music.channel > -1)
				fmodapi->FSOUND_SetVolume(CHANNEL_MUSIC, Sound_CalcChannelVolume(&channels[CHANNEL_MUSIC]) * sound_musicVolume.value);
		}
	}

	if (music.stream && music.channel > -1)
	{
		soundChannel_t *chan = &channels[CHANNEL_MUSIC];		

		memset(chan, 0, sizeof(*chan));

		chan->sound = 0;
		chan->sourcenum = -1;
		chan->index = CHANNEL_MUSIC;
		chan->inUse = true;
		chan->volume = sound_musicVolume.value;				
	}
}


/*==========================

  Sound_UpdateGlobals  

 ==========================*/

void	Sound_UpdateGlobals()
{
	//check changes in global settings
	if (sound_masterVolume.modified || 
		sound_dopplerFactor.modified)
	{
		Sound_SetGlobals();

		sound_masterVolume.modified = false;
		sound_dopplerFactor.modified = false;
	}
}


/*==========================

  Sound_Frame

  called once per frame to update the sound system

 ==========================*/

void	Sound_Frame()
{
	if (!sound_initialized)
		return;

	//move around sound sources, etc
	Sound_UpdateChannels();

	//process all looping sounds that were added this frame
	Sound_UpdateLoopingSounds();

	//process the music stream
	Sound_UpdateMusic();

	//update master volume, etc
	Sound_UpdateGlobals();

	//update FMOD
	fmodapi->FSOUND_Update();
}




/*==========================

  Sound_RegisterVars

 ==========================*/

void	Sound_RegisterVars()
{
	Cvar_Register(&snd_disable);
	Cvar_Register(&sound_numDrivers);
	Cvar_Register(&sound_debug);
	Cvar_Register(&sound_masterVolume);
	//Cvar_Register(&sound_distanceFactor);
	//Cvar_Register(&sound_rolloff);
	Cvar_Register(&sound_musicVolume);
	Cvar_Register(&sound_sfxVolume);
	Cvar_Register(&sound_dopplerFactor);
	Cvar_Register(&sound_defaultMinFalloff);
	Cvar_Register(&sound_defaultMaxFalloff);	
	Cvar_Register(&sound_mixrate);
	Cvar_Register(&sound_noMusic);
	Cvar_Register(&sound_numChannels);	
	Cvar_Register(&sound_noCullLooping);	
	Cvar_Register(&sound_driver);	
	Cvar_Register(&sound_softwareMode);
	//Cvar_Register(&sound_softwareMixing);
}


/*==========================

  Sound_Shutdown

 ==========================*/

void	Sound_Shutdown()
{
	if (!sound_initialized)
		return;

	Sound_StopAllSounds();
	fmodapi->FSOUND_Close();
	FMOD_FreeInstance(fmodapi);
}

//FSOUND_ALLOCCALLBACK Sound_FMODAlloc;

void *F_CALLBACKAPI Sound_FMODAlloc(unsigned int size)
{
	return Tag_Malloc(size, MEM_SOUND);
}

void F_CALLBACKAPI Sound_FMODFree(void *ptr)
{
	Tag_Free(ptr);
}

void *F_CALLBACKAPI Sound_FMODRealloc(void *ptr, unsigned int size)
{
	return Tag_Realloc(ptr, size, MEM_SOUND);
}

/*==========================

  Sound_Init

  initialize FMOD, register vars and commands

 ==========================*/

void Sound_Init()
{
	unsigned int caps = 0;
	int i;
	int driver = 0;

	if (snd_disable.integer)
		return;

	memset(sources, 0, sizeof(sources));	

	Sound_ClearChannelData();

	//load DLL

#ifdef unix
	fmodapi = FMOD_CreateInstance(fmt("%slibs/libfmod.so", System_GetRootDir()));
#else
	fmodapi = FMOD_CreateInstance(fmt("%sfmod.dll", System_GetRootDir()));
#endif
	if (!fmodapi)
	{
#ifdef unix
		Console_Printf("Error initializing fmod!  %s\n", dlerror());
#else
		Console_Printf("Error initializing fmod!\n");
#endif
		return;
	}

	if (fmodapi->FSOUND_GetVersion() < FMOD_VERSION)
	{
		Console_Printf("Wrong FMOD DLL version!\n");
		return;
	}

	// ==========================================================================================
	// SELECT DRIVER
	// ==========================================================================================

	// The following list are the drivers for the output method selected above.
	Console_Printf("---------------------------------------------------------\n");	
	Console_Printf("Using sound output driver: ");

	if (!sound_softwareMode.integer)
		fmodapi->FSOUND_SetOutput(-1);

	switch (fmodapi->FSOUND_GetOutput())
	{
	 	case FSOUND_OUTPUT_NOSOUND:	Console_Printf("NoSound\n"); break;
		case FSOUND_OUTPUT_WINMM:	Console_Printf("Windows Multimedia Waveout\n"); break;
		case FSOUND_OUTPUT_DSOUND:	Console_Printf("Direct Sound\n"); break;
		case FSOUND_OUTPUT_A3D:		Console_Printf("A3D\n"); break;
   		case FSOUND_OUTPUT_OSS:     Console_Printf("Open Sound System\n"); break;
		case FSOUND_OUTPUT_ESD:     Console_Printf("Enlightment Sound Daemon\n"); break;
		case FSOUND_OUTPUT_ALSA:    Console_Printf("Alsa\n"); break;
		 
	};

	Cvar_SetVarValue(&sound_numDrivers, fmodapi->FSOUND_GetNumDrivers());

	if (fmodapi->FSOUND_GetNumDrivers() > 1)
	{
		for (i=0; i < fmodapi->FSOUND_GetNumDrivers(); i++) 
		{
			if (stricmp(fmodapi->FSOUND_GetDriverName(i), sound_driver.string)==0)
			{
				driver = i;
			}

			Cvar_Set(fmt("sound_driver%i", i), fmodapi->FSOUND_GetDriverName(i));

			Console_Printf("%d - %s\n", i, fmodapi->FSOUND_GetDriverName(i));	// print driver names
			{
				unsigned int caps = 0;

				fmodapi->FSOUND_GetDriverCaps(i, &caps);
				
				if (caps & FSOUND_CAPS_HARDWARE)
					Console_Printf("  * Driver supports hardware 3D sound!\n");
				if (caps & FSOUND_CAPS_EAX2)
					Console_Printf("  * Driver supports EAX 2 reverb!\n");
				if (caps & FSOUND_CAPS_EAX3)
					Console_Printf("  * Driver supports EAX 3 reverb!\n");
				/*
				if (caps & FSOUND_CAPS_GEOMETRY_OCCLUSIONS)
					Console_Printf("  * Driver supports hardware 3d geometry processing with occlusions!\n");
				if (caps & FSOUND_CAPS_GEOMETRY_REFLECTIONS)
					Console_Printf("  * Driver supports hardware 3d geometry processing with reflections!\n");
				*/
			}
		}
	}

	//get driver caps
	Console_Printf("Using driver %i\n", driver);
	fmodapi->FSOUND_SetDriver(driver);					// Select sound card (0 = default)

	Cvar_SetVar(&sound_driver, fmodapi->FSOUND_GetDriverName(driver));

	fmodapi->FSOUND_GetDriverCaps(fmodapi->FSOUND_GetDriver(), &caps);
	
	Console_Printf("---------------------------------------------------------\n");	
	Console_Printf("Driver capabilities\n");
	Console_Printf("---------------------------------------------------------\n");	
	if (!caps)
		Console_Printf("- This driver will support software mode only.\n  It does not properly support 3D sound hardware.\n");
	if (caps & FSOUND_CAPS_HARDWARE)
		Console_Printf("- Driver supports hardware 3D sound!\n");
	if (caps & FSOUND_CAPS_EAX2)
		Console_Printf("- Driver supports EAX 2 reverb!\n");
	if (caps & FSOUND_CAPS_EAX3)
		Console_Printf("- Driver supports EAX 3 reverb!\n");
	/*
	if (caps & FSOUND_CAPS_GEOMETRY_OCCLUSIONS)
		Console_Printf("- Driver supports hardware 3d geometry processing with occlusions!\n");
	if (caps & FSOUND_CAPS_GEOMETRY_REFLECTIONS)
		Console_Printf("- Driver supports hardware 3d geometry processing with reflections!\n");
	*/
	Console_Printf("---------------------------------------------------------\n");	
	Console_Printf("Hardware 3D channels : %d\n", fmodapi->FSOUND_GetNumHardwareChannels());

	
	//set the minimum number of channels we want FMOD to support
	//if not all these channels are available in hardware, FMOD
	//will revert to software mixing
	fmodapi->FSOUND_SetMinHardwareChannels(sound_numChannels.integer);
	Console_Printf("Using %i total channels\n", sound_numChannels.integer);
	numChannels = sound_numChannels.integer;

	//initialize FMOD
	if (!fmodapi->FSOUND_Init(sound_mixrate.integer, sound_numChannels.integer, 0))
	{
		Console_Printf("Init: %s\n", FMOD_ErrorString(fmodapi->FSOUND_GetError()));
		return;
	}
//	fmodapi->FSOUND_Reverb_SetProperties(&props);

	
	fmodapi->FSOUND_SetMemorySystem(NULL, 0, Sound_FMODAlloc, Sound_FMODRealloc, Sound_FMODFree);

	Sound_SetListenerPosition(zero_vec, vec3(0,1,0), vec3(0,0,1), true);

	Cmd_Register("playMusic", Sound_PlayMusic_Cmd);
	Cmd_Register("stopMusic", Sound_StopMusic_Cmd);
	Cmd_Register("sndEdit", Sound_Edit_Cmd);
	Cmd_Register("sndSave", Sound_Save_Cmd);
	Cmd_Register("sndSet", Sound_Set_Cmd);
	Cmd_Register("sndLoad", Sound_Load_Cmd);

	sound_initialized = true;

	Sound_SetGlobals();

	return;
}
