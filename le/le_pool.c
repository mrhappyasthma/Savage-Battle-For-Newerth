// (C) 2003 S2 Games

// le_pool.c

// handles simple struct and string allocation

#include "../le/le.h"

#define LE_POOLSIZE 262144  //256k

static char	pool[LE_POOLSIZE];
static int	marker = 0;
static char *null_string = "";

void	*LE_Alloc(int size)
{
	void *newmem;

	if (marker + size > LE_POOLSIZE)
	{
		corec.Console_Errorf("LE_Alloc: pool size of %i bytes exceeded (couldn't allocate %i bytes)\n", LE_POOLSIZE, size);
		return NULL;
	}

	newmem = &pool[marker];

	marker += (size + 31) & ~31;	//align to 32 byte intervals

	return newmem;
}

char	*LE_StrDup(const char *s)
{
	char *newstring;

	newstring = LE_Alloc(strlen(s)+1);

	if (newstring)
	{
		strcpy(newstring, s);
		return newstring;
	}
	else
		return null_string;
}

void	LE_Pool_Init()
{
	marker = 0;
}
