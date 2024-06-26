// (C) 2003 S2 Games

// interface.h



#include "savage.h"

#include "gui_scrollbuffer.h"
#include "gui_orderedscrollbuffer.h"
#include "gui_textbuffer.h"
#include "gui_button.h"
#include "gui_graphic.h"
#include "gui_label.h"
#include "gui_textbox.h"

extern gui_o_scrollbuffer_t *gamelist_scrollbuffer_widget;
extern gui_scrollbuffer_t *buddysearch_scrollbuffer_widget;
extern gui_scrollbuffer_t *irc_userlist;
extern gui_scrollbuffer_t *buddylist;
extern gui_scrollbuffer_t *irc_window;
extern gui_scrollbuffer_t *buddy_msgwindow;
extern gui_button_t *buddy_joingame;
extern gui_graphic_t *buddy_joingame_graphic;

extern coreAPI_shared_t core;

bool	INT_Locked();
void	INT_SetCursorShader(residx_t shader, float hotspotx, float hotspoty);
void	INT_InitWidgetPointers();
void	INT_LoadingFrame(const char *currentResource);
void	INT_DrawCrosshair(int x, int y, int w, int h, float focus, bool melee, bool gui);