// (C) 2003 S2 Games

// le_brushpanel.h

// brush panel routines

typedef struct
{
	gui_element_t			*parent;

	gui_element_t			*element;
} gui_brushpanel_t;

typedef struct
{
	gui_element_t			*parent;

	int						brushgrid_rows;
	int						brushgrid_selection;

	gui_element_t			*element;
} gui_brushgrid_t;


void	GUI_BPanel_Init();
void	*GUI_BPanel_Create(gui_element_t *obj, int argc, char *argv[]);
