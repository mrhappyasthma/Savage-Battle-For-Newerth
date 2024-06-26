
#include "gui_userlist_ping.h"

static char *class_name = "userlist";

#define MODE_NAMES  0
#define MODE_SCORES 1
#define MODE_DEATHS 2
#define MODE_PINGS  3
#define MODE_KILLS  4

static gui_userlist_t *sorting_userlist;
static bool incremental = true;

extern cvar_t  cl_showClanAbbrevInUserlist;
extern cvar_t  cl_showClanIconInUserlist;

int _GUI_UserList_ValueCompare(const void *p1, const void *p2)
{
	char *str1 = NULL;
	char *str2 = NULL;
	int pos1 = *(int *)(incremental ? p1 : p2);
	int pos2 = *(int *)(incremental ? p2 : p1);

	switch (sorting_userlist->sortColumn)
	{
		case MODE_NAMES:
			str1 = cl.clients[pos1].info.name;
			str2 = cl.clients[pos2].info.name;
			return stricmp(str1, str2);
		case MODE_PINGS:
			return cl.clients[pos1].score.ping - cl.clients[pos2].score.ping;
		case MODE_KILLS:
			return cl.clients[pos1].score.kills - cl.clients[pos2].score.kills;
		case MODE_DEATHS:
			return cl.clients[pos1].score.deaths - cl.clients[pos2].score.deaths;
		default:
		case MODE_SCORES:
			return cl.clients[pos1].score.experience - cl.clients[pos2].score.experience;
			break;
	}
}

void	GUI_UserList_Draw(gui_element_t *obj, int w, int h)
{
	int y = 0;
	int i, j;
	char clanInformation[4096] = {0};
	gui_userlist_t *userlist;
	sharedClientInfo_t *info;
	playerScore_t *score;
	vec4_t textcolor;

	userlist = corec.GUI_GetUserdata(class_name, obj);

	if (!userlist)
		return;

	if (userlist->mode == MODE_NAMES)
		corec.Client_GetStateString(ST_CLAN_INFO, clanInformation, 4096);

	i = 0;

	if (1) //userlist->needsSorting)
	{
		int num_rows = 0;
		j = 0;
		
		for (i = 0; i < MAX_CLIENTS; i++)
		{
			info = &cl.clients[i].info;
	  		if (!info->active || (info->team != userlist->team && userlist->team != -1))
			{
				continue;
			}
			userlist->order[j] = i;
			j++;
		}
		num_rows = j;
		while (j < MAX_CLIENTS)
		{
			userlist->order[j] = -1;
			j++;
		}
		sorting_userlist = userlist;
		incremental = userlist->incremental;
		//corec.Console_DPrintf("Sorting %i rows of the scrollbuffer using column %i as the sort column\n", num_rows, userlist->sortColumn);
		qsort(userlist->order, num_rows, sizeof(int), _GUI_UserList_ValueCompare);
		userlist->needsSorting = false;
	}
	
//	TL_SetCoordSystem(GUI_COORDS);

	i = 0;
	while (i < MAX_CLIENTS)
	{
		if (userlist->order[i] < 0)
		{
			i++;
			continue;
		}

		info = &cl.clients[userlist->order[i]].info;
		score = &cl.clients[userlist->order[i]].score;

	  	if (!info->active || (info->team != userlist->team && userlist->team != -1))
		{
			i++;
			continue;
		}

		if (obj->interactive)
		{
			if (CL_UnitIsSelected(&cl.selection, userlist->order[i]))
			{
				M_CopyVec4(userlist->selected_bg, textcolor);
				textcolor[3] = MAX(0, obj->alpha * (textcolor[3]));
				corec.Draw_SetColor(textcolor);
				corec.GUI_Quad2d_S(0, y, obj->width, obj->char_height, corec.GetWhiteShader());
				
				M_CopyVec4(userlist->selected_textcolor, textcolor);
				textcolor[3] = MAX(0, obj->alpha * (textcolor[3]));
				corec.Draw_SetColor(textcolor);
				M_CopyVec3(obj->textcolor, textcolor);
				textcolor[3] = MAX(0, obj->alpha);
			}
			else
			{
				M_CopyVec3(obj->textcolor, textcolor);
				textcolor[3] = MAX(0, obj->alpha);
				corec.Draw_SetColor(textcolor);
			}
		}
		else
		{
			M_CopyVec3(obj->textcolor, textcolor);
			textcolor[3] = MAX(0, obj->alpha);
			corec.Draw_SetColor(textcolor);
		}
		switch (userlist->mode)
		{
			case MODE_NAMES:
				if (cl.teams[1].commander == userlist->order[i] || cl.teams[2].commander == userlist->order[i])
				{
					corec.GUI_Quad2d_S(-(obj->char_height + 2), y, obj->char_height, obj->char_height, corec.Res_LoadShader(Tex("commander_icon")));
					corec.GUI_DrawString(0, y, fmt("%s%s%s", 
												info->clan_id && cl_showClanAbbrevInUserlist.integer ? ST_GetState(clanInformation, fmt("c%ia", info->clan_id)) : "",
												info->clan_id && cl_showClanIconInUserlist.integer ? fmt("^clan %i^", info->clan_id) : "",
												info->name), obj->char_height, obj->char_height, 1, obj->width, corec.GetNicefontShader());
				}
				else
				{
					corec.GUI_DrawString(0, y, fmt("%s%s%s", 
											info->clan_id && cl_showClanAbbrevInUserlist.integer ? ST_GetState(clanInformation, fmt("c%ia", info->clan_id)) : "",
											info->clan_id && cl_showClanIconInUserlist.integer ? fmt("^clan %i^", info->clan_id) : "",
											info->name), obj->char_height, obj->char_height, 1, obj->width, corec.GetNicefontShader());
				}
				break;
			case MODE_PINGS:
				corec.GUI_DrawString(0, y, fmt("%i", score->ping), obj->char_height, obj->char_height, 1, obj->width, corec.GetNicefontShader());
				break;
			case MODE_SCORES:
				corec.GUI_DrawString(0, y, fmt("%i", (int)score->experience), obj->char_height, obj->char_height, 1, obj->width, corec.GetNicefontShader());
				break;
			case MODE_KILLS:
				corec.GUI_DrawString(0, y, fmt("%i", (int)score->kills), obj->char_height, obj->char_height, 1, obj->width, corec.GetNicefontShader());
				break;
			case MODE_DEATHS:
				corec.GUI_DrawString(0, y, fmt("%i", score->deaths), obj->char_height, obj->char_height, 1, obj->width, corec.GetNicefontShader());
				break;
		}
		y += obj->char_height+userlist->gap;

		i++;
	}
}

