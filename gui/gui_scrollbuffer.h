// (C) 2002 S2 Games

// gui_scrollbuffer.h

#define _MAX_COMMAND_LENGTH 256

typedef struct
{
	gui_element_t			*parent;

	int						thickness;

	int						visible_start_line;
	int						visible_lines;
	int						selected_line;

	int						last_line;
	int						current_line;

	int                     delay_msecs;

	int						icon_height;

	bool					wrap;
	bool					wraparound;
	bool					selectable;

	bool					anchor_to_current_line;
	
	int						rows, cols;

	bool					dropshadow;
	vec3_t					dropshadowcolor;
	int						dropshadowoffset;

	vec4_t              	selected_textcolor;
	vec4_t              	selected_bgcolor;


	char					**buffer;
	char					**linevalue;

	char					variable[64];
	char					scrollvar[64];

	char					rclick_cmd[_MAX_COMMAND_LENGTH];

	int                     *line_times;

	gui_element_t			*element;
} gui_scrollbuffer_t;

void	GUI_ScrollBuffer_Init();
void	*GUI_ScrollBuffer_Create(gui_element_t *obj, int argc, char *argv[]);
void    GUI_ScrollBuffer_Printf(gui_scrollbuffer_t *scrollbuffer, char *message, int indent, char *value);
void    GUI_ScrollBuffer_Destroy(gui_element_t *obj);
void    GUI_ScrollBuffer_Param(gui_element_t *obj, int argc, char *argv[]);
void    GUI_ScrollBuffer_Clear(gui_element_t *obj);
