
#include "core.h"
#include "zip.h"

#define MAX_KEYFRAMES 20
#define MAX_VARIABLES 40

typedef struct
{
	bool active;

	int	time;
	
	float values[MAX_VARIABLES];
} timeofday_t;

timeofday_t	times[MAX_KEYFRAMES];
cvar_t	*vars[MAX_VARIABLES];

static int	current_keyframe = -1;

cvar_t	tod_interpolate = { "tod_interpolate", "1" };

void	TOD_Summary()
{
	int i, j;
	
	Console_DPrintf("TOD: Summary:\n");
	i = 0;
	while (times[i].active)
	{
		Console_DPrintf("TOD: slot %i, time %i\n", i, times[i].time);
		j = 0;

		while (vars[j])
		{
			Console_DPrintf("%s: %f\n", vars[j]->name, times[i].values[j]);
			j++;
		}

		i++;
	}
}

void	TOD_Summary_Cmd(int argc, char *argv[])
{
	TOD_Summary();
}


int		TOD_GetNumKeyframes()
{
	int i=0;

	while (times[i].active && i < MAX_KEYFRAMES)
		i++;
	return i;
}

int		TOD_FindLowKeyframe(int time)
{
	int i = 0;

	//time is before the first keyframe, return the last keyframe
	if (times[0].active && time < times[0].time)
	{
		while (times[i].active)
			i++;
		return i-1;
	}
	
	//time is inbetween two keyframe, so find the highest without going over
	while (i < MAX_KEYFRAMES && times[i].active)
	{
		//if this keyframe is smaller and the next either doesn't exist or is larger, return this
		if (times[i].time <= time 
			&& ((i+1 >= MAX_KEYFRAMES || !times[i+1].active || times[i+1].time > time)))
			return i;
		i++;
	}
	return -1;
}

int		TOD_FindHighKeyframe(int time)
{
	int low;

	low = TOD_FindLowKeyframe(time);
	if (low == -1) //no keyframes!
		return -1;
	if (times[low].time == time) //exact match
		return low;
	if ((low == MAX_KEYFRAMES-1 || !times[low+1].active) && times[0].active) //last keyframe in the list, so roll around
		return 0;
	if (times[low+1].active) //otherwise just return the next one
		return low+1;
	return -1;
}

bool	TOD_OnKeyframe(int time)
{
	int low, high;
	
	low = TOD_FindLowKeyframe(time);
	high = TOD_FindHighKeyframe(time);

	if (low == high && low != -1)
	{
		current_keyframe = low;
		return true;
	}
	return false;
}

bool	TOD_InterpolateVars(float time)
{
	int low, high, varnum, totaltime, elapsedtime;
	static int lasttime = 0;
	float lowval, highval, amount, newval;
	
	low = TOD_FindLowKeyframe(time);
	high = TOD_FindHighKeyframe(time);

	if (low == -1)
		return false;
	
	if (low == high)
		current_keyframe = low;
	else
		current_keyframe = -1;

	varnum = 0;

	totaltime = times[high].time - times[low].time;
	if (totaltime < 0)
		totaltime = (times[high].time + MINUTESPERDAY) - times[low].time;
	elapsedtime = time - times[low].time;
	if (elapsedtime < 0)
		elapsedtime = (time + MINUTESPERDAY) - times[low].time;

	if (time != lasttime)
	{
		//Console_DPrintf("TOD: time %i: low time: %i, high time: %i, total diff = %i, elapsed time = %i\n", time, times[low].time, times[high].time, totaltime, elapsedtime);
	}
		
	while (vars[varnum])
	{
		lowval = times[low].values[varnum];
		highval = times[high].values[varnum];

		amount = (float)(elapsedtime) / MAX(1, totaltime);
		
		if (time != lasttime)
		{
			//Console_DPrintf("TOD: time %i: var %s: interpolating %f between %f and %f\n", time, vars[varnum]->name, amount, lowval, highval);
		}
		
		newval = LERP(amount, lowval, highval);
		Cvar_SetVarValue(vars[varnum], newval);
		
		varnum++;
	}
	lasttime = time;
	return true;
}

