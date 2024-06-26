// (C) 2002 S2 Games

// gui_textbuffer.h

typedef struct
{
	gui_element_t			*parent;

	int						thickness;

	int						width;

	int						delay_msecs;

	int						icon_height;

	unsigned int			current_line;
	int						rows;

	bool					dropshadow;
	vec3_t					dropshadowcolor;
	int						dropshadowoffset;

	char					**buffer;
	int						*line_times;

	gui_element_t			*element;
} gui_textbuffer_t;

void	GUI_TextBuffer_Init();
void	*GUI_TextBuffer_Create(gui_element_t *obj, int argc, char *argv[]);
void	GUI_TextBuffer_Printf(gui_textbuffer_t *textbuffer, char *message, int indent);
void    GUI_TextBuffer_Destroy(gui_element_t *obj);
void    GUI_TextBuffer_Param(gui_element_t *obj, int argc, char *argv[]);
void    GUI_TextBuffer_Clear(gui_element_t *obj);
