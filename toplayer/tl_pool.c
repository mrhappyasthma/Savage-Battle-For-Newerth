// (C) 2003 S2 Games

// tl_pool.c

// handles simple struct and string allocation

#include "tl_shared.h"

#define TL_POOLSIZE 262144  //256k

static char	pool[TL_POOLSIZE];
static int	marker = 0;
static char *null_string = "";

void	*TL_Alloc(int size)
{
	void *newmem;

	if (marker + size > TL_POOLSIZE)
	{
		corec.Console_DPrintf("TL_Alloc: pool size of %i bytes exceeded (couldn't allocate %i bytes)\n", TL_POOLSIZE, size);
		return NULL;
	}

	newmem = &pool[marker];

	marker += (size + 31) & -32;	//align to 32 byte intervals

	memset(newmem, 0, size);

	return newmem;
}

char	*TL_StrDup(const char *s)
{
	char *newstring;

	newstring = TL_Alloc(strlen(s)+1);

	if (newstring)
	{
		strcpy(newstring, s);
		return newstring;
	}
	else
		return null_string;
}

void	TL_Pool_Init()
{
	marker = 0;
}
