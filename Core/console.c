// (C) 2003 S2 Games

// console.c

// Console functions

#include "core.h"
#include "../gui/gui_textbuffer.h"

console_t	console = { 0, 0, 0, {0}, {0}, 0, 0, 0, 0, 0, 0 };

cvar_t con_logtodisk =		{ "con_logtodisk",		"1" };
cvar_t con_developer =		{ "con_developer",		"0" };
cvar_t con_cursorspeed =	{ "con_cursorspeed",	"4",	CVAR_SAVECONFIG };
cvar_t con_fontwidth =		{ "con_fontwidth",		"10",	CVAR_SAVECONFIG };
cvar_t con_fontheight =		{ "con_fontheight",		"16",	CVAR_SAVECONFIG };
cvar_t con_alpha =			{ "con_alpha",			"1",	CVAR_SAVECONFIG };
cvar_t con_fadespeed =		{ "con_fadespeed",		"9999" };
cvar_t con_margin =			{ "con_margin",			"3" };
cvar_t con_overlay =		{ "con_overlay",		"0" };
cvar_t con_bufferfile =		{ "con_bufferfile",		"1" };
cvar_t con_echoClient =		{ "con_echoClient",		"-1" };

#define		CMDHIST_SIZE	50

char	cmdhist[CMDHIST_SIZE][CONSOLE_CMD_SIZE];
int		cmdhist_pos;
int		cmdhist_cursor;

extern cvar_t gfx;

void (*outputCallback)(char *string);

void	Console_SetOutputCallback(void (*callback)(char *string))
{
	outputCallback = callback;
}

void	Console_LineFeed()
{
	console.xpos = 0;
	console.ypos++;
	if (!console.lines)
		console.lines = 10;
	memset(&console.buf[((console.ypos) % console.lines) * console.linelength], 0, console.linelength);
}

void	Console_EnablePrinting()
{
	console.enabled = true;
}

void	Console_DisablePrinting()
{
	console.enabled = false;
}

void	Console_Printf(const char *fmtline, ...)
{
	char s[4096] = {0};
	va_list argptr;
	int n = 0, i = 0, wordlength = 0;	

	if (!console.enabled)
		return;

	fmtline = _(fmtline);
	
	va_start(argptr, fmtline);
	vsprintf(s, fmtline, argptr);
	va_end(argptr);	

	if (con_logtodisk.value && DEBUG_LOG_FILE)
	{
		File_Printf(DEBUG_LOG_FILE, "%s", s);
		if (!con_bufferfile.integer)
			File_Flush(DEBUG_LOG_FILE);
	}

	//if an additional output function is set, send the text there as well
	//this is used for echoing the console to remote admins in game and via TCP
	if (outputCallback)
	{
		outputCallback(s);
	}
	//otherwise, see if a specific client has been picked to receive console output
	//and send it to them
	else if (con_echoClient.integer >= 0 && con_echoClient.integer < MAX_CLIENTS)
	{
		if (localServer.clients[con_echoClient.integer].active)
			Server_SendOutputToClient(s);
	}

	if (!gfx.integer || !console.initialized)
	{
		System_Printf("%s", s);
		return;
	}	
		
	for (n=0; n<(int)strlen(s); n++)
	{				
		for (wordlength=0; (wordlength<console.linelength && s[n+wordlength]>' '); wordlength++);
	
		if (console.xpos + wordlength > console.linelength-1)
		{
			for (i=console.xpos; i<console.linelength; i++)
			{
				console.buf[(console.ypos % console.lines) * console.linelength + i] = s[n++];
			}
			n--;
			Console_LineFeed();
			continue;
		}

		console.buf[(console.ypos % console.lines) * console.linelength + console.xpos] = s[n];
		console.xpos++;

		if ((console.xpos>=console.linelength) || (s[n] == '\n'))					
			Console_LineFeed();
	}
}

void	Console_DPrintf(const char *fmt, ...)
{
	va_list	argptr;
	char text[4096];

	fmt = _(fmt);
	
	va_start(argptr, fmt);
	vsprintf(text, fmt, argptr);
	va_end(argptr);

	if (con_developer.value && console.enabled)
		Console_Printf("%s", text);
}

void	Console_Errorf(const char *fmt, ...)
{
	va_list	argptr;
	char text[4096];

	fmt = _(fmt);
	
	va_start(argptr, fmt);
	vsprintf(text, fmt, argptr);
	va_end(argptr);

	if (console.enabled)
		Console_Printf("%s", text);
	
	if (int_api.ErrorDialog)
		int_api.ErrorDialog(text);	
}


static vec4_t console_color = {0.2, 0.2, 0.2, 1};
static vec4_t console_textcolor = {1,1,1,1.0};
static vec4_t console_inputcolor = {1,1,1,1};

