
#include "../toplayer/tl_shared.h"
#include "gui_button.h"

static char *class_name = "button";

#define ALIGN_LEFT  0
#define ALIGN_CENTER    1
#define ALIGN_RIGHT     2

void	GUI_Button_DrawAltText(gui_element_t *obj, int w, int h)
{
	int x,y;
	ivec2_t pos;
	int altwidth, altheight, rows, cols;
	gui_button_t *button;

	button = corec.GUI_GetUserdata(class_name, obj);

	if (!button)
	{
		corec.Console_Printf("Error: couldn't get userdata for button %s\n", obj->name);
		return;
	}

	if (!button->alt_text[0])
		return;
		
	x = 0;
	y = 0;
	
	corec.GUI_GetStringRowsCols(button->alt_text, &rows, &cols);
	
	altwidth = button->altTextWidth + 8;
	altheight = rows*obj->char_height + 8;

	corec.GUI_GetPosition(obj, pos);

 	if (pos[X] + altwidth + w/2 - 4 > corec.GUI_GetScreenWidth())
 		x -= pos[X] + altwidth + w/2 - 4 - corec.GUI_GetScreenWidth();
 	if (pos[Y] + altheight + h/2 - 4 > corec.GUI_GetScreenWidth())
 		y -= pos[Y] + altheight + h/2 -4 - corec.GUI_GetScreenHeight();
	
	corec.GUI_SetRGBA(0.8,0.8,0.15,1);
	corec.GUI_Quad2d_S(x + w/2 - 4, y + h/2-8, altwidth, altheight, corec.GetWhiteShader());
	corec.GUI_SetRGBA(0,0,0,1);
	corec.GUI_DrawString(x + w/2, y + h/2-4, button->alt_text, obj->char_height, obj->char_height, rows, button->altTextWidth, corec.GetNicefontShader());
	corec.GUI_SetRGBA(1,1,1,1);
}

void	GUI_Button_Draw(gui_element_t *obj, int x, int y)
{
	gui_button_t *button;
	int w, h, shader, rows, cols, text_x;
	float r = 1, g = 1, b = 1, oshader = 0;
	float or = 0, og = 0, ob = 0, oa = 0;

	button = corec.GUI_GetUserdata(class_name, obj);

	if (!button)
	{
		corec.Console_Printf("Error: couldn't get userdata for button %s\n", obj->name);
		return;
	}

	x = 0;
	y = 0;
	w = obj->width;
	h = obj->height;

	if (!button->enabled)
	{
		if (button->disable_shader)
		{
			shader = button->disable_shader;
		}
		else
		{
			oshader = shader = button->up_shader;
			or = og = ob = 0.1;
			oa = 0.6;
		}
	}
	else if (button->down)
	{
		if (button->down_shader)
		{
			shader = button->down_shader;
		}
		else
		{
			//default button push effect
			shader = button->up_shader;
			x += 1;
			y += 1;			
		}
	}
	else
	{
		if (button->mouseover)
		{
			if (button->up_mouseover_shader)
			{
				shader = button->up_mouseover_shader;
			}
			else
			{
				shader = button->up_shader;
				oshader = corec.Res_LoadShader("/textures/core/1_nl_over.tga");
				or = og = ob = 1;
				oa = 1;
			}
		}
		else
		{
			shader = button->up_shader;
		}

	}

	if (shader)
	{
		corec.GUI_SetRGBA(r, g, b, obj->alpha);
		corec.GUI_Quad2d_S(x, y, w, h, shader);

		if (oshader)
		{
			corec.GUI_SetRGBA(or, og, ob, oa);
			corec.GUI_Quad2d_S(x, y, w, h, oshader);
		}
	}

	if (strcmp(button->text, "") != 0)
	{
		button->textWidth = corec.GUI_StringWidth(button->text, obj->char_height, obj->char_height, button->element->height / obj->char_height, GUI_BUTTON_TEXT_LENGTH, corec.GetNicefontShader());
		if (button->element->width < button->textWidth)
			corec.GUI_Resize(button->element, button->textWidth, button->element->height);

		switch (button->align)
		{
			case ALIGN_LEFT:
				text_x = 0;
				break;
			case ALIGN_CENTER:
			default:
				text_x = w/2 - button->textWidth/2;
				break;
			case ALIGN_RIGHT:
				text_x = w - button->textWidth;
				break;
		}
	
		corec.GUI_SetRGBA(obj->textcolor[0], obj->textcolor[1], obj->textcolor[2], obj->alpha);
		corec.GUI_GetStringRowsCols(button->text, &rows, &cols);
		corec.GUI_DrawString(x + text_x, y + (h - obj->char_height)/2, button->text, obj->char_height, obj->char_height, rows, obj->width, corec.GetNicefontShader());
	}
}

