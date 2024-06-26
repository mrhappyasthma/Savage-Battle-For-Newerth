// (C) 2003 S2 Games

// main_win32.c

//don't even attempt to compile if this is not win32
#ifdef _WIN32

#define	SYS_MAX_ARGV	32

#include "core.h"
#include "resource.h"
#include <mmsystem.h>

#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL                   0x020A
#endif
#ifndef WM_XBUTTONDOWN
#define WM_XBUTTONDOWN					0x020B
#endif
#ifndef WM_XBUTTONUP
#define WM_XBUTTONUP					0x020C
#endif
#ifndef MK_XBUTTON1
#define MK_XBUTTON1						0x0020
#endif
#ifndef MK_XBUTTON2
#define MK_XBUTTON2						0x0040
#endif

int sys_startTime = 0;
int sys_time = 0;

int screensaver;
HINSTANCE win32_hinstance;
HWND hConWindow = 0;
char	rootdir[_MAX_PATH];
char	originaldir[_MAX_PATH];
static HANDLE hInput;

extern cvar_t sys_enumdir;
extern cvar_t sys_sleep;
extern cvar_t sys_focus;
extern cvar_t inverty;

char engine_hash[MAX_HASH_SIZE];
int engine_hashlen;

char *sys_cmdline;

void System_Quit(void);

typedef struct
{
	byte			key1;
	byte			key2;
	const char*		key;
	byte			map;
}
KEYINIT;

byte keymap[256][2] = { 0 };

