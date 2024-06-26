
#include "../toplayer/tl_shared.h"
#include "gui_orderedscrollbuffer.h"

static char *class_name = "o_scrollbuffer";

//we can assume each char is at least 3 pixels wide
#define MIN_CHAR_WIDTH 3
#define LINE_VALUE_LENGTH 64

#define X_SPACING 1

void	GUI_OrderedScrollbuffer_Scrollvar(gui_o_scrollbuffer_t *o_scrollbuffer);

static gui_o_scrollbuffer_t *sorting_scrollbuffer;
static bool incremental = true;

int	_GUI_OrderedOrderedScrollBuffer_ValueCompare(const void *p1, const void *p2)
{
	char *str1 = sorting_scrollbuffer->values[*(int *)(incremental ? p1 : p2)][sorting_scrollbuffer->sortColumn];
	char *str2 = sorting_scrollbuffer->values[*(int *)(incremental ? p2 : p1)][sorting_scrollbuffer->sortColumn];
	//corec.Console_DPrintf("comparing row %i (value %s) with row %i (value %s)\n", *(int *)p1, str1, *(int *)p2, str2);
	if (atoi(str1) || atoi(str2))
	{
		//corec.Console_DPrintf("comparing as an int!\n");
		return (atoi(str1) - atoi(str2));
	}
	return stricmp(str1, str2);
}

void	GUI_OrderedScrollBuffer_Draw(gui_element_t *obj, int x, int y)
{
	vec4_t textcolor;
	gui_o_scrollbuffer_t *o_scrollbuffer;
	char column[256];
	char *tmp, *pos;
	int i, row, col;

	o_scrollbuffer = corec.GUI_GetUserdata(class_name, obj);
	if (!o_scrollbuffer)
		return;

	o_scrollbuffer->visible_lines = floor(obj->height/o_scrollbuffer->icon_height);
	if (o_scrollbuffer->scrollvar)
		GUI_OrderedScrollbuffer_Scrollvar(o_scrollbuffer);
	
	corec.GUI_SetRGBA(1,1,1, obj->alpha);

	if (o_scrollbuffer->thickness)
	{
		corec.GUI_LineBox2d_S(0,0, 
				obj->width, 
				obj->height, 
				o_scrollbuffer->thickness);
	}

    if (o_scrollbuffer->icon_height < obj->char_height)
		o_scrollbuffer->icon_height = obj->char_height;
	
	textcolor[3] = obj->alpha;
	M_CopyVec3(obj->textcolor, textcolor);
	corec.Draw_SetColor(textcolor);

	if (o_scrollbuffer->needsSorting)
	{
		for (i = 0; i < o_scrollbuffer->rows; i++)
			o_scrollbuffer->sortedRows[i] = i;
		sorting_scrollbuffer = o_scrollbuffer;
		incremental = o_scrollbuffer->incremental;
		//corec.Console_DPrintf("Sorting %i rows of the scrollbuffer using column %i as the sort column\n", o_scrollbuffer->last_line+1, o_scrollbuffer->sortColumn);
		qsort(o_scrollbuffer->sortedRows, o_scrollbuffer->last_line+1, sizeof(int), _GUI_OrderedOrderedScrollBuffer_ValueCompare);
		o_scrollbuffer->needsSorting = false;
	}

	for (i = 0; i < o_scrollbuffer->visible_lines && i < o_scrollbuffer->rows && o_scrollbuffer->visible_start_line + i < o_scrollbuffer->rows; i++)
	{
		row = o_scrollbuffer->sortedRows[(o_scrollbuffer->visible_start_line + i)];
		col = 0;
		strcpy(column, o_scrollbuffer->buffer[row]);
		tmp = column;
		pos = column;

		if (strcmp(o_scrollbuffer->buffer[row], "") == 0)
			continue;

		if (o_scrollbuffer->selected_line == row)
		{
			corec.Draw_SetColor(o_scrollbuffer->selected_bgcolor);
			corec.GUI_Quad2d_S(o_scrollbuffer->thickness, o_scrollbuffer->thickness + o_scrollbuffer->icon_height*i, obj->width - o_scrollbuffer->thickness*2, o_scrollbuffer->icon_height, corec.GetWhiteShader());
			corec.Draw_SetColor(o_scrollbuffer->selected_textcolor);
		}
		else
		{
			corec.Draw_SetColor(textcolor);
		}


		while (pos)
		{
			tmp = strstr(pos, "^col^");
			if (tmp)
			{
				tmp[0] = 0;
				tmp += 5;
			}
			if (o_scrollbuffer->dropshadow)
			{
				M_CopyVec3(obj->textcolor, textcolor);
	
				corec.GUI_DrawShadowedString(o_scrollbuffer->thickness+X_SPACING + o_scrollbuffer->columnPositions[col], 
						o_scrollbuffer->thickness + o_scrollbuffer->icon_height * i, 
						pos, obj->char_height, o_scrollbuffer->icon_height,
						1, o_scrollbuffer->columnPositions[col+1] - o_scrollbuffer->columnPositions[col], corec.GetNicefontShader(), textcolor[0], textcolor[1], textcolor[2], obj->alpha);
			}
			else
			{
				M_CopyVec3(obj->textcolor, textcolor);
				corec.Draw_SetColor(textcolor);

				corec.GUI_DrawString(o_scrollbuffer->thickness+X_SPACING + o_scrollbuffer->columnPositions[col], 
					o_scrollbuffer->thickness + obj->char_height * i, 
					pos, obj->char_height, o_scrollbuffer->icon_height,
					1, o_scrollbuffer->columnPositions[col+1] - o_scrollbuffer->columnPositions[col], corec.GetNicefontShader());
			}

			col++;
			pos = tmp;
		}
	}
}

