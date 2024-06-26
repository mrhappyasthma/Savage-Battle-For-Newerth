// (C) 2003 S2 Games

// le_ui.h

// simple ui stuff

char	*LE_GenericMenu_GetItemName();

//generic menu

void	LE_GenericMenu_Init(gui_element_t *elem);
void	LE_GenericMenu_MouseUp(gui_element_t *obj, int x, int y);
void	LE_GenericMenu_MouseOver(gui_element_t *obj, int x, int y);
void	LE_GenericMenu_MouseOut(gui_element_t *obj);
void	LE_GenericMenu_Draw(gui_element_t *obj, int w, int h);
void	LE_GenericMenu_MouseDown(gui_element_t *obj, int x, int y);
void	LE_GenericMenu_Idle(gui_element_t *obj);
void	LE_GenericMenu_SetDims(gui_element_t *elem, int xpos, int ypos, gui_menuitem_t items[]);
void	LE_GenericMenu_AddElements(gui_element_t *elem);
