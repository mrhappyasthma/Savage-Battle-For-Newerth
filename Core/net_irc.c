/*
 * (C) 2003 S2 Games
 *
 * net_irc.c - a very basic irc client
 */

#include "core.h"
#include "../keyserver/auth_common.h"

#ifdef SAVAGE_DEMO
cvar_t	irc_disable = { "irc_disable", "1", CVAR_READONLY };
#else
cvar_t	irc_disable = { "irc_disable", "0" };
#endif

#ifdef _WIN32
    static SOCKET irc_sock = 0;
#else
	static int irc_sock = 0;
#endif

#define DEFAULT_IRC_PORT 6667
#define MAX_BUF 1024
#define TMP_BUF 128
#define MAX_NICK_LENGTH 32
#define MAX_CHANNEL_LENGTH 32
#define MAX_CHANNEL_USERS 500
	
#define PING_COMMAND "PING :"
#define JOIN_COMMAND "JOIN :"
#define PART_COMMAND "PART :"
#define RENAME_COMMAND "NICK :"
#define INVALID_NICK_COMMAND "Nickname is already in use"
#define PRIVMSG_COMMAND "PRIVMSG"
#define TOPIC_COMMAND "332 "
#define CHANNELLIST_COMMAND "353 "

#define GAME_INFO_REQUEST	"__GAMEINFOREQ"
#define GAME_INFO_MSG		"__GAMEINFO"

static int nick_collision_counter = 0;
static char _nickAttempt[MAX_NICK_LENGTH];
static int last_buddy_check = 0;
static bool free_users_next_353 = true;
static int num_channel_users;

cvar_t irc_nickname	=	{"irc_nickname", "user" };
cvar_t irc_channel = 	{"irc_channel", "savage.chat" };
cvar_t irc_channel_number = 	{"irc_channel_number", "0" };
cvar_t irc_realchannel = 	{"irc_realchannel", "" };
cvar_t irc_maxusers = 	{"irc_maxusers", "60" };

static char *channel_users[MAX_CHANNEL_USERS];
static char *online_buddies[MAX_BUDDY_LIST_USERS];

static bool looking_for_channel = false;
static bool in_channel = false;
	
extern char *buddy_list[MAX_BUDDY_LIST_USERS];
extern cvar_t user_id;
extern cvar_t server_address; //from host.c
extern cvar_t server_port;

void (*_outputCallback)(char *string);
void (*_outputMsgCallback)(char *user, char *string, bool incoming);

void	IRC_PrintPrivMsg(char *nick, char *str);
bool    IRC_List(char *channel);
		
bool	IRC_SetNick(char *newname)
{
	char buf[TMP_BUF];
	int size, ret;

	if (irc_disable.integer)
		return false;

	if (strcmp(_nickAttempt, newname) != 0)
		strncpySafe(_nickAttempt, newname, MAX_NICK_LENGTH);

	if (irc_sock > 0)
	{
		BPrintf(buf, TMP_BUF, "NICK _%i|%s\n", user_id.integer, newname);
		size = strlen(buf);
		ret = TCP_Write(irc_sock, buf, size);
		if (ret == -1)
		{
#ifndef _WIN32
			if (errno == EPIPE)
			{
				Console_DPrintf("The other side closed the tcp connection\n");
			}
#endif
			irc_sock = 0;
			return false;
		}
		if (ret != size)
			return false;
	}
	return true;
}

void	IRC_DefaultOutput(char *string)
{
	if (irc_disable.integer)
		return;

	Console_DPrintf(string);
}

void    IRC_OutputMsgCallback(void (*outputcallback)(char *user, char *string, bool incoming))
{
	_outputMsgCallback = outputcallback;
}

void    IRC_OutputCallback(void (*outputcallback)(char *string))
{
	if (irc_disable.integer)
		return;

	_outputCallback = outputcallback;
}

char **IRC_GetUserList()
{
	return channel_users;
}

