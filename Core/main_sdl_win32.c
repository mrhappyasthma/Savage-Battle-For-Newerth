
#include "core.h"

extern char rootdir[_MAX_PATH];
extern cvar_t sys_enumdir;


typedef int (*_initfunc_t)(coreAPI_shared_t *core_api_shared);
typedef void (*_cl_initapis_t)(coreAPI_client_t *core_api, clientAPI_t *client_api);
typedef void (*_sv_initapis_t)(coreAPI_server_t *core_api, serverAPI_t *server_api);
typedef void (*_int_initapis_t)(coreAPI_interface_t *interface_api, interfaceAPI_t *int_api);

HINSTANCE gameDLL;

void System_InitGameDLL()
{
	char *s;
	coreAPI_client_t coreAPI_c;
	coreAPI_server_t coreAPI_s;
	coreAPI_shared_t coreAPI_shared;
	coreAPI_interface_t coreAPI_i;
	char dir[_MAX_PATH];

	_initfunc_t InitGameDLL;
	_cl_initapis_t CL_InitAPIs;
	_sv_initapis_t SV_InitAPIs;
	_int_initapis_t INT_InitAPIs;

	// load game dll

	gameDLL = LoadLibrary(fmt("%s/game.dll", mod.string));
	if (!gameDLL)
		System_Error("Couldn't load game.dll\n");

	//set the root directory to the mod's directory
	getcwd(dir, _MAX_PATH-1);
	strcpy(rootdir, fmt("%s/%s/", dir, mod.string));

	s = rootdir;

	while (*s)
	{
		if (*s == '\\')
			*s = '/';
		s++;
	}

	chdir(rootdir);

	//find entry function InitGameDLL (this is common to server and client game code)

	InitGameDLL = (_initfunc_t)GetProcAddress(gameDLL, "InitGameDLL");
	if (!InitGameDLL)
		System_Error("Couldn't find entry function InitGameDLL()\n");

	//call InitGameDLL to get the DLL type
	DLLTYPE = InitGameDLL(&coreAPI_shared);
	if (DLLTYPE != DLLTYPE_GAME && DLLTYPE != DLLTYPE_EDITOR)
	{
		System_Error("Unrecognized DLLTYPE\n");
	}

	memset(&coreAPI_shared, 0, sizeof(coreAPI_shared));
	memset(&coreAPI_c, 0, sizeof(coreAPI_c));
	memset(&coreAPI_s, 0, sizeof(coreAPI_s));
	memset(&coreAPI_i, 0, sizeof(coreAPI_i));
	
	//fill in the coreAPI structures from host_getcoreapi.c
	Host_GetCoreAPI(&coreAPI_shared, &coreAPI_c, &coreAPI_s, &coreAPI_i);
	
	//find and call CL_InitAPIs to get function pointers to the client game functions
	CL_InitAPIs = (_cl_initapis_t)GetProcAddress(gameDLL, "CL_InitAPIs");
	if (!CL_InitAPIs)
		System_Error("Couldn't find entry function CL_InitAPIs()\n");
	CL_InitAPIs(&coreAPI_c, &cl_api);

	//if this a game dll (as opposed to editor dll), we need to get at the server code, too
	if (DLLTYPE == DLLTYPE_GAME)
	{
		SV_InitAPIs = (_sv_initapis_t)GetProcAddress(gameDLL, "SV_InitAPIs");
		if (!SV_InitAPIs)
			System_Error("Coudln't find entry function SV_InitAPIs()\n");

		SV_InitAPIs(&coreAPI_s, &sv_api);

		//find CL_InitAPIs to get function pointers to the client game functions
		INT_InitAPIs = (_int_initapis_t)dlsym(gameDLL, "INT_InitAPIs");
		if (!INT_InitAPIs)
			System_Error("Couldn't find entry function INT_InitAPIs()\n");
		
		//call INT_InitAPIs to get function pointers to the interface functions
		INT_InitAPIs(&coreAPI_i, &int_api);
	}

	//make sure the game code has filled in all its functions
	Host_TestGameAPIValidity(&cl_api, &sv_api, &int_api);
}

void System_UnloadGameDLL()
{
	if (gameDLL)
		FreeLibrary(gameDLL);
}

void	System_Dir(char *directory, char *wildcard, bool recurse,
				   void(*dirCallback)(const char *dir, void *userdata),
				   void(*fileCallback)(const char *filename, void *userdata),
				   void *userdata)
{
	WIN32_FIND_DATA		finddata;
	HANDLE				handle;
	char				searchstring[1024];
	char				*slash;
	char				dirname[1024];	
	char				wcard[256];

	strncpy(dirname, File_GetFullPath(directory), 1023);

	slash = &dirname[strlen(dirname)-1];
	if (*slash=='/' || *slash=='\\')
		*slash = 0;

	if (!*wildcard)
		strcpy(wcard, "*");
	else
		strcpy(wcard, wildcard);

	//first search for files only

	if (fileCallback)
	{
		BPrintf(searchstring, 1023, "%s%s/%s", System_GetRootDir(), dirname, wildcard);
		
		Cvar_SetVar(&sys_enumdir, dirname);

		handle = FindFirstFile(searchstring, &finddata);

		if (handle != INVALID_HANDLE_VALUE)
		{
			do
			{
				if (!(finddata.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) &&
					!(finddata.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) &&
					!(finddata.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY) &&
					!(finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				{			
					fileCallback(finddata.cFileName, userdata);		
				}
			} while (FindNextFile(handle, &finddata));
		}

		FindClose(handle);
	}

	//next search for subdirectories only

	BPrintf(searchstring, 1023, "%s%s/*", System_GetRootDir(), dirname);
	searchstring[1023] = 0;

	handle = FindFirstFile(searchstring, &finddata);

	if (handle == INVALID_HANDLE_VALUE)
		return;

	do
	{
		if (!(finddata.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) &&
			!(finddata.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) &&
			!(finddata.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY))
		{
			if (finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{					
	 			if (strcmp(finddata.cFileName, ".") && strcmp(finddata.cFileName, ".."))
				{
					char dname[256];
					BPrintf(dname, 255, "%s/%s", dirname, finddata.cFileName);

					Cvar_SetVar(&sys_enumdir, dirname);

					if (dirCallback)
						dirCallback(dname, userdata);

					if (recurse)
					{
						System_Dir(dname, wcard, true, dirCallback, fileCallback, userdata);
					}			
				}
			}
		}
	} while (FindNextFile(handle, &finddata));

	FindClose(handle);

	Cvar_SetVar(&sys_enumdir, "");
}

bool	System_CreateDir(const char *dirname)
{
	if (!dirname)
		return false;

		// && strcmp(dirname, "") != 0 //probably not necessary
	if (_mkdir(fmt("%s%s", System_GetRootDir(), File_GetFullPath(dirname)))==0)
		return true;
	else
		return false;
}