void    GUI_OrderedScrollBuffer_MouseDown(gui_element_t *obj, mouse_button_enum button, int x, int y)
{
	gui_o_scrollbuffer_t *o_scrollbuffer;
	
	o_scrollbuffer = corec.GUI_GetUserdata(class_name, obj);
	
	if (!o_scrollbuffer)
		return;
	
	o_scrollbuffer->selected_line = o_scrollbuffer->sortedRows[(y - o_scrollbuffer->thickness) / obj->char_height + o_scrollbuffer->visible_start_line];
	
	if (o_scrollbuffer->variable[0])
	{
		corec.Cvar_Set(o_scrollbuffer->variable, o_scrollbuffer->linevalue[o_scrollbuffer->selected_line]);
	}

	if (button == MOUSE_RBUTTON)
	{
		if (o_scrollbuffer->rclick_cmd[0])
			corec.Cmd_Exec(o_scrollbuffer->rclick_cmd);
	}
}

void	GUI_OrderedScrollbuffer_Scrollvar(gui_o_scrollbuffer_t *o_scrollbuffer)
{
	int numLines;

	if (!o_scrollbuffer->scrollvar)
		return;
	
	numLines = o_scrollbuffer->last_line - o_scrollbuffer->visible_lines + 1;
	if (numLines)
	{
		o_scrollbuffer->visible_start_line = floor(corec.Cvar_GetValue(o_scrollbuffer->scrollvar) * numLines + 0.5);
		if (o_scrollbuffer->visible_start_line + o_scrollbuffer->visible_lines > o_scrollbuffer->last_line)
			o_scrollbuffer->visible_start_line = o_scrollbuffer->last_line - (o_scrollbuffer->visible_lines -1);
		if (o_scrollbuffer->visible_start_line < 0)
			o_scrollbuffer->visible_start_line = 0;
	}
}

void	GUI_OrderedScrollBuffer_Move(gui_element_t *obj, int x, int y)
{
	corec.GUI_Move(obj, x, y);
}

void	GUI_OrderedScrollBuffer_Clear(gui_element_t *obj)
{
	gui_o_scrollbuffer_t *o_scrollbuffer;
	int i;

	o_scrollbuffer = corec.GUI_GetUserdata(class_name, obj);
	if (!o_scrollbuffer)
		return;

	for (i = 0; i < o_scrollbuffer->rows; i++)
	{
		strcpy(o_scrollbuffer->buffer[i], "");
	}
	//so the game list won't change you position when it refreshes - ugly
	//o_scrollbuffer->visible_start_line = 0;
}

