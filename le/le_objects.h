// (C) 2003 S2 Games

// le_objects.h

// object handling in the editor

#define		MAX_EDITOROBJECTS			1024

void		LE_DrawObjects();
void		LE_Objects_Init();

void		LE_ObjectMouseDown();
void		LE_ObjectMouseUp();
void		LE_ObjectRightMouseDown();
void		LE_ObjectRightMouseUp();
void		LE_ObjectMouseOver();

void		LE_ClearEdObjects();
residx_t	LE_GetActiveModel();
void		LE_LoadWorldObjects();