void	TOD_SetTime(float time)
{
	static int lasttime = -1;
	int i;

	if (DLLTYPE == DLLTYPE_EDITOR)
	{
		if (lasttime == time && TOD_OnKeyframe(time))
		{
			//don't interpolate them, set the current keyframe's values to the current value
			for (i = 0; vars[i]; i++)
				times[current_keyframe].values[i] = vars[i]->value;
		}
		else
			TOD_InterpolateVars(time);
	}
	else
	{
		if (tod_interpolate.integer)
			TOD_InterpolateVars(time);
	}
	lasttime = time;
}

int		TOD_GetVarNum(char *name)
{
	int i = 0;

	while (i < MAX_VARIABLES && vars[i] && stricmp(vars[i]->name, name) != 0)
		i++;

	if (i < MAX_VARIABLES && vars[i])
		return i;

	return -1;
}

bool	TOD_SetValue(char *var, float value)
{
	int i;

	i = TOD_GetVarNum(var);

	if (i < 0)
	{
		Console_Printf("TOD Variable %s not registered\n");
		return false;
	}

	if (current_keyframe < 0)
	{
		Console_Printf("TOD: No current keyframe\n");
		return false;
	}

	//Console_DPrintf("TOD: setting variable %s to %f\n", vars[i]->name, value);
	times[current_keyframe].values[i] = value;
	return true;
}

void	TOD_SetValue_Cmd(int argc, char *argv[])
{
	if (argc < 2)
	{
		Console_Printf("error: you must supply the name of the cvar and the value\n");
		return;
	}

	TOD_SetValue(argv[0], atof(argv[1]));
}

void	TOD_SetKeyframeAtPos(int pos, int time)
{
	int i;

	times[pos].active = true;
	times[pos].time = time;

	for (i = 0; i < MAX_VARIABLES; i++)
	{
		if (vars[i])
			times[pos].values[i] = vars[i]->value;
		else
			times[pos].values[i] = 0;
	}
}

int	_GetTime(char *str)
{
	char *tmp;
	int time;

	if ((tmp = strchr(str, ':')))
	{
		time = atoi(str) * 60 + atoi(tmp+1);
	}
	else
		time = atoi(str);

	return time;
}

bool	TOD_RemoveKeyframe(int time)
{
	int low, high;

	low = TOD_FindLowKeyframe(time);
	high = TOD_FindHighKeyframe(time);

	if (low != high || low == -1)
		return false;

	if (high < low) //cycles around
	{
		times[low].active = false;
		TOD_Summary();
		return true;
	}

	Mem_Move(&times[low], &times[high], sizeof(timeofday_t) * (MAX_KEYFRAMES - high));

	if (current_keyframe == low)
		current_keyframe = -1;

	TOD_Summary();
	return true;
}

void	TOD_RemoveKeyframe_Cmd(int argc, char *argv[])
{
	if (!argc)
	{
		Console_Printf("error: you must supply the time of the keyframe to remove (either in minutes or 24h time)\n");
		return;
	}

	TOD_RemoveKeyframe(_GetTime(argv[0]));
}

int		TOD_AddKeyframe(int time)
{
	int low, high, pos;

	if (times[MAX_KEYFRAMES-1].active)
	{
		Console_Printf("TOD: No more keyframes allowed\n");
		return -1;
	}
	
	low = TOD_FindLowKeyframe(time);
	high = TOD_FindHighKeyframe(time);

	Console_DPrintf("TOD: adding a new keyframe at time %i\n", time);
	Console_DPrintf("TOD: low time is %i (slot %i), high time is %i (slot %i)\n", times[low].time, low, times[high].time, high);
	if (!times[0].active)
	{
		pos = 0;
	}
	else if (low > high && time > times[low].time)
	{
		pos = low+1;
	}
	else
	{
		if (time < times[high].time && time < times[low].time)
			pos = 0;
		else if (low == high)
		{
			if (time > times[low].time)
				pos = low+1;
			else
				pos = low;
		}
		else
			pos = high;
		Console_DPrintf("TOD: Moving %i entries from slot %i to slot %i\n", (MAX_KEYFRAMES - pos - 1), pos, pos+1);
		Mem_Move(&times[pos+1], &times[pos], sizeof(timeofday_t) * (MAX_KEYFRAMES - pos -1));
	}
	Console_DPrintf("TOD: putting keyframe into slot %i\n", pos);
			
	TOD_SetKeyframeAtPos(pos, time);
	
	current_keyframe = pos;
	return current_keyframe;
}

