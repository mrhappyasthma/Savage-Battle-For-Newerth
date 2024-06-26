// (C) 2003 S2 Games

// savage_common.h

// common types and function headers

#ifndef SAVAGE_COMMON_H
#define SAVAGE_COMMON_H

#ifdef __MACH__
#define unix
#define DYLD
#endif

#ifdef _WIN32
	#pragma warning (disable: 4244)
#endif

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#ifdef unix
#include <ctype.h>

#define stricmp strcasecmp
#define strnicmp strncasecmp
#endif

#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN 1
#endif
#ifndef BYTE_ORDER
        #ifdef _WIN32
                #define BYTE_ORDER LITTLE_ENDIAN
        #endif
#endif

//define our inline keyword across platforms
#ifdef _WIN32
#define INLINE __forceinline
#else
#define INLINE inline
#endif

#ifdef _WIN32
#define USE_EAX
#else
#define _vsnprintf vsnprintf
#define _isnan isnan
#endif //not _WIN32

#define LAZY_VECTORS

#ifndef __cplusplus

#undef bool
#define bool int

#undef true
#define true	1

#undef TRUE
#define TRUE	1

#undef false
#define false	0

#undef FALSE
#define FALSE	0

#endif	//__cplusplus

#define		DEFAULT_LANGUAGE		"US"

#define		MINUTES_PER_DAY			1440

#ifndef _MAX_PATH
#define _MAX_PATH 256
#endif

#ifndef MAX_UINT
#define MAX_UINT 0xFFFFFFFF
#endif

#ifndef M_PI
#define M_PI	3.14159265358979323846f
#endif

//if MAX_COORD_RANGE changes, COORD2SHORT and SHORT2COORD functions must change accordingly below
//#define	MAX_COORD_RANGE 32768.0f

#define DEG2RAD(a) ((a) * (M_PI / 180.0))
#define RAD2DEG(a) ((a) / (M_PI / 180.0))

#define	BYTE_TO_FLOAT	0.003921568627450980392156862745098f   //(1.0 / 255)

#define WORD2ANGLE(a) (((float)(a) / 65536) * 360)
#define ANGLE2WORD(a) (word)(((int)((a) * 65536.0 / 360.0)) & 65535)
#define BYTE2ANGLE(a) (((float)(a) / 256) * 360)
#define ANGLE2BYTE(a) (byte)(((int)((a) * 256.0 / 360.0)) & 255)
//#define COORD2SHORT(a) ((word)((a) * 4))
//#define SHORT2COORD(a) ((float)((word)(a)) * 0.25)
#define COORD2SHORT(a) ((short)(a * 2))
#define SHORT2COORD(a) ((float)(a * 0.5))

#define FAR_AWAY	999999999

typedef float			vec_t;
typedef int				ivec_t;
typedef unsigned int	uivec_t;
typedef unsigned char	bvec_t;
#ifndef byte
typedef unsigned char	byte;
#endif
typedef unsigned short int word;
typedef unsigned short int wvec_t;

typedef vec_t	vec2_t[2];
typedef vec_t	vec3_t[3];
typedef vec_t	vec4_t[4];
typedef ivec_t	ivec2_t[2];
typedef uivec_t	uivec3_t[3]; //for facelists
typedef bvec_t	bvec2_t[2];
typedef bvec_t	bvec3_t[3];	 //for single precision mesh vertices
typedef bvec_t	bvec4_t[4];  //for colors
typedef wvec_t	wvec3_t[3];  //for double precision mesh vertices

#define X	0
#define Y	1
#define Z	2
#define W	3

//axes
#define RIGHT	0
#define FORWARD	1
#define UP	2

#ifndef _S2_EXPORTER		//3dsmax defines these enums, so this is to prevent a compile error

typedef enum
{
	MOUSE_LBUTTON,
	MOUSE_MBUTTON,
	MOUSE_RBUTTON
} mouse_button_enum;

#else

typedef enum
{
	___This_____________,
	___IsA___,
	___Hack___
} mouse_button_enum;

#endif

//convert any parameter into a string
#define STR(n) #n

