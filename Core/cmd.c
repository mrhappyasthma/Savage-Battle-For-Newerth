// (C) 2003 S2 Games

// cmd.c

// Console command parsing and executing

#include "core.h"

#define	CMD_BUF_SIZE	16384
#define	CMD_MAX_LINES	1024

file_t	*SCRIPT_LOG_FILE = NULL;
int total_script_executions = 0;
int total_script_execution_time = 0;
int total_script_loads = 0;
int	total_script_load_failures = 0;
int total_script_load_time = 0;
int total_script_cache_hits = 0;
int total_script_cache_read_time = 0;
int total_script_preprocess_passes = 0;
int total_script_preprocess_time = 0;


cvar_t	cmd_showScriptDebugInfo =	{ "cmd_showScriptDebugInfo",	"0" };
cvar_t	cmd_useScriptCache =		{ "cmd_useScriptCache",			"1" };
cvar_t	cmd_traceScripts =			{ "cmd_traceScripts",			"0" };

static char	cmdbuf[CMD_BUF_SIZE];
static char	*cmdlines[CMD_MAX_LINES];
static int	cmdnumlines = 0;
static int	cmdbufpos;

char	current_cmd[CMD_MAX_LENGTH];
GCompletion *completionList = NULL;
char *completionMatch;

int		scriptRunning = 0;	//value indicates script recursion depth
bool	seekingLabel = false;
char	labelName[64] = "";

typedef struct cmdlist_s
{
	char		*name;		//console command name
	bool		enabled;
	cmdfunc_t	func;		//function associated with command

	struct		cmdlist_s	*next;
} cmdlist_t;

scriptBufferEntry_t scriptBuffer[SCRIPT_BUFFER_SIZE];

GHashTable *cmd_hashtable;

void	Cmd_Exec(const char *cmd);


/*==========================

  Cmd_List_Cmd

  Display a list of all the console commands

 ==========================*/

static int _numFound = 0;

void    _printCmd(gpointer key, gpointer value, gpointer user_data)
{
	char *cmd = key;
	char *wildcard = user_data;

	if (!wildcard || strstr(cmd, wildcard))
	{
		Console_Printf("%s\n", cmd);
		_numFound++;
	}
}

void	Cmd_List_Cmd(int argc, char *argv[])
{
	_numFound = 0;
	g_hash_table_foreach(cmd_hashtable, _printCmd, argc ? argv[0] : NULL);

	Console_Printf("\n%i matching commands found\n\n", _numFound);
}


/*==========================

  Cmd_FlushScriptBuffer

  Removes all cached scripts from memory, forcing them to be reloaded from disk

 ==========================*/

void	Cmd_FlushScriptBuffer()
{
	int index;

	Console_DPrintf("Flushing out script buffer...\n");

	for (index = 0; index < SCRIPT_BUFFER_SIZE; index++)
	{
		if (scriptBuffer[index].locked)
		{
			if (cmd_showScriptDebugInfo.integer)
				Console_DPrintf("Skipped locked script %s\n", scriptBuffer[index].scriptName);
			continue;
		}

		Tag_Free(scriptBuffer[index].buffer);
		scriptBuffer[index].buffer = NULL;
		scriptBuffer[index].lastRun = 0;
		scriptBuffer[index].length = 0;
		scriptBuffer[index].scriptName[0] = 0;
	}
}

void	Cmd_FlushScriptBuffer_Cmd(int argc, char *argv[])
{
	Cmd_FlushScriptBuffer();
}


/*==========================

  Cmd_RemoveBufferedScript

  Removes a specified script from the buffer

 ==========================*/

void	Cmd_RemoveBufferedScript(int scriptnum)
{
	if (scriptnum < 0 || scriptnum >= SCRIPT_BUFFER_SIZE)
		return;

	if (scriptBuffer[scriptnum].locked)
	{
		if (cmd_showScriptDebugInfo.integer)
			Console_DPrintf("Skipped locked script %s\n", scriptBuffer[scriptnum].scriptName);
		return;
	}

	Tag_Free(scriptBuffer[scriptnum].buffer);
	scriptBuffer[scriptnum].buffer = NULL;
	scriptBuffer[scriptnum].lastRun = 0;
	scriptBuffer[scriptnum].length = 0;
	scriptBuffer[scriptnum].scriptName[0] = 0;
}


