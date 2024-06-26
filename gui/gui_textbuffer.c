
#include "../toplayer/tl_shared.h"
#include "gui_textbuffer.h"

static char *class_name = "textbuffer";

//we can assume each char is at least 3 pixels wide
#define MIN_CHAR_WIDTH 3

void	GUI_TextBuffer_Draw(gui_element_t *obj, int x, int y)
{
	vec4_t textcolor;
	gui_textbuffer_t *textbuffer;
	int i, row;

	textbuffer = corec.GUI_GetUserdata(class_name, obj);
	if (!textbuffer)
		return;

	corec.GUI_SetRGBA(1,1,1, obj->alpha);

	if (textbuffer->thickness)
	{
		corec.GUI_LineBox2d_S(0,0, 
				obj->width, 
				obj->height, 
				textbuffer->thickness);
	}

    if (textbuffer->icon_height < obj->char_height)
		textbuffer->icon_height = obj->char_height;
	
	M_CopyVec3(obj->textcolor, textcolor);

	textcolor[3] = obj->alpha;

	for (i = 0; i < textbuffer->rows; i++)
	{
		row = i % textbuffer->rows;

		if (strcmp(textbuffer->buffer[row], "") == 0)
			continue;

		if (textbuffer->delay_msecs > 0)
			textcolor[3] = MAX(0, 1 - (corec.Milliseconds() - textbuffer->delay_msecs - textbuffer->line_times[row]) / 5000.0);
		else
			textcolor[3] = obj->alpha;

		if (textbuffer->dropshadow)
		{
			M_CopyVec3(obj->textcolor, textcolor);
			corec.GUI_DrawShadowedString(textbuffer->thickness, textbuffer->thickness + textbuffer->icon_height * i,
					textbuffer->buffer[row], obj->char_height, textbuffer->icon_height,
					1, obj->width, corec.GetNicefontShader(), textcolor[0], textcolor[1], textcolor[2], textcolor[3]);
		}
		else
		{
			M_CopyVec3(obj->textcolor, textcolor);
			corec.Draw_SetColor(textcolor);

			corec.GUI_DrawString(textbuffer->thickness, textbuffer->thickness + textbuffer->icon_height * i, 
				textbuffer->buffer[row], obj->char_height, textbuffer->icon_height,
				1, obj->width, corec.GetNicefontShader());
		}
	}
}

void	GUI_TextBuffer_Move(gui_element_t *obj, int x, int y)
{
	corec.GUI_Move(obj, x, y);
}

void	GUI_TextBuffer_Clear(gui_element_t *obj)
{
	gui_textbuffer_t *textbuffer;
	int i;

	textbuffer = corec.GUI_GetUserdata(class_name, obj);
	if (!textbuffer)
		return;

	for (i = 0; i < textbuffer->rows; i++)
	{
		strcpy(textbuffer->buffer[i], "");
	}
	textbuffer->current_line = 0; //textbuffer->rows-1;
}

void	GUI_TextBuffer_Resize(gui_element_t *obj, int rows, int width)
{
	gui_textbuffer_t *textbuffer;
	int i;

	textbuffer = corec.GUI_GetUserdata(class_name, obj);
	if (!textbuffer)
		return;

	for (i = 0; i < textbuffer->rows; i++)
		corec.GUI_Free(textbuffer->buffer[i]);
		
	textbuffer->rows = rows;
	textbuffer->width = width;
	
	textbuffer->buffer = corec.GUI_ReAlloc(textbuffer->buffer, textbuffer->rows * sizeof (char *));
	for (i = 0; i < textbuffer->rows; i++)
	{
		textbuffer->buffer[i] = corec.GUI_Malloc(width / MIN_CHAR_WIDTH + 1);
		strcpy(textbuffer->buffer[i], "");
	}
	textbuffer->line_times = corec.GUI_ReAlloc(textbuffer->line_times, textbuffer->rows * sizeof (int));
	
	corec.GUI_Resize(obj, width + 2*textbuffer->thickness, rows * textbuffer->icon_height + 2*textbuffer->thickness);
}

