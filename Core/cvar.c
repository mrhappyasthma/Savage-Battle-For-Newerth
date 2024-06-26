// (C) 2003 S2 Games

// cvar.c

// Console variables


#include "core.h"
#include "zip.h"

static char	*null_string = "";

GHashTable *cvar_hashtable = NULL;
cvar_t		cvar_netList;
cvar_t		cvar_transmitList;
cvar_t		cvar_serverInfoList;
bool		cvar_netSettingsModified = false;
bool		cvar_transmitModified = false;
bool		cvar_serverInfoModified = false;

cvar_t		cvar_list;

static bool		_lastIfResult = true;	//used for else command

cvar_t	svr_allowCheats =	{ "svr_allowCheats", "0", CVAR_READONLY | CVAR_TRANSMIT | CVAR_SERVERINFO };

extern bool	host_developer_mode;

static int	_numFound = 0;


#if 0

typedef struct latch_s
{
	char *name;
	char *string;
	struct latch_s *next;
	struct latch_s *prev;
} latch_t;

latch_t		latch_list;




/*==========================

  Latch_Find

 ==========================*/

latch_t	*Latch_Find(const char *name)
{
	latch_t *list;

	for (list = latch_list.next; list != &latch_list; list=list->next)
	{
		if (stricmp(name, list->name)==0)
			return list;
	}

	return NULL;
}


/*==========================

  Latch_Set

  store the value of a "set" command before a variable is registered

 ==========================*/

void	Latch_Set(const char *name, const char *string)
{
	latch_t *latch;

	latch = Latch_Find(name);
	if (!latch)
	{
		latch = Tag_Malloc(sizeof(latch_t), MEM_CVAR);

		latch->name = Tag_Strdup(name, MEM_CVAR);
		latch->string = Tag_Strdup(string, MEM_CVAR);

		LIST_INSERT(&latch_list, latch);
	}

	Tag_Free(latch->string);
	latch->string = Tag_Strdup(string, MEM_CVAR);
}


/*==========================

  Latch_Remove

 ==========================*/

void	Latch_Remove(latch_t *latch)
{
	LIST_REMOVE(latch);
	Tag_Free(latch->string);
	Tag_Free(latch->name);
}


/*==========================

  Latch_List_Cmd

 ==========================*/

void	Latch_List_Cmd(int argc, char *argv[])
{
	latch_t *list;

	for (list = latch_list.next; list != &latch_list; list=list->next)
	{
		Console_Printf("%s is latched to \"%s\"\n", list->name, list->string);
	}
}

#endif


void	Cvar_AllowCheats()
{
	Cvar_SetVarValue(&svr_allowCheats, 1);
}

void	Cvar_BlockCheats()
{
	if (!host_developer_mode && DLLTYPE != DLLTYPE_EDITOR)
		Cvar_SetVarValue(&svr_allowCheats, 0);
}

void	Cvar_Print(cvar_t *cvar)
{
	char flags[32];
	char *s = flags;

	memset(flags, ' ', sizeof(flags));

	if (cvar->flags & CVAR_SAVECONFIG)
	{
		memcpy(s, "^cS", 3);
		s+=3;
	}
	else
		s++;

	if (cvar->flags & CVAR_READONLY)
	{
		memcpy(s, "^rR", 3);
		s+=3;
	}
	else
		s++;

	if (cvar->flags & CVAR_USER_DEFINED)
	{
		memcpy(s, "^wU", 3);		
		s+=3;
	}
	else
		s++;

	if (cvar->flags & CVAR_WORLDCONFIG)
	{
		memcpy(s, "^gW", 3);
		s+=3;
	}
	else
		s++;	

	if (cvar->flags & CVAR_CHEAT)
	{
		memcpy(s, "^yC", 3);		
		s+=3;
	}
	else
		s++;

	if (cvar->flags & CVAR_TRANSMIT)
	{
		memcpy(s, "^mT", 3);		
		s+=3;
	}
	else
		s++;	

	if (cvar->flags & CVAR_SERVERINFO)
	{
		memcpy(s, "^mV", 3);		
		s+=3;
	}
	else
		s++;

	if (cvar->flags & CVAR_NETSETTING)
	{
		memcpy(s, "^bN", 3);
		s+=3;
	}
	else
		s++;	

	*s = 0;

	Console_Printf("^w[%s^w]  %s \"%s\"\n", flags, cvar->name, cvar->string);
}

