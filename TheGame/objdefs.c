// (C) 2001 S2 Games

// game_objdefs.c

// links netObjectTypes_enum to an objdef

#include "game.h"

int	game_objdefs[MAX_OBJECT_TYPES];

cvar_t	structureScale	= { "structureScale", "0.75",	CVAR_READONLY };


void		EnumObjdefFile(const char *filename, void *userdata)
{
	core.Cmd_Exec(fmt("exec #sys_enumdir#/%s\n", filename));
}

void	InitObjdefReferences()
{
	core.Cvar_Register(&structureScale);
	
	core.System_Dir("props", "*.objdef", true, NULL, EnumObjdefFile, NULL);
}
