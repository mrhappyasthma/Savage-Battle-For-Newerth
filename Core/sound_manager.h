// (C) 2001 S2 Games

// sound_manager.h


// interface
void	SC_PlayObjectSound(int id, residx_t sound, float volume, int channel, bool loop);
void	SC_StopObjectChannel(int id, int channel);
void	SC_ResetObjectPosition(int id, vec3_t pos);
void	SC_MoveObject(int id, vec3_t pos);

// core
void	SC_Init();
