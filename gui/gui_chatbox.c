#include "client_game.h"
#include "gui_chatbox.h"
#include "cl_translator.h"
static char *class_name = "chatbox";

bool	GUI_Chatbox_CheckParties(gui_chatbox_t *chatbox, char *party)
{
	int i;

	i = 0;
	while (i < chatbox->num_parties)
	{
		if (strcmp(party, chatbox->parties[i]) == 0)
			return true;
		i++;
	}
	return false;
}

void	GUI_Chatbox_FreeParties(gui_chatbox_t *chatbox)
{
	int i;

	for (i = 0; i < chatbox->num_parties; i++)
		corec.GUI_Free(chatbox->parties[i]);
	corec.GUI_Free(chatbox->parties);
	chatbox->parties = NULL;
}

void	GUI_Chatbox_Notify(gui_element_t *obj, int argc, char *argv[])
{
	gui_chatbox_t *chatbox;
	gui_scrollbuffer_t *sb;
	char message[1024];
	char spoken_message[1024];
	char *msgpos;
	char *color = "";

	if (!argc)
		return;

	chatbox = corec.GUI_GetUserdata(class_name, obj);
	if (!chatbox)
		return;

	if (argc < 2)
	{
		return;
	}

	sb = chatbox->parent;
	
	if (strcmp(argv[0], "server") == 0)		//console message
	{
		strncpy(message, argv[1], 1023);
		message[1023] = 0;
		GUI_ScrollBuffer_Printf(chatbox->parent, fmt("^y%s", gettext(argv[1])), 0, "");
		sb->visible_start_line = (sb->current_line - sb->visible_lines + sb->rows) % sb->rows;
	}
	else
	{
		if (argc < 3)
			return;

		msgpos = argv[2];
		
		//parse out sounds
		if (strncmp(argv[2], "$sound ", 7)==0)
		{
			char snd[64];
			char *tokend;

			msgpos += 7;

			strncpy(snd, msgpos, 63);

			tokend = strchr(snd, ' ');
			if (tokend)
			{		
				*tokend = 0;
				if (chatbox->audible)
					corec.Cmd_Exec(fmt("playonce2d sound/%s\n", Filename_GetFilename(snd)));
				msgpos = GetNextWord(msgpos);
				if (!*msgpos)				
					msgpos = snd;	//so that we can see who played the sound and what sound they tried to play
			}
		}

		if (strcmp(argv[0], "msg_public") == 0)
		{
			if (chatbox->type != MESSAGE_PUBLIC)
				return;

			if (CL_ClientNameIsCommander(argv[1]))
				color = "^g[C]^w";			
			else if (CL_ClientNameIsOfficer(argv[1]))
				color = "^b[O]^w";
			else if (CL_ClientNameIsSpectator(argv[1]))
				color = "^555[S]^w";
			BPrintf(message, 1023, "%s<%s^w> %s\n", color, argv[1], msgpos);		
			BPrintf(spoken_message, 1023, "%s says %s\n", argv[1], msgpos);
		} 
		else if (strcmp(argv[0], "msg_sent") == 0)
		{
			if (chatbox->type != MESSAGE_TEAM
				&& chatbox->type != MESSAGE_PUBLIC)
				return;

			BPrintf(message, 1023, "->%s^w> %s\n", argv[1], msgpos);				
			strcpy(spoken_message, "");
		}
		else if (strcmp(argv[0], "msg_team") == 0)
		{
			if (chatbox->type != MESSAGE_TEAM
				&& chatbox->type != MESSAGE_PUBLIC)
				return;

			if (CL_ClientNameIsCommander(argv[1]))
				color = "^g[C]^w";			
			else if (CL_ClientNameIsOfficer(argv[1]))
				color = "^b[O]^w";			
			else if (CL_ClientNameIsSpectator(argv[1]))
				color = "^555[S]^w";

			BPrintf(message, 1023, "%s%s<%s^w> %s\n", cl.info->team ? "(team)" : "(spectator)", color, argv[1], msgpos);				
			BPrintf(spoken_message, 1023, "%s says to the %s, %s\n", argv[1], cl.info->team ? "team" : "spectators", msgpos);
		}
		else if (strcmp(argv[0], "msg_private") == 0)
		{
			if (chatbox->type != MESSAGE_PRIVATE
				&& chatbox->type != MESSAGE_PUBLIC)
				return;

			if (chatbox->type != MESSAGE_PUBLIC)
				if (!GUI_Chatbox_CheckParties(chatbox, argv[1]))
					return;

			if (CL_ClientNameIsCommander(argv[1]))
				color = "^g[C]^w";
			if (CL_ClientNameIsOfficer(argv[1]))
				color = "^b[O]^w";
			else if (CL_ClientNameIsSpectator(argv[1]))
				color = "^555[S]^w";

			color = "";

			BPrintf(message, 1023, "(msg)%s[%s^w]: %s\n", color, argv[1], msgpos);				
			BPrintf(spoken_message, 1023, "%s whispers %s\n", argv[1], msgpos);
		}

		if (chatbox->audible)
			corec.Speak(spoken_message);
		GUI_ScrollBuffer_Printf(chatbox->parent, getstring(message), 4, "");
		//if (((sb->last_line - sb->visible_lines + sb->rows) % sb->rows) == sb->visible_start_line)
		sb->visible_start_line = (sb->current_line - sb->visible_lines + sb->rows) % sb->rows;
	}
}

