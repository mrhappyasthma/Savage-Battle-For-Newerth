// (C) 2003 S2 games

// input.c

// mouse / keyboard handling routines

#include "core.h"
#include "zip.h"

cvar_t	move_forward =	{ "move_forward",	"0" };
cvar_t	move_backward =	{ "move_backward",	"0" };
cvar_t	move_left =		{ "move_left",		"0" };
cvar_t	move_right =	{ "move_right",		"0" };
cvar_t	move_up =		{ "move_up",		"0" };
cvar_t	move_down =		{ "move_down",		"0" };
cvar_t	move_zig =		{ "move_zig",		"0" };
cvar_t	move_zag =		{ "move_zag",		"0" };
cvar_t	button1 =		{ "button1",		"0" };
cvar_t	button2 =		{ "button2",		"0" };
cvar_t	button3 =		{ "button3",		"0" };
cvar_t	button4 =		{ "button4",		"0" };
cvar_t	button5 =		{ "button5",		"0" };
cvar_t	button6 =		{ "button6",		"0" };
cvar_t	button7 =		{ "button7",		"0" };
cvar_t	button8 =		{ "button8",		"0" };
cvar_t	item =			{ "item",			"0" };
cvar_t	yawSpeed =		{ "yawSpeed",		"1" };
cvar_t	pitchSpeed =	{ "pitchSpeed",		"1.5" };
cvar_t	sensitivity =	{ "sensitivity",	"12",	CVAR_SAVECONFIG };
cvar_t	ysensitivity =	{ "ysensitivity",	"0.9",	CVAR_SAVECONFIG };
cvar_t	invertmouse =	{ "invertmouse",	"0",	CVAR_SAVECONFIG };
cvar_t	pitch_high =	{ "maxpitch",		"89",	CVAR_SAVECONFIG };
cvar_t	pitch_low =		{ "minpitch",		"-89",	CVAR_SAVECONFIG };
cvar_t	consolekeys =	{ "consolekeys",	"`~\xB2" };
cvar_t	anykeycommand = { "anykeycommand", "" };

#define MAX_KEYS 255

mousepos_t	mousepos;

int		mousemode = 0;
bool	keys[MAX_KEYS];  //if true, key is down
static float sensitivityScale = 1.0;

//=============================================================================
// Aliases
//
// An alias allows a key to be bound to a short name, but can contain a longer
// command line that is executed when the key is pressed.  Each alias also
// tracks which keys are bound to it, which can be used for config interfaces
//=============================================================================
#define	BIND_MAX_CMD_LEN	512

#define	ALIAS_MAX_NAME_LEN	64
#define	ALIAS_MAX_BINDS		8

#define	MAX_ALIASES			256

typedef struct
{
	int		index;

	char	name[ALIAS_MAX_NAME_LEN];
	char	keys[ALIAS_MAX_BINDS];
	char	cmd[BIND_MAX_CMD_LEN];
	char	upcmd[BIND_MAX_CMD_LEN];
}
alias_t;

alias_t aliasList[MAX_ALIASES];
//=============================================================================

//=============================================================================
// Binds
//=============================================================================
#define MAX_BIND_PROFILES	2

typedef enum
{
	BIND_NONE,
	BIND_CMD,
	BIND_ALIAS,

	NUM_BIND_TYPES
}
bindType_e;

typedef struct
{
	bindType_e	type;

	char	cmd[BIND_MAX_CMD_LEN];
	char	upcmd[BIND_MAX_CMD_LEN];
	int		alias;
}
keybind_t;

keybind_t	binds[MAX_BIND_PROFILES][MAX_KEYS];

int currentBindProfile = 0;
//=============================================================================

inputState_t	inputState;

inputcallback_t		input_callback_func;
bool				input_callback_active;
void				*input_callback_userdata;

typedef struct
{
	char *desc;
	int num;
} keydesc_t;


