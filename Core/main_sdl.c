// (C) 2003 S2 Games

// main_sdl.c

#define	SYS_MAX_ARGV	32

#include "core.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <glib.h>
#include <SDL/SDL.h>
#ifdef linux
#include <execinfo.h>
#endif
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define WM_MOUSEWHEEL                   0x020A

#define BACKTRACE_SIZE 20

bool hasMouseFocus = true;
int screensaver;
char    rootdir[_MAX_PATH];
char *sys_cmdline;
void *gameDLL;

int sys_time = 0;

char engine_hash[MAX_HASH_SIZE];
int engine_hashlen;

int	keyDown = -1;
int keyDownTime = 999999999;

FILE *festival = NULL;

typedef void (*_shutdownfunc_t)(void);
extern _shutdownfunc_t _ShutdownGameDLL;

extern cvar_t gfx;
extern cvar_t speak;
extern cvar_t voice;
extern cvar_t sys_enumdir;
extern cvar_t sys_sleep;
extern cvar_t sys_focus;
extern cvar_t sys_useUnicode;
extern cvar_t sys_showKeycodes;
extern cvar_t inverty;

void SDL_GrabInput(bool grab);

/* clean quit on Ctrl-C for SDL and thread shutdown as per SDL example
 *    (we don't use any threads, but libSDL does) */
static void sigint_handler (int signal) {
	System_Quit();
}	

void System_InitTimer();

