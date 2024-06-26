// (C) 2003 S2 Games

// file.c

// file io functions

#include "core.h"
#include "unzip.h"

#define	MAX_OPEN_TRACKED_FILES 100

char *null_string = "";
char buffer[4096];
char current_dir[1024] = "/";

typedef struct
{
	file_t *f;
	char filename[256];
} openFileTracker_t;

openFileTracker_t open_files[MAX_OPEN_TRACKED_FILES] = { { 0, "" } };

extern FILE *filenamelist_file;

#define MAX_ARCHIVE_FILES 100
#define MAX_CLAN_ICONS 100

archive_t archives[MAX_ARCHIVE_FILES];

#ifdef SHOW_MISSING_FILES
file_t *MISSINGFILES = NULL;
#endif

GHashTable *clanHash = NULL;

extern cvar_t clan_icon_url;
extern cvar_t world_save_path;
extern cvar_t svr_pure;
extern cvar_t homedir;
extern cvar_t archive_precedence;

extern char dll_hash[MAX_HASH_SIZE];
extern int dll_hashlen;
extern char engine_hash[MAX_HASH_SIZE];
extern int engine_hashlen;

int	numOpenFiles = 0;

int	_getSlot()
{
	int i = 0;
	while (i < MAX_OPEN_TRACKED_FILES)
	{
		if (!open_files[i].filename[0])
			return i;
		i++;
	}
	Console_Printf("Error, no free slots, recycling slot 0!\n");
	return 0;
}

void	File_FileTrackingRename(file_t *f, const char *name)
{
	int i = 0;

	while (i < MAX_OPEN_TRACKED_FILES)
	{
		if (open_files[i].f == f)
		{
			strncpy(open_files[i].filename, name, 255);
			open_files[i].filename[255] = 0;
			return;
		}
		i++;
	}
}

void	File_AddOpenFileTracking(const char *name, file_t *f)
{
	int i = _getSlot();

	if (!f)
		Console_Printf("Error trying to track null file %s\n", name);
	
	strncpy(open_files[i].filename, name, 256);
	open_files[i].f = f;
}

void	File_DelOpenFileTracking(file_t *f)
{
	int i = 0;

	while (i < MAX_OPEN_TRACKED_FILES)
	{
		if (open_files[i].f == f)
		{
			strcpy(open_files[i].filename, "");
			open_files[i].f = NULL;
			return;
		}
		i++;
	}
	Console_Printf("trying to delete tracking of open file %p, but we can't find it\n", f);
}

void	Archive_SystemDir(archive_t *archive, char *directory, char *wildcard, bool recurse,
					   void(*dirCallback)(const char *dir, void *userdata),
					   void(*fileCallback)(const char *filename, void *userdata),
					   void *userdata)
{
	switch (archive->type)
	{
		case ARCHIVE_TYPE_PATH:
				//search for the file in this patch
				break;
		case ARCHIVE_TYPE_ZIP:
				archive->System_Dir(archive->data, directory, wildcard, recurse, dirCallback, fileCallback, userdata);
				break;
		default:
				Console_DPrintf("Unknown archive type %i\n", archive->type);
				break;
	}
}

bool	Archive_FileExists(archive_t *archive, const char *filename)
{
	const char *relative_filename = filename;
	if (filename[0] == '/')
		relative_filename = &filename[1];

	switch (archive->type)
	{
		case ARCHIVE_TYPE_PATH:
				//search for the file in this patch
				break;
		case ARCHIVE_TYPE_ZIP:
				return ZIP_FileExists(archive->data, relative_filename);
				break;
		default:
				Console_DPrintf("Unknown archive type %i\n", archive->type);
				break;
	}
	return false;
}

bool	Archive_Stat(archive_t *archive, const char *filename, struct stat *stats)
{
	const char *relative_filename = filename;
	if (filename[0] == '/')
		relative_filename = &filename[1];

	switch (archive->type)
	{
		case ARCHIVE_TYPE_PATH:
				//search for the file in this patch
				break;
		case ARCHIVE_TYPE_ZIP:
				return ZIP_StatFile(archive->data, relative_filename, stats);
				break;
		default:
				Console_DPrintf("Unknown archive type %i\n", archive->type);
				break;
	}
	return false;
}

bool	Archive_StatFile(const char *filename, struct stat *stats)
{
	char partial_path[256];
	char *archive_path;
	int i;

	File_FixPath(filename, partial_path, false);
	for (i = 0; i < MAX_ARCHIVE_FILES; i++)
	{
		if (!archives[i].active)
			continue;

		archive_path = partial_path;
		if (strncmp(archive_path, &GAME_PATH[1], strlen(&GAME_PATH[1])) == 0)
		{
			archive_path = &archive_path[strlen(&GAME_PATH[1])];
		}
		if (Archive_Stat(&archives[i], archive_path, stats))
			return true;
	}

	return false;
}

file_t	*Archive_OpenFile(archive_t *archive, const char *filename, char *mode)
{
	const char *relative_filename = filename;
	if (filename[0] == '/')
		relative_filename = &filename[1];

	switch (archive->type)
	{
		case ARCHIVE_TYPE_PATH:
				//search for the file in this patch
				break;
		case ARCHIVE_TYPE_ZIP:
				return ZIP_OpenFile(archive->data, relative_filename, mode);
				break;
		default:
				Console_DPrintf("Unknown archive type %i\n", archive->type);
				break;
	}
	return false;

}

archive_t	*Archive_GetArchive(const char *path)
{
	int i = MAX_ARCHIVE_FILES - 1;
	char *fullpath;
	char tmp[1024];
	
					/*fmt("%s%s%s%s", System_GetRootDir(), 
					DLLTYPE == DLLTYPE_EDITOR ? world_save_path.string : "", 
					DLLTYPE == DLLTYPE_EDITOR ? "/" : "", 
					path[0] == '/' ? &path[1] : path));*/

	if (DLLTYPE == DLLTYPE_EDITOR)
	{
		if (File_Exists(fmt("%s%s", GAME_PATH, path)))
		{
			File_FixPath(fmt("%s%s", GAME_PATH, path), tmp, true);
		}
		else
			File_FixPath(path, tmp, true);
	}
	else
	{
		File_FixPath(path, tmp, true);
	}

#ifdef _WIN32
	fullpath = fmt("%s", tmp);
#else
	fullpath = fmt("%s%s", tmp[0] == '/' ? "" : "/", tmp);
#endif
	
	while (i >= 0 && archives[i].active && strcmp(archives[i].filename, fullpath) != 0)
	{
		Console_DPrintf("Comparing %s with %s\n", fullpath, archives[i].filename);
		i--;
	}

	if (i < 0 || !archives[i].active || strcmp(archives[i].filename, fullpath) != 0)
	{
		if (DLLTYPE == DLLTYPE_EDITOR)
		{
			i = MAX_ARCHIVE_FILES - 1;

			File_FixPath(fmt("%s/%s", world_save_path.string, path), fullpath, true);

			while (i <= 0 && archives[i].active && strcmp(archives[i].filename, fullpath) != 0)
			{
				i--;
			}
		}
	}
	if (i < 0 || !archives[i].active || strcmp(archives[i].filename, fullpath) != 0)
		return NULL;

	return &archives[i];
}

