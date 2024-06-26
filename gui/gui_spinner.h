/*
 *  Spinners
 */

#define SPINNER_BUFFER_Y 4		// Pixels before starting dragging

#include "gui_button.h"

typedef struct
{
	gui_button_t			*parent;
	char					variable[64];

	int						curtime;

	float					speed;
	
	float					low;
	float					hi;

	float					step;
	
	gui_element_t			*element;
} gui_spinner_t;

void GUI_Spinners_Init();
