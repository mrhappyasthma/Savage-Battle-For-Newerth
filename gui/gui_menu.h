/*
 * Menus
 */

#define MENU_CLASS	"menu"	//fixme: we should do this for all widgets rather than have a local class_name inside the module

#define MENU_MAXTRIMSTRING	32

typedef struct
{
	char					*name;
	int						nameWidth;
	char					*command;
} gui_menu_item_t;


typedef struct
{
	gui_element_t			*parent;

	gui_menu_item_t			*items;
	int						num_items;

	int						selection;

	char					trimstring[32];
	
	bool					mouse_down;
	int						mousedown_time;

	float					bg_r;
	float					bg_g;
	float					bg_b;

	float					highlight_text_r;
	float					highlight_text_g;
	float					highlight_text_b;

	gui_element_t			*element;
} gui_menu_t;

void	GUI_Menus_Init();
void    GUI_Menu_Destroy(gui_element_t *obj);
void    *GUI_Menu_Create(gui_element_t *obj, int argc, char *argv[]);
void    GUI_Menu_Cmd(int argc, char *argv[]);
bool	GUI_Menu_ItemExists(gui_menu_t *menu, const char *item);
void	GUI_Menu_Clear(gui_menu_t *menu);
void	GUI_Menu_AddItem(gui_menu_t *menu, const char *itemname, char *command);
void 	GUI_Menu_RemoveItem(gui_menu_t *menu, const char *itemname);
char	*GUI_Menu_GetValue(gui_element_t *obj);

