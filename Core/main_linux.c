#include "core.h"

#ifndef _WIN32
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <glob.h>

#include <dlfcn.h>

#endif //not _WIN32

#ifdef STATIC_BUILD
#include "../TheGame/dllentry.h"
#endif

char dll_hash[MAX_HASH_SIZE];
int dll_hashlen;

extern char rootdir[_MAX_PATH];
extern cvar_t sys_enumdir;
extern cvar_t speak;
extern cvar_t voice;
extern FILE *festival;

#ifndef STATIC_BUILD
void *gameDLL;
#endif

#ifdef __APPLE__
#define LIBRARY_EXTENSION ".dylib"
#else
#define LIBRARY_EXTENSION ".so"
#endif

void System_UnloadGameDLL()
{
#ifndef STATIC_BUILD
	if (gameDLL)
		dlclose(&gameDLL);
#endif //not STATIC_BUILD
}

typedef int (*_initfunc_t)(coreAPI_shared_t *core_api_shared);
typedef void (*_cl_initapis_t)(coreAPI_client_t *core_api, clientAPI_t *client_api);
typedef void (*_shutdownfunc_t)(void);
typedef void (*_sv_initapis_t)(coreAPI_server_t *core_api, serverAPI_t *server_api);
typedef void (*_int_initapis_t)(coreAPI_interface_t *interface_api, interfaceAPI_t *int_api);

_shutdownfunc_t _ShutdownGameDLL;

void System_InitGameDLL()
{
	char *s;
	coreAPI_client_t coreAPI_c;
	coreAPI_server_t coreAPI_s;
	coreAPI_shared_t coreAPI_shared;
	coreAPI_interface_t coreAPI_i;

	_initfunc_t _InitGameDLL;
	_cl_initapis_t _CL_InitAPIs;
	_sv_initapis_t _SV_InitAPIs;
	_int_initapis_t _INT_InitAPIs;
	
#ifdef __APPLE__
	Console_Printf("%s%s", _("Savage on OS X - Core build "), __DATE__ " " __TIME__ "\n");
#else
	Console_Printf("%s%s", _("Savage on Linux - Core build "),  __DATE__ " " __TIME__ "\n");
#endif
	
#ifndef STATIC_BUILD
	// load game dll
#ifdef SAVAGE_DEMO
	s = Tag_Strdup(fmt("%sgame_demo%s", rootdir, LIBRARY_EXTENSION), MEM_SYSTEM);
#else
	s = Tag_Strdup(fmt("%sgame%s", rootdir, LIBRARY_EXTENSION), MEM_SYSTEM);
#endif

	gameDLL = dlopen(s, RTLD_LAZY); //was RTLD_NOW for debugging
	if (!gameDLL)
		System_Error("Couldn't load %s - %s\n", s, dlerror());

	//hash the file once it's already opened, to make a file switch harder
	dll_hashlen = Hash_FilenameAbsolute(s, dll_hash);
	
	Tag_Free(s);
#endif

#ifndef STATIC_BUILD

	//standard dlsym unix method -- there is a #define for OS X

	//find entry function InitGameDLL (this is common to server and client game code)
	_InitGameDLL = (_initfunc_t)dlsym(gameDLL, "InitGameDLL");

#else //STATIC_BUILD

	_InitGameDLL = InitGameDLL;

#endif //STATIC_BUILD

	if (!_InitGameDLL)
		System_Error("Couldn't find entry function InitGameDLL()\n");

	memset(&coreAPI_shared, 0, sizeof(coreAPI_shared));
	memset(&coreAPI_c, 0, sizeof(coreAPI_c));
	memset(&coreAPI_s, 0, sizeof(coreAPI_s));
	memset(&coreAPI_i, 0, sizeof(coreAPI_i));
	
	//fill in the coreAPI structures from host_getcoreapi.c
	Host_GetCoreAPI(&coreAPI_shared, &coreAPI_c, &coreAPI_s, &coreAPI_i);
	
	//call InitGameDLL to get the DLL type
	DLLTYPE = _InitGameDLL(&coreAPI_shared);
	if (DLLTYPE != DLLTYPE_GAME && DLLTYPE != DLLTYPE_EDITOR)
	{
		System_Error("Unrecognized DLLTYPE\n");
	}

#ifndef STATIC_BUILD
	//find CL_InitAPIs to get function pointers to the client game functions
	_CL_InitAPIs = (_cl_initapis_t)dlsym(gameDLL, "CL_InitAPIs");
#else
	_CL_InitAPIs = CL_InitAPIs;
#endif

	if (!_CL_InitAPIs)
		System_Error("Couldn't find entry function CL_InitAPIs()\n");

	//call CL_InitAPIs to get function pointers to the client game functions
	_CL_InitAPIs(&coreAPI_c, &cl_api);

	//if this a game dll (as opposed to editor dll), we need to get at the server code, too
	if (DLLTYPE == DLLTYPE_GAME)
	{
#ifndef STATIC_BUILD
		_SV_InitAPIs = (_sv_initapis_t)dlsym(gameDLL, "SV_InitAPIs");
#else
		_SV_InitAPIs = SV_InitAPIs;
#endif

		if (!_SV_InitAPIs)
			System_Error("Coudln't find entry function SV_InitAPIs()\n");

		_SV_InitAPIs(&coreAPI_s, &sv_api);
	}
		
#ifndef STATIC_BUILD
	//find CL_InitAPIs to get function pointers to the client game functions
	_INT_InitAPIs = (_int_initapis_t)dlsym(gameDLL, "INT_InitAPIs");
#else
	_INT_InitAPIs = INT_InitAPIs;
#endif
	
	if (!_INT_InitAPIs)
		System_Error("Couldn't find entry function INT_InitAPIs()\n");

	//call INT_InitAPIs to get function pointers to the interface functions
	_INT_InitAPIs(&coreAPI_i, &int_api);

#ifndef STATIC_BUILD
	//find ShutdownGameDLL function
	_ShutdownGameDLL = (_shutdownfunc_t)dlsym(gameDLL, "ShutdownGameDLL");
#else
	_ShutdownGameDLL = ShutdownGameDLL;
#endif
	if (!_ShutdownGameDLL)
		System_Error("Couldn't find entry function ShutdownGameDLL()\n");

	//make sure the game code has filled in all its functions
	Host_TestGameAPIValidity(&cl_api, &sv_api, &int_api);

	if (speak.integer)
	{
		festival = popen("festival", "w");
		signal(SIGPIPE, SIG_IGN);
		if (festival)
		{
			fprintf(festival, voice.string);
		}
	}
}

