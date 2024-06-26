#include "../toplayer/tl_shared.h"
#include "gui_graphic.h"

static char *class_name = "graphic";

float	GetShaderFrameTime(residx_t shader, int frame)
{
	return ((float)frame / corec.Res_GetAnimatedTextureFPS(shader));
}

#ifndef EDITOR_DLL
void	INT_DrawCrosshair(int x, int y, int w, int h, float focus, bool melee);
#endif

void	GUI_DrawGraphic(gui_element_t *obj, int x, int y)
{
	gui_graphic_t *graphic;
	int rows, cols, playcount;

	graphic = corec.GUI_GetUserdata(class_name, obj);
	if (!graphic)
		return;

#ifndef EDITOR_DLL
	if (!stricmp(obj->name, "crosshair"))
	{
		int	size = core.Cvar_GetValue("int_crosshairSize");

		INT_DrawCrosshair((obj->width / 2) - (size / 2), (obj->height / 2) - (size / 2), size, size, 1.0, false);
		return;
	}
#endif

	playcount = corec.Res_GetMoviePlayCount(graphic->shader);
	
	if (playcount < 0 || playcount > graphic->startPlayNum)
	{
		if (graphic->movieEndCommand)
			corec.GUI_Exec(graphic->movieEndCommand);
	}

	if (obj->alpha > 0)
	{
		corec.GUI_SetRGBA(
				graphic->color[0],
				graphic->color[1],
				graphic->color[2],
				obj->alpha);

		if (graphic->shader)
		{			
			corec.Draw_SetShaderTime(GetShaderFrameTime(graphic->shader, (int)graphic->curFrame));
			corec.GUI_Quad2d_S(0,0, 
					obj->width, 
					obj->height, 
					graphic->shader);
		}
	}

	if (graphic->text[0])
	{
		corec.GUI_GetStringRowsCols(graphic->text, &rows, &cols);
	
		corec.GUI_SetRGBA(obj->textcolor[0], obj->textcolor[1], obj->textcolor[2], obj->alpha);
		corec.GUI_DrawString(obj->width/2 - 4*strlen(graphic->text), obj->height/2-4, graphic->text, obj->char_height, obj->char_height, rows, graphic->element->width, corec.GetNicefontShader());
	}

	//fixme: if draw isn't getting called every frame or the widget is hidden this won't get updated correctly.
	//probably need an 'idle' function for this widget
	graphic->curFrame += (float)graphic->shaderFPS * corec.FrameSeconds();

	if (graphic->endFrame >= 0)
	{
		if (graphic->curFrame >= (graphic->endFrame+1))
		{
			if (!graphic->numloops)
			{
				//loop forever
				graphic->curFrame = graphic->startFrame;
			}
			else
			{
				graphic->numloops--;
				if (graphic->numloops <= 0)
				{
					graphic->startFrame = graphic->freezeFrame;
					graphic->curFrame = graphic->freezeFrame;
					graphic->endFrame = graphic->freezeFrame;
					graphic->numloops = 0;
				}
				else
				{
					graphic->curFrame = graphic->startFrame;	//loop back to start of anim
				}
			}
		}
	}
}

void	GUI_MoveGraphic(gui_element_t *obj, int x, int y)
{
	corec.GUI_Move(obj, x, y);
}

void	GUI_Graphic_ShowText(gui_element_t *obj, char *text)
{
	gui_graphic_t *graphic = corec.GUI_GetUserdata(class_name, obj);

	if (!graphic)
		return;

	BPrintf(graphic->text, 255, "%s", text);
}

void	GUI_Graphic_ShowFloat(gui_element_t *obj, float number)
{
	GUI_Graphic_ShowText(obj, fmt("%f", number));
}

void	GUI_Graphic_ShowInt(gui_element_t *obj, int number)
{
	GUI_Graphic_ShowText(obj, fmt("%i", number));
}

void	GUI_Graphic_ChangeImage(gui_element_t *obj, residx_t image)
{
	gui_graphic_t *graphic;

	graphic = corec.GUI_GetUserdata(class_name, obj);

	if (!graphic)
		return;

	graphic->shader = image;
}

