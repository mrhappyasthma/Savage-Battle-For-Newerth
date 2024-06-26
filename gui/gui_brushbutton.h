
/*
 * Brushbuttons
 *
 *
 * We don't even really need a struct for this type, this is partially for consistency and 
 * partially to enforce a structure in the widgets
 */
#include "gui_button.h"

typedef struct
{
	gui_button_t			*parent;

	gui_element_t			*element;
} gui_brushbutton_t;

void GUI_BrushButtons_Init();