void	GUI_OrderedScrollBuffer_Resize(gui_element_t *obj, int width, int height)
{
	gui_o_scrollbuffer_t *o_scrollbuffer;

	o_scrollbuffer = corec.GUI_GetUserdata(class_name, obj);
	if (!o_scrollbuffer)
		return;
	
	corec.GUI_Resize(obj, width, height);
	
	o_scrollbuffer->visible_lines = floor(height/o_scrollbuffer->icon_height);
}

void	GUI_OrderedScrollBuffer_Realloc(gui_element_t *obj, int rows, int width)
{
	gui_o_scrollbuffer_t *o_scrollbuffer;
	int i, j;

	o_scrollbuffer = corec.GUI_GetUserdata(class_name, obj);
	if (!o_scrollbuffer)
		return;

	for (i = 0; i < o_scrollbuffer->rows; i++)
	{
		corec.GUI_Free(o_scrollbuffer->buffer[i]);
		corec.GUI_Free(o_scrollbuffer->linevalue[i]);
		corec.GUI_Free(o_scrollbuffer->values[i]);
	}
		
	o_scrollbuffer->rows = rows;
	o_scrollbuffer->cols = width/MIN_CHAR_WIDTH;

	o_scrollbuffer->sortedRows = corec.GUI_ReAlloc(o_scrollbuffer->sortedRows, o_scrollbuffer->rows * sizeof (int));
	o_scrollbuffer->values = corec.GUI_ReAlloc(o_scrollbuffer->values, o_scrollbuffer->rows * sizeof (char **));
	memset(o_scrollbuffer->values, 0, sizeof(char **) * o_scrollbuffer->rows);

	for (j = 0; j < o_scrollbuffer->rows; j++)
	{
		o_scrollbuffer->values[j] = corec.GUI_ReAlloc(o_scrollbuffer->values[j], sizeof (char *) * _MAX_COLUMNS);
		for (i = 0; i < _MAX_COLUMNS; i++)
		{
			o_scrollbuffer->values[j][i] = corec.Tag_Malloc(1);
			o_scrollbuffer->values[j][i][0] = 0;
		}
	}
	
	o_scrollbuffer->buffer = corec.GUI_ReAlloc(o_scrollbuffer->buffer, o_scrollbuffer->rows * sizeof (char *));
	o_scrollbuffer->linevalue = corec.GUI_ReAlloc(o_scrollbuffer->linevalue, o_scrollbuffer->rows * sizeof (char *));
	for (i = 0; i < o_scrollbuffer->rows; i++)
	{
		o_scrollbuffer->buffer[i] = corec.GUI_Malloc(o_scrollbuffer->cols + 1);
		o_scrollbuffer->linevalue[i] = corec.GUI_Malloc(LINE_VALUE_LENGTH);
		strcpy(o_scrollbuffer->buffer[i], "");
		strcpy(o_scrollbuffer->linevalue[i], "");
	}
	
	//uncommenting this makes the box HUGE
	//corec.GUI_Resize(obj, width + 2*o_scrollbuffer->thickness, rows * obj->char_height + 2*o_scrollbuffer->thickness);
}

//messy line-wrapping here
void	GUI_OrderedScrollBuffer_Printf(gui_o_scrollbuffer_t *o_scrollbuffer, char *message, char *value, int numSortValues, char **sortvalues)
{
	int messagelen, i, n, current_line;

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
	
	n = 0;
	while (n < o_scrollbuffer->rows)
	{
		if (o_scrollbuffer->buffer[n][0] == 0)
		{
			current_line = n;
			strncpy(o_scrollbuffer->buffer[n], message, o_scrollbuffer->cols);
			o_scrollbuffer->buffer[n][o_scrollbuffer->cols-1] = 0;
			strncpy(o_scrollbuffer->linevalue[n], value, LINE_VALUE_LENGTH);
			o_scrollbuffer->linevalue[n][LINE_VALUE_LENGTH-1] = 0;

			if (o_scrollbuffer->selected_line == current_line && o_scrollbuffer->variable)
				corec.Cvar_Set(o_scrollbuffer->variable, o_scrollbuffer->linevalue[o_scrollbuffer->selected_line]);

			for (i = 0; i < numSortValues && i < _MAX_COLUMNS; i++)
			{
				if (!sortvalues[i])
					sortvalues[i] = "";
				
				if (o_scrollbuffer->values[current_line][i])
					corec.GUI_Free(o_scrollbuffer->values[current_line][i]);

				o_scrollbuffer->values[current_line][i] = corec.GUI_Malloc((strlen(sortvalues[i])+1) * sizeof(char));
				strcpy(o_scrollbuffer->values[current_line][i], sortvalues[i]);
			}
			o_scrollbuffer->last_line = current_line;
			o_scrollbuffer->needsSorting = true;
			return;
		}
		n++;
	}
}

