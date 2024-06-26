#include "../TheGame/client_game.h"
#include "gui_map.h"

static char *class_name = "map";

#define BLIP_DOT_SIZE 16
#define BUILDING_DOT_SIZE 6
#define PEON_DOT_SIZE	3
#define PLAYER_DOT_SIZE	4
#define OFFICER_DOT_SIZE	12
#define SIEGE_DOT_SIZE	12

extern cvar_t	world_width;
extern cvar_t	world_height;

extern cvar_t	cl_cmdr_minimapRefresh;

/*==========================

  GUI_GetMapDotCoords

 ==========================*/

static bool	GUI_GetMapDotCoords(gui_element_t *widget, clientObject_t *object, float *x, float *y, float *w, float *h)
{
	if (IsBuildingType(object->base.type))
	{
		if (!CL_ObjectType(object->base.type)->isMine && !object->base.team)
			return false;

		*w = BUILDING_DOT_SIZE;
		*h = BUILDING_DOT_SIZE;
	}
	else if (IsCharacterType(object->base.type))
	{
		if (!object->base.team)
			return false;

		if (object->base.index >= MAX_CLIENTS)
		{
			*w = PEON_DOT_SIZE;
			*h = PEON_DOT_SIZE;
		}
		else
		{
			if (CMDR_UnitIsOfficer(object->base.index))
			{
				*w = OFFICER_DOT_SIZE;
				*h = OFFICER_DOT_SIZE;
			}
			else
			{
				*w = PLAYER_DOT_SIZE;
				*h = PLAYER_DOT_SIZE;
			}
		}
	}
	else if (CL_ObjectType(object->base.type)->isSiegeWeapon)
	{
		*w = SIEGE_DOT_SIZE;
		*h = SIEGE_DOT_SIZE;
		
	}
	else if (CL_ObjectType(object->base.type)->objclass == OBJCLASS_ITEM)
	{
		if (!object->base.team)
			return false;

		if (object->base.index >= MAX_CLIENTS)
		{
			*w = 2;
			*h = 2;
		}
	}
	else
		return false;

	*x = ((object->base.pos[X] / world_width.value) * widget->width) - (*w/2);
	*y = ((1 - (object->base.pos[Y] / world_height.value)) * widget->height) - (*h/2);

	return true;
}


/*==========================

  GUI_DrawMapViewBox

  Draws a polygon on the mini map that represents what the commander can currently see
  TODO: draw the side lines of the poly
  TODO: handle yaw and roll changes to the camera
  TODO: even those this is techically accurate, it's really fricken big... what might make it look better?

 ==========================*/

extern cvar_t	cl_cmdr_camtiltx;
extern cvar_t	cl_cmdr_camtilty;
extern cvar_t	cl_cmdr_camtiltz;

void	GUI_DrawMapViewBox(gui_element_t *widget)
{
	float	offset_x, offset_y;
	vec3_t	up = {0, 1, 0}, tmp;
	float	zoomdist, dist;
	float	top, bottom;
	float	topwidth, bottomwidth;

	//get the look at point on the widget
	offset_x = (cl.cmdrLookAtPoint[X] / world_width.value) * widget->width;
	offset_y = ((world_height.value - cl.cmdrLookAtPoint[Y]) / world_height.value) * widget->height;

	//draw a crosshair at the cameras lookat point
	corec.GUI_LineBox2d_S(offset_x - 4, offset_y, 8, 1, 1);
	corec.GUI_LineBox2d_S(offset_x, offset_y - 4, 1, 8, 1);

	//get distance from camera to lookat point
	M_SubVec3(cl.cmdrLookAtPoint, cl.camera.origin, tmp);
	zoomdist = M_GetVec3Length(tmp);

	////
	//top
	////
	
	//get real world y offset
	top = (sin(DEG2RAD(cl.camera.fovy / 2)) * zoomdist) / sin(DEG2RAD((180 - cl_cmdr_camtiltx.value) - (cl.camera.fovy / 2)));

	//get real world point and distance from camera
	M_MultVec3(up, -top, tmp);
	M_AddVec3(cl.cmdrLookAtPoint, tmp, tmp);
	M_SubVec3(cl.camera.origin, tmp, tmp);
	dist = M_GetVec3Length(tmp);

	//get width
	topwidth = (dist * sin(DEG2RAD(cl.camera.fovx / 2))) / sin(DEG2RAD(90 - (cl.camera.fovx / 2)));
	topwidth = (topwidth / world_width.value) * widget->width * 2;

	//adjust real world yoffset to widget offset
	top = (top / world_height.value) * widget->height;
	
	////
	//bottom
	////
	
	//get real world offset
	bottom = (sin(DEG2RAD(cl.camera.fovy / 2)) * zoomdist) / sin(DEG2RAD(cl_cmdr_camtiltx.value - (cl.camera.fovy / 2)));

	//get real world point and distance from camera
	M_MultVec3(up, bottom, tmp);
	M_AddVec3(cl.cmdrLookAtPoint, tmp, tmp);
	M_SubVec3(cl.camera.origin, tmp, tmp);
	dist = M_GetVec3Length(tmp);

	//get width
	bottomwidth = (dist * sin(DEG2RAD(cl.camera.fovx / 2))) / sin(DEG2RAD(90 - (cl.camera.fovx / 2)));
	bottomwidth = (bottomwidth / world_width.value) * widget->width * 2;

	//adjust real world yoffset to widget offset
	bottom = -(bottom / world_height.value) * widget->height;

	
	//draw it!
	if (offset_y + top >= 0)
	{
		float	under = MAX(0.0, offset_x - (topwidth / 2)) - (offset_x - (topwidth / 2));
		float	over = (offset_x + (topwidth / 2)) - MIN(widget->width, offset_x + (topwidth / 2));

		corec.GUI_LineBox2d_S(MAX(0, offset_x - (topwidth / 2)), offset_y + top, topwidth - under - over, 1, 1);

	}
	if (offset_y + bottom <= widget->height)
	{
		float	under = MAX(0.0, offset_x - (bottomwidth / 2)) - (offset_x - (bottomwidth / 2));
		float	over = (offset_x + (bottomwidth / 2)) - MIN(widget->width, offset_x + (bottomwidth / 2));

		corec.GUI_LineBox2d_S(MAX(0, offset_x - (bottomwidth / 2)), offset_y + bottom, bottomwidth - under - over, 1, 1);
	}
}


/*==========================

  GUI_DrawMap

 ==========================*/

void	GUI_DrawMap(gui_element_t *widget, int dummy_x, int dummy_y)
{
	gui_map_t	*map;
	float		width, height, offset_x, offset_y;
	residx_t	shader;
	int			obj, n;

	map = corec.GUI_GetUserdata(class_name, widget);
	if (!map)
		return;

	corec.GUI_SetRGBA(0.5, 0.5, 0.5, widget->alpha);

	//draw the map itself
	shader = corec.Res_LoadShader(corec.Cvar_GetString(map->bg_shader_cvar));
	corec.GUI_Quad2d_S(0, 0, widget->width, widget->height, shader);

	//populate the map with icons for active objects
	//TODO: track these in the widget and only update as often as the FOW
	//TODO: rather than setting a bunch of types of icons with params, each object just stores it's own icon
	for (obj = 0; obj < MAX_OBJECTS; obj++)
	{
		clientObject_t *object = &cl.objects[obj];
		shader = 0;

		if (!object->base.active
			|| object->base.flags & BASEOBJ_NO_RENDER)
			continue;

		if (CL_ObjectType(object->base.type)->objclass == OBJCLASS_WEAPON)
			continue;

		if (object->inFogOfWar)
			corec.GUI_SetRGBA(1,1,1,widget->alpha/2);
		else
			corec.GUI_SetRGBA(1,1,1,widget->alpha);

		if (!GUI_GetMapDotCoords(widget, object, &offset_x, &offset_y, &width, &height))
			continue;

		if (CL_ObjectType(object->base.type)->objclass == OBJCLASS_ITEM)
		{
			if (!CL_ObjectType(object->base.type)->race)
				continue;
			shader = corec.Res_LoadShader(CL_ObjectType(object->base.type)->mapIcon);
		}

		if (!shader)
		{
			if (object->base.team == cl.info->team)
			{
				if (object->base.index < MAX_CLIENTS)
				{
					if (CMDR_UnitIsOfficer(object->base.index))
						shader = map->friend_officer_shader;
					else if (CL_ObjectType(object->base.type)->isSiegeWeapon)
						shader = map->friend_seige_shader;
					else	
						shader = map->friend_player_shader;
				}
				else
					shader = map->friend_shader;
			}
			else
			{
				if (!cl.isCommander)
					continue;

				if (object->base.team == TEAM_UNDECIDED)
					shader = map->other_shader;
				else
					shader = map->enemy_shader;
			}
		}

		corec.GUI_Quad2d_S(offset_x, offset_y, width, height, shader);
	}
	
	//draw the FOW, everythin after this will be on top of it
	if (cl.gametime - cl.lastMinimapRefresh > cl_cmdr_minimapRefresh.integer)
	{
		map->fow_shader = corec.Res_GetDynamapShader(map->fow_shader);
		cl.lastMinimapRefresh = cl.gametime;
	}
	corec.GUI_Quad2d_S(0, 0, widget->width, widget->height, map->fow_shader);

	//draw notification blips
	for (n = 0; n < cl.noticeCount; n++)
	{
		clientObject_t *obj = &cl.objects[cl.noticeQueue[n].objnum];
		float	size = BLIP_DOT_SIZE + (BLIP_DOT_SIZE * (cl.noticeQueue[n].expireTime - cl.gametime)/cl_cmdrNoticePersistTime.value);

		offset_x = ((obj->visual.pos[X] / world_width.value) * widget->width) - (size/2);
		offset_y = ((1 - (obj->visual.pos[Y] / world_height.value)) * widget->height) - (size/2);
		corec.GUI_Quad2d_S(offset_x, offset_y, size, size, map->blip_shader);
	}

	//draw the view box
	corec.GUI_SetRGBA(1,1,1,1);
	GUI_DrawMapViewBox(widget);
}

void	GUI_MoveMap(gui_element_t *obj, int x, int y)
{
	corec.GUI_Move(obj, x, y);
}

void GUI_Map_Param_Cmd(gui_element_t *obj, int argc, char *argv[])
{
	gui_map_t *map;
	char filename[256];

	if (argc < 1)
	{
		corec.Console_Printf("map param <panel:object> <var> <value>\n");
		corec.Console_Printf("valid parameters:\n");
		corec.Console_Printf("   friend_image <image>  - sets the icon to use for friendly objects\n");
		corec.Console_Printf("   friend_player_image <image> - sets the icon to use for friendly objects\n");
		corec.Console_Printf("   friend_officer_image <image> - sets the icon to use for friendly objects\n");
		corec.Console_Printf("   friend_seige_image <image> - sets the icon to use for friendly objects\n");
		corec.Console_Printf("   enemy_image <image>   - sets the icon to use for enemy objects\n");
		corec.Console_Printf("   enemy_seige_image <image>   - sets the icon to use for enemy objects\n");
		corec.Console_Printf("   other_image <image>   - sets the icon to use for neutral objects\n");
		corec.Console_Printf("   bg_cvar <cvar_name>   - sets the name of the cvar that specifies the background image\n");
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
	else if (strcmp(argv[0], "blip_image") == 0)
	{
		if (argc > 1)
		{
			BPrintf(filename, 255, "%s%s", UI_PREFIX, argv[1]);
			filename[255] = 0;
			map->blip_shader = corec.Res_LoadShaderEx(filename, SHD_NO_MIPMAPS | SHD_FULL_QUALITY);
		} else {
			corec.Console_Printf("Not enough parameters.  You must specify the image filename\n");
		}
	}
	if (strcmp(argv[0], "bg_cvar") == 0)
	{
		if (argc > 1)
		{
			strncpySafe(map->bg_shader_cvar, argv[1], sizeof(map->bg_shader_cvar));
		} else {
			corec.Console_Printf("Not enough parameters.  You must specify the cvar name\n");
		}
	}
}

void	GUI_Map_MouseDown(gui_element_t *obj, mouse_button_enum button, int x, int y)
{
	vec3_t pos;

	if (button == MOUSE_RBUTTON)
	{
		pos[0] = ((float)x / obj->width) * cl.worldbounds[0];
		pos[1] = ((float)(obj->height - y) / obj->height) * cl.worldbounds[1];

		CL_SendLocationGoalToSelection(GOAL_REACH_WAYPOINT, pos);
	}
}

void	GUI_Map_MouseOver(gui_element_t *obj, int mx, int my)
{
	gui_map_t *map;
	
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
}

//map "name" x y w h
void	*GUI_Map_Create(gui_element_t *obj, int argc, char *argv[])
{
	gui_map_t *map;

	if (argc < 4)
	{
		corec.Console_Printf("syntax: map name x y w h\n");
		return NULL;
	}

	corec.GUI_SetClass(obj, class_name);
	corec.GUI_SetName(obj, argv[0]);
	corec.GUI_Move (obj, atoi(argv[1]), atoi(argv[2]) );
	corec.GUI_Resize (obj, atoi(argv[3]), atoi(argv[4]) );

	map = corec.GUI_Malloc(sizeof (gui_map_t));

	if (!map)
	{
		corec.Console_Printf("Map error: couldn't enough space to hold map\n");
		return NULL; 		
	}

	corec.GUI_SetUserdata(class_name, obj, map);
	map->element = obj;

	obj->interactive = true;
	obj->draw = GUI_DrawMap;
	obj->move = GUI_MoveMap;
	obj->param = GUI_Map_Param_Cmd;
	obj->mouseover = GUI_Map_MouseOver;
	obj->mousedown = GUI_Map_MouseDown;

	map->fow_shader = 0;
	map->friend_shader = 0;
	map->friend_player_shader = 0;
	map->enemy_shader = 0;
	map->other_shader = 0;
	map->blip_shader = 0;	

	map->parent = NULL;

	return map;
}

void	GUI_Map_Cmd(int argc, char *argv[])
{
	gui_element_t *obj;

	if (!argc)
	{
		corec.Console_Printf("map <command> <args>\n");
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
			GUI_Map_Param_Cmd(obj, argc -2, &argv[2]);
		}
	}
}

void	GUI_Map_Init()
{
	corec.Cmd_Register("map", GUI_Map_Cmd);

	corec.GUI_RegisterClass(class_name, GUI_Map_Create);
}
