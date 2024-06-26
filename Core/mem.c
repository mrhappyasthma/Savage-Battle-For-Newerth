// (C) 2003 S2 Games

// mem.c
// Memory Allocation routines

#include "core.h"

cvar_t	mem_freeOnExit = { "mem_freeOnExit", "1" };

typedef struct
{
	int tag;
	char *string;
} tagstring_t;

tagstring_t tagstrings[] =
{
	{ MEM_FILESYSTEM, "MEM_FILESYSTEM" },
	{ MEM_CLIENTGAME, "MEM_CLIENTGAME" },
	{ MEM_SERVERGAME, "MEM_SERVERGAME" },
	{ MEM_WORLD, "MEM_WORLD" },
	{ MEM_SKIN, "MEM_SKIN" },
	{ MEM_SKELETON, "MEM_SKELETON" },
	{ MEM_COLLISION, "MEM_COLLISION" },
	{ MEM_MEDIA, "MEM_MEDIA" },
	{ MEM_GUI, "MEM_GUI" },
	{ MEM_GUI_CLASSNAMES, "MEM_GUI_CLASSNAMES" },
	{ MEM_VIDDRIVER, "MEM_VIDDRIVER" },
	{ MEM_SCRIPTS, "MEM_SCRIPTS" },
	{ MEM_MODEL, "MEM_MODEL" },
	{ MEM_BITMAP, "MEM_BITMAP" },
	{ MEM_BUDDIES, "MEM_BUDDIES" },
	{ MEM_ALLOCATOR, "MEM_ALLOCATOR" },
	{ MEM_CMD, "MEM_CMD" },
	{ MEM_CVAR, "MEM_CVAR" },
	{ MEM_FONT, "MEM_FONT" },
	{ MEM_SOUND, "MEM_SOUND" },
	{ MEM_SET, "MEM_SET" },
	{ MEM_PATH, "MEM_PATH" },
	{ MEM_NET, "MEM_NET" },
	{ MEM_IRC, "MEM_IRC" },
	{ MEM_HOST, "MEM_HOST" },
	{ MEM_SYSTEM, "MEM_SYSTEM" },
	{ MEM_HEAP, "MEM_HEAP" },
	{ MEM_NEEDS_TO_USE_NEW_LIB_SYSTEM, "MEM_NEEDS_TO_USE_NEW_LIB_SYSTEM" },
	{ MEM_POTENTIAL, "MEM_POTENTIAL" },
	{ MEM_SCENE, "MEM_SCENE" },
	{ MEM_STRINGTABLE, "MEM_STRINGTABLE" },
	{ MEM_HTTP, "MEM_HTTP" },
	{ MEM_FILE, "MEM_FILE" },
	{ MEM_SERVER, "MEM_SERVER" },
	{ MEM_CLIENT, "MEM_CLIENT" },
	{ MEM_CLANICONS, "MEM_CLANICONS" },
	{ MEM_SHAREDGAME, "MEM_SHAREDGAME" },
	{ MEM_BINK, "MEM_BINK" },
	{ MEM_THEORA, "MEM_THEORA" },
	{ MEM_BANS, "MEM_BANS" },
	{ MEM_GAMESCRIPT, "MEM_GAMESCRIPT" },
	{ MEM_FILEBUFFER, "MEM_FILEBUFFER" },
	{ MEM_ARCHIVE, "MEM_ARCHIVE" },
	{ MEM_ZIP, "MEM_ZIP" },
	{ MEM_ZIPBUFFER, "MEM_ZIPBUFFER" },
	{ 0, "" },
};


//#define DEBUG_ALLOCATIONS

long	bytes_copied;

#ifdef _S2_EXPORTER

void	System_Error(char *msg, ...)
{
}

#endif

void	*_Mem_Alloc(int size)
{
	void *newmem;	

	newmem = malloc(size);
	if (!newmem) 
		System_Error("Mem_Alloc: Couldn't allocate %i bytes", size);

	return newmem;
}

void	_Mem_Free(void *ptr)
{
	if (!ptr)
		return;
	free(ptr);
}

void	*_Mem_ReAlloc(void *ptr, int size)
{
	void *newmem;

	newmem = realloc(ptr, size);
	if (!newmem)
		System_Error("Mem_ReAlloc: Couldn't allocate %i bytes", size);

	return newmem;
}