char **IRC_GetBuddyList()
{
	return online_buddies;
}

bool	IRC_Connect(char *servername)
{
	char buf[TMP_BUF];

	if (irc_disable.integer)
		return false;

	if (irc_sock > 0)
	{
		//IRC_Disconnect(NULL);
		return true;
	}
	
	irc_sock = TCP_Connect(servername, DEFAULT_IRC_PORT);
	nick_collision_counter = 0;

	if (irc_sock > 0)
	{
		set_nonblocking(irc_sock);
		BPrintf(buf, TMP_BUF, "USER S2Client localhost irc :S. Avage\n");

		IRC_SetNick(_nickAttempt);
		if (TCP_Write(irc_sock, buf, strlen(buf)) == -1)
		{
#ifndef _WIN32
			if (errno == EPIPE)
			{
				Console_DPrintf("The other side closed the tcp connection\n");
			}
#endif
			irc_sock = 0;
		}
		return true;
	}
	return false;
}

bool	IRC_Disconnect(char *message)
{
	char buf[TMP_BUF];

	if (irc_disable.integer)
		return false;

	if (irc_sock > 0)
	{
		BPrintf(buf, TMP_BUF, "QUIT :%s\n", message ? message : "");
		if (TCP_Write(irc_sock, buf, strlen(buf)) == -1)
		{
#ifndef _WIN32
			if (errno == EPIPE)
			{
				Console_DPrintf("The other side closed the tcp connection\n");
			}
#endif
			irc_sock = 0;
		}
		else
		{
			TCP_Close(irc_sock);
			irc_sock = 0;
		}
		return true;
	}
	return false;
}

static bool	_getRestOfLine(char *str, char *to_buf, int maxlen)
{
	int i;
	char *tmp = str;
	while (*tmp && *tmp != '\n')
		tmp++;

	i = tmp - str;
	if (maxlen < i)
		i = maxlen - 1;
	strncpy(to_buf, str, i);
	to_buf[i] = 0;
	return tmp - str;
}

void	IRC_PrintChannelMsg(char *nick, char *str)
{
	if (irc_disable.integer)
		return;

	_outputCallback(fmt("<%s> %s\n", nick, str));
}

char	*_stripUserId(char *nick)
{
	char *pos;

	pos = strchr(nick, '|');
	if (!pos)
		pos = nick;
	else
		pos++;

	return pos;
}

bool	_specialMsg(char *nick, char *str)
{
	char **tokens;
	bool success = false;
	
	tokens = g_strsplit(str, " ", MAX_BUDDY_LIST_USERS + 5);

	if (tokens && tokens[0] && strcmp(tokens[0], GAME_INFO_REQUEST) == 0)
	{
		//IRC_PrintPrivMsg(nick, " -- requested your current game info\n");
		if (localClient.cstate > CCS_CONNECTING)
		{
			IRC_Msg(nick, fmt("%s %s %i", GAME_INFO_MSG, server_address.string, server_port.integer), false);
		}
		else
		{
			IRC_Msg(nick, fmt("%s 0 0", GAME_INFO_MSG), false);
		}
		success = true;
	}
	else if (tokens && tokens[0] && strcmp(tokens[0], GAME_INFO_MSG) == 0)
	{
		//IRC_PrintPrivMsg(nick, " -- received current game info\n");
		if (strcmp(tokens[1], "0") != 0)
		{
			Cvar_Set("buddy_gameip", tokens[1]);
			Cvar_Set("buddy_gameport", tokens[2]);
			//IRC_PrintPrivMsg(nick, fmt(" -- playing on server %s, port %i\n", tokens[1], atoi(tokens[2])));
		}
		else
		{
			Cvar_Set("buddy_gameip", "");
			Cvar_Set("buddy_gameport", "");
		}
		success = true;
	}
	
	g_strfreev(tokens);
	return success;
}

