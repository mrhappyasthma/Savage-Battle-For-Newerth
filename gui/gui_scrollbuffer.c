
#include "../toplayer/tl_shared.h"
#include "gui_scrollbuffer.h"

static char *class_name = "scrollbuffer";

//we can assume each char is at least 3 pixels wide
#define MIN_CHAR_WIDTH 3
#define LINE_VALUE_LENGTH 64

#define X_SPACING 1

void	GUI_ScrollBuffer_Scrollvar(gui_scrollbuffer_t *scrollbuffer);

void	GUI_ScrollBuffer_Draw(gui_element_t *obj, int x, int y)
{
	vec4_t				textcolor, selectedtextcolor;
	gui_scrollbuffer_t	*scrollbuffer;
	int					i, row;
	bool				altcolor = false;
	residx_t			fontshader = corec.GetNicefontShader();
	residx_t			whiteshader = 0;

	scrollbuffer = corec.GUI_GetUserdata(class_name, obj);
	if (!scrollbuffer)
		return;

	scrollbuffer->visible_lines = floor(obj->height/scrollbuffer->icon_height);

	if (scrollbuffer->scrollvar)
		GUI_ScrollBuffer_Scrollvar(scrollbuffer);
	
	corec.GUI_SetRGBA(1,1,1, obj->alpha);

	if (scrollbuffer->thickness)
	{
		corec.GUI_LineBox2d_S(0,0, 
				obj->width, 
				obj->height, 
				scrollbuffer->thickness);
	}

	if (scrollbuffer->icon_height < obj->char_height)
		scrollbuffer->icon_height = obj->char_height;

	textcolor[3] = obj->alpha;
	M_CopyVec3(obj->textcolor, textcolor);
	corec.Draw_SetColor(textcolor);

	M_CopyVec3(scrollbuffer->selected_textcolor, selectedtextcolor);

	for (i = 0; 
			i < scrollbuffer->visible_lines 
			&& i < scrollbuffer->rows 
			&& (scrollbuffer->wraparound || scrollbuffer->visible_start_line + i < scrollbuffer->rows); i++)
	{
		row = (scrollbuffer->visible_start_line + i) % scrollbuffer->rows;

		if (strcmp(scrollbuffer->buffer[row], "") == 0)
			continue;

		if (scrollbuffer->delay_msecs > 0)
	 		textcolor[3] = MAX(0, 1 - (corec.Milliseconds() - scrollbuffer->delay_msecs - scrollbuffer->line_times[row]) / 5000.0);
		else
			textcolor[3] = obj->alpha;
		
		if (scrollbuffer->selected_line == row)
		{
			altcolor = true;
			corec.Draw_SetColor(scrollbuffer->selected_bgcolor);
			if (!whiteshader)
				whiteshader = corec.GetWhiteShader();
			corec.GUI_Quad2d_S(scrollbuffer->thickness, scrollbuffer->thickness + scrollbuffer->icon_height*i, obj->width - scrollbuffer->thickness, scrollbuffer->icon_height, whiteshader);

			//M_CopyVec3(scrollbuffer->selected_textcolor, textcolor);
			corec.Draw_SetColor(selectedtextcolor);

			corec.GUI_DrawShadowedString(scrollbuffer->thickness+X_SPACING, 
				scrollbuffer->thickness + scrollbuffer->icon_height * i, 
				scrollbuffer->buffer[row], obj->char_height, scrollbuffer->icon_height,
				1, obj->width - scrollbuffer->thickness*2 - X_SPACING*2, fontshader, textcolor[0], textcolor[1], textcolor[2], obj->alpha * textcolor[3]);
		}
		else if (scrollbuffer->dropshadow)
		{
			//M_CopyVec3(obj->textcolor, textcolor);

			corec.GUI_DrawShadowedString(scrollbuffer->thickness+X_SPACING, 
							scrollbuffer->thickness + scrollbuffer->icon_height * i,
					scrollbuffer->buffer[row], obj->char_height, scrollbuffer->icon_height,
					1, obj->width - scrollbuffer->thickness*2 - X_SPACING*2, fontshader, textcolor[0], textcolor[1], textcolor[2], obj->alpha * textcolor[3]);
		}
		else
		{
			//M_CopyVec3(obj->textcolor, textcolor);
			if (altcolor)
			{
				corec.Draw_SetColor(textcolor);
				altcolor = false;
			}

			corec.GUI_DrawString(scrollbuffer->thickness+X_SPACING, 
				scrollbuffer->thickness + scrollbuffer->icon_height * i, 
				scrollbuffer->buffer[row], obj->char_height, scrollbuffer->icon_height,
				1, obj->width - scrollbuffer->thickness*2 - X_SPACING*2, fontshader);
		}
	}
}