//messy line-wrapping here
void	GUI_TextBuffer_Printf(gui_textbuffer_t *textbuffer, char *message, int indent)
{
	char line[256];
	char color[3] = {0};
	char *caret, *tmp;
	int messagelen, lineend, n, message_offset;

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

	/*if (indent >= textbuffer->cols)
	{
		corec.Console_DPrintf("you have a widget (%s) that is only %i columns wide, and you're trying to print with an index of %i\n", textbuffer->element->name, textbuffer->cols, indent);
		return;
	}
	*/
	
	//message_offset walks along the message to say where we are
	//lineend walks backwards down the message to find where to break the current line
	message_offset = 0;
	while (message_offset < messagelen)
	{

		//is this the first line of this message?
		if (message_offset)
		{
	  		//start lineend at the current offset + the width of the textbuffer + indent size
			lineend = message_offset + MIN(255-indent, textbuffer->element->width/MIN_CHAR_WIDTH - indent);

			//add the indent of spaces
			for (n = 0;  n < indent; n++)
				textbuffer->buffer[textbuffer->current_line][n] = ' ';
			textbuffer->buffer[textbuffer->current_line][indent] = '\0';
		}
		else
		{
	  		//start lineend at the current offset + the width of the textbuffer 
			lineend = message_offset + MIN(255-indent, textbuffer->element->width/MIN_CHAR_WIDTH);
			textbuffer->buffer[textbuffer->current_line][0] = '\0';
		}
		strcat(textbuffer->buffer[textbuffer->current_line], color);

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
			&& corec.GUI_StringWidth(line, textbuffer->element->char_height, 
					textbuffer->icon_height, 1, 
					textbuffer->element->width - textbuffer->thickness*2 + 10, 
					corec.GetNicefontShader()) 
				> textbuffer->element->width)
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
					lineend = message_offset + textbuffer->element->width/MIN_CHAR_WIDTH - indent;
				else
					lineend = message_offset + textbuffer->element->width/MIN_CHAR_WIDTH;
			}
		}
		
		//add in the line we just calculated
		strncpy(line, &message[message_offset], lineend - message_offset + 1);
		line[lineend - message_offset + 1] = 0;

		strcat(textbuffer->buffer[textbuffer->current_line], line);

		caret = line;
		while (caret && (caret = strchr(caret, '^')))
		{
			if (strncmp(caret, "^clan", 5) != 0
				&& strncmp(caret, "^econ", 5) != 0)
			{
				color[2] = 0;
				strncpy(color, caret, 2);
			}
			caret++;
		}
		
		//set the time of the current line to now
		textbuffer->line_times[textbuffer->current_line] = corec.Milliseconds();
		//increment the current line number
		textbuffer->current_line = (textbuffer->current_line + 1) % textbuffer->rows;
		
		message_offset = lineend + 1;
	}
}

void	GUI_TextBuffer_Destroy(gui_element_t *obj)
{
	gui_textbuffer_t *textbuffer;
	int i;

	textbuffer = corec.GUI_GetUserdata(class_name, obj);
	if (!textbuffer)
		return;
	
	for (i = 0; i < textbuffer->rows; i++)
	{
		corec.GUI_Free(textbuffer->buffer[i]);
	}
	corec.GUI_Free(textbuffer->buffer);
	corec.GUI_Free(textbuffer->line_times);
}