void	TOD_AddKeyframe_Cmd(int argc, char *argv[])
{
	if (!argc)
	{
		Console_Printf("error: you must supply the time of the keyframe to add (either in minutes or 24h time)\n");
		return;
	}

	TOD_AddKeyframe(_GetTime(argv[0]));
}

void	TOD_AddVar(char *name)
{
	int i = 0;

	while (i < MAX_VARIABLES && vars[i])
		i++;

	if (i >= MAX_VARIABLES)
		return;

	vars[i] = Cvar_Find(name);
	if (!vars[i])
		Console_Printf("cvar %s not found\n", name);
	else
		Console_DPrintf("cvar %s added to spot %i\n", name, i);
}

void	TOD_AddVar_Cmd(int argc, char *argv[])
{
	int i;
	if (!argc)
	{
		Console_Printf("error: you must supply the name of the cvar(s) to add\n");
		return;
	}

	for (i = 0; i < argc; i++)
		TOD_AddVar(argv[i]);
}

void	_deletefile(const char *filename, void *userdata)
{
	File_Delete(filename);
}

void	TOD_SaveToZip(void *zipfile, char *name)
{
	int keyframe, cvarnum, ret;
	char str[512];
	int method = Z_DEFLATED;
	int level = Z_DEFAULT_COMPRESSION;

	if ((ZIPW_AddFileInZip(zipfile, fmt("world/%s/tod.cfg", name), NULL, NULL, 0, NULL, 0, NULL, method, level)) != ZIP_OK)
	{
		Console_Printf("failed to add world/%s/tod.cfg to zip file\n", name);
		return;
	}

	//write out the list of cvars
	for (cvarnum = 0; vars[cvarnum]; cvarnum++)
	{
		BPrintf(str, 512, "tod_addvar %s\n", vars[cvarnum]->name);
		if ((ret = ZIPW_WriteFileInZip(zipfile, str, strlen(str))) < 0)
			Console_Printf("Error %i writing to zip file\n", ret);
	}
	ret = ZIPW_CloseFileInZip(zipfile);
	if (ret!=ZIP_OK)
		Console_Printf("error in closing %s in the zipfile\n",name);
	
	//now save out all our new tod files
	for (keyframe = 0; times[keyframe].active; keyframe++)
	{
		if ((ZIPW_AddFileInZip(zipfile, fmt("world/%s/%i.tod", name, times[keyframe].time), NULL, NULL, 0, NULL, 0, NULL, method, level)) != ZIP_OK)
		{
			Console_Printf("failed to add %s to zip file\n", name);
			return;
		}

		for (cvarnum = 0; vars[cvarnum]; cvarnum++)
		{
			BPrintf(str, 512, "tod_set %s %f\n", vars[cvarnum]->name, times[keyframe].values[cvarnum]);
			if ((ret = ZIPW_WriteFileInZip(zipfile, str, strlen(str))) < 0)
				Console_Printf("Error %i writing to zip file\n", ret);
		}

		ret = ZIPW_CloseFileInZip(zipfile);
		if (ret!=ZIP_OK)
			Console_Printf("error in closing %s in the zipfile\n",name);
	}
}