#define SAVAGE_DEMO_CORRUPT_INSTALL_MSG "Your Savage demo installation is corrupt.  Please reinstall the Savage demo."
#define NUM_DEMO_ARCHIVES 5		//be sure to modify this whenever the demo install changes

#ifdef SAVAGE_DEMO
bool	Archive_DemoHashCompare(archive_t *archive, const char *filename, const char *hashstring)
{
	char *archname = Filename_GetFilename(archive->filename);

	if (strcmp(filename, archname)!=0)
		return false;

	if (strcmp(hashstring, BinaryToHexWithXor(archive->hash, archive->hashlen, 0))!=0)
	{
		//Console_Printf("The hash of %s should be %s\n", filename, BinaryToHexWithXor(archive->hash, archive->hashlen, 0));
		return false;
	}

	return true;
}

#endif

bool	Archive_RegisterArchive(const char *path, int flags)
{
	int i = MAX_ARCHIVE_FILES - 1;
	char fullpath[2048] = {0};

	if (Archive_GetArchive(path))
		return true;
	
	while (i >= 0 && archives[i].active)
	{
		i--;
	}

	if (i < 0)
		return false;

	File_FixPath(path, fullpath, true);//File_GetFullPath(fmt("%s%s", System_GetRootDir(), path[0] == '/' ? &path[1] : path));

	if (strstr(fullpath, ".s2z"))
	{
		archives[i].data = ZIP_Open(fullpath, archives[i].hash, &archives[i].hashlen);
		if (!archives[i].data)
		{
			if (DLLTYPE == DLLTYPE_EDITOR || DLLTYPE == DLLTYPE_NOTLOADED)
			{
				File_FixPath(fmt("%s%s", GAME_PATH, path), fullpath, true);//File_GetFullPath(fmt("%s%s", System_GetRootDir(), path[0] == '/' ? &path[1] : path));
				archives[i].data = ZIP_Open(fullpath, archives[i].hash, &archives[i].hashlen);
			}
			if (!archives[i].data)
			{ 
				Console_Printf("Invalid archive '%s' - loading failed\n", path);
				return false;
			}
		}
		archives[i].filename = Tag_Strdup(fullpath, MEM_FILE);
		
		archives[i].type = ARCHIVE_TYPE_ZIP;
		archives[i].System_Dir = ZIP_SystemDir;
		archives[i].flags = flags;
		archives[i].active = true;

#ifdef SAVAGE_DEMO
		//Console_Printf("filename = %s, hash = %s\n", archives[i].filename, BinaryToHexWithXor(archives[i].hash, archives[i].hashlen, 0));

		if (
			!Archive_DemoHashCompare(&archives[i], "savage0.s2z", "cb484349925878d0aa0464b1cc1faa37e19ffaa8") &&
			!Archive_DemoHashCompare(&archives[i], "images0.s2z", "1a2292e0a54fd37b247326a191783900cae3c5f8") &&
			!Archive_DemoHashCompare(&archives[i], "gui0.s2z", "fdd9de5822066ee644567a97178d84d470aea69f") &&			
			!Archive_DemoHashCompare(&archives[i], "sounds0.s2z", "28fd0b29d77206a3e75c60a0002e350920abc07b") &&
			!Archive_DemoHashCompare(&archives[i], "eden2.s2z", "e6070a1994375aae91f7847ffb8088bf12fb88b7") &&
			!Archive_DemoHashCompare(&archives[i], "crossroads.s2z", "5fab63e4236dd3c043144e814a1d1f2f98a1d7bf"))
			System_Error(SAVAGE_DEMO_CORRUPT_INSTALL_MSG);
#endif //SAVAGE_DEMO

		return true;
	}
	else if (path[strlen(path)-1] == '/' 
			|| path[strlen(path)-1] == '\\')
	{
		archives[i].filename = NULL;
		archives[i].type = ARCHIVE_TYPE_PATH;
		archives[i].data = strdup(fullpath);
		archives[i].System_Dir = NULL;
		archives[i].active = true;

		return true;
	}

	return false;
}

int		Archive_UnregisterUnusedArchives()
{
	int i;
	int count = 0;

	for (i = 0; i < MAX_ARCHIVE_FILES; i++)
	{
		if (!archives[i].active)
			continue;

		if (archives[i].flags & ARCHIVE_THIS_CONNECTION_ONLY)
		{
			Archive_Close(&archives[i]);
			count++;
		}
	}
	return count;
}

bool	Archive_Close(archive_t *archive)
{
	switch (archive->type)
	{
		case ARCHIVE_TYPE_ZIP:
				ZIP_Close(archive->data);
				break;
		default:
				Console_Printf("Unknown archive type %i - not closing\n", archive->type);
				return false;
	}
	archive->active = false;
	if (archives->filename)
	{
		Tag_Free(archives->filename);
		archive->filename = NULL;
	}
	return true;
}

void	Archives_List_Cmd(int argc, char *argv[])
{
	int i, j;

	Console_Printf("Listing open archives, in order or priority (first = highest priority)\n");
	j = 0;
	
	for (i = 0; i < MAX_ARCHIVE_FILES; i++)
	{
		if (!archives[i].active)
			continue;

		Console_Printf("%i. %s\n", j, archives[i].filename);
		j++;
	}
}

file_t *Archives_OpenFile(const char *filename, const char *mode)
{
	char partial_path[1024];
	char *archive_path;
	file_t *f;
	int i;

	File_FixPath(filename, partial_path, false);
	archive_path = partial_path;
	if (strncmp(archive_path, &GAME_PATH[1], strlen(&GAME_PATH[1])) == 0)
	{
		archive_path = &archive_path[strlen(&GAME_PATH[1])];
	}
		
	for (i = 0; i < MAX_ARCHIVE_FILES; i++)
	{
		if (!archives[i].active)
			continue;

		f = Archive_OpenFile(&archives[i], archive_path, (char *)mode);

		if (f)
		{
			Console_DPrintf("Opening %s in archive %s\n", archive_path, archives[i].filename);
			return f;
		}
	}
	return NULL;
}