void    GUI_ScrollBuffer_MouseDown(gui_element_t *obj, mouse_button_enum button, int x, int y)
{
	gui_scrollbuffer_t *scrollbuffer;
	
	scrollbuffer = corec.GUI_GetUserdata(class_name, obj);
	
	if (!scrollbuffer)
		return;
	
	if (scrollbuffer->selectable)
	{
		scrollbuffer->selected_line = (y - scrollbuffer->thickness) / scrollbuffer->icon_height + scrollbuffer->visible_start_line;
	
		if (scrollbuffer->variable[0])
		{
			corec.Cvar_Set(scrollbuffer->variable, scrollbuffer->linevalue[scrollbuffer->selected_line]);
		}

		if (button == MOUSE_RBUTTON)
		{
			if (scrollbuffer->rclick_cmd[0])
				corec.Cmd_Exec(scrollbuffer->rclick_cmd);
		}
	}
}

void	GUI_ScrollBuffer_Scrollvar(gui_scrollbuffer_t *scrollbuffer)
{
	int numLines;
	int scrollvar;
	
	if (!scrollbuffer->scrollvar[0])
		return;

	scrollvar = corec.Cvar_GetValue(scrollbuffer->scrollvar);

	if (scrollbuffer->wraparound)
		numLines = scrollbuffer->last_line - scrollbuffer->visible_lines;
	else
		numLines = MAX(0, scrollbuffer->last_line - scrollbuffer->visible_lines);

	if (numLines)
	{
		if (scrollbuffer->anchor_to_current_line && scrollbuffer->wraparound)
		{
			scrollbuffer->visible_start_line = ((scrollbuffer->current_line - (int)(numLines*(1- scrollvar)) - scrollbuffer->visible_lines) + scrollbuffer->rows*2) % scrollbuffer->rows;
		}
		else
		{
			scrollbuffer->visible_start_line = floor(scrollvar * numLines + 0.5);
			if (scrollbuffer->visible_start_line + scrollbuffer->visible_lines > scrollbuffer->last_line)
				scrollbuffer->visible_start_line = scrollbuffer->last_line - (scrollbuffer->visible_lines - 1);
			if (scrollbuffer->visible_start_line < 0)
				scrollbuffer->visible_start_line = 0;
		}
	}
}

void	GUI_ScrollBuffer_Move(gui_element_t *obj, int x, int y)
{
	corec.GUI_Move(obj, x, y);
}

void	GUI_ScrollBuffer_Clear(gui_element_t *obj)
{
	gui_scrollbuffer_t *scrollbuffer;
	int i;

	scrollbuffer = corec.GUI_GetUserdata(class_name, obj);
	if (!scrollbuffer)
		return;

	for (i = 0; i < scrollbuffer->rows; i++)
	{
		strcpy(scrollbuffer->buffer[i], "");
	}
	scrollbuffer->visible_start_line = 0;
	scrollbuffer->last_line = 0;
	scrollbuffer->current_line = 0;
}

void	GUI_ScrollBuffer_Resize(gui_element_t *obj, int width, int height)
{
	gui_scrollbuffer_t *scrollbuffer;

	scrollbuffer = corec.GUI_GetUserdata(class_name, obj);
	if (!scrollbuffer)
		return;
	
	corec.GUI_Resize(obj, width, height);
	
	scrollbuffer->visible_lines = floor(height/scrollbuffer->icon_height);
}