keydesc_t keydescs[] = 
{
	{ "tab",			KEY_TAB },
	{ "enter",			KEY_ENTER },
	{ "space",			' ' },
	{ "esc",			KEY_ESCAPE },
	{ "backspace",		KEY_BACKSPACE },
	{ "up",				KEY_UP },
	{ "left",			KEY_LEFT },
	{ "right",			KEY_RIGHT },
	{ "down",			KEY_DOWN },
	{ "alt",			KEY_ALT },
	{ "ctrl",			KEY_CTRL },
	{ "shift",			KEY_SHIFT },
	{ "f1",				KEY_F1 },
	{ "f2",				KEY_F2 },
	{ "f3",				KEY_F3 },
	{ "f4",				KEY_F4 },
	{ "f5",				KEY_F5 },
	{ "f6",				KEY_F6 },
	{ "f7",				KEY_F7 },
	{ "f8",				KEY_F8 },
	{ "f9",				KEY_F9 },
	{ "f10",			KEY_F10 },
	{ "f11",			KEY_F11 },
	{ "f12",			KEY_F12 },
	{ "ins",			KEY_INS },
	{ "del",			KEY_DEL },
	{ "pgdn",			KEY_PGDN },
	{ "pgup",			KEY_PGUP },
	{ "home",			KEY_HOME },
	{ "end",			KEY_END },
	{ "pause",			KEY_PAUSE },
	{ "lbutton",		KEY_LBUTTON },
	{ "rbutton",		KEY_RBUTTON },
	{ "mbutton",		KEY_MBUTTON },
	{ "wheelup",		KEY_MWHEELUP },
	{ "wheeldown",		KEY_MWHEELDOWN },
	{ "button4",		KEY_BUTTON4 },
	{ "button5",		KEY_BUTTON5 },
	{ "button6",		KEY_BUTTON6 },
	{ "button7",		KEY_BUTTON7 },
	{ "button8",		KEY_BUTTON8 },
	{ "button9",		KEY_BUTTON9 },
	{ "kp_9",			KEY_KEYPAD_9 },
	{ "kp_8",			KEY_KEYPAD_8 },
	{ "kp_7",			KEY_KEYPAD_7 },
	{ "kp_6",			KEY_KEYPAD_6 },
	{ "kp_5",			KEY_KEYPAD_5 },
	{ "kp_4",			KEY_KEYPAD_4 },
	{ "kp_3",			KEY_KEYPAD_3 },
	{ "kp_2",			KEY_KEYPAD_2 },
	{ "kp_1",			KEY_KEYPAD_1 },
	{ "kp_0",			KEY_KEYPAD_0 },
	{ "kp_del",			KEY_KEYPAD_DEL },
	{ "kp_ins",			KEY_KEYPAD_INS },
	{ "kp_plus",		KEY_KEYPAD_PLUS },
	{ "kp_minus",		KEY_KEYPAD_MINUS },
	{ "kp_slash",		KEY_KEYPAD_SLASH },
	{ "kp_asterisk",	KEY_KEYPAD_ASTERISK },
	{ "kp_enter",		KEY_KEYPAD_ENTER },
	{ "kp_decimal",		KEY_KEYPAD_DECIMAL },
	{ "lwindows",		KEY_LEFT_WINDOWS },
	{ "rwindows",		KEY_RIGHT_WINDOWS },
	{ "menu",			KEY_MENU },
	{ "num_lock",		KEY_NUMLOCK },
	{ "scr_lock",		KEY_SCROLLLOCK },
	{ "caps_lock",		KEY_CAPSLOCK },
	{ "app",			KEY_APP },
	{ "xbutton1",		KEY_XBUTTON1 },
	{ "xbutton2",		KEY_XBUTTON2 },
	{ NULL, 0 }  //terminate list
};

int	Input_GetMouseMode()
{
	return mousemode;
}

void	Input_SetMouseAngles(const vec3_t angles)
{
	mousepos.realpitch = angles[PITCH];
	mousepos.realyaw = angles[YAW];
}

void	Input_UpdateMouse(int x, int y)
{
	mousepos.deltax += (x - mousepos.x);
	mousepos.deltay += (y - mousepos.y);

	mousepos.x = x;
	mousepos.y = y;

	if (mousemode == MOUSE_RECENTER)
	{
		//pitch increases as we rotate up
		mousepos.realpitch += (invertmouse.integer ? 1 : -1) * WORD2ANGLE(mousepos.deltay * (ysensitivity.value * sensitivity.value * sensitivityScale * pitchSpeed.value));
		if (mousepos.realpitch < pitch_low.value)
			mousepos.realpitch = pitch_low.value;
		else if (mousepos.realpitch > pitch_high.value)
			mousepos.realpitch = pitch_high.value;
		inputState.pitch = ANGLE2WORD(mousepos.realpitch);

		//yaw increases as we turn counterclockwise around the up (Z) axis
		mousepos.realyaw -= WORD2ANGLE(mousepos.deltax * (sensitivity.value * sensitivityScale * yawSpeed.value));
		inputState.yaw = ANGLE2WORD(mousepos.realyaw);
	}
	else if (mousemode == MOUSE_FREE_INPUT)
	{
		inputState.yaw = (short)((mousepos.x / (float)Vid_GetScreenW()) * 0x7fff);
		inputState.pitch = (short)((mousepos.y / (float)Vid_GetScreenH()) * 0x7fff);
	}
}

void	Input_SetSensitivityScale(float scale)
{
	sensitivityScale = scale;
}

void	Input_SetMouseXY(int x, int y)
{
	if (input_driver.SetMouseXY(x, y))
	{
		mousepos.x = x;		//ensures delta values will be 0
		mousepos.y = y;
	}
}

void	Input_CenterMouse()
{
	if (Cvar_GetValue("sys_focus") < 1.0)
		return;
	Input_SetMouseXY(Vid_GetScreenW() >> 1, Vid_GetScreenH() >> 1);	
}

//call this once per frame
void	Input_Frame()
{
#ifdef _WIN32
	if (!Vid_IsFullScreen())
	{
		//this is retarded.
		//why must windows behave so irrationally?
		//why does show cursor not just show or hide the damn cursor?!
		if (Input_Win32_CheckCursorInClientArea())
			while(ShowCursor(false) > 0);
		else
			while(ShowCursor(true) < 1);
	}
#endif

	//todo: update key times?
}

void	Input_GetMousePos(mousepos_t *mp)
{
	Mem_Copy(mp, &mousepos, sizeof(mousepos_t));

	if (mousemode == MOUSE_RECENTER)
	{
		Input_CenterMouse();
	}

	mousepos.deltax = 0;
	mousepos.deltay = 0;
}