bool	_getNick(char *str, char *nick)
{
	char *pos, *nickstart;

	if (irc_disable.integer)
		return false;

	if (str[0] != ':')
	{
		Console_DPrintf("IRC PARSE ERROR: unknown initial char '%c' in line %s\n", str[0], str);
		return false;
	}
	pos = strchr(&str[1], '!');
	/*nickstart = strchr(&str[1], '|');
	if (pos && !nickstart)*/
		nickstart = &str[1];
	if (pos && nickstart)
	{
		//nickstart++;
		strncpy(nick, nickstart, pos - nickstart);
		nick[pos - nickstart] = 0;
		return true;
	}
	else
	{
		nick[0] = 0;
	}
	return false;
}

void	IRC_PrintPrivMsg(char *nick, char *str)
{
	if (irc_disable.integer)
		return;

	_outputMsgCallback(nick, str, true);
}

void	IRC_Rename(char *old, char *new)
{
	int i;
	char *tmp;
	char leadingchar;
	bool leadingchar_necessary;

	i = 0;
	while (channel_users[i])
	{
		tmp = channel_users[i];
		leadingchar = tmp[0];
		leadingchar_necessary = false;
		if (leadingchar == '@' || leadingchar == '+')
		{
			leadingchar_necessary = true;
			tmp++;
		}
		if (strcmp(tmp, old) == 0)
		{
			Tag_Free(channel_users[i]);
			if (leadingchar_necessary)			
				channel_users[i] = Tag_Strdup(fmt("%c%s", leadingchar, new), MEM_IRC);							
			else
				channel_users[i] = Tag_Strdup(new, MEM_IRC);
			return;
		}
		i++;
	}
}

void	IRC_NickChanged(char *nick, char *newnick)
{
	if (strcmp(nick, irc_nickname.string) == 0)
	{
		Cvar_SetVar(&irc_nickname, newnick);
	}
}