void	Mem_ResetCopyCount()
{	
	bytes_copied = 0;
}

long	Mem_GetBytesCopied()
{
	return bytes_copied;
}

void	*Mem_Copy(void *dest, const void *src, size_t n)
{
	byte *ret;

	OVERHEAD_INIT;	

	if (!dest || !src)
		return NULL;

	bytes_copied += n;
	ret = memcpy(dest, src, n);

	OVERHEAD_COUNT(OVERHEAD_MEMCOPY);

	return ret;
}

void	*Mem_Move(void *dest, const void *src, size_t n)
{
	bytes_copied += n;
	return memmove(dest, src, n);
}

//memory allocation routines, for easier memory cleanup and debugging


typedef struct taglist_s
{
	int tag;
	int size;
	struct taglist_s *next;
	struct taglist_s *prev;
} taglist_t;

static taglist_t taglist;
//static taglist_t *startmarker = NULL;
//static taglist_t *endmarker = NULL;


/*==========================

  Mem_TagCount

 ==========================*/

int	Mem_TagCount()
{
	int count = 0;
	taglist_t *list;

	for (list = taglist.next; list != &taglist; list = list->next)
	{
		count++;
	}

	return count;
}


/*==========================

  Mem_TagCount_Cmd

 ==========================*/

void Mem_TagCount_Cmd(int argc, char *argv[])
{
	Console_Printf("Tag count: %i\n", Mem_TagCount());
}



/*==========================

  TagToString

 ==========================*/

char	*TagToString(int tag)
{
	int n = 0;

	while (tagstrings[n].tag)
	{
		if (tag == tagstrings[n].tag)
		{
			return tagstrings[n].string;
		}
		
		n++;
	}

	return "UNKNOWN";
}



/*==========================

  Tag_Malloc

 ==========================*/

void	*Tag_Malloc(int size, int tag)
{
	taglist_t *mem;

#ifdef DEBUG_ALLOCATIONS
	Console_DPrintf("Tag_Malloc(%i, %s): ", size, TagToString(tag));
#endif

	mem = (taglist_t *)_Mem_Alloc(size + sizeof(taglist_t));

	mem->tag = tag;
	mem->size = size;

	//zero out the memory
	memset(mem+1, 0, size);

	//add this memory block to the list
	LIST_INSERT(&taglist, mem);

#ifdef DEBUG_ALLOCATIONS
	Console_DPrintf("%p\n", mem+1);
#endif
	
	return mem+1;
}


void	*Tag_MallocSharedGame(int size)
{
	return Tag_Malloc(size, MEM_SHAREDGAME);
}

void	*Tag_MallocServerGame(int size)
{
	return Tag_Malloc(size, MEM_SERVERGAME);
}

void	*Tag_MallocClientGame(int size)
{
	return Tag_Malloc(size, MEM_CLIENTGAME);
}

void	*Tag_MallocGameScript(int size)
{
	return Tag_Malloc(size, MEM_GAMESCRIPT);
}

void	Tag_FreeGameScript()
{
	Tag_FreeAll(MEM_GAMESCRIPT);
}

/*==========================

  Tag_Free

 ==========================*/

void	Tag_Free(void *mem)
{
	taglist_t *tagmem;

#ifdef DEBUG_ALLOCATIONS
	Console_DPrintf("Tag_Free(%p)\n", mem);
#endif
	
	if (!mem)
		return;

	tagmem = ((taglist_t *)mem) - 1;

	if (tagmem->tag < TAG_FIRST || tagmem->tag > TAG_LAST)
	{
		System_Error("Invalid free attempt or invalid tag at %p\n", tagmem);
	}

	tagmem->tag = TAG_INVALID;		//set this to 0 so that subsequent Free() calls on the same mem will fail

	//remove this memory block from the list
	LIST_REMOVE(tagmem);
	
	_Mem_Free(tagmem);

}



/*==========================

  Tag_Realloc

 ==========================*/