void	GUI_Chatbox_Destroy(gui_element_t *obj)
{
	gui_chatbox_t *chatbox;

	chatbox = corec.GUI_GetUserdata(class_name, obj);
	if (!chatbox)
		return;
	
	GUI_ScrollBuffer_Destroy(obj->parent);

	GUI_Chatbox_FreeParties(chatbox);
}

void	GUI_Chatbox_Type_Cmd(gui_element_t *obj, int argc, char *argv[])
{
	gui_chatbox_t *chatbox;
	int i;

	chatbox = corec.GUI_GetUserdata(class_name, obj);
	if (!chatbox)
		return;

	if (!argc)
	{
		corec.Console_Printf("chatbox type error: no type specified\n");
	}

	if (strcmp(argv[0], "public") == 0)
	{
		chatbox->type = MESSAGE_PUBLIC;
	}
	else if (strcmp(argv[0], "team") == 0)
	{
		if (argc > 1)
		{
			GUI_Chatbox_FreeParties(chatbox);
			chatbox->parties = corec.GUI_ReAlloc(chatbox->parties, sizeof (char *) * (argc -1));
			for (i = 0; i < argc -1; i++)
				chatbox->parties[i] = strdup(argv[i+1]);

			chatbox->type = MESSAGE_TEAM;
		} 
		else 
		{
			corec.Console_Printf("chatbox error: No team listed for team chat.\n");
		}
	} 
	else if (strcmp(argv[0], "private") == 0)
	{
		if (argc > 1)
		{
			GUI_Chatbox_FreeParties(chatbox);
			chatbox->parties = corec.GUI_ReAlloc(chatbox->parties, sizeof (char *) * (argc -1));
			for (i = 0; i < argc -1; i++)
				chatbox->parties[i] = strdup(argv[i+1]);

			chatbox->type = MESSAGE_PRIVATE;
		} 
		else 
		{
			corec.Console_Printf("chatbox error: No parties listed for private chat.\n");
		}
	}
}

void	GUI_Chatbox_Say_Cmd(int argc, char *argv[])
{
	gui_chatbox_t *chatbox;
	char message[1024];

	if (argc < 2)
	{
		corec.Console_Printf("chatbox say <panel:object> [<name>] <message>\n");
		return;
	}

	chatbox = corec.GUI_GetClass(argv[0], class_name);

	if (!chatbox)
		return;

	if (chatbox->type == MESSAGE_PUBLIC)
	{
		if (argv[1][0] != '/')
		{
			BPrintf(message, 1023, "%s %s", CLIENT_CHAT_MSG, argv[1]);
		}
		else
		{
			//starts with a /, just execute it like a console command
			BPrintf(message, 1023, "%s", &argv[1][1]);
		}
		corec.GUI_Exec(message);
	}
	else if (chatbox->type == MESSAGE_TEAM)
	{
		if (argv[1][0] != '/')
		{
			BPrintf(message, 1023, "%s %s", CLIENT_CHAT_TEAM_MSG, argv[1]);
		}
		else
		{
			//starts with a /, just execute it like a console command
			BPrintf(message, 1023, "%s", &argv[1][1]);
		}
		corec.GUI_Exec(message);
	}
	else if (chatbox->type == MESSAGE_PRIVATE)
	{
		if (argc > 2)
		{
			if (argv[2][0] != '/')
			{
				BPrintf(message, 1023, "%s %s %s", CLIENT_CHAT_PRIVATE_MSG, argv[1], argv[2]);
			}
			else
			{
				//starts with a /, just execute it like a console command
				BPrintf(message, 1023, "%s", &argv[2][1]);
			}
			corec.GUI_Exec(message);
		}
	}
}

