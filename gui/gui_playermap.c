#include "../TheGame/client_game.h"
#include "gui_playermap.h"

static char *class_name = "playermap";

#define MIN_DOT_SIZE 6
#define MIN_WORKER_DOT_SIZE 1
#define BLIP_DOT_SIZE 16
#define PLAYER_ICON_SIZE 24
#define SIEGE_ICON_SIZE 24
#define OFFICER_ICON_SIZE 16
#define SPAWNPOINT_ICON_SIZE 32
#define WAYPOINT_ICON_SIZE 32

static void	GUI_GetMapDotCoords(gui_element_t *widget, clientObject_t *object, float world_width, float world_height, float *x, float *y, float *w, float *h)
{
	float minsize = MIN_DOT_SIZE;
	if (IsWorkerType(object->base.type))
	{
		minsize = MIN_WORKER_DOT_SIZE;
	}
	*w = MAX(minsize, (object->base.bmax[X] - object->base.bmin[X]) / world_width  * widget->width);
	*h = MAX(minsize, (object->base.bmax[Y] - object->base.bmin[Y]) / world_height * widget->height);
	*x = ((object->base.pos[X] / world_width) * widget->width) - (*w/2);
	*y = ((1 - (object->base.pos[Y] / world_height)) * widget->height) - (*h/2);
}

void	DrawBlip(gui_playermap_t *map, clientObject_t *object, float world_width, float world_height)
{
	gui_element_t *widget = map->element;
	float width, height, offset_x, offset_y;
	residx_t shader = 0;

	GUI_GetMapDotCoords(map->element, object, world_width, world_height, &offset_x, &offset_y, &width, &height);

	if (cl.info && object->base.team == cl.info->team)
	{
		if (IsBuildingType(object->base.type) && 
			(CL_ObjectType(object->base.type)->spawnFrom || 
			CL_ObjectType(object->base.type)->commandCenter) && 
			!(object->base.flags & BASEOBJ_UNDER_CONSTRUCTION) && 
			map->spawnselect)
		{
			objectData_t *def = CL_ObjectType(object->base.type);
			/*if (def->commandCenter)
				shader = map->commandcenter_shader;
			else if (def->spawnFrom && def->canEnter)
				shader = map->garrison_shader;
			else if (def->spawnFrom)
				shader = map->spawnpoint_shader;*/
			shader = corec.Res_LoadShader(def->mapIcon);

			width = SPAWNPOINT_ICON_SIZE;
			height = SPAWNPOINT_ICON_SIZE;
			offset_x = ((object->base.pos[X] / world_width) * widget->width) - (width/2);
			offset_y = ((1 - (object->base.pos[Y] / world_height)) * widget->height) - (height/2);
		}
		else if (object->base.index == cl.clientnum)
		{
			shader = map->player_shader;
			width = PLAYER_ICON_SIZE;
			height = PLAYER_ICON_SIZE;
			offset_x = ((object->base.pos[X] / world_width) * widget->width) - (width/2);
			offset_y = ((1 - (object->base.pos[Y] / world_height)) * widget->height) - (height/2);

	        corec.GUI_RotatedQuad2d_S(offset_x, offset_y, 
					   width, height, object->visual.angle[2], shader);
//					corec.GUI_Quad2d_S(offset_x, offset_y, 
//					   width, height, shader);

			return;
		}
		else
		{
			if (object->base.index < MAX_CLIENTS)
			{
				if (CL_ObjectType(object->base.type)->isSiegeWeapon)
				{
					shader = map->friend_seige_shader;
					width = SIEGE_ICON_SIZE;
					height = SIEGE_ICON_SIZE;
					offset_x = ((object->base.pos[X] / world_width) * widget->width) - (width/2);
					offset_y = ((1 - (object->base.pos[Y] / world_height)) * widget->height) - (height/2);
				}
				else if (CMDR_UnitIsOfficer(object->base.index))
				{
					shader = map->friend_officer_shader;
					width = OFFICER_ICON_SIZE;
					height = OFFICER_ICON_SIZE;
					offset_x = ((object->base.pos[X] / world_width) * widget->width) - (width/2);
					offset_y = ((1 - (object->base.pos[Y] / world_height)) * widget->height) - (height/2);
				}
				else
					shader = map->friend_player_shader;
			}
			else
			{
				shader = map->friend_shader;
			}
		}
	}
	else if (object->base.team == TEAM_UNDECIDED)
	{
   		//don't draw npcs
		//shader = map->other_shader;
		return;
	}
	else
    {
   		//draw enemies that are revealed by an electric eye
		if (CL_ObjectType(object->base.type)->isSiegeWeapon)
		{
			shader = map->enemy_seige_shader;
			width = SIEGE_ICON_SIZE;
			height = SIEGE_ICON_SIZE;
			offset_x = ((object->base.pos[X] / world_width) * widget->width) - (width/2);
			offset_y = ((1 - (object->base.pos[Y] / world_height)) * widget->height) - (height/2);
		}
		else if (CMDR_UnitIsOfficer(object->base.index))
		{
			shader = map->enemy_officer_shader;
			width = OFFICER_ICON_SIZE;
			height = OFFICER_ICON_SIZE;
			offset_x = ((object->base.pos[X] / world_width) * widget->width) - (width/2);
			offset_y = ((1 - (object->base.pos[Y] / world_height)) * widget->height) - (height/2);
		}
		else
			shader = map->enemy_shader;
    }

	//offset_y = (1 - ((object->base.pos[Y] - (height/2)) / world_height)) * widget->height;
		
	corec.GUI_Quad2d_S(offset_x, offset_y, width, height, shader);
}