void System_Init()
{
	char dir[_MAX_PATH], *s;

	System_Printf("System_Init()\n");

	Mem_Init();

	if ( SDL_Init(SDL_INIT_TIMER | SDL_INIT_NOPARACHUTE) < 0 ) 
	{
		fprintf(stderr,
			"Couldn't initialize SDL: %s\n", SDL_GetError());
		exit(1);
   	}

	/* install signal handler as SDL clobbered the default */
	signal (SIGINT, sigint_handler);

	System_InitTimer();
	
	SDL_GrabInput(true);
	SDL_EnableUNICODE(sys_useUnicode.integer);

	getcwd(dir, _MAX_PATH-1);
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

void SDL_GrabInput(bool grab)
{
  	bool mode;
	//grab our input here
	mode = SDL_WM_GrabInput(grab);

	mode = SDL_WM_GrabInput(SDL_GRAB_QUERY);
	if (mode != grab)
		Console_DPrintf("failed to grab input!\n");
}

void System_ShutDown()
{
	static bool shuttingDown = false;
		
	if (shuttingDown)
	{
		return;
	}
	
	shuttingDown = true;
	if (_ShutdownGameDLL)
		_ShutdownGameDLL();
	System_UnloadGameDLL();
    //fixme: re-enable screensaver
    //if (screensaver)
	//	SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, TRUE, 0, 0);
	SDL_GrabInput(false);
	SDL_EnableUNICODE(false);
	SDL_ShowCursor(true);

	if (festival)
	{
		fprintf(festival, "(quit)\n");
		pclose(festival);
	}
	
	shuttingDown = false;

	SDL_QuitSubSystem(SDL_INIT_TIMER | SDL_INIT_VIDEO);

	Host_ShutDown();

}

extern cvar_t sys_restartProcess;

void System_Quit()
{
	System_ShutDown();

	if (sys_restartProcess.integer)
	{
		chdir(rootdir);
		system(sys_cmdline);
	}

	exit(0);
}

void System_ProcessEvents()
{ 
	static float lastRepeat = 0;
	int	now = System_Milliseconds();
	if (keyDown >= 0)
	{
		if (now - keyDownTime > SDL_DEFAULT_REPEAT_DELAY)
		{
			if (now - lastRepeat > SDL_DEFAULT_REPEAT_INTERVAL)
			{
				Input_Event(keyDown, keyDown, true);
				lastRepeat = now;
			}
		}
	}
}

#ifndef SAVAGE_CONSOLE_APP

#define MAX_BUTTONS	9

static int buttonDownThisFrame[MAX_BUTTONS] = { 0 };;
static int buttonUpThisFrame[MAX_BUTTONS] = { 0 };;

void	ResetButtonData()
{
	int i;
	for (i = 0; i < MAX_BUTTONS; i++)
	{
		buttonDownThisFrame[i] = false;
		buttonUpThisFrame[i] = false;
	}
}

int HandleEvent(SDL_Event *event)
{
	static int buttonStates[MAX_BUTTONS] = { 0 };;
	int done, rawkey, key, unicode;

	/*
	 * from the win32 event handling, not sure if it's necessary
		case WM_SYSKEYDOWN:			
			Input_Event(MapKey(lParam), true);
			return 1;
		case WM_SYSKEYUP:					
			Input_Event(MapKey(lParam), false);
			return 1;			
	 *
	 */
	done = 0;
	switch( event->type ) {
	    case SDL_ACTIVEEVENT:
			/* See what happened */
			printf( "app %s ", event->active.gain ? "gained" : "lost" );
			if ( event->active.state & SDL_APPACTIVE ) 
			{
				printf( "active " );
				/*
				if (event->active.gain)
					Cvar_SetVarValue(&gfx, 1);
				else
					Cvar_SetVarValue(&gfx, 0);
				*/
			} 
			else if ( event->active.state & SDL_APPMOUSEFOCUS ) {
				printf( "mouse " );
				hasMouseFocus = event->active.gain;
			} else if ( event->active.state & SDL_APPINPUTFOCUS ) {
				printf( "input " );
				hasMouseFocus = event->active.gain;
			}
			printf( "focus\n" );
			break;

	    case SDL_KEYDOWN:
	    case SDL_KEYUP:
			/*if ( (event->key.keysym.unicode & 0xFF80) == 0 )
				key = event->key.keysym.unicode & 0x7F;
			else
				//international keyboard, try using the keysym
			*/
			key = event->key.keysym.sym;
			if (event->key.keysym.unicode)
				unicode = event->key.keysym.unicode & 0x7F;
			else
				unicode = key;
			rawkey = key;
			
			switch (key)
			{
				case SDLK_LSHIFT:
				case SDLK_RSHIFT:
				  			key = KEY_SHIFT;
				  			break;
				case SDLK_LCTRL:
				case SDLK_RCTRL:
				  			key = KEY_CTRL;
				  			break;
				case SDLK_LALT:
				case SDLK_RALT:
				  			key = KEY_ALT;
				  			break;
				case SDLK_ESCAPE:
							key = KEY_ESCAPE;
							break;
				case SDLK_BACKSPACE:
							key = KEY_BACKSPACE;
							break;
				case SDLK_PAUSE:
							key = KEY_PAUSE;
							break;
				case SDLK_HOME:
							key = KEY_HOME;
							break;
				case SDLK_END:
							key = KEY_END;
							break;
				case SDLK_DELETE:
							key = KEY_DEL;
							break;
				case SDLK_PAGEUP:
							key = KEY_PGUP;
							break;
				case SDLK_PAGEDOWN:
							key = KEY_PGDN;
							break;
				case SDLK_UP:
				  			key = KEY_UP;
				  			break;
				case SDLK_DOWN:
				  			key = KEY_DOWN;
				  			break;
				case SDLK_LEFT:
				  			key = KEY_LEFT;
				  			break;
				case SDLK_RIGHT:
				  			key = KEY_RIGHT;
				  			break;
				case SDLK_KP0:
				case SDLK_KP1:
				case SDLK_KP2:
				case SDLK_KP3:
				case SDLK_KP4:
				case SDLK_KP5:
				case SDLK_KP6:
				case SDLK_KP7:
				case SDLK_KP8:
				case SDLK_KP9:
							key = KEY_KEYPAD_0 + key - SDLK_KP0;
							break;
				case SDLK_KP_PERIOD:
							key = KEY_KEYPAD_DEL;
							break;
				case SDLK_KP_DIVIDE:
							key = KEY_KEYPAD_SLASH;
							break;
				case SDLK_KP_MULTIPLY:
							key = KEY_KEYPAD_ASTERISK;
							break;
				case SDLK_KP_MINUS:
							key = KEY_KEYPAD_MINUS;
							break;
				case SDLK_KP_PLUS:
							key = KEY_KEYPAD_PLUS;
							break;
				case SDLK_KP_ENTER:
							key = KEY_KEYPAD_ENTER;
							break;
				case SDLK_LSUPER:
							key = KEY_LEFT_WINDOWS;
							break;
				case SDLK_RSUPER:
							key = KEY_RIGHT_WINDOWS;
							break;
				case SDLK_MENU:
							key = KEY_MENU;
							break;
				case SDLK_F1:
				case SDLK_F2:
				case SDLK_F3:
				case SDLK_F4:
				case SDLK_F5:
				case SDLK_F6:
				case SDLK_F7:
				case SDLK_F8:
				case SDLK_F9:
				case SDLK_F10:
				case SDLK_F11:
				case SDLK_F12:
				case SDLK_F13:
				case SDLK_F14:
				case SDLK_F15:
							key = KEY_F1 + key - SDLK_F1;
							break;
				case 132:
							key = KEY_ALT;
							break;
			}
			if (key != rawkey) //we modified the value
				unicode = key;

			if (sys_showKeycodes.integer)
				Console_Printf("key %i (%s) (unicode %i [%s]) %s\n", key, SDL_GetKeyName(key), unicode, SDL_GetKeyName(unicode), event->type == SDL_KEYDOWN ? "pressed" : "released");

			if (event->type == SDL_KEYDOWN)
			{
				//alt-enter hack
				if (key == KEY_ENTER && keyDown == KEY_ALT)
				{
					Cmd_Exec("toggle vid_fullscreen; changevideo");
				}
				//alt-tab hack
				if (key == KEY_TAB && keyDown == KEY_ALT)
				{
					!SDL_WM_IconifyWindow();
				}
					
				//Console_DPrintf("key %i pressed\n", key);
				keyDown = key;
				keyDownTime = System_Milliseconds();
			}
			else
			{
				if (keyDown == key)
				{
					//Console_DPrintf("key %i released\n", key);
					keyDown = -1;
				}
			}

			//printf("button %c %s\n", key, event->key.state ? "pressed" : "unpressed");
			Input_Event(key, unicode, event->key.state);
			done = 1;
			break;
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			//fprintf(stderr, "mouse button event!  button %d is %d\n", event->button.button, event->button.state);
			if (((event->button.state && !buttonDownThisFrame[event->button.button]) 
				|| (!event->button.state && !buttonUpThisFrame[event->button.button]))
				&& event->button.state != buttonStates[event->button.button])
			{
				if (event->button.state)
					buttonDownThisFrame[event->button.button] = true;
				else
					buttonUpThisFrame[event->button.button] = true;
				buttonStates[event->button.button] = event->button.state;
				switch(event->button.button)
				{
					case SDL_BUTTON_LEFT:
						Input_Event(KEY_LBUTTON, 0, event->button.state);
						done = 1;
						break;
					case SDL_BUTTON_RIGHT:
						Input_Event(KEY_RBUTTON, 0, event->button.state);
						done = 1;
						break;
					case SDL_BUTTON_MIDDLE:
						Input_Event(KEY_MBUTTON, 0, event->button.state);
						done = 1;
						break;
					case SDL_BUTTON_WHEELUP:
						Input_Event(KEY_MWHEELUP, 0, event->button.state);
						done = 1;
						break;
					case SDL_BUTTON_WHEELDOWN:
						Input_Event(KEY_MWHEELDOWN, 0, event->button.state);
						done = 1;
						break;
					default:
						Console_DPrintf("unknown button %d\n", event->button.button);
						break;
				}
			}
			break;
		case SDL_MOUSEMOTION:
			//fprintf(stderr, "mouse motion event!  now (%d, %d)\n", event->motion.x, event->motion.y);
			//Input_UpdateMouse(event->motion.x, event->motion.y);
			break;
        case SDL_VIDEORESIZE:
		    done = 1;
			break;
	    case SDL_QUIT:
			System_Quit();
			done = 1;
		break;
	}
	return(done);
}

unsigned int System_GetTicks()
{
	struct timeval tv;

	gettimeofday(&tv, NULL);
	return (tv.tv_sec % 1000000) * 1000 +  tv.tv_usec / 1000;
}

bool	System_Say(const char *string)
{
#ifndef NO_FESTIVAL
	static float currentSpeed = 0.0;
	static float currentPitch = 0.42;
	char parsedString[1024];
	char *tmp, *end;
	if (festival && speak.integer && string && string[0])
	{
		if (voice.modified)
		{
			if (strcmp(voice.string, "faster") == 0)
			{
				currentSpeed -= 0.1;
				fprintf(festival, fmt("(set! hts_duration_stretch %f)", currentSpeed));
			}
			else if (strcmp(voice.string, "slower") == 0)
			{
				currentSpeed += 0.1;
				fprintf(festival, fmt("(set! hts_duration_stretch %f)", currentSpeed));
			}
			else if (strcmp(voice.string, "lower") == 0)
			{
				currentPitch += 0.4;
				fprintf(festival, fmt("(set! hts_fw_factor %f)", currentPitch));
			}
			else if (strcmp(voice.string, "higher") == 0)
			{
				currentPitch -= 0.4;
				fprintf(festival, fmt("(set! hts_fw_factor %f)", currentPitch));
			}
			else
			{
				fprintf(festival, fmt("(%s)", voice.string));
			}
			voice.modified = false;
		}

		strncpy(parsedString, string, 1024);
		parsedString[1023] = 0;
		while ((tmp = strstr(parsedString, "^")))
		{
			end = strstr(tmp+1, "^");
			if (!end)
				break;

			memmove(tmp, end+1, strlen(end));
		}
		
		
		fprintf(festival, "(SayText \"%s\")\n", parsedString);
		fflush(festival);
	}
#endif
	return true;
}

static void print(const char *line) {
  while(*line) {
    putchar(*line);
    line++;
  }
}

void print_backtrace(int signum) 
{
#ifdef linux
  void *bt_array[BACKTRACE_SIZE]; // Static means always allocated
  int size = BACKTRACE_SIZE, i;   //  == probably safer
  const char *signame;
  char **symbols;
  size = backtrace(bt_array, BACKTRACE_SIZE);
  symbols = backtrace_symbols(bt_array, size); // Print out symbol names on stderr
  switch(signum) {
  case SIGILL: signame = "SIGILL"; break;
  case SIGQUIT: signame = "SIGQUIT"; break;
  case SIGABRT: signame = "SIGABRT"; break;
  case SIGFPE: signame = "SIGFPE"; break;
  case SIGSEGV: signame = "SIGSEGV"; break;
  case SIGBUS: signame = "SIGBUS"; break;
#ifdef SIGSYS
  case SIGSYS: signame = "SIGSYS"; break;
#endif
  default: signame = "(unexpected)"; break;
  }
  Console_Printf("Signal ");
  Console_Printf(signame);

  Console_Printf(" received.\n\n"
	 "Stack dump:\n{\n" );
  for (i = 0; i < size; i++ ) {
    Console_Printf("\t");
    if(symbols[i]) {
      Console_Printf(symbols[i]);
    } else {
      Console_Printf(bt_array[i]);
    }
    Console_Printf("\n");
  }

  Console_Printf("}\n\n" );
#endif
  SDL_Quit();
  exit(127+signum);
}

/* as children die we should get catch their returns or else we get * zombies, A Bad Thing. */
void killchild(int signal)
{
	printf("killing child process\n");
	while (waitpid(-1, NULL, WNOHANG) > 0) ;
}

int file_exists(char *filename)
{
	FILE *f;

	f = fopen(filename, "r");
	if (f)
	{
		fclose(f);
		return 1;
	}
	return 0;
}

bool launched_url = false;

void LaunchURL_cmd(int argc, char *argv[])
{
	if (file_exists("/usr/bin/galeon"))
		system(fmt("/usr/bin/galeon '%s'", argv[0]));
	else if (file_exists("/usr/bin/mozilla"))
		system(fmt("/usr/bin/mozilla '%s'", argv[0]));
	else if (file_exists("/usr/bin/konquerer"))
		system(fmt("/usr/bin/konquerer '%s'", argv[0]));
	else if (file_exists("/usr/bin/netscape"))
		system(fmt("/usr/bin/netscape '%s'", argv[0]));
	
	launched_url = true;

//	Cmd_Exec("quit");
}

int	System_Milliseconds()
{
	//printf("ms: %i\n", SDL_GetTicks());
	return SDL_GetTicks();
}

int main(int argc, char *argv[])
{	
  	SDL_Event event;
	double time, lasttime;
	int i, length, x, y;
	bool done;

	signal(SIGILL, print_backtrace);
	//signal(SIGQUIT, print_backtrace);
	signal(SIGABRT, print_backtrace);
	signal(SIGFPE, print_backtrace);
	signal(SIGSEGV, print_backtrace);
	signal(SIGBUS, print_backtrace);
#ifdef SIGSYS
	signal(SIGSYS, print_backtrace);
#endif	
	
	Completion_Init();
	System_Init();
	
	length = 1;
	for (i = 1; i < argc; i++)
		length += 1 + strlen(argv[i]);
	sys_cmdline = Tag_Malloc(length, MEM_SYSTEM);
	strcpy(sys_cmdline, "");
	for (i = 1; i < argc; i++)
	{
		strcat(sys_cmdline, argv[i]);
	    strcat(sys_cmdline, " ");
	}

	signal(SIGCHLD, killchild); /* this eliminates zombies */
	
	Host_Init();

	Cmd_Register("launchurl", &LaunchURL_cmd);
	
	//hash the file once it's already opened, to make a file switch harder
	engine_hashlen = Hash_FilenameAbsolute(argv[0], engine_hash);
	
	lasttime = System_Milliseconds();
	sys_time = 0;

	SDL_EnableUNICODE(sys_useUnicode.integer);
	
	while(1)
	{				
		int elapsed;

		if (dedicated_server.integer || sys_sleep.integer || !sys_focus.integer)
			System_Sleep(1);		//allow time for other processes

		//the following loop is necessary because System_GetTime() - lasttime could be dangerously close to 0
		do
		{
			time = System_Milliseconds();
		} while (time-lasttime < 1);

		if (gfx.integer)
		{
			ResetButtonData();
			/* Check if there's a pending event. */
			while( SDL_PollEvent( &event ) ) 
			{
				done = HandleEvent(&event);
			}
			SDL_GetMouseState(&x, &y);
			if (inverty.integer)
				y = Vid_GetScreenH() - y;
			Input_UpdateMouse(x, y);
		}
		if (sys_useUnicode.modified)
		{
			SDL_EnableUNICODE(sys_useUnicode.integer);
			sys_useUnicode.modified = false;
		}

		elapsed = (time-lasttime) * 1000;

		sys_time += elapsed;

		Host_Frame((float)elapsed/1000);

		lasttime = time;
	}
	return TRUE;  //this will never get called
}

#endif

static double		curtime = 0.0;
static double		lastcurtime = 0.0;

void System_Printf(char* msg, ...)
{
	va_list	argptr;

	va_start(argptr, msg);
	vprintf(msg, argptr);
	va_end(argptr);
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

	fprintf(stderr, s);

	print_backtrace(SIGABRT);
	System_ShutDown();
	exit(1);
}

void System_InitTimer()
{
	System_GetPerfCounter();

	curtime = 0.0;

	lastcurtime = curtime;
}

char	*System_GetRootDir()
{
	//Console_DPrintf("root dir is %s\n", rootdir);
	return rootdir;
}

char *System_GetCommandLine()
{
      return sys_cmdline;
}

void System_Sleep(unsigned int ms)
{
	//SDL_Delay(ms);
	usleep(ms * 1000);
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
	unsigned int		temp;
	double				time;

	temp = SDL_GetTicks();

	if (first)
	{
		oldtime = temp;
		first = 0;
		curtime = 0;
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
			time = temp - oldtime;

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

	//fprintf(stderr, "time is %f\n", curtime/1000);
    return curtime / 1000;
}