void	*Tag_Realloc(void *oldmem, int size, int tag)
{
	taglist_t *oldtagmem;

#ifdef DEBUG_ALLOCATIONS
	Console_DPrintf("Tag_Realloc(%p, %i, %s)\n", oldmem, size, TagToString(tag));
#endif

	if (oldmem)
	{
		taglist_t *newtagmem;

		oldtagmem = ((taglist_t *)oldmem) - 1;
		if (oldtagmem->tag < TAG_FIRST || oldtagmem->tag > TAG_LAST)
			System_Error("Invalid realloc at %p\n", oldtagmem);

		LIST_REMOVE(oldtagmem);
		newtagmem = (taglist_t *)_Mem_ReAlloc(oldtagmem, size + sizeof(taglist_t));
		LIST_INSERT(&taglist, newtagmem);
		newtagmem->size = size;
		newtagmem->tag = tag;
		return newtagmem+1;
	}
		
	return Tag_Malloc(size, tag);	
}


/*==========================

  Tag_Strdup

 ==========================*/

char	*Tag_Strdup(const char *string, int tag)
{
	char *s;

#ifdef DEBUG_ALLOCATIONS
	Console_DPrintf("Tag_Strdup(%s, %s)\n", string, TagToString(tag));
#endif

	s = (char *)Tag_Malloc(strlen(string) + 1, tag);

	strcpy(s, string);
	return s;
}



/*==========================

  Tag_FreeAll

  free all blocks of memory that were allocated with 'tag'
  return the total size the blocks took up

  iterating through all the tags is slow, because the list can get HUGE

 ==========================*/

int	Tag_FreeAll(int tag)
{
	int total = 0;
	taglist_t *list, *next;

#ifdef DEBUG_ALLOCATIONS
	Console_DPrintf("Tag_FreeAll(%s)\n", TagToString(tag));
#endif

	for (list = taglist.next; list != &taglist; list = next)
	{
		next = list->next;
		if (list->tag == tag)
		{
			total += list->size + sizeof(taglist_t);
			Tag_Free(list+1);
		}
	}

	return total;
}


/*==========================

  Mem_ShutDown

  iterating through all the tags is slow, because the list can get HUGE

 ==========================*/

void	Mem_ShutDown()
{
	taglist_t *list, *next;

	Console_DPrintf("Mem_ShutDown()\n");

	if (mem_freeOnExit.integer)
	{
		for (list = taglist.next; list != &taglist; list = next)
		{
			next = list->next;
			Tag_Free(list+1);
		}
	}
}


/*==========================

  Mem_CountTagBytes

 ==========================*/

int		Mem_CountTagBytes(int tag)
{
	int size = 0;
	taglist_t *list;

	for (list = taglist.next; list != &taglist; list = list->next)
	{
		if (tag == list->tag)
			size += list->size + sizeof(taglist_t);
	}

	return size;
}


/*==========================

  Mem_CountTagEntries

 ==========================*/

int		Mem_CountTagEntries(int tag)
{
	int entries = 0;
	taglist_t *list;

	for (list = taglist.next; list != &taglist; list = list->next)
	{
		if (tag == list->tag)
			entries++;
	}

	return entries;
}


/*==========================

  Mem_MemStat_Cmd

  display memory usage info

 ==========================*/

void	Mem_MemStat_Cmd(int argc, char *argv[])
{
	int n = 0;
	int totalbytes = 0;
	int totalentries = 0;

	Console_Printf("Tagged memory statistics:\n\n");

	while (tagstrings[n].tag)
	{
		int bytes,entries;

		if ( argc==0 || (argv[0] && !stricmp(tagstrings[n].string, argv[0])) )
		{
			Console_Printf("%s\n", tagstrings[n].string);

			bytes = Mem_CountTagBytes(tagstrings[n].tag);
			entries = Mem_CountTagEntries(tagstrings[n].tag);

			Console_Printf("Bytes used: %i\n", bytes);
			Console_Printf("Entries: %i\n", entries);

			totalbytes += bytes;
			totalentries += entries;
		}

		n++;
	}

	Console_Printf("\nTotal bytes: %i\n", totalbytes);
	Console_Printf("\nTotal entries: %i\n", totalentries);
}


/*==========================

  Mem_Init

 ==========================*/

void	Mem_Init()
{
	LIST_CLEAR(&taglist);	
}

void	Mem_RegisterCmds()
{
	Cmd_Register("tagcount", Mem_TagCount_Cmd);
	Cmd_Register("memstat", Mem_MemStat_Cmd);
}
