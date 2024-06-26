// (C) 2003 S2 Games

// le_menus.c

// menu commands

#include "../le/le.h"

#if 0
void	LE_RaiseTerrain_M(void)
{
	le.leftbrushmode = BRUSHMODE_RAISE_TERRAIN;
}

void	LE_LowerTerrain_M(void)
{
	le.leftbrushmode = BRUSHMODE_LOWER_TERRAIN;
}

void	LE_AddModel_M(void)
{
	LE_SetModelFunction(LE_AddModel);
}

void	LE_SubtractModel_M(void)
{
	LE_SetModelFunction(LE_SubtractModel);
}

void	LE_FlattenModel_M(void)
{
	LE_SetModelFunction(LE_FlattenModel);
}

void	LE_TmatCallback(const char *dir)
{
	LE_AddMenuItem(dir, NULL);
}

void	LE_InitMenus()
{
/*	LE_SelectMenu(MENU_MODEL);
	LE_AddMenuItem("Undo", NULL);
	LE_AddMenuItem("Redo", NULL);
	LE_AddMenuItem(" ", NULL);
	LE_AddMenuItem("Add", LE_AddModel_M);
	LE_AddMenuItem("Subtract", LE_SubtractModel_M);
	LE_AddMenuItem("Flatten", LE_FlattenModel_M);
	
	LE_SelectMenu(MENU_PAINT);
	LE_AddMenuItem("Undo", NULL);
	LE_AddMenuItem("Redo", NULL);
	LE_AddMenuItem(" ", NULL);
	LE_AddMenuItem("", NULL);

	LE_SelectMenu(MENU_TEXTURE);
	System_Dir("tmats/\*", false, LE_TmatCallback, NULL);*/
}
#endif
