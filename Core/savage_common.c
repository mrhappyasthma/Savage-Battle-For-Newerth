// (C) 2003 S2 Games

// savage_common.c

// various useful functions

#include "savage_common.h"

vec3_t	unitcube_min = { -1,-1,-1 };
vec3_t	unitcube_max = { 1,1,1 };
vec3_t	zero_vec = { 0,0,0 };
vec4_t	white = { 1,1,1,1 };
vec4_t	green = { 0,1,0,1 };
vec4_t	red = { 1,0,0,1 };
vec4_t	blue = { 0,0,1,1 };
vec4_t	yellow = { 1,1,0,1 };
vec4_t	pink = { 1,0,1,1 };
vec4_t	grey = { 0.5,0.5,0.5,1 };
vec4_t	black = { 0,0,0,1 };

#ifdef LAZY_VECTORS
//vectors for lazy people:

vec3_t __lazyvec__[8];
int __lazyidx__ = 0;
int __tempidx__ = 0;

#endif

char *BinaryToHexWithXor(char *binary, int len, int xorbits)
{
	static char hex[256+1];
	int j;
	int xor;

	if (len*2 > 256) //erm, this sucks, but the alternative is memory allocation
		return NULL;
	
	hex[len*2] = 0;
	for (j = 0; j < len/4; j++) //assumes sizeof(int) is 4
	{
		xor = (*(int *)&binary[j * 4] ^ xorbits);
		memcpy(&hex[j*8], fmt("%08x", SwapIntEndian(xor)), 8); 
	}
	return hex;
}

int	HexToBinary(char *hex, char *binary, int maxlen)
{
	int block;
	int len, i, max;
	
	len = strlen(hex);
	max = MIN(maxlen/4, len/8);
	for (i = 0; i < max; i++)
	{
		sscanf(&hex[i * 8], "%08x", &block);
		block = SwapIntEndian(block);
		memcpy(&binary[i * 4], &block, 4);
	}
	return max*4;
}

bool	IsLineSeparator(int c)
{
	switch(c)
	{
		case '\0':
		case '\n':
		case '\r':
			return true;
	}

	return false;
}

bool	IsTokenSeparator(int c)
{
	switch(c)
	{
		case '\n':
		case '\r' :
		case ' ' :
		case '\t':
			return true;
	}

	return false;
}

bool	IsSpace(int c)
{
	switch(c)
	{
		case ' ' :
		case '\t':
			return true;
	}

	return false;
}

bool	IsEOL(int c)
{
	switch(c)
	{
		case '\n':
		case '\r':		
			return true;
	}

	return false;
}

char	*StripEOL(char *string)
{
	char *s = string;

	while(*s)
	{
		if (IsLineSeparator(*s))
		{
			*s = 0;
			return string;
		}
		s++;
	}
	
	return string;
}

char	*GetNextWord(const char *string)
{
	char *s = (char *)string;
	
	while (!IsTokenSeparator(*s) && *s)
		s++;
	
	if (!*s)
		return s;

	while (IsTokenSeparator(*s) && *s)
		s++;

	if (!*s)
		return s;

	return s;
}

char	*SkipSpaces(char *string)
{
	char *s = string;

	while (*s)
	{
		switch (*s)
		{
		case ' ': case '\n': case '\r': case '\t':
				s++;
				break;
			default:
				return s;
		}
	}

	return s;
}

const char	*Filename_StripExtension(const char *in, char *out)
{
	int n;
	static char buf[1024];

	if (out == NULL)
		out = buf;

	if (out != in)
		strcpy(out, in);

	for (n=strlen(out)-1; n>=0; n--)
	{		if (out[n] == '.')
		{
			out[n] = '\0';
			break;
		}
	}

	return out;
}



const char *Filename_GetDir(const char *filename)
{
	static char dir[1024];
#ifdef _WIN32
	_splitpath(filename, NULL, dir, NULL, NULL);
	return dir;
#else
	char *filename_pos;

	BPrintf(dir, 1023, filename);
	filename_pos = strrchr(dir, '/');
	if (!filename_pos)
		filename_pos = strrchr(dir, '\\');
	if (filename_pos)
		filename_pos[1] = 0;
	else
		strcpy(dir, "");
	return dir;
#endif
}

const char	*Filename_GetExtension(char *filename)
{
	//go backwards until we find a dot and return the pointer
	char *s;

	s = strrchr(filename, '.');
	if (s)
		return s+1;

	return "";
}

char	*StrChrBackwards(const char *str, int c)
{
	int n;

	for (n=strlen(str)-1; n>=0; n--)
	{
		if (str[n] == c)
			return (char*)&str[n];
	}

	if (n<0) n=0;

	return (char*)&str[n];
}


#define	STATE_CHAR -1
#define VALUE_CHAR -2

void	ST_RemoveState(char *string, const char *state)
{
	int len;
	char *statepos;
	char *s = string;
	
	len = strlen(state);

	statepos = strstr(s, fmt("%c%s%c", STATE_CHAR, state, VALUE_CHAR));

	if (!statepos)
		return;
		
	if (statepos[1+len] == VALUE_CHAR)
	{
		//we found the state we want to remove
		char *endValue = strchr(&statepos[1+len+1], STATE_CHAR);
		if (!endValue)
			endValue = string + strlen(string) + 1;
			
		memmove(statepos, endValue, strlen(endValue)+1);

		return;
	}
}

/*==========================

  ST_SetState

  add or change a state in an existing string

 ==========================*/

bool	ST_SetState(char *string, const char *state, const char *value, int maxbuf)
{	 
	if (strchr(state, STATE_CHAR) || strchr(state, VALUE_CHAR))
		return false;
	if (strchr(value, STATE_CHAR) || strchr(value, VALUE_CHAR))
		return false;

	//remove the state if it exists
	ST_RemoveState(string, state);		

	if ((int)(strlen(state) + strlen(value) + strlen(string) + 3) >= maxbuf)
		return false;

	//append the state
	strcat(string, fmt("%c%s%c%s", STATE_CHAR, state, VALUE_CHAR, value));

	return true;
}


/*==========================

  ST_FindState

  search a string for a state name

  returns false if not found, true if found

 ==========================*/

bool	ST_FindState(const char *stateString, const char *state)
{
	char *statepos;		

	statepos = strstr(stateString, fmt("%c%s%c", STATE_CHAR, state, VALUE_CHAR));

	if (!statepos)
		return false;

	return true;
}


/*==========================

  ST_GetState

  search a string for a state value

  returns "" if the value isn't found or "BUFFER_OVERFLOW" if 'buf' overflowed
  
  uses a static buf for convenience, limit 1024 bytes

 ==========================*/

char	*ST_GetState(const char *stateString, const char *state)
{
	static char buf[8][1024];
	static unsigned int marker = 0;
	unsigned int idx = marker % 8;
	int n = 0;
	int len;
	char *statepos;	
	
	len = strlen(state);

	statepos = strstr(stateString, fmt("%c%s%c", STATE_CHAR, state, VALUE_CHAR));

	if (!statepos)
		return "";

	statepos += 2 + strlen(state);

	if (!statepos)
		return "";

	while (*statepos && *statepos != STATE_CHAR)
	{		
		buf[idx][n++] = *statepos;

		statepos++;

		if (n>=1023)
			return "BUFFER_OVERFLOW";
	}

	buf[idx][n] = 0;
	marker++;

	return buf[idx];
}


int	ST_ForeachState(const char *stateString, void (*callback)(const char *key, const char *value))
{
	const char *pos = stateString;
	const char *marker;
	char stateStringName[1024];
	char stateValue[1024];
	int len, count = 0;

	while (pos && (stateString = strchr(pos, STATE_CHAR)))
	{
		pos++;
		stateString++;

		//now pos starts are the first character after the STATE_CHAR 
		//stateString also points there
		
		//find the position of the VALUE_CHAR
		marker = strchr(stateString, VALUE_CHAR);
		if (!marker) //no value?  invalid state string
			break;
		
		//find the length of the state name
		len = marker - stateString;
		strncpy(stateStringName, stateString, len);
		stateStringName[len] = 0;
		
		//now let's find the value for this statestring
		stateString = marker;
		marker = strchr(stateString, STATE_CHAR);
		if (!marker)
			marker = &stateString[strlen(stateString)]; //point to the NULL, use it as the terminator
		
		//now point stateString to the start of the value by incrementing it past the VALUE_CHAR
		stateString++;
		
		//copy out the value
		len = marker - stateString;
		strncpy(stateValue, stateString, len);
		stateValue[len] = 0;

		if (callback)
			callback(stateStringName, stateValue);
		
		pos = marker;
		count++;
	}

	return count;
}

void	strncpySafe(char *strDest, const char *strSource, int count)
{
	strncpy(strDest, strSource, count-1);
	strDest[count-1] = '\0';
}

//returns a filename which was preceeded by a directory name
const char *Filename_GetFilename(char *filename)
{
	char *f;
	int probablyFile = false;

	for (f = &filename[strlen(filename)-1]; f>=filename; f--)
	{
		if (*f=='/' || *f=='\\')
			return f+1;
		if (*f=='.')
			probablyFile = true;
	}

	if (probablyFile)
		return filename;
	else
		return "";
}

//converts any string into an acceptable filename
//'out' must be at least the same length as 'string'
char	*ConvertStringToFilename(const char *string, char *out)
{
	char *ret = out;
	char *s = (char *)string;		

	while (*s)
	{
		if ((*s >= 'a' && *s <= 'z') ||
			(*s >= 'A' && *s <= 'Z') ||
			(*s >= '0' && *s <= '9'))
		{
			*out = *s;
		}
		else
		{
			//convert other chars to underscores
			*out = '_';
		}

		s++;
		out++;
	}

	*out = 0;

	return ret;
}

bool BPrintf(char *buffer, int count, const char *format, ...)
{
	va_list argptr;

	va_start(argptr, format);
	if (_vsnprintf(buffer, count, format, argptr) == -1)
	{
		va_end(argptr);
		buffer[count] = 0;
//		Console_DPrintf("Warning: Buffer overflow occured during BPrintf\n");
		return false;
	}
	va_end(argptr);

	return true;
}

//for passing a formatted string into any function that accepts a char * parameter
//borrowing from a function i saw in the q3 game code

#define NUM_FMT_STRINGS 16

char *fmt(const char *s, ...)
{
	static char buf[NUM_FMT_STRINGS][8192];
	static unsigned int marker = 0;
	unsigned int idx = marker;

	va_list argptr;

	va_start(argptr, s);

	if (_vsnprintf(buf[idx], 8191, s, argptr) == -1)
	{
//		Console_DPrintf("Warning: Buffer overflow occured during fmt()\n");
		buf[idx][8191] = 0;
	}

	va_end(argptr);

	marker = (marker + 1) % NUM_FMT_STRINGS;

	return buf[idx];
}


#define NUM_TOK_STRINGS	16

char	*FirstTok(const char *string)
{
	static char buf[NUM_TOK_STRINGS][128];
	static unsigned int marker = 0;
	unsigned int idx = marker;
	char *s = (char *)string;
	int pos = 0;

	while(*s)
	{
		if (IsTokenSeparator(*s))
		{			
			break;
		}

		buf[idx][pos] = *s;
		pos++;
		if (pos >= 128)
			return "";

		s++;
	}

	buf[idx][pos] = 0;

	marker = (marker + 1) % NUM_TOK_STRINGS;

	return buf[idx];
}

int	SplitArgs(char *in, char **argv, int maxargs)
{
	char *s = in;
	int argc = 0;

	s = SkipSpaces(s);

	while (*s)
	{
		argv[argc] = s;
		argc++;

		if (argc >= maxargs)
			return argc;
		
		s++;

		while (!IsTokenSeparator(*s) && *s)
			s++;

		if (*s)
		{
			*s = 0;		//null terminate each arg

			s = SkipSpaces(s+1);
		}
	}

	return argc;
}

//this assumes we've allocated the correct amount of space to hold all the buffer args
void	ConcatArgs(char *argv[], int numargs, char *out)
{
	int n;

	out[0] = 0;

	for (n=0; n<numargs; n++)
	{
		strcat(out, argv[n]);
		if (n < numargs-1)
			strcat(out, " ");
	}
}


float	*vec2(float x, float y)
{
	static vec2_t vecs[32];
	static unsigned int marker = 0;
	unsigned int idx = marker % 32;

	vecs[idx][0] = x;
	vecs[idx][1] = y;

	marker++;

	return vecs[idx];
}


float	*vec3(float x, float y, float z)
{
	static vec3_t vecs[32];
	static unsigned int marker = 0;
	unsigned int idx = marker % 32;

	vecs[idx][0] = x;
	vecs[idx][1] = y;
	vecs[idx][2] = z;

	marker++;

	return vecs[idx];
}

float	*vec4(float x, float y, float z, float a)
{
	static vec4_t vecs[32];
	static unsigned int marker = 0;
	unsigned int idx = marker % 32;

	vecs[idx][0] = x;
	vecs[idx][1] = y;
	vecs[idx][2] = z;
	vecs[idx][3] = a;

	marker++;

	return vecs[idx];
}

void	StringToVec3(const char *string, vec3_t vec)
{
	int n;
	const char *s = (char *)string;

	vec[0] = atof(string);

	for (n=1; n<3; n++)
	{
		s = GetNextWord(s);
		vec[n] = atof(s);
	}
}


void	StringToVec4(const char *string, vec4_t vec)
{
	int n;
	const char *s = (char *)string;

	vec[0] = atof(string);

	for (n=1; n<4; n++)
	{
		s = GetNextWord(s);
		vec[n] = atof(s);
	}
}

#ifndef _WIN32
char *strlwr(char *s)
{
	char *start = s;

	while (*s)
	{
		*s = tolower(*s);
		s++;
	}

	return start;
}

char *strupr(char *s)
{
	char *start = s;

	while (*s)
	{
		*s = toupper(*s);
		s++;
	}

	return start;
}

void strset(char *s, char fill)
{
	int i = 0;
	while (s[i])
	{
		s[i] = fill;
		i++;
	}
}
#endif

int	SwapIntEndian(int in)
{
	return ((in >> 24) & 255) + 
		   (((in >> 16) & 255) << 8) + 
		   (((in >> 8) & 255) << 16) + 
		   ((in & 255) << 24);
}

float SwapFloat(float in)
{
	union
	{
		float f;
		byte b[4];
	} u1,u2;

	u1.f = in;
	u2.b[0] = u1.b[3];
	u2.b[1] = u1.b[2];
	u2.b[2] = u1.b[1];
	u2.b[3] = u1.b[0];
	return u2.f;
}

int	SwapShortEndian(int in)
{
	return ((in >> 8) & 255) + ((in & 255) << 8);
}

#if BYTE_ORDER == LITTLE_ENDIAN

int	LittleInt(int in)
{
	return in;
}

int LittleShort(int in)
{
	return in;
}

float LittleFloat(float in)
{
	return in;
}


#else

int	LittleInt(int in)
{
	return SwapIntEndian(in);
}

int LittleShort(int in)
{
	return SwapShortEndian(in);
}

float LittleFloat(float in)
{
	return SwapFloat(in);
}

#endif

/*
void DblLink_Clear(doublelink_t *link)
{
	link->prev = link;
	link->next = link;
}

void DblLink_Append(doublelink_t *link, doublelink_t *item)
{
	item->next = link;
	item->prev = link->prev;
	item->prev->next = item;
	item->next->prev = item;
}

void DblLink_Insert(doublelink_t *link, doublelink_t *item)
{
	item->next = link->next;
	item->prev = link;
	item->prev->next = item;
	item->next->prev = item;
}

void DblLink_GetTail(doublelink_t *link)
{
	doublelink_t *l, ret;

	ret = link;

	for (l=link; l!=link; l=l->next)
	{
		ret = l;
	}

	return ret;
}

void DblLink_Remove(doublelink_t *link)
{
	link->prev->next = link->next;
	link->next->prev = link->prev;
	link->next = NULL;
	link->prev = NULL;
}

int DblLink_Count(doublelink_t *link)
{
	int n = 0;
	doublelink_t *l;

	for (l=link; l!=link; l=l->next)
	{
		n++;
	}

	return n;
}
*/
