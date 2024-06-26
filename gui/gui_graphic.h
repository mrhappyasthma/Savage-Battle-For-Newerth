/*
 * Graphics
 */

#define GUI_GRAPHIC_CMD_LENGTH				1024

typedef struct
{
	gui_element_t	*parent;
	residx_t		shader;
	float			curFrame;	//for animated textures
	int				startFrame;
	int				endFrame;
	int				freezeFrame;
	int				shaderFPS;
	int				numloops;

	vec3_t			color;

	char			text[256];

	char			movieEndCommand[GUI_GRAPHIC_CMD_LENGTH];
	int				startPlayNum;

	gui_element_t	*element;
} gui_graphic_t;

void	GUI_Graphics_Init();
void	*GUI_Graphic_Create(gui_element_t *obj, int argc, char *argv[]);
void	GUI_Graphic_Cmd(int argc, char *argv[]);
void 	GUI_Graphic_Param_Cmd(gui_element_t *obj, int argc, char *argv[]);
void    GUI_Graphic_ChangeImage(gui_element_t *obj, residx_t image);
void	GUI_Graphic_ShowText(gui_element_t *obj, char *text);
void	GUI_Graphic_ShowFloat(gui_element_t *obj, float number);
void	GUI_Graphic_ShowInt(gui_element_t *obj, int number);
