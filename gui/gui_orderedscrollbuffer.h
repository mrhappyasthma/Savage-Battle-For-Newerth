// (C) 2003 S2 Games

// gui_orderedscrollbuffer.h

#define _MAX_COMMAND_LENGTH 256
#define _MAX_COLUMNS 15

typedef struct
{
	gui_element_t			*parent;

	int						thickness;

	int						visible_start_line;
	int						visible_lines;
	int						selected_line;

	int						last_line;

	int						icon_height;

	bool					wrap;
	
	int						rows, cols;

	bool					dropshadow;
	vec3_t					dropshadowcolor;
	int						dropshadowoffset;

	vec4_t              	selected_textcolor;
	vec4_t              	selected_bgcolor;

	int						columnPositions[_MAX_COLUMNS+1];

	char					***values;
	char					**buffer;
	char					**linevalue;

	bool					needsSorting;
	int						sortColumn;
	bool					incremental;
	int						*sortedRows;

	char					variable[64];
	char					scrollvar[64];

	char					rclick_cmd[_MAX_COMMAND_LENGTH];

	gui_element_t			*element;
} gui_o_scrollbuffer_t;

void	GUI_OrderedScrollBuffer_Init();
void	*GUI_OrderedScrollBuffer_Create(gui_element_t *obj, int argc, char *argv[]);
void    GUI_OrderedScrollBuffer_Printf(gui_o_scrollbuffer_t *o_scrollbuffer, char *message, char *value, int numSortValues, char **sortvalues);
void    GUI_OrderedScrollBuffer_Destroy(gui_element_t *obj);
void    GUI_OrderedScrollBuffer_Param(gui_element_t *obj, int argc, char *argv[]);
void    GUI_OrderedScrollBuffer_Clear(gui_element_t *obj);
