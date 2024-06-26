
#include "../toplayer/tl_shared.h"
#include "gui_label.h"

static char *class_name = "label";

#define ALIGN_LEFT 	0
#define ALIGN_CENTER 	1
#define ALIGN_RIGHT 	2

void	GUI_Label_Draw(gui_element_t *obj, int w, int h)
{
	gui_label_t *label;
	int rows, cols, text_x;
	vec4_t color;

	label = corec.GUI_GetUserdata(class_name, obj);

	if (!label)
	{
		corec.Console_Printf("Error: couldn't get userdata for label %s\n", obj->name);
		return;
	}

	if (label->cvar[0])
	{
		char *string = corec.Cvar_GetString(label->cvar);
		if (strcmp(string, label->text) != 0)
			GUI_Label_ShowText(obj, string);
	}

	M_CopyVec3(obj->textcolor, color);
	color[3] = 1 - MAX(0, 1 - obj->alpha);
	corec.Draw_SetColor(color);

	if (strcmp(label->text, "") != 0)
	{
		corec.GUI_GetStringRowsCols(label->text, &rows, &cols);
		label->textWidth = corec.GUI_StringWidth(label->text, obj->char_height, obj->char_height, 50, 1024, corec.GetNicefontShader());
	
		switch (label->align)
		{
			case ALIGN_LEFT:
			default:
				text_x = 0;
				break;
			case ALIGN_CENTER:
				text_x = w/2 - label->textWidth/2;
				break;
			case ALIGN_RIGHT:
				text_x = w - label->textWidth;
				break;

		}
		if (label->bgcolor[3])
		{
			corec.Draw_SetColor(label->bgcolor);
			corec.GUI_Quad2d_S(text_x-5,0,label->textWidth+10,obj->char_height,corec.GetWhiteShader());
		}


		if (label->shadow)
			corec.GUI_DrawShadowedString(text_x, 0, label->text, obj->char_height, obj->char_height, rows, label->textWidth, corec.GetNicefontShader(), color[0], color[1], color[2], color[3] * obj->alpha);
		else
			corec.GUI_DrawString(text_x, 0, label->text, obj->char_height, obj->char_height, rows, label->textWidth, corec.GetNicefontShader());
	}
}

void	GUI_Label_Move(gui_element_t *obj, int x, int y)
{
	corec.GUI_Move(obj, x, y);
}

void    GUI_Label_ShowText(gui_element_t *obj, char *text)
{
	gui_label_t *label = corec.GUI_GetUserdata(class_name, obj);

	if (!label)
		return;
	
	if (label->text)
		corec.GUI_Free(label->text);
	label->text = corec.GUI_StrDup(text);
	corec.Cmd_ProcessString(label->text);
	label->textWidth = corec.GUI_StringWidth(text, obj->char_height, obj->char_height, 50, 1024, corec.GetNicefontShader());
}

void    GUI_Label_ShowFloat(gui_element_t *obj, float number)
{
	GUI_Label_ShowText(obj, fmt("%f", number));
}

void    GUI_Label_ShowInt(gui_element_t *obj, int number)
{
	GUI_Label_ShowText(obj, fmt("%i", number));
}