/*==========================

  Cmd_GetBufferedScript

  Returns the index into the scriptbuffer of a file (if it is in the buffer)
  If the file is not in the buffer, returns -1

 ==========================*/

int	Cmd_GetBufferedScript(const char *filename)
{
	int index;

	for (index = 0; index < SCRIPT_BUFFER_SIZE; index++)
	{
		if (!stricmp(filename, scriptBuffer[index].scriptName))
		{
			scriptBuffer[index].lastRun = System_Milliseconds();
			if (cmd_showScriptDebugInfo.integer)
			{
				Console_DPrintf("Using buffered file \"%s\"\n", filename);
				File_Printf(SCRIPT_LOG_FILE, "Using buffered file \"%s\"\n", filename);
			}
			return index;
		}
	}

	return -1;
}

cmdlist_t	*_Cmd_Find(const char *cmd);


/*==========================

  Cmd_AddScriptToBuffer

  Finds a slot for a newly loaded script in the scriptBuffer

 ==========================*/

int	Cmd_AddScriptToBuffer(const char *filename, char *buffer, int length)
{
	int index = 0, stalest = 0, staleness = -1;
	int	now = System_Milliseconds();
	char cleanfullname[_MAX_PATH];

	File_FixPath(File_GetFullPath(filename), cleanfullname, true);
	
	//make sure we're not trying to add a duplicate
	index = Cmd_GetBufferedScript(cleanfullname);
	if (index != -1)
		return index;

	for (index = 0; index < SCRIPT_BUFFER_SIZE; index++)
	{
		//keep track of script that hasn't been run in the longest time
		if (now - scriptBuffer[index].lastRun > staleness && !scriptBuffer[index].locked)
		{
			staleness = now - scriptBuffer[index].lastRun;
			stalest = index;
		}

		if (!scriptBuffer[index].buffer)
			break;
	}

	if (index == SCRIPT_BUFFER_SIZE)
	{
		index = stalest;
		if (scriptBuffer[index].locked)
		{
			int n;
			
			for (n = 0; n < SCRIPT_BUFFER_SIZE; n++)
			{
				Console_Printf("%i: %s  last: %i length: %i locked: %i\n",
					n, scriptBuffer[n].scriptName, scriptBuffer[n].lastRun, scriptBuffer[n].length, scriptBuffer[n].locked);
			}
			Game_Error(fmt("Overwriting locked script! ([%i] %s : %i)\n", stalest, scriptBuffer[stalest].scriptName, scriptBuffer[stalest].lastRun));
		}

		Tag_Free(scriptBuffer[index].buffer);
		scriptBuffer[index].buffer = NULL;
		scriptBuffer[index].scriptName[0] = 0;
		scriptBuffer[index].lastRun = 0;
		scriptBuffer[index].length = 0;
	}

	strcpy(scriptBuffer[index].scriptName, cleanfullname);
	scriptBuffer[index].lastRun = now;
	Tag_Free(scriptBuffer[index].buffer);
	scriptBuffer[index].buffer = buffer;
	scriptBuffer[index].length = length;

	if (cmd_showScriptDebugInfo.integer)
	{
		Console_DPrintf("*** Added entry to buffer: %s\n", cleanfullname);
		File_Printf(SCRIPT_LOG_FILE, "*** Added entry to buffer: %s\n", cleanfullname);
	}

	return index;
}


/*==========================

  Cmd_GetNextLine

  Returns a pointer to the buffer holding the next line in a buffered script
  Returns NULL if at end of script

  'line' must be CMD_MAX_LENGTH in size

 ==========================*/

char	*Cmd_GetNextLine(int *at, int scriptnum, char *line)
{	
	char *l = line;

	if ((*at) >= scriptBuffer[scriptnum].length)
		return NULL;

	memset(line, 0, CMD_MAX_LENGTH);

	while (*(scriptBuffer[scriptnum].buffer + (*at)) == '\n' ||
			*(scriptBuffer[scriptnum].buffer + (*at)) == '\r')
	{
		(*at)++;
		if ((*at) >= scriptBuffer[scriptnum].length)
			return NULL;
	}

	while ((*at) < scriptBuffer[scriptnum].length)
	{
		*l = *(scriptBuffer[scriptnum].buffer + (*at));
		if (*l == '\n' || *l == '\r')
		{
			*l = 0;
			(*at)++;
			return line;
		}
		(*at)++;
		l++;
	}
	return line;
}


