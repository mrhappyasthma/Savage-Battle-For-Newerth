// (C) 2001 S2 Games

// cvar_container.c

// provides a structure for storing copies of cvars for use with higher level modules, such as particles (todo) and world objects


#include "core.h"

#define CVAR_CONTAINER_CLASS_MAX_VARS	MAX_CVAR_CONTAINER_VARS

cvarContainer_t null_cvarContainer = { "", NULL, 0 };

typedef struct cvarContainerClass_s
{
	char				*name;

	cvar_t				*vars[CVAR_CONTAINER_CLASS_MAX_VARS];
	unsigned int		num_vars;
	
	struct cvarContainerClass_s	*next;
} cvarContainerClass_t;

cvarContainerClass_t	*class_list = NULL;


cvarContainerClass_t	*CvarContainer_FindClass(const char *classname)
{
	cvarContainerClass_t *list;

	for (list = class_list; list; list=list->next)
	{
		if (strcmp(list->name, classname)==0)
		{
			return list;
		}
	}

	return NULL;
}

cvarContainerClass_t	*_CvarContainer_CreateClass(const char *classname)
{
	cvarContainerClass_t *list;

	list = CvarContainer_FindClass(classname);
	if (list)
		return NULL;

	
	list = Tag_Malloc(sizeof(cvarContainerClass_t), MEM_CVAR);
	list->name = Tag_Strdup(classname, MEM_CVAR);

	Console_DPrintf("Creating CvarClass '%s'\n", classname);

	list->next = class_list;
	class_list = list;

	return list;
}

bool			CvarContainer_CreateClass(const char *classname)
{
	if (!_CvarContainer_CreateClass(classname))
		return false;

	return true;
}

void			CvarContainer_ResetCvars(const char *classname)
{
	cvarContainerClass_t *list;
	unsigned int n;

	list = CvarContainer_FindClass(classname);

	if (!list)
		return;

	for (n=0; n<list->num_vars; n++)
	{
		Cvar_SetVar(list->vars[n], list->vars[n]->default_string);
	}
}

int				CvarContainer_RegisterVar(const char *containerClass, const char *varName)
{
	cvarContainerClass_t *con;
	cvar_t *var;
	unsigned int n;

	con = CvarContainer_FindClass(containerClass);

	if (!con)
	{
		Console_DPrintf("CvarContainer_RegisterVar: class %s does not exist\n", containerClass);
		return 0;
	}

	if (con->num_vars >= CVAR_CONTAINER_CLASS_MAX_VARS)
	{
		Console_DPrintf("CvarContainer_RegisterVar: number of vars in %s exceeds CVAR_CONTAINER_CLASS_MAX_VARS\n", containerClass);
		return 0;
	}
	
	var = Cvar_Find(varName);

	if (!var)
	{
		Console_DPrintf("CvarContainer_RegisterVar: variable %s does not exist\n", varName);
		return 0;
	}

	for (n=0; n<con->num_vars; n++)
	{
		if (con->vars[n] == var)
		{
			Console_DPrintf("CvarContainer_RegisterVar: variable %s was already registered with the container class %s\n", varName, containerClass);
			return 0;
		}
	}

	con->vars[con->num_vars] = var;
	con->num_vars++;

	return con->num_vars;
}

bool				CvarContainer_Alloc(const char *containerClass, const char *name, cvarContainer_t *ptr)
{
	cvarContainerClass_t *con;
	unsigned int n;

	if (!ptr)
		return false;

	con = CvarContainer_FindClass(containerClass);
	if (!con)
	{
		Console_DPrintf("CvarContainer_Alloc: class %s does not exist\n", containerClass);
		return false;
	}

	memset(ptr, 0, sizeof(cvarContainer_t));

	ptr->name = Tag_Strdup(name, MEM_CVAR);
	ptr->num_vars = con->num_vars;
	ptr->vars = Tag_Malloc(sizeof(cvarContainerVar_t) * con->num_vars, MEM_CVAR);
	ptr->classname = Tag_Strdup(containerClass, MEM_CVAR);
	
	for (n=0; n<con->num_vars; n++)
	{
		ptr->vars[n].var = con->vars[n];
		ptr->vars[n].initial = Tag_Strdup(con->vars[n]->string, MEM_CVAR);
		ptr->vars[n].string = Tag_Strdup(con->vars[n]->string, MEM_CVAR);
		ptr->vars[n].integer = con->vars[n]->integer;		//alternative here would be atoi, but the cvar has done the work for us already
		ptr->vars[n].value = con->vars[n]->value;			//same goes for float	
	}

	return true;
}

