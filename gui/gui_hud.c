#include "../TheGame/client_game.h"
#include "gui_hud.h"

static char *class_name = "hud";

void	GUI_DrawHUD(gui_element_t *widget, int dummy_x, int dummy_y)
{
	gui_hud_t *hud;
	int x, y, obj, width, height, offset_x, offset_y, offset = -1, objType = -1;
	int cell_x, cell_y;
	vec2_t topleft, bottomright;
	residx_t shader;

	hud = corec.GUI_GetUserdata(class_name, widget);
	if (!hud)
		return;

	corec.GUI_SetRGBA(1,1,1,widget->alpha);

	

	for (y = 0; y < hud->num_cells_down; y++)
	{
		for (x = 0; x < hud->num_cells_across; x++)
		{
			cell_x = hud->center[X] + x - hud->num_cells_across/2;
			cell_y = hud->center[Y] + y - hud->num_cells_down/2;
			corec.WOG_GetGridSquareCoords(cell_x,
										  cell_y,
										  topleft, bottomright);

			corec.GUI_LineBox2d_S(x * hud->cell_width,
							      y * hud->cell_height,
							      hud->cell_width, 
							      hud->cell_height, 
							      1);

			obj = 0;
			offset = -1;
			while (obj >= 0)
			{
				obj = corec.WOG_GetNextObjectInGridSquare(&objType, 1, 
														  cell_x,
														  cell_y,
														  &offset);	

				if (obj < 0)
					break;
				
				if (cl.objects[obj].base.team == cl.team)
					shader = hud->friend_shader;
				else if (cl.objects[obj].base.team == TEAM_UNDECIDED)
					shader = hud->other_shader;
				else
					shader = hud->enemy_shader;

				width = MAX(2, (cl.objects[obj].base.bmax[X] + cl.objects[obj].base.bmin[X])  / hud->wc_cell_size[X]);
				height = MAX(2, (cl.objects[obj].base.bmax[Y] + cl.objects[obj].base.bmin[Y]) / hud->wc_cell_size[Y]);
				offset_x = (cl.objects[obj].base.pos[X] + cl.objects[obj].base.bmin[X] - topleft[X]) 
							/ hud->wc_cell_size[X] * hud->cell_width;
				offset_y = hud->cell_height - ((cl.objects[obj].base.pos[Y] + cl.objects[obj].base.bmin[Y] - topleft[Y]) 
							/ hud->wc_cell_size[Y] * hud->cell_height);
				
				corec.GUI_Quad2d_S(hud->cell_width * x + offset_x, 
								   hud->cell_height * y + offset_y, 
								   width, height, shader);

				offset++;
			}

		}
	}
	
	corec.GUI_SetRGBA(1,1,1,1);
}

void	GUI_HUD_Center(gui_element_t *obj, vec3_t center)
{
	gui_hud_t *hud;

	hud = corec.GUI_GetUserdata(class_name, obj);
	
	if (!hud)
		return;
	
	corec.WOG_GetGridSquareForPos(center, hud->center);
}

void	GUI_HUD_SetCells(gui_element_t *obj, int h_cells, int v_cells)
{
	gui_hud_t *hud;
	vec2_t bottomright, topleft;

	hud = corec.GUI_GetUserdata(class_name, obj);
	
	if (!hud)
		return;
	
	hud->num_cells_across = h_cells;
	hud->num_cells_down = v_cells;

	hud->cell_width = obj->width / h_cells;
	hud->cell_height = obj->height / v_cells;
	
	corec.WOG_GetGridSquareCoords(hud->center[X], hud->center[Y], topleft, bottomright);
	
	hud->wc_cell_size[X] = bottomright[X] - topleft[X];
	hud->wc_cell_size[Y] = bottomright[Y] - topleft[Y];
}

void	GUI_MoveHUD(gui_element_t *obj, int x, int y)
{
	corec.GUI_Move(obj, x, y);
}