void	GUI_ScrollBuffer_Realloc(gui_element_t *obj, int rows, int width)
{
	gui_scrollbuffer_t *scrollbuffer;
	int i;

	scrollbuffer = corec.GUI_GetUserdata(class_name, obj);
	if (!scrollbuffer)
		return;

	for (i = 0; i < scrollbuffer->rows; i++)
	{
		corec.GUI_Free(scrollbuffer->buffer[i]);
		corec.GUI_Free(scrollbuffer->linevalue[i]);
	}
		
	scrollbuffer->rows = rows;
	scrollbuffer->cols = width/MIN_CHAR_WIDTH;
	
	scrollbuffer->buffer = corec.GUI_ReAlloc(scrollbuffer->buffer, scrollbuffer->rows * sizeof (char *));
	scrollbuffer->linevalue = corec.GUI_ReAlloc(scrollbuffer->linevalue, scrollbuffer->rows * sizeof (char *));
	for (i = 0; i < scrollbuffer->rows; i++)
	{
		scrollbuffer->buffer[i] = corec.GUI_Malloc(scrollbuffer->cols + 1);
		scrollbuffer->linevalue[i] = corec.GUI_Malloc(LINE_VALUE_LENGTH);
		strcpy(scrollbuffer->buffer[i], "");
		strcpy(scrollbuffer->linevalue[i], "");
	}
	
	scrollbuffer->line_times = corec.GUI_ReAlloc(scrollbuffer->line_times, scrollbuffer->rows * sizeof (int));
	
	//uncommenting this makes the box HUGE
	//corec.GUI_Resize(obj, width + 2*scrollbuffer->thickness, rows * obj->char_height + 2*scrollbuffer->thickness);
}

//messy line-wrapping here
void	GUI_ScrollBuffer_Printf(gui_scrollbuffer_t *scrollbuffer, char *message, int indent, char *value)
{
	char line[256];
	char *tmp;
	int messagelen, lineend, n, message_offset, current_line, length, numLines;

	messagelen = strlen(message);
	while ((message[messagelen] == ' ' 
			|| message[messagelen] == '\n'
			|| message[messagelen] == '\t'
			|| message[messagelen] == '\0')
		   && messagelen > 0)
	{
		messagelen--;
	}
	messagelen++;

	if (messagelen <= 0)
		return;
	
	//do we even want to linewrap?
	if (!scrollbuffer->wrap)
	{
		n = 0;
		while (n < scrollbuffer->rows)
		{
			if (scrollbuffer->buffer[n][0] == 0)
			{
				current_line = n;
				strncpy(scrollbuffer->buffer[n], message, scrollbuffer->cols);
				scrollbuffer->buffer[n][scrollbuffer->cols-1] = 0;
				strncpy(scrollbuffer->linevalue[n], value, LINE_VALUE_LENGTH);
				scrollbuffer->linevalue[n][LINE_VALUE_LENGTH-1] = 0;
				scrollbuffer->line_times[current_line] = corec.Milliseconds();

				if (scrollbuffer->selected_line == current_line && scrollbuffer->variable[0])
					corec.Cvar_Set(scrollbuffer->variable, scrollbuffer->linevalue[scrollbuffer->selected_line]);

				scrollbuffer->last_line = MAX(scrollbuffer->last_line, current_line);

				return;
			}
			n++;
		}
		return;
	}
	
	current_line = scrollbuffer->current_line;

	if (indent >= scrollbuffer->cols)
	{
		corec.Console_DPrintf("you have a widget (%s) that is only %i columns wide, and you're trying to print with an index of %i\n", scrollbuffer->element->name, scrollbuffer->cols, indent);
		return;
	}
	
	//message_offset walks along the message to say where we are
	//lineend walks backwards down the message to find where to break the current line
	message_offset = 0;
	while (message_offset < messagelen)
	{

		//is this the first line of this message?
		if (message_offset)
		{
	  		//start lineend at the current offset + the width of the scrollbuffer + indent size
			lineend = message_offset + MIN(255-indent, scrollbuffer->element->width/MIN_CHAR_WIDTH - indent);

			//add the indent of spaces
			for (n = 0;  n < indent; n++)
				scrollbuffer->buffer[current_line][n] = ' ';
			scrollbuffer->buffer[current_line][indent] = '\0';
		}
		else
		{
	  		//start lineend at the current offset + the width of the scrollbuffer 
			lineend = message_offset + MIN(255-indent, scrollbuffer->element->width/MIN_CHAR_WIDTH);
			scrollbuffer->buffer[current_line][0] = '\0';
		}

		//this 3 lines handles messages with \n's in them
		tmp = strchr(&message[message_offset], '\n');
		if (tmp)
		{
			if (tmp - &message[message_offset] < lineend - message_offset)
				lineend = message_offset + tmp - &message[message_offset];
		}
		
		if (lineend - message_offset + 1 <= 0)
			return; //abort!  abort!
		
		//add in the line we just calculated
		strncpy(line, &message[message_offset], lineend - message_offset + 1);
		line[lineend - message_offset + 1] = 0;

		while (lineend - message_offset > 0
			&& corec.GUI_StringWidth(line, scrollbuffer->element->char_height, scrollbuffer->icon_height, 1, 
					scrollbuffer->element->width - scrollbuffer->thickness*2 + 10, 
					corec.GetNicefontShader()) 
				> scrollbuffer->element->width)
		{
			lineend--;
			line[lineend - message_offset + 1] = 0;
		}
			
		//if lineend is past the end of the message, move it to the last char of the message
		if (lineend > messagelen)
			lineend = messagelen - 1;
		else
		{
			//walk backwards until we find the first whitespace char, or we hit
			// the beginning of the line
			while (message[lineend] != ' ' 
				   && message[lineend] != '\n'
				   && message[lineend] != '\t'
				   && (lineend - message_offset < 5 || strncmp(&message[message_offset + lineend - 5], "^clan", 5) != 0)
				   && (lineend - message_offset < 5 || strncmp(&message[message_offset + lineend - 5], "^econ", 5) != 0)
				   && lineend > message_offset + 1)
			{
				lineend--;
			}

			//is it a solid line of non-whitespace?
			// if so, just cut it at the end of the line
			if (lineend <= message_offset + 1)
			{
				if (message_offset)
					lineend = message_offset + scrollbuffer->element->width/MIN_CHAR_WIDTH - indent;
				else
					lineend = message_offset + scrollbuffer->element->width/MIN_CHAR_WIDTH;
			}
		}

		//add in the line we just calculated
		length = MIN(scrollbuffer->cols - strlen(scrollbuffer->buffer[current_line]), lineend - message_offset + 1);
		strncpy(line, &message[message_offset], length);
		line[length] = 0;

		strcat(scrollbuffer->buffer[current_line], line);

		//set the line time for fadeouts
		scrollbuffer->line_times[current_line] = corec.Milliseconds();

		//increment the current line number
		current_line = (current_line + 1) % scrollbuffer->rows;
		scrollbuffer->current_line = current_line;
		
		message_offset = lineend + 1;
	}

	scrollbuffer->last_line = MAX(scrollbuffer->last_line, current_line);

	if (scrollbuffer->anchor_to_current_line && !scrollbuffer->scrollvar[0])
	{
		if (scrollbuffer->wraparound)
			numLines = MAX(0, scrollbuffer->last_line - scrollbuffer->visible_lines);
		else
			numLines = MAX(0, scrollbuffer->last_line - scrollbuffer->visible_lines);

		scrollbuffer->visible_start_line = ((scrollbuffer->current_line - numLines - scrollbuffer->visible_lines) + scrollbuffer->rows*2) % scrollbuffer->rows;
	}
}

void	GUI_ScrollBuffer_Destroy(gui_element_t *obj)
{
	gui_scrollbuffer_t *scrollbuffer;
	int i;

	scrollbuffer = corec.GUI_GetUserdata(class_name, obj);
	if (!scrollbuffer)
		return;
	
	for (i = 0; i < scrollbuffer->rows; i++)
	{
		corec.GUI_Free(scrollbuffer->buffer[i]);
		corec.GUI_Free(scrollbuffer->linevalue[i]);
	}
	corec.GUI_Free(scrollbuffer->buffer);
	corec.GUI_Free(scrollbuffer->linevalue);
	corec.GUI_Free(scrollbuffer->line_times);
}