void	Input_GetMouseXY(int *x, int *y)
{
	*x = mousepos.x;
	*y = mousepos.y;
}


/*==========================

  Input_ActivateBind

 ==========================*/

bool	Input_ActivateBind(int key, bool down)
{
	keybind_t	*bind;

	if (key < 0 || key >= MAX_KEYS)
		return false;

	bind = &binds[currentBindProfile][key];

	switch (bind->type)
	{
	case BIND_NONE:
		return false;


	case BIND_CMD:
		if (down)
		{
			if (bind->cmd[0])
			{
				Cmd_Exec(bind->cmd);
				return true;
			}
		}
		else
		{
			if (bind->upcmd[0])
			{
				Cmd_Exec(bind->upcmd);
				return true;
			}
		}
		return false;


	case BIND_ALIAS:
		if (!bind->alias)
			return false;

		if (down)
		{
			if (aliasList[bind->alias].cmd[0])
			{
				Cmd_Exec(aliasList[bind->alias].cmd);
				return true;
			}
		}
		else
		{
			if (aliasList[bind->alias].upcmd[0])
			{
				Cmd_Exec(aliasList[bind->alias].upcmd);
				return true;
			}
		}
		return false;
	}

	return false;
}

/*==========================

  Input_Event

 ==========================*/

void	Input_Event(int key, int rawchar, bool down)
{
	static bool skip_next_raw = false;

	bool	cl_event_handled = false;
	bool	int_event_handled = false;
	int		i = 0;

	//validate key
	if ((key > MAX_KEYS - 1) || (key < 0))
		return;

	//store the keys current state
	keys[key] = down;

	//check for toggling the console
	while (consolekeys.string[i])
	{
		if (rawchar == (unsigned char)consolekeys.string[i])
		{
			if (down)
			{
				console.active = !console.active;
				skip_next_raw = false;
				return;
			}
			else
			{
				//ignore up events for console keys
				return;
			}
		}
		if (key >= 32 && key < 127 && key == consolekeys.string[i])
		{
			//ignore 'key' param for console keys
			return;
		}
		i++;
	}

	//discards a raw char because we handled it as a bind
	//this is the only thing that I'm a little dissapointed with in the new input code
	//everyhting is pretty straight forward and nice, but since windows is going to be
	//sending two sperate events, first the key code and then the raw character, if we
	//don't discard the next event we get a stray unnecesasary character.
	if (skip_next_raw && !key && rawchar)
	{
		skip_next_raw = false;
		return;
	}

	//if the client is connecting, handle escape specially
	if (key == KEY_ESCAPE && (localClient.cstate > CCS_DISCONNECTED && localClient.cstate < CCS_IN_GAME))
	{
		Cvar_Set("nextSessionCommand", "");
		//hardcoded quit out of connect
		Game_Error("Cancelled connect\n");
		return;
	}

	//when the console is open, all input goes to it
	if (console.active)
	{
		//only send key down events to console
		if (down)
		{
			//we want to use the virtual codes for non-standard stuff,
			//but the raw character output for the alpha/numeric/punctuation
			if (key && (key < 32 || key >= 127))
				Console_Key(key);
			else if (rawchar)
				Console_Key(rawchar);
		}
		return;
	}

	if (down && anykeycommand.string[0] && key >= 32 && key < 127)
	{
		Cmd_BufPrintf("%s\n", anykeycommand.string);
		return;
	}

	if (demo.playing)
	{
		//basic demo controls
		if (key && down)
		{
			switch(key)
			{
				//case KEY_LBUTTON:
				case KEY_ESCAPE:
				//case KEY_SPACE:
				//case KEY_ENTER:
					Cmd_BufPrintf("demostop\n");
					break;
				case KEY_PAUSE:
					demo.paused ^= 1;
					break;
				case KEY_LEFT:
				{
					demo.speed -= 0.1;
					if (demo.speed < 0)
						demo.speed = 0;
					break;
				}
				case KEY_RIGHT:
					demo.speed += 0.1;
					break;
				case KEY_UP:
					demo.speed = 1;
					break;
				default:
					break;
			}			
		}

		return;
	}

	//check for a callback function grabbing the input
	//if the function returns true, we'll exit right away
	//otherwise we'll go on to check binds, etc
	if (input_callback_active && GUI_WidgetWantsKey(key, down))
	{
		//if (input_callback_func(key, (char)(rawchar & 0x7f), down, input_callback_userdata))
		if (input_callback_func(key, (char)rawchar, down, input_callback_userdata))
			return;
	}

	//let the interface and client try to handle it	
	if (int_api.InputEvent)		//make sure the pointer is valid because occasionally we get input events before the game DLL has been initialized
		//int_event_handled = int_api.InputEvent(key, (char)(rawchar & 0x7f), down);
		int_event_handled = int_api.InputEvent(key, (char)rawchar, down);

	if (localClient.cstate == CCS_IN_GAME)			
		//cl_event_handled = cl_api.InputEvent(key, (char)(rawchar & 0x7f), down);
		cl_event_handled = cl_api.InputEvent(key, (char)rawchar, down);
	
	if (int_event_handled || cl_event_handled)
		return;

	//check binds, but don't bother doing this for the raw chars
	if (key && (lc.cstate == CCS_IN_GAME || DLLTYPE == DLLTYPE_EDITOR))
	{
		bool executed_bind = false;

		if (Input_ActivateBind(key, down))
			executed_bind = true;

		if (executed_bind && key >= 32 && key < 127)
			skip_next_raw = true;
	}
}