/*==========================

  Cmd_ReadConfigFile

  Execute a config script
  The script will be loaded from the file and placed into scriptBuffer, overwriting the script that
  has had the most time elapsed since it's last execution

 ==========================*/

bool	__Cmd_ReadConfigFile(archive_t *archive, const char *filename, bool absolutePath, bool changeToDir)
{
	char	*cmd;
	char	old_dir[1024];
	char	cleanfullname[_MAX_PATH];
	char	*scriptbuf;
	int		scriptnum;
	int		at = 0;
	int		starttime = System_Milliseconds();
	char	line[CMD_MAX_LENGTH];
	int		linenum = 1;

	//check to see if the script is in the scriptbuffer
	File_FixPath(File_GetFullPath(filename), cleanfullname, true);
	if (cmd_showScriptDebugInfo.integer)
	{
		Console_DPrintf("*** Seeking entry: %s\n", cleanfullname);
		File_Printf(SCRIPT_LOG_FILE, "*** Seeking entry: %s\n", cleanfullname);
	}
	
	scriptnum = Cmd_GetBufferedScript(cleanfullname);
	if (scriptnum == -1)
	{
		int		fsize;
		int		loadtime = System_Milliseconds();

#ifdef unix
		if (absolutePath)
			scriptbuf = File_LoadIntoBufferAbsolute(filename, &fsize, MEM_SCRIPTS);
		else
#endif
			scriptbuf = File_LoadIntoBuffer(filename, &fsize, MEM_SCRIPTS);
		if (!scriptbuf && cmd_showScriptDebugInfo.integer)
		{
			Console_DPrintf("Could not read \"%s\"\n", filename);
			File_Printf(SCRIPT_LOG_FILE, "Could not read \"%s\"\n", filename);
			return false;
		}

		scriptnum = Cmd_AddScriptToBuffer(filename, scriptbuf, fsize);
		if (cmd_showScriptDebugInfo.integer)
		{
			total_script_loads++;
			total_script_load_time += System_Milliseconds() - loadtime;
			Console_DPrintf("Loaded file \"%s\"\n", filename);
			File_Printf(SCRIPT_LOG_FILE, "Loaded file \"%s\"\n", filename);
		}
	}
	else
	{
		total_script_cache_read_time += System_Milliseconds() - starttime;
		total_script_cache_hits++;
	}

	if (changeToDir)
	{
		strncpySafe(old_dir, File_GetCurrentDir(), 1024);
		//change to the directory of the .cfg file
		File_ChangeDir(Filename_GetDir(filename));
	}

	scriptRunning++;
	scriptBuffer[scriptnum].locked = true;
	if (scriptRunning >= SCRIPT_BUFFER_SIZE)
	{
		Game_Error(fmt("Script execution stack exceeded script buffer! (%i)\n", scriptRunning));
	}

	while ((cmd = Cmd_GetNextLine(&at, scriptnum, line)))
	{
		if (cmd_traceScripts.integer)
			Console_Printf("^w[^y%s:^r%i^w] ^c%s\n", filename, linenum++, cmd);

		Cmd_Exec(cmd);

		//goto was called, look for the matching label
		if (seekingLabel)
		{
			int		p;
			char	*buf;
			bool	found = false;

			//save the current pos, in case we don't find it
			p = at;
			
			//start at the top ofthe file
			at = 0;
			while((buf = Cmd_GetNextLine(&at, scriptnum, line)))
			{
				//found a label
				if (buf[0] == '@')
				{
					//if the label matches the one we're looking for, break out
					if (stricmp(&buf[1], labelName) == 0)
					{
						found = true;
						break;
					}
				}
			}

			//couldn't find the label, reset the file possition and keep going
			if (!found)
			{
				if (cmd_showScriptDebugInfo.integer)
				{
					Console_DPrintf("Label \"%s\" not found.\n", labelName);
					File_Printf(SCRIPT_LOG_FILE, "Label \"%s\" not found.\n", labelName);
				}
				at = p;
			}

			//no longer seeking a label
			seekingLabel = false;
			labelName[0] = 0;
		}
	}

	scriptRunning--;
	scriptBuffer[scriptnum].locked = false;

	if (changeToDir)
	{
		//change back to the original dir
		File_ChangeDir(old_dir);		
	}

	if (cmd_showScriptDebugInfo.integer)
	{
		Console_DPrintf("Executed %s in %i milliseconds\n", filename, System_Milliseconds() - starttime);
		File_Printf(SCRIPT_LOG_FILE, "Executed %s in %i milliseconds\n", filename, System_Milliseconds() - starttime);
	}
	total_script_executions++;
	total_script_execution_time += System_Milliseconds() - starttime;
	
	if (!cmd_useScriptCache.integer)
		Cmd_RemoveBufferedScript(scriptnum);
	return true;
}