void	GUI_ScrollBuffer_Param(gui_element_t *obj, int argc, char *argv[])
{
	gui_scrollbuffer_t *scrollbuffer;

	if (argc < 2)
	{
		corec.Console_Printf("scrollbuffer param <panel:object> <param> <value>\n");
		corec.Console_Printf("Available params:\n");
		corec.Console_Printf("     thickness			- (line thickness of chat box)\n");
		corec.Console_Printf("     delay_secs			- (# seconds delay before text fades)\n");
		corec.Console_Printf("     char_height			- (height of character [aka font size])\n");
		corec.Console_Printf("     icon_height			- (height of icons)\n");
		corec.Console_Printf("     textcolor r g b		- the color to draw the text\n");
		corec.Console_Printf("     selected_textcolor r g b	- the color to draw the selected text\n");
		corec.Console_Printf("     selected_bgcolor r g b	- the color to draw the background of selected text\n");
		corec.Console_Printf("	   dropshadow			- set to 1 to draw a dropshadow\n");
		corec.Console_Printf("	   dropshadowoffset		- set the offset of the dropshadow\n");
		corec.Console_Printf("	   dropshadowcolor r g b- set the color of the dropshadow\n");
		corec.Console_Printf("     rows					- (number of rows in scrollbuffer)\n");
		corec.Console_Printf("     linewrap				- turn on/off linewrapping\n");
		corec.Console_Printf("     variable				- the variable to set the value to\n");
		corec.Console_Printf("     scrollvariable		- the variable to determine the scrolling amount\n");
		corec.Console_Printf("     rclick_cmd			- the command to run when someone right-clicks\n");
		corec.Console_Printf("     url					- the url to grab data from to put in the scrollbuffer\n");
		corec.Console_Printf("     selectable			- whether the lines of the scrollbuffer are selectable\n");
		corec.Console_Printf("     wrap					- whether the lines of the scrollbuffer wrap around\n");
		corec.Console_Printf("     wraparound			- whether the scrollbuffer wraps around\n");
		corec.Console_Printf("     delay_secs           - (# seconds delay before text fades)\n");
		return;
	}

	scrollbuffer = corec.GUI_GetUserdata(class_name, obj);

	if (!scrollbuffer)
		return;

	if (strcmp(argv[0], "thickness") == 0)
	{
		scrollbuffer->thickness = atoi(argv[1]);
	} 
	else if (strcmp(argv[0], "delay_secs") == 0)
	{
		scrollbuffer->delay_msecs = atof(argv[1]) * 1000;
	}
	else if (strcmp(argv[0], "wraparound") == 0)
	{
		scrollbuffer->wraparound = atoi(argv[1]) || (strcmp(argv[1], "true")==0);
	} 
	else if (strcmp(argv[0], "anchor_to_current_line") == 0)
	{
		scrollbuffer->anchor_to_current_line = atoi(argv[1]) || (strcmp(argv[1], "true")==0);
	} 
	else if (strcmp(argv[0], "dropshadow") == 0)
	{
		if (argc > 3)
		{
			scrollbuffer->dropshadow = atoi(argv[1]);			
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
			scrollbuffer->dropshadowcolor[0] = atof(argv[1]);
			scrollbuffer->dropshadowcolor[1] = atof(argv[2]);
			scrollbuffer->dropshadowcolor[2] = atof(argv[3]);
		}
		else
		{
			corec.Console_Printf("Error: Not enough parameters.  You must specify the text r g b values\n");
		}
	}
	else if (strcmp(argv[0], "selected_textcolor") == 0 || strcmp(argv[0], "highlight_color") == 0)
	{
		if (argc > 3)
		{
			scrollbuffer->selected_textcolor[0] = atof(argv[1]);
			scrollbuffer->selected_textcolor[1] = atof(argv[2]);
			scrollbuffer->selected_textcolor[2] = atof(argv[3]);
		}
		else
		{
			corec.Console_Printf("Error: Not enough parameters.  You must specify the text r g b values\n");
		}
	}
	else if (strcmp(argv[0], "selected_bgcolor") == 0)
	{
		if (argc > 3)
		{
			scrollbuffer->selected_bgcolor[0] = atof(argv[1]);
			scrollbuffer->selected_bgcolor[1] = atof(argv[2]);
			scrollbuffer->selected_bgcolor[2] = atof(argv[3]);
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
			scrollbuffer->dropshadowoffset = atoi(argv[1]);			
		}
		else
		{
			corec.Console_Printf("Error: Not enough parameters.  You must specify the offset value\n");
		}
	}
	else if (strcmp(argv[0], "rows") == 0)
	{
		if (argc > 1)
		{
			GUI_ScrollBuffer_Realloc(scrollbuffer->element, atoi(argv[1]), obj->width);
		}
	}
	else if (strcmp(argv[0], "wrap") == 0
			|| strcmp(argv[0], "linewrap") == 0)
	{
		if (argc > 1)
		{
			if (strcmp(argv[1],"true") == 0
				|| strcmp(argv[1],"1") == 0)
			{
				scrollbuffer->wrap = true;
			}
			else
			{
				scrollbuffer->wrap = false;
			}
		}
	}
	else if (strcmp(argv[0], "wraparound") == 0)
	{
		if (argc > 1)
		{
			if (strcmp(argv[1],"true") == 0
				|| strcmp(argv[1],"1") == 0)
			{
				scrollbuffer->wraparound = true;
			}
			else
			{
				scrollbuffer->wraparound = false;
			}
		}
	}
	else if (strcmp(argv[0], "variable") == 0)
	{
		if (argc > 1)
		{
			strncpySafe(scrollbuffer->variable, argv[1], sizeof(scrollbuffer->variable));
		}
	}
	else if (strcmp(argv[0], "scrollvariable") == 0)
	{
		if (argc > 1)
		{
			strncpySafe(scrollbuffer->scrollvar, argv[1], sizeof(scrollbuffer->scrollvar));			
		}
	}
	else if (strcmp(argv[0], "rclick_cmd") == 0)
	{
		if (argc > 1)
		{
			strncpy(scrollbuffer->rclick_cmd, argv[1], _MAX_COMMAND_LENGTH);
			scrollbuffer->rclick_cmd[_MAX_COMMAND_LENGTH-1] = 0;
		}
	}
	else if (strcmp(argv[0], "url") == 0)
	{
		char buf[2048];
		char *tmp, *end;

		corec.HTTP_GetText(argv[1], buf, 2048);
		buf[2047] = 0;
		buf[2046] = 0;

		GUI_ScrollBuffer_Clear(scrollbuffer->element);

		tmp = buf;
		while (*tmp && (end = strchr(tmp, '\n')))
		{
			end[0] = 0;
			GUI_ScrollBuffer_Printf(scrollbuffer, tmp, 0, "0");
			tmp = end + 1;
		}
		
	}
	else if (strcmp(argv[0], "icon_height") == 0)
	{
		if (argc > 1)
		{
			scrollbuffer->icon_height = MAX(atoi(argv[1]), scrollbuffer->element->char_height);
		}
	}
	else if (strcmp(argv[0], "selectable") == 0)
	{
		if (argc > 1)
		{
			scrollbuffer->selectable = atoi(argv[1]);
			if (!scrollbuffer->selectable)
			{
				scrollbuffer->selected_line = -1;
				obj->interactive = false;
			}
		}
	}

}