void	_findCvar(gpointer key, gpointer value, gpointer user_data)
{
	cvar_t *cvar = value;
	char *str = user_data;	
	char *s;
	char lowstr[256];
	char lowname[256];
	int n;

	s = str;
	n = 0;
	while (*s && n<1023)
	{
		lowstr[n] = tolower(*s);
		s++;
		n++;
	}
	lowstr[n] = 0;

	s = cvar->name;
	n = 0;
	while (*s && n<1023)
	{
		lowname[n] = tolower(*s);
		s++;
		n++;
	}
	lowname[n] = 0;

	if (strstr(lowname, lowstr))
	{
		Cvar_Print(cvar);		
		_numFound++;
	}
}

void	Cvar_List_Cmd(int argc, char *argv[])
{
	bool search;	

	if (argc)
		search = true;
	else
		search = false;

	if (search)
	{
		_numFound = 0;
		g_hash_table_foreach(cvar_hashtable, _findCvar, argv[0]);
		Console_Printf("\n%i matching variables found\n\n", _numFound);
	}
	else
	{
		cvar_t *list;
		int num = 0;

		for (list = cvar_list.next; list != &cvar_list; list = list->next)
 		{
			Cvar_Print(list);
			num++;
		}

		Console_Printf("\n%i matching variables found\n\n", num);
	}

	Console_Printf("Flags:\n"
		           "^cS        ^wSaved to startup.cfg\n"
				   "^rR        ^wRead only\n"
				   "^wU        ^wUser defined\n"
				   "^gW        ^wWorld config setting\n"
				   "^yC        ^wCheat protected - enable with \"devworld\" command\n"
				   "^bT        ^wServer dictated setting\n"
				   "^bV        ^wServer info\n"
				   "^bN        ^wLocal client network setting\n"				   
				   );
}


typedef struct
{
	file_t *f;
	char *wildcard;
	int flags;
} cvarSaveData_t;

void	_writeCvar(gpointer key, gpointer value, gpointer user_data)
{
	cvar_t *cvar = value;
	cvarSaveData_t *data = user_data;
	
	if (cvar->flags & CVAR_DONTSAVE)
		return;
	
	if (data->wildcard[0])
	{
		if (strncmp(cvar->name, data->wildcard, strlen(data->wildcard)) == 0)
		{
			File_Printf(data->f, "set %s %s\n", cvar->name, cvar->string);
		}
	}
	else
	{
		if (cvar->flags & data->flags)
		{
			if (cvar->flags & CVAR_SAVECONFIG)
			{
				File_Printf(data->f, "setsave %s %s\n", cvar->name, cvar->string);
			}
			else
			{
				File_Printf(data->f, "set %s %s\n", cvar->name, cvar->string);
			}
		}
	}
}

void	_zipWriteCvar(gpointer key, gpointer value, gpointer user_data)
{
	char line[256];
	cvar_t *cvar = value;
	cvarSaveData_t *data = user_data;
	
	if (cvar->flags & CVAR_DONTSAVE)
		return;
	
	if (data->wildcard[0])
	{
		if (strncmp(cvar->name, data->wildcard, strlen(data->wildcard)) == 0)
		{
			BPrintf(line, 256, "set %s %s\n", cvar->name, cvar->string);
			ZIPW_WriteFileInZip((zipFile)data->f, line, strlen(line));
		}
	}
	else
	{
		if (cvar->flags & data->flags)
		{
			if (cvar->flags & CVAR_SAVECONFIG)
			{
				BPrintf(line, 256, "setsave %s %s\n", cvar->name, cvar->string);
			}
			else
			{
				BPrintf(line, 256, "set %s %s\n", cvar->name, cvar->string);
			}

			ZIPW_WriteFileInZip((zipFile)data->f, line, strlen(line));
		}
	}
}