extern cvar_t con_developer;

bool	_Cmd_ReadConfigFile(archive_t *archive, const char *filename, bool absolutePath, bool changeToDir)
{
	bool ret;

	if (!con_developer.integer)
		Console_DisablePrinting();
	ret = __Cmd_ReadConfigFile(archive, filename, absolutePath, changeToDir);
	if (!con_developer.integer)
		Console_EnablePrinting();

	return ret;
}


bool	Cmd_ReadConfigFileFromArchive(archive_t *archive, const char *filename, bool changeToDir)
{
	bool ret;

	OVERHEAD_INIT;
	ret = _Cmd_ReadConfigFile(archive, filename, false, changeToDir);
	OVERHEAD_COUNT(OVERHEAD_SCRIPTS);

	return ret;
	
}

bool	Cmd_ReadConfigFile(const char *filename, bool changeToDir)
{
	bool ret;

	OVERHEAD_INIT;
	ret = _Cmd_ReadConfigFile(NULL, filename, false, changeToDir);
	OVERHEAD_COUNT(OVERHEAD_SCRIPTS);

	return ret;
}

bool	Cmd_ReadConfigFileAbsolute(const char *filename, bool changeToDir)
{
	bool ret;

	OVERHEAD_INIT;
	ret = _Cmd_ReadConfigFile(NULL, filename, true, changeToDir);
	OVERHEAD_COUNT(OVERHEAD_SCRIPTS);

	return ret;
}

void	Cmd_ReadConfigFile_Cmd(int argc, char *argv[])
{
	if (!argc)
	{
		Console_Printf("syntax: exec <filename>\n");
		return;
	}

	Cmd_ReadConfigFile(argv[0], true);
}

void	Cmd_ReadConfigFileVars(const char *filename, void(*callback)(char *varname, char *string, void *userdata), void *userdata)
{
	file_t *f;
	char cmd[CMD_MAX_LENGTH];

	if (!callback)
		return;

	if ((f = File_Open(filename, "r")) != NULL)
	{		
		while(File_gets(cmd, CMD_MAX_LENGTH, f)!=NULL)		
		{
			if (memcmp(cmd, "set ", 4)==0)
			{
				const char *var;
				const char *str;
				char *s;

				var = GetNextWord(cmd);
				str = GetNextWord(var);
				
				s = (char *)var;

				while (*s != ' ' && *s)
					s++;

				*s = 0;

				callback(StripEOL((char *)var), StripEOL((char *)str), userdata);
			}
		}
		File_Close(f);
	}
}

char	*Cmd_GetObjectValueString(char *objname)
{
	cvar_t *cvar;
	gui_element_t *elem;

	cvar = Cvar_Find(objname);

	if (cvar)
	{
		return cvar->string;
	}

	elem = GUI_GetObject(objname);

	if (elem)
	{
		if (elem->getvalue)
		{
			return elem->getvalue(elem);
		}
	}

	return "(UNDEFINED)";
}

float	Cmd_GetObjectValue(char *objname)
{
	cvar_t *cvar;
	gui_element_t *elem;

	cvar = Cvar_Find(objname);

	if (cvar)
	{
		return cvar->value;
	}

	elem = GUI_GetObject(objname);

	if (elem)
	{
		if (elem->getvalue)
		{
			return atof(elem->getvalue(elem));
		}
	}

	return 0;
}


