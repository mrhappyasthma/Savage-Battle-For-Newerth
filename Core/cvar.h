// (C) 2003 S2 Games

// cvar.h

void	Cvar_Init();
void	Cvar_Register(cvar_t *var);
void	Cvar_AllowCheats();
void	Cvar_BlockCheats();
void	Cvar_Set(const char *name, const char *value);
void	Cvar_SetVar(cvar_t *var, const char *string);
void	Cvar_SetVarValue(cvar_t *var, float value);
void	Cvar_SetValue(const char *name, float value);
float	Cvar_GetValue(const char *name);
int		Cvar_GetInteger(const char *name);
char	*Cvar_GetString(const char *name);
int		Cvar_GetModifiedCount(const char *name);
cvar_t	*Cvar_Find(const char *name);
bool	Cvar_SaveConfigFile(char *filename, int wildcardCount, char **wildcards, int flags, bool writeBinds);
void	Cvar_WriteConfigToFile(file_t *file, int wildcardCount, char **wildcards, int flags, bool writeBinds);
bool    Cvar_SaveConfigFileToZip(void *zipfile, char *filename, int wildcardCount, char **wildcards, int flags, bool writeBinds);
void	Cvar_Set_Cmd(int argc, char *argv[]);
void	Cvar_ResetVar(cvar_t *var);
void	Cvar_ResetVars(int flag);
bool	Cvar_GetTransmitCvars(char *buf, int size);
void	Cvar_SetLocalTransmitVars(const char *stateString);
bool	Cvar_GetNetSettings(char *buf, int size);
bool	Cvar_GetServerInfo(char *buf, int size);

extern bool		cvar_netSettingsModified;
extern bool		cvar_transmitModified;
extern bool		cvar_serverInfoModified;