bool	Cvar_SaveConfigFileToZip(void *zipfile, char *filename, int wildcardCount, char **wildcards, int flags, bool writeBinds)
{
	cvarSaveData_t saveData;
	int method = Z_DEFLATED;
	int level = Z_DEFAULT_COMPRESSION;
	int i;
	
	if ((ZIPW_AddFileInZip((zipFile)zipfile, filename, NULL, NULL, 0, NULL, 0, NULL, method, level)) == ZIP_OK)
	{
		if (!wildcardCount)
		{
			saveData.f = (file_t *)zipfile;
			saveData.wildcard = "";
			saveData.flags = flags;

			//write variables
			g_hash_table_foreach(cvar_hashtable, _zipWriteCvar, &saveData);
		}
		else
		{
			for (i = 0; i < wildcardCount; i++)
			{
				saveData.f = (file_t *)zipfile;
				saveData.wildcard = wildcards[i];
				saveData.flags = flags;
	
				//write variables
				g_hash_table_foreach(cvar_hashtable, _zipWriteCvar, &saveData);
			}
		}

		//write key bindings
		if (writeBinds)
		{
			Input_WriteBindingsToZip(zipfile);
		}
		ZIPW_CloseFileInZip(zipfile);
		return true;
	}
	else
		return false;
}

void	Cvar_WriteConfigToFile(file_t *f, int wildcardCount, char **wildcards, int flags, bool writeBinds)
{
	cvarSaveData_t saveData;

	if (!wildcardCount)
	{
		saveData.f = f;
		saveData.wildcard = "";
		saveData.flags = flags;

		//write variables
		g_hash_table_foreach(cvar_hashtable, _writeCvar, &saveData);
	}
	else
	{
		int i;

		for (i = 0; i < wildcardCount; i++)
		{
			saveData.f = f;
			saveData.wildcard = wildcards[i];
			saveData.flags = flags;

			//write variables
			g_hash_table_foreach(cvar_hashtable, _writeCvar, &saveData);
		}
	}

	//write key bindings
	if (writeBinds)
	{
		Input_WriteBindings(f);
	}
}

bool	Cvar_SaveConfigFile(char *filename, int wildcardCount, char **wildcards, int flags, bool writeBinds)
{
	file_t *f;	

	f = File_Open(filename, "w");
	if (!f)
		return false;

	Cvar_WriteConfigToFile(f, wildcardCount, wildcards, flags, writeBinds);
	
	File_Close(f);
	
	return true;	
}

void	Cvar_SaveConfigFile_Cmd(int argc, char *argv[])
{
	char **wildcards;

	if (!argc)
	{
		Console_Printf("syntax: writeconfig <filename> [wildcard]\n");
		return;
	}

	if (argc > 1)
		wildcards = &argv[1];
	else
		wildcards = NULL;

	if (!Cvar_SaveConfigFile(argv[0], argc-1, wildcards, CVAR_SAVECONFIG|CVAR_WORLDCONFIG, (wildcards ? false : true)))
	{
		Console_Printf("Bad filename\n", argv[0]);
	}
	else
	{
		Console_Printf("Wrote %s\n", argv[0]);
	}
}

cvar_t *Cvar_New(const char *name)
{
	cvar_t *new_cvar;

	new_cvar = Tag_Malloc(sizeof(cvar_t), MEM_CVAR);
	memset(new_cvar, 0, sizeof(cvar_t));
	new_cvar->name = Tag_Strdup(name, MEM_CVAR);
	new_cvar->string = "";

	new_cvar->flags = CVAR_USER_DEFINED;
	
	Cvar_Register(new_cvar);

	return new_cvar;
}