void	GUI_DrawPlayerMap(gui_element_t *widget, int dummy_x, int dummy_y)
{
	gui_playermap_t *map;
	int obj, enemyObj, n;
	vec3_t bmin, bmax;
	float world_width, world_height, dist, viewDistance;
	float offset_x, offset_y, width, height;
	waypoint_t *waypoint = &cl.clients[cl.clientnum].waypoint;
	residx_t shader;

	corec.World_GetBounds(bmin, bmax);
	world_width = bmax[X] - bmin[X];
	world_height = bmax[Y] - bmin[Y];

	map = corec.GUI_GetUserdata(class_name, widget);
	if (!map)
		return;

	corec.GUI_SetRGBA(0.5,0.5,0.5,widget->alpha);

	shader = corec.Res_LoadShader(corec.Cvar_GetString(map->bg_shader_cvar));
	corec.GUI_Quad2d_S(0, 0, widget->width, widget->height, shader);

	corec.GUI_SetRGBA(1,1,1,widget->alpha);

	for (obj = 0; obj < MAX_OBJECTS; obj++)
	{
		clientObject_t *object = &cl.objects[obj];
		if (!object->base.active
			|| object->base.health <= 0
			|| object->base.flags & BASEOBJ_NO_RENDER
			|| object->base.team != cl.info->team
			|| IsWeaponType(object->base.type))
			continue;

		//we'll draw ourselves last
		if (obj == cl.clientnum)
			continue;

		DrawBlip(map, object, world_width, world_height);

		if (CL_ObjectType(cl.objects[obj].base.type)->revealHidden)
		{

			for (enemyObj = 0; enemyObj < MAX_OBJECTS; enemyObj++)
			{
				clientObject_t *enemy = &cl.objects[enemyObj];
				if (!enemy->base.active
					|| enemy->base.health <= 0
					|| enemy->base.flags & BASEOBJ_NO_RENDER
					|| enemy->base.team == cl.info->team
					|| enemy->base.team == TEAM_UNDECIDED
					|| IsWeaponType(enemy->base.type))
					continue;
				
				viewDistance = CL_ObjectType(cl.objects[obj].base.type)->viewDistance;
				viewDistance = viewDistance * viewDistance;
				dist = M_GetDistanceSq(enemy->base.pos, object->base.pos);
				if (dist < viewDistance)
				{
					DrawBlip(map, enemy, world_width, world_height);
				}
			}
		}
	}

    for (n = 0; n < cl.noticeCount; n++)
	{
		vec3_t bmin, bmax;
		float world_width, world_height;
		clientObject_t *obj = &cl.objects[cl.noticeQueue[n].objnum];
		float   size = BLIP_DOT_SIZE + (BLIP_DOT_SIZE * (cl.noticeQueue[n].expireTime - cl.gametime)/cl_cmdrNoticePersistTime.value);
		
		if (obj->base.team == TEAM_UNDECIDED)
			continue;
		
		if (cl.noticeQueue[n].expireTime < cl.gametime)
			continue;
		
		corec.World_GetBounds(bmin, bmax);
		world_width = bmax[X] - bmin[X];
		world_height = bmax[Y] - bmin[Y];
		
		offset_x = ((obj->visual.pos[X] / world_width) * widget->width) - (size/2);
		offset_y = ((1 - (obj->visual.pos[Y] / world_height)) * widget->height) - (size/2);

		if (cl.noticeQueue[n].noticeType == NOTICE_UNDER_ATTACK)
		{
			corec.GUI_Quad2d_S(offset_x, offset_y, size, size, map->underattack_shader);
		}
		else if (cl.noticeQueue[n].noticeType == NOTICE_BUILDING_COMPLETE)
		{
			corec.GUI_Quad2d_S(offset_x, offset_y, size, size, map->buildingcomplete_shader);
		}
	}
	
	if (waypoint->active && !map->spawnselect)
	{
		float *pos = waypoint->object ? cl.objects[waypoint->object_index].visual.pos : waypoint->pos;
		width = WAYPOINT_ICON_SIZE;
		height = WAYPOINT_ICON_SIZE;
		offset_x = (((pos[X]) / world_width) * widget->width) - width/2;
		offset_y = ((1 - (pos[Y] / world_height)) * widget->height) - height/2;

		corec.GUI_Quad2d_S(offset_x, offset_y, 
						   width, height, map->waypoint_shader);
	}
	
	//draw ourself, on top of everything
	if (cl.objects[cl.clientnum].base.active)
		DrawBlip(map, &cl.objects[cl.clientnum], world_width, world_height);

	corec.GUI_SetRGBA(1,1,1,1);

}