void	Cmd_Do_Cmd(int argc, char *argv[])
{
	if (!argc)
	{
		Console_Printf("syntax: do <variablename>");
		return;
	}

	Cmd_Exec(Cmd_GetObjectValueString(argv[0]));
}

extern cvar_t	svr_allowCheats;

void	Cmd_For_Cmd(int argc, char *argv[])
{
	cvar_t	*forvar;
	int start, stop, step, index;

	if (argc < 5)
	{
		Console_Printf("Syntax: for <cvar name> <start value> <stop value> <increment> <command>\n");
		return;
	}

	//get the cvar to use
	forvar = Cvar_Find(argv[0]);
	if (!forvar)
	{
		Console_Printf("Couldn't find cvar named \"%s\"\n", argv[0]);
		return;
	}

	if (!svr_allowCheats.integer && (forvar->flags & CVAR_CHEAT))
	{
		Console_Printf("Can not alter cheat protected cvar: %s\n", argv[0]);
		return;
	}

	//get the loop parameters and validate them
	start = atoi(argv[1]);
	stop = atoi(argv[2]);
	step = atoi(argv[3]);

	if ((start > stop && step > 0) ||
		(start < stop && step < 0) ||
		step == 0 ||
		start == stop)
	{
		Console_Printf("Invalid start/stop/step combination\n");
		return;
	}

	//do the loop
	if (step > 0)
	{
		for (index = start; index <= stop; index += step)
		{
			Cvar_SetVarValue(forvar, index);
			Cmd_Exec(argv[4]);
		}
	}
	else
	{
		for (index = start; index >= stop; index += step)
		{
			Cvar_SetVarValue(forvar, index);
			Cmd_Exec(argv[4]);
		}
	}
}

void	Cmd_Goto_Cmd(int argc, char *argv[])
{
	//must specify a label
	if (!argc)
		return;

	//only works while in a script
	if (scriptRunning < 1)
		return;

	seekingLabel = true;
	strcpy(labelName, argv[0]);
}

void	Cmd_CmdInfo_Cmd(int agrc, char *argv[])
{
	Console_Printf("Total commands: %i\n", g_hash_table_size(cmd_hashtable));
}

void	Cmd_Crash_Cmd(int argc, char *argv[])
{
	int *p = 0;

	*p = 1;	//crash!!!
}

void	Cmd_Help_Cmd(int argc, char *argv[])
{
	Console_Printf("^y" GAME_VERSION "\n\n");
	Console_Printf("For technical support, visit support.s2games.com\n");
	Console_Printf("For modding support, visit mods.s2games.com or forums.s2games.com\n\n");
	Console_Printf("For a list of commands, type \"cmdlist\"\n");
	Console_Printf("For a list of variables, type \"cvarlist\"\n");
	Console_Printf("To load a map, type \"world mapname\"\n");
	Console_Printf("Type \"quit\" to exit %s\n", GAME_VERSION);
	
}

void	Cmd_Init()
{
	cmd_hashtable = g_hash_table_new(g_str_hash, g_str_equal);

	memset(cmdbuf, 0, CMD_BUF_SIZE);
	cmdbufpos = 0;

	memset(scriptBuffer, 0, sizeof(scriptBufferEntry_t) * SCRIPT_BUFFER_SIZE);
	SCRIPT_LOG_FILE = File_Open("scripts.log", "w");
	if (!SCRIPT_LOG_FILE)
		Game_Error("Couldn't open scripts.log\n");

	Cmd_Register("exec",				Cmd_ReadConfigFile_Cmd);
	Cmd_Register("cmdlist",				Cmd_List_Cmd);
	Cmd_Register("cmdinfo",				Cmd_CmdInfo_Cmd);
	Cmd_Register("do",					Cmd_Do_Cmd);
	Cmd_Register("for",					Cmd_For_Cmd);
	Cmd_Register("goto",				Cmd_Goto_Cmd);
	Cmd_Register("flushScriptBuffer",	Cmd_FlushScriptBuffer_Cmd);
	Cmd_Register("help",				Cmd_Help_Cmd);

	Cmd_Register("crash",	Cmd_Crash_Cmd);
}

