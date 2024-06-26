// (C) 2003 S2 Games

// cmd.h

// Console command parsing

#define TAB_COMPLETION_LIST_TIME 0.5

void	Cmd_Init();
void	Cmd_Shutdown();
void	Cmd_ExecBuf();
void	Cmd_ClearBuf();
void	Cmd_Exec(const char *cmd);
void	Cmd_FlushScriptBuffer();
void	Cmd_Register(const char *name, cmdfunc_t cmdfunc);
void	Cmd_Enable(const char *name);
void	Cmd_Disable(const char *name);
void	Cmd_BufPrintf(const char *fmt, ...);
bool	Cmd_Find(const char *cmd);
void	Cmd_ReadConfigFileVars(const char *filename, void(*callback)(char *varname, char *string, void *userdata), void *userdata);
bool	Cmd_ReadConfigFile(const char *filename, bool changeToDir);
bool	Cmd_ReadConfigFileFromArchive(archive_t *archive, const char *filename, bool changeToDir);
bool    Cmd_ReadConfigFileAbsolute(const char *filename, bool changeToDir);
void    Cmd_ProcessString(char *str);

//command/cvar completion
void	Completion_Init();
void	Completion_Shutdown();
void	Completion_AddMatch(const char *str);
char 	*Completion_CompleteString(char *str, bool printMatches);