void	*GUI_ScrollBuffer_Create(gui_element_t *obj, int argc, char *argv[])
{
	gui_scrollbuffer_t *scrollbuffer;

	if (argc < 5)
	{
		corec.Console_Printf("syntax: scrollbuffer name x y width height\n");
		return NULL;
	}

	corec.GUI_SetClass(obj, class_name);
	corec.GUI_SetName(obj, argv[0]);
	corec.GUI_Move (obj, atoi(argv[1]), atoi(argv[2]) );

	scrollbuffer = corec.GUI_Malloc(sizeof (gui_scrollbuffer_t));

	if (!scrollbuffer)
	{
		corec.Console_Printf("ScrollBuffer error: couldn't alloc enough space to hold scrollbuffer\n");
		return NULL; 		
	}

	memset(scrollbuffer, 0, sizeof(gui_scrollbuffer_t));
	corec.GUI_SetUserdata(class_name, obj, scrollbuffer);
	scrollbuffer->element = obj;

	scrollbuffer->thickness = 0;

	obj->interactive = true;
	obj->destroy = GUI_ScrollBuffer_Destroy;
	obj->draw = GUI_ScrollBuffer_Draw;
	obj->move = GUI_ScrollBuffer_Move;
	obj->param = GUI_ScrollBuffer_Param;
	obj->mousedown = GUI_ScrollBuffer_MouseDown;
	obj->notify = NULL;

	scrollbuffer->parent = NULL;

	scrollbuffer->wrap = false; //whether to line-wrap lines
	scrollbuffer->wraparound = false; //whether to wrap it around the bottom, i.e. be a circular buffer type setup
	scrollbuffer->visible_start_line = 0;
	scrollbuffer->selected_line = 0;
	scrollbuffer->current_line = 0;

	M_SetVec3(scrollbuffer->dropshadowcolor, 0, 0, 0);
	scrollbuffer->dropshadowoffset = 1;
	scrollbuffer->dropshadow = 1;
	scrollbuffer->selectable = true;

	M_SetVec3(scrollbuffer->selected_textcolor, 0, 0, 0);
	scrollbuffer->selected_textcolor[3] = obj->alpha;
	M_SetVec3(scrollbuffer->selected_bgcolor, 0.7, 0.6, 0);
	scrollbuffer->selected_bgcolor[3] = obj->alpha;

	//rows will get set inside the Resize function
	scrollbuffer->rows = 0;
	scrollbuffer->buffer = 0;
	scrollbuffer->linevalue = 0;
	scrollbuffer->rclick_cmd[0] = 0;
	scrollbuffer->icon_height = obj->char_height;
	scrollbuffer->anchor_to_current_line = false;

	scrollbuffer->delay_msecs = 0;

	GUI_ScrollBuffer_Realloc(obj, atoi(argv[4])/scrollbuffer->icon_height, atoi(argv[3])/MIN_CHAR_WIDTH);
	GUI_ScrollBuffer_Resize(obj, atoi(argv[3]), atoi(argv[4]));

	return scrollbuffer;
}


