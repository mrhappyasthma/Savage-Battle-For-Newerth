/*
 * (C) 2003 S2 Games
 *
 * buddies.c - a buddy list implement
 */

#include "core.h"

char *buddy_list[MAX_BUDDY_LIST_USERS];

extern cvar_t homedir;

void    Buddies_Load(char *filename)
{
	int i = 0;

	while (i < MAX_BUDDY_LIST_USERS && buddy_list[i])
	{
		Tag_Free(buddy_list[i]);
		buddy_list[i] = NULL;
		i++;
	}

#ifdef unix
	Cmd_ReadConfigFileAbsolute(filename, false);
#else
	Cmd_ReadConfigFile(filename, false);
#endif
}

void    Buddies_Save(char *filename)
{
	file_t *file;
	int i;

	Console_DPrintf("buddies_save\n");

#ifdef unix
	file = File_OpenAbsolute(filename, "w");
#else
	file = File_Open(filename, "w");
#endif
	if (!file)
	{
		Console_DPrintf("Error trying to open %s to write to\n", filename);
		return;
	}

	i = 0;
	while (i < MAX_BUDDY_LIST_USERS && buddy_list[i])
	{
		File_Printf(file, "buddy %s\n", buddy_list[i]);
		i++;
	}
	
	File_Close(file);
}

bool	Buddies_Remove(char *name)
{
	int i = 0;

	while (i < MAX_BUDDY_LIST_USERS && buddy_list[i])
	{
		if (strcmp(name, buddy_list[i]) == 0)
		{
			Tag_Free(buddy_list[i]);
			Mem_Move(&buddy_list[i], &buddy_list[i+1], MAX_BUDDY_LIST_USERS-i-1 * sizeof(char *));
			return true;
		}
		i++;
	}
	return false;
}

void	Buddies_Remove_cmd(int argc, char *argv[])
{
	int i;
	for (i = 0; i < argc; i++)
		Buddies_Remove(argv[i]);
}

void	Buddies_Add(char *name)
{
	int i = 0;
	
	if (name[0] != '_')
		name++;
	
	while (i < MAX_BUDDY_LIST_USERS && buddy_list[i])
	{
		if (strcmp(name, buddy_list[i]) == 0)
			return;
		i++;
	}

	if (i < MAX_BUDDY_LIST_USERS)
		buddy_list[i] = Tag_Strdup(name, MEM_BUDDIES);
}

void	Buddies_Add_cmd(int argc, char *argv[])
{
	int i;
	for (i = 0; i < argc; i++)
		Buddies_Add(argv[i]);
}

void 	Buddies_Init()
{
	memset(buddy_list, 0, MAX_BUDDY_LIST_USERS * sizeof( char *));
	Cmd_Register("buddy", Buddies_Add_cmd);
	Cmd_Register("buddyremove", Buddies_Remove_cmd);
#ifdef unix
	Buddies_Load(fmt("%sbuddies.cfg", homedir.string));
#else
	Buddies_Load("/buddies.cfg");
#endif
}

void 	Buddies_Shutdown()
{
#ifdef unix
	Buddies_Save(fmt("%sbuddies.cfg", homedir.string));
#else
	Buddies_Save("/buddies.cfg");
#endif
}