void	Cvar_CreateVar_Cmd(int argc, char *argv[])
{
	cvar_t *newvar, *oldvar;

	if (!argc)
	{
		Console_Printf("syntax: createvar <variable> [initial value]\n");
		return;
	}

	oldvar = Cvar_Find(argv[0]);
	if (oldvar)
	{
		if (oldvar->flags & CVAR_READONLY)
		{
			Console_Printf("%s is already registered and is write protected.\n", argv[0]);
			return;
		}

		if (!svr_allowCheats.integer && (oldvar->flags & CVAR_CHEAT))
		{
			Console_Printf("%s is already registered and is cheat protected.\n", argv[0]);
			return;
		}

		//just set the value
		if (argv[1][0])
			Cvar_Set(argv[0], argv[1]);
		//else
		//	Cvar_Set(argv[0], "");
		return;
	}

	if (Cmd_Find(argv[0]))
	{
		Console_Printf("%s is a command\n", argv[0]);
		return;
	}

	newvar = Cvar_New(argv[0]);
	if (argc > 1)
		Cvar_SetVar(newvar, argv[1]);
}

void	Cvar_Set_Cmd(int argc, char *argv[])
{
	cvar_t *var;

	if (!argc)
	{
		Console_Printf("syntax: set <variable> <value>\n");
	}
	
	if (argc==1)
	{
		if ((var = Cvar_Find(argv[0])))
		{
			Console_Printf("%s is \"%s\"\n", argv[0], var->string);
		}
		else
		{
			Console_Printf("\"%s\" not found\n", argv[0]);
		}
	}
	else
	{
		int n;

		char str[1024] = "";
		for (n=1; n<argc; n++)
		{
			strcat(str, argv[n]);
			strcat(str, " ");
		}
		if (strlen(str)>0)
			str[strlen(str)-1] = '\0';

		if ((var = Cvar_Find(argv[0])))
		{
			if (var->flags & CVAR_READONLY || (var->flags & CVAR_TRANSMIT && !localServer.active))
			{
				Console_Printf("%s is a read-only value\n", argv[0]);
				return;
			}		

			if (!svr_allowCheats.integer && (var->flags & CVAR_CHEAT))
			{
				Console_Printf("Can not alter cheat protected cvar: %s\n", argv[0]);
				return;
			}			
		}

		Cvar_Set(argv[0], str);		
	}
}

void	Cvar_SetSave_Cmd(int argc, char *argv[])
{
	cvar_t *var;

	Cvar_Set_Cmd(argc, argv);

	var = Cvar_Find(argv[0]);
	if (var)
		var->flags |= CVAR_SAVECONFIG;
}

void	Cvar_Clear_Cmd(int argc, char *argv[])
{
	cvar_t *var;

	if (!argc)
	{
		Console_Printf("syntax: clear <variable>\n");
		return;
	}

	if (!(var = Cvar_Find(argv[0])))
		Console_Printf("\"%s\" not found\n", argv[0]);
	
	if (!svr_allowCheats.integer && (var->flags & CVAR_CHEAT))
	{
		Console_Printf("Can not alter cheat protected cvar: %s\n", argv[0]);
		return;
	}
	
	if (var->flags & CVAR_READONLY)
	{
		Console_Printf("%s is a read-only value\n", argv[0]);
		return;
	}

	Cvar_Set(argv[0], "");
}

//increment variable command
void	Cvar_Inc_Cmd(int argc, char *argv[])
{
	cvar_t *var;
	float increment;

	if (!argc)
	{
		Console_Printf("syntax: inc <variable> <value>\n");
	}
	
	var = Cvar_Find(argv[0]);
	if (var)
	{
		if (var->flags & CVAR_READONLY)
		{
			Console_Printf("%s is a read-only value\n", argv[0]);
			return;
		}

		if (!svr_allowCheats.integer && (var->flags & CVAR_CHEAT))
		{
			Console_Printf("Can not alter cheat protected cvar: %s\n", argv[0]);
			return;
		}
	
		increment = (argc > 1 ? atof(argv[1]) : 1);
		Cvar_SetValue(argv[0], var->value + increment);			
	}
	else
		Console_Printf("\"%s\" not found\n", argv[0]);
}