void	GUI_OrderedScrollBuffer_Destroy(gui_element_t *obj)
{
	gui_o_scrollbuffer_t *o_scrollbuffer;
	int i;

	o_scrollbuffer = corec.GUI_GetUserdata(class_name, obj);
	if (!o_scrollbuffer)
		return;
	
	for (i = 0; i < o_scrollbuffer->rows; i++)
	{
		corec.GUI_Free(o_scrollbuffer->buffer[i]);
		corec.GUI_Free(o_scrollbuffer->linevalue[i]);
		corec.GUI_Free(o_scrollbuffer->values[i]);
	}
	corec.GUI_Free(o_scrollbuffer->values);
	corec.GUI_Free(o_scrollbuffer->buffer);
	corec.GUI_Free(o_scrollbuffer->linevalue);
	corec.GUI_Free(o_scrollbuffer->sortedRows);
}

void	GUI_OrderedScrollBuffer_Param(gui_element_t *obj, int argc, char *argv[])
{
	gui_o_scrollbuffer_t *o_scrollbuffer;

	if (argc < 2)
	{
		corec.Console_Printf("o_scrollbuffer param <panel:object> <param> <value>\n");
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
		corec.Console_Printf("     rows					- (number of rows in o_scrollbuffer)\n");
		corec.Console_Printf("     linewrap				- turn on/off linewrapping\n");
		corec.Console_Printf("     variable				- the variable to set the value to\n");
		corec.Console_Printf("     scrollvariable		- the variable to determine the scrolling amount\n");
		corec.Console_Printf("     rclick_cmd			- the command to run when someone right-clicks\n");
		corec.Console_Printf("     sort_column			- the column to sort on\n");
		corec.Console_Printf("     colpos	<i> <pos>	- the pixel position to start column number <i>\n");
		corec.Console_Printf("     url					- the url to grab data from to put in the o_scrollbuffer\n");
		return;
	}

	o_scrollbuffer = corec.GUI_GetUserdata(class_name, obj);

	if (!o_scrollbuffer)
		return;

	if (strcmp(argv[0], "thickness") == 0)
	{
		o_scrollbuffer->thickness = atoi(argv[1]);
	} 
	else if (strcmp(argv[0], "dropshadow") == 0)
	{
		if (argc > 3)
		{
			o_scrollbuffer->dropshadow = atoi(argv[1]);			
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
			o_scrollbuffer->dropshadowcolor[0] = atof(argv[1]);
			o_scrollbuffer->dropshadowcolor[1] = atof(argv[2]);
			o_scrollbuffer->dropshadowcolor[2] = atof(argv[3]);
		}
		else
		{
			corec.Console_Printf("Error: Not enough parameters.  You must specify the text r g b values\n");
		}
	}
	else if (strcmp(argv[0], "selected_textcolor") == 0)
	{
		if (argc > 3)
		{
			o_scrollbuffer->selected_textcolor[0] = atof(argv[1]);
			o_scrollbuffer->selected_textcolor[1] = atof(argv[2]);
			o_scrollbuffer->selected_textcolor[2] = atof(argv[3]);
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
			o_scrollbuffer->selected_bgcolor[0] = atof(argv[1]);
			o_scrollbuffer->selected_bgcolor[1] = atof(argv[2]);
			o_scrollbuffer->selected_bgcolor[2] = atof(argv[3]);
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
			o_scrollbuffer->dropshadowoffset = atoi(argv[1]);			
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
			GUI_OrderedScrollBuffer_Realloc(o_scrollbuffer->element, atoi(argv[1]), obj->width);
		}
	}
	else if (strcmp(argv[0], "linewrap") == 0)
	{
		if (argc > 1)
		{
			if (strcmp(argv[1],"true") == 0
				|| strcmp(argv[1],"1") == 0)
			{
				o_scrollbuffer->wrap = true;
			}
			else
			{
				o_scrollbuffer->wrap = false;
			}
		}
	}
	else if (strcmp(argv[0], "variable") == 0)
	{
		if (argc > 1)
		{
			strncpySafe(o_scrollbuffer->variable, argv[1], sizeof(o_scrollbuffer->variable));
		}
	}
	else if (strcmp(argv[0], "scrollvariable") == 0)
	{
		if (argc > 1)
		{
			strncpySafe(o_scrollbuffer->scrollvar, argv[1], sizeof(o_scrollbuffer->scrollvar));
		}
	}
	else if (strcmp(argv[0], "rclick_cmd") == 0)
	{
		if (argc > 1)
		{
			strncpy(o_scrollbuffer->rclick_cmd, argv[1], _MAX_COMMAND_LENGTH);
			o_scrollbuffer->rclick_cmd[_MAX_COMMAND_LENGTH-1] = 0;
		}
	}
	else if (strcmp(argv[0], "url") == 0)
	{
		char buf[2048];
		char *tmp, *end;

		corec.HTTP_GetText(argv[1], buf, 2048);
		buf[2047] = 0;
		buf[2046] = 0;

		GUI_OrderedScrollBuffer_Clear(o_scrollbuffer->element);

		tmp = buf;
		while (*tmp && (end = strchr(tmp, '\n')))
		{
			end[0] = 0;
			GUI_OrderedScrollBuffer_Printf(o_scrollbuffer, tmp, "0", 0, (char **)"");
			tmp = end + 1;
		}
		
	}
	else if (strcmp(argv[0], "icon_height") == 0)
	{
		if (argc > 1)
		{
			o_scrollbuffer->icon_height = MAX(atoi(argv[1]), o_scrollbuffer->element->char_height);
		}
	}
	else if (strcmp(argv[0], "sort_column") == 0)
	{
		int col;
		if (argc > 1)
		{
			col = atoi(argv[1]);
			if (o_scrollbuffer->sortColumn == col)
				o_scrollbuffer->incremental = !o_scrollbuffer->incremental;
			else
				o_scrollbuffer->incremental = true;
			o_scrollbuffer->sortColumn = col;
			if (o_scrollbuffer->sortColumn < 0)
				o_scrollbuffer->sortColumn = 0;
			else if (o_scrollbuffer->sortColumn >= _MAX_COLUMNS)
				o_scrollbuffer->sortColumn = 0;
			o_scrollbuffer->needsSorting = true;
		}
	}
	else if (strcmp(argv[0], "colpos") == 0)
	{
		int col, pos;
		if (argc > 2)
		{
			col = atoi(argv[1]);
			pos = atoi(argv[2]);
			if (col > 0 && col < _MAX_COLUMNS)
			{
				if (pos > 0 && pos < obj->width)
				{
					o_scrollbuffer->columnPositions[col] = pos;
				}
				else
				{
					corec.Console_Printf("Invalid position\n");
				}
			}
			else
			{
				corec.Console_Printf("Invalid column\n");
			}
		}
	}
	else
	{
		corec.Console_Printf("Unknown command %s\n", argv[0]);
	}
}

void	*GUI_OrderedScrollBuffer_Create(gui_element_t *obj, int argc, char *argv[])
{
	gui_o_scrollbuffer_t *o_scrollbuffer;
	int i;

	if (argc < 5)
	{
		corec.Console_Printf("syntax: o_scrollbuffer name x y width height\n");
		return NULL;
	}

	corec.GUI_SetClass(obj, class_name);
	corec.GUI_SetName(obj, argv[0]);
	corec.GUI_Move (obj, atoi(argv[1]), atoi(argv[2]) );

	o_scrollbuffer = corec.GUI_Malloc(sizeof (gui_o_scrollbuffer_t));

	if (!o_scrollbuffer)
	{
		corec.Console_Printf("OrderedScrollBuffer error: couldn't alloc enough space to hold o_scrollbuffer\n");
		return NULL; 		
	}

	corec.GUI_SetUserdata(class_name, obj, o_scrollbuffer);
	o_scrollbuffer->element = obj;

	o_scrollbuffer->thickness = 0;

	obj->interactive = true;
	obj->destroy = GUI_OrderedScrollBuffer_Destroy;
	obj->draw = GUI_OrderedScrollBuffer_Draw;
	obj->move = GUI_OrderedScrollBuffer_Move;
	obj->param = GUI_OrderedScrollBuffer_Param;
	obj->mousedown = GUI_OrderedScrollBuffer_MouseDown;
	obj->notify = NULL;

	o_scrollbuffer->parent = NULL;

	o_scrollbuffer->wrap = false;
	o_scrollbuffer->visible_start_line = 0;
	o_scrollbuffer->selected_line = 0;

	M_SetVec3(o_scrollbuffer->dropshadowcolor, 0, 0, 0);
	o_scrollbuffer->dropshadowoffset = 1;
	o_scrollbuffer->dropshadow = 1;

	M_SetVec3(o_scrollbuffer->selected_textcolor, 0, 0, 0);
	o_scrollbuffer->selected_textcolor[3] = obj->alpha;
	M_SetVec3(o_scrollbuffer->selected_bgcolor, 0.7, 0.6, 0);
	o_scrollbuffer->selected_bgcolor[3] = obj->alpha;

	//rows will get set inside the Resize function
	o_scrollbuffer->rows = 0;
	o_scrollbuffer->buffer = 0;
	o_scrollbuffer->linevalue = 0;
	o_scrollbuffer->rclick_cmd[0] = 0;
	o_scrollbuffer->icon_height = obj->char_height;

	o_scrollbuffer->needsSorting = true;
	o_scrollbuffer->sortColumn = 0;
	o_scrollbuffer->values = 0;

	for (i = 0; i < _MAX_COLUMNS; i++)
		o_scrollbuffer->columnPositions[i] = i*100; //whatever, guess something
	
	GUI_OrderedScrollBuffer_Realloc(obj, atoi(argv[4])/o_scrollbuffer->icon_height, atoi(argv[3])/MIN_CHAR_WIDTH);
	GUI_OrderedScrollBuffer_Resize(obj, atoi(argv[3]), atoi(argv[4]));

	return o_scrollbuffer;
}