bool	Archives_FileExists(const char *filename)
{
	int i;
	char *archive_path;

	for (i = 0; i < MAX_ARCHIVE_FILES; i++)
	{
		if (!archives[i].active)
			continue;
		
		archive_path = (char *)filename;
		if (strncmp(archive_path, &GAME_PATH[1], strlen(&GAME_PATH[1])) == 0)
		{
			archive_path = &archive_path[strlen(&GAME_PATH[1])];
		}
		if (Archive_FileExists(&archives[i], archive_path))
			return true;
	}

	return false;
}

void	File_SystemDir(char *directory, char *wildcard, bool recurse,
					   void(*dirCallback)(const char *dir, void *userdata),
					   void(*fileCallback)(const char *filename, void *userdata),
					   void *userdata)
{
	int i;

	for (i = 0; i < MAX_ARCHIVE_FILES; i++)
	{
		if (!archives[i].active || !archives[i].System_Dir)
			continue;

		Archive_SystemDir(&archives[i], directory, wildcard, recurse, dirCallback, fileCallback, userdata);
	}
}

void	PrintOpenFiles()
{
	int i = 0, j = 0;

	while (i < MAX_OPEN_TRACKED_FILES)
	{
		if (open_files[i].filename[0])
		{
			Console_Printf("%i. '%s'\n", j, open_files[i].filename);
			j++;
		}
		i++;
	}
}

void	File_ListOpenFiles_Cmd(int argc, char *argv[])
{
	PrintOpenFiles();
}

void	File_FlushClanIcon(int clan_id)
{
	g_hash_table_remove(clanHash, &clan_id);
}

void	File_CacheClanIcon(int clan_id)
{
	residx_t res;
	bitmap_t image;
	file_t *f;
	int *ptr;

	if (File_GetClanIcon(clan_id) > 0)
		return;
	
	f = HTTP_OpenFileNonBlocking(fmt("%s%i.png", clan_icon_url.string, clan_id));
	if (!f)
	{
		return; //don't have it yet
	}
	else if (f->eof(f)) //is it empty (i.e. the request failed?)
	{
		File_Close(f);
		Console_Printf("Couldn't load clan icon for clan %i\n", clan_id);
		ptr = Tag_Malloc(sizeof(int), MEM_CLANICONS);
		*ptr = clan_id;
		g_hash_table_insert(clanHash, ptr, GUINT_TO_POINTER(1));
		return;
	}
	
	if (Bitmap_LoadPNGFile(f, &image))
	{
		res = Res_LoadRawTextureFromMemoryEx(0, &image, SHD_FULL_QUALITY | SHD_NO_MIPMAPS | SHD_NO_COMPRESS);
		ptr = Tag_Malloc(sizeof(int), MEM_CLANICONS);
		*ptr = clan_id;
		g_hash_table_insert(clanHash, ptr, GUINT_TO_POINTER(res));
		if (!g_hash_table_lookup(clanHash, &clan_id))
		{
			Console_Printf("hash table insert failed!\n");
		}
	}
	else
	{
		ptr = Tag_Malloc(sizeof(int), MEM_CLANICONS);
		*ptr = clan_id;
		g_hash_table_insert(clanHash, ptr, GUINT_TO_POINTER(1));
	}

	File_Close(f);
}

residx_t	File_GetClanIcon(int clan_id)
{
	return GPOINTER_TO_UINT(g_hash_table_lookup(clanHash, &clan_id));
}

void	File_ClanIconInit()
{
	if (clanHash)
	{
		g_hash_table_destroy(clanHash);
	}
	Tag_FreeAll(MEM_CLANICONS);
	clanHash = g_hash_table_new(g_int_hash, g_int_equal);
}

void	File_GetHashStrings(char *hashstring, int size, int xorbits)
{
	int i = 0;
	char *publicHash;
	
	//Console_Printf("the xorbits are %i\n", xorbits);
	
	while (i < MAX_ARCHIVE_FILES)
	{
		if ((archives[i].flags & ARCHIVE_OFFICIAL
			|| archives[i].flags & ARCHIVE_THIS_CONNECTION_ONLY)
			&& archives[i].filename)
		{
			publicHash = BinaryToHexWithXor(archives[i].hash, archives[i].hashlen, xorbits);
			ST_SetState(hashstring, Filename_GetFilename(archives[i].filename), publicHash, size);
			//Console_Printf("Sending archive hash %s\n", publicHash);
		}
		i++;
	}

	//also include the game dll hash
	publicHash = BinaryToHexWithXor(dll_hash, dll_hashlen, xorbits);
#ifdef _WIN32
	ST_SetState(hashstring, "dll", publicHash, size);
#else
	ST_SetState(hashstring, "so", publicHash, size);
#endif
	//Console_Printf("Sending dll hash %s\n", publicHash);

	//also include the engine hash
	publicHash = BinaryToHexWithXor(engine_hash, engine_hashlen, xorbits);
#ifdef _WIN32
	ST_SetState(hashstring, "win32engine", publicHash, size);
#else
	ST_SetState(hashstring, "linuxengine", publicHash, size);
#endif
	//Console_Printf("Sending engine hash %s\n", publicHash);
}

#if 0
void	File_PrintHash_Cmd(int argc, char *argv[])
{
	char string[1024];

	File_GetHashStrings(string, 1024, 0);

	Console_Printf("%s\n", string);
}
#endif

bool	File_DLLHashOkay(const char *filename, const char *hash)
{
	file_t *f;
	char line[512];
	char *marker;

	f = File_Open("hashes.txt", "r");
	if (!f)
		return true; //why bother with pure when we have no hashes?

	while (File_gets(line, 512, f))
	{
		if ((marker = strchr(line, ':')))
		{
			marker[0] = 0;
			marker++;
			if (stricmp(filename, line) == 0)
			{
				Console_Printf("Comparing the file hash %s with %s\n", marker, hash);
				if (strcmp(hash, marker) == 0)
				{
					Console_Printf("Match!\n");
					File_Close(f);
					return true;
				}
			}
		}
	}
	
	File_Close(f);
	return false;
}