void	Cvar_Toggle_Cmd(int argc, char *argv[])
{
	cvar_t *var;

	if (!argc)
	{
		Console_Printf("syntax: toggle <variable>\n");
	}
	else
	{
		var = Cvar_Find(argv[0]);
		if (var)
		{
			if (var->flags & CVAR_READONLY)
			{
				Console_Printf("%s is a read-only value\n");
				return;
			}

			if (!svr_allowCheats.integer && (var->flags & CVAR_CHEAT))
			{
				Console_Printf("Can not alter cheat protected cvar\n");
				return;
			}
	
			if (var->value)
				Cvar_SetValue(argv[0], 0);
			else
				Cvar_SetValue(argv[0], 1);
		}
		else
			Console_Printf("\"%s\" not found\n", argv[0]);
	}	
}	


/*==========================

  Cvar_If_Cmd

 ==========================*/

void	Cvar_If_Cmd(int argc, char *argv[])
{
	char cmd[CMD_MAX_LENGTH] = "";
	cvar_t *var;
	float value;	

	if (argc < 2)
	{
		Console_Printf("syntax: if <value OR variable> <command>\n");
		return;
	}

	var = Cvar_Find(argv[0]);
	if (!var)
		value = atof(argv[0]);
	else
		value = var->value;

	if (value)
	{
		ConcatArgs(&argv[1], argc - 1, cmd);
		Cmd_Exec(cmd);
		_lastIfResult = true;
	}
	else
	{
		_lastIfResult = false;
	}
}


/*==========================

  Cvar_Else_Cmd

 ==========================*/

void	Cvar_Else_Cmd(int argc, char *argv[])
{
	char cmd[CMD_MAX_LENGTH] = "";

	if (argc < 1)
	{
		Console_Printf("syntax: else <command>\n");
		return;
	}

	if (!_lastIfResult)
	{
		ConcatArgs(argv, argc, cmd);
		Cmd_Exec(cmd);
	}
}


/*==========================

  Cvar_Set

 ==========================*/

void	Cvar_Set(const char *name, const char *string)
{
	cvar_t *var;
	var = Cvar_Find(name);

	if (!var)
	{
		//if the variable isn't found, create it.  this allows us to latch values before vars are actually registered
		var = Cvar_New(name);		
		
		//Console_Errorf("Cvar_Set: variable %s not found\n", name);
		//return;	
	}

	Cvar_SetVar(var, string);
}


void	Cvar_SetVar(cvar_t *var, const char *string)
{	
	bool valchanged;
	
	if (!string)
		string = "";

	valchanged = (strcmp(string, var->string) == 0) ? false : true;

	var->modified = true;
	var->modifiedCount++;

	if (!valchanged)
	{		
		return;
	}
	
	if (var->flags & CVAR_PATH_TO_FILE)
	{
		//if the file exists, set the cvar to the full path to the file
		if (File_Exists(string))
		{
			string = File_GetFullPath(string);
		}		
	}

	if (valchanged)
	{
		if (var->flags & CVAR_NETSETTING)
		{
			cvar_netSettingsModified = true;
		}
		if (var->flags & CVAR_TRANSMIT)
		{
			cvar_transmitModified = true;
		}
		if (var->flags & CVAR_SERVERINFO)
		{
			cvar_serverInfoModified = true;
		}
	}
	
	if (strlen(string) > strlen(var->string))
	{
		var->string = Tag_Realloc(var->string, strlen(string)+1, MEM_CVAR);		
	}

	strcpy(var->string, string);
	var->value = atof(var->string);

	var->integer = (int)var->value;

	if ((var->flags & CVAR_TERRAIN_FX) && var->modified)
	{
		Vid_Notify(VID_NOTIFY_TERRAIN_COLOR_MODIFIED, 0, 0, 1);
	}
	if (var->flags & CVAR_VALUERANGE)
	{
		if (var->value > var->hirange)
			Cvar_SetVarValue(var, var->hirange);
		else if (var->value < var->lorange)
			Cvar_SetVarValue(var, var->lorange);
	}
}

