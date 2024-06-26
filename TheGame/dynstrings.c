
// (C) 2001 S2 Games

// dynstrings.c

//=============================================================================
// Dynamic Strings
//=============================================================================
#include "game.h"

static char *null_string = "";

/*==========================

  DynAllocString

  Returns a pointer to a dynamicly allocated copy of <s>

 ==========================*/

char	*DynAllocString(const char *s)
{
	char *newstring = (char*)core.Tag_Malloc(strlen(s)+1);

	strcpy(newstring, s);
	return newstring;
}


/*==========================

  DynFree

  Free a string created with DynAllocString

 ==========================*/

void	DynFree(char *s)
{
	if (!s)
		return;

	core.Tag_Free(s);
	s = NULL;
}


/*==========================

  DynSetString

  Replaces contents of dynstring <s> with contents of <a>
  Returns true if operation was successful

 ==========================*/

bool	DynSetString(char **s, const char *a)
{
	if (*s && (*s != ""))
		DynFree(*s);	

	*s = DynAllocString(a);

	return (*s != null_string);
}
