#include "../toplayer/tl_shared.h"
#include "gui_textbox.h"

static char *class_name = "textbox";

#define	TEXTBOX_BORDER 1
#define TEXTBOX_SPACING_X 2
#define TEXTBOX_SPACING_Y 1

#define HIDDEN_CHAR '*'

#define MAX_HIDDEN_CHARS 256

static void _trimend(gui_textbox_t *textbox);

static char hidden_chars[MAX_HIDDEN_CHARS];

bool	GUI_Textbox_ReadingInput(gui_element_t *obj)
{
	return (corec.Input_GetCallbackWidget() == obj);
}

void	GUI_DrawTextbox(gui_element_t *obj, int x, int y)
{
	gui_textbox_t *textbox;
	bool drawcursor;
	int cursorPos;
	int yoffset;
	int stringLength = 0;
	int length = 0;
	char *bufStart;

	textbox = corec.GUI_GetUserdata(class_name, obj);

	if (!textbox)
		return;

	yoffset = TEXTBOX_SPACING_Y + (obj->height - obj->char_height)/2;
	
    //else if (textbox->reading_input && !obj->focus)
    //    GUI_Textbox_Abort(textbox);

	//if (!textbox->reading_input)
	//{
		memset(textbox->buffer, 0, GUI_TEXTBOX_BUFFER_SIZE); 
		if (textbox->variable[0])
		{
			int modifiedCount = corec.Cvar_GetModifiedCount(textbox->variable);
			strcpy(textbox->buffer, corec.Cvar_GetString(textbox->variable));
			if (textbox->variableModifiedCount != modifiedCount)
			{
				textbox->cursor = strlen(textbox->buffer);
				textbox->variableModifiedCount = modifiedCount;
			}
		}
		
		if (textbox->type == INPUT_TYPE_FLOAT)
			_trimend(textbox);
	//}
	//else
	//{
		if ((int)strlen(textbox->buffer) < textbox->cursor)
			textbox->cursor = strlen(textbox->buffer);

		if (corec.GUI_StringWidth(textbox->buffer, obj->char_height, obj->char_height, 1, obj->width + 100, corec.GetNicefontShader()) < obj->width)
			textbox->pos = 0;
	//}

	drawcursor = ((int)((corec.Milliseconds() / 1000.0)*corec.Cvar_GetValue("con_cursorspeed")) & 1);

	if (textbox->hide_input)
	{
		bufStart = hidden_chars;
		stringLength = strlen(&textbox->buffer[textbox->pos]);
		bufStart[stringLength] = 0;
	}
	else
		bufStart = &textbox->buffer[textbox->pos];

	corec.GUI_SetRGBA(1, 1, 1, obj->alpha);
	if (textbox->border)
		corec.GUI_LineBox2d_S(0, 0, obj->width, obj->height, textbox->border);

	length = corec.GUI_StringWidth(&bufStart[textbox->pos], obj->char_height, obj->char_height, 1, textbox->max_length, corec.GetNicefontShader()); 
	cursorPos = TEXTBOX_SPACING_X + corec.GUI_StringWidth(&textbox->buffer[textbox->pos], obj->char_height, obj->char_height, 1, textbox->max_length, corec.GetNicefontShader());
	cursorPos -= TEXTBOX_SPACING_X + corec.GUI_StringWidth(&textbox->buffer[textbox->cursor], obj->char_height, obj->char_height, 1, textbox->max_length, corec.GetNicefontShader()); 

	corec.GUI_SetRGBA(obj->textcolor[0], obj->textcolor[1], obj->textcolor[2], obj->alpha);
	if (textbox->dropshadow)
	{
		corec.GUI_DrawShadowedString(TEXTBOX_SPACING_X,
				yoffset,
				bufStart,
				obj->char_height,
				obj->char_height,
				1,
				textbox->element->width - TEXTBOX_SPACING_X*2,
				corec.GetNicefontShader(),
				obj->textcolor[0], obj->textcolor[1], obj->textcolor[2], obj->alpha);
	}
	else
	{
		//draw input buffer
		corec.GUI_DrawString(TEXTBOX_SPACING_X,
				yoffset,
				bufStart,
				obj->char_height,
				obj->char_height,
				1,
				textbox->element->width - TEXTBOX_SPACING_X*2,
				corec.GetNicefontShader());
	}

	//draw cursor
	if (GUI_Textbox_ReadingInput(obj))
	{/*
		corec.GUI_DrawString(TEXTBOX_SPACING_X + length - afterCursorLength,
				yoffset,
				">",
				obj->char_height,
				obj->char_height,
				1, 50,
				corec.GetNicefontShader());
*/
		if (drawcursor)
		{
			corec.GUI_Quad2d_S(cursorPos,
				yoffset+obj->char_height-2, obj->char_height/2, 2, corec.GetWhiteShader());
		}
		else
		{
			//corec.GUI_Quad2d_S(TEXTBOX_SPACING_X + length - afterCursorLength,
			//	yoffset, 1, obj->char_height, corec.GetWhiteShader());
		}
				
	}

	
	if (textbox->hide_input)
	{
		bufStart[stringLength] = HIDDEN_CHAR;
	}
}