void	IRC_ParseLine(char *str)
{
	char buf[MAX_BUF];
	char nick[MAX_NICK_LENGTH];
	char **tokens;
	char *tmp;
	int i, j;

	if (irc_disable.integer)
		return;

	tokens = g_strsplit(str, " ", 4);
	i = 0;
	while ((tmp = tokens[i]))
	{
		if (strlen(tmp) > 0 && tmp[strlen(tmp)-1] == '\r')
			tmp[strlen(tmp)-1] = 0;
		i++;
	}
	
	if (tokens[0] && tokens[1])
	{
		if (strcmp(tokens[0], "PING") == 0)
		{
			//respond to a PING command with a PONG
			BPrintf(buf, MAX_BUF, "PONG %s\n", &tokens[1][1]);
			if (TCP_Write(irc_sock, buf, strlen(buf)) == -1)
			{
#ifndef _WIN32
				if (errno == EPIPE)
				{
					Console_DPrintf("The other side closed the tcp connection\n");
				}
#endif
				irc_sock = 0;
			}
			goto done;
		}
		else if (strcmp(tokens[1], "001") == 0) //made it on
		{
			IRC_NickChanged(irc_nickname.string, tokens[2]);
			if (irc_channel.string[0])
				IRC_List(irc_channel.string);
			
			BPrintf(buf, MAX_BUF, "MODE %s +i\n", irc_nickname.string);
			if (TCP_Write(irc_sock, buf, strlen(buf)) == -1)
			{
#ifndef _WIN32
				if (errno == EPIPE)
				{
					Console_DPrintf("The other side closed the tcp connection\n");
				}
#endif
				irc_sock = 0;
			}
			goto done;
		}
		else if ((strcmp(tokens[1], "311") == 0) //whois lines
				 || (strcmp(tokens[1], "319") == 0)
				 || (strcmp(tokens[1], "312") == 0)
				 || (strcmp(tokens[1], "318") == 0)
				 || (strcmp(tokens[1], "330") == 0)
				 || (strcmp(tokens[1], "401") == 0))
		{
			_outputCallback(fmt("%s\n", tokens[3]));
		}
		else if (strcmp(tokens[1], "321") == 0)
		{
			in_channel = true;
			num_channel_users = 0;
		}
		else if (strcmp(tokens[1], "322") == 0)
		{
			g_strfreev(tokens);
			tokens = g_strsplit(str, " ", 6);
			if (tokens[0] && tokens[1] && tokens[2] && tokens[3] && tokens[4])
				num_channel_users = atoi(tokens[4]);
			else
				num_channel_users = 0;
		}
		else if (strcmp(tokens[1], "323") == 0)
		{
			if (looking_for_channel)
			{
				if (num_channel_users > irc_maxusers.integer)
				{
					Cvar_SetVarValue(&irc_channel_number, irc_channel_number.integer + 1);
					IRC_List(fmt("%s%i", irc_channel.string, irc_channel_number.integer));
				}
				else
				{
					if (irc_channel_number.integer == 0)
						IRC_Join(fmt("%s", irc_channel.string));
					else
						IRC_Join(fmt("%s%i", irc_channel.string, irc_channel_number.integer));
				}
			}
		}
		/*
		else if (strcmp(tokens[1], "433") == 0)
		{
			nick_collision_counter++;
			IRC_SetNick(fmt("%s%i", _nickAttempt, nick_collision_counter));
			goto done;
		}
		*/
		else if (strcmp(tokens[1], "438") == 0)
		{
			if (strchr(tokens[3], ':'))
				_outputCallback(fmt("%s\n", strchr(tokens[3], ':')+1));
			goto done;
		}
		else if (strcmp(tokens[1], "PRIVMSG") == 0)
		{
			if (!_getNick(tokens[0], nick))
				goto done;

			if (!tokens[2] || !tokens[3])
				goto done;
			
			if (_specialMsg(nick, &tokens[3][1]))
				goto done;
			
			if (tokens[2][0] == '#')
				IRC_PrintChannelMsg(_stripUserId(nick), &tokens[3][1]);
			else
				IRC_PrintPrivMsg(nick, &tokens[3][1]);
			goto done;
		}
		else if (strcmp(tokens[1], "JOIN") == 0)
		{
			if (!_getNick(tokens[0], nick))
				goto done;

			if (strlen(tokens[0]) > 3)
			{
				if (!_stripUserId(nick))
					goto done;

				if (!&tokens[0][strlen(nick)+2] || !&tokens[2][1])
					goto done;

				_outputCallback(fmt("%s has joined %s\n", _stripUserId(nick), &tokens[2][1]));
				i = 0;
				while (i < MAX_CHANNEL_USERS && channel_users[i])
					i++;

				if (i < MAX_CHANNEL_USERS)
				{
					channel_users[i] = Tag_Strdup(nick, MEM_IRC);
				}
			}
			goto done;
		}
		else if (strcmp(tokens[1], "PART") == 0)
		{
			if (!_getNick(tokens[0], nick))
				goto done;

			if (!tokens[2])
				goto done;
			
			if (tokens[3] && strlen(tokens[3]) < 1)
			{
				Console_DPrintf("IRC Parse error: '%s'\n", str);
				goto done;
			}
			
			_outputCallback(fmt("%s has left %s (%s)\n", _stripUserId(nick), tokens[2], tokens[3] ? &tokens[3][1] : ""));

			i = 0;
			while (i < MAX_CHANNEL_USERS && channel_users[i])
			{
				if (strcmp(channel_users[i], nick) == 0)
				{
					Tag_Free(channel_users[i]);
					Mem_Move(&channel_users[i], &channel_users[i+1], sizeof(char *) * (MAX_CHANNEL_USERS - (i + 1)));
					channel_users[MAX_CHANNEL_USERS-1] = NULL;
				}
				i++;
			}
			
			goto done;
		}
		else if (strcmp(tokens[1], "QUIT") == 0)
		{
			if (!_getNick(tokens[0], nick))
				goto done;

			if (tokens[2] && strlen(tokens[2]) >= 2)
				_outputCallback(fmt("%s has quit (%s %s)\n", _stripUserId(nick), &tokens[2][1], tokens[3] ? tokens[3] : ""));
			else
				_outputCallback(fmt("%s has quit\n", _stripUserId(nick)));

			i = 0;
			while (i < MAX_CHANNEL_USERS && channel_users[i])
			{
				if (strcmp(channel_users[i], nick) == 0)
				{
					Tag_Free(channel_users[i]);
					Mem_Move(&channel_users[i], &channel_users[i+1], sizeof(char *) * (MAX_CHANNEL_USERS - (i + 1)));
					channel_users[MAX_CHANNEL_USERS-1] = NULL;
				}
				i++;
			}
			
			goto done;
		}
		else if (strcmp(tokens[1], "NICK") == 0)
		{
			if (!_getNick(tokens[0], nick))
				goto done;

			if (strlen(tokens[2]) > 0 && tokens[2][strlen(tokens[2])-1] == '\r')
				tokens[2][strlen(tokens[2])-1] = 0;
			if (tokens[2][0] == ':')
			{
				IRC_NickChanged(nick, &tokens[2][1]);
				_outputCallback(fmt("%s has renamed themself to %s\n", _stripUserId(nick), _stripUserId(&tokens[2][1])));
				IRC_Rename(nick, &tokens[2][1]);
			}
			goto done;
		}
		else if (strcmp(tokens[1], "303") == 0)
		{
			g_strfreev(tokens);
	
			//free the existing channel users array, if any
			i = 0;
			while (online_buddies[i] && i < MAX_CHANNEL_USERS)
			{
				Tag_Free(online_buddies[i]);
				online_buddies[i] = NULL;
				i++;
			}
			
			//re-tokenize to get the full list of names
			tokens = g_strsplit(str, " ", MAX_BUDDY_LIST_USERS + 5);
			i = 0;
			while ((tmp = tokens[i]))
			{
				if (strlen(tmp) > 0 && tmp[strlen(tmp)-1] == '\r')
					tmp[strlen(tmp)-1] = 0;
				i++;
			}

			strcpy(buf, "");

			if (tokens[2] && tokens[3])
			{
				j = 0;
				i = 3;
				while (tokens[i])
				{
					if (i == 3)
					{
						tmp = &tokens[i][1];
					}
					else
					{
						tmp = tokens[i];
					}
					if (tmp[0] == '\r')
					{
						i++;
						continue;
					}
					if (strlen(tmp) > 0 && tmp[strlen(tmp)-1] == '\r')
						tmp[strlen(tmp)-1] = 0;
		
					online_buddies[j] = Tag_Strdup(tmp, MEM_IRC);
					
					i++;
					j++;
					if (j >= MAX_BUDDY_LIST_USERS - 1)
						break;
				}
				online_buddies[j] = NULL;
				goto done;
			}
		}
		else if (strcmp(tokens[1], "366") == 0)
		{
			free_users_next_353 = true;
		}
		else if (strcmp(tokens[1], "353") == 0)
		{
			g_strfreev(tokens);
	
			if (free_users_next_353)
			{
				_outputCallback(fmt("The following users are in %s\n", irc_realchannel.string));

				//free the existing channel users array, if any
				i = 0;
				while (channel_users[i])
				{
					Tag_Free(channel_users[i]);
					channel_users[i] = NULL;
					i++;
				}
				free_users_next_353 = false;
			}
			
			//re-tokenize to get the full list of names
			tokens = g_strsplit(str, " ", MAX_CHANNEL_USERS + 5);
			i = 0;
			while ((tmp = tokens[i]))
			{
				if (strlen(tmp) > 0 && tmp[strlen(tmp)-1] == '\r')
					tmp[strlen(tmp)-1] = 0;
				i++;
			}
			if (i < 5)
				goto done;

			strcpy(buf, "");
			j = 0;
			while (j < MAX_CHANNEL_USERS - 1 && channel_users[j])
				j++;

			i = 5;
			while (tokens[i])
			{
				if (i == 5)
				{
					tmp = &tokens[i][1];
				}
				else
				{
					tmp = tokens[i];
				}
				if (!tmp || tmp[0] == '\r')
				{
					i++;
					continue;
				}
	
				channel_users[j] = Tag_Strdup(tmp, MEM_IRC);
				
				strncat(buf,  _stripUserId(tmp), MIN(strlen( _stripUserId(tmp)), MAX_BUF - strlen( _stripUserId(buf))));
				if (strlen(buf) >= MAX_BUF - 1)
					break;
				strcat(buf, " ");
				i++;
				j++;
				if (j >= MAX_CHANNEL_USERS - 1)
					break;
			}
			channel_users[j] = NULL;
			if (strlen(buf) < MAX_BUF - 2)
				strcat(buf, "\n");
			_outputCallback(buf);
			goto done;
		}
	}

done:
	g_strfreev(tokens);
}

