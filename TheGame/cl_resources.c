// (C) 2001 S2 Games

// cl_resources.c

#include "client_game.h"

clientResources_t res;

cvar_t	cl_animTransitionTime = { "cl_animTransitionTime", "0.1" };		//0.1 seconds to transition into different animations
cvar_t	cl_precacheAll = { "cl_precacheAll", "1" };

extern cvar_t	*gui_basepath;

void 	CL_LoadInterfaceResources()
{
	res.connectionProblemShader = corec.Res_LoadShader("/textures/icons/network.tga");
}



/*==========================

  CL_LoadEffectResources

 ==========================*/

void	CL_LoadEffectResources(int index)
{
	int n;

	if (index < 1 || index >= MAX_EFFECTS)
		return;

	if (effectData[index].loopSound[0])
		corec.Res_LoadSound(effectData[index].loopSound);

	for (n = 0; n < 4; n++)
	{
		if (effectData[index].sound[n][0])
			corec.Res_LoadSound(effectData[index].sound[n]);
	}

	for (n = 0; n < VISUALS_PER_EFFECT; n++)
	{
		residx_t	model = 0;

		if (effectData[index].visuals[n].model[0])
			model = corec.Res_LoadModel(effectData[index].visuals[n].model);

		if (effectData[index].visuals[n].shader[0])
			corec.Res_LoadShader(effectData[index].visuals[n].shader);

		if (model && effectData[index].visuals[n].skin[0])
			corec.Res_LoadSkin(model, effectData[index].visuals[n].skin);
	}
}


/*==========================

  CL_LoadStateResources

 ==========================*/

void	CL_LoadStateResources(int index)
{
	static int	loadedstates = 0;

	int m = 0;

	if (index < 1 || index >= MAX_STATES)
		return;

	if (!stateData[index].active)
		return;

	//protect from infinite recursion
	if (loadedstates & (1 << index))
		return;

	loadedstates |= (1 << index);

	CL_LoadEffectResources(stateData[index].effect);

	if (strlen(stateData[index].icon) > 0)
		corec.Res_LoadShader(stateData[index].icon);

	//FIXME: needs to account for different objects
	/*for (n = 0; n < 5; n++)
	{
		if (strlen(stateData[index].model[n]) > 0)
			m = corec.Res_LoadModel(stateData[index].model[n]);

		if (m && strlen(stateData[index].skin[n]) > 0)
			corec.Res_LoadSkin(m, stateData[index].skin[n]);
	}*/

	CL_LoadStateResources(stateData[index].radiusState);

	if (strlen(stateData[index].singleShader) > 0)
		corec.Res_LoadShader(stateData[index].singleShader);

}


/*==========================

  CL_LoadObjectResources

 ==========================*/

void	CL_LoadObjectResources(byte objnum)
{
	int i, m = 0;
	objectData_t	*objdata = CL_ObjectType(objnum);

	//main model/skin

	if (strlen(objdata->model) > 0)
		res.objres[objnum].model = corec.Res_LoadModel(objdata->model);

	res.objres[objnum].skin[SKIN_TEAM1] = corec.Res_LoadSkin(res.objres[objnum].model, objdata->skin);
	//load the team 2 skin
	res.objres[objnum].skin[SKIN_TEAM2] = corec.Res_LoadSkin(res.objres[objnum].model, "team2");

	//weapon model
	if (strlen(objdata->handModel) > 0)
		res.objres[objnum].handModel = corec.Res_LoadModel(objdata->handModel);
	if (strlen(objdata->handSkin) > 0)
		res.objres[objnum].handSkin	= corec.Res_LoadSkin(res.objres[objnum].handModel, objdata->handSkin);


	//projectile models
	if (objdata->objclass == OBJCLASS_WEAPON || objdata->objclass == OBJCLASS_ITEM)
	{
		if (strlen(objdata->projectileModel) > 0)
			res.objres[objnum].projectileModel = corec.Res_LoadModel(objdata->projectileModel);
		if (strlen(objdata->projectileSkin) > 0)
			res.objres[objnum].projectileSkin	= corec.Res_LoadSkin(res.objres[objnum].projectileModel, objdata->projectileSkin);
	}

	//melee models
	if (strlen(objdata->leftMeleeModel) > 0)
		corec.Res_LoadModel(objdata->leftMeleeModel);

	if (strlen(objdata->rightMeleeModel) > 0)
		corec.Res_LoadModel(objdata->rightMeleeModel);

	//for upgrades
	if (strlen(objdata->proximitySound) > 0)
		corec.Res_LoadSound(objdata->proximitySound);

	//icons and such
	if (strlen(objdata->selectionIcon) > 0)
		corec.Res_LoadShader(objdata->selectionIcon);

	if (strlen(objdata->selectionSound) > 0)
		corec.Res_LoadShader(objdata->selectionSound);

	//effects
	for (i = 0; i < NUM_EVENTS; i++)
		CL_LoadEffectResources(objdata->effects[i]);

	CL_LoadEffectResources(objdata->activeEffect);
	CL_LoadEffectResources(objdata->delayEffect);

	for (i = 0; i < BUILDING_HEALTH_STAGES; i++)
		CL_LoadEffectResources(objdata->healthStageEffects[i]);

	CL_LoadEffectResources(objdata->projectileEffect);
	CL_LoadEffectResources(objdata->trailEffect);
	CL_LoadEffectResources(objdata->flybyEffect);

	//states
	CL_LoadStateResources(objdata->radiusState);

	res.objres[objnum].loaded = true;
}