void	GUI_Textbox_Mousedown(gui_element_t *obj, mouse_button_enum button, int x, int y)
{
//	GUI_Textbox_Activate(obj);
}

void	GUI_MoveTextbox(gui_element_t *obj, int x, int y)
{
	corec.GUI_Move(obj, x, y);
}

void	GUI_Textbox_Activate(gui_element_t *obj)
{
	gui_textbox_t *textbox;

	textbox = corec.GUI_GetUserdata(class_name, obj);

//	corec.GUI_Focus(obj);
	
	if (!textbox)
		return;
	
	if (!corec.Input_ActivateInputCallback(GUI_Textbox_InputCallback, false, obj))
	{
		corec.Console_Printf("Textbox error: Can't get input focus\n");
		return;
	}

	textbox->cursor = strlen(textbox->buffer);
}

void	GUI_Textbox_Deactivate(gui_element_t *obj)
{
	gui_textbox_t *textbox = corec.GUI_GetUserdata(class_name, obj);
	if (!textbox)
		return;

	if (GUI_Textbox_ReadingInput(obj))
		corec.Input_DeactivateInputCallback(true);

	if (textbox->pos || textbox->cursor)
		GUI_Textbox_Abort(obj);	
}


void	GUI_HideTextbox(gui_element_t *obj)
{
	if (GUI_Textbox_ReadingInput(obj))
		GUI_Textbox_Deactivate(obj);
}

static void	_trimend(gui_textbox_t *textbox)
{
	int size;
	
	size = strlen(textbox->buffer);
	if (!strchr(textbox->buffer, '.'))
		return;

	while (textbox->buffer[size-1] == '0')
		size--;
	textbox->buffer[size] = 0;

	//try to get rid of any trailing .
	size--;
	if (textbox->buffer[size] == '.')
	{
		textbox->buffer[size] = 0;
	}

	if (textbox->cursor > size)
		textbox->cursor = size;
	/*if (size - textbox->pos <  textbox->size)
		textbox->pos = size - textbox->size + 1;*/
	if (textbox->variable)
	{
		corec.Cvar_Set(textbox->variable, textbox->buffer);
		textbox->variableModifiedCount = corec.Cvar_GetModifiedCount(textbox->variable);
	}
}

void	GUI_Textbox_Commit(gui_textbox_t *textbox)
{
	if (textbox->type == INPUT_TYPE_FLOAT)
		_trimend(textbox);
	textbox->pos = 0;
	textbox->cursor = 0;
	if (textbox->exec_cmd)
		corec.GUI_Exec(textbox->exec_cmd);

	if (!textbox->keepInput)
		GUI_Textbox_Deactivate(textbox->element);
}

void	GUI_Textbox_Abort(gui_element_t *obj)
{
	gui_textbox_t *textbox;

	textbox = corec.GUI_GetUserdata(class_name, obj);
	if (!textbox)
		return;

	if (textbox->abort_cmd)
		corec.GUI_Exec(textbox->abort_cmd);
}