bool    File_CompareHashStrings(char *hashString, int xorbits)
{
	int len, i = 0;
	int numClient, numServer = 0;
	char binaryHash[MAX_HASH_SIZE+1];
	char *hash;
	const char *s;
	bool success = true;
		
	Console_Printf("the client's xorbits are %i\n", xorbits);
	
	numClient = ST_ForeachState(hashString, NULL);

	hash = ST_GetState(hashString, "dll");
	if (hash[0])
	{
		len = HexToBinary(hash, binaryHash, MAX_HASH_SIZE);
		hash = BinaryToHexWithXor(binaryHash, len, xorbits);
		if (!File_DLLHashOkay("dll", hash))
		{
			Console_Printf(".dll hash (%s) doesn't match acceptable list\n", hash);
			if (svr_pure.integer > 1)
				success = false;
		}
		numServer++;
	}
	
	hash = ST_GetState(hashString, "so");
	if (hash[0])
	{
		len = HexToBinary(hash, binaryHash, MAX_HASH_SIZE);
		hash = BinaryToHexWithXor(binaryHash, len, xorbits);
		if (!File_DLLHashOkay("so", hash))
		{
			Console_Printf(".so hash (%s) doesn't match acceptable list\n", hash);
			if (svr_pure.integer > 1)
				success = false;
		}
		numServer++;
	}
	
	hash = ST_GetState(hashString, "win32engine");
	if (hash[0])
	{
		len = HexToBinary(hash, binaryHash, MAX_HASH_SIZE);
		hash = BinaryToHexWithXor(binaryHash, len, xorbits);
		if (!File_DLLHashOkay("win32engine", hash))
		{
			Console_Printf("win32 binary hash (%s) doesn't match acceptable list\n", hash);
			if (svr_pure.integer > 1)
				success = false;
		}
		numServer++;
	}
	
	hash = ST_GetState(hashString, "linuxengine");
	if (hash[0])
	{
		len = HexToBinary(hash, binaryHash, MAX_HASH_SIZE);
		hash = BinaryToHexWithXor(binaryHash, len, xorbits);
		if (!File_DLLHashOkay("linuxengine", hash))
		{
			Console_Printf("linux binary hash (%s) doesn't match acceptable list\n", hash);
			if (svr_pure.integer > 1)
				success = false;
		}
		numServer++;
	}
	
	i = 0;
	while (i < MAX_ARCHIVE_FILES)
	{
		if (archives[i].flags & ARCHIVE_OFFICIAL && archives[i].filename)
		{
			s = Filename_GetFilename(archives[i].filename);
			hash = ST_GetState(hashString, s);

			len = HexToBinary(hash, binaryHash, MAX_HASH_SIZE);
			hash = BinaryToHexWithXor(binaryHash, len, xorbits);
			if (!File_DLLHashOkay(s, hash))
			{
				Console_Printf("The client file %s differs from ours (they have hash %s)\n", Filename_GetFilename(archives[i].filename), hash);
				success = false; //client doesn't match server
			}
			numServer++;
		}
		i++;
	}
		
	if (numClient != numServer)
	{
		if (numClient > numServer)
			Console_Printf("the client has %i more files than the server\n", numClient - numServer);
		else
			Console_Printf("the client has %i less files than the server\n", numServer - numClient);
		success = false; //client has more files than the server
	}
		
	return success;
}

bool	Archive_IsOfficialArchiveName(const char *partial_path)
{
	unsigned int number;

	if (!sscanf(partial_path, "/savage%u.s2z", &number)
		&& !sscanf(partial_path, "/gui%u.s2z", &number)
		&& !sscanf(partial_path, "/opt%u.s2z", &number)
		&& !sscanf(partial_path, "/images%u.s2z", &number)
		&& !sscanf(partial_path, "/sounds%u.s2z", &number))
	{
		return false;
	}

	return true;
}

void    Archive_FileCallback(const char *filename, void *userdata)
{
	char partial_path[2048];
	bool officialArchive = (int)userdata;
	
	File_FixPath(filename, partial_path, false);
	if (!Archive_IsOfficialArchiveName(partial_path))
		Archive_RegisterArchive(partial_path, officialArchive);
}


int		Archive_RegisterArchivesInDir(const char *path)
{
	bool officialArchive = false;
	
	System_Dir((char *)path, "*.s2z", false, NULL, Archive_FileCallback, (void *)officialArchive);
	return true;
}

void	Archive_RegisterArchivesInDir_cmd(int argc, char *argv[])
{
	if (!argc)
		return;

	Archive_RegisterArchivesInDir(argv[0]);
}

void	Archive_RegisterOfficialArchives()
{
	unsigned int i;

	i = 0;
	while (i < MAX_ARCHIVE_FILES)
	{
		if (!File_Exists(fmt("opt%u.s2z", i)))
			break;
		Archive_RegisterArchive(fmt("opt%u.s2z", i), true);
		i++;
	}

	i = 0;
	while (i < MAX_ARCHIVE_FILES)
	{
		if (!File_Exists(fmt("sounds%u.s2z", i)))
			break;
		Archive_RegisterArchive(fmt("sounds%u.s2z", i), false);  //don't make these official so people can modify them
		i++;
	}
	
	i = 0;
	while (i < MAX_ARCHIVE_FILES)
	{
		if (!File_Exists(fmt("gui%u.s2z", i)))
			break;
		Archive_RegisterArchive(fmt("gui%u.s2z", i), false); //don't make these official so people can modify them
		i++;
	}

	i = 0;
	while (i < MAX_ARCHIVE_FILES)
	{
		if (!File_Exists(fmt("images%u.s2z", i)))
			break;
		Archive_RegisterArchive(fmt("images%u.s2z", i), true);
		i++;
	}

	i = 0;
	while (i < MAX_ARCHIVE_FILES)
	{
		if (!File_Exists(fmt("savage%u.s2z", i)))
			break;
		Archive_RegisterArchive(fmt("savage%u.s2z", i), true);
		i++;
	}

#ifdef SAVAGE_DEMO
	{
		int num_archives;
		i = 0;
		num_archives = 0;
		while (i < MAX_ARCHIVE_FILES)
		{
			if (archives[i].active)
				num_archives++;
			i++;
		}/*
		if (num_archives != NUM_DEMO_ARCHIVES)
			System_Error(fmt("%s [%i %i]", SAVAGE_DEMO_CORRUPT_INSTALL_MSG, i, num_archives));*/
	}
#endif
}

void	Archive_Init()
{
#ifdef SHOW_MISSING_FILES
	MISSINGFILES = File_Open("missingfiles.txt", "w");
#endif

	memset(archives, 0, sizeof(archive_t) * MAX_ARCHIVE_FILES);
	Cmd_Register("listopenfiles", File_ListOpenFiles_Cmd);
	Cmd_Register("listarchives", Archives_List_Cmd);
	//Cmd_Register("addarchives", Archive_RegisterArchivesInDir_cmd);
#if 0
	Cmd_Register("printhash", File_PrintHash_Cmd);
#endif
}

void	Archive_Shutdown()
{
	int i;

	for (i = 0; i < MAX_ARCHIVE_FILES; i++)
	{
		if (archives[i].active)
		{
			switch (archives[i].type)
			{
				case ARCHIVE_TYPE_ZIP:
						ZIP_Close(archives[i].data);
						break;
				default:
						Console_Printf("Unknown archive type %i - not closing\n", archives[i].type);
			}
			archives[i].active = false;
		}
	}
#ifdef SHOW_MISSING_FILES
	File_Close(MISSINGFILES);
	MISSINGFILES = NULL;
#endif
}

