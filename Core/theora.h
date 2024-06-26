/* 
 * (C) 2003 S2 Games
 * 
 * theora_unix.c
 */
	
#ifndef WIN32
	#ifndef THEORA
		#define THEORA
	#endif
#endif

void    Theora_Initialize();
void    Theora_ShutDown();
void    Theora_GetFrame(shader_t *shader);
int     Theora_Load(const char *filename, bitmap_t *bitmap, bool preload);
void    Theora_Unload(shader_t *shader);
void    Theora_Stop(shader_t *shader);
void    Theora_Restart(shader_t *shader);
void    Theora_Continue(shader_t *shader);

