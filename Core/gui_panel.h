/*
 * Panels
 */

void	GUI_Panels_Init();
void	GUI_Panel_AddObject(gui_element_t *object);
void    GUI_Panel_UpdateWidgetSize(gui_element_t *obj);
void	GUI_Panel_Move(gui_panel_t *panel, int x, int y);
void	GUI_Panel_MoveRelative(gui_panel_t *panel, int x, int y);
void	GUI_Panel_Show(gui_panel_t *panel);
void	GUI_Panel_Hide(gui_panel_t *panel);
void	GUI_Panel_ChangeRes();
void	GUI_Panel_BringTreeToFront(gui_panel_t *panel);
void    GUI_RemovePanel(gui_panel_t *panel);
void    GUI_RemovePanels();
gui_panel_t	*GUI_Panel_GetList();
gui_panel_t *GUI_GetPanel(char *name);
void	GUI_HideAllPanels();

void    GUI_Panel_FadeOut(gui_panel_t *panel, float time);
void    GUI_Panel_FadeIn(gui_panel_t *panel, float time);

extern gui_panel_t panelList;
