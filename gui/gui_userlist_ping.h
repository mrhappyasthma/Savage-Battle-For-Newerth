/*
 * User list / ping times
 */
#include "client_game.h"

typedef struct
{
	int				mode;
	int				team;
	int				char_width;

	int				gap;

	int				order[MAX_CLIENTS];
	bool			needsSorting;
	bool			incremental;
	int				sortColumn;

	vec4_t				selected_textcolor;
	vec4_t				selected_bg;

	gui_element_t			*element;
} gui_userlist_t;

void GUI_UserList_Init();