void	GUI_MovePlayerMap(gui_element_t *obj, int x, int y)
{
	corec.GUI_Move(obj, x, y);
}

void GUI_PlayerMap_Param_Cmd(gui_element_t *obj, int argc, char *argv[])
{
	gui_playermap_t *map;
	char filename[256];

	if (argc < 1)
	{
		corec.Console_Printf("playermap param <panel:object> <var> <value>\n");
		corec.Console_Printf("valid parameters:\n");
		corec.Console_Printf("   friend_image <image>  - sets the icon to use for friendly objects\n");
		corec.Console_Printf("   friend_player_image <image>   - sets the icon to use for friendly players\n");
		corec.Console_Printf("   friend_seige_image <image>   - sets the icon to use for friendly seige weapons\n");
		corec.Console_Printf("   friend_officer_image <image>   - sets the icon to use for friendly officers\n");
		corec.Console_Printf("   enemy_image <image>   - sets the icon to use for enemy objects\n");
		corec.Console_Printf("   enemy_seige_image <image>   - sets the icon to use for enemy seige weapons\n");
		corec.Console_Printf("   enemy_officer_image <image>   - sets the icon to use for enemy officers\n");
		corec.Console_Printf("   other_image <image>   - sets the icon to use for neutral objects\n");
		corec.Console_Printf("	 spawnpoint_image <image> - sets the icon to use for a spawnpoint\n");
		corec.Console_Printf("	 commandcenter_image <image> - sets the icon to use for the command center\n");
		corec.Console_Printf("	 waypoint_image <image> - sets the icon to use for a waypoint\n");
		corec.Console_Printf("	 player_image <image>  - sets the icon to use for the player\n");
		corec.Console_Printf("   underattack_image <image> - sets the icon to use for a building under attack\n");
		corec.Console_Printf("   buildingcomplete_image <image> - sets the icon to use for a building construction complete image\n");
		corec.Console_Printf("   bg_cvar <cvar_name>   - sets the name of the cvar that specifies the background image\n");
		corec.Console_Printf("	 spawnselect <1|0>\n");
		return;
	}

	map = corec.GUI_GetUserdata(class_name, obj);

	if (!map)
		return;

	if (strcmp(argv[0], "friend_image") == 0)
	{
		if (argc > 1)
		{
			BPrintf(filename, 255, "%s%s", UI_PREFIX, argv[1]);
			filename[255] = 0;
			map->friend_shader = corec.Res_LoadShaderEx(filename, SHD_NO_MIPMAPS | SHD_FULL_QUALITY);
		} else {
			corec.Console_Printf("Not enough parameters.  You must specify the image filename\n");
		}
	}
	else if (strcmp(argv[0], "player_image") == 0)
	{
		if (argc > 1)
		{
			BPrintf(filename, 255, "%s%s", UI_PREFIX, argv[1]);
			filename[255] = 0;
			map->player_shader = corec.Res_LoadShaderEx(filename, SHD_NO_MIPMAPS | SHD_FULL_QUALITY);
		} else {
			corec.Console_Printf("Not enough parameters.  You must specify the image filename\n");
		}
	}
	else if (strcmp(argv[0], "friend_player_image") == 0)
	{
		if (argc > 1)
		{
			BPrintf(filename, 255, "%s%s", UI_PREFIX, argv[1]);
			filename[255] = 0;
			map->friend_player_shader = corec.Res_LoadShaderEx(filename, SHD_NO_MIPMAPS | SHD_FULL_QUALITY);
		} else {
			corec.Console_Printf("Not enough parameters.  You must specify the image filename\n");
		}
	}
	else if (strcmp(argv[0], "friend_officer_image") == 0)
	{
		if (argc > 1)
		{
			BPrintf(filename, 255, "%s%s", UI_PREFIX, argv[1]);
			filename[255] = 0;
			map->friend_officer_shader = corec.Res_LoadShaderEx(filename, SHD_NO_MIPMAPS | SHD_FULL_QUALITY);
		} else {
			corec.Console_Printf("Not enough parameters.  You must specify the image filename\n");
		}
	}
	else if (strcmp(argv[0], "friend_seige_image") == 0)
	{
		if (argc > 1)
		{
			BPrintf(filename, 255, "%s%s", UI_PREFIX, argv[1]);
			filename[255] = 0;
			map->friend_seige_shader = corec.Res_LoadShaderEx(filename, SHD_NO_MIPMAPS | SHD_FULL_QUALITY);
		} else {
			corec.Console_Printf("Not enough parameters.  You must specify the image filename\n");
		}
	}
	else if (strcmp(argv[0], "enemy_image") == 0)
	{
		if (argc > 1)
		{
			BPrintf(filename, 255, "%s%s", UI_PREFIX, argv[1]);
			filename[255] = 0;
			map->enemy_shader = corec.Res_LoadShaderEx(filename, SHD_NO_MIPMAPS | SHD_FULL_QUALITY);
		} else {
			corec.Console_Printf("Not enough parameters.  You must specify the image filename\n");
		}
	}
	else if (strcmp(argv[0], "enemy_officer_image") == 0)
	{
		if (argc > 1)
		{
			BPrintf(filename, 255, "%s%s", UI_PREFIX, argv[1]);
			filename[255] = 0;
			map->enemy_officer_shader = corec.Res_LoadShaderEx(filename, SHD_NO_MIPMAPS | SHD_FULL_QUALITY);
		} else {
			corec.Console_Printf("Not enough parameters.  You must specify the image filename\n");
		}
	}
	else if (strcmp(argv[0], "enemy_seige_image") == 0)
	{
		if (argc > 1)
		{
			BPrintf(filename, 255, "%s%s", UI_PREFIX, argv[1]);
			filename[255] = 0;
			map->enemy_seige_shader = corec.Res_LoadShaderEx(filename, SHD_NO_MIPMAPS | SHD_FULL_QUALITY);
		} else {
			corec.Console_Printf("Not enough parameters.  You must specify the image filename\n");
		}
	}
	else if (strcmp(argv[0], "other_image") == 0)
	{
		if (argc > 1)
		{
			BPrintf(filename, 255, "%s%s", UI_PREFIX, argv[1]);
			filename[255] = 0;
			map->other_shader = corec.Res_LoadShaderEx(filename, SHD_NO_MIPMAPS | SHD_FULL_QUALITY);
		} else {
			corec.Console_Printf("Not enough parameters.  You must specify the image filename\n");
		}
	}
	else if (strcmp(argv[0], "spawnpoint_image") == 0)
	{
		if (argc > 1)
		{
			BPrintf(filename, 255, "%s%s", UI_PREFIX, argv[1]);
			filename[255] = 0;
			map->spawnpoint_shader = corec.Res_LoadShaderEx(filename, SHD_NO_MIPMAPS | SHD_FULL_QUALITY);
		} else {
			corec.Console_Printf("Not enough parameters.  You must specify the image filename\n");
		}
	}
	else if (strcmp(argv[0], "commandcenter_image") == 0)
	{
		if (argc > 1)
		{
			BPrintf(filename, 255, "%s%s", UI_PREFIX, argv[1]);
			filename[255] = 0;
			map->commandcenter_shader = corec.Res_LoadShaderEx(filename, SHD_NO_MIPMAPS | SHD_FULL_QUALITY);
		} else {
			corec.Console_Printf("Not enough parameters.  You must specify the image filename\n");
		}
	}
	else if (strcmp(argv[0], "garrison_image") == 0)
	{
		if (argc > 1)
		{
			BPrintf(filename, 255, "%s%s", UI_PREFIX, argv[1]);
			filename[255] = 0;
			map->garrison_shader = corec.Res_LoadShaderEx(filename, SHD_NO_MIPMAPS | SHD_FULL_QUALITY);
		} else {
			corec.Console_Printf("Not enough parameters.  You must specify the image filename\n");
		}	
	}
	else if (strcmp(argv[0], "waypoint_image") == 0)
	{
		if (argc > 1)
		{
			BPrintf(filename, 255, "%s%s", UI_PREFIX, argv[1]);
			filename[255] = 0;
			map->waypoint_shader = corec.Res_LoadShaderEx(filename, SHD_NO_MIPMAPS | SHD_FULL_QUALITY);
		} else {
			corec.Console_Printf("Not enough parameters.  You must specify the image filename\n");
		}
	}
	else if (strcmp(argv[0], "underattack_image") == 0)
	{
		if (argc > 1)
		{
			BPrintf(filename, 255, "%s%s", UI_PREFIX, argv[1]);
			filename[255] = 0;
			map->underattack_shader = corec.Res_LoadShaderEx(filename, SHD_NO_MIPMAPS | SHD_FULL_QUALITY);
		} else {
			corec.Console_Printf("Not enough parameters.  You must specify the image filename\n");
		}
	}
	else if (strcmp(argv[0], "buildingcomplete_image") == 0)
	{
		if (argc > 1)
		{
			BPrintf(filename, 255, "%s%s", UI_PREFIX, argv[1]);
			filename[255] = 0;
			map->buildingcomplete_shader = corec.Res_LoadShaderEx(filename, SHD_NO_MIPMAPS | SHD_FULL_QUALITY);
		} else {
			corec.Console_Printf("Not enough parameters.  You must specify the image filename\n");
		}
	}
	else if (strcmp(argv[0], "bg_cvar") == 0)
	{
		if (argc > 1)
		{
			strncpySafe(map->bg_shader_cvar, argv[1], sizeof(map->bg_shader_cvar));
		} else {
			corec.Console_Printf("Not enough parameters.  You must specify the cvar name\n");
		}
	}
	else if (strcmp(argv[0], "spawnselect") == 0)
	{
		if (argc > 1)
		{
			map->spawnselect = atoi(argv[1]);
		}
	}
}