void	IRC_CheckBuddies()
{
	char buf[MAX_BUF];
	int i = 0;

	if (!irc_sock)
		return;

	BPrintf(buf, MAX_BUF, "ISON ");
	while (i < MAX_BUDDY_LIST_USERS 
			&& buddy_list[i]
			&& strlen(buf) + strlen(buddy_list[i]) + 3 < MAX_BUF)
	{
		strcat(buf, buddy_list[i]);
		strcat(buf, " ");
		i++;
	}
	strcat(buf, "\n");

	if (i > 0)
	{
		if (TCP_Write(irc_sock, buf, strlen(buf)) == -1)
		{
#ifndef _WIN32
			if (errno == EPIPE)
			{
				Console_DPrintf("The other side closed the tcp connection\n");
			}
#endif
			irc_sock = 0;
		}
	}
	else
	{
		for (i = 0; i < MAX_BUDDY_LIST_USERS; i++)
			online_buddies[i] = NULL;
	}
}

void	IRC_Frame()
{
	char buf[MAX_BUF];
	char line[MAX_BUF];
	char *pos;
	int ret, linenum;	

	OVERHEAD_INIT;
	
	if (irc_disable.integer)
		return;

	if (irc_sock <= 0)
		return;
	
	ret = TCP_Read(irc_sock, buf, MAX_BUF-1);
	if (ret > 0)
	{
		buf[ret] = 0;
		pos = buf;
		Console_DPrintf("%s", buf);
		
		linenum = 0;
		while (pos - buf < ret && _getRestOfLine(pos, line, MAX_BUF))
		{
			//Console_DPrintf("linenum %i: '%s'\n", linenum, line);
			IRC_ParseLine(line);
			pos += strlen(line)+1;
			linenum++;
		}
	}
	if (last_buddy_check == 0
		|| System_Milliseconds() - last_buddy_check > 30000)
	{
		IRC_CheckBuddies();
		last_buddy_check = System_Milliseconds();
	}

	OVERHEAD_COUNT(OVERHEAD_IRC);
}