bool	Input_IsKeyDown(int key)
{
	if (key>MAX_KEYS || key<0) return false;

	return keys[key];
}

bool	Input_ActivateInputCallback(inputcallback_t func, bool binds_active, void *widget)
{
	Input_DeactivateInputCallback(true);

	input_callback_active = true;
	input_callback_func = func;
	input_callback_userdata = widget;

	return true;
}

bool	Input_DeactivateInputCallback(bool binds_active)
{
	static bool recursion = false;

	if (recursion)
		return false;

	if (!input_callback_active)
		return true;
			
	if (input_callback_userdata)
	{
		gui_element_t *widg = input_callback_userdata;

		if (widg->loseInput)
		{
			recursion = true;
			widg->loseInput(widg);
			recursion = false;
		}
	}

	input_callback_active = false;
	input_callback_func = NULL;
	input_callback_userdata = NULL;
	
	return true;
}

void	*Input_GetCallbackWidget()
{
	if (!input_callback_active)
		return NULL;

	return input_callback_userdata;
}

void	Input_SetMouseMode(int mode)
{
	mousemode = mode;
	if (mode==MOUSE_RECENTER) 
	{
		Input_CenterMouse();
		mousepos.realpitch = 0;
	}
	GUI_ResetButtons();
}


/*==========================

  Input_GetKeyFromName

 ==========================*/

int		Input_GetKeyFromName(char *keyname)
{
	int n;

	//first determine if key is alphanumeric
	if (strlen(keyname)==1)
		return tolower(keyname[0]);

	for (n=0;;n++)
	{
		if (!keydescs[n].desc)
			return -1;
		if (stricmp(keyname, keydescs[n].desc)==0)
			return keydescs[n].num;
	}

	return -1;
}


/*==========================

  Input_GetBindDownCmd

 ==========================*/

char	*Input_GetBindDownCmd(int key)
{
	return binds[currentBindProfile][key].cmd;
}


/*==========================

  Input_GetBindUpCmd

 ==========================*/

char	*Input_GetBindUpCmd(int key)
{
	return binds[currentBindProfile][key].upcmd;
}


/*==========================

  Input_GetNameFromKey

 ==========================*/

char	*Input_GetNameFromKey(int key)
{
	int n;

	//check the name list
	for (n = 0; ; n++)
	{
		if (!keydescs[n].desc) break;

		if (key==keydescs[n].num)
			return keydescs[n].desc;		
	}

	//low and high ascii chars are not valid binds
	if (key < 32 || key >= 127)
		return fmt("%c", 0);

	//key binds are not case sensitive
	if (key >= 'A' && key <= 'Z')
		key += ('a' - 'A');

	return fmt("%c", key);
}


/*==========================

  Input_FindAlias

 ==========================*/

alias_t	*Input_FindAlias(const char *name)
{
	int index;

	for (index = 1; index < MAX_ALIASES; index++)
	{
		if (!stricmp(aliasList[index].name, name))
			return &aliasList[index];
	}

	return NULL;
}


/*==========================

  Input_AddKeyToAlias

 ==========================*/

void	Input_AddKeyToAlias(alias_t *alias, int key)
{
	int index;

	if (!alias)
		return;

	for (index = 0; index < ALIAS_MAX_BINDS; index++)
	{
		if (alias->keys[index] == key)
			return;

		if (!alias->keys[index])
		{
			alias->keys[index] = key;
			return;
		}
	}

	Console_Printf("Warning: Too many keys bound to alias '%s'\n", alias->name);
}


/*==========================

  Input_FindFreeAlias

  Returns a pointer to the first alias is aliasList not being used

 ==========================*/

alias_t	*Input_FindFreeAlias()
{
	int	index;

	for (index = 1; index < MAX_ALIASES; index++)
	{
		if (!aliasList[index].name[0])
			return &aliasList[index];
	}

	return NULL;
}


/*==========================

  Input_RemoveKeyFromAliases

  Removes a key from any alias that thinks it owns this key

 ==========================*/

void	Input_RemoveKeyFromAliases(int key)
{
	int	index, keys;

	for (index = 0; index < MAX_ALIASES; index++)
	{
		for (keys = 0; keys < ALIAS_MAX_BINDS; keys++)
		{
			if (aliasList[index].keys[keys] == key)
				aliasList[index].keys[keys] = 0;
		}
	}
}


/*==========================

  Input_PrintBindInfo

 ==========================*/