void	GUI_Button_MouseDown (gui_element_t *obj, mouse_button_enum mbutton, int x, int y)
{
	gui_button_t *button;

	if (mbutton != MOUSE_LBUTTON)
		return;
	
	button = corec.GUI_GetUserdata(class_name, obj);
	if (!button || !button->enabled)
		return;

	button->down = true;

	corec.GUI_Exec(button->down_command);
	button->nextRepeatTime = corec.Milliseconds() + GUI_BUTTON_REPEAT_DELAY;
}


void	GUI_Button_MouseUp (gui_element_t *obj, mouse_button_enum mbutton, int x, int y)
{
	gui_button_t *button;
	button = corec.GUI_GetUserdata(class_name, obj);
	if (!button || !button->enabled)
		return;

	button->down = false;
}

void	GUI_Button_MouseOut(gui_element_t *obj)
{
	gui_button_t *button;
	button = corec.GUI_GetUserdata(class_name, obj);
	if (!button)
		return;

	button->mouseover = false;
	button->down = false;
	if (button->mouseaway_command)
		corec.GUI_Exec(button->mouseaway_command);
}

void	GUI_Button_MouseOver(gui_element_t *obj, int x, int y)
{
	gui_button_t *button;
	button = corec.GUI_GetUserdata(class_name, obj);
	if (!button)
		return;

	if (!button->mouseover)
	{
		button->mouseover = true;
	}
	if (button->mouseover_command)
		corec.GUI_Exec(button->mouseover_command);

	if (button->repeat && button->down && corec.Milliseconds() > button->nextRepeatTime)
	{
		button->nextRepeatTime = corec.Milliseconds() + GUI_BUTTON_REPEAT_INTERVAL;
		corec.GUI_Exec(button->down_command);
	}
	
	if (obj->focus)
	{
		//button->mouseover = false;
	} 
}

void	GUI_Button_Move(gui_element_t *obj, int x, int y)
{
	corec.GUI_Move(obj, x, y);
}

void	GUI_Button_ShowText(gui_element_t *obj, char *string)
{
	gui_button_t *button;
	button = corec.GUI_GetUserdata(class_name, obj);
	if (!button)
		return;

	strncpy(button->text, string, GUI_BUTTON_TEXT_LENGTH-1);
	button->text[GUI_BUTTON_TEXT_LENGTH-1] = 0;
	corec.Cmd_ProcessString(button->text);
	if (obj->char_height < 1)
		obj->char_height = 1;
}

void	GUI_Button_Command(gui_element_t *obj, char *command)
{
	gui_button_t *button;
	button = corec.GUI_GetUserdata(class_name, obj);
	if (!button)
		return;

	strncpy(button->down_command, command, GUI_BUTTON_CMD_LENGTH-1);
	button->down_command[GUI_BUTTON_CMD_LENGTH-1] = 0;
}

void GUI_Button_FixShaders(gui_button_t *button)
{
	//if (!button->down_shader && button->up_shader)
	//	button->down_shader = button->up_shader;
	//if (!button->up_shader && button->down_shader)
	//	button->up_shader = button->down_shader;
	//if (!button->disable_shader && button->up_shader)
	//	button->disable_shader = button->up_shader;	

	//if (!button->up_mouseover_shader && button->up_shader)
	//	button->up_mouseover_shader = button->up_shader;
	//if (button->up_mouseover_shader == button->down_shader)
	//	button->up_mouseover_shader = button->up_shader;
}

