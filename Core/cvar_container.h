// (C) 2001 S2 Games

// cvar_container.h

// provides a structure for storing copies of cvars for use with higher level modules, such as particles (todo) and world objects



#define		MAX_CVAR_CONTAINER_VARS		256

typedef struct
{
	cvar_t				*var;			//variable this was derived from

	char				*initial;		//initial value/string of var

	char				*string;		//what the variable is set to	
	float				value;			//floating point representation of string
	int					integer;		//integer representation of value
} cvarContainerVar_t;

typedef struct
{
	char					*classname;
	char					*name;

	cvarContainerVar_t 		*vars;
	unsigned int 			num_vars;
} cvarContainer_t;


bool				CvarContainer_CreateClass(const char *classname);
int					CvarContainer_RegisterVar(const char *containerClass, const char *varName);
bool				CvarContainer_Alloc(const char *containerClass, const char *name, cvarContainer_t *ptr);
void				CvarContainer_Recopy(cvarContainer_t *container);
float				CvarContainer_GetFloat(cvarContainer_t *container, unsigned int varId);
int					CvarContainer_GetInteger(cvarContainer_t *container, unsigned int varId);
char				*CvarContainer_GetString(cvarContainer_t *container, unsigned int varId);
char				*CvarContainer_GetVarName(const char *containerClass, unsigned int varId);
cvarContainer_t		*CvarContainer_Find(const char *name, cvarContainer_t list[], int num_entries);
void				CvarContainer_SetCvars(cvarContainer_t *container);
void				CvarContainer_ResetCvars(const char *classname);
void				CvarContainer_Free(cvarContainer_t *ptr);
int					CvarContainer_GetVarID(cvarContainer_t *container, const char *varname);
void				CvarContainer_Set(cvarContainer_t *container, unsigned int varId, const char *string);
cvar_t				*CvarContainer_GetCvar(cvarContainer_t *container, int varId);

extern cvarContainer_t	null_cvarContainer;