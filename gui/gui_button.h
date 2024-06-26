/*
 * Buttons
 */
#define GUI_BUTTON_CMD_LENGTH				1024
#define GUI_BUTTON_TEXT_LENGTH				1024
#define GUI_BUTTON_ALT_TEXT_LENGTH			1024

#define GUI_BUTTON_ALT_TEXT_DELAY_TIME		100

#define GUI_BUTTON_REPEAT_DELAY				350
#define GUI_BUTTON_REPEAT_INTERVAL			100

typedef struct
{
	gui_element_t			*parent;
	bool					down;
	bool					mouseover;
	bool					enabled;
	bool					repeat;
	int						nextRepeatTime;

	int						textWidth;
	int						altTextWidth;
	int						align;

	char					text[GUI_BUTTON_TEXT_LENGTH];
	char					down_command[GUI_BUTTON_CMD_LENGTH];
	char					mouseenter_command[GUI_BUTTON_CMD_LENGTH];
	char					mouseover_command[GUI_BUTTON_CMD_LENGTH];
	char					mouseaway_command[GUI_BUTTON_CMD_LENGTH];
	char					alt_text[GUI_BUTTON_ALT_TEXT_LENGTH];

	residx_t				disable_shader;
	residx_t				down_shader;
	residx_t				up_mouseover_shader;
	residx_t				up_shader;

	gui_element_t			*element;
} gui_button_t;

void	GUI_Buttons_Init();
void	*GUI_Button_Create(gui_element_t *obj, int argc, char *argv[]);
void	GUI_Button_Param_Cmd(gui_element_t *obj, int argc, char *argv[]);
void    GUI_Button_ShowText(gui_element_t *obj, char *string);
void    GUI_Button_Command(gui_element_t *obj, char *command);
void    GUI_Button_Draw(gui_element_t *obj, int x, int y);