//modifies the string to take out ../'s
char	*File_ProcessPathHacks(char *path)
{
	char *tmpptr, *parentstring, *orig_path = path;

	while ((tmpptr = strstr(path, "//")))
	{
		Mem_Move(tmpptr, tmpptr+1, strlen(tmpptr+1)+1);
	}
	
	while ((tmpptr = strstr(path, "/..")))
	{
		tmpptr[0] = 0;
		tmpptr += 3; //point to the rest of the string
		
		parentstring = strrchr(path, '/');
		if (!parentstring)
		{
			//move the "original" pointer up to not include this first /.., since we can't get rid of it
			path = tmpptr;
			//un-null it 
			tmpptr -= 3; //point to the rest of the string
			tmpptr[0] = '/';
			continue;
		}

		Mem_Move(parentstring, tmpptr, strlen(tmpptr)+1);
	}

	if (DLLTYPE == DLLTYPE_GAME)
	{
		if (strncmp(path, GAME_PATH, strlen(GAME_PATH)) == 0)
		{
			Mem_Move(path, &path[strlen(GAME_PATH)], strlen(&path[strlen(GAME_PATH)])+1);
		}
		if (strncmp(path, &GAME_PATH[1], strlen(&GAME_PATH[1])) == 0)
		{
			Mem_Move(path, &path[strlen(&GAME_PATH[1])], strlen(&path[strlen(&GAME_PATH[1])])+1);
		}

	}
	else if (DLLTYPE == DLLTYPE_EDITOR)
	{
	}
	/*
	 * this kind of path makes mods hell, but the warning isn't useful until we worry about mods
	if (strstr(path, "../game"))
		printf("damn path...\n");
	*/

	return orig_path;
}

char	*File_GetFullPath(const char *path)
{
	static char fullpath[4][1024] = { "", "", "", "" };
	static unsigned int marker = 0;
	unsigned int idx = marker % 4;
	char *filestart;
	bool startAtRoot;

	if (path[0] == '/')
	{
		startAtRoot = true;
		filestart = (char *)&path[1];
	}
	else
	{
		startAtRoot = false;
		filestart = (char *)&path[0];
	}

	BPrintf(fullpath[idx], 1023, "%s%s", startAtRoot ? "" : current_dir, filestart);

	File_ProcessPathHacks(fullpath[idx]);
	marker++;

	return fullpath[idx];
}

static char __fullpath[1024];

//safe version we use for File_Open
char *File_GetFullPathInternal(const char *path)
{	
	char *filestart;
	bool startAtRoot;

	if (path[0] == '/')
	{
		startAtRoot = true;
		filestart = (char *)&path[1];
	}
	else
	{
		startAtRoot = false;
		filestart = (char *)&path[0];
	}

	BPrintf(__fullpath, 1023, "%s%s", startAtRoot ? "" : current_dir, filestart);

	File_ProcessPathHacks(__fullpath);

	return __fullpath;
}

void	File_FixPath(const char *in, char *out, bool systemPath)
{
	bool previousSlash = false;
	char tmp_fname[1024];
	char *s, *internal;

	internal = File_GetFullPathInternal(in);
	
	if (systemPath)
		BPrintf(tmp_fname, 1024, "%s%s", System_GetRootDir(), internal);
	else
		strncpySafe(tmp_fname, internal, 1024);

	tmp_fname[1023] = 0;
	s = tmp_fname;

	//remove double slashes and change backslashes to forward slashes
	while (*s)
	{
		if (*s == '\\' || *s == '/')
		{
			if (!previousSlash)
			{
				*out = *s;
				previousSlash = true;
				out++;
			}
		}
		else
		{
			previousSlash = false;
			*out = *s;
			out++;
		}

		s++;		
	}

	*out = 0;			//null terminate it
}


extern	scriptBufferEntry_t scriptBuffer[SCRIPT_BUFFER_SIZE];

typedef struct
{
	char *buf;
	int curPos;
	int length;
} bufferedFile_t;

size_t File_Buffered_Read(void *ptr, size_t size, size_t nmemb, file_t *file)
{
	int length;
	bufferedFile_t *bFile = file->file;

	length = MIN(bFile->length - bFile->curPos, (int)size * nmemb);
	memcpy(ptr, &bFile->buf[bFile->curPos], length);
	bFile->curPos += length;
	return length;
}

int		File_Buffered_Close(file_t *f)
{
	bufferedFile_t *bFile = f->file;
	if (bFile->buf)
		Tag_Free(bFile->buf);
	Tag_Free(bFile);
	return true;
}

int		File_Buffered_Eof(file_t *f)
{
	bufferedFile_t *bFile = f->file;
	return bFile->curPos >= bFile->length - 1;
}

int		File_Buffered_Import(file_t *f, char *buf, int size)
{
	bufferedFile_t *bFile = f->file;
	//bFile->buf = Tag_Malloc(size, MEM_FILE);
	//Mem_Copy(bFile->buf, buf, size);
	bFile->buf = buf;
	bFile->length = size;
	bFile->curPos = 0;
	return size;
}

file_t	*File_NewBufferFile()
{
	file_t *file;
	file = Tag_Malloc(sizeof(file_t), MEM_FILE);
	file->file = Tag_Malloc(sizeof(bufferedFile_t), MEM_FILE);
	memset(file->file, 0, sizeof(bufferedFile_t));
	file->read = File_Buffered_Read;
	file->eof = File_Buffered_Eof;
	file->close = File_Buffered_Close;
	file->flush = NULL;
	file->buffered = true;

	File_AddOpenFileTracking("--temporary file--", file);
	return file;
}

file_t *File_Buffer(char *filename, int memtag)
{
	file_t *file, *bFile;
	struct stat fstat;
	char *buf;

	if (File_Stat(filename, &fstat))
	{
		file = File_Open(filename, "rb");
		if (!file)
		{
			Console_Printf("Couldn't open %s\n", filename);
			return NULL;
		}
		buf = Tag_Malloc(fstat.st_size, memtag);
		File_Read(buf, fstat.st_size, 1, file);
		File_Close(file);
		
		bFile = File_NewBufferFile();
		if (!bFile)
		{
			Console_Printf("Couldn't create a new buffered file\n");
			return NULL;
		}
		//file->filename = Tag_Strdup(filename, MEM_FILE);
		File_Buffered_Import(bFile, buf, fstat.st_size);
		File_FileTrackingRename(bFile, filename);
		return bFile;
	}
	return NULL;
}