void	Cvar_SetVarValue(cvar_t *var, float value)
{
	char str[32];

	if (!var)
		return;
/*
	if (ABS(var->value - value) < 0.000001)
	{
		var->modified = false;
		return;
	}
*/
	if (ABS(value - (int)value) < 0.000001)
		BPrintf(str, 31, "%.0f", value);		//don't display decimals if we're close to an integer
	else
		BPrintf(str, 31, "%f", value);
	Cvar_SetVar(var, str);
}

char	*Cvar_GetString(const char *name)
{
	cvar_t	*var;

	var = Cvar_Find(name);
	if (!var) return null_string;  //works better than returning NULL, so we don't have to worry about a null pointer

	return var->string;
}

char	*Cvar_GetDefaultString(const char *name)
{
	cvar_t	*var;

	var = Cvar_Find(name);
	if (!var) return null_string;

	return var->default_string;
}

float	Cvar_GetValue(const char *name)
{
	cvar_t	*var = Cvar_Find(name);

	if (!var) 
		return 0;
	
	return var->value;
}

int		Cvar_GetInteger(const char *name)
{
	cvar_t *var = Cvar_Find(name);

	if (!var)
		return 0;

	return var->integer;
}

int		Cvar_GetModifiedCount(const char *name)
{
	cvar_t *var = Cvar_Find(name);

	if (!var)
		return 0;

	return var->modifiedCount;
}

void	Cvar_SetValue (const char *name, float value)
{
	cvar_t *var = Cvar_Find(name);
	if (!var)
		return;

	Cvar_SetVarValue(var, value);
}

cvar_t	*Cvar_Find(const char *name)
{
	gpointer tableentry;

	if (!name || !cvar_hashtable)
		return NULL;
	
	tableentry = g_hash_table_lookup(cvar_hashtable, name);

	return tableentry;
}

void	Cvar_ResetVar(cvar_t *var)
{
	Cvar_SetVar(var, var->default_string);
}

void	_resetVar(gpointer key, gpointer value, gpointer user_data)
{
	cvar_t *cvar = value;
	int flag = GPOINTER_TO_UINT(user_data);

	if (cvar->flags & flag)
		Cvar_ResetVar(cvar);
}

//set all cvars with the given flag to their default value
void	Cvar_ResetVars(int flag)
{
	g_hash_table_foreach(cvar_hashtable, _resetVar, GUINT_TO_POINTER(flag));
}



/*==========================

  Cvar_Remove

  removes the cvar's linkage

 ==========================*/

void	Cvar_Remove(cvar_t *var)
{	
	g_hash_table_remove(cvar_hashtable, var->name);

	var->prev->next = var->next;
	var->next->prev = var->prev;
	var->next = var->prev = NULL;
	
	if (var->flags & CVAR_NETSETTING)
	{
		var->prevNetSetting->nextNetSetting = var->nextNetSetting;
		var->nextNetSetting->prevNetSetting = var->prevNetSetting;				
		var->nextNetSetting = var->prevNetSetting = NULL;
	}

	if (var->flags & CVAR_TRANSMIT)
	{
		var->prevTransmit->nextTransmit = var->nextTransmit;
		var->nextTransmit->prevTransmit = var->prevTransmit;
		var->nextTransmit = var->prevTransmit = NULL;
	}

	if (var->flags & CVAR_SERVERINFO)
	{
		var->prevServerInfo->nextServerInfo = var->nextServerInfo;
		var->nextServerInfo->prevServerInfo = var->prevServerInfo;
		var->nextServerInfo = var->prevServerInfo = NULL;
	}

	Tag_Free(var->name);
	Tag_Free(var->string);

	if (var->flags & CVAR_USER_DEFINED)
	{
		//free the original pointer
		Tag_Free(var);
	}
}


/*==========================

  Cvar_Register

  Registers a pre-allocated variable with the engine

 ==========================*/