void				CvarContainer_Free(cvarContainer_t *ptr)
{
	unsigned int n;

	if (!ptr)
		return;

	for (n=0; n<ptr->num_vars; n++)
	{
		Tag_Free(ptr->vars[n].initial);
		Tag_Free(ptr->vars[n].string);
	}

	Tag_Free(ptr->vars);

	memset(ptr, 0, sizeof(cvarContainer_t));
}

void				CvarContainer_Recopy(cvarContainer_t *ptr)
{
	unsigned int n;

	if (!ptr)
		return;

	for (n=0; n<ptr->num_vars; n++)
	{
		cvar_t *ref = ptr->vars[n].var;

		Tag_Free(ptr->vars[n].string);
		ptr->vars[n].string = Tag_Strdup(ref->string, MEM_CVAR);
		ptr->vars[n].integer = ref->integer;
		ptr->vars[n].value = ref->value;
	}
}

float				CvarContainer_GetFloat(cvarContainer_t *container, unsigned int varId)
{
	if (varId > container->num_vars || !varId)
		return 0;

	return container->vars[varId-1].value;
}

int					CvarContainer_GetInteger(cvarContainer_t *container, unsigned int varId)
{
	if (varId > container->num_vars || !varId)
		return 0;

	return container->vars[varId-1].integer;

}

char				*CvarContainer_GetString(cvarContainer_t *container, unsigned int varId)
{
	if (varId > container->num_vars || !varId)
		return "";

	return container->vars[varId-1].string;
}

char				*CvarContainer_GetVarName(const char *containerClass, unsigned int varId)
{
	cvarContainerClass_t *con;

	con = CvarContainer_FindClass(containerClass);
	if (!con)
	{
		Console_DPrintf("CvarContainer_Alloc: class %s does not exist\n", containerClass);
		return "";
	}

	if (varId > con->num_vars || !varId)
		return "";

	return con->vars[varId-1]->name;
}

int					CvarContainer_GetVarID(cvarContainer_t *container, const char *varname)
{
	unsigned int n;

	for (n=0; n<container->num_vars; n++)
	{
		if (strcmp(varname, container->vars[n].var->name)==0)
		{
			return n+1;
		}
	}

	return 0;
}

void				CvarContainer_Set(cvarContainer_t *container, unsigned int varId, const char *string)
{
	cvarContainerVar_t *var;

	if (varId > container->num_vars || !varId)
		return;

	var = &container->vars[varId-1];

	Tag_Free(var->string);
	var->string = Tag_Strdup(string, MEM_CVAR);
	var->value = atof(string);
	var->integer = (int)var->value;
}

cvarContainer_t		*CvarContainer_Find(const char *name, cvarContainer_t list[], int num_entries)
{
	int n;

	for (n=0; n<num_entries; n++)
	{
		if (strcmp(list[n].name, name)==0)
			return &list[n];
	}

	return NULL;
}


//copy the values of this container to the actual cvars
void				CvarContainer_SetCvars(cvarContainer_t *container)
{
	unsigned int n;

	for (n=0; n<container->num_vars; n++)
	{
		if (container->vars[n].var->flags & CVAR_PATH_TO_FILE)
		{
			container->vars[n].var->flags &= ~CVAR_PATH_TO_FILE;
			Cvar_SetVar(container->vars[n].var, container->vars[n].string);
			container->vars[n].var->flags |= CVAR_PATH_TO_FILE;
		}
		else
		{
			Cvar_SetVar(container->vars[n].var, container->vars[n].string);
		}
	}
}

cvar_t				*CvarContainer_GetCvar(cvarContainer_t *container, int varId)
{
	if (varId > 0 
		&& (unsigned int)varId <= container->num_vars)
	{
		return container->vars[varId-1].var;
	}
	return NULL;
}