
/*
 * Togglebuttons
 */
#include "gui_button.h"

typedef struct
{
	gui_button_t			*parent;
	char					up_command[GUI_BUTTON_CMD_LENGTH];
	residx_t				down_mouseover_shader;

	char					variable[64];

	gui_element_t			*element;
} gui_togglebutton_t;

void GUI_ToggleButtons_Init();