void GUI_HUD_Param_Cmd(gui_element_t *obj, int argc, char *argv[])
{
	gui_hud_t *hud;
	char filename[256];

	if (argc < 1)
	{
		corec.Console_Printf("hud param <panel:object> <var> <value>\n");
		corec.Console_Printf("valid parameters:\n");
		corec.Console_Printf("   cells_horiz <num>     - changes the number of cells across to show\n");
		corec.Console_Printf("   cells_vert <num>      - changes the number of cells down to show\n");
		corec.Console_Printf("   friend_image <image>  - sets the icon to use for friendly objects\n");
		corec.Console_Printf("   enemy_image <image>   - sets the icon to use for enemy objects\n");
		corec.Console_Printf("   other_image <image>   - sets the icon to use for neutral objects\n");
		return;
	}

	hud = corec.GUI_GetUserdata(class_name, obj);

	if (!hud)
		return;

	if (strcmp(argv[0], "friend_image") == 0)
	{
		if (argc > 1)
		{
			BPrintf(filename, 255, "%s%s", UI_PREFIX, argv[1]);
			filename[255] = 0;
			hud->friend_shader = corec.Res_LoadShaderEx(filename, SHD_NO_MIPMAPS | SHD_FULL_QUALITY);
		} else {
			corec.Console_Printf("Not enough parameters.  You must specify the image filename\n");
		}
	}
	if (strcmp(argv[0], "enemy_image") == 0)
	{
		if (argc > 1)
		{
			BPrintf(filename, 255, "%s%s", UI_PREFIX, argv[1]);
			filename[255] = 0;
			hud->enemy_shader = corec.Res_LoadShaderEx(filename, SHD_NO_MIPMAPS | SHD_FULL_QUALITY);
		} else {
			corec.Console_Printf("Not enough parameters.  You must specify the image filename\n");
		}
	}
	if (strcmp(argv[0], "other_image") == 0)
	{
		if (argc > 1)
		{
			BPrintf(filename, 255, "%s%s", UI_PREFIX, argv[1]);
			filename[255] = 0;
			hud->other_shader = corec.Res_LoadShaderEx(filename, SHD_NO_MIPMAPS | SHD_FULL_QUALITY);
		} else {
			corec.Console_Printf("Not enough parameters.  You must specify the image filename\n");
		}
	}
	else if (strcmp(argv[0], "cells_horiz") == 0)
	{
		if (argc > 1)
		{
			GUI_HUD_SetCells(hud->element, atoi(argv[1]), hud->num_cells_down);
		} else {
			corec.Console_Printf("Not enough parameters.  You must specify the number of horizontal cells to show.\n");
		}
	}
	else if (strcmp(argv[0], "cells_vert") == 0)
	{
		if (argc > 1)
		{
			GUI_HUD_SetCells(hud->element, hud->num_cells_across, atoi(argv[1]));
		} else {
			corec.Console_Printf("Not enough parameters.  You must specify the number of vertical cells to show.\n");
		}
	}
}

//hud "name" x y w h hud ["text"]
void	*GUI_HUD_Create(gui_element_t *obj, int argc, char *argv[])
{
	gui_hud_t *hud;
	vec2_t topleft, bottomright;

	if (argc < 4)
	{
		corec.Console_Printf("syntax: hud name x y w h\n");
		return NULL;
	}

	corec.GUI_SetClass(obj, class_name);
	corec.GUI_SetName(obj, argv[0]);
	corec.GUI_Move (obj, atoi(argv[1]), atoi(argv[2]) );
	corec.GUI_Resize (obj, atoi(argv[3]), atoi(argv[4]) );

	hud = corec.GUI_Malloc(sizeof (gui_hud_t));

	if (!hud)
	{
		corec.Console_Printf("HUD error: couldn't enough space to hold hud\n");
		return NULL; 		
	}

	corec.GUI_SetUserdata(class_name, obj, hud);
	hud->element = obj;
	GUI_HUD_SetCells(hud->element, 5, 5);

	corec.WOG_GetGridSquareCoords(0, 0, topleft, bottomright);
	hud->wc_view_size[X] = (bottomright[X] - topleft[X]) * hud->num_cells_across;
	hud->wc_view_size[Y] = (bottomright[Y] - topleft[Y]) * hud->num_cells_down;

	obj->interactive = false;
	obj->draw = GUI_DrawHUD;
	obj->move = GUI_MoveHUD;
	obj->param = GUI_HUD_Param_Cmd;

	hud->friend_shader = 0;
	hud->enemy_shader = 0;
	hud->other_shader = 0;

	hud->parent = NULL;

	return hud;
}

void	GUI_HUD_Cmd(int argc, char *argv[])
{
	gui_element_t *obj;

	if (!argc)
	{
		corec.Console_Printf("hud <command> <args>\n");
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
			GUI_HUD_Param_Cmd(obj, argc -2, &argv[2]);
		}
	}
}

void	GUI_HUD_Init()
{
	corec.Cmd_Register("hud", GUI_HUD_Cmd);

	corec.GUI_RegisterClass(class_name, GUI_HUD_Create);
}