void	Completion_Init()
{
	if (completionList)
		g_completion_free(completionList);
	completionList = g_completion_new(NULL);
	
	//fixme:(maybe?) this works, but i think using the function is the 'correct' way to do it
	//		the commented line causes a linker error for some reason though...
	completionList->strncmp_func = strnicmp;
	//g_completion_set_compare(completionList, strnicmp);
}

void	Completion_Shutdown()
{
	if (completionList)
		g_completion_free(completionList);
}

void	Completion_AddMatch(const char *str)
{
	GList *list = NULL;
	//list = g_list_append(list, g_strdup(str));
	list = g_list_append(list, (char *)str);
	g_completion_add_items(completionList, list);
}

char *Completion_CompleteString(char *str, bool printMatches)
{
	int i = 0;
	char *match;
	GList *list;
	if (completionMatch)
		g_free(completionMatch);
	list = g_completion_complete(completionList, str, &completionMatch);
	if (printMatches)
	{
		Console_Printf("Matches:\n");
		while ((match = g_list_nth_data(list, i)))
		{
			cvar_t	*cv = Cvar_Find(match);

			if (cv)
				Console_Printf("  %s = \"%s\"\n", match, cv->string);
			else
				Console_Printf("  %s\n", match);

			i++;
		}
	}
	return completionMatch;
}

cmdlist_t	*_Cmd_Find(const char *cmd)
{
	gpointer tableentry;
	char *temp;
	
	if (!cmd)
		return NULL;

	temp = g_ascii_strdown(cmd, strlen(cmd));
	tableentry = g_hash_table_lookup(cmd_hashtable, temp);
	g_free(temp);

	if (tableentry);
	{
		return (cmdlist_t *)tableentry;
	}
	return NULL;
}

//from eval.c
float evaluate(char *equation, int *error, float (*variable)(char *name));

//execute a single command (parsed out by Cmd_Exec)
void	Cmd_DoCommand(int argc, char *argv[])
{
	cvar_t *var;
	cmdlist_t *cmd;
	int n;

	if (!argc)
		return;

	//skip labels (labels must be at the start of a line)
	if (argv[0][0] == '@')
		return;

	//weed out comments
	for (n=0; n<argc; n++)
	{
		if (strncmp(argv[n], "//", 2)==0)
		{
			argc = n;
			break;
		}
	}

	//if it was a pure comment line, we're done
	if (!argc)
		return;

	for (n=argc; n<CMD_MAX_ARGS; n++)
	{
		argv[n] = "";
	}


	//otherwise execute as normal

	cmd = _Cmd_Find(argv[0]);
	if (cmd)
	{
		if (cmd->func && cmd->enabled)
			cmd->func(argc-1, &argv[1]);

		return;
	}

	//first check the first argument to see if it is a variable name
	//if so, assume we are trying to set the variable

	var = Cvar_Find(argv[0]);
	if (var)
	{
		Cvar_Set_Cmd(argc, argv);
		return;
	}

	Console_Printf("Unknown command: '%s'\n", argv[0]);
}