void	Input_PrintBindInfo(int profile, int key)
{
	keybind_t	*bind;

	if (key < 0 || key >= MAX_KEYS)
		return;

	bind = &binds[profile][key];

	switch (bind->type)
	{
	case BIND_NONE:
		Console_Printf("[%i] %s: <not bound>\n", profile, Input_GetNameFromKey(key));
		return;

	case BIND_CMD:
		if (bind->cmd[0])
			Console_Printf("[%i] %s (down): %s\n", profile, Input_GetNameFromKey(key), bind->cmd);
		if (bind->upcmd[0])
			Console_Printf("[%i] %s (up): %s\n", profile, Input_GetNameFromKey(key), bind->upcmd);
		return;

	case BIND_ALIAS:
		if (bind->alias)
			Console_Printf("[%i] %s (alias): %s\n", profile, Input_GetNameFromKey(key), aliasList[bind->alias].name);
		return;
	}
}


/*==========================

  Input_Bind_Cmd

 ==========================*/

void	Input_Bind_Cmd(int argc, char *argv[])
{
	char	bind[CMD_MAX_LENGTH] = "";
	int		key;

	if (argc < 1)
	{
		Console_Printf("syntax: bind [key] [command]\n");
		return;
	}

	key = Input_GetKeyFromName(argv[0]);
	if (key == -1)
	{
		Console_Printf("%s is not a valid key name\n", argv[0]);
		return;
	}

	if (argc < 2)
	{
		Console_Printf("Current active profile: %i\n", currentBindProfile);
		Input_PrintBindInfo(0, key);
		Input_PrintBindInfo(1, key);
		return;
	}

	binds[currentBindProfile][key].type = BIND_CMD;

	ConcatArgs(&argv[1], argc - 1, bind);
	memset(binds[currentBindProfile][key].cmd, 0, BIND_MAX_CMD_LEN);
	strncpy(binds[currentBindProfile][key].cmd, bind, BIND_MAX_CMD_LEN - 1);
	Input_RemoveKeyFromAliases(key);
}


/*==========================

  Input_BindUp_Cmd

 ==========================*/

void	Input_BindUp_Cmd(int argc, char *argv[])
{
	char	bind[CMD_MAX_LENGTH] = "";
	int		key;

	if (argc < 1)
	{
		Console_Printf("syntax: bindup [key] [command]\n");
		return;
	}

	key = Input_GetKeyFromName(argv[0]);
	if (key == -1)
	{
		Console_Printf("%s is not a valid key name\n", argv[0]);
		return;
	}

	if (argc < 2)
	{
		Console_Printf("Current active profile: %i\n", currentBindProfile);
		Input_PrintBindInfo(0, key);
		Input_PrintBindInfo(1, key);
		return;
	}

	binds[currentBindProfile][key].type = BIND_CMD;

	ConcatArgs(&argv[1], argc - 1, bind);
	memset(binds[currentBindProfile][key].upcmd, 0, BIND_MAX_CMD_LEN);
	strncpy(binds[currentBindProfile][key].upcmd, bind, BIND_MAX_CMD_LEN - 1);
	Input_RemoveKeyFromAliases(key);
}

/*==========================

  Input_Unbind_Cmd

 ==========================*/

void	Input_Unbind_Cmd(int argc, char *argv[])
{
	int key;

	if (argc != 1)
	{
		Console_Printf("syntax: unbind [key]\n");
		return;
	}

	key = Input_GetKeyFromName(argv[0]);
	if (key == -1)
	{
		Console_Printf("%s is not a valid key name\n", argv[0]);
		return;
	}

	binds[currentBindProfile][key].type = BIND_NONE;
	binds[currentBindProfile][key].alias = 0;
	memset(binds[currentBindProfile][key].cmd, 0, BIND_MAX_CMD_LEN);
	memset(binds[currentBindProfile][key].upcmd, 0, BIND_MAX_CMD_LEN);
	Input_RemoveKeyFromAliases(key);
}


void	Input_UnbindAll_Cmd(int argc, char *argv[])
{
	memset(&binds, 0, sizeof(binds));
}


/*==========================

  Input_BindButton_Cmd

 ==========================*/

void	Input_BindButton_Cmd(int argc, char *argv[])
{
	int key;

	if (argc < 2)
	{
		Console_Printf("syntax: bindbutton [key] [var]\n");
		return;
	}

	key = Input_GetKeyFromName(argv[0]);
	if (key == -1)
	{
		Console_Printf("%s is not a valid key name\n", argv[0]);
		return;
	}

	binds[currentBindProfile][key].type = BIND_CMD;

	memset(binds[currentBindProfile][key].cmd, 0, BIND_MAX_CMD_LEN);
	strncpy(binds[currentBindProfile][key].cmd, fmt("%s 1", argv[1]), BIND_MAX_CMD_LEN - 1);
	memset(binds[currentBindProfile][key].upcmd, 0, BIND_MAX_CMD_LEN);
	strncpy(binds[currentBindProfile][key].upcmd, fmt("%s 0", argv[1]), BIND_MAX_CMD_LEN - 1);
	Input_RemoveKeyFromAliases(key);
}


/*==========================

  Input_BindAlias_Cmd

 ==========================*/