KEYINIT keyinits[] = {
/*'`','`',"`",'`',
'~','~',"~",'~',
'!','!',"!",'!',
'@','@',"@",'@',
'#','#',"#",'#',
'$','$',"$",'$',
'%','%',"%",'%',
'^','^',"^",'^',
'&','&',"&",'&',
'*','*',"*",'*',
'(','(',"(",'(',
')',')',")",')',
'-','-',"-",'-',
'_','_',"_",'_',
'=','=',"=",'=',
'+','+',"+",'+',
'[','[',"[",'[',
'{','{',"{",'{',
']',']',"]",']',
'}','}',"}",'}',
'\\','\\',"\\",'\\',
'|','|',"|",'|',
';',';',";",';',
':',':',":",':',
'\'','\'',"\'",'\'',
'\"','\"',"\"",'\"',
',',',',",",',',
'<','<',"<",'<',
'.','.',".",'.',
'>','>',">",'>',
'/','/',"/",'/',
'?','?',"?",'?',
'1','1',"1",'1',
'2','2',"2",'2',
'3','3',"3",'3',
'4','4',"4",'4',
'5','5',"5",'5',
'6','6',"6",'6',
'7','7',"7",'7',
'8','8',"8",'8',
'9','9',"9",'9',
'0','0',"0",'0',
'a','a',"a",'a',
'b','b',"b",'b',
'c','c',"c",'c',
'd','d',"d",'d',
'e','e',"e",'e',
'f','f',"f",'f',
'g','g',"g",'g',
'h','h',"h",'h',
'i','i',"i",'i',
'j','j',"j",'j',
'k','k',"k",'k',
'l','l',"l",'l',
'm','m',"m",'m',
'n','n',"n",'n',
'o','o',"o",'o',
'p','p',"p",'p',
'q','q',"q",'q',
'r','r',"r",'r',
's','s',"s",'s',
't','t',"t",'t',
'u','u',"u",'u',
'v','v',"v",'v',
'w','w',"w",'w',
'x','x',"x",'x',
'y','y',"y",'y',
'z','z',"z",'z',
'A','A',"A",'a',
'B','B',"B",'b',
'C','C',"C",'c',
'D','D',"D",'d',
'E','E',"E",'e',
'F','F',"F",'f',
'G','G',"G",'g',
'H','H',"H",'h',
'I','I',"I",'i',
'J','J',"J",'j',
'K','K',"K",'k',
'L','L',"L",'l',
'M','M',"M",'m',
'N','N',"N",'n',
'O','O',"O",'o',
'P','P',"P",'p',
'Q','Q',"Q",'q',
'R','R',"R",'r',
'S','S',"S",'s',
'T','T',"T",'t',
'U','U',"U",'u',
'V','V',"V",'v',
'W','W',"W",'w',
'X','X',"X",'x',
'Y','Y',"Y",'y',
'Z','Z',"Z",'z',
*/VK_BACK,0x08,"VK_BACK",KEY_BACKSPACE,
VK_TAB,0x09,"VK_TAB",KEY_TAB,
//VK_CLEAR,0x0C,"VK_CLEAR",KEY_CLEAR,
VK_RETURN,0x0D,"VK_RETURN",KEY_ENTER,
VK_SHIFT,0x10,"VK_SHIFT",KEY_SHIFT,
VK_CONTROL,0x11,"VK_CONTROL",KEY_CTRL,
VK_MENU,0x12,"VK_MENU",KEY_ALT,
VK_PAUSE,0x13,"VK_PAUSE",KEY_PAUSE,
VK_CAPITAL,0x14,"VK_CAPITAL",KEY_CAPSLOCK,
//VK_KANA,0x15,"VK_KANA",
//VK_HANGEUL,0x15,"VK_HANGEUL",
//VK_HANGUL,0x15,"VK_HANGUL",
//VK_JUNJA,0x17,"VK_JUNJA",
//VK_FINAL,0x18,"VK_FINAL",
//VK_HANJA,0x19,"VK_HANJA",
//VK_KANJI,0x19,"VK_KANJI",
VK_ESCAPE,0x1B,"VK_ESCAPE",KEY_ESCAPE,
//VK_CONVERT,0x1C,"VK_CONVERT",
//VK_NONCONVERT,0x1D,"VK_NONCONVERT",
//VK_ACCEPT,0x1E,"VK_ACCEPT",
//VK_MODECHANGE,0x1F,"VK_MODECHANGE",
VK_SPACE,0x20,"VK_SPACE",KEY_SPACE,
VK_PRIOR,0x21,"VK_PRIOR",KEY_PGUP,
VK_NEXT,0x22,"VK_NEXT",KEY_PGDN,
VK_END,0x23,"VK_END",KEY_END,
VK_HOME,0x24,"VK_HOME",KEY_HOME,
VK_LEFT,0x25,"VK_LEFT",KEY_LEFT,
VK_UP,0x26,"VK_UP",KEY_UP,
VK_RIGHT,0x27,"VK_RIGHT",KEY_RIGHT,
VK_DOWN,0x28,"VK_DOWN",KEY_DOWN,
//VK_SELECT,0x29,"VK_SELECT",,
//VK_PRINT,0x2A,"VK_PRINT",,
//VK_EXECUTE,0x2B,"VK_EXECUTE",,
//VK_SNAPSHOT,0x2C,"VK_SNAPSHOT",,
VK_INSERT,0x2D,"VK_INSERT",KEY_INS,
VK_DELETE,0x2E,"VK_DELETE",KEY_DEL,
//VK_HELP,0x2F,"VK_HELP",,
VK_LWIN,0x5B,"VK_LWIN",KEY_LEFT_WINDOWS,
VK_RWIN,0x5C,"VK_RWIN",KEY_RIGHT_WINDOWS,
VK_APPS,0x5D,"VK_APPS",KEY_APP,
VK_NUMPAD0,0x60,"VK_NUMPAD0",KEY_KEYPAD_0,
VK_NUMPAD1,0x61,"VK_NUMPAD1",KEY_KEYPAD_1,
VK_NUMPAD2,0x62,"VK_NUMPAD2",KEY_KEYPAD_2,
VK_NUMPAD3,0x63,"VK_NUMPAD3",KEY_KEYPAD_3,
VK_NUMPAD4,0x64,"VK_NUMPAD4",KEY_KEYPAD_4,
VK_NUMPAD5,0x65,"VK_NUMPAD5",KEY_KEYPAD_5,
VK_NUMPAD6,0x66,"VK_NUMPAD6",KEY_KEYPAD_6,
VK_NUMPAD7,0x67,"VK_NUMPAD7",KEY_KEYPAD_7,
VK_NUMPAD8,0x68,"VK_NUMPAD8",KEY_KEYPAD_8,
VK_NUMPAD9,0x69,"VK_NUMPAD9",KEY_KEYPAD_9,
VK_MULTIPLY,0x6A,"VK_MULTIPLY",KEY_KEYPAD_ASTERISK,
VK_ADD,0x6B,"VK_ADD",KEY_KEYPAD_PLUS,
VK_SEPARATOR,0x6C,"VK_SEPARATOR",',',
VK_SUBTRACT,0x6D,"VK_SUBTRACT",KEY_KEYPAD_MINUS,
VK_DECIMAL,0x6E,"VK_DECIMAL",KEY_KEYPAD_DECIMAL,
VK_DIVIDE,0x6F,"VK_DIVIDE",KEY_KEYPAD_SLASH,
VK_F1,0x70,"VK_F1",KEY_F1,
VK_F2,0x71,"VK_F2",KEY_F2,
VK_F3,0x72,"VK_F3",KEY_F3,
VK_F4,0x73,"VK_F4",KEY_F4,
VK_F5,0x74,"VK_F5",KEY_F5,
VK_F6,0x75,"VK_F6",KEY_F6,
VK_F7,0x76,"VK_F7",KEY_F7,
VK_F8,0x77,"VK_F8",KEY_F8,
VK_F9,0x78,"VK_F9",KEY_F9,
VK_F10,0x79,"VK_F10",KEY_F10,
VK_F11,0x7A,"VK_F11",KEY_F11,
VK_F12,0x7B,"VK_F12",KEY_F12,
//VK_F13,0x7C,"VK_F13",,
//VK_F14,0x7D,"VK_F14",,
//VK_F15,0x7E,"VK_F15",,
//VK_F16,0x7F,"VK_F16",,
//VK_F17,0x80,"VK_F17",,
//VK_F18,0x81,"VK_F18",,
//VK_F19,0x82,"VK_F19",,
//VK_F20,0x83,"VK_F20",,
//VK_F21,0x84,"VK_F21",,
//VK_F22,0x85,"VK_F22",,
//VK_F23,0x86,"VK_F23",,
//VK_F24,0x87,"VK_F24",,
VK_NUMLOCK,0x90,"VK_NUMLOCK",KEY_NUMLOCK,
VK_SCROLL,0x91,"VK_SCROLL",KEY_SCROLLLOCK,
};
int numkeyinits = sizeof(keyinits)/sizeof(KEYINIT);


int wmemcmp(const wchar_t *_S1, const wchar_t *_S2, size_t _N)
{
	for (; 0 < _N; ++_S1, ++_S2, --_N)
	{
		if (*_S1 != *_S2)
			return (*_S1 < *_S2 ? -1 : +1);
	}

	return (0);
}


void	System_ConsoleShutDown()
{	
	System_Printf("> quit\n");
	Cmd_Exec("quit\n");
}



BOOL CALLBACK System_ConsoleWndProc(
		HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_INITDIALOG:			
			break;
		case WM_COMMAND:
			//control messages
			switch (LOWORD(wParam))
			{
				case IDC_QUIT:

					System_ConsoleShutDown();
					break;
				case IDC_COMMAND:
				case IDC_CHAT:
					if (HIWORD(wParam) == EN_CHANGE)
					{
						char cmd[1024];

						GetWindowText(GetDlgItem(hwndDlg, LOWORD(wParam)), cmd, 1023);

						if (strlen(cmd) > 2)
						{
							if (cmd[strlen(cmd)-2] == '\r')
							{								
								cmd[strlen(cmd)-2] = 0;
								if (LOWORD(wParam) == IDC_COMMAND)
									System_Printf("> %s\n", cmd);
								Cmd_BufPrintf("%s%s", LOWORD(wParam) == IDC_CHAT ? "svchat " : "", cmd);
								SetWindowText(GetDlgItem(hwndDlg, LOWORD(wParam)), "");
							}
						}
					}
					break;
				case IDC_DEVELOPER:
					if (IsDlgButtonChecked(hwndDlg, IDC_DEVELOPER))
					{
						System_Printf("> con_developer 1\n");
						Cmd_BufPrintf("con_developer 1\n");
					}
					else
					{
						System_Printf("> con_developer 0\n");
						Cmd_BufPrintf("con_developer 0\n");
					}
					break;

				case IDC_OUTPUT:				
					if (HIWORD(wParam) == EN_MAXTEXT)
						SetWindowText((HWND)lParam, "");
					break;
				default:
					break;
			}
			break;
		case WM_CLOSE:
			System_ConsoleShutDown();
			break;		
		default:
			return FALSE;
	}
	return TRUE;
}


void System_InitConsoleWindow()
{
	if (hConWindow)
		return;

	hConWindow = CreateDialog(
		win32_hinstance,
		MAKEINTRESOURCE(IDD_CONSOLE),
		0,
		System_ConsoleWndProc);	

	ShowWindow(hConWindow, SW_SHOWNORMAL);
}

void System_InitTimer();

void System_Init()
{	
	char dir[_MAX_PATH];
	char *s;
	int key;	

	for ( key = 0 ; key < numkeyinits ; ++key )
	{
		byte map = keyinits[key].key1;
		keymap[map][0] = keyinits[key].map;
		keymap[map][1] = key;
	}

	Mem_Init();

	System_InitConsoleWindow();
	
	System_Printf("System_Init()\n");

	System_InitTimer();
	//shut down screensaver
    SystemParametersInfo(SPI_GETSCREENSAVEACTIVE, 0, &screensaver, 0);
    SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, FALSE, 0, 0);

	getcwd(dir, _MAX_PATH-1);
	getcwd(originaldir, _MAX_PATH-1);
	strcpy(rootdir, fmt("%s/", dir));

	s = rootdir;

	while (*s)
	{
		if (*s == '\\')
			*s = '/';
		s++;
	}

	chdir(rootdir);
}

typedef int (*_initfunc_t)(coreAPI_shared_t *core_api_shared);
typedef void (*_shutdownfunc_t)(void);
typedef void (*_cl_initapis_t)(coreAPI_client_t *core_api, clientAPI_t *client_api);
typedef void (*_sv_initapis_t)(coreAPI_server_t *core_api, serverAPI_t *server_api);
typedef void (*_int_initapis_t)(coreAPI_interface_t *interface_api, interfaceAPI_t *int_api);

char dll_hash[MAX_HASH_SIZE];
int dll_hashlen;

HINSTANCE gameDLL;

void System_InitGameDLL()
{
	char *filename;
	coreAPI_client_t coreAPI_c;
	coreAPI_server_t coreAPI_s;
	coreAPI_shared_t coreAPI_shared;
	coreAPI_interface_t coreAPI_i;
	//char dir[_MAX_PATH];

	_initfunc_t InitGameDLL;
	_cl_initapis_t CL_InitAPIs;
	_sv_initapis_t SV_InitAPIs;
	_int_initapis_t INT_InitAPIs;

	// load game dll

#ifdef NDEBUG

#ifdef SAVAGE_DEMO
	filename = "game_demo.dll";
#else	//SAVAGE_DEMO
	filename = "game.dll";
#endif	//SAVAGE_DEMO

#else	//NDEBUG

#ifdef SAVAGE_DEMO
	filename = "game_demo_debug.dll";
#else	//SAVAGE_DEMO
	filename = "game_debug.dll";
#endif	//SAVAGE_DEMO

#endif	//NDEBUG
	gameDLL = LoadLibrary(filename);

	if (!gameDLL)
		System_Error("Couldn't load game.dll\n");

	//hash the file once it's already opened, to make a file switch (slightly) harder
	dll_hashlen = Hash_FilenameAbsolute(filename, dll_hash);
	
	memset(&coreAPI_shared, 0, sizeof(coreAPI_shared));
	memset(&coreAPI_c, 0, sizeof(coreAPI_c));
	memset(&coreAPI_s, 0, sizeof(coreAPI_s));
	memset(&coreAPI_i, 0, sizeof(coreAPI_i));

	//fill in the coreAPI structures from host_getcoreapi.c
	Host_GetCoreAPI(&coreAPI_shared, &coreAPI_c, &coreAPI_s, &coreAPI_i);

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

	//find and call CL_InitAPIs to get function pointers to the client game functions
	CL_InitAPIs = (_cl_initapis_t)GetProcAddress(gameDLL, "CL_InitAPIs");
	if (!CL_InitAPIs)
		System_Error("Couldn't find entry function CL_InitAPIs()\n");
	CL_InitAPIs(&coreAPI_c, &cl_api);

	//find INT_InitAPIs to get function pointers to the client interface functions
	INT_InitAPIs = (_int_initapis_t)GetProcAddress(gameDLL, "INT_InitAPIs");
	if (!INT_InitAPIs)
		System_Error("Couldn't find entry function INT_InitAPIs()\n");

	//call INT_InitAPIs to get function pointers to the interface functions
	INT_InitAPIs(&coreAPI_i, &int_api);

	//if this a game dll (as opposed to editor dll), we need to get at the server code, too
	if (DLLTYPE == DLLTYPE_GAME)
	{
		SV_InitAPIs = (_sv_initapis_t)GetProcAddress(gameDLL, "SV_InitAPIs");
		if (!SV_InitAPIs)
			System_Error("Coudln't find entry function SV_InitAPIs()\n");

		SV_InitAPIs(&coreAPI_s, &sv_api);
	}

	//make sure the game code has filled in all its functions
	Host_TestGameAPIValidity(&cl_api, &sv_api, &int_api);

	if (strcmp(System_GetCommandLine(), "write_version") == 0)
	{
		FILE *f = fopen("current_version.txt", "w");

		if (!f)
		{
			System_Printf("Couldn't open file\n");
			exit(1);
		}

		fprintf(f, "%s\n", cl_api.GetBuild());
		fclose(f);
		exit(0);
	}

	if (dedicated_server.integer)
	{
		if (hConWindow)
		{
			ShowWindow(hConWindow, SW_SHOWNORMAL);		
			//set KB focus to the command entry box
			SetFocus(GetDlgItem(hConWindow, IDC_COMMAND));
		}
	}
	else
	{
		if (hConWindow)
		{
			DestroyWindow(hConWindow);
			hConWindow = 0;
		}
	}
}

void    System_ChangeRootDir(const char *newdir)
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

void System_UnloadGameDLL()
{
	if (gameDLL)
		FreeLibrary(gameDLL);
}

void System_ShutDown()
{
	static bool shuttingDown = false;
	_shutdownfunc_t ShutdownGameDLL;

	if (shuttingDown)
		return;

	shuttingDown = true;
	
	Bink_ShutDown();

	ShutdownGameDLL = (	_shutdownfunc_t)GetProcAddress(gameDLL, "ShutdownGameDLL");
	if (ShutdownGameDLL)
		ShutdownGameDLL();

	Host_ShutDown();

	System_UnloadGameDLL();
    if (screensaver)
		SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, TRUE, 0, 0);		

	timeEndPeriod(1);

	shuttingDown = false;
}

bool System_Say(const char *message)
{
	return false;
}

extern cvar_t sys_restartProcess;

void System_Quit()
{
	char *cmdline;
	bool success;

	System_ShutDown();

	if (sys_restartProcess.integer)
	{
		STARTUPINFO			si;
		PROCESS_INFORMATION	pi;

		memset(&si, 0, sizeof(si));
		memset(&pi, 0, sizeof(pi));

		chdir(originaldir);
		cmdline = GetCommandLine();
		success = CreateProcess(NULL, cmdline, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	}

	exit(0);
}

HINSTANCE	System_Win32_GetHInstance()
{
	return win32_hinstance;
}

void	System_ProcessEvents()
{
    MSG msg;

	OVERHEAD_INIT;

	while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
	{	
		if (GetMessage(&msg, NULL, 0, 0)==0)
			System_Quit();		//received a WM_QUIT message
      	TranslateMessage(&msg);
      	DispatchMessage(&msg);
	}

	OVERHEAD_COUNT(OVERHEAD_SYSTEM_PROCESSEVENTS);
}

int MapKey(int wParam)
{
	if ( (wParam >= 0 && wParam <= 255) && keymap[wParam][0] )
	{
//		OutputDebugString(keyinits[keymap[wParam][1]].key ? keyinits[keymap[wParam][1]].key : "NULL");
//		OutputDebugString("\n");
		return keymap[wParam][0];
	}
	else
	{
		return tolower(wParam);
		/*byte map = MapVirtualKey(wParam, 2);
		if ( map )
		{
			char ch[2] = { map, 0 };
//			OutputDebugString(ch);
//			OutputDebugString("\n");

			switch ( map )
			{
				case 0x08: return KEY_BACKSPACE;
			}
		
			return tolower(map);
		}
		else
		{
			return 0;
		}*/
	}
}

#ifndef SAVAGE_CONSOLE_APP

LONG WINAPI GL_Win32_MainWndProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int key = 0;
	bool down = true;
	static PAINTSTRUCT ps;

	switch(uMsg)
	{
		case WM_ACTIVATE:
			//Console_Printf("WM_ACTIVATE: %i %i\n", LOWORD(wParam), HIWORD(wParam));
			if (LOWORD(wParam))
				Cvar_SetVarValue(&sys_focus, 1);
			else
				Cvar_SetVarValue(&sys_focus, 0);
			return DefWindowProc (hWnd, uMsg, wParam, lParam);

		case WM_CREATE:
//			SetForegroundWindow(hWnd);
			return 1;		
		case WM_CLOSE:
			Cmd_Exec("quit\n");
			return 1;		
		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_XBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		case WM_XBUTTONUP:		
			if (inverty.integer)
				Input_UpdateMouse(LOWORD(lParam), Vid_GetScreenH() - HIWORD(lParam));
			else
				Input_UpdateMouse(LOWORD(lParam), HIWORD(lParam));
			switch(uMsg)
			{
				case WM_LBUTTONDOWN: key = KEY_LBUTTON; break;
				case WM_RBUTTONDOWN: key = KEY_RBUTTON; break;
				case WM_MBUTTONDOWN: key = KEY_MBUTTON; break;
				case WM_XBUTTONDOWN:
					switch(LOWORD(wParam))
					{
						case MK_XBUTTON1:	
							key = KEY_XBUTTON1;
							break;
						case MK_XBUTTON2:
							key = KEY_XBUTTON2;
							break;
						default:
							break;
					}
				case WM_LBUTTONUP: key = KEY_LBUTTON; down=false; break;
				case WM_RBUTTONUP: key = KEY_RBUTTON; down=false; break;
				case WM_MBUTTONUP: key = KEY_MBUTTON; down=false; break;
				case WM_XBUTTONUP:
					switch(LOWORD(wParam))
					{
						case MK_XBUTTON1:
							key = KEY_XBUTTON1;
							down = false;
							break;
						case MK_XBUTTON2:
							key = KEY_XBUTTON2;
							down = false;
							break;
						default:
							break;
					}
					
			}
			if (key)
  				Input_Event(key, 0, down);
			return 1;
		case WM_MOUSEWHEEL: 
			if ((short)HIWORD(wParam) > 0)
			{
				Input_Event(KEY_MWHEELUP, 0, true);
				Input_Event(KEY_MWHEELUP, 0, false);
			} 
			else
			{
				Input_Event(KEY_MWHEELDOWN, 0, true);
				Input_Event(KEY_MWHEELDOWN, 0, false);
			}
			return 1;
	
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		{
			int map = MapKey(wParam);
			if (map)
				Input_Event(map, 0, true);
			return 1;
		}
		case WM_KEYUP:
		case WM_SYSKEYUP:					
		{
			int map = MapKey(wParam);
			if (map)
				Input_Event(map, 0, false);
			return 1;			
		}
		case WM_CHAR:
		{
			if (wParam >= 32)
				Input_Event(0, (wParam & 0xff), true);
			return 1;
		}
		case WM_PAINT:
			//i'm not convinced this actually serves any purpose, but
			//several people think it does, so i'll stick it in
			BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
			return 0;
		default:
            return DefWindowProc (hWnd, uMsg, wParam, lParam);
	}
}

unsigned int System_GetTicks()
{
	return (unsigned int)GetTickCount();
}

int			sys_argc = 0;
char		*sys_argv[SYS_MAX_ARGV];

int			System_GetArgc()
{
	return sys_argc;
}

char		*System_GetArgv(int arg)
{
	if (arg < sys_argc)
	{
		return sys_argv[arg];
	}

	return NULL;
}

void	_SplitCommandLine(char *s)
{
	while (*s)
	{
		while (*s && ((*s <= 32) || (*s > 126)))
			s++;

		if (*s)
		{
			sys_argv[sys_argc++] = s;

			sys_argc++;

			if (sys_argc >= SYS_MAX_ARGV)
			{
				return;
			}

			while (*s && ((*s > 32) && (*s <= 126)))
				s++;

			if (*s)
			{
				*s = 0;
				s++;
			}
		}
	}
}

char *System_GetCommandLine()
{
	return sys_cmdline;
}

void System_Sleep(unsigned int ms)
{
	Sleep(ms);
}

/*LPVOID crashRpt;

cvar_t	sys_restartOnCrash =	{ "sys_restartOnCrash",	"1", CVAR_SAVECONFIG };
cvar_t	sys_politeShutdown =	{ "sys_politeShutdown",	"1" };

bool CALLBACK CrashRptCallback(LPVOID crashRpt)
{
	STARTUPINFO			si;
	PROCESS_INFORMATION	pi;

	memset(&si, 0, sizeof(si));
	memset(&pi, 0, sizeof(pi));

	Console_Printf("\n\n\n*** CRASH ***\n\n\n");
	File_Flush(DEBUG_LOG_FILE);
	if (sys_politeShutdown.integer)
	{
		if (localClient.cstate)
			Client_Disconnect();
		if (localServer.active)
			Server_Disconnect();
		System_ShutDown();
	}

	while(ShowCursor(true) < 1);

	AddFile(crashRpt, "..\\debug.log", "Console log");
	AddFile(crashRpt, "..\\scripts.log", "Script log");

	if (dedicated_server.integer && sys_restartOnCrash.integer)
	{
		chdir(originaldir);
		CreateProcess(NULL, GetCommandLine(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	}
	return true;
}*/

bool	launched_url = false;

void	LaunchURL_Cmd(int argc, char *argv[])
{
	if (argc < 1)
		return;

	system(fmt("explorer %s", argv[0]));

	launched_url = true;
//	Cmd_Exec("quit");
}

int	System_Milliseconds()
{
	return timeGetTime() - sys_startTime;
}

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{	
	int time, lasttime;
	char *argv[2], *fullcmdline, *at;

	//this will stop multiple instances from starting up, but
	//only on non-NT based versions of windows
	if (hPrevInstance)
		return 0;

	//if another dedicated server is running already, give it up to five seconds to shut down
	//before starting up (if this is a crash restart)
	time = timeGetTime();
	while (timeGetTime() - time < 5000 && FindWindow(NULL, "Savage Dedicated Console"));

	win32_hinstance = hInstance;

	sys_cmdline = lpCmdLine;
	at = fullcmdline = strdup(GetCommandLine());

	//crashRpt = Install(CrashRptCallback, "jason@s2games.com", "Savage bug report ["GAME_VERSION"]");

	if (fullcmdline[0] == '"')
	{
		at++;
	}
	
	SplitArgs(at, argv, 2);
	
	if (argv[0][strlen(argv[0])-1] == '"')
	{
		argv[0][strlen(argv[0])-1] = 0;
	}

	//	_SplitCommandLine(lpCmdLine);

	Bink_Initialize();
	Completion_Init();
	System_Init();
	Host_Init();

	engine_hashlen = Hash_FilenameAbsolute(argv[0], engine_hash);

	free(fullcmdline);

	Cmd_Register("launchURL", LaunchURL_Cmd);

	timeBeginPeriod(1);

	lasttime = timeGetTime();
	sys_time = 0;

	while(1)
	{
		int elapsed;

		//we call Host_Frame() as much as possible, and let it
		//handle the actual frame rate the game logic runs at		

		if (dedicated_server.integer || sys_sleep.integer || !sys_focus.integer)
			System_Sleep(1);		//allow time for other processes

		//the following loop is necessary because time - lasttime could round to 0
		do
		{
			time = timeGetTime();
		} while (time-lasttime < 1);

		elapsed = time-lasttime;
		sys_time += elapsed;
		Host_Frame(elapsed);
		
		lasttime = time;
	}
	return TRUE;  //we'll never get here
}

#endif

static double		pfreq;
static double		curtime = 0.0;
static double		lastcurtime = 0.0;
static int			lowshift;

