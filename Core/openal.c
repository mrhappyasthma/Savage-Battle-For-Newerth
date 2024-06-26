/*********************************************************************

  Blatently ripped from audioenv.cpp from the Creative OpenAL Sample code
   PROGRAM: audioenv.cpp

   DESCRIPTION: audio object class code

****************************************************************************/

#include "core.h"

cvar_t	al_debug = { "al_debug", "0" };

// EAX-test
// load up eax for Windows and MacOS, and add initguid.h for Windows for good measure...
#define OPENAL

#include "AL/alut.h"
#include "AL/al.h"

#ifndef unix

#ifdef _WIN32
#include <initguid.h>
#define alGetSourceiv alGetSourcei
#else //non-linux, non-win32
#define INITGUID
#endif

#ifdef USE_EAX
#include "eax.h"
#endif
#endif

void change_eax (int argc, char *argv[]);
void test_openal (int argc, char *argv[]);

cvar_t openal_rolloff = { "openal_rolloff", "1" };

vec3_t zeros;

// init
int OpenAL_Init ()
{
#ifdef USE_EAX
	EAXSet pfPropSet;
	unsigned long ulEAXVal;
	long lGlobalReverb;
#endif

	SET_VEC3(zeros,0,0,0);
	Cmd_Register("eax", change_eax);
	Cvar_Register(&openal_rolloff);
	
	//fixme: openal seg faults if there is no sound support
	alutInit (NULL, 0); // init OpenAL

#ifdef USE_EAX
	// EAX test -- set EAX environment if EAX is available
	if (alIsExtensionPresent((ALubyte *)"EAX2.0") == AL_TRUE)
	{
		pfPropSet = (EAXSet) alGetProcAddress((ALubyte *)"EAXSet");
		if (pfPropSet != NULL)
		{
		    lGlobalReverb = 0.2;
			pfPropSet(&DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_ROOM, 0, &lGlobalReverb, sizeof(unsigned long));
			ulEAXVal = EAX_ENVIRONMENT_MOUNTAINS;
			pfPropSet(&DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_ENVIRONMENT, 0, &ulEAXVal, sizeof(unsigned long));
		}
	}
#endif
	alListenerf(AL_GAIN, 1.5);
	alDistanceModel(AL_INVERSE_DISTANCE);

	//alDopplerFactor(10);
	
	Cvar_Register(&al_debug);

	return true;
}

void OpenAL_Shutdown()
{
	alutExit();
}

void	OpenAL_PrintError(char *pre, char *file, int line)
{
  	ALuint error = alGetError();

	if (!al_debug.integer)
		return;

  	switch (error)
	{
		case AL_NO_ERROR:
						break;
		case AL_ILLEGAL_ENUM:
						Console_DPrintf(pre, file, line);
						Console_DPrintf(" (Invalid Enum)\n");
	  					break;
		case AL_INVALID_NAME:
						Console_DPrintf(pre, file, line);
						Console_DPrintf(" (Invalid Name)\n");
	  					break;
		case AL_INVALID_VALUE:
						Console_DPrintf(pre, file, line);
						Console_DPrintf(" (Invalid Value)\n");
	  					break;
		default:
						Console_DPrintf(pre, file, line);
						Console_DPrintf(" error %d\n", error);
	  					break;
	}
}

sound_id_t OpenAL_NewBuffer (void)
{
	ALuint buffer;

	// create buffer
	alGetError(); /* clear */
	alGenBuffers(1, &buffer);
	if(alGetError() != AL_NO_ERROR) 
	{
		return 0;
	}

	return buffer;
}

bool	OpenAL_Buffer(sound_handle_t source, char *data, int size, sound_information_t *sound_info)
{
	ALenum format;
	ALint	buffer;
	ALint	buffers[1];

	format=(sound_info->channels==1?
						(sound_info->bits_per_sample==8?AL_FORMAT_MONO8:AL_FORMAT_MONO16):
						(sound_info->bits_per_sample==8?AL_FORMAT_STEREO8:AL_FORMAT_STEREO16));

	buffer = OpenAL_NewBuffer();
	alBufferData (buffer, format, data, size, sound_info->sample_rate);
	OpenAL_PrintError("alBufferData error : (%s line %d): ", __FILE__, __LINE__);

	if (alIsBuffer(buffer))
	{
		buffers[0] = buffer;
		alSourceQueueBuffers (source, 1, buffers);
		OpenAL_PrintError("alSourceQueueBuffers error : (%s line %d): ", __FILE__, __LINE__);
	}

	if (sound_info->channels > 1)
	{
		alSourcefv(source, AL_POSITION, zeros);
		OpenAL_PrintError("alSourcefv error (%s line %d): ", __FILE__, __LINE__);
		alSourcei(source, AL_SOURCE_RELATIVE, AL_TRUE);
		OpenAL_PrintError("alSourcei error (%s line %d): ", __FILE__, __LINE__);
	}
		
	return true;
}

bool	OpenAL_DataNeeded(sound_handle_t handle)
{
	ALint buffers_queued;
	
	if (!alIsSource(handle))
	{
		Console_DPrintf("DataNeeded called on a source that doesn't exist!  handle is %i\n", handle);
		return false;
	}

	// Retrieve number of queued buffers
	alGetSourceiv(handle, AL_BUFFERS_QUEUED, &buffers_queued);
	OpenAL_PrintError("alGetSourceiv error (%s line %d): ", __FILE__, __LINE__);

	//the following number determines how many buffers to queue for each source
	if (buffers_queued <= 15)
	{
		return true;
	}
	return false;
}

bool	OpenAL_EndSource(sound_handle_t handle)
{
	ALint state;
	ALint buffersremoved[1];
	
	if (!alIsSource(handle))
	{
		Console_DPrintf("We tried to delete a source that doesn't exist!  Source %i\n", handle);
		return true;
	}
	// Retrieve number of queued buffers
	alGetSourceiv(handle, AL_SOURCE_STATE, &state);

	//if 0, assume we're finally done with the source
	if (state == AL_STOPPED)
	{
		buffersremoved[0] = handle;
		alDeleteSources(1, buffersremoved);
		OpenAL_PrintError("alDeleteSources error (%s line %d): ", __FILE__, __LINE__);
		//return false, this is no longer a valid source
		return true;
	}

	//source is still valid
	return false;
}

bool	OpenAL_FreeBuffers(sound_handle_t handle)
{
	ALint buffers_processed;
	ALint buffersremoved[5];

	if (!alIsSource(handle))
	{
		Console_DPrintf("We tried to FreeBuffers on a source that doesn't exist!  Source %i\n", handle);
		return false;
	}

	// Retrieve number of queued buffers
	alGetSourceiv(handle, AL_BUFFERS_PROCESSED, &buffers_processed);
	OpenAL_PrintError("alGetSourceiv error (%s line %d): ", __FILE__, __LINE__);

	if (buffers_processed > 0)	
	{
		// if there are any that are done, unqueue them
		alSourceUnqueueBuffers(handle, MIN(5, buffers_processed), buffersremoved);
		OpenAL_PrintError("alSourceUnqueueBuffers error (%s line %d): ", __FILE__, __LINE__);
		alDeleteBuffers(MIN(5, buffers_processed), buffersremoved);
		OpenAL_PrintError("alDeleteBuffers error (%s line %d): ", __FILE__, __LINE__);
	}

	return true;
}

// init
void OpenAL_SetListenerPosition (vec3_t position, vec3_t forward, vec3_t up)
{
	float orientation[6] = { forward[0], forward[1], forward[2], up[0], up[1], up[2] };
	alListenerfv(AL_POSITION, position);
	OpenAL_PrintError("alListenerfv error (%s line %d): ", __FILE__, __LINE__);
	alListenerfv(AL_ORIENTATION, orientation);
	OpenAL_PrintError("alListenerfv error (%s line %d): ", __FILE__, __LINE__);
}

//SetSoundPosition
bool OpenAL_SetSoundPosition (sound_handle_t handle, vec3_t position)
{
	if (alIsSource(handle))
	{
		alSourcefv(handle, AL_POSITION, position);
		OpenAL_PrintError("alSourcefv error (%s line %d): ", __FILE__, __LINE__);
		return true;
	} else {
		OpenAL_PrintError(fmt("OpenAL_SetSoundPosition error: handle %d is not recognized as a handle!\n", handle) ,__FILE__, __LINE__);
		return false;
	}
}

//SetSoundVolume
bool OpenAL_SetSoundVolume (sound_handle_t handle, float volume)
{
	if (alIsSource(handle))
	{
		alSourcef(handle, AL_MAX_GAIN, volume);
		OpenAL_PrintError("alSourcef error (%s line %d): ", __FILE__, __LINE__);
		alSourcef(handle, AL_GAIN, volume);
		OpenAL_PrintError("alSourcef error (%s line %d): ", __FILE__, __LINE__);
		alSourcef(handle, AL_MAX_DISTANCE, openal_rolloff.value * 10);
		OpenAL_PrintError("alSourcef error (%s line %d): ", __FILE__, __LINE__);
		alSourcef(handle, AL_ROLLOFF_FACTOR, openal_rolloff.value);
		OpenAL_PrintError("alSourcef error (%s line %d): ", __FILE__, __LINE__);
		return true;
	} else {
		OpenAL_PrintError(fmt("OpenAL_SetSoundVolume error: handle %d is not recognized as a handle!\n", handle), __FILE__, __LINE__);
		return false;
	}
}