void	GUI_UserList_MouseDown(gui_element_t *obj, mouse_button_enum button, int x, int y)
{
	gui_userlist_t *userlist;
	int client;
	sharedClientInfo_t *info;
	
	userlist = corec.GUI_GetUserdata(class_name, obj);

	if (!userlist)
		return;

	client = userlist->order[y / (obj->char_height + userlist->gap)];

	info = &cl.clients[client].info;

	if (info->active && (info->team == userlist->team || userlist->team == -1))
	{
		corec.Console_DPrintf("user %i appears to be named '%s'\n", client, info->name);
		CL_ClearUnitSelection(&cl.potentialSelection);
		CL_ToggleUnitSelection(&cl.potentialSelection, client);
		CL_ProcessPotentialSelection();
	}
}

void	GUI_UserList_Param_Cmd(gui_element_t *obj, int argc, char *argv[])
{
	gui_userlist_t *userlist;

	if (argc < 1)
	{
		corec.Console_Printf("togglebutton param <panel:object> <var> <value>\n");
		corec.Console_Printf("valid parameters:\n");
		corec.Console_Printf("   team <number>        - the team to display users from (-1 will display users from all teams)\n");		
		corec.Console_Printf("   type <names|pings|scores|deaths|kills> - change whether this draws the names, pings, scores, or deaths\n");		
		corec.Console_Printf("   textcolor <r> <g> <b> - change the text color\n");
		corec.Console_Printf("   selected_textcolor <r> <g> <b> - change the selected text color\n");
		corec.Console_Printf("   gap <number>         - change the gap between vertical lines of the text\n");
		corec.Console_Printf("   interactive 		  - changes it so you can select players from the widget\n");
		corec.Console_Printf("   orderby <names|pings|scores|deaths|kills> - set the draw order\n");
		return;
	}

	userlist = corec.GUI_GetUserdata(class_name, obj);

	if (!userlist)
		return;

	if (strcmp(argv[0], "team")==0)
	{
		if (argc > 1)
			userlist->team = atoi(argv[1]);
	}
	else if (strcmp(argv[0], "orderby")==0)
	{
		if (argc > 1)
		{
			if (strcmp(argv[1], "names")==0)
				userlist->mode = MODE_NAMES;
			if (strcmp(argv[1], "pings")==0)
				userlist->mode = MODE_PINGS;
			if (strcmp(argv[1], "scores")==0)
				userlist->mode = MODE_SCORES;
			if (strcmp(argv[1], "deaths")==0)
				userlist->mode = MODE_DEATHS;
			if (strcmp(argv[1], "kills")==0)
				userlist->mode = MODE_KILLS;
		}
		else
		{
			corec.Console_Printf("You must specify the mode type out of: names, pings, scores\n");
		}
	}
	else if (strcmp(argv[0], "type")==0)
	{
		if (argc > 1)
		{
			if (strcmp(argv[1], "names")==0)
				userlist->mode = MODE_NAMES;
			if (strcmp(argv[1], "pings")==0)
				userlist->mode = MODE_PINGS;
			if (strcmp(argv[1], "scores")==0)
				userlist->mode = MODE_SCORES;
			if (strcmp(argv[1], "deaths")==0)
				userlist->mode = MODE_DEATHS;
			if (strcmp(argv[1], "kills")==0)
				userlist->mode = MODE_KILLS;
		}
		else
		{
			corec.Console_Printf("You must specify the mode type out of: names, pings, scores\n");
		}
	}
	else if (strcmp(argv[0], "selected_textcolor")==0)
	{
		if (argc > 3)
		{
			userlist->selected_textcolor[0] = atof(argv[1]);
			userlist->selected_textcolor[1] = atof(argv[2]);
			userlist->selected_textcolor[2] = atof(argv[3]);
		}
		else
		{
			corec.Console_Printf("You must specify the r g and b parameters\n");
		}
	}
	else if (strcmp(argv[0], "selected_bg")==0)
	{
		if (argc > 3)
		{
			userlist->selected_bg[0] = atof(argv[1]);
			userlist->selected_bg[1] = atof(argv[2]);
			userlist->selected_bg[2] = atof(argv[3]);
			userlist->selected_bg[3] = 1;
		}
		else
		{
			corec.Console_Printf("You must specify the r g and b parameters\n");
		}
	}
	else if (strcmp(argv[0], "gap")==0)
	{
		if (argc > 1)
		{
			userlist->gap = atoi(argv[1]);
		}
	}
	else if (strcmp(argv[0], "interactive")==0)
	{
		if (argc > 1)
		{
			if (stricmp(argv[1], "1")==0)
				userlist->element->interactive = true;
			else
				userlist->element->interactive = false;
		}
	}
}