bool	GUI_Textbox_InputCallback(int key, unsigned char rawchar, bool down, void *userdata)
{
	gui_textbox_t *textbox;

	if (!userdata)
		return false;

	textbox = corec.GUI_GetUserdata(class_name, (gui_element_t *)userdata);

	if (!textbox)
		return false;

	if (!down)
		return true;

	switch(key)
	{
		case KEY_DEL:
			if (textbox->cursor<0)
				break;

			//textbox->buffer[textbox->cursor-1] = '\0';
			memmove(&textbox->buffer[textbox->cursor], &textbox->buffer[textbox->cursor+1], strlen(&textbox->buffer[textbox->cursor]));
			break;
		case KEY_BACKSPACE:
			if (textbox->cursor<=0)
				break;

			//textbox->buffer[textbox->cursor-1] = '\0';
			memmove(&textbox->buffer[textbox->cursor-1], &textbox->buffer[textbox->cursor], strlen(&textbox->buffer[textbox->cursor])+1);
			textbox->cursor--;

			if (textbox->cursor < textbox->pos && textbox->pos > 0)
				textbox->pos--;
			break;
		case KEY_ENTER:
			GUI_Textbox_Commit(textbox);
			return true;
		case KEY_ESCAPE:
			GUI_Textbox_Abort(textbox->element);
			if (!textbox->keepInput)
				GUI_Textbox_Deactivate(textbox->element);
			else
				return false;		//pass through to check binds
			return true;
		case KEY_LEFT:
			textbox->cursor--;
			if (textbox->cursor<0) 
				textbox->cursor=0;

			if (textbox->cursor < textbox->pos && textbox->pos > 0)
				textbox->pos--;
			return true;
		case KEY_RIGHT:
			textbox->cursor++;
			if (textbox->cursor > (int)strlen(textbox->buffer))
				textbox->cursor = (int)strlen(textbox->buffer);

			while (corec.GUI_StringWidth(&textbox->buffer[textbox->pos], textbox->element->char_height, textbox->element->char_height, 1, textbox->max_length, corec.GetNicefontShader()) 
					- corec.GUI_StringWidth(&textbox->buffer[textbox->cursor], textbox->element->char_height, textbox->element->char_height, 1, textbox->max_length, corec.GetNicefontShader()) 
					+ corec.GUI_StringWidth("_", textbox->element->char_height, textbox->element->char_height, 1, textbox->max_length, corec.GetNicefontShader()) > textbox->element->width)
				textbox->pos++;
			return true;
		case KEY_TAB:
			GUI_Textbox_Commit(textbox);
			corec.GUI_ActivateNextInteractiveWidget();
			return true;
	}

	if (rawchar >= 32 && rawchar < 256)
	{
		char *tailend = &textbox->buffer[textbox->cursor];
		
		if (strlen(textbox->buffer) <= textbox->max_length-2)
		{
			memmove(tailend+1, tailend, strlen(tailend));
		}

		key = rawchar;
		switch (textbox->type)
		{
			case INPUT_TYPE_FLOAT:
			  		if ((key >= '0' && key <= '9') 
						|| (key == '-' && textbox->cursor == 0))
					{
						textbox->buffer[textbox->cursor++] = key;	
					}
					else if (key == '.' 
						&& !strchr(textbox->buffer, '.'))
					{
						textbox->buffer[textbox->cursor++] = key;	
					}
					break;
			case INPUT_TYPE_INT:
			  		if ((key >= '0' && key <= '9') 
						|| (key == '-' && textbox->cursor == 0))
						textbox->buffer[textbox->cursor++] = key;	
					break;
			case INPUT_TYPE_STRING:
					textbox->buffer[textbox->cursor++] = key;	
					break;
		}
	}

	if (textbox->cursor > textbox->max_length-2)
		textbox->cursor = textbox->max_length-2;

	if (textbox->cursor < textbox->pos && textbox->pos > 0)
		textbox->pos--;
	
	while (corec.GUI_StringWidth(&textbox->buffer[textbox->pos], textbox->element->char_height, textbox->element->char_height, 1, textbox->max_length, corec.GetNicefontShader()) 
			- corec.GUI_StringWidth(&textbox->buffer[textbox->cursor], textbox->element->char_height, textbox->element->char_height, 1, textbox->max_length, corec.GetNicefontShader()) 
			+ corec.GUI_StringWidth("_", textbox->element->char_height, textbox->element->char_height, 1, textbox->max_length, corec.GetNicefontShader()) > textbox->element->width)
				textbox->pos++;

	if (textbox->variable)
	{
		corec.Cvar_Set(textbox->variable, textbox->buffer);
		textbox->variableModifiedCount = corec.Cvar_GetModifiedCount(textbox->variable);	
	}

	return true;
}