//convenient macros for various grid references
#define GRIDREF(xref,yref) ((yref) * (world.gridwidth) + (xref))
#define WORLD_GRIDREF(xref,yref) (world.grid[(yref) * (world.gridwidth) + (xref)])
#define WR_COLREF(xref,yref) (wr.colormap[(yref) * (world.gridwidth) + (xref)])
#define WR_SHADERREF(xref,yref) (wr.shadermap[0][(yref) * (world.gridwidth) + (xref)])
#define WR_SHADERREF2(xref,yref) (wr.shadermap[1][(yref) * (world.gridwidth) + (xref)])
#define WR_FOLIAGECHANCE(xref,yref) (wr.foliageChance[(yref) * (world.gridwidth) + (xref)])

//these macros won't work from the game code
#define GRID_TO_WORLD(coord) ((coord) * (world_scale.value*100.0))
#define WORLD_TO_GRID(coord) ((coord) * (1 / (world_scale.value*100.0)))
#define WORLD_TILESIZE  (world_scale.value*100.0)     //number of units per grid tile

//useful macros
#define SET_VEC2(vec, x, y) ( vec[0]=(x), vec[1]=(y) )
#define	SET_VEC3(vec, x, y, z) ( vec[0]=(x), vec[1]=(y), vec[2]=(z) )
#define	SET_VEC4(vec, x, y, z, w) ( vec[0]=(x), vec[1]=(y), vec[2]=(z), vec[3]=(w) )

//#define	LIST_DO(listname, listptr) for (listptr=listname; listptr; listptr=listptr->next)

#define LIST_CLEAR(link) ( (link)->prev = (link), (link)->next = (link) )
#define LIST_APPEND(link, item) ( (item)->next = (link), (item)->prev = (link)->prev, (item)->prev->next = (item), (item)->next->prev = (item) )
#define LIST_INSERT(link, item) ( (item)->next = (link)->next, (item)->prev = (link), (item)->prev->next = (item), (item)->next->prev = (item) )
#define LIST_REMOVE(link) ( (link)->prev->next = (link)->next, (link)->next->prev = (link)->prev, (link)->next = NULL, (link)->prev = NULL )
#define LIST_EMPTY(link) ( ((link)->next == (link)) ? 1 : 0 )
#define	LIST_GETTAIL(tail, link) tail = link; while(tail->next!=link) tail=tail->next

//generic double linked list type.  to use, use as the first field in a struct and typecast to get at other fields (nice and hacky!)
/*typedef struct doublelink_s
{
	struct doublelink_s *next;
	struct doublelink_s *prev;
} doublelink_t;
*/

//resource related types
typedef	unsigned int		residx_t;

//useful functions
#define FloatToInt(a, b) (*a = (int)b)
/*
#ifdef _WIN32
__forceinline void FloatToInt(int *int_pointer, float f) 
{
	__asm  fld  f
  __asm  mov  edx,int_pointer
  __asm  FRNDINT
  __asm  fistp dword ptr [edx];
}
#else
#define FloatToInt(a, b) (*a = (int)b)
*/
/* commented out for now until I settle the syntax issues
INLINE void FloatToInt(int *int_pointer, float f) 
{
  asm("fld $1\n\t" \
      "movl %edx, $0\n\t" \
      "frndint\n\t" \
      "fistp dword ptr %edx"
	  : // no output registers
	  : // no input registers
	  : "%eax"); //eax gets clobbered
}
*/

//#endif