bool	IRC_Part()
{
	char buf[TMP_BUF];

	if (irc_disable.integer)
		return false;

	if (irc_realchannel.string[0])
	{
		BPrintf(buf, TMP_BUF, "PART %s\n", irc_realchannel.string);
		if (TCP_Write(irc_sock, buf, strlen(buf)) == -1)
		{
#ifndef _WIN32
			if (errno == EPIPE)
			{
				Console_DPrintf("The other side closed the tcp connection\n");
			}
#endif
			irc_sock = 0;
		}
		in_channel = false;
		Cvar_SetVar(&irc_realchannel, "");
		return true;
	}
	return false;
}

bool    IRC_List(char *channel)
{
	char buf[TMP_BUF];

	if (irc_disable.integer)
		return false;

	if (irc_sock > 0)
	{
		BPrintf(buf, TMP_BUF, "LIST %s\n", channel);
		if (TCP_Write(irc_sock, buf, strlen(buf)) == -1)
		{
#ifndef _WIN32
			if (errno == EPIPE)
			{
				Console_DPrintf("The other side closed the tcp connection\n");
			}
#endif
			irc_sock = 0;
		}
		return true;
	}
	return false;
}

bool    IRC_Join(char *channel)
{
	char buf[TMP_BUF];

	if (irc_disable.integer)
		return false;

	looking_for_channel = !looking_for_channel;
	
	if (looking_for_channel)
	{
		Cvar_SetVarValue(&irc_channel_number, 0);
		Cvar_SetVar(&irc_channel, channel);
		IRC_List(channel);
		return true;
	}
	
	if (strcmp(irc_realchannel.string, channel) != 0)
	{
		if (irc_sock > 0)
			IRC_Part();
		Cvar_SetVar(&irc_realchannel, channel);
	}

	if (irc_sock > 0)
	{
		BPrintf(buf, TMP_BUF, "JOIN %s\n", irc_realchannel.string);
		if (TCP_Write(irc_sock, buf, strlen(buf)) == -1)
		{
#ifndef _WIN32
			if (errno == EPIPE)
			{
				Console_DPrintf("The other side closed the tcp connection\n");
			}
#endif
			irc_sock = 0;
		}
		return true;
	}
	return false;
}