clientObject_t *GUI_GetSelectedSpawnPoint(gui_element_t *widget, int mx, int my)
{
	clientObject_t *spawn;
	vec3_t bmin, bmax;
	float world_width, world_height;

	corec.World_GetBounds(bmin, bmax);
	world_width = bmax[X] - bmin[X];
	world_height = bmax[Y] - bmin[Y];

	for (spawn = cl.spawnList; spawn; spawn = spawn->nextSpawnPoint)
	{
		float x,y,w,h;

		w = SPAWNPOINT_ICON_SIZE;
		h = SPAWNPOINT_ICON_SIZE;
		x = ((spawn->base.pos[X] / world_width) * widget->width) - (w/2);
		y = ((1 - (spawn->base.pos[Y] / world_height)) * widget->height) - (h/2);

		if (mx >= x && mx <= x+w)
		{
			if (my >= y && my <= y+h)
			{
				return spawn;
			}
		}
	}

	return NULL;
}

void	GUI_PlayerMap_MouseOver(gui_element_t *obj, int mx, int my)
{
	gui_playermap_t *map;
	
	map = corec.GUI_GetUserdata(class_name, obj);

	if (!map)
		return;

	if (cl.isCommander)
	{
		if (corec.Input_IsKeyDown(KEY_LBUTTON))
		{
			cl.cmdrLookAtPoint[0] = ((float)mx / obj->width) * cl.worldbounds[0];
			cl.cmdrLookAtPoint[1] = ((float)(obj->height - my) / obj->height) * cl.worldbounds[1];
		}
	}
	else
	{		
		if (map->spawnselect)
		{
			map->selectedSpawn = GUI_GetSelectedSpawnPoint(obj, mx, my);
		}
	}
}