int		CL_Skin(byte objtype, int team)
{
	if (!res.objres[objtype].loaded)
		CL_LoadObjectResources(objtype);

	if (CL_ObjectType(objtype)->objclass == OBJCLASS_WEAPON)
		return res.objres[objtype].projectileSkin;
	else
	{
		if (team == 2)
			return res.objres[objtype].skin[1];
		else
			return res.objres[objtype].skin[0];
	}
}

residx_t	CL_Model(byte objnum)
{
	if (!res.objres[objnum].loaded)
		CL_LoadObjectResources(objnum);

	if (CL_ObjectType(objnum)->objclass == OBJCLASS_WEAPON)
		return res.objres[objnum].projectileModel;
	else
		return res.objres[objnum].model;
}

residx_t	CL_WeaponModel(byte objnum)
{
	if (!res.objres[objnum].loaded)
		CL_LoadObjectResources(objnum);

	return res.objres[objnum].model;
}

residx_t	CL_WeaponSkin(byte objnum)
{
	if (!res.objres[objnum].loaded)
		CL_LoadObjectResources(objnum);

	return res.objres[objnum].skin[0];
}

residx_t	CL_HandModel(byte objnum)
{
	if (!res.objres[objnum].loaded)
		CL_LoadObjectResources(objnum);

	return res.objres[objnum].handModel;
}

int		CL_HandSkin(byte objtype, int team)
{
	if (!res.objres[objtype].loaded)
		CL_LoadObjectResources(objtype);

	return res.objres[objtype].handSkin;
}

residx_t	CL_StateModel(byte objtype, int modelnum, int statenum, int param)
{
	char	fname[_MAX_PATH];

	strcpy(fname, fmt("%s%s.model", Filename_GetDir(CL_ObjectType(objtype)->model), stateData[statenum].model[modelnum]));

	return corec.Res_LoadModel(fname);
}

int	CL_StateSkin(byte objtype, int statenum, int param)
{
	return 0;		//use default skin
}



/*==========================

  CL_PrecacheResources()

  called by the core engine when it's time to load resources

  all server state strings have arrived at this point and the world has just been loaded

 ==========================*/

extern cvar_t int_crosshairShader;