void	GUI_TextBuffer_Param(gui_element_t *obj, int argc, char *argv[])
{
	gui_textbuffer_t *textbuffer;

	if (argc < 2)
	{
		corec.Console_Printf("textbuffer param <panel:object> <param> <value>\n");
		corec.Console_Printf("Available params:\n");
		corec.Console_Printf("     thickness			- (line thickness of chat box)\n");
		corec.Console_Printf("     delay_secs			- (# seconds delay before text fades)\n");
		corec.Console_Printf("     char_height			- (height of character [aka font size])\n");
		corec.Console_Printf("     icon_height			- (height of icons [aka line size])\n");
		corec.Console_Printf("     textcolor r g b		- the color to draw the text\n");
		corec.Console_Printf("	   dropshadow			- set to 1 to draw a dropshadow\n");
		corec.Console_Printf("	   dropshadowoffset\n");
		corec.Console_Printf("	   dropshadowcolor r g b\n");
		corec.Console_Printf("     rows					- (number of rows in textbuffer)\n");
		corec.Console_Printf("     url					- (url to grab the text from)\n");
		return;
	}

	textbuffer = corec.GUI_GetUserdata(class_name, obj);

	if (!textbuffer)
		return;

	if (strcmp(argv[0], "thickness") == 0)
	{
		textbuffer->thickness = atoi(argv[1]);
		GUI_TextBuffer_Resize(textbuffer->element, textbuffer->rows, textbuffer->width);
	} 
	else if (strcmp(argv[0], "delay_secs") == 0)
	{
		textbuffer->delay_msecs = atof(argv[1]) * 1000;
	}
	else if (strcmp(argv[0], "dropshadow") == 0)
	{
		if (argc > 1)
		{
			textbuffer->dropshadow = atoi(argv[1]);			
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
			textbuffer->dropshadowcolor[0] = atof(argv[1]);
			textbuffer->dropshadowcolor[1] = atof(argv[2]);
			textbuffer->dropshadowcolor[2] = atof(argv[3]);
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
			textbuffer->dropshadowoffset = atoi(argv[1]);			
		}
		else
		{
			corec.Console_Printf("Error: Not enough parameters.  You must specify the offset value\n");
		}
	}
	else if (strcmp(argv[0], "rows") == 0)
	{
		GUI_TextBuffer_Resize(textbuffer->element, atoi(argv[1]), textbuffer->width);
	}
	else if (strcmp(argv[0], "url") == 0)
	{
		char buf[2048];
		char *tmp, *end;
		
		corec.HTTP_GetText(argv[1], buf, 2048);
		buf[2047] = 0;
		buf[2046] = 0;
		
		tmp = buf;
		while (*tmp && (end = strchr(tmp, '\n')))
		{
			end[0] = 0;
			GUI_TextBuffer_Printf(textbuffer, tmp, 0);
			tmp = end + 1;
		}
		
	}
	else if (strcmp(argv[0], "icon_height") == 0)
	{
		if (argc > 1)
		{
			textbuffer->icon_height = MAX(atoi(argv[1]), textbuffer->element->char_height);
		}
	}
}

void	*GUI_TextBuffer_Create(gui_element_t *obj, int argc, char *argv[])
{
	gui_textbuffer_t *textbuffer;

	if (argc < 5)
	{
		corec.Console_Printf("syntax: textbuffer name x y width rows\n");
		return NULL;
	}

	corec.GUI_SetClass(obj, class_name);
	corec.GUI_SetName(obj, argv[0]);
	corec.GUI_Move (obj, atoi(argv[1]), atoi(argv[2]) );

	textbuffer = corec.GUI_Malloc(sizeof (gui_textbuffer_t));

	if (!textbuffer)
	{
		corec.Console_Printf("TextBuffer error: couldn't alloc enough space to hold textbuffer\n");
		return NULL; 		
	}

	corec.GUI_SetUserdata(class_name, obj, textbuffer);
	textbuffer->element = obj;

	textbuffer->thickness = 0;

	obj->interactive = false;
	obj->destroy = GUI_TextBuffer_Destroy;
	obj->draw = GUI_TextBuffer_Draw;
	obj->move = GUI_TextBuffer_Move;
	obj->param = GUI_TextBuffer_Param;
	obj->notify = NULL;

	textbuffer->parent = NULL;

	M_SetVec3(textbuffer->dropshadowcolor, 0, 0, 0);
	textbuffer->dropshadowoffset = 2;
	textbuffer->dropshadow = 1;
	textbuffer->current_line = 0;
	textbuffer->delay_msecs = 10000;
	textbuffer->icon_height = obj->char_height;

	//rows will get set inside the Resize function
	textbuffer->rows = 0;
	GUI_TextBuffer_Resize(obj, atoi(argv[4]), atoi(argv[3]));

	return textbuffer;
}


void	GUI_TextBuffer_Cmd(int argc, char *argv[])
{
	gui_textbuffer_t *textbuffer;
	gui_element_t	*obj;
	
	if (!argc)
	{
		corec.Console_Printf("textbuffer <command> <args>\n");
		corec.Console_Printf("  commands:\n");
		corec.Console_Printf("    list\n");
		corec.Console_Printf("    param\n");
		corec.Console_Printf("    print\n");
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
			GUI_TextBuffer_Param(obj, argc-2, &argv[2]);
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
			textbuffer = corec.GUI_GetClass(argv[1], class_name);
			if (textbuffer)
				GUI_TextBuffer_Printf(textbuffer, argv[2], 0);
			else
				corec.Console_Printf("Couldn't find textbuffer type widget %s\n", argv[1]);
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
				GUI_TextBuffer_Clear(obj);
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

void	GUI_TextBuffer_Init()
{
	corec.Cmd_Register("textbuffer", GUI_TextBuffer_Cmd);

	corec.GUI_RegisterClass(class_name, GUI_TextBuffer_Create);
}