void	GUI_PlayerMap_MouseDown(gui_element_t *obj, mouse_button_enum button, int x, int y)
{
	clientObject_t *spawn = GUI_GetSelectedSpawnPoint(obj, x, y);

	if (spawn)
	{
		corec.Cmd_Exec(fmt("spawnfrom %i", spawn->base.index));
	}
}

//map "name" x y w h
void	*GUI_PlayerMap_Create(gui_element_t *obj, int argc, char *argv[])
{
	gui_playermap_t *map;

	if (argc < 4)
	{
		corec.Console_Printf("syntax: playermap name x y w h\n");
		return NULL;
	}

	corec.GUI_SetClass(obj, class_name);
	corec.GUI_SetName(obj, argv[0]);
	corec.GUI_Move (obj, atoi(argv[1]), atoi(argv[2]) );
	corec.GUI_Resize (obj, atoi(argv[3]), atoi(argv[4]) );

	map = corec.GUI_Malloc(sizeof (gui_playermap_t));

	if (!map)
	{
		corec.Console_Printf("Map error: couldn't enough space to hold map\n");
		return NULL; 		
	}

	corec.GUI_SetUserdata(class_name, obj, map);
	map->element = obj;

	obj->interactive = true;
	obj->draw = GUI_DrawPlayerMap;
	obj->move = GUI_MovePlayerMap;
	obj->param = GUI_PlayerMap_Param_Cmd;
	obj->mouseover = GUI_PlayerMap_MouseOver;
	obj->mousedown = GUI_PlayerMap_MouseDown;

	map->friend_shader = 0;
	map->friend_player_shader = 0;
	map->enemy_shader = 0;
	map->other_shader = 0;
	map->waypoint_shader = 0;

	map->parent = NULL;

	return map;
}

void	GUI_PlayerMap_Cmd(int argc, char *argv[])
{
	gui_element_t *obj;

	if (!argc)
	{
		corec.Console_Printf("playermap <command> <args>\n");
		corec.Console_Printf("  commands:\n");
		corec.Console_Printf("    list\n");
		corec.Console_Printf("    param\n");
		return;
	}

	if (strcmp(argv[0], "list") == 0)
	{
		corec.GUI_List_Cmd(1, &class_name);
	} 
	else if (strcmp(argv[0], "param") == 0)
	{
		if (argc > 1)
		{
			obj = corec.GUI_GetObject(argv[1]);
			if (!obj)
				return;
			GUI_PlayerMap_Param_Cmd(obj, argc -2, &argv[2]);
		}
	}
}

void	GUI_PlayerMap_Init()
{
	corec.Cmd_Register("playermap", GUI_PlayerMap_Cmd);

	corec.GUI_RegisterClass(class_name, GUI_PlayerMap_Create);
}