//SetSoundVelocity
bool OpenAL_SetSoundVelocity (sound_handle_t handle, vec3_t velocity)
{
	if (alIsSource(handle))
	{
		alSourcefv(handle, AL_VELOCITY, velocity);
		OpenAL_PrintError("alSourcefv error (%s line %d): ", __FILE__, __LINE__);
		return true;
	} else {
		OpenAL_PrintError(fmt("OpenAL_SetSoundVelocity error: handle %d is not recognized as a handle!\n", handle), __FILE__, __LINE__);
		return false;
	}
}

bool	OpenAL_Handle_IsActive(sound_handle_t handle)
{
	int queued;
	int processed;

	alGetSourceiv(handle, AL_BUFFERS_QUEUED, &queued);
	OpenAL_PrintError("alGetSourceiv error (%s line %d): ", __FILE__, __LINE__);
	alGetSourceiv(handle, AL_BUFFERS_PROCESSED, &processed);
	OpenAL_PrintError("alGetSourceiv error (%s line %d): ", __FILE__, __LINE__);

	return (queued == processed);
}

bool	OpenAL_IsHandle(sound_handle_t handle)
{
	if (alIsSource(handle))
		return true;
	return false;
}

//Create a new sound source
sound_handle_t OpenAL_CreateSource()
{
	ALuint source = 0;

	// create source
	alGetError(); /* clear */
	alGenSources(1, &source);
	OpenAL_PrintError("alGenSources error (%s line %d): ", __FILE__, __LINE__);
	//alSourcef(source, AL_REFERENCE_DISTANCE, 0.7);
	//OpenAL_PrintError("alSourcef error (%s line %d): ", __FILE__, __LINE__);
	return source;
}

// Play
void OpenAL_Play(const sound_handle_t handle)
{
	alSourcei(handle, AL_LOOPING, AL_FALSE);
	OpenAL_PrintError("alSourcei error (%s line %d): ", __FILE__, __LINE__);
	alSourcePlay(handle);
	OpenAL_PrintError("alSourcePlay error (%s line %d): ", __FILE__, __LINE__);
}

// PlayLooping
void OpenAL_PlayLooped(const sound_handle_t handle)
{
	alSourcei(handle, AL_LOOPING, AL_TRUE);
	OpenAL_PrintError("alSourcei error (%s line %d): ", __FILE__, __LINE__);
	alSourcePlay(handle);
	OpenAL_PrintError("alSourcePlay error (%s line %d): ", __FILE__, __LINE__);
}


// Stop
bool OpenAL_Stop(sound_handle_t handle)
{
	if (alIsSource(handle))
	{
		alSourceStop(handle);
		OpenAL_PrintError("alSourceStop error (%s line %d): ", __FILE__, __LINE__);
		if (al_debug.integer)
			Console_DPrintf("Deleting handle %i\n", handle);
		alDeleteSources(1, &handle);
		OpenAL_PrintError("alDeleteSources error (%s line %d): ", __FILE__, __LINE__);
		return true;
	} else {
		OpenAL_PrintError(fmt("OpenAL_Stop error: handle %d is not recognized as a handle!\n", handle), __FILE__, __LINE__);
	}
	return false;
}

// IncrementEnv
int OpenAL_IncrementEnv()
{
#ifdef USE_EAX
	// increment EAX environment if EAX is available
	EAXSet pfPropSet;
	unsigned long ulEAXVal;
	static unsigned long ulEAXEnv = 0;
	if (alIsExtensionPresent((ALubyte *)"EAX2.0") == AL_TRUE)
	{
		pfPropSet = (EAXSet) alGetProcAddress((ALubyte *)"EAXSet");
		if (pfPropSet != NULL)
		{
		    ulEAXVal = 65535;
			pfPropSet(&DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_ROOM, 0, &ulEAXVal, sizeof(unsigned long));
			ulEAXEnv += 1;
			if (ulEAXEnv >= EAX_ENVIRONMENT_COUNT) { ulEAXEnv = EAX_ENVIRONMENT_GENERIC; }
			pfPropSet(&DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_ENVIRONMENT, 0, &ulEAXEnv, sizeof(unsigned long));
		}
	}
	return (int) ulEAXEnv;
#else
    return 0;
#endif
}

void change_eax (int argc, char *argv[])
{
	OpenAL_IncrementEnv();
}