void	GUI_Chatbox_Param(gui_element_t *obj, int argc, char *argv[])
{
	gui_chatbox_t *chatbox;

	if (argc < 2)
	{
		corec.Console_Printf("chatbox param <panel:object> <param> <value>\n");
		corec.Console_Printf("Available params:\n");
		corec.Console_Printf("     thickness    - (line thickness of chat box)\n");
		corec.Console_Printf("     delay_secs   - (# seconds delay before text fades)\n");
		corec.Console_Printf("     char_height  - (height of character [aka font size])\n");
		corec.Console_Printf("     rows         - (number of rows in chatbox)\n");
		corec.Console_Printf("     width        - (width of the chatbox)\n");
		corec.Console_Printf("     type         - (change the chatbox type [public, team, private])\n");
		return;
	}

	chatbox = corec.GUI_GetUserdata(class_name, obj);

	if (!chatbox)
		return;

	if (strcmp(argv[0], "type") == 0)
	{
		GUI_Chatbox_Type_Cmd(chatbox->element, argc-1, &argv[1]);
	}
	else if (strcmp(argv[0], "audible") == 0)
	{
		chatbox->audible = atoi(argv[1]);
	}
	else
	{
		GUI_ScrollBuffer_Param(obj, argc, argv);
	}
}

void	GUI_Chatbox_Cmd(int argc, char *argv[])
{
	gui_element_t *obj;

	if (!argc)
	{
		corec.Console_Printf("chatbox <command> <args>\n");
		corec.Console_Printf("  commands:\n");
		corec.Console_Printf("    list\n");
		corec.Console_Printf("    param\n");
		corec.Console_Printf("    say\n");
		return;
	}

	if (strcmp(argv[0], "list") == 0)
	{
		corec.GUI_List_Cmd(1, &class_name);
	} 
	else if (strcmp(argv[0], "param") == 0)
	{
		if (argc > 2)
		{
			obj = corec.GUI_GetObject(argv[1]);
			if (obj)
				GUI_Chatbox_Param(obj, argc-2, &argv[2]);
		}
	} 
	else if (strcmp(argv[0], "say") == 0)
	{
		GUI_Chatbox_Say_Cmd(argc-1, &argv[1]);
	} 

}

//chatbox "name" x y w h chatbox ["text"]
void	*GUI_Chatbox_Create(gui_element_t *obj, int argc, char *argv[])
{
	gui_chatbox_t *chatbox;
	gui_scrollbuffer_t *parent;

	if (argc < 5)
	{
		corec.Console_Printf("syntax: chatbox name x y width height [thickness] [char_height]\n");
		return NULL;
	}

	corec.GUI_SetClass(obj, class_name);

	parent = GUI_ScrollBuffer_Create(obj, argc, argv);

	if (!parent)
	{
		corec.Console_Printf("Chatbox error: couldn't instantiate scrollbuffer class.  See error above.\n");
		return NULL;
	}
	
	corec.GUI_SetName(obj, argv[0]);
	corec.GUI_Move (obj, atoi(argv[1]), atoi(argv[2]) );

	chatbox = corec.GUI_Malloc(sizeof (gui_chatbox_t));

	if (!chatbox)
	{
		corec.Console_Printf("Chatbox error: couldn't enough space to hold chatbox\n");
		return NULL; 		
	}

	corec.GUI_SetUserdata(class_name, obj, chatbox);
	chatbox->element = obj;
	chatbox->parent = parent;
	parent->wrap = true;
	parent->wraparound = true;
	parent->selectable = false;
	parent->selected_line = -1;
	parent->anchor_to_current_line = true;

//	obj->interactive = false;
	obj->destroy = GUI_Chatbox_Destroy;
	obj->notify = GUI_Chatbox_Notify;
	obj->param = GUI_Chatbox_Param;

	chatbox->type = MESSAGE_PUBLIC;
	chatbox->audible = false;

	return chatbox;
}

void	GUI_Chatbox_Init()
{
	corec.Cmd_Register("chatbox", GUI_Chatbox_Cmd);

	corec.GUI_RegisterClass(class_name, GUI_Chatbox_Create);
}