void	IRC_Whois(char *string)
{
	if (TCP_Write(irc_sock, fmt("WHOIS %s\n", string), strlen(string)+7) == -1)
	{
#ifndef _WIN32
		if (errno == EPIPE)
		{
			Console_DPrintf("The other side closed the tcp connection\n");
		}
#endif
		irc_sock = 0;
	}
}

void	IRC_Raw(char *string)
{
	if (TCP_Write(irc_sock, fmt("%s\n", string), strlen(string)+1) == -1)
	{
#ifndef _WIN32
		if (errno == EPIPE)
		{
			Console_DPrintf("The other side closed the tcp connection\n");
		}
#endif
		irc_sock = 0;
	}
}

bool    IRC_Msg(char *user, char *msg, bool printmsg)
{
	char buf[MAX_BUF];
	int ret;

	if (irc_disable.integer)
		return false;

	if (msg[0] && msg[0] == '/')
	{
		Cmd_Exec(fmt("irc_%s", &msg[1]));
		return true;
	}
	
	if (irc_sock > 0 && strlen(msg) > 0)
	{
		BPrintf(buf, MAX_BUF, "PRIVMSG %s :%s\n", user, msg);
		buf[MAX_BUF-1] = 0;
		ret = TCP_Write(irc_sock, buf, strlen(buf));
		if (ret > 0)
		{
			if (printmsg)
			{
				if (user[0] == '#')
					_outputCallback(fmt("<%s> %s\n", _stripUserId(irc_nickname.string), msg));
				else
					_outputMsgCallback(user, msg, false);
			}
			return true;
		}
		else if (ret == -1)
		{
#ifndef _WIN32
			if (errno == EPIPE)
			{
				Console_DPrintf("The other side closed the tcp connection\n");
			}
#endif
			irc_sock = 0;
		}
	}
	return false;
}

bool    IRC_Say(char *msg)
{
	return IRC_Msg(fmt("%s", irc_realchannel.string), msg, true);
}

bool	IRC_RequestUserGameInfo(char *username)
{
	if (irc_disable.integer)
		return false;

	return IRC_Msg(username, GAME_INFO_REQUEST, false);
}

//============================================================

void	IRC_Connect_cmd(int argc, char *argv[])
{
	if (irc_disable.integer)
		return;

	if (argc)
		IRC_Connect(argv[0]);
}

void	IRC_Disconnect_cmd(int argc, char *argv[])
{
	if (irc_disable.integer)
		return;

	IRC_Disconnect(argc ? argv[0] : NULL);
}