void	Console_DrawBuffer(int lines)
{
	int y, n, row;
	int inc = 0;
	char *txt;
	
	if (!console.initialized)
		return;

	n = 0;

	if (console.buf[(console.ypos % console.lines) * console.linelength])
		inc = 1;

	if (lines >= console.lines) lines = console.lines-1;
	for (y=console.ypos-lines+1+inc; y<=console.ypos-1+inc; y++)
	{
		Draw_SetColor(console_textcolor);
		row = y - console.backscroll;
		if (row<0) row=0;
		txt = &console.buf[(row % console.lines) * console.linelength];		
		DU_DrawStringMonospaced(con_fontwidth.value + CONSOLE_SPACING_X + con_margin.value,
								n * (con_fontheight.value + CONSOLE_SPACING_Y),
								txt,
								con_fontwidth.integer,
								con_fontheight.integer,
								1,
								console.linelength,									
								hostmedia.sysfontShader);			
		
		n++;
	}
}

void	Console_DrawBackground(int lines)
{
	if (!console.active)
		return;

	if (lines<=0) return;
	Draw_SetColor(console_color);
	Draw_Quad2d(0,0,Vid_GetScreenW(),(con_fontheight.value+CONSOLE_SPACING_Y)*(lines),0,0,1,1,hostmedia.consoleShader);
}

//draws an input line for typing commands
void	Console_DrawInput(int lines)
{
	char txt[CONSOLE_CMD_SIZE+1];
	int	pos;
	bool drawcursor;

	if (!console.active)
		return;

	memset(txt, '\0', CONSOLE_CMD_SIZE);
	strcpy(txt, console.cmdbuf);
	drawcursor = ((int)((System_Milliseconds()/1000.0)*con_cursorspeed.value) & 1);
		
	pos = 2 + console.cursor - console.linelength;
	if (pos<0) pos = 0;

	Draw_SetColor(console_inputcolor);
	//draw prompt
	DU_DrawCharMonospaced(0,
				(con_fontheight.value+CONSOLE_SPACING_Y)*(lines-1),
				con_fontwidth.value,
				con_fontheight.value,
				']',
				hostmedia.sysfontShader
				);
	//draw cursor
	if (drawcursor)
		DU_DrawCharMonospaced((con_fontwidth.value+CONSOLE_SPACING_X)*(console.cursor+1),
				(con_fontheight.value+CONSOLE_SPACING_Y)*(lines-1),
				con_fontwidth.value,
				con_fontheight.value,
				'_',
				hostmedia.sysfontShader
				);
	//draw input buffer
	DU_DrawStringMonospaced(con_fontwidth.value+CONSOLE_SPACING_X,
				  (con_fontheight.value+CONSOLE_SPACING_Y)*(lines-1),
				  &txt[pos],
				  con_fontwidth.value,
				  con_fontheight.value,
				  1,
				  strlen(&txt[pos]),
				  hostmedia.sysfontShader
				  );
}

void	Console_Draw()
{
	float maxlines;

	OVERHEAD_INIT;

	if (localClient.cstate == CCS_IN_GAME || (DLLTYPE == DLLTYPE_EDITOR && world.loaded))
	{
		maxlines = (Vid_GetScreenH() >> 1) / (con_fontheight.value+CONSOLE_SPACING_Y);
	}
	else
	{
		maxlines = (Vid_GetScreenH()) / (con_fontheight.value+CONSOLE_SPACING_Y);
	}
	//hardcoded console display effect (fixme: client side?)
	if (console.active)
	{
		console_color[3]+=(con_fadespeed.value / 10);
		if (console_color[3] > con_alpha.value) console_color[3] = con_alpha.value;
	}
	else
	{
		if (!con_overlay.integer)
		{
			console_color[3]-=(con_fadespeed.value / 10);
			if (console_color[3] < 0)
			{
				console_color[3] = 0;
				return;
			}
		}
	}
	
	console_textcolor[3] = console_color[3];
	Console_DrawBackground(maxlines);
	Console_DrawBuffer(maxlines);
	Console_DrawInput(maxlines);

	OVERHEAD_COUNT(OVERHEAD_CONSOLE_DRAW);
}

void	Console_ClearCmdBuf()
{
	memset(console.cmdbuf, '\0', CONSOLE_CMD_SIZE);
	console.cursor = 0;
}

void	Console_AddHistory(char *cmd)
{
	strcpy(cmdhist[cmdhist_cursor % CMDHIST_SIZE], cmd);
	cmdhist_pos = cmdhist_cursor;
	cmdhist_cursor++;	
}

void	Console_Key(int key)
{
	int i;
	char *match;
	int conlines = (Vid_GetScreenH() >> 1) / (con_fontheight.value+CONSOLE_SPACING_Y);

	switch(key)
	{
		case KEY_BACKSPACE:
			if (console.cursor<=0) return;			
			console.cmdbuf[console.cursor-1] = '\0';
			console.cursor--;
			return;
		case KEY_RETURN:
		case KEY_ENTER:
			//add command to the execute buffer
			Cmd_BufPrintf("%s", console.cmdbuf);
			Console_Printf("]%s\n", console.cmdbuf); //echo to the console
			Console_AddHistory(console.cmdbuf);
			Console_ClearCmdBuf();									
			return;
		case KEY_LEFT:
			console.cursor--;
			if (console.cursor<0) console.cursor=0;
			return;
		case KEY_RIGHT:
			console.cursor++;
			if (console.cursor > (int)strlen(console.cmdbuf))
				console.cursor = (int)strlen(console.cmdbuf);
			return;
		case KEY_PGUP:
			console.backscroll++;
			if (console.backscroll > console.lines - conlines) console.backscroll = console.lines - conlines;
			if (console.backscroll < 0)
				console.backscroll = 0;
			return;
		case KEY_PGDN:
			console.backscroll--;
			if (console.backscroll < 0) console.backscroll=0;
			return;
		case KEY_HOME:
			console.backscroll = console.lines - conlines;
			if (console.backscroll < 0) console.backscroll = 0;
			return;
		case KEY_END:
			console.backscroll = 0;
			return;
		case KEY_UP:
			cmdhist_cursor--;
			if (cmdhist_cursor < 0) cmdhist_cursor = 0;
			Console_ClearCmdBuf();
			strcpy(console.cmdbuf, cmdhist[cmdhist_cursor % CMDHIST_SIZE]);
			console.cursor = strlen(console.cmdbuf);
			return;
		case KEY_DOWN:	
			cmdhist_cursor++;
			if (cmdhist_cursor > ((cmdhist_pos) + 1)) cmdhist_cursor = cmdhist_pos + 1;
			Console_ClearCmdBuf();
			strcpy(console.cmdbuf, cmdhist[cmdhist_cursor % CMDHIST_SIZE]);
			console.cursor = strlen(console.cmdbuf);
			return;
		case KEY_TAB:
			i = console.cursor;
			while (i > 0)
			{
				if (console.cmdbuf[i] == ' '
					|| console.cmdbuf[i] == ';')
					break;
				i--;
			}
			match = Completion_CompleteString(&console.cmdbuf[i], true);
			if (match)
			{
				console.cursor = i;
				for (i = 0; i < (int)strlen(match); i++)
					Console_Key(match[i]);
			}
			return;
	}


	if (key<127)
		console.cmdbuf[console.cursor++] = key;	

	if (console.cursor > CONSOLE_CMD_SIZE-2)
		console.cursor = CONSOLE_CMD_SIZE-2;
}

char newbuf[CONSOLE_BUF_SIZE];

//formats the buffer to adjust to screen dimensions
void	Console_Format()
{
	int oldlines, oldlinelength, newlinelength, x, y, numchars, numlines;

	OVERHEAD_INIT;

	if (!console.initialized)
		return;
	if (demo.playing)		//when making movies, the screen width/height will be different, but we don't want to format the console
		return;
	
	newlinelength = Vid_GetScreenW() / (con_fontwidth.value + CONSOLE_SPACING_X);
	if (newlinelength == console.linelength) return;

	oldlinelength = console.linelength;
	console.linelength = newlinelength;
	if (console.linelength > CONSOLE_MAX_LINE_LENGTH)
		console.linelength = CONSOLE_MAX_LINE_LENGTH;
	oldlines = console.lines;
	console.lines = CONSOLE_BUF_SIZE / console.linelength;

	numlines = oldlines;

	if (console.lines < numlines)
		numlines = console.lines;

	numchars = oldlinelength;
	
	if (console.linelength < numchars)
		numchars = console.linelength;

	Mem_Copy(newbuf, console.buf, CONSOLE_BUF_SIZE);
	memset(console.buf, ' ', CONSOLE_BUF_SIZE);

	//copy over the old buffer to the new one
	for (y=0; y<numlines; y++)
		for (x=0; x<numchars; x++)
		{
			console.buf[(console.lines - 1 - y) * console.linelength + x] =
						newbuf[((console.ypos - y + oldlines) %
						 oldlines) * oldlinelength + x];		
		}
	
	console.ypos = console.lines - 1;

	OVERHEAD_COUNT(OVERHEAD_CONSOLE_FORMAT);
}

void	Console_Init()
{		
	console.cursor = 0;
	console.xpos = 0;
	console.enabled = true;

	Cvar_Register(&con_fontwidth);
	Cvar_Register(&con_fontheight);

	console.linelength = 1024 / (con_fontwidth.value + CONSOLE_SPACING_X);
	console.lines = CONSOLE_BUF_SIZE / console.linelength;
	console.ypos = console.lines - 1;
	console.active = false;
	memset(console.cmdbuf, '\0', CONSOLE_CMD_SIZE);
	memset(console.buf, ' ', CONSOLE_BUF_SIZE);
	memset(cmdhist, 0, sizeof(cmdhist));
	cmdhist_pos = 0;
	cmdhist_cursor = 0;

	Cvar_Register(&con_logtodisk);
	Cvar_Register(&con_developer);
	Cvar_Register(&con_cursorspeed);
	Cvar_Register(&con_alpha);
	Cvar_Register(&con_fadespeed);
	Cvar_Register(&con_margin);
	Cvar_Register(&con_overlay);
	Cvar_Register(&con_bufferfile);
	Cvar_Register(&con_echoClient);

	console.initialized = true;
}