void	Input_BindAlias_Cmd(int argc, char *argv[])
{
	int		key;
	alias_t	*alias;

	if (argc != 2)
	{
		Console_Printf("Usage:\nbindAlias <key name> <alias name>\n");
		return;
	}

	key = Input_GetKeyFromName(argv[0]);
	if (key == -1)
	{
		Console_Printf("%s is not a valid key name\n", argv[0]);
		return;
	}

	alias = Input_FindAlias(argv[1]);
	if (!alias)
	{
		Console_Printf("No such alias '%s'\n", argv[1]);
		return;
	}

	binds[currentBindProfile][key].type = BIND_ALIAS;
	binds[currentBindProfile][key].alias = alias->index;
	Input_RemoveKeyFromAliases(key);
	Input_AddKeyToAlias(alias, key);
}


/*==========================

  Input_Alias_Cmd

 ==========================*/

void	Input_Alias_Cmd(int argc, char *argv[])
{
	alias_t	*alias;
	char	cmd[CMD_MAX_LENGTH];

	if (argc < 2)
	{
		Console_Printf("Usage:\nalias <alias name> <command>\n");
		return;
	}

	alias = Input_FindAlias(argv[0]);
	if (!alias)
	{
		alias = Input_FindFreeAlias();
		if (!alias)
		{
			Console_Printf("Could not add another alias, there are already too many\n");
			return;
		}
		Console_Printf("Creating new alias '%s'\n", argv[0]);
		memset(alias->name, 0, ALIAS_MAX_NAME_LEN);
		strncpy(alias->name, argv[0], ALIAS_MAX_NAME_LEN - 1);
	}

	ConcatArgs(&argv[1], argc - 1, cmd);
	memset(alias->cmd, 0, BIND_MAX_CMD_LEN);
	strncpy(alias->cmd, cmd, BIND_MAX_CMD_LEN - 1);
}


/*==========================

  Input_AliasUp_Cmd

 ==========================*/

void	Input_AliasUp_Cmd(int argc, char *argv[])
{
	alias_t	*alias;
	char	cmd[CMD_MAX_LENGTH];

	if (argc < 2)
	{
		Console_Printf("Usage:\naliasUp <alias name> <command>\n");
		return;
	}

	alias = Input_FindAlias(argv[0]);
	if (!alias)
	{
		alias = Input_FindFreeAlias();
		if (!alias)
		{
			Console_Printf("Could not add another alias, there are already too many\n");
			return;
		}
		Console_Printf("Creating new alias '%s'\n", argv[0]);
		memset(alias->name, 0, ALIAS_MAX_NAME_LEN);
		strncpy(alias->name, argv[0], ALIAS_MAX_NAME_LEN - 1);
	}

	ConcatArgs(&argv[1], argc - 1, cmd);
	memset(alias->upcmd, 0, BIND_MAX_CMD_LEN);
	strncpy(alias->upcmd, cmd, BIND_MAX_CMD_LEN - 1);
}


/*==========================

  Input_Unalias_Cmd

 ==========================*/

void	Input_Unalias_Cmd(int argc, char *argv[])
{
	alias_t	*alias;

	if (argc != 1)
	{
		Console_Printf("Usage: unalias <alias name>\n");
		return;
	}

	alias = Input_FindAlias(argv[0]);
	if (!alias)
	{
		Console_Printf("Could not find an alias names '%s'\n", argv[0]);
		return;
	}

	memset(alias, 0, sizeof(alias_t));
}


/*==========================

  Input_PutAliasKeys_Cmd

  Sets a cvar to be a string of the names of all the keys that
  are bound to this alias.

 ==========================*/

void	Input_PutAliasKeys_Cmd(int argc, char *argv[])
{
	int		index;
	alias_t	*alias;
	cvar_t	*cvar;
	char	string[ALIAS_MAX_BINDS * 16];
	bool	firstkey = true;

	if (argc != 2)
	{
		Console_Printf("Usage: putAliasKeys <alias name> <cvar>\n");
		return;
	}

	alias = Input_FindAlias(argv[0]);
	if (!alias)
	{
		Console_Printf("No such alias '%s'\n", argv[0]);
		return;
	}

	cvar = Cvar_Find(argv[1]);
	if (!cvar)
	{
		Console_Printf("No such cvar '%s'\n", argv[1]);
		return;
	}

	memset(string, 0, ALIAS_MAX_BINDS * 16);
	for (index = 0; index < ALIAS_MAX_BINDS; index++)
	{
		char	*keyname = Input_GetNameFromKey(alias->keys[index]);

		if (*keyname)
		{
			if (!firstkey)
				strcat(string, ", ");
			strncat(string, keyname, 14);
			firstkey = false;
		}
	}
	Cvar_SetVar(cvar, string);
}


/*==========================

  Input_ListAliases_Cmd

 ==========================*/

void	Input_ListAliases_Cmd(int argc, char *argv[])
{
	int index;

	for (index = 0; index < MAX_ALIASES; index++)
	{
		if (!aliasList[index].name[0])
			continue;

		if (aliasList[index].cmd[0])
			Console_Printf("%s <down>: %s\n", aliasList[index].name, aliasList[index].cmd);
		if (aliasList[index].upcmd[0])
			Console_Printf("%s <up>: %s\n", aliasList[index].name, aliasList[index].upcmd);
	}
}


/*==========================

  Input_ListBinds_Cmd

 ==========================*/