bool	File_ReplaceBuffer(file_t *file, char *newbuf)
{
	bufferedFile_t *bFile = file->file;
	bFile->buf = newbuf;
	return true;
}

int		File_GetBufferSize(file_t *file)
{
	bufferedFile_t *bFile = file->file;
	return bFile->length;	
}

int		File_GetBuffer(file_t *file, char **buf)
{
	bufferedFile_t *bFile = file->file;
	*buf = bFile->buf;
	return bFile->length;	
}

bool	File_IsBuffered(file_t *file)
{
	return file->buffered;
}

void	File_Raw_Flush(file_t *file)
{
	fflush(file->file);
}

void	File_Flush(file_t *file)
{
	if (file && file->flush)
		file->flush(file);
}

int		File_Raw_Seek(file_t *file, long offset, int whence)
{
	return fseek(file->file, offset, whence);
}

int		File_Seek(file_t *file, long offset, int whence)
{
	if (file && file->seek)
		return file->seek(file, offset, whence);
	return -1;
}

long	File_Raw_Tell(file_t *file)
{
	return ftell(file->file);
}

long	File_Tell(file_t *file)
{
	if (file && file->tell)
		return file->tell(file);
	return -1;
}

size_t File_Raw_Read(void *ptr, size_t size, size_t nmemb, file_t *file)
{
	if (!file->file)
		return 0;
	return fread(ptr, size, nmemb, file->file);
}

size_t File_Read(void *ptr, size_t size, size_t nmemb, file_t *file)
{
	return file->read(ptr, size, nmemb, file);
}

int File_ReadInt(file_t *file)
{
	int ret;

	file->read(&ret, sizeof(int), 1, file);

	return LittleInt(ret);
}

float File_ReadFloat(file_t *file)
{
	float ret;

	file->read(&ret, sizeof(float), 1, file);
	
	return LittleFloat(ret);
}

short File_ReadShort(file_t *file)
{
	short ret;

	file->read(&ret, sizeof(short), 1, file);

	return LittleShort(ret);
}

byte File_ReadByte(file_t *file)
{
	byte ret;

	file->read(&ret, 1, 1, file);

	return ret;
}


char	File_getc(file_t *file)
{
	char c = 0;
	file->read(&c, 1, 1, file);
	return c;
}

size_t File_Raw_Write (const void *ptr, size_t size, size_t nmemb, file_t *file)
{
	if (!file->file)
		return 0;
	return fwrite(ptr, size, nmemb, file->file);
}

size_t File_Write (const void *ptr, size_t size, size_t nmemb, file_t *file)
{
	return file->write(ptr, size, nmemb, file);
}

size_t File_WriteInt(file_t *file, int i)
{
	int li = LittleInt(i);

	return file->write(&li, sizeof(int), 1, file);	
}

size_t File_WriteFloat(file_t *file, float f)
{
	float lf = LittleFloat(f);

	return file->write(&lf, sizeof(float), 1, file);	
}

size_t File_WriteShort(file_t *file, short s)
{
	short ls = LittleShort(s);

	return file->write(&ls, sizeof(short), 1, file);	
}

size_t File_WriteByte(file_t *file, byte b)
{
	return file->write(&b, 1, 1, file);	
}


int		File_Raw_Close(file_t *f)
{
	int ret;
	OVERHEAD_INIT;
	ret = fclose(f->file);
	f->file = NULL;
	OVERHEAD_COUNT(OVERHEAD_IO);
	return ret;
}

int		File_Raw_Eof(file_t *f)
{
	return feof((FILE *)f->file);
}

char *File_gets(char *s, int size, file_t *f)
{
	int i = 0;
	
	while (i < size - 1)
	{
		s[i] = File_getc(f);
		if (!s[i] || s[i] == '\n')
		{
			//we're done
			s[i] = 0;
			break;
		}
		//only increment i if we have a valid character
		if (s[i] != '\r')
			i++;
	}
	if (i == 0)
	{
		s[0] = 0;
		return NULL;
	}
	
	if (i+1 < size)
		s[i+1] = 0;
	return s;
}

/*******************
 *
 * File_OpenAbsolute
 *
 * This is really only used directly by the Linux port, which
 * has to "pierce" the game install dir and write files to the user's
 * home directory.
 *
 * It is used by File_Open though.
 *
 ******************/

file_t 	*File_OpenAbsolute(const char *filename, const char *mode)
{
	FILE *f;
	file_t *filestruct = NULL;

	f = fopen(filename, mode);
	
	if (strlen(filename) == 0 || filename[strlen(filename)-1] == '/')
	{
		Console_DPrintf("weird filename %s, mode %s\n", filename, mode);
	}
	
	if (f)
	{
		filestruct = Tag_Malloc(sizeof(file_t), MEM_FILE);
		memset(filestruct, 0, sizeof(file_t));
		filestruct->file = f;
		filestruct->data = NULL;
		filestruct->read = File_Raw_Read;
		filestruct->write = File_Raw_Write;
		filestruct->close = File_Raw_Close;
		filestruct->eof = File_Raw_Eof;
		filestruct->flush = File_Raw_Flush;
		filestruct->seek = File_Raw_Seek;
		filestruct->tell = File_Raw_Tell;
		filestruct->buffered = false;

		if (filenamelist_file)
		{
			fprintf(filenamelist_file, "%s\n", filename);
		}

		File_AddOpenFileTracking(filename, filestruct);
	}


#ifdef SHOW_MISSING_FILES
	if (!filestruct)
	{
		File_Printf(MISSINGFILES, "%s\n", filename);
	}
#endif
	return filestruct;

}