//execute one line of script
void	Cmd_Exec(const char *cmd)
{
	static char cmdbuffer[CMD_MAX_LENGTH];
	static bool concatinating = false;
	char cmdfull[CMD_MAX_LENGTH];

	char *c;
	char outcmd[CMD_MAX_LENGTH];
	char *out;
	bool grouped = false;
	bool escaped = false;
	char *cmd_argv[CMD_MAX_ARGS];
	int argcount = 1;

	if (!cmd)
		return;
	
	if (!concatinating)
		cmdbuffer[0] = 0;

	//check for a concatination
	if (cmd[strlen(cmd) - 1] == '~')
	{
		strncat(cmdbuffer, cmd, CMD_MAX_LENGTH);
		concatinating = true;
		*(cmdbuffer + strlen(cmdbuffer) - 1) = 0;
		return;
	}
	else
	{
		concatinating = false;
	}

	strcpy(cmdfull, cmdbuffer);
	strcat(cmdfull, cmd);

	//skip leading spaces
	c = SkipSpaces((char *)cmdfull);

	if (!*c)			//no command
		return;

	if (strlen(c) > CMD_MAX_LENGTH)
	{
		Console_Printf("Command too long: %.15s.....\n", c);
		return;
	}

	out = outcmd;
	cmd_argv[0] = out;

	//set blah 5; do bleh blih bloh "erferf; afafaf; agaga"; blah
	while(*c && out < &outcmd[CMD_MAX_LENGTH])
	{
		if (!escaped)
		{
			switch(*c)
			{

				case '"':				//argument grouper
					grouped = !grouped;
					break;
				case '\\':				//escape character
					escaped = true;
					break;
				case ';':				//command separator
					if (!grouped)
					{
						//perform command
						*out = 0;
						Cmd_DoCommand(argcount, cmd_argv);
						argcount = 1;
						out = outcmd;	//clear output for next command						
						cmd_argv[0] = out;
						c = SkipSpaces(c+1) - 1;	//- 1 because of the c++ below
					}
					else
					{
						*out = *c;
						out++;
					}
					break;
				case ' ':				//argument separator
				case '\t':
				case '\n':
					if (!grouped)
					{
						*out = 0;
						out++;
						c = SkipSpaces(c);
						if (*c)
						{
							//start a new argument
							cmd_argv[argcount] = out;
							argcount++;
							if (argcount >= CMD_MAX_ARGS)
							{
								Console_Printf("%s: too many arguments!\n", cmd);
								return;
							}
						}
						c--;	//to compensate for the c++ below
					}
					else
					{
						*out = *c;
						out++;
					}
					break;
				case '#':				//value insertion command
				case '$':				//value insertion command, evaluate immediately (ignore quotes)
					if (!grouped || *c == '$')
					{
						char *nextPound = strchr(c+1, *c);

						if (nextPound)
						{
							char objname[256];
							int namelen = (nextPound - (c+1));
							char *valuestring;

							Mem_Copy(objname, c+1, namelen > 255 ? 255 : namelen);
							objname[namelen] = 0;	//null terminate the string

							valuestring = Cmd_GetObjectValueString(objname);
							strncpy(out, valuestring, &outcmd[CMD_MAX_LENGTH] - out - 1);

							c = nextPound;	//skip over the object name since we have parsed it
							out += strlen(valuestring);
						}
						else
						{
							*out = *c;
							out++;
						}
					}
					else
					{
						*out = *c;
						out++;
					}
					break;
				case '[':				//expression
				case '{':				//expression, evaluate immediately (ignore quotes)
				{
					if (!grouped || *c == '{')
					{
						char *closeBracket = strchr(c+1, *c == '[' ? ']' : '}');

						if (closeBracket)
						{
							int error;
							float value;
							char *valuestring;
							static char expr[1024];
							int exprlen = (closeBracket - (c+1));						
							Mem_Copy(expr, c+1, exprlen > 1023 ? 1023 : exprlen);
							expr[exprlen] = 0;		//null terminate the string

							value = evaluate(expr, &error, Cmd_GetObjectValue);
							
							if (error)
							{
								valuestring = "(EXPR_ERROR)";								
							}
							else
							{
								if (value - floor(value) < EPSILON)
								{
									valuestring = fmt("%.0f", value);									
								}
								else
								{
									valuestring = fmt("%f", value);									
								}
							}
							strncpySafe(out, valuestring, &outcmd[CMD_MAX_LENGTH] - out);

							c = closeBracket;
							out += MIN((int)strlen(valuestring), &outcmd[CMD_MAX_LENGTH] - out);
						}
						else
						{
							*out = *c;
							out++;
						}
					}
					else
					{
						*out = *c;
						out++;
					}
					break;
				}
				default:
					*out = *c;
					out++;
					break;
			}
		}
		else
		{
			//we've ignored the previous \ character
			*out = *c;
			out++;
			escaped = false;
		}

		c++;
	}

	*out = 0;

	//any remaining commands

	if (*cmd_argv[0])
	{
		Cmd_DoCommand(argcount, cmd_argv);
	}
}

#define MAX_CMD_LINES 1024

void	Cmd_ClearBuf()
{
	cmdnumlines = 0;
	cmdbufpos = 0;
}