void	GUI_OrderedScrollBuffer_Cmd(int argc, char *argv[])
{
	gui_o_scrollbuffer_t *o_scrollbuffer;
	gui_element_t	*obj;
	int inc, numLines;
	float val;
	
	if (!argc)
	{
		corec.Console_Printf("o_scrollbuffer <command> <args>\n");
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
		o_scrollbuffer = corec.GUI_GetClass(argv[1], class_name);
		if (o_scrollbuffer && argc > 1)
		{
			numLines = MAX(0, o_scrollbuffer->last_line - o_scrollbuffer->visible_lines + 1);
			if (numLines > 0)
			{	
				inc = atoi(argv[2]);
				val = corec.Cvar_GetValue(o_scrollbuffer->scrollvar) + ((float)inc / numLines);
				if (val < 0)
					val = 0;
				if (val > 1)
					val = 1;
				corec.Cvar_SetValue(o_scrollbuffer->scrollvar, val);
			}
		}
		
	}
	else if (strcmp(argv[0], "param") == 0)
	{
		if (argc > 2)
		{
			obj = corec.GUI_GetObject(argv[1]);
			GUI_OrderedScrollBuffer_Param(obj, argc-2, &argv[2]);
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
			o_scrollbuffer = corec.GUI_GetClass(argv[1], class_name);
			if (o_scrollbuffer)
				GUI_OrderedScrollBuffer_Printf(o_scrollbuffer, argv[2], argc > 3 ? argv[3] : "", 
								argc > 4 ? argc - 4 : 0, argc > 4 ? &argv[4] : (char **)"");
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
				GUI_OrderedScrollBuffer_Clear(obj);
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

void	GUI_OrderedScrollBuffer_Init()
{
	corec.Cmd_Register("o_scrollbuffer", GUI_OrderedScrollBuffer_Cmd);

	corec.GUI_RegisterClass(class_name, GUI_OrderedScrollBuffer_Create);
}