char	*GUI_Textbox_GetValue(gui_element_t *obj)
{
	gui_textbox_t *textbox = GUI_GETWIDGET(obj);
	if (!textbox)
		return "";

	return textbox->buffer;
}

void	GUI_Textbox_Param_Cmd(gui_element_t *obj, int argc, char *argv[])
{
	gui_textbox_t *textbox;
	int oldBorder;

	if (argc < 2)
	{
		corec.Console_Printf("textbox param <panel:object> <param> <args>\n");
		corec.Console_Printf("  params:\n");
		corec.Console_Printf("    commit_cmd - command to exec if they hit enter (success)\n");
		corec.Console_Printf("    abort_cmd  - command to exec if they hit esc (abort)\n");
		corec.Console_Printf("    char_height - height of the text\n");
		corec.Console_Printf("    border     - size of the border to draw around the widget\n");
		corec.Console_Printf("    dropshadow           - set to 1 to draw a dropshadow\n");
		corec.Console_Printf("    dropshadowoffset\n");
		corec.Console_Printf("    dropshadowcolor r g b\n");
		corec.Console_Printf("    type       - float, int, string\n");
		corec.Console_Printf("    variable   - link the value to a variable\n");
		corec.Console_Printf("    hidden     - set to 1 to hide the contents\n");
		return;
	}

	textbox = corec.GUI_GetUserdata(class_name, obj);

	if (!textbox)
		return;

	if (strcmp(argv[0], "commit_cmd") == 0)
	{
		strncpy(textbox->exec_cmd, argv[1], CMD_MAX_LENGTH);
	}
	else if (strcmp(argv[0], "abort_cmd") == 0)
	{
		strncpy(textbox->abort_cmd, argv[1], CMD_MAX_LENGTH);
	}
    else if (strcmp(argv[0], "hidden") == 0)
	{
		if (argc > 1)
		{
			textbox->hide_input = atoi(argv[1]);
		}
		else
		{
			corec.Console_Printf("Error: Not enough parameters.  You must specify 0 or 1\n");
		}
	}
    else if (strcmp(argv[0], "dropshadow") == 0)
	{
		if (argc > 1)
		{
			textbox->dropshadow = atoi(argv[1]);
		}
		else
		{
			corec.Console_Printf("Error: Not enough parameters.  You must specify 0 or 1\n");
		}
	}
	else if (strcmp(argv[0], "dropshadowcolor") == 0)
	{
		if (argc > 3)
		{
			textbox->dropshadowcolor[0] = atof(argv[1]);
			textbox->dropshadowcolor[1] = atof(argv[2]);
			textbox->dropshadowcolor[2] = atof(argv[3]);
		}
		else
		{
			corec.Console_Printf("Error: Not enough parameters.  You must specify the text r g b values\n");
		}
	}
	else if (strcmp(argv[0], "dropshadowoffset") == 0)
	{
		if (argc > 3)
		{
			textbox->dropshadowoffset = atoi(argv[1]);
		}
		else
		{
			corec.Console_Printf("Error: Not enough parameters.  You must specify the offset value\n");
		}
	}
	else if (strcmp(argv[0], "border") == 0)
	{
		oldBorder = textbox->border;
		textbox->border = atoi(argv[1]);
		corec.GUI_Resize (obj,
			textbox->element->width + textbox->border*2 - oldBorder*2 + TEXTBOX_SPACING_Y*2,
			obj->char_height + textbox->border*2 + TEXTBOX_SPACING_X*2);
	}
	else if (strcmp(argv[0], "variable") == 0)
	{
		strncpySafe(textbox->variable, argv[1], sizeof(textbox->variable));
	}
	else if (strcmp(argv[0], "prefix") == 0)
	{
		strncpySafe(textbox->prefix, argv[1], sizeof(textbox->prefix));
	}
	else if (strcmp(argv[0], "type") == 0)
	{
		if (strcmp(argv[1], "float") == 0)
		{
			textbox->type = INPUT_TYPE_FLOAT;
		}
		else if (strcmp(argv[1], "int") == 0)
		{
			textbox->type = INPUT_TYPE_INT;
		}
		else if (strcmp(argv[1], "string") == 0)
		{
			textbox->type = INPUT_TYPE_STRING;
		}
	}
	else if (stricmp(argv[0], "keepInput") == 0)
	{
		textbox->keepInput = atoi(argv[1]);
	}

}