void GUI_Label_Param_Cmd(gui_element_t *obj, int argc, char *argv[])
{
	gui_label_t *label;

	if (argc < 1)
	{
		corec.Console_Printf("label param <panel:object> <var> <value>\n");
		corec.Console_Printf("valid parameters:\n");
		corec.Console_Printf("   char_height <height>   - size of the font, vertically\n");
		corec.Console_Printf("   text <text>            - the text to use as the label\n");
		corec.Console_Printf("   cvar <cvarname>        - the cvar to use as the label\n");
		corec.Console_Printf("   variable <cvarname>	- the cvar to use as the label\n");
		corec.Console_Printf("   textcolor <r> <g> <b>  - change the text color\n");
		corec.Console_Printf("   shadow <1|0>  			- whether to draw a dropshadow\n");
		corec.Console_Printf("   align <left|center|right> - the alignment to use for the text\n");
		return;
	}

	label = corec.GUI_GetUserdata(class_name, obj);

	if (!label)
		return;

	if (strcmp(argv[0], "text") == 0)
	{
		if (argc > 1)
		{
			label->cvar[0] = 0;
			GUI_Label_ShowText(obj, argv[1]);
		}
		else
		{
			label->text = corec.GUI_StrDup("");
			label->textWidth = 0;
		}
	}	
	else if (strcmp(argv[0], "cvar") == 0 || strcmp(argv[0], "variable") == 0)
	{
		if (argc > 1)
		{
			strncpySafe(label->cvar, argv[1], sizeof(label->cvar));			
		}
		else
		{
			label->text = corec.GUI_StrDup("");
			label->textWidth = 0;
		}
	}	
	else if (strcmp(argv[0], "align") == 0)
	{
		if (argc > 1)
		{
			if (strcmp(argv[1], "left")==0)
				label->align = ALIGN_LEFT;
			else if (strcmp(argv[1], "center")==0)
				label->align = ALIGN_CENTER;
			else if (strcmp(argv[1], "right")==0)
				label->align = ALIGN_RIGHT;
		}
	}
	else if (strcmp(argv[0], "shadow") == 0)
	{
		if (argc > 1)
		{
			label->shadow = atoi(argv[1]);
		}
	}
	else if (strcmp(argv[0], "bgcolor") == 0)
	{
		if (argc > 4)
		{
			label->bgcolor[0] = atof(argv[1]);
			label->bgcolor[1] = atof(argv[2]);
			label->bgcolor[2] = atof(argv[3]);
			label->bgcolor[3] = atof(argv[4]);
		}
	}
}

void	GUI_Label_Destroy(gui_element_t *obj)
{
	gui_label_t *label;

	label = corec.GUI_GetUserdata(class_name, obj);

	corec.GUI_Free(label->text);
}

//label name x y w h
void	*GUI_Label_Create(gui_element_t *obj, int argc, char *argv[])
{
	gui_label_t *label;

	if (argc < 5)
	{
		corec.Console_Printf("syntax: create %s name x y w h\n", class_name);
		return NULL;
	}

	corec.GUI_SetClass(obj, class_name);
	corec.GUI_SetName(obj, argv[0]);
	corec.GUI_Move (obj, atoi(argv[1]), atoi(argv[2]) );
	corec.GUI_Resize (obj, atoi(argv[3]), atoi(argv[4]) );

	label = corec.GUI_Malloc(sizeof (gui_label_t));

	if (!label)
	{
		corec.Console_Printf("Label error: couldn't enough space to hold label\n");
		return NULL; 		
	}

	corec.GUI_SetUserdata(class_name, obj, label);
	label->element = obj;

	obj->interactive = false;

	obj->destroy = GUI_Label_Destroy;
	obj->draw = GUI_Label_Draw;
	obj->idle = NULL;
	obj->move = GUI_Label_Move;
	obj->mouseup = NULL;
	obj->mousedown = NULL;
	obj->mouseout = NULL;
	obj->mouseover = NULL;

	obj->param = GUI_Label_Param_Cmd;
	
	label->text = corec.GUI_StrDup("");
	label->textWidth = 0;
	label->align = ALIGN_CENTER;	

	label->parent = NULL;
	label->shadow = false;	

	return label;
}

void	GUI_Label_List()
{
	corec.GUI_List_Cmd(1, &class_name);
}

void	GUI_Label_Cmd(int argc, char *argv[])
{
	gui_element_t *obj;

	if (!argc)
	{
		corec.Console_Printf("label <command> <args>\n");
		corec.Console_Printf("  commands:\n");
		corec.Console_Printf("    list\n");
		corec.Console_Printf("    param\n");
		return;
	}

	if (strcmp(argv[0], "list") == 0)
	{
		GUI_Label_List();
	}
	else if (strcmp(argv[0], "param") == 0)
	{
		if (argc > 2)
		{
			obj = corec.GUI_GetObject(argv[1]);
			GUI_Label_Param_Cmd(obj, argc-2, &argv[2]);
		}
	}
}

void	GUI_Labels_Init()
{
	corec.GUI_RegisterClass(class_name, GUI_Label_Create);

	corec.Cmd_Register("label", GUI_Label_Cmd);
}