void	Cmd_ExecBuf()
{	
	int n;
	int oldnumlines;

	OVERHEAD_INIT;

	oldnumlines = cmdnumlines;

	for (n=0; n<cmdnumlines; n++)
	{
		//NOTE: cmdnumlines may get increased during this loop if console commands do Cmd_BufPrintf()
		Cmd_Exec(cmdlines[n]);
	}

	Cmd_ClearBuf();

	OVERHEAD_COUNT(OVERHEAD_CMD_EXECBUF);
}

void	Cmd_Register(const char *name, cmdfunc_t cmdfunc)
{
	const char *temp;
	char *lowercase;
	cmdlist_t	*newcmd;

	if (!cmdfunc) return;

/*	if (Cvar_Find(name))
	{
		Console_Errorf("Cmd_Register: Can't register %s (it's a variable)\n", name);
		return;
	}*/

	if (Cmd_Find(name))
	{
	//	Console_Errorf("Cmd_Register: Command %s already exists\n", name);
		return;
	}

	newcmd = Tag_Malloc(sizeof(cmdlist_t), MEM_CMD);

	temp = name;
	newcmd->name = Tag_Malloc(strlen(name)+1, MEM_CMD);
	strcpy(newcmd->name, temp);
	newcmd->func = cmdfunc;
	newcmd->enabled = true;

	//add command to the list
	lowercase = g_ascii_strdown(name, strlen(newcmd->name));
	g_hash_table_insert(cmd_hashtable, lowercase, newcmd);
	//don't free it, it needs it in the hashtable
	
	//cmdlist = newcmd;

	Completion_AddMatch(newcmd->name);
}

void	Cmd_Disable(const char *name)
{
	cmdlist_t *cmd = _Cmd_Find(name);
	if (!cmd)
		return;

	cmd->enabled = false;
}

void	Cmd_Enable(const char *name)
{
	cmdlist_t *cmd = _Cmd_Find(name);
	if (!cmd)
		return;

	cmd->enabled = true;
}

void	Cmd_BufPrintf(const char *fmt, ...)
{
	va_list argptr;
	char s[CMD_MAX_LENGTH];
	int len;

	va_start(argptr, fmt);
	vsprintf(s, fmt, argptr);
	va_end(argptr);

	len = strlen(s)+1;

	if (len+cmdbufpos > CMD_BUF_SIZE)
	{
		Game_Error("Cmd_BufPrintf: Command buffer overflow (bufsize)\n");
		return;
	}
	if (cmdnumlines >= MAX_CMD_LINES)
	{
		Game_Error("Cmd_BufPrintf: Command buffer overflow (numlines)\n");
	}

	Mem_Copy(&cmdbuf[cmdbufpos], s, len);
	cmdlines[cmdnumlines++] = &cmdbuf[cmdbufpos];

	cmdbufpos+=len;
}

bool	Cmd_Find(const char *cmd)
{
	cmdlist_t *func;
	
	if (!cmd)
		return false;

	func = _Cmd_Find(cmd);

	return func != NULL;
}

void	Cmd_ProcessString(char *str)
{
	bool esc = false;
	char *c = str;
	
	while (c && *c)
	{
		if (esc)
		{
			switch (*c)
			{
			  	case 'n':
					*c = '\n';
					break;
				default:
					break;
			}
			//for esc to be true, we must always be at least one char in, so doing [-1] can't fail
			memmove(&c[-1], c, strlen(c));
			c[strlen(c)-1] = '\0';

			esc = false;
		}
		else
		{
			if (*c == '\\')
				esc = true;
		}
		c++;
	}
}
/*
#define MAX_SUBNAME_LENGTH	128

static char current_subname[MAX_SUBNAME_LENGTH] = "";
static char current_params[

void	Cmd_StartSub_Cmd(int argc, char *argv[])
{
	if (argc < 2)
	{
		Console_Printf("syntax: cmdblock <name> [param1] [param2] ... [paramN]\n");
		return;
	}

	//make sure the sub name is valid
	if (Cmd_Find(argv[0]) || Cvar_Find(argv[0])
	{
		Console_Printf("Error creating sub: there is already a variable or command called that\n");
		
		return;
	}
}*/