file_t	*File_Open(const char *filename, const char *mode)
{
	char full_system_path[1024];
	bool isWriting = false;
	file_t *f = NULL;
	OVERHEAD_INIT;

	if (!*filename)
		return NULL;

	File_FixPath(filename, full_system_path, true);

	//if we are writing, check to see if the file is in the script buffer and knock it out
	if (strchr(mode, 'w'))
	{
		int index;

		for (index = 0; index < SCRIPT_BUFFER_SIZE; index++)
		{
			if (!stricmp(full_system_path, scriptBuffer[index].scriptName))
			{
				Tag_Free(scriptBuffer[index].buffer);
				scriptBuffer[index].buffer = NULL;
				scriptBuffer[index].lastRun = 0;
				scriptBuffer[index].length = 0;
				scriptBuffer[index].scriptName[0] = 0;
			}
		}
		isWriting = true;
	}
	else
	{
		if (strncmp(filename, "http:", 5) == 0)
		{
			f = HTTP_OpenFile((char *)filename);
			if (f)
			{
				OVERHEAD_COUNT(OVERHEAD_IO);
				return f;
			}
		}

		//if archives override files on the hard drive, load them here
		if (!f && archive_precedence.integer == 1)
		{
			f = Archives_OpenFile(filename, mode);
			if (f)
				return f;
		}
	}

	f = File_OpenAbsolute(full_system_path, mode);
	if (!f)
	{
		//check the default game path
		//fixme: if we are the game module, we're gonna be checking the same path twice	
		char temp[1024];
		char systempath[1024];
		char engine_path[1024];

		File_FixPath(filename, engine_path, false);
		
		if (DLLTYPE == DLLTYPE_EDITOR || DLLTYPE == DLLTYPE_NOTLOADED)
			BPrintf(temp, 1024, "%s%s", GAME_PATH, engine_path);
		else
			BPrintf(temp, 1024, "%s", engine_path);
			
		File_FixPath(temp, systempath, true);

		f = File_OpenAbsolute(systempath, mode);
	}

#ifndef _WIN32
	//for unixes, if we can't find it or can't open it for writing, check their home dir in .savage to read/write it there
	if (!f)
	{
		f = File_OpenAbsolute(fmt("%s%s", homedir.string, Filename_GetFilename((char *)filename)), mode);
	}
#endif
	
	//if we want to load archives second, try them here
	if (!f && archive_precedence.integer != 1)
	{
		f = Archives_OpenFile(filename, mode);
	}

	OVERHEAD_COUNT(OVERHEAD_IO);
	return f;
}

#ifdef unix
bool    File_StatAbsolute(const char *filename, struct stat *stats)
{
	return (stat(filename, stats) == 0);
}
#endif

bool    File_Stat(const char *filename, struct stat *stats)
{
	char full_system_path[1024];	
	bool ret = false;

	OVERHEAD_INIT;
	
	File_FixPath(filename, full_system_path, true);
		
	if (archive_precedence.integer == 1)
	{
		ret = Archive_StatFile(filename, stats);
	}
	
	if (!ret)
	{
		ret = (stat(full_system_path, stats) == 0);

		if (!ret)
		{
			//check the default game path
			//fixme: if we are the game module, we're gonna be checking the same path twice	
			char temp[1024];
			char systempath[1024];
			char engine_path[1024];
	
			File_FixPath(filename, engine_path, false);
	
			if (DLLTYPE == DLLTYPE_EDITOR || DLLTYPE == DLLTYPE_NOTLOADED)
				BPrintf(temp, 1024, "%s%s", GAME_PATH, engine_path);
			else
				BPrintf(temp, 1024, "%s", engine_path);
			File_FixPath(temp, systempath, true);
	
			ret = (stat(systempath, stats) == 0);		
		}
	}

	if (!ret && archive_precedence.integer != 1)
	{
		ret = Archive_StatFile(filename, stats);
	}
	
	OVERHEAD_COUNT(OVERHEAD_IO);

#ifdef SHOW_MISSING_FILES
	if (ret == 0)
	{
		File_Printf(MISSINGFILES, "%s\n", filename);
	}
#endif
	return ret;
}

bool	File_Delete(const char *filename)
{
	if (remove(filename) == 0)
		return true;

	return false;
}

void	File_Printf(file_t *f, const char *fmt, ...)
{
	char s[4096];
	va_list argptr;
	OVERHEAD_INIT;

	if (!f)
		return;

	va_start(argptr, fmt);
	vsprintf(s, fmt, argptr);
	va_end(argptr);

	f->write(s, strlen(s), 1, f);
	OVERHEAD_COUNT(OVERHEAD_IO);
}

//current_dir always begins with a / and ends with a /.  the first / denotes the root directory of the game.
void	File_ChangeDir(const char *dir)
{
	char tmpdir[1024];
	OVERHEAD_INIT;

	if (!dir[0])
		return;

	if (dir[0] == '/')		//change directory relative to root
	{
		strcpy(tmpdir, dir);
	}
	else					//change directory relative to current directory
	{
		strcpy(tmpdir, fmt("%s%s", current_dir, dir));
	}

	//make sure a / is on the end
	if (tmpdir[strlen(tmpdir)-1] != '/')
		strcat(tmpdir, "/");
	if (tmpdir[0] != '/')
		strcpy(tmpdir, fmt("/%s", tmpdir));

	File_ProcessPathHacks(tmpdir);
	if (stricmp(current_dir, tmpdir) == 0)
		return;

	//Console_DPrintf("Trying to chdir to '%s%s'\n", System_GetRootDir(), &tmpdir[1]);
	if (chdir(fmt("%s%s", System_GetRootDir(), &tmpdir[1]))==0)
	{
		Console_DPrintf("changed to directory %s\n", tmpdir);
	}
	else
	{
		Console_DPrintf("Invalid directory %s - might exist in archive though\n", tmpdir);
	}
	strcpy(current_dir, tmpdir);

	//Console_Printf("current dir: %s\n", current_dir);
	OVERHEAD_COUNT(OVERHEAD_IO);
}

/*
void	File_ChangeDir(const char *dir)
{
	if (!dir[0])
		return;

	if (dir[0] == '/')		//change the directory relative to the root
	{
		if (!dir[1])
		{
			current_dir[0] = 0;
		}
		else
		{
			strncpy(current_dir, dir, 1022);
			current_dir[1022] = 0;
		}
	}
	else					//change the directory relative to the current directory
	{
		strncat(current_dir, dir, 1022);
		current_dir[1022]=0;
	}

	chdir(fmt("%s%s", System_GetRootDir(), current_dir));

	if (current_dir[strlen(current_dir)-1] != '/')
		strcat(current_dir, "/");
}*/

char	*File_GetCurrentDir()
{
	return current_dir;
}

void	File_ResetDir()
{
	File_ChangeDir("/");
}

void	File_Close(file_t *f)
{
	if (!f)
	{
		Console_Printf("trying to close a NULL file\n");
		return;
	}
	File_DelOpenFileTracking(f);
	f->close(f);
	//if (f->filename)
	//	Tag_Free(f->filename);
	Tag_Free(f);
}

void	File_SeekToNewLine(file_t *f)
{
	int c;
	OVERHEAD_INIT;

	while (feof((FILE *)f->file)==0)
	{
		c = fgetc(f->file);
		if (c=='\n')
		{
			OVERHEAD_COUNT(OVERHEAD_IO);
			return;
		}
	}
}

bool	File_Exists(const char *filename)
{
	STRUCT_STAT tmp;
	bool ret = false;

	/*
	 * this kind of path makes mods hell, but the warning isn't useful until we worry about mods
	if (strstr(filename, "../game"))
		printf("damn path...\n");
	*/
	
	if (archive_precedence.integer == 1)
	{
		ret = Archives_FileExists(filename);
	}

	if (!ret)
		ret = File_Stat(filename, &tmp);

	if (!ret && archive_precedence.integer != 1)
	{
		ret = Archives_FileExists(filename);
	}

	return ret;
}