void	Cvar_Register(cvar_t *var)
{
	cvar_t *existing;
	char *latchedString = NULL;
	bool replacement = false;

	if (!cvar_hashtable)
	{
		Game_Error("Cvar_Register: Cannot register cvars (%s) before hash is initialized!\n", var->name);
		return;
	}

	if (!var)
		return;	

	if (Cmd_Find(var->name))
	{
		Console_Errorf("Cvar_Register: Can't register %s (it's a command)\n", var->name);
		return;
	}

	if (existing = Cvar_Find(var->name))
	{
		//if the existing variable is user created, replace it with the "proper" pointer

		if (existing == var)
			return;
		if (!(existing->flags & CVAR_USER_DEFINED))
			return;

		if (!(var->flags & CVAR_READONLY) && !(var->flags & CVAR_CHEAT))
			latchedString = Tag_Strdup(existing->string, MEM_CVAR);
		
		Cvar_Remove(existing);		

		replacement = true;
	}

	//copy values in "var" to dynamic memory

	//copy name	
	var->name = Tag_Strdup(var->name, MEM_CVAR);	

	//copy string
	if (latchedString)
	{		
		//copy over the value from the user defined variable to the new one
		//this allows us to latch values before cvars are registered
		var->string = latchedString;
	}
	else
	{
		var->string = Tag_Strdup(var->string, MEM_CVAR);		
	}
	
	var->default_string = Tag_Strdup(var->string, MEM_CVAR);	

	//do all the value representation
	var->value = atof(var->string);
	var->integer = (int)var->value;
	var->default_value = var->value;

	var->nextNetSetting = var->prevNetSetting = NULL;
	var->nextTransmit = var->prevTransmit = NULL;
	var->nextServerInfo = var->prevServerInfo = NULL;

	if (var->flags & CVAR_NETSETTING)
	{
		//add to the client net setting list
		var->nextNetSetting = &cvar_netList;
		var->prevNetSetting = cvar_netList.prevNetSetting;
		var->nextNetSetting->prevNetSetting = var;
		var->prevNetSetting->nextNetSetting = var;		
	}
	if (var->flags & CVAR_TRANSMIT)
	{
		//add to the transmit (server dictated var) list
		var->nextTransmit = &cvar_transmitList;
		var->prevTransmit = cvar_transmitList.prevTransmit;
		var->nextTransmit->prevTransmit = var;
		var->prevTransmit->nextTransmit = var;	
	}
	if (var->flags & CVAR_SERVERINFO)
	{
		//add to the serverinfo list
		var->nextServerInfo = &cvar_serverInfoList;
		var->prevServerInfo = cvar_serverInfoList.prevServerInfo;
		var->nextServerInfo->prevServerInfo = var;
		var->prevServerInfo->nextServerInfo = var;
	}	

	//add variable to the system
	g_hash_table_insert(cvar_hashtable, var->name, var);
	LIST_APPEND(&cvar_list, var);
	Completion_AddMatch(var->name);

	var->modified = true;
	var->modifiedCount = 1;
}

void	Cvar_CvarInfo_Cmd(int argc, char *argv[])
{
	Console_Printf("Total cvars: %i\n", g_hash_table_size(cvar_hashtable));
}


/*==========================

  Cvar_GetServerInfoVars

  build a string containing all CVAR_SERVERINFO cvars

 ==========================*/

bool	Cvar_GetServerInfo(char *buf, int size)
{
	cvar_t *list;

	buf[0] = 0;

	for (list = cvar_serverInfoList.nextServerInfo; list != &cvar_serverInfoList; list = list->nextServerInfo)
	{
		if (!ST_SetState(buf, list->name, list->string, size))
		{
			return false;				
		}			
	}

	return true;
}


/*==========================

  Cvar_GetNetSettings

  build a string containing all CVAR_NETSETTING cvars

 ==========================*/