void	Input_ListBinds_Cmd(int argc, char *argv[])
{
	int n;

	Console_Printf("Current active profile: %i\n", currentBindProfile);
	for (n = 0; n < MAX_KEYS; n++)
	{
		char	*keyname = Input_GetNameFromKey(n);

		//uppercase and lowercase refer to the same keys, so skip the uppercase alphabet
		if (n == 'A')
			n = 'Z' + 1;

		if (!(*keyname))
			continue;

		Input_PrintBindInfo(currentBindProfile, n);
	}
}


/*==========================

  Input_ListKeys_Cmd

 ==========================*/

void	Input_ListKeys_Cmd(int argc, char *argv[])
{
	int n;

	Console_Printf("Extended key names:\n\n");

	for (n = 0; n < MAX_KEYS; n++)
	{
		if (!keydescs[n].desc) 
			return;
		Console_Printf("%s\n", keydescs[n].desc);
	}
}


/*==========================

  Input_WriteBindingsToZip

 ==========================*/

void Input_WriteBindingsToZip(void *zipfile)
{
	int i, n;
	char *keyname;
	char line[256];

	if (!zipfile) return;

	//write the aliases
	for (i = 0; i < MAX_ALIASES; i++)
	{
		if (!aliasList[i].name[0])
			break;

		if (aliasList[i].cmd[0])
		{
			BPrintf(line, 256, "alias %s \"%s\"\n", aliasList[i].name, aliasList[i].cmd);
			ZIPW_WriteFileInZip(zipfile, line, strlen(line));
		}
		if (aliasList[i].upcmd[0])
		{
			BPrintf(line, 256, "aliasup %s \"%s\"\n", aliasList[i].name, aliasList[i].upcmd);
			ZIPW_WriteFileInZip(zipfile, line, strlen(line));
		}
	}

	//write the binds
	for (i = 0; i < MAX_BIND_PROFILES; i++)
	{
		BPrintf(line, 256, "setBindProfile %i\n", i);
		ZIPW_WriteFileInZip(zipfile, line, strlen(line));

		for (n = 0; n < 256; n++)
		{
			keyname = Input_GetNameFromKey(n);
			if (!(*keyname))
				continue;

			switch (binds[i][n].type)
			{
			case BIND_NONE:
				break;

			case BIND_CMD:
				BPrintf(line, 256, "bind %s \"%s\"\n", keyname, binds[i][n]);
				ZIPW_WriteFileInZip(zipfile, line, strlen(line));
				BPrintf(line, 256, "bindup %s \"%s\"\n", keyname, binds[i][n]);
				ZIPW_WriteFileInZip(zipfile, line, strlen(line));
				break;

			case BIND_ALIAS:
				BPrintf(line, 256, "bind %s %s\n", keyname, aliasList[binds[i][n].alias].name);
				ZIPW_WriteFileInZip(zipfile, line, strlen(line));
				break;
			}
		}
	}

	BPrintf(line, 256, "setBindProfile 0\n");
	ZIPW_WriteFileInZip(zipfile, line, strlen(line));
}


/*==========================

  Input_WriteBindings

 ==========================*/

void Input_WriteBindings(file_t *f)
{
	int i, n;
	char *keyname;

	if (!f) return;

	//write the aliases
	for (i = 1; i < MAX_ALIASES; i++)
	{
		if (!aliasList[i].name[0])
			break;

		if (aliasList[i].cmd[0])
			File_Printf(f, "alias %s \"%s\"\n", aliasList[i].name, aliasList[i].cmd);
		if (aliasList[i].upcmd[0])
			File_Printf(f, "aliasup %s \"%s\"\n", aliasList[i].name, aliasList[i].cmd);
	}

	//write the bindings
	for (i = 0; i < MAX_BIND_PROFILES; i++)
	{
		File_Printf(f, "setBindProfile %i\n", i);

		for (n = 0; n < MAX_KEYS; n++)
		{
			keyname = Input_GetNameFromKey(n);
			if (!(*keyname))
				continue;

			switch(binds[i][n].type)
			{
			case BIND_NONE:
				break;

			case BIND_CMD:
				File_Printf(f, "bind %s \"%s\"\n", keyname, binds[i][n].cmd);
				File_Printf(f, "bindup %s \"%s\"\n", keyname, binds[i][n].upcmd);
				break;

			case BIND_ALIAS:
				File_Printf(f, "bindalias %s %s\n", keyname, aliasList[binds[i][n].alias].name);
				break;
			}
		}
	}

	File_Printf(f, "setBindProfile 0\n");
}


/*==========================

  Input_GetInputState

 ==========================*/

void	Input_GetInputState(inputState_t *is)
{
	if (!is)
		return;

	is->movement = 0;
	is->buttons = 0;

	is->pitch = inputState.pitch;
	is->yaw = inputState.yaw;

	if (mousemode == 0)
		return;

	if (move_forward.integer)
		is->movement |= MOVE_FORWARD;
	if (move_backward.integer)
		is->movement|= MOVE_BACKWARD;
	if (move_left.integer)
		is->movement |= MOVE_LEFT;
	if (move_right.integer)
		is->movement |= MOVE_RIGHT;
	if (move_up.integer)
		is->movement |= MOVE_UP;
	if (move_down.integer)
		is->movement |= MOVE_DOWN;
	if (move_zig.integer)
		is->movement |= MOVE_ZIG;
	if (move_zag.integer)
		is->movement |= MOVE_ZAG;
	is->buttons = Input_GetButtons();

	if (item.integer < 0)
		Cvar_SetVarValue(&item, 255);
	else if (item.integer > 255)
		Cvar_SetVarValue(&item, 0);
	is->item = item.integer;

//	mousepos.deltax = 0;
//	mousepos.deltay = 0;

	if (mousemode == MOUSE_RECENTER)
		Input_CenterMouse();
}


