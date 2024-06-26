// (C) 2003 S2 Games

// sound.h

#define SOUND_2D 		0x000001
#define SOUND_3D 		0x000002
#define SOUND_MONO 		0x000004
#define SOUND_STEREO 	0x000008

//this holds the data for the loaded sounds (whether they are playing or not)
typedef struct
{
	struct media_s		*media;

	//bool				in_use;
	//bool				stream;
	
	float				falloff_min;
	float				falloff_max;
	bool				falloff_linear;
	float				volume;

	bool				looping;

	void				*data;			//data needed by fmod

	//bool				forced2d;
} sample_t;


// init
void			Sound_RegisterVars();
void 			Sound_Init();
void 			Sound_Shutdown();
	
void			Sound_Frame();

bool 			Sound_LoadSample (const char *filename, sample_t *sample);
unsigned int	Sound_Play(residx_t sampleidx, int sourcenum, const vec3_t pos, float volume, int channel, int priority);
void			Sound_ClearLoopingSounds();
void			Sound_AddLoopingSound(residx_t sampleidx, int sourcenum, int priority);
void			Sound_AddLoopingSoundEx(residx_t sampleidx, int sourcenum, float volume, int priority);
void 			Sound_StopSource(int sourcenum);
void			Sound_StopHandle(unsigned int handle);
void			Sound_StopChannel(int channel);
void			Sound_MoveSource(int sourcenum, vec3_t pos, vec3_t velocity);

void 			Sound_SetListenerPosition (const vec3_t pos, const vec3_t forward, const vec3_t up, bool warp);

void			Sound_PlayMusic(const char *streamFile);
void			Sound_StopMusic();

void			Sound_StopAllSounds();