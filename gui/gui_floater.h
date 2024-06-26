/*
 * Floaters
 */
#include "gui_graphic.h"

typedef struct
{
	gui_graphic_t			*parent;
	residx_t				shader;

	char					text[256];

	bool					mousedown;
	ivec2_t					mousedown_loc;
	gui_element_t			*element;
} gui_floater_t;

void GUI_Floaters_Init();