/*==========================

  Input_GetButtons

 ==========================*/

int		Input_GetButtons()
{
	int buttons = 0;

	if (button1.integer)
		buttons |= BUTTON1;
	if (button2.integer)
		buttons |= BUTTON2;
	if (button3.integer)
		buttons |= BUTTON3;
	if (button4.integer)
		buttons |= BUTTON4;
	if (button5.integer)
		buttons |= BUTTON5;
	if (button6.integer)
		buttons |= BUTTON6;
	if (button7.integer)
		buttons |= BUTTON7;
	if (button8.integer)
		buttons |= BUTTON8;

	return buttons;
}


/*==========================

  Input_SetBindProfile_Cmd

 ==========================*/

void	Input_SetBindProfile_Cmd(int argc, char *argv[])
{
	int i;

	if (argc < 1)
	{
		Console_Printf("Current bind profile: %i\n", currentBindProfile);
		return;
	}

	i = atoi(argv[0]);
	if (i >= 0 && i < MAX_BIND_PROFILES)
		currentBindProfile = i;
}


void	Input_ClearKeyStates()
{
	memset(keys, 0, sizeof(keys));

	Cvar_SetVarValue(&move_backward, 0);
	Cvar_SetVarValue(&move_forward, 0);
	Cvar_SetVarValue(&move_left, 0);
	Cvar_SetVarValue(&move_right, 0);
	Cvar_SetVarValue(&move_up,	0);
	Cvar_SetVarValue(&move_down, 0);
	Cvar_SetVarValue(&move_zig, 0);
	Cvar_SetVarValue(&move_zag, 0);
	Cvar_SetVarValue(&button1, 0);
	Cvar_SetVarValue(&button2, 0);
	Cvar_SetVarValue(&button3, 0);
	Cvar_SetVarValue(&button4, 0);
	Cvar_SetVarValue(&button5, 0);
	Cvar_SetVarValue(&button6, 0);
	Cvar_SetVarValue(&button7, 0);
	Cvar_SetVarValue(&button8, 0);
}

/*==========================

  Input_Init

 ==========================*/

void	Input_Init()
{
	int index;

	input_callback_active = false;

	memset(keys, 0, sizeof(keys));
	memset(binds, 0, sizeof(binds));
	memset(aliasList, 0, sizeof(aliasList));
	for (index = 0; index < MAX_ALIASES; index++)
		aliasList[index].index = index;
	memset(&inputState, 0, sizeof(inputState_t));

	Cmd_Register("bind",			Input_Bind_Cmd);
	Cmd_Register("bindUp",			Input_BindUp_Cmd);
	Cmd_Register("bindButton",		Input_BindButton_Cmd);
	Cmd_Register("bindAlias",		Input_BindAlias_Cmd);
	Cmd_Register("unbind",			Input_Unbind_Cmd);
	Cmd_Register("unbindAll",		Input_UnbindAll_Cmd);
	Cmd_Register("alias",			Input_Alias_Cmd);
	Cmd_Register("aliasUp",			Input_AliasUp_Cmd);
	Cmd_Register("unalias",			Input_Unalias_Cmd);
	Cmd_Register("listKeys",		Input_ListKeys_Cmd);
	Cmd_Register("listBinds",		Input_ListBinds_Cmd);
	Cmd_Register("listAliases",		Input_ListAliases_Cmd);
	Cmd_Register("setBindProfile",	Input_SetBindProfile_Cmd);
	Cmd_Register("putAliasKeys",	Input_PutAliasKeys_Cmd);

	//movement variables.  these are toggles that are used to fill up the inputState
	Cvar_Register(&move_forward);
	Cvar_Register(&move_backward);
	Cvar_Register(&move_left);
	Cvar_Register(&move_right);
	Cvar_Register(&move_up);
	Cvar_Register(&move_down);
	Cvar_Register(&move_zig);
	Cvar_Register(&move_zag);
	Cvar_Register(&button1);
	Cvar_Register(&button2);
	Cvar_Register(&button3);
	Cvar_Register(&button4);
	Cvar_Register(&button5);
	Cvar_Register(&button6);
	Cvar_Register(&button7);
	Cvar_Register(&button8);
	Cvar_Register(&yawSpeed);
	Cvar_Register(&pitchSpeed);
	Cvar_Register(&sensitivity);
	Cvar_Register(&ysensitivity);
	Cvar_Register(&invertmouse);
	Cvar_Register(&pitch_low);
	Cvar_Register(&pitch_high);
	Cvar_Register(&item);
	Cvar_Register(&consolekeys);
	Cvar_Register(&anykeycommand);

	Input_SetMouseMode(MOUSE_FREE);
}

void	Input_Shutdown()
{
}

void	Input_RegisterVars()
{
}
