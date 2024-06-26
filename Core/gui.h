// (C) 2003 S2 Games

// gui_main.h

// simple ui stuff

//all the classes
#include "gui_panel.h"
#include "gui_drawutils.h"

/*
 * Base class functions
 */

void			GUI_Frame();

void		   *GUI_GetUserdata(char *class_name, gui_element_t *obj);
void		   *GUI_SetUserdata(char *class_name, gui_element_t *obj, void *user_data);
void		   *GUI_RegisterClass(char *class_name, void *(*create)(gui_element_t *obj, int argc, char *argv[]));

char		   *GUI_SetName(gui_element_t *obj, char *name);
char		   *GUI_SetClass(gui_element_t *obj, char *class_name);

void			GUI_Notify(int argc, char *argv[]);

bool			GUI_IsVisible(gui_element_t *obj);
bool			GUI_IsInteractive(gui_element_t *obj);

void			GUI_Exec(char *command);
void			GUI_Move(gui_element_t *obj, int x, int y);
void			GUI_Resize(gui_element_t *obj, int w, int h);
void			GUI_Show(gui_element_t *obj);
void			GUI_Hide(gui_element_t *obj);
void			GUI_Unfocus(gui_element_t *obj);
void			GUI_Focus(gui_element_t *obj);

bool			GUI_Select(gui_element_t *obj);
void			GUI_Param(gui_element_t *obj, int argc, char *argv[]);

gui_element_t   *GUI_New();
void    		GUI_RemoveElement(gui_element_t *elem);

char			*GUI_StrDup(const char *string);

void			GUI_GetScreenPosition(gui_element_t *obj, ivec2_t pos);
void			GUI_GetScreenSize(gui_element_t *obj, ivec2_t size);
gui_element_t   *GUI_GetObject(char *hierarchy);
void			*GUI_GetClass(char *hierarchy, char *classname);
void			GUI_List_Cmd(int argc, char *argv[]);

void			*GUI_Malloc(int size);
void			*GUI_ReAlloc(void *ptr, int size);
void			GUI_Free(void *ptr);

void			GUI_ClearWidgets();
void    		GUI_Reset();
void			GUI_Init();

gui_element_t 	*GUI_GetWidgetAtXY(int x, int y);
bool			GUI_CheckMouseAgainstUI(int x, int y);

bool			GUI_SendMouseDown(mouse_button_enum button, int x, int y);
bool			GUI_SendMouseUp(mouse_button_enum button, int x, int y);

bool    		GUI_WidgetWantsMouseClicks(mouse_button_enum button, int x, int y, bool down);
bool    		GUI_WidgetWantsKey(int key, bool down);

void			GUI_AddElement(gui_element_t *elem);
void			GUI_Draw();
void			GUI_UpdateDims();

void			GUI_Coord(int *x, int *y);
void			GUI_ConvertToScreenCoord(int *x, int *y);
void			GUI_ScaleToScreen(int *w, int *h);
void			GUI_Scale(int *w, int *h);

int				GUI_GetScreenWidth();
int				GUI_GetScreenHeight();

void			GUI_GetPosition(gui_element_t *obj, ivec2_t pos);
void    		GUI_GetSize(gui_element_t *obj, ivec2_t size);

void    		GUI_ChangeAlpha(gui_element_t *obj, float alpha);
void    		GUI_FadeInObject(gui_element_t *obj, float time);
void    		GUI_FadeOutObject(gui_element_t *obj, float time);

void			GUI_ResetFocus();
void    		GUI_ActivateNextInteractiveWidget();

void   	 		GUI_ResetButtons();