#ifdef unix
int		File_SizeAbsolute(const char *filename)
{
	STRUCT_STAT fstats;
	OVERHEAD_INIT;
	
	if (!filename[0])
		return 0;

	if (File_StatAbsolute(filename, &fstats))
		return fstats.st_size;
	else
		return 0;

	OVERHEAD_COUNT(OVERHEAD_IO);
}
#endif

int		File_Size(const char *filename)
{
	STRUCT_STAT fstats;
	OVERHEAD_INIT;
	
	if (!filename[0])
		return 0;

	if (File_Stat(filename, &fstats))
		return fstats.st_size;
	else
		return 0;

	OVERHEAD_COUNT(OVERHEAD_IO);
}

char *File_GetNextFileIncrement(int num_digits, const char *basename, const char *ext, char *filename, int size)
{
	//const int int_size = 5;
	int i,limit;
	char fname[1024];
	const char *format;

	if (num_digits > 6)
		num_digits = 6;

	switch(num_digits)
	{
		case 1:
			format = "%s%01d.%s";
			limit = 9;
			break;
		case 2:
			format = "%s%02d.%s";
			limit = 99;
			break;
		case 3:
			format = "%s%03d.%s";
			limit = 999;
			break;
		case 4:
			format = "%s%04d.%s";
			limit = 9999;
			break;
		case 5:
			format = "%s%05d.%s";
			limit = 99999;
			break;
		case 6:
			format = "%s%06d.%s";
			limit = 999999;
			break;
		default:
			format = "%s%04d.%s";
			limit = 9999;
			break;
	}
	
	i = 0;
	
	do
	{		
		BPrintf(fname, sizeof(fname), format, basename, i, ext);
		i++;
	} while (File_Exists(fname) && i <= limit);

	strncpySafe(filename, fname, size);

	return filename;
}

#ifdef unix
//big buffers might take a bit of time to read in,
//in which case we might want to provide a function which reads
//in the file incrementally and executes a callback to display
//progress
void	*File_LoadIntoBufferAbsolute(const char *filename, int *len, int tag)
{
	int length;
	char *mem;
	bool buffered = false;
	file_t *f = File_OpenAbsolute(filename, "rb");

	*len = 0;

	if (!f)
		return NULL;

	if (File_IsBuffered(f))
	{
		buffered = true;
		length = File_GetBuffer(f, &mem);
	}
	else
		length = File_SizeAbsolute(filename);

	if (buffered)
	{
		File_ReplaceBuffer(f, NULL); //it's ours now, don't free it when you Close, etc.
	}
	else
	{
		mem = Tag_Malloc(length, tag ? tag : MEM_FILESYSTEM);

		if (!f->read(mem, 1, length, f))
		{
			Tag_Free(mem);
			File_Close(f);
			return NULL;
		}
	}

	File_Close(f);

	*len = length;

	return mem;
}
#endif

//big buffers might take a bit of time to read in,
//in which case we might want to provide a function which reads
//in the file incrementally and executes a callback to display
//progress
void	*File_LoadIntoBuffer(const char *filename, int *len, int tag)
{
	int length;
	char *mem;
	bool buffered = false;
	file_t *f = File_Open(filename, "rb");

	*len = 0;

	if (!f)
		return NULL;

	if (File_IsBuffered(f))
	{
		buffered = true;
		length = File_GetBuffer(f, &mem);
	}
	else
		length = File_Size(filename);

	if (buffered)
	{
		File_ReplaceBuffer(f, NULL); //it's ours now, don't free it when you Close, etc.
	}
	else
	{
		mem = Tag_Malloc(length, tag ? tag : MEM_FILESYSTEM);

		if (!f->read(mem, 1, length, f))
		{
			Tag_Free(mem);
			File_Close(f);
			return NULL;
		}
	}

	File_Close(f);

	*len = length;

	return mem;
}


void	File_FreeBuffer(void *mem)
{
	Tag_Free(mem);
}



/*==========================

  File_AllocBlockList

  allocate a new blocklist structure and fill it in

  the 'filename' field is ONLY used for outputting debugging info
  the file should have been read in and put into 'buf' prior to
  File_AllocBlockList

 ==========================*/

blockList_t *File_AllocBlockList(const void *buf, int buflen, const char *filename_debug)
{
	blockList_t *blocklist;
	int n = 0;
	int curpos = 0;
	char *cbuf = (char *)buf;

	blocklist = Tag_Malloc(sizeof(blockList_t), MEM_FILESYSTEM);
	//allocate blocks in increments of 64
	blocklist->blocks = Tag_Malloc(sizeof(block_t) * 64, MEM_FILESYSTEM);
	blocklist->_num_allocated = 64;	

	while (curpos+8 < buflen)
	{
		int length;

		blocklist->blocks[n].name[0] = cbuf[curpos++];
		blocklist->blocks[n].name[1] = cbuf[curpos++];
		blocklist->blocks[n].name[2] = cbuf[curpos++];
		blocklist->blocks[n].name[3] = cbuf[curpos++];
		blocklist->blocks[n].name[4] = '\0';
		
		length = blocklist->blocks[n].length = LittleInt(*(int *)(&cbuf[curpos]));
		if (curpos + length > buflen)
		{
			System_Error("File_BufParseBlocks: Invalid block\n");
			Tag_Free(blocklist->blocks);
			Tag_Free(blocklist);
			return NULL;
		}
		
		curpos+=4;
		blocklist->blocks[n].pos = curpos;
		blocklist->blocks[n].data = &cbuf[curpos];
		curpos+=length;

		n++;
		if (n >= blocklist->_num_allocated)
		{
			//allocate some more blocks
			blocklist->blocks = Tag_Realloc(blocklist->blocks, sizeof(block_t) * (blocklist->_num_allocated + 64), MEM_FILESYSTEM);
			blocklist->_num_allocated += 64;
		}		
	}

	if (curpos != buflen)
	{
		Console_DPrintf(fmt("File_AllocBlockList: bad filesize in %s\n", filename_debug));
		Tag_Free(blocklist->blocks);
		Tag_Free(blocklist);
		return NULL;
	}

	blocklist->num_blocks = n;

	return blocklist;
}

void	File_FreeBlockList(blockList_t *blocklist)
{
	if (!blocklist)
		return;

	Tag_Free(blocklist->blocks);
	Tag_Free(blocklist);
}

void	File_Shutdown()
{
	Tag_FreeAll(MEM_FILESYSTEM);
}