void	IRC_RequestUserGameInfo_cmd(int argc, char *argv[])
{
	if (argc)
		IRC_RequestUserGameInfo(argv[0]);
}

void	IRC_Say_cmd(int argc, char *argv[])
{
	char *s;

	if (irc_disable.integer)
		return;

	if (argc)
	{
		argv[argc] = NULL;
		s = g_strjoinv(" ", argv);

		IRC_Say(s);
	
		g_free(s);
	}
}

void	IRC_Msg_cmd(int argc, char *argv[])
{
	char *s;	

	if (irc_disable.integer)
		return;

	if (argc > 1)
	{
		argv[argc] = NULL;
		s = g_strjoinv(" ", &argv[1]);

		IRC_Msg(argv[0], s, true);
	
		g_free(s);
	}
}

void	IRC_List_cmd(int argc, char *argv[])
{
	if (irc_disable.integer)
		return;

	if (argc)
		IRC_List(argv[0]);
}

void	IRC_Join_cmd(int argc, char *argv[])
{
	if (irc_disable.integer)
		return;

	if (argc)
		IRC_Join(argv[0]);
}

void	IRC_Nick_cmd(int argc, char *argv[])
{
	if (irc_disable.integer)
		return;

	if (argc)
	{
		nick_collision_counter = 0;
		strncpySafe(_nickAttempt, argv[0], MAX_NICK_LENGTH);
		_nickAttempt[MAX_NICK_LENGTH-1] = 0;

		IRC_SetNick(argv[0]);
	}
}

void	IRC_Whois_cmd(int argc, char *argv[])
{
	if (argc)
	{
		IRC_Whois(argv[0]);
	}
}

void	IRC_Part_cmd(int argc, char *argv[])
{
	if (irc_disable.integer)
		return;

	IRC_Part();
}

void	IRC_Raw_cmd(int argc, char *argv[])
{
	char *s;
	if (argc > 1)
	{
		argv[argc] = NULL;
		s = g_strjoinv(" ", argv);

		IRC_Raw(s);
	
		g_free(s);
	}
}

void	IRC_RefreshBuddies_cmd(int argc, char *argv[])
{
	last_buddy_check = 0;
}

//==============================================================

void	IRC_Shutdown()
{
	if (irc_sock > 0)
		TCP_Close(irc_sock);
}

bool	IRC_Init()
{
	memset(channel_users, 0, MAX_CHANNEL_USERS * sizeof( char *));
	memset(online_buddies, 0, MAX_BUDDY_LIST_USERS * sizeof( char *));
	
	irc_sock = 0;
	strcpy(_nickAttempt, "");

	channel_users[0] = NULL;

	_outputCallback = IRC_DefaultOutput;
	
	Cvar_Register(&irc_disable);
	Cvar_Register(&irc_nickname);
	Cvar_Register(&irc_channel);
	Cvar_Register(&irc_realchannel);
	Cvar_Register(&irc_channel_number);
	Cvar_Register(&irc_maxusers);

	Cmd_Register("irc_connect", &IRC_Connect_cmd);
	Cmd_Register("irc_disconnect", &IRC_Disconnect_cmd);
	Cmd_Register("irc_nick", &IRC_Nick_cmd);
	Cmd_Register("irc_say", &IRC_Say_cmd);
	Cmd_Register("irc_msg", &IRC_Msg_cmd);
	Cmd_Register("irc_whois", &IRC_Whois_cmd);
	Cmd_Register("irc_raw", &IRC_Raw_cmd);
	Cmd_Register("irc_m", &IRC_Msg_cmd);
	Cmd_Register("irc_join", &IRC_Join_cmd);
	Cmd_Register("irc_part", &IRC_Part_cmd);
	Cmd_Register("refreshbuddies", &IRC_RefreshBuddies_cmd);
	Cmd_Register("getgameinfo", &IRC_RequestUserGameInfo_cmd);
	return true;
}