void	System_ChangeRootDir(const char *newdir)
{
	char *s, dir[_MAX_PATH];
	//set the root directory to the mod's directory
	getcwd(dir, _MAX_PATH-1);
	strcpy(rootdir, fmt("%s/%s/", dir, newdir));

	s = rootdir;

	while (*s)
	{
		if (*s == '\\')
			*s = '/';
		s++;
	}
	
	chdir(rootdir);

}

void	System_Dir(char *directory, char *wildcard, bool recurse,
				   void(*dirCallback)(const char *dir, void *userdata),
				   void(*fileCallback)(const char *filename, void *userdata),
				   void *userdata)
{
	char				searchstring[1024];
	char				*slash;
	char				dirname[1024];	
	char 				wcard[256];
	glob_t				globbuf;
	struct stat			file_info;				
	int n;

	//BPrintf(dirname, 256, "%s%s", System_GetRootDir(), File_GetFullPath(directory));
	strncpy(dirname, File_GetFullPath(directory), 1023);

	if (!dirname || strlen(dirname) == 0)
		strcpy(dirname, ".");
	
	slash = &dirname[strlen(dirname)-1];
	if (*slash=='/' || *slash=='\\')
		*slash = 0;

	if (!*wildcard)
		strcpy(wcard, "*");
	else
		strcpy(wcard, wildcard);

	File_SystemDir(dirname, wcard, recurse, dirCallback, fileCallback, userdata);

	//first search for files only

	if (fileCallback)
	{
		BPrintf(searchstring, 1023, "%s%s/%s", System_GetRootDir(), dirname, wildcard);
		
		Cvar_SetVar(&sys_enumdir, directory);

		if (glob(searchstring, 0, NULL, &globbuf) == 0)
		{
		  	n = globbuf.gl_pathc;
			while(n--) 
			{
			  	stat(globbuf.gl_pathv[n], &file_info);
				if (!S_ISDIR(file_info.st_mode))
				{
					fileCallback(Filename_GetFilename(globbuf.gl_pathv[n]), userdata);		
				}
			}
		}
		globfree(&globbuf);
	}

	//next search for subdirectories only

	BPrintf(searchstring, 1023, "%s%s/*", System_GetRootDir(), dirname);
	searchstring[1023] = 0;

	if (glob(searchstring, 0, NULL, &globbuf) == 0)
	{
	  	n = globbuf.gl_pathc;
		while(n--) 
		{
		  	stat(globbuf.gl_pathv[n], &file_info);
			if (S_ISDIR(file_info.st_mode))
			{			
	 			if (strcmp(globbuf.gl_pathv[n], ".") && strcmp(globbuf.gl_pathv[n], "..") && strcmp(globbuf.gl_pathv[n], "CVS"))
				{
					Cvar_SetVar(&sys_enumdir, &globbuf.gl_pathv[n][strlen(System_GetRootDir())]);
					
					if (dirCallback)
						dirCallback(&globbuf.gl_pathv[n][strlen(System_GetRootDir())], userdata);

					if (recurse)
					{
						System_Dir(&globbuf.gl_pathv[n][strlen(System_GetRootDir())], wcard, true, dirCallback, fileCallback, userdata);
					}			
				}
			}
		}
	}
	globfree(&globbuf);

	Cvar_SetVar(&sys_enumdir, "");
}

bool	System_CreateDir(const char *dirname)
{
	if (dirname) 
		if (mkdir(fmt("%s%s", System_GetRootDir(), dirname), S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH)==0)
			return true;
	return false;
}