void GUI_Graphic_Param_Cmd(gui_element_t *obj, int argc, char *argv[])
{
	gui_graphic_t *graphic;
	char filename[256];

	if (argc < 1)
	{
		corec.Console_Printf("graphic param <panel:object> <var> <value>\n");
		corec.Console_Printf("valid parameters:\n");
		corec.Console_Printf("   image <image> - changes the image\n");
		corec.Console_Printf("   char_height <size> - changes the text size\n");
		corec.Console_Printf("   textcolor <r> <g> <b> - changes the text color\n");
		corec.Console_Printf("   color <r> <g> <b> - changes the color of the image\n");
		corec.Console_Printf("   text <text>   - sets the text color\n");
		corec.Console_Printf("   animate <startframe> <endframe> <numloops> <freezeframe> - sets the animation parameters\n");
		corec.Console_Printf("   fps <float> - sets the fps of the animation\n");
		corec.Console_Printf("   movieEndCommand <command> - execute this command when a movie finishes playing\n");
		return;
	}

	graphic = corec.GUI_GetUserdata(class_name, obj);

	if (!graphic)
		return;

	if (stricmp(argv[0], "image") == 0)
	{
		if (argc > 1)
		{
			if (stricmp(argv[1], "unload") == 0)
			{
				corec.Res_BinkUnload(graphic->shader);
				graphic->shader = 0;
				graphic->startPlayNum = 0;
				return;
			}

			BPrintf(filename, 255, "%s%s", UI_PREFIX, argv[1]);
			filename[255] = 0;

			graphic->shader = corec.Res_LoadShaderEx(filename, SHD_NO_MIPMAPS | SHD_FULL_QUALITY);
			graphic->startPlayNum = corec.Res_GetMoviePlayCount(graphic->shader);
		}
		else
		{
			corec.Console_Printf("Not enough parameters.  You must specify the image filename\n");
		}
	}
	else if (stricmp(argv[0], "text") == 0)
	{
		if (argc > 1)
		{
			BPrintf(graphic->text, 255, "%s", argv[1]);
		} else {
			corec.Console_Printf("Not enough parameters.  You must specify the text.\n");
		}
	}
	else if (stricmp(argv[0], "color") == 0)
	{
		if (argc > 3)
		{
			graphic->color[0] = atof(argv[1]);
			graphic->color[1] = atof(argv[2]);
			graphic->color[2] = atof(argv[3]);
		}
		else
		{
			corec.Console_Printf("Error: Not enough parameters.  You must specify the color r g b values\n");
		}
	}
	else if (stricmp(argv[0], "animate") == 0)
	{
		//animate startframe endframe num_loops stopframe
		if (argc >= 3)
		{
			graphic->startFrame = graphic->curFrame = atoi(argv[1]);
			graphic->endFrame = atoi(argv[2]);
			if (argc > 3)
			{
				graphic->numloops = atoi(argv[3]);
				if (argc > 4)
					graphic->freezeFrame = atoi(argv[4]);
				else
					graphic->freezeFrame = graphic->endFrame;
			}
			else
			{
				graphic->numloops = 1;
				graphic->freezeFrame = graphic->endFrame;
			}
			graphic->shaderFPS = corec.Res_GetAnimatedTextureFPS(graphic->shader);
		}
	}
	else if (strcmp(argv[0], "fps") == 0)
	{
		if (argc > 1)
			graphic->shaderFPS = atoi(argv[1]);
	}
	else if (stricmp(argv[0], "movieEndCommand") == 0)
	{
		if (argc > 1)
		{
			strncpy(graphic->movieEndCommand, argv[1], GUI_GRAPHIC_CMD_LENGTH);
			graphic->startPlayNum = corec.Res_GetMoviePlayCount(graphic->shader);
		}
	}
	/*else if (strcmp(argv[0], "playonce") == 0)
	{
		graphic->startShaderTime = 0;
		graphic->endShaderTime = GetShaderFrameTime(graphic->shader, corec.Res_GetNumTextureFrames(graphic->shader));
		graphic->loop = false;
	}*/
}

void	GUI_ShowGraphic(gui_element_t *obj)
{
	gui_graphic_t *graphic;

	graphic = corec.GUI_GetUserdata(class_name, obj);

	if (!graphic)
		return;

	corec.Res_BinkContinue(graphic->shader);
}

void	GUI_HideGraphic(gui_element_t *obj)
{
	gui_graphic_t *graphic;

	graphic = corec.GUI_GetUserdata(class_name, obj);

	if (!graphic)
		return;

	corec.Res_BinkStop(graphic->shader);
}

//graphic "name" x y w h graphic ["text"]
void	*GUI_Graphic_Create(gui_element_t *obj, int argc, char *argv[])
{
	gui_graphic_t *graphic;

	if (argc < 4)
	{
		corec.Console_Printf("syntax: graphic name x y w h\n");
		return NULL;
	}

	corec.GUI_SetClass(obj, class_name);
	corec.GUI_SetName(obj, argv[0]);
	corec.GUI_Move (obj, atoi(argv[1]), atoi(argv[2]) );
	corec.GUI_Resize (obj, atoi(argv[3]), atoi(argv[4]) );

	graphic = corec.GUI_Malloc(sizeof (gui_graphic_t));

	if (!graphic)
	{
		corec.Console_Printf("Graphic error: couldn't enough space to hold graphic\n");
		return NULL; 		
	}

	corec.GUI_SetUserdata(class_name, obj, graphic);
	graphic->element = obj;
	M_SetVec3(graphic->color, 1, 1, 1);

	strcpy(graphic->text, "");

	obj->interactive = false;
	obj->draw = GUI_DrawGraphic;
	obj->move = GUI_MoveGraphic;
	obj->param = GUI_Graphic_Param_Cmd;

	obj->show = GUI_ShowGraphic;
	obj->hide = GUI_HideGraphic;

	graphic->parent = NULL;

	graphic->curFrame = 0;
	graphic->shaderFPS = corec.Res_GetAnimatedTextureFPS(graphic->shader);
	graphic->endFrame = -1;
	graphic->startFrame = 0;
	graphic->numloops = 0;

	return graphic;
}

void	GUI_Graphic_Cmd(int argc, char *argv[])
{
	gui_element_t *obj;

	if (!argc)
	{
		corec.Console_Printf("graphic <command> <args>\n");
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
			GUI_Graphic_Param_Cmd(obj, argc -2, &argv[2]);
		}
	}
	else if (stricmp(argv[0], "binkrestart") == 0)
	{
		if (argc > 1)
		{
			gui_graphic_t	*graphic;

			graphic = corec.GUI_GetClass(argv[1], class_name);
			if (graphic)
				corec.Res_BinkRestart(graphic->shader);
			else
				corec.Console_Printf("Couldn't find graphic type widget %s\n", argv[1]);
		}
		else
		{
			corec.Console_Printf("Missing object name\n");
		}
	}
	else if (stricmp(argv[0], "binkstop") == 0)
	{
		if (argc > 1)
		{
			gui_graphic_t	*graphic;

			graphic = corec.GUI_GetClass(argv[1], class_name);
			if (graphic)
				corec.Res_BinkStop(graphic->shader);
			else
				corec.Console_Printf("Couldn't find graphic type widget %s\n", argv[1]);
		}
		else
		{
			corec.Console_Printf("Missing object name\n");
		}
	}
	else if (stricmp(argv[0], "binkcontinue") == 0)
	{
		if (argc > 1)
		{
			gui_graphic_t	*graphic;

			graphic = corec.GUI_GetClass(argv[1], class_name);
			if (graphic)
				corec.Res_BinkContinue(graphic->shader);
			else
				corec.Console_Printf("Couldn't find graphic type widget %s\n", argv[1]);
		}
		else
		{
			corec.Console_Printf("Missing object name\n");
		}
	}
}

void	GUI_Graphics_Init()
{
	corec.Cmd_Register("graphic", GUI_Graphic_Cmd);

	corec.GUI_RegisterClass(class_name, GUI_Graphic_Create);
}