void	CL_PrecacheResources()
{
	int n;
	int r;
	int races[MAX_TEAMS];
	int numRaces=0;

	if (!cl.gotObjectTypes)
		core.Game_Error("Started precaching resources without object type data!\n");

	CL_LoadInterfaceResources();

	corec.Res_LoadShader(int_crosshairShader.string);

	res.minimap = corec.Res_LoadShaderEx(corec.Cvar_GetString("world_overhead"), SHD_FULL_QUALITY);

	res.mainCursor.shader = corec.Res_LoadShaderEx("/textures/cursors/arrow.tga", SHD_NO_MIPMAPS | SHD_FULL_QUALITY);
	res.errorCursor.shader = corec.Res_LoadShaderEx("/textures/cursors/red_x.tga", SHD_NO_MIPMAPS | SHD_FULL_QUALITY);	
	res.unknownCursor.shader = corec.Res_LoadShaderEx("/textures/cursors/unknown.tga", SHD_NO_MIPMAPS | SHD_FULL_QUALITY);	
	res.crosshairCursor.shader = corec.Res_LoadShaderEx("/textures/cursors/crosshair.tga", SHD_NO_MIPMAPS | SHD_FULL_QUALITY);
	res.crosshairCursor.hotspotx = 0.5;		//put the hotspot in the middle of the crosshair
	res.crosshairCursor.hotspoty = 0.5;		//put the hotspot in the middle of the crosshair
	Rend_SetMouseCursor(&res.mainCursor);

	res.blackShader = corec.Res_LoadShaderEx("/textures/black.tga", SHD_NO_MIPMAPS | SHD_FULL_QUALITY);
	res.shadowShader = corec.Res_LoadShader("/textures/effects/shadow/shadow.tga");
	res.longshadowShader = corec.Res_LoadShader("/textures/effects/shadow/longshadow.tga");
	res.spotshadowShader = corec.Res_LoadShader("/textures/effects/shadow/spotshadow.tga");
	res.lightbeamShader = corec.Res_LoadShader("/textures/effects/1_nl_beam.tga");
	res.lightbeamRedShader = corec.Res_LoadShader("/textures/effects/1_nl_beamred.tga");

	res.selectionIndicatorSmallShader = corec.Res_LoadShaderEx(Tex("selection_indicator_small"), SHD_FULL_QUALITY);
	res.selectionIndicatorLargeShader = corec.Res_LoadShaderEx(Tex("selection_indicator_large"), SHD_FULL_QUALITY);
	
	res.whiteShader = corec.GetWhiteShader();

	res.friendlyIcon =	corec.Res_LoadShaderEx("/textures/icons/nl_friendly.tga", SHD_FULL_QUALITY);
	res.reviveIcon =	corec.Res_LoadShaderEx("/textures/icons/nl_revive.tga", SHD_FULL_QUALITY);
	res.voiceIcon =		corec.Res_LoadShaderEx("/textures/icons/nl_voice.tga", SHD_FULL_QUALITY);

	res.waypointMoveIcon = corec.Res_LoadShaderEx("/textures/icons/waypoint_default.tga", SHD_FULL_QUALITY);
	res.waypointAttackIcon = corec.Res_LoadShaderEx("/textures/icons/waypoint_attack.tga", SHD_FULL_QUALITY);

	//build a list of races being played
	for (n=ST_TEAM_INFO + 1; n<=ST_TEAM_INFO + MAX_TEAMS; n++)
	{
		char s[1024];
		corec.Client_GetStateString(n, s, 1024);
		races[numRaces++] = atoi(ST_GetState(s, "r"));
	}

	for (n = 0; n < numRaces; n++)
		CL_LoadStateResources(raceData[races[n]].officerState);
	
	if (cl_precacheAll.integer)
	{
		for (n = 1; n < MAX_OBJECT_TYPES; n++)
		{
			if (CL_ObjectType(n)->race != 0)
			{
				for (r=0; r<numRaces; r++)
				{
					if (CL_ObjectType(n)->race == races[r])
						break;
				}
	
				if (r == numRaces)
					continue;			//don't load this resource if the race is not being played
			}

			CL_LoadObjectResources((byte)n);
			//touch icon
			corec.Res_LoadShader(fmt("%s.tga", CL_ObjectType(n)->icon));
			corec.Res_LoadShader(fmt("%s_down.tga", CL_ObjectType(n)->icon));
			corec.Res_LoadShader(fmt("%s_notavail.tga", CL_ObjectType(n)->icon));
			corec.Res_LoadShader(fmt("%s_over.tga", CL_ObjectType(n)->icon));
		}
	}
}

void	cl_resReload_Cmd(int argc, char *argvp[])
{
	int index;

	corec.Console_Printf("Flagging client data for reload...\n");
	
	for (index = 0; index < MAX_OBJECT_TYPES; index++)
		res.objres[index].loaded = false;
}

void	CL_InitResources()
{
	corec.Cmd_Register("cl_resReload",	cl_resReload_Cmd);

	core.Cvar_Register(&cl_animTransitionTime);
	core.Cvar_Register(&cl_precacheAll);
}