void	GUI_ScrollBuffer_Cmd(int argc, char *argv[])
{
	gui_scrollbuffer_t *scrollbuffer;
	gui_element_t	*obj;
	int numLines, inc;
	float val;
	
	if (!argc)
	{
		corec.Console_Printf("scrollbuffer <command> <args>\n");
		corec.Console_Printf("  commands:\n");
		corec.Console_Printf("    list\n");
		corec.Console_Printf("    param\n");
		corec.Console_Printf("    print\n");
		corec.Console_Printf("    scroll\n");
		return;
	}

	if (strcmp(argv[0], "list") == 0)
	{
		corec.GUI_List_Cmd(1, &class_name);
	}	
	else if (strcmp(argv[0], "scroll") == 0)
	{
		scrollbuffer = corec.GUI_GetClass(argv[1], class_name);
		if (scrollbuffer && argc > 1)
		{
			numLines = MAX(0, scrollbuffer->last_line - scrollbuffer->visible_lines + 1);
			if (numLines > 0)
			{
				inc = atoi(argv[2]);
				if (scrollbuffer->scrollvar)
				{
					val = corec.Cvar_GetValue(scrollbuffer->scrollvar) + ((float)inc / numLines);
					if (val < 0)
						val = 0;
					if (val > 1)
						val = 1;
					corec.Cvar_SetValue(scrollbuffer->scrollvar, val);
				}
			}
		}
	}
	else if (strcmp(argv[0], "param") == 0)
	{
		if (argc > 2)
		{
			obj = corec.GUI_GetObject(argv[1]);
			GUI_ScrollBuffer_Param(obj, argc-2, &argv[2]);
		}
		else
		{
			corec.Console_Printf("missing arguments.  Usage: %s param <object> <var> <value>\n", class_name);
		}
	}
	else if (strcmp(argv[0], "print") == 0)
	{
		if (argc > 2)
		{
			scrollbuffer = corec.GUI_GetClass(argv[1], class_name);
			if (scrollbuffer)
				GUI_ScrollBuffer_Printf(scrollbuffer, argv[2], 0, "");
		}
		else
		{
			corec.Console_Printf("Missing object name to print to and the text to print\n");
		}
	}
	else if (strcmp(argv[0], "clear") == 0)
	{
		if (argc > 1)
		{
			obj = corec.GUI_GetObject(argv[1]);
			if (obj)
				GUI_ScrollBuffer_Clear(obj);
			else
				corec.Console_Printf("Object %s not found\n", argv[1]);
		}
		else
		{
			corec.Console_Printf("Missing object name to clear\n");
		}
	}
	else
	{
		corec.Console_Printf("Unknown %s command %s\n", class_name, argv[0]);
	}

}

void	GUI_ScrollBuffer_Init()
{
	corec.Cmd_Register("scrollbuffer", GUI_ScrollBuffer_Cmd);

	corec.GUI_RegisterClass(class_name, GUI_ScrollBuffer_Create);
}
