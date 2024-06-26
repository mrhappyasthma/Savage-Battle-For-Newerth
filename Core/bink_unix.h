/* 
 * (C) 2003 S2 Games
 * 
 * bink_unix.c
 */


void    Bink_Initialize();
void    Bink_ShutDown();
void    Bink_GetFrame(shader_t *shader);
int     Bink_Load(const char *filename, bitmap_t *bitmap, bool preload);
void    Bink_Unload(shader_t *shader);
void    Bink_Stop(shader_t *shader);
void    Bink_Restart(shader_t *shader);
void    Bink_Continue(shader_t *shader);
