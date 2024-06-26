// (C) 2003 S2 Games

// gui_thumbnailgrid.h

// thumbnail grid routines

typedef struct
{
	gui_element_t			*parent;

	bool					grid_visible;

	int						rows;
	int						cols;
	int						thumbnail_display_size;
	int						selection;
	char					cvar[64];
	char					*cmd;

	int						num_thumbnails;

	int						alloced_shaders;
	residx_t				*shaders;
	char					**filenames;

	gui_element_t			*element;
} gui_thumbnailgrid_t;

typedef struct
{
	gui_element_t			*obj;
	char					*dir;
} gui_thumbnailgrid_path_t;

void	GUI_Thumbnailgrid_Init();
