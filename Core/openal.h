/**
* OpenAL functions
*****/

int OpenAL_Init (void);
void OpenAL_Shutdown(void);

sound_id_t OpenAL_NewBuffer (void);
bool	OpenAL_Buffer(sound_handle_t source, char *data, int size, sound_information_t *sound_info);
bool	OpenAL_DataNeeded(sound_handle_t handle);
bool	OpenAL_FreeBuffers(sound_handle_t handle);
bool	OpenAL_EndSource(sound_handle_t handle);

sound_handle_t OpenAL_CreateSource();
void OpenAL_Play (const sound_handle_t handle);
void OpenAL_PlayLooped(const sound_handle_t handle);
bool OpenAL_Stop(const sound_handle_t handle);

bool OpenAL_Handle_IsActive(sound_handle_t handle);

bool OpenAL_SetSoundPosition (sound_handle_t handle, vec3_t position);
bool OpenAL_SetSoundVelocity (sound_handle_t handle, vec3_t velocity);
bool OpenAL_SetSoundVolume (sound_handle_t handle, float volume);

void OpenAL_SetListenerPosition (vec3_t position, vec3_t forward, vec3_t up);

int OpenAL_IncrementEnv();
