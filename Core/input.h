// (C) 2003 S2 Games

// input.h

// mouse/keyboard input routines


#define		KEY_TAB			9
#define		KEY_RETURN		10
#define		KEY_ENTER		13
#define		KEY_ESCAPE		27
#define		KEY_SPACE		32
#define		KEY_BACKSPACE	127
#define		KEY_UP			128
#define		KEY_LEFT		129
#define		KEY_RIGHT		130
#define		KEY_DOWN		131
#define		KEY_ALT			132
#define		KEY_CTRL		133
#define		KEY_SHIFT		134
#define		KEY_F1			135
#define		KEY_F2			136
#define		KEY_F3			137
#define		KEY_F4			138
#define		KEY_F5			139
#define		KEY_F6			140
#define		KEY_F7			141
#define		KEY_F8			142
#define		KEY_F9			143
#define		KEY_F10			144
#define		KEY_F11			145
#define		KEY_F12			146
#define		KEY_INS			147
#define		KEY_DEL			148
#define		KEY_PGDN		149
#define		KEY_PGUP		150
#define		KEY_HOME		151
#define		KEY_END			152
#define		KEY_PAUSE		153

#define		KEY_LBUTTON		200
#define		KEY_MOUSEFIRSTBUTTON	KEY_LBUTTON
#define		KEY_RBUTTON		201
#define		KEY_MBUTTON		202
#define		KEY_MWHEELUP	203
#define		KEY_MWHEELDOWN	204
#define		KEY_BUTTON4		205
#define		KEY_BUTTON5		206
#define		KEY_BUTTON6		207
#define		KEY_BUTTON7		208
#define		KEY_BUTTON8		209
#define		KEY_BUTTON9		210
#define		KEY_MOUSELASTBUTTON		KEY_BUTTON9
#define		KEY_KEYPAD_0 	211
#define		KEY_KEYPAD_1 	212
#define		KEY_KEYPAD_2 	213
#define		KEY_KEYPAD_3 	214
#define		KEY_KEYPAD_4 	215
#define		KEY_KEYPAD_5 	216
#define		KEY_KEYPAD_6 	217 
#define		KEY_KEYPAD_7 	218
#define		KEY_KEYPAD_8 	219
#define		KEY_KEYPAD_9 	220
#define		KEY_KEYPAD_INS	221
#define		KEY_KEYPAD_DEL	222
#define		KEY_KEYPAD_PLUS	223
#define		KEY_KEYPAD_MINUS	224
#define		KEY_KEYPAD_ENTER	225
#define		KEY_KEYPAD_SLASH	226
#define		KEY_KEYPAD_ASTERISK	227
#define		KEY_LEFT_WINDOWS 228
#define		KEY_RIGHT_WINDOWS 229
#define		KEY_MENU 		230
#define		KEY_KEYPAD_DECIMAL	231
#define		KEY_NUMLOCK		232
#define		KEY_SCROLLLOCK	233
#define		KEY_CAPSLOCK	234
#define		KEY_APP			235
#define		KEY_XBUTTON1	236
#define		KEY_XBUTTON2	237

/*
 * erm, I found these in savage_types too, 
 * so I'm commenting these out until we're sure where they go
#define		MOUSE_FREE		0
#define		MOUSE_RECENTER	1
*/

typedef struct
{
	char *drivername;

	bool	(*SetMouseXY)(int x, int y);
} input_driver_t;

extern input_driver_t input_driver;

//initializes input devices
void	Input_Init();
void	Input_Frame();
void	Input_Shutdown();
void	Input_RegisterVars();

void	Input_SetSensitivityScale(float scale);

//sets mouse position.  mouse buttons are handled by calling
//Input_Event()
void	Input_UpdateMouse(int x, int y);

void	Input_SetMouseMode(int mode);
int		Input_GetMouseMode();

//sets the mouse position to the center of the screen
void	Input_CenterMouse();
void	Input_SetMouseXY(int x, int y);
void	Input_SetMouseAngles(const vec3_t angles);

//gets current mouse position
void	Input_GetMousePos(mousepos_t *mp);

#ifdef _WIN32
bool	Input_Win32_CheckCursorInClientArea(void);
#endif //_WIN32

bool	Input_IsKeyDown(int key);

//used for keyboard and also mouse buttons
void	Input_Event(int key, int rawchar, bool down);

void	Input_Clear();

void	Input_WriteBindings(file_t *f);
void 	Input_WriteBindingsToZip(void *zipfile);

bool	Input_ActivateInputCallback(inputcallback_t func, bool binds_active, void *widget);
bool	Input_DeactivateInputCallback(bool binds_active);
void	*Input_GetCallbackWidget();

void	Input_GetInputState(inputState_t *is);
int     Input_GetButtons();

char    *Input_GetNameFromKey(int key);
int     Input_GetKeyFromName(char *keyname);
char	*Input_GetBindDownCmd(int key);
char	*Input_GetBindUpCmd(int key);

void	Input_ClearKeyStates();