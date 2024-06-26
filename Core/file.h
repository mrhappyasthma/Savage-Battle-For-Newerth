// (C) 2003 S2 Games

// file.h

// file io functions

#define MAX_FILE_BLOCKS 1024

typedef struct
{
	char name[5];
	int pos;		//position in buffer
	unsigned int length;
	byte *data;		//data pointer
} block_t;

typedef struct
{
	int num_blocks;
	int _num_allocated;

	block_t *blocks;
} blockList_t;

void    File_ClanIconInit();
residx_t    File_GetClanIcon(int clan_id);
void    File_CacheClanIcon(int clan_id);
void	File_FlushClanIcon(int clan_id);

void    Archive_Init();
void    Archive_Shutdown();
int     Archive_RegisterArchivesInDir(const char *path);
void    Archive_RegisterOfficialArchives();
bool    Archive_RegisterArchive(const char *path, int flags);
archive_t   *Archive_GetArchive(const char *path);
file_t  *Archive_OpenFile(archive_t *archive, const char *filename, char *mode);
bool    Archive_Close(archive_t *archive);
int     Archive_UnregisterUnusedArchives(); //should only be called on a disconnect
bool    Archive_IsOfficialArchiveName(const char *partial_path);

void    File_GetHashStrings(char *hashstring, int size, int xorbits);
bool    File_CompareHashStrings(char *hashString, int xorbits);

int     File_Buffered_Import(file_t *f, char *buf, int size);
file_t 	*File_Buffer(char *filename, int memtag);
file_t  *File_NewBufferFile();
int     File_GetBuffer(file_t *file, char **buf);
int     File_GetBufferSize(file_t *file);
bool    File_ReplaceBuffer(file_t *file, char *newbuf);
bool    File_IsBuffered(file_t *file);

file_t  *File_OpenAbsolute(const char *filename, const char *mode);

file_t	*File_Open(const char *filename, const char *mode);
size_t File_Read(void *ptr, size_t size, size_t nmemb, file_t *file);
size_t File_Write (const void *ptr, size_t size, size_t nmemb, file_t *file);
int     File_Seek(file_t *file, long offset, int whence);
long    File_Tell(file_t *file);
void	File_Close(file_t *f);
bool	File_Exists(const char *filename);
bool	File_Stat(const char *filename, struct stat *stats);
#ifdef unix
bool	File_StatAbsolute(const char *filename, struct stat *stats);
int		File_SizeAbsolute(const char *filename);
void	*File_LoadIntoBufferAbsolute(const char *filename, int *length, int tag);
#endif
char	*File_GetNextFileIncrement(int num_digits, const char *basename, const char *ext, char *filename, int size);
void	File_ChangeDir(const char *dir);
int		File_Size(const char *filename);
void	File_ResetDir();
char	*File_GetCurrentDir();
char	*File_GetFullPath(const char *path);
void	File_FixPath(const char *in, char *out, bool systemPath);
void	File_Printf(file_t *f, const char *fmt, ...);
void	*File_LoadIntoBuffer(const char *filename, int *length, int tag);
void	File_FreeBuffer(void *mem);
blockList_t *File_AllocBlockList(const void *buf, int buflen, const char *filename_debug);
void	File_FreeBlockList(blockList_t *blocklist);

void	File_Flush(file_t *file);

char	File_getc(file_t *file);
char 	*File_gets(char *s, int size, file_t *f);

//export this through the core api and I'll personally hunt you down :) - Jon
bool    File_Delete(const char *filename);

void    File_SystemDir(char *directory, char *wildcard, bool recurse,
				       void(*dirCallback)(const char *dir, void *userdata),
					   void(*fileCallback)(const char *filename, void *userdata),
					   void *userdata);

void	File_Shutdown();

//utilities
int File_ReadInt(file_t *file);
float File_ReadFloat(file_t *file);
short File_ReadShort(file_t *file);
byte File_ReadByte(file_t *file);
char *File_ReadString(file_t *file, char *buf, int size);
size_t File_WriteInt(file_t *file, int i);
size_t File_WriteFloat(file_t *file, float f);
size_t File_WriteShort(file_t *file, short s);
size_t File_WriteByte(file_t *file, byte b);

void    File_AddOpenFileTracking(const char *name, file_t *f);
void    File_DelOpenFileTracking(file_t *f);
void    File_FileTrackingRename(file_t *f, const char *name);

extern char *file_last_fname;