//userlist "name" x y w h userlist ["text"]
void	*GUI_UserList_Create(gui_element_t *obj, int argc, char *argv[])
{
	gui_userlist_t *userlist;
	int i;

	if (argc < 5)
	{
		corec.Console_Printf("syntax: create userlist name x y width height\n");
		return NULL;
	}

	corec.GUI_SetClass(obj, class_name);
	corec.GUI_SetName(obj, argv[0]);
	corec.GUI_Move (obj, atoi(argv[1]), atoi(argv[2]) );
	corec.GUI_Resize (obj, atoi(argv[3]), atoi(argv[4]) );

	userlist = corec.GUI_Malloc(sizeof (gui_userlist_t));

	if (!userlist)
	{
		corec.Console_Printf("UserList error: couldn't alloc enough space to hold userlist\n");
		return NULL; 		
	}

	corec.GUI_SetUserdata(class_name, obj, userlist);

	userlist->team = 0;		//show team 0 by default
	userlist->mode = MODE_NAMES;	//draw names by default
	userlist->gap = 1;

	M_SetVec3(userlist->selected_textcolor, 0.3, 0.3, 0.3);
	userlist->selected_textcolor[3] = 1;
	
	M_SetVec3(userlist->selected_bg, 0, 0, 0);
	userlist->selected_bg[3] = 1;
	
	obj->draw = GUI_UserList_Draw;
	obj->param = GUI_UserList_Param_Cmd;
	obj->move = corec.GUI_Move;
	obj->mousedown = GUI_UserList_MouseDown;

	userlist->element = obj;
	userlist->sortColumn = MODE_SCORES;
	userlist->needsSorting = false;
	userlist->incremental = false;

	obj->interactive = false;

	for (i = 0; i < MAX_CLIENTS; i++)
		userlist->order[i] = i;
	
	return userlist;
}

void	GUI_UserList_Cmd(int argc, char *argv[])
{
	if (!argc)
	{
		corec.Console_Printf("userlist <command> <args>\n");
		corec.Console_Printf("  commands:\n");
		corec.Console_Printf("    list\n");
		return;
	}

	if (strcmp(argv[0], "list") == 0)
	{
		corec.GUI_List_Cmd(1, &class_name);
	}
}

void	GUI_UserList_Init()
{
	corec.Cmd_Register("userlist", GUI_UserList_Cmd);

	corec.GUI_RegisterClass(class_name, GUI_UserList_Create);
}

