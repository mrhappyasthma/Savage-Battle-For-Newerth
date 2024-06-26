/*
 * Labels
 */
typedef struct
{
	gui_element_t			*parent;

	char				*text;
	char				cvar[64];

	vec4_t			bgcolor;
	int				align;
	int				textWidth;

	bool				shadow;

	gui_element_t			*element;
} gui_label_t;

void	GUI_Labels_Init();
void	*GUI_Label_Create(gui_element_t *obj, int argc, char *argv[]);
void	GUI_Label_Param_Cmd(gui_element_t *obj, int argc, char *argv[]);
void	GUI_Label_ShowText(gui_element_t *obj, char *text);
void	GUI_Label_ShowInt(gui_element_t *obj, int number);
void	GUI_Label_ShowFloat(gui_element_t *obj, float number);
