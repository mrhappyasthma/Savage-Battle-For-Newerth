// (C) 2003 S2 Games

// console.h

// Functions for adding text to the console buffer

#define		CONSOLE_CMD_SIZE	256
#define		CONSOLE_BUF_SIZE	65536
#define		CONSOLE_FONTWIDTH	8
#define		CONSOLE_FONTHEIGHT	12
#define		CONSOLE_SPACING_X	0
#define		CONSOLE_SPACING_Y	0
#define		CONSOLE_MAX_LINE_LENGTH	200

//fixme
#define		Console_Notify Console_Printf

typedef struct
{
	bool	active;
	bool	initialized;
	bool	enabled;

	char	buf[CONSOLE_BUF_SIZE];
	char	cmdbuf[CONSOLE_CMD_SIZE];
	int		lines;
	int		cursor;
	int		linelength;
	int		xpos;
	int		ypos;
	int		backscroll;
} console_t;

extern console_t console;

void	Console_SetOutputCallback(void (*callback)(char *string));

//normal messages
void	Console_Printf(const char *fmt, ...);
//debug messages
void	Console_DPrintf(const char *fmt, ...);
//error messages
void	Console_Errorf(const char *fmt, ...);
void	Console_Draw();
void	Console_Key(int key);
void	Console_Init();
void	Console_Format();
void	Console_EnablePrinting();
void	Console_DisablePrinting();

//cvars

extern	cvar_t	con_logtodisk;