void GUI_Button_Param_Cmd(gui_element_t *obj, int argc, char *argv[])
{
	char filename[256];
	gui_button_t *button;

	if (argc < 1)
	{
		corec.Console_Printf("button param <panel:object> <var> <value>\n");
		corec.Console_Printf("valid parameters:\n");
		corec.Console_Printf("   command <command>      - the command to execute when a button is pressed\n");
		corec.Console_Printf("   mouseenter_command <command> - the command to execute when the mouse enters the widget\n");
		corec.Console_Printf("   mouseover_command <command> - the command to execute while the mouse is over the widget\n");
		corec.Console_Printf("   mouseaway_command <command> - the command to execute when the mouse exits the widget\n");
		corec.Console_Printf("   down_image <image>     - the image file to use when the button is down (pressed)\n");
		corec.Console_Printf("   up_image <image>       - the image file to use when the button is up   (normal)\n");
		corec.Console_Printf("   up_hover_image <image> - the image file to use when the button is up and the mouse is over it (mouseover)\n");
		corec.Console_Printf("   disable_image <image>  - the image file to use when the button is disabled\n");
		corec.Console_Printf("   reset                  - clear all params\n");
		corec.Console_Printf("   enable <true|false>    - set the button to be enabled or disabled (unclickable)\n");
		corec.Console_Printf("   text <text>            - the text to overlay on top of the button\n");
		corec.Console_Printf("   text_color <r> <g> <b> - the text color for text that is overlaid on top of the button\n");
		corec.Console_Printf("   alt <text>             - the alt text to overlay when they hover on the button\n");
		corec.Console_Printf("   align <left|center|right> - how to align the text on the button\n");
		corec.Console_Printf("   repeat					- repeat the command as long as the button is pressed\n");
		return;
	}

	button = corec.GUI_GetUserdata(class_name, obj);

	if (!button)
		return;

	if (strcmp(argv[0], "reset") == 0)
	{
		strcpy(button->down_command, "");
		strcpy(button->mouseover_command, "");
		strcpy(button->mouseaway_command, "");
		button->down_shader = 0;
		button->up_shader = 0;
		button->up_mouseover_shader = 0;
		button->disable_shader = 0;
		strcpy(button->text, "");
		button->textWidth = 0;
		strcpy(button->alt_text, "");
		button->altTextWidth = 0;
	}
	else if (strcmp(argv[0], "down_image") == 0)
	{
		if (argc > 1)
		{
			BPrintf(filename, 255, "%s%s", UI_PREFIX, argv[1]);
			filename[255] = 0;

			button->down_shader = corec.Res_LoadShaderEx(filename, SHD_NO_MIPMAPS | SHD_FULL_QUALITY);
			GUI_Button_FixShaders(button);
		}
		else
		{
			corec.Console_Printf("button error: you must specify a filename to use with this command.\n");
		}
	}
	else if (strcmp(argv[0], "up_image") == 0)
	{
		if (argc > 1)
		{
			BPrintf(filename, 255, "%s%s", UI_PREFIX, argv[1]);
			filename[255] = 0;

			button->up_shader = corec.Res_LoadShaderEx(filename, SHD_NO_MIPMAPS | SHD_FULL_QUALITY);
			GUI_Button_FixShaders(button);
		}
		else
		{
			corec.Console_Printf("button error: you must specify a filename to use with this command.\n");
		}
	}
	else if (strcmp(argv[0], "up_hover_image") == 0)
	{
		if (argc > 1)
		{
			BPrintf(filename, 255, "%s%s", UI_PREFIX, argv[1]);
			filename[255] = 0;

			button->up_mouseover_shader = corec.Res_LoadShaderEx(filename, SHD_NO_MIPMAPS | SHD_FULL_QUALITY);
			GUI_Button_FixShaders(button);
		}
		else
		{
			corec.Console_Printf("button error: you must specify a filename to use with this command.\n");
		}
	}
	else if (strcmp(argv[0], "disable_image") == 0)
	{
		if (argc > 1)
		{
			BPrintf(filename, 255, "%s%s", UI_PREFIX, argv[1]);
			filename[255] = 0;

			button->disable_shader = corec.Res_LoadShaderEx(filename, SHD_NO_MIPMAPS | SHD_FULL_QUALITY);
			GUI_Button_FixShaders(button);
		}
		else
		{
			corec.Console_Printf("button error: you must specify a filename to use with this command.\n");
		}
	}
	else if (strcmp(argv[0], "command") == 0)
	{
		if (argc > 1)
		{
			GUI_Button_Command(button->element, argv[1]);
		}
		else
		{
			strcpy(button->down_command, "");
		}
	}
	else if (strcmp(argv[0], "mouseenter_command") == 0)
	{
		if (argc > 1)
		{
			strncpy(button->mouseenter_command, argv[1], GUI_BUTTON_CMD_LENGTH-1);
			button->mouseenter_command[GUI_BUTTON_CMD_LENGTH-1] = 0;
		}
		else
		{
			strcpy(button->mouseenter_command, "");
		}
	}
	else if (strcmp(argv[0], "mouseover_command") == 0)
	{
		if (argc > 1)
		{
			strncpy(button->mouseover_command, argv[1], GUI_BUTTON_CMD_LENGTH-1);
			button->mouseover_command[GUI_BUTTON_CMD_LENGTH-1] = 0;
		}
		else
		{
			strcpy(button->mouseover_command, "");
		}
	}
	else if (strcmp(argv[0], "mouseaway_command") == 0)
	{
		if (argc > 1)
		{
			strncpy(button->mouseaway_command, argv[1], GUI_BUTTON_CMD_LENGTH-1);
			button->mouseaway_command[GUI_BUTTON_CMD_LENGTH-1] = 0;
		}
		else
		{
			strcpy(button->mouseaway_command, "");
		}
	}
	else if (strcmp(argv[0], "enable") == 0)
	{
		if (argc > 1)
		{
			if (atoi(argv[1]) || strcmp(argv[1], "true") == 0)
				button->enabled = true;
			else
				button->enabled = false;
		}
	}
	else if (strcmp(argv[0], "repeat") == 0)
	{
		if (argc > 1)
		{
			if (atoi(argv[1]) || strcmp(argv[1], "true") == 0)
				button->repeat = true;
			else
				button->repeat = false;
		}
	}
	else if (strcmp(argv[0], "text") == 0)
	{
		if (argc > 1)
		{
			GUI_Button_ShowText(button->element, argv[1]);
		}
		else
		{
			strcpy(button->text, "");
			button->textWidth = 0;
		}
	}	
	else if (strcmp(argv[0], "alt") == 0)
	{
		if (argc > 1)
		{
			strncpy(button->alt_text, argv[1], GUI_BUTTON_ALT_TEXT_LENGTH-1);
			button->altTextWidth = corec.GUI_StringWidth(button->alt_text, obj->char_height, obj->char_height, 255, GUI_BUTTON_TEXT_LENGTH, corec.GetNicefontShader());
			corec.Cmd_ProcessString(button->alt_text);
			button->alt_text[GUI_BUTTON_ALT_TEXT_LENGTH-1] = 0;
		}
		else
		{
			strcpy(button->alt_text, "");
		}
	}
	else if (strcmp(argv[0], "align") == 0)
	{
		if (argc > 1)
		{
			if (strcmp(argv[1], "left")==0)
				button->align = ALIGN_LEFT;
			else if (strcmp(argv[1], "center")==0)
				button->align = ALIGN_CENTER;
			else if (strcmp(argv[1], "right")==0)
				button->align = ALIGN_RIGHT;
		}
	}
		
}

//button name x y w h
void	*GUI_Button_Create(gui_element_t *obj, int argc, char *argv[])
{
	gui_button_t *button;

	if (argc < 5)
	{
		corec.Console_Printf("syntax: create %s name x y w h\n", class_name);
		return NULL;
	}

	corec.GUI_SetClass(obj, class_name);
	corec.GUI_SetName(obj, argv[0]);
	corec.GUI_Move (obj, atoi(argv[1]), atoi(argv[2]) );
	corec.GUI_Resize (obj, atoi(argv[3]), atoi(argv[4]) );

	button = corec.GUI_Malloc(sizeof (gui_button_t));

	if (!button)
	{
		corec.Console_Printf("Button error: couldn't enough space to hold button\n");
		return NULL; 		
	}

	corec.GUI_SetUserdata(class_name, obj, button);
	button->element = obj;

	obj->draw = GUI_Button_Draw;
	obj->drawalttext = GUI_Button_DrawAltText;
	obj->move = GUI_Button_Move;
	obj->mouseup = GUI_Button_MouseUp;
	obj->mousedown = GUI_Button_MouseDown;
	obj->mouseout = GUI_Button_MouseOut;
	obj->mouseover = GUI_Button_MouseOver;
	
	obj->param = GUI_Button_Param_Cmd;
	
	button->down = false;
	button->mouseover = false;
	button->enabled = true;

	strcpy(button->text, "");
	strcpy(button->alt_text, "");
	strcpy(button->down_command, "");
	strcpy(button->mouseover_command, "");
	strcpy(button->mouseaway_command, "");

	button->down_shader = 0;
	button->up_mouseover_shader = 0;
	button->up_shader = 0;

	button->parent = NULL;
	button->align = ALIGN_CENTER;

	return button;
}

//GUI_Button_Destroy only needs to be implemented if we allocate memory other than the button_t struct or inside it


void	GUI_Button_List()
{
	corec.GUI_List_Cmd(1, &class_name);
}

void	GUI_Button_Cmd(int argc, char *argv[])
{
	gui_element_t *obj;

	if (!argc)
	{
		corec.Console_Printf("button <command> <args>\n");
		corec.Console_Printf("  commands:\n");
		corec.Console_Printf("    list\n");
		corec.Console_Printf("    param\n");
		return;
	}

	if (strcmp(argv[0], "list") == 0)
	{
		GUI_Button_List();
	}
	else if (strcmp(argv[0], "param") == 0)
	{
		if (argc > 2)
		{
			obj = corec.GUI_GetObject(argv[1]);
			GUI_Button_Param_Cmd(obj, argc-2, &argv[2]);
		}
	}
}

void	GUI_Buttons_Init()
{
	corec.GUI_RegisterClass(class_name, GUI_Button_Create);

	corec.Cmd_Register("button", GUI_Button_Cmd);
}