//textbox "name" x y size max_length variable cmd textwidth textheight
void	*GUI_Textbox_Create(gui_element_t *obj, int argc, char *argv[])
{
	gui_textbox_t *textbox;

	if (argc < 5)
	{
		corec.Console_Printf("syntax: create textbox name x y width size var [max_length textheight]\n");
		return NULL;
	}

	corec.GUI_SetClass(obj, class_name);
	corec.GUI_SetName(obj, argv[0]);
	corec.GUI_Move (obj, atoi(argv[1]), atoi(argv[2]) );

	textbox = corec.GUI_Malloc(sizeof (gui_textbox_t));

	if (!textbox)
	{
		corec.Console_Printf("Textbox error: couldn't enough space to hold textbox\n");
		return NULL; 		
	}

	corec.GUI_SetUserdata(class_name, obj, textbox);
	textbox->element = obj;

	strncpySafe(textbox->variable, argv[5], sizeof(textbox->variable));	
	strcpy(textbox->exec_cmd, "");
	strcpy(textbox->abort_cmd, "");

	textbox->type = INPUT_TYPE_STRING;

	if (argc > 6)
	{
		textbox->max_length = atoi(argv[6]);
	} else {
		textbox->max_length = 1023;
	}

	if (argc > 7)
	{
		obj->char_height =	atoi(argv[7]);
	}
	textbox->border = TEXTBOX_BORDER;

	corec.GUI_Resize (obj,
		atoi(argv[3]) + textbox->border*2 + TEXTBOX_SPACING_X*2,
		obj->char_height + textbox->border*2 + TEXTBOX_SPACING_Y*2);

	obj->interactive = true;
	obj->hide = GUI_HideTextbox;
	obj->draw = GUI_DrawTextbox;
	obj->move = GUI_MoveTextbox;
	obj->mousedown = GUI_Textbox_Mousedown;
	obj->focused = GUI_Textbox_Activate;
	obj->unfocused = GUI_Textbox_Deactivate;
	obj->loseInput = GUI_Textbox_Deactivate;

	obj->param = GUI_Textbox_Param_Cmd;

	obj->getvalue = GUI_Textbox_GetValue;

	obj->loseFocusOnMouseout = false;

	memset(textbox->buffer, 0, GUI_TEXTBOX_BUFFER_SIZE);
	textbox->cursor = 0;
	textbox->pos = 0;

	M_SetVec3(textbox->dropshadowcolor, 0, 0, 0);
	textbox->dropshadowoffset = 2;
	textbox->dropshadow = 1;

	textbox->parent = NULL;

	textbox->hide_input = false;	

	return textbox;
}

void	GUI_Textbox_Commit_Cmd(int argc, char *argv[])
{
	gui_textbox_t *textbox;

	if (argc < 1)
	{
		corec.Console_Printf("textbox commit <panel:object>\n");
		return;
	}

	textbox = corec.GUI_GetClass(argv[0], class_name);

	if (!textbox)
		return;

	GUI_Textbox_Commit(textbox);
}

