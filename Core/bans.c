/*
 * (C) 2003 S2 Games
 *
 * bans.c - a ban list implementation
 */

#include "core.h"
#include "bans.h"

typedef struct 
{
	char *ban;
	bool ip_address;
	int  expirationTime;
} ban_t;

ban_t ban_list[MAX_BANNED_USERS];
static bool	loadingBans = false;

extern cvar_t homedir;

void    Bans_Load(char *filename)
{
	int i = 0;

	while (i < MAX_BANNED_USERS && ban_list[i].ban)
	{
		Tag_Free(ban_list[i].ban);
		ban_list[i].ban = NULL;
		i++;
	}

#ifdef unix
	Cmd_ReadConfigFileAbsolute(filename, false);
#else
	Cmd_ReadConfigFile(filename, false);
#endif
}

void    Bans_Save(char *filename)
{
	file_t *file;
	int i;

	Console_DPrintf("bans_save\n");

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
	while (i < MAX_BANNED_USERS && ban_list[i].ban)
	{
		if (!ban_list[i].expirationTime)
			File_Printf(file, "ban %s\n", ban_list[i].ban);
		i++;
	}
	
	File_Close(file);
}

bool	Bans_Remove(char *name)
{
	int i = 0;

	while (i < MAX_BANNED_USERS && ban_list[i].ban)
	{
		if (strcmp(name, ban_list[i].ban) == 0)
		{
			Tag_Free(ban_list[i].ban);
			Mem_Move(&ban_list[i], &ban_list[i+1], MAX_BANNED_USERS-i-1 * sizeof(ban_t));
			return true;
		}
		i++;
	}
	return false;
}

void	Bans_Remove_cmd(int argc, char *argv[])
{
	int i;
	for (i = 0; i < argc; i++)
		Bans_Remove(argv[i]);
}

void	Bans_Add(char *name, int expirationTime)
{
	int n;
	int i = 0;
	int numDots = 0;
	
	while (i < MAX_BANNED_USERS && ban_list[i].ban)
	{
		if (strcmp(name, ban_list[i].ban) == 0)
			return;
		i++;
	}

	while (name[strlen(name)-1] == '*')
		name[strlen(name)-1] = '\0';
	while (name[strlen(name)-1] == '.')
		name[strlen(name)-1] = '\0';

	for (n = 0; n < strlen(name); n++)
	{
		if (name[n] == '.')
			numDots++;
	}
		
	if (i < MAX_BANNED_USERS)
	{
		ban_list[i].ban = Tag_Malloc(strlen(name)+2, MEM_BANS);
		if (numDots && numDots < 3)
			strcpy(ban_list[i].ban, fmt("%s.", name)); //add a final dot if it's not a full IP
		else
			strcpy(ban_list[i].ban, name); // either a guid or a complete IP address
		ban_list[i].expirationTime = System_Milliseconds() + expirationTime;
		ban_list[i].ip_address = (strchr(name, '.') != NULL);


		if (!loadingBans && !expirationTime) //save it out immediately in case of a ctrl-c or server crash
		{
#ifdef unix
			Bans_Save(fmt("%sbans.cfg", homedir.string));
#else
			Bans_Save("/bans.cfg");
#endif
		}
	}
}

void	Bans_Add_cmd(int argc, char *argv[])
{
	int time = 0;

	if (!argc)
	{
		Console_Printf("usage: ban <ip address|guid> <time>\n");
		return;
	}

	if (argc > 1)
		time = atoi(argv[1]);

	if (argc > 0)
	{
		Bans_Add(argv[0], time ? System_Milliseconds() + time : time);
	}
}

bool	Ban_IsUserIdBanned(int guid)
{
	int i = 0;
	char *str_userid;

	str_userid = fmt("%i", guid);
	while (i < MAX_BANNED_USERS && ban_list[i].ban)
	{
		if (!ban_list[i].ip_address)
			if (strcmp(ban_list[i].ban, str_userid) == 0)
				return true;
		i++;
	}
	return false;
}

char *Ban_IsIPBanned(char *ip_address)
{
	int i = 0;
	while (i < MAX_BANNED_USERS && ban_list[i].ban)
	{
		if (ban_list[i].ip_address)
			if (strncmp(ban_list[i].ban, ip_address, strlen(ban_list[i].ban)) == 0)
				return ban_list[i].ban;
		i++;
	}
	return NULL;
}

void	Bans_List_cmd(int argc, char *argv[])
{
	int i = 0;
	Console_Printf("Listing bans:\n");
	while (i < MAX_BANNED_USERS && ban_list[i].ban)
	{
		if (ban_list[i].ip_address)
			Console_Printf(" - %s (IP)\n", ban_list[i].ban);
		else
			Console_Printf(" - %s (GUID)\n", ban_list[i].ban);
		i++;
	}
}

void	Bans_Frame()
{
	int i = 0;
	int now = System_Milliseconds();

	while (i < MAX_BANNED_USERS && ban_list[i].ban)
	{
		if (ban_list[i].expirationTime && now > ban_list[i].expirationTime)
			Bans_Remove(ban_list[i].ban);
		else
			i++; //only inc i if we didn't just move a new ban into this slot
	}
}

void 	Bans_Init()
{
	memset(ban_list, 0, MAX_BANNED_USERS * sizeof(ban_t));
	Cmd_Register("ban", Bans_Add_cmd);
	Cmd_Register("unban", Bans_Remove_cmd);
	Cmd_Register("listbans", Bans_List_cmd);
	
	loadingBans = true;
#ifdef unix
	Bans_Load(fmt("%sbans.cfg", homedir.string));
#else
	Bans_Load("/bans.cfg");
#endif
	loadingBans = false;
}

void 	Bans_Shutdown()
{
#ifdef unix
	Bans_Save(fmt("%sbans.cfg", homedir.string));
#else
	Bans_Save("/bans.cfg");
#endif
}