bool	Cvar_GetNetSettings(char *buf, int size)
{
	cvar_t *list;

	buf[0] = 0;

	for (list = cvar_netList.nextNetSetting; list != &cvar_netList; list = list->nextNetSetting)
	{
		if (!ST_SetState(buf, list->name, list->string, size))
		{
			return false;				
		}			
	}

	return true;
}

/*==========================

  Cvar_GetTransmitCvars

  build a string containing all CVAR_TRANSMIT cvars  

 ==========================*/

bool	Cvar_GetTransmitCvars(char *buf, int size)
{
	cvar_t *list;

	buf[0] = 0;

	for (list = cvar_transmitList.nextTransmit; list != &cvar_transmitList; list = list->nextTransmit)
	{
		if (!ST_SetState(buf, list->name, list->string, size))
		{
			return false;
		}
	}

	return true;
}


/*==========================

  Cvar_SetLocalTransmitVars

  go through all cvars with the CVAR_TRANSMIT flag and set them based on the stateString

 ==========================*/

void	Cvar_SetLocalTransmitVars(const char *stateString)
{
	cvar_t *list;
	
	if (localServer.active)
		return;				//setting these vars locally is redundant

	for (list = cvar_transmitList.nextTransmit; list != &cvar_transmitList; list = list->nextTransmit)
	{
		if (ST_FindState(stateString, list->name))
		{
			Cvar_SetVar(list, ST_GetState(stateString, list->name));
		}
		else
		{
			//hmm, should we really error out here?
			//this will happen if the client code has defined a CVAR_TRANSMIT var that the server doesn't have
			Console_Printf("Client has an invalid CVAR_TRANSMIT cvar");
		}
	}
}

extern cvar_t cmd_showScriptDebugInfo;
extern cvar_t cmd_useScriptCache;
extern cvar_t cmd_traceScripts;

gboolean Cvar_Str_Equal(gconstpointer v, gconstpointer v2)
{
	char *s1 = (char *)v;
	char *s2 = (char *)v2;

	if (stricmp(s1, s2)==0)
		return true;

	return false;
}


guint Cvar_Hash (gconstpointer key)
{
  const char *p = key;
  guint h = *p;

  if (h)
    for (p += 1; *p != '\0'; p++)
      h = (h << 5) - h + (tolower(*p));

  return h;
}


/*==========================

  Cvar_Init

 ==========================*/

void	Cvar_Init()
{	
	//init linked lists
	cvar_list.next = &cvar_list;
	cvar_list.prev = &cvar_list;
	cvar_netList.nextNetSetting = &cvar_netList;
	cvar_netList.prevNetSetting = &cvar_netList;
	cvar_transmitList.prevTransmit = &cvar_transmitList;
	cvar_transmitList.nextTransmit = &cvar_transmitList;
	cvar_serverInfoList.nextServerInfo = &cvar_serverInfoList;
	cvar_serverInfoList.prevServerInfo = &cvar_serverInfoList;

	//init hash table
	cvar_hashtable = g_hash_table_new(Cvar_Hash, Cvar_Str_Equal);	

	Cvar_Register(&svr_allowCheats);
	Cvar_Register(&cmd_showScriptDebugInfo);
	Cvar_Register(&cmd_useScriptCache);
	Cvar_Register(&cmd_traceScripts);

	Cmd_Register("set", Cvar_Set_Cmd);
	Cmd_Register("setsave", Cvar_SetSave_Cmd);
	Cmd_Register("cvarlist", Cvar_List_Cmd);
	Cmd_Register("inc", Cvar_Inc_Cmd);
	Cmd_Register("writeconfig", Cvar_SaveConfigFile_Cmd);
	Cmd_Register("toggle", Cvar_Toggle_Cmd);
	Cmd_Register("if", Cvar_If_Cmd);
	Cmd_Register("else", Cvar_Else_Cmd);
	Cmd_Register("createvar", Cvar_CreateVar_Cmd);
	Cmd_Register("clear", Cvar_Clear_Cmd);
	Cmd_Register("cvarinfo", Cvar_CvarInfo_Cmd);
}