void	TOD_Save_Cmd(int argc, char *argv[])
{
	/*
	file_t *file;
	int keyframe, cvarnum;
	char *str;

	//delete all the existing .tod files
	System_Dir(fmt("%s/world/%s/", world_save_path.string, World_GetName()), "*.tod", false, NULL, _deletefile, NULL);
	
	file = File_Open(fmt("%s/world/%s/tod.cfg", world_save_path.string, World_GetName()), "w");
	if (!file)
	{
		Console_Printf("error trying to open /world/%s/tod.cfg\n", World_GetName());
		return;
	}
	//write out the list of cvars
	for (cvarnum = 0; vars[cvarnum]; cvarnum++)
	{
		str = fmt("tod_addvar %s\n", vars[cvarnum]->name);
		file->write(str, strlen(str), 1, file);
	}
	File_Close(file);
	
	//now save out all our new tod files
	for (keyframe = 0; times[keyframe].active; keyframe++)
	{
		file = File_Open(fmt("%s/world/%s/%i.tod", world_save_path.string, World_GetName(), times[keyframe].time), "w");
		if (!file)
		{
			Console_Printf("error trying to open %s\n", fmt("%s%i.tod"));
			return;
		}

		for (cvarnum = 0; vars[cvarnum]; cvarnum++)
		{
			str = fmt("tod_set %s %f\n", vars[cvarnum]->name, times[keyframe].values[cvarnum]);
			file->write(str, strlen(str), 1, file);
		}

		File_Close(file);
	}
	*/
}

void	_loadfile(const char *filename, void *userdata)
{
	char *tmppos;
	int time;
	archive_t *archive = (archive_t *)userdata;

	if ((tmppos = strchr(filename, '.')))
	{
		time = atoi(filename);
		if (time >= 0 && time < 1440)
		{
			current_keyframe = TOD_AddKeyframe(time);
		
			Cmd_ReadConfigFileFromArchive(archive, fmt("world/%s/%s", World_GetName(), filename), false);
		}
	}
}

void	TOD_Load(archive_t *archive, const char *worldname)
{
	int i;

	//clear all the current vars and values
	for (i = 0; i < MAX_KEYFRAMES; i++)
		times[i].active = 0;

	for (i = 0; i < MAX_VARIABLES; i++)
		vars[i] = NULL;

	//read the tod cfg to get the variables we're going to use
	Cmd_ReadConfigFileFromArchive(archive, fmt("%s/tod.cfg", worldname), false);
		
	//load all the .tod files to get the values for each time
	System_Dir(fmt("%s/", worldname), "*.tod", false, NULL, _loadfile, archive);

	//TOD_Summary();
}

int     TOD_GetKeyframeTime(int keyframe)
{
	if (keyframe < MAX_KEYFRAMES && keyframe >= 0 && times[keyframe].active)
		return times[keyframe].time;
	else
		return -1;
}

int     TOD_MoveKeyframe(int keyframe, int newtime)
{
	timeofday_t tmp;
	int numKeyframes;

	numKeyframes = TOD_GetNumKeyframes();
	if (keyframe == 0)
	{
		if (!times[1].active || newtime < times[1].time)
		{
			times[0].time = newtime;
			return keyframe; //we know keyframe is 0
		}
	}
	else if (keyframe == numKeyframes-1)
	{
		if (newtime > times[numKeyframes-2].time)
		{
			times[keyframe].time = newtime;
			return keyframe;
		}
	}
	else if (newtime > times[keyframe-1].time && newtime <= times[keyframe+1].time)
	{
		times[keyframe].time = newtime;
		return keyframe;
	}
	//else we have to rearrange!
	tmp = times[keyframe];
	tmp.time = newtime;
	
	TOD_RemoveKeyframe(keyframe);
	keyframe = TOD_AddKeyframe(newtime);
	times[keyframe] = tmp;
	return keyframe;
}

void	TOD_Init()
{
	int i;

	for (i = 0; i < MAX_KEYFRAMES; i++)
		times[i].active = 0;

	for (i = 0; i < MAX_VARIABLES; i++)
		vars[i] = NULL;

	Cmd_Register("tod_addvar", TOD_AddVar_Cmd);
	Cmd_Register("tod_set", TOD_SetValue_Cmd);
	Cmd_Register("tod_summary", TOD_Summary_Cmd);

	if (DLLTYPE == DLLTYPE_EDITOR)
	{
		Cmd_Register("tod_addkeyframe", TOD_AddKeyframe_Cmd);
		Cmd_Register("tod_delkeyframe", TOD_RemoveKeyframe_Cmd);
		Cmd_Register("tod_save", TOD_Save_Cmd);
	}

	Cvar_Register(&tod_interpolate);
}