void System_Printf(char* msg, ...)
{
	char s[4096], out[4096];
	char *linestart;
	char *sptr = s;

	va_list	argptr;

	va_start(argptr, msg);
	vsprintf(s, msg, argptr);
	va_end(argptr);

	if (hConWindow)
	{
		linestart = sptr;

		while (*sptr)
		{
			switch (*sptr)
			{
				case '\n':
					*sptr = 0;
					strcpy(out, linestart);
					strcat(out, "\r\n");

					SendDlgItemMessage(
						hConWindow,
						IDC_OUTPUT,
						EM_SETSEL,
						65535,
						65535);

					SendDlgItemMessage(
						hConWindow,
						IDC_OUTPUT,
						EM_REPLACESEL,
						0,
						(LPARAM)out);

					SendDlgItemMessage(
						hConWindow,			
						IDC_OUTPUT,
						EM_LINESCROLL,
						0,
						65535);
					SendDlgItemMessage(
						hConWindow,
						IDC_OUTPUT,
						EM_SCROLLCARET,
						0,
						0);
					
					linestart = sptr+1;

					break;
				default:
					break;
			}

			sptr++;
		}
	}
}


void System_Error(char* msg, ...)
{
	va_list argptr;
	char s[1024];
	
/*	printf("\n******************** ERROR ********************\n");	

	va_start(argptr, msg);
	vprintf(msg, argptr);
	va_end(argptr);

	printf("\n***********************************************\n");
*/
	va_start(argptr, msg);
	vsprintf(s, msg, argptr);
	va_end(argptr);
	
	System_ShutDown();

	MessageBox(NULL, s, "Silverback Engine: Error", MB_OK);

	exit(1);
}

//FIXME: some timer source stolen from winquake

void System_InitTimer()
{
	LARGE_INTEGER timerfreq;
	unsigned int lowpart, highpart;

	if (!QueryPerformanceFrequency(&timerfreq))
	{
		System_Error("System_InitTimer: Hardware timer not available");
	}

	// get 32 out of the 64 time bits such that we have around
	// 1 microsecond resolution
	lowpart = (unsigned int)timerfreq.LowPart;
	highpart = (unsigned int)timerfreq.HighPart;
	lowshift = 0;

	while (highpart || (lowpart > 2000000.0))
	{
		lowshift++;
		lowpart >>= 1;
		lowpart |= (highpart & 1) << 31;
		highpart >>= 1;
	}

	pfreq = 1.0 / (double)lowpart;

	System_GetPerfCounter();

	curtime = 0.0;

	lastcurtime = curtime;

	sys_startTime = timeGetTime();
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

	File_SystemDir(dirname, wcard, recurse, dirCallback, fileCallback, userdata);

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
	 			if (strcmp(finddata.cFileName, ".") && strcmp(finddata.cFileName, "..") && strcmp(finddata.cFileName, "CVS"))
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

char	*System_GetRootDir()
{
	return rootdir;
}

//returns time since program started in seconds
double System_GetPerfCounter()
{	/*
	QueryPerformanceCounter(&time);
	
	return ((time/timerfreq) - starttime);
	*/
	static int			sametimecount;
	static unsigned int	oldtime;
	static int			first = 1;
	LARGE_INTEGER		PerformanceCount;
	unsigned int		temp, t2;
	double				time;

	QueryPerformanceCounter (&PerformanceCount);

	temp = ((unsigned int)PerformanceCount.LowPart >> lowshift) |
		   ((unsigned int)PerformanceCount.HighPart << (32 - lowshift));

	if (first)
	{
		oldtime = temp;
		first = 0;
	}
	else
	{
	// check for turnover or backward time
		if ((temp <= oldtime) && ((oldtime - temp) < 0x10000000))
		{
			oldtime = temp;	// so we can't get stuck
		}
		else
		{
			t2 = temp - oldtime;

			time = (double)t2 * pfreq;
			oldtime = temp;

			curtime += time;

			if (curtime == lastcurtime)
			{
				sametimecount++;

				if (sametimecount > 100000)
				{
					curtime += 1.0;
					sametimecount = 0;
				}
			}
			else
			{
				sametimecount = 0;
			}

			lastcurtime = curtime;
		}
	}

    return curtime;
}

#endif //_WIN32