void	GUI_Textbox_Abort_Cmd(int argc, char *argv[])
{
	gui_textbox_t *textbox;

	if (argc < 1)
	{
		corec.Console_Printf("textbox abort <panel:object>\n");
		return;
	}

	textbox = corec.GUI_GetClass(argv[0], class_name);

	if (!textbox)
		return;

	GUI_Textbox_Abort(textbox->element);
}

void	GUI_Textbox_Activate_Cmd(int argc, char *argv[])
{
	gui_textbox_t *textbox;

	if (argc < 1)
	{
		corec.Console_Printf("textbox activate <panel:object>\n");
		return;
	}

	textbox = corec.GUI_GetClass(argv[0], class_name);

	if (!textbox)
		return;

	GUI_Textbox_Activate(textbox->element);
}

void	GUI_Textbox_Deactivate_Cmd(int argc, char *argv[])
{
	gui_textbox_t *textbox;

	if (argc < 1)
	{
		corec.Console_Printf("textbox activate <panel:object>\n");
		return;
	}

	textbox = corec.GUI_GetClass(argv[0], class_name);

	if (!textbox)
		return;

	GUI_Textbox_Deactivate(textbox->element);
}

void	GUI_Textbox_Clear_Cmd(int argc, char *argv[])
{
	gui_textbox_t *textbox = corec.GUI_GetClass(argv[0], class_name);

	if (!textbox)
		return;

	textbox->cursor = 0;
	textbox->pos = 0;
	memset(textbox->buffer, 0, GUI_TEXTBOX_BUFFER_SIZE); 

}

void	GUI_Textbox_Cmd(int argc, char *argv[])
{
	if (!argc)
	{
		corec.Console_Printf("textbox <command> <args>\n");
		corec.Console_Printf("  commands:\n");
		corec.Console_Printf("    list     - lists all the textboxes\n");
		corec.Console_Printf("    activate - activates a textbox (causes it to focus and start reading input)\n");
		corec.Console_Printf("    deactivate - deactivates a textbox (causes it to lose focus and stop reading input)\n");
		corec.Console_Printf("    abort    - deactivates a textbox (causes it to lose focus and stop reading input)\n");
		corec.Console_Printf("    commit   - \"commits\" a textbox (causes it to lose focus and stop reading input, and executes the exec_cmd)\n");
		corec.Console_Printf("    param	  - set parameters for this widget\n");
		return;
	}

	if (strcmp(argv[0], "list") == 0)
	{
		corec.GUI_List_Cmd(1, &class_name);
	} 
	else if (strcmp(argv[0], "activate") == 0)
	{
		GUI_Textbox_Activate_Cmd(argc-1, &argv[1]);
	} 
	else if (strcmp(argv[0], "deactivate") == 0)
	{
		GUI_Textbox_Deactivate_Cmd(argc-1, &argv[1]);
	} 
	else if (strcmp(argv[0], "abort") == 0)
	{
		GUI_Textbox_Abort_Cmd(argc-1, &argv[1]);
	} 
	else if (strcmp(argv[0], "commit") == 0)
	{
		GUI_Textbox_Commit_Cmd(argc-1, &argv[1]);
	} 
	else if (strcmp(argv[0], "param") == 0)
	{
		gui_element_t *obj;

		if (argc > 2)
		{
			obj = corec.GUI_GetObject(argv[1]);
			GUI_Textbox_Param_Cmd(obj, argc-2, &argv[2]);
		}
	} 
	else if (strcmp(argv[0], "clear") == 0)
	{
		GUI_Textbox_Clear_Cmd(argc-1, &argv[1]);
	}
	else 
	{
		corec.Console_Printf("textbox error:  no textbox command %s\n", argv[0]);
	}
}

void	GUI_Textbox_Init()
{
	corec.Cmd_Register("textbox", GUI_Textbox_Cmd);

	corec.GUI_RegisterClass(class_name, GUI_Textbox_Create);

	memset(hidden_chars, HIDDEN_CHAR, MAX_HIDDEN_CHARS * sizeof(char));
	hidden_chars[MAX_HIDDEN_CHARS-1] = 0;
}