bool	IsLineSeparator(int c);
bool	IsSpace(int c);
bool	IsEOL(int c);
bool	IsTokenSeparator(int c);
const char	*Filename_StripExtension(const char *in, char *out);
const char	*Filename_GetDir(const char *filename);
const char	*Filename_GetExtension(char *filename);
const char	*Filename_GetFilename(char *filename);
char	*GetNextWord(const char *string);
char	*StripEOL(char *string);
bool	BPrintf(char *buffer, int count, const char *format, ...);
void	StrInsert(char *str, char *insert, int pos);
char	*StrChrBackwards(const char *str, int c);
char	*SkipSpaces(char *string);
char	*fmt(const char *s, ...);
char	*FirstTok(const char *string);
int		SplitArgs(char *s, char **argv, int maxargs);
void	ConcatArgs(char *argv[], int numargs, char *out);
float	*vec2(float x, float y);
float	*vec3(float x, float y, float z);
float	*vec4(float x, float y, float z, float a);
void	StringToVec3(const char *string, vec3_t vec);
void	StringToVec4(const char *string, vec4_t vec);
char	*ConvertStringToFilename(const char *string, char *out);
void	strncpySafe(char *strDest, const char *strSource, int count);
//endian conversion (convert a little endian value to the correct endianness for this platform)
int 	SwapIntEndian(int in);
int		LittleInt(int in);
int		LittleShort(int in);
float	LittleFloat(float in);

char 	*BinaryToHexWithXor(char *binary, int len, int xorbits);
int 	HexToBinary(char *hex, char *binary, int maxlen);

#ifndef _WIN32
char	*strupr(char *s);
char 	*strlwr(char *s);
void 	strset(char *s, char fill);
#endif

void	ST_RemoveState(char *string, const char *state);
bool	ST_SetState(char *string, const char *state, const char *value, int maxbuf);
bool	ST_FindState(const char *stateString, const char *state);
char	*ST_GetState(const char *stateString, const char *state);
int		ST_ForeachState(const char *stateString, void (*callback)(const char *key, const char *value));


extern vec3_t	unitcube_min;
extern vec3_t	unitcube_max;
extern vec3_t	zero_vec;
extern vec4_t	white;
extern vec4_t	green;
extern vec4_t	red;
extern vec4_t	blue;
extern vec4_t	yellow;
extern vec4_t	pink;
extern vec4_t	grey;
extern vec4_t	black;

/*	
	stack manager template
	
	here's a simple pop/push mechanism that can be implemented for any data type

	ex:

	CREATE_STACK_DATATYPE(quadstack_t, quadnode_t, MAX_QUADSTACK_SIZE);
	
	IMPLEMENT_STACK_FUNCTIONS(quadstack_t, quadnode_t, MAX_QUADSTACK_SIZE, WT_PushNode, WT_PopNode, WT_ClearStack)

	//new stack datatype is now ready for use
	static quadstack_t stack;

	these macros won't allocate the datatype for you.  allocate them in a separate array and use the stack functions to reference them.
*/

#define CLEARBIT(var, bit) (var &= ~bit)

#define CREATE_STACK_DATATYPE(stackstruct, datatype, stacksize) \
typedef struct \
{ \
	int			startpos; \
	int			endpos; \
\
	datatype	*nodes[stacksize]; \
} stackstruct;

#define IMPLEMENT_STACK_FUNCTIONS(stackstruct, datatype, stacksize, pushFunctionName, popFunctionName, clearFunctionName) \
void	pushFunctionName(stackstruct *stack, datatype *node) \
{ \
	stack->nodes[stack->endpos] = node; \
\
	stack->endpos = (stack->endpos + 1) % stacksize; \
	if (stack->endpos == stack->startpos) \
	{ \
		stack->endpos = (stack->endpos - 1) % stacksize; \
		Console_DPrintf(#pushFunctionName ": out of stack space\n"); \
	} \
} \
\
datatype *popFunctionName(stackstruct *stack) \
{ \
	datatype *ret; \
\
	if (stack->startpos == stack->endpos) \
	{ \
		return NULL; \
	} \
\
	ret = stack->nodes[stack->startpos]; \
	stack->startpos = (stack->startpos + 1) % stacksize; \
\
	return ret; \
} \
\
void	clearFunctionName(stackstruct *stack) \
{ \
	stack->startpos = stack->endpos = 0; \
}


#include "savage_mathlib.h"

/*
void	DblLink_Clear(doublelink_t *link);
void	DblLink_Append(doublelink_t *link, doublelink_t *item);
void	DblLink_Insert(doublelink_t *link, doublelink_t *item);
void	DblLink_GetTail(doublelink_t *link);
void	DblLink_Remove(doublelink_t *link);
int		DblLink_Count(doublelink_t *link);
*/

#endif //SAVAGE_COMMON_H
