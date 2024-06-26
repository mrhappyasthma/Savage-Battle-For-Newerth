// (C) 2003 S2 Games

// gui_brushgrid.h

// brush grid routines

typedef struct
{
	gui_element_t			*parent;

	int						rows;
	int						cols;
	int						brush_display_size;
	int						selection;

	gui_element_t			*element;
} gui_brushgrid_t;

void	GUI_Brushgrid_Init();
