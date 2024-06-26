// (C) 2003 S2 Games

// system.h


void    System_ChangeRootDir(const char *newdir);
void	System_Error(char* msg, ...);
double	System_GetPerfCounter();
int		System_Milliseconds();
unsigned int System_GetTicks();
int		System_Milliseconds();
void	System_Printf(char* msg, ...);
void	System_ShutDown();
void	System_Quit();
//handle system events such as keypresses
void	System_ProcessEvents();
void	System_Dir(char *directory, char *wildcard, bool recurse,
					void(*dirCallback)(const char *dir, void *userdata),
					void(*fileCallback)(const char *filename, void *userdata),
					void *userdata);
char	*System_GetRootDir();
bool	System_CreateDir(const char *dirname);
int		System_GetArgc();
char	*System_GetArgv(int arg);
void	System_InitGameDLL();
void 	System_UnloadGameDLL();
char 	*System_GetCommandLine();
bool    System_Say(const char *string);
void	System_Sleep(unsigned int ms);
