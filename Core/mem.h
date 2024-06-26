// (C) 2003 S2 Games

// mem.h

#define TAG_INVALID			0
#define TAG_FIRST			0x10000000
#define MEM_FILESYSTEM		0x10000000		//memory allocated by the file system
#define MEM_CLIENTGAME		0x10000001		//memory allocated from the game DLL, client side
#define MEM_SERVERGAME		0x10000002		//memory allocated from the game DLL, server side
#define MEM_WORLD			0x10000003		//data needed for the game world while it's loaded
#define MEM_SKIN			0x10000004
#define MEM_SKELETON		0x10000005		//memory used by the skeletal animation system
#define MEM_COLLISION		0x10000006		//memory used by the collision system
#define MEM_MEDIA			0x10000007		//memory used by the media resource system (res.c)
#define MEM_GUI				0x10000008 		//memory used by the GUI
#define MEM_GUI_CLASSNAMES	0x10000009 		//memory used by the GUI classnames
#define MEM_VIDDRIVER		0x10000010
#define MEM_SCRIPTS			0x10000011
#define MEM_MODEL			0x10000012		//memory used by models
#define MEM_BITMAP			0x10000013
#define MEM_BUDDIES			0x10000014
#define MEM_ALLOCATOR		0x10000015
#define MEM_CMD				0x10000016
#define MEM_CVAR			0x10000017
#define MEM_FONT			0x10000018
#define MEM_SOUND			0x10000019
#define MEM_SET				0x10000020
#define MEM_PATH			0x10000021
#define MEM_NET				0x10000022
#define MEM_IRC				0x10000023
#define MEM_HOST			0x10000024
#define MEM_SYSTEM			0x10000025
#define MEM_HEAP			0x10000026
#define MEM_NEEDS_TO_USE_NEW_LIB_SYSTEM	0x10000027
#define MEM_POTENTIAL		0x10000028
#define MEM_SCENE			0x10000029
#define MEM_STRINGTABLE		0x10000030
#define MEM_HTTP			0x10000031
#define MEM_FILE			0x10000032
#define MEM_SERVER			0x10000033
#define MEM_CLIENT			0x10000034
#define MEM_CLANICONS		0x10000035
#define MEM_SHAREDGAME		0x10000036
#define	MEM_BINK			0x10000037
#define	MEM_THEORA			0x10000038
#define	MEM_BANS			0x10000039
#define	MEM_GAMESCRIPT		0x10000040
#define	MEM_FILEBUFFER		0x10000041
#define MEM_ARCHIVE			0x10000042
#define MEM_ZIP				0x10000043
#define MEM_ZIPBUFFER		0x10000044
#define MEM_AI				0x10000045
#define TAG_LAST			0x10000045

void    *Mem_Copy(void *dest, const void *src, size_t n);
void    *Mem_Move(void *dest, const void *src, size_t n);
void    Mem_ResetCopyCount();
long    Mem_GetBytesCopied();
void	Mem_Init();
void	*Tag_Malloc(int size, int tag);
void	*Tag_MallocSharedGame(int size);
void	*Tag_MallocServerGame(int size);
void	*Tag_MallocClientGame(int size);
void	*Tag_MallocGameScript(int size);
void	Tag_FreeGameScript();
void	Tag_Free(void *mem);
void	*Tag_Realloc(void *oldmem, int size, int tag);
char	*Tag_Strdup(const char *string, int tag);
int		Tag_FreeAll(int tag);
void	Mem_ShutDown();
void    Mem_RegisterCmds();
