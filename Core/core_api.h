/* 
 *
 *  Core engine interface functions
 *
 *
 */

typedef struct
{
	//system

	void		(*System_Dir)(char *directory, char *wildcard, bool recurse, void(*dirCallback)(const char *dir, void *userdata), void(*fileCallback)(const char *filename, void *userdata), void *userdata);
	
	//cmd

//swapping these functions order in the demo, to severely break demo/reailt exe/dll compatability
#ifdef SAVAGE_DEMO
	void		(*Cmd_BufPrintf)(const char *fmt, ...);
	void		(*Cmd_Register)(const char *name, cmdfunc_t cmdfunc);
#else	//SAVAGE_DEMO
	void		(*Cmd_Register)(const char *name, cmdfunc_t cmdfunc);
	void		(*Cmd_BufPrintf)(const char *fmt, ...);
#endif	//SAVAGE_DEMO

//	void		(*Cmd_ExecBuf)();
	void		(*Cmd_Exec)(const char *cmd);
	void		(*Cmd_FlushScriptBuffer)();
	void		(*Cmd_ReadConfigFileVars)(const char *filename, void(*callback)(char *varname, char *string, void *userdata), void *userdata);
	void		(*Cmd_Enable)(const char *name);
	void		(*Cmd_Disable)(const char *name);

	//console variables
	void		(*Cvar_AllowCheats)();
	void		(*Cvar_BlockCheats)();

	void		(*Cvar_Register)(cvar_t *var);
	void		(*Cvar_Set)(const char *name, const char *string);
	void		(*Cvar_SetVar)(cvar_t *var, const char *string);
	void		(*Cvar_SetValue)(const char *name, float value);
	float		(*Cvar_GetValue)(const char *name);
	char		*(*Cvar_GetString)(const char *name);
	int			(*Cvar_GetInteger)(const char *name);
	void		(*Cvar_SetVarValue)(cvar_t *var, float value);
	void		(*Cvar_GetModifiedCount)(const char *name);

	//media
	residx_t	(*Res_LoadStringTable)(const char *name);

	//string table
	char		*(*Str_Get)(residx_t stringtable, const char *stringname);


	//console printing

	//normal messages
	void		(*Console_Printf)(const char *fmt, ...);
	//debug messages
	void		(*Console_DPrintf)(const char *fmt, ...);
	//error messages
	void		(*Console_Errorf)(const char *fmt, ...);
	
	//host functions

	int			(*Milliseconds)();	
	float		(*FrameSeconds)();
	int			(*FrameMilliseconds)();
	float		(*GetLogicFPS)();
	double		(*System_GetPerfCounter)();

	residx_t	(*GetSavageLogoShader)();
	residx_t	(*GetWhiteShader)();
	residx_t	(*GetSysfontShader)();

	//file management
	file_t		*(*File_Open)(const char *filename, const char *mode);
#ifdef unix
	file_t		*(*File_OpenAbsolute)(const char *filename, const char *mode);
#endif
	size_t 		(*File_Read)(void *ptr, size_t size, size_t nmemb, file_t *file);
	size_t 		(*File_Write)(const void *ptr, size_t size, size_t nmemb, file_t *file);
	void		(*File_Close)(file_t *f);
	char    	(*File_getc)(file_t *file);
	char    	*(*File_gets)(char *s, int size, file_t *f);
	
	bool		(*File_Exists)(const char *filename);
	void		(*File_Printf)(file_t *f, const char *fmt, ...);
	char		*(*File_GetNextFileIncrement)(int num_digits, const char *basename, const char *ext, char *filename, int size);
	char		*(*File_GetCurrentDir)();

	void    	(*IRC_OutputCallback)(void (*outputcallback)(char *string));
	void    	(*IRC_OutputMsgCallback)(void (*outputcallback)(char *user, char *string, bool incoming));
	char 		**(*IRC_GetUserList)();
	char 		**(*IRC_GetBuddyList)();

	// memory allocators

	void*		(*Allocator_Allocate)(allocator_t* allocator, int size);
	void		(*Allocator_Deallocate)(allocator_t* allocator, void* pv);

	void*		(*Tag_Malloc)(int size);
	void		(*Tag_Free)(void *mem);

	/*void		(*Mem_ResetCopyCount)();
	long		(*Mem_GetBytesCopied)();*/

	void		(*Game_Error)(const char *msg, ...);
} coreAPI_shared_t;


typedef struct
{
	//system

	void		(*System_Dir)(char *directory, char *wildcard, bool recurse, void(*dirCallback)(const char *dir, void *userdata), void(*fileCallback)(const char *filename, void *userdata), void *userdata);

	//cmd

	void		(*Cmd_Register)(const char *name, cmdfunc_t cmdfunc);
	void		(*Cmd_BufPrintf)(const char *fmt, ...);
	//void		(*Cmd_ExecBuf)();
	void		(*Cmd_FlushScriptBuffer)();
	void		(*Cmd_Exec)(const char *cmd);
	void		(*Cmd_ReadConfigFileVars)(const char *filename, void(*callback)(char *varname, char *string, void *userdata), void *userdata);
	void		(*Cmd_Enable)(const char *name);
	void		(*Cmd_Disable)(const char *name);
	void    	(*Cmd_ProcessString)(char *str);



	//console variables
	void		(*Cvar_Register)(cvar_t *var);
	void		(*Cvar_Set)(const char *name, const char *string);
	void		(*Cvar_SetVar)(cvar_t *var, const char *string);
	void		(*Cvar_SetValue)(const char *name, float value);
	float		(*Cvar_GetValue)(const char *name);
	int			(*Cvar_GetInteger)(const char *name);
	char		*(*Cvar_GetString)(const char *name);
	void		(*Cvar_SetVarValue)(cvar_t *var, float value);
	int			(*Cvar_GetModifiedCount)(const char *name);



	//console printing

	//normal messages
	void		(*Console_Printf)(const char *fmt, ...);
	//debug messages
	void		(*Console_DPrintf)(const char *fmt, ...);
	//error messages
	void		(*Console_Errorf)(const char *fmt, ...);

	//world management

	void		(*World_SampleGround)(float xpos, float ypos, pointinfo_t *result);
	void		(*World_GetBounds)(vec3_t bmin, vec3_t bmax);
	heightmap_t	(*World_GetGridHeight)(int xpos, int ypos);
	void		(*World_Destroy)();
	void		(*World_DeformGround)(int xpos, int ypos, heightmap_t newheight);
//	void		(*World_PlaceSolid)(solid_t *solid);
//	void		(*World_RemoveSolid)(solid_t *solid);
//	int			(*World_TestBox)(const vec3_t bpos, const vec3_t bext, solid_t *solidlist[]);
//	void		(*World_TraceLineSegment)(traceinfo_t *result, const vec3_t start, const vec3_t end, bool testEntities, bool testTerrain, const solid_t *exclude);
//	int			(*World_TestSolid)(solid_t *solid, solid_t *solidlist[]);
	bool		(*World_TraceBox)(traceinfo_t *result, const vec3_t start, const vec3_t end, const vec3_t bmin, const vec3_t bmax, int ignoreSurface);
	bool		(*World_TraceBoxEx)(traceinfo_t *result, const vec3_t start, const vec3_t end, const vec3_t bmin, const vec3_t bmax, int ignoreSurface, int ignoreIndex);
	void		(*World_LinkObject)(baseObject_t *obj);
	void		(*World_UnlinkObject)(baseObject_t *obj);
	void		(*World_FitPolyToTerrain)(scenefacevert_t *verts, int num_verts, residx_t shader, int flags, void (*clipCallback)(int num_verts, scenefacevert_t *verts, residx_t shader, int flags));
	char		*(*World_GetName)();
	bool        (*World_UseColormap)(bool use);
	float		(*World_CalcMaxZ)(float x1, float y1, float x2, float y2);
	float		(*WorldToGrid)(float coord);
	float		(*GridToWorld)(float gridcoord);
	void		(*World_GetOccluder)(int num, occluder_t *out);
	bool		(*World_UpdateOccluder)(int num, const occluder_t *occ);
	int			(*World_GetNumOccluders)();
	void		(*World_ClearOccluders)();
	int			(*World_AddOccluder)(occluder_t *out);
	void		(*World_RemoveOccluder)(int num);

	//world render

	void		(*WR_GetSunVector)(vec3_t vec);
	void		(*WR_ClearDynamap)();
	void		(*WR_ClearDynamapToColor)(bvec4_t color);
	void        (*WR_ClearDynamapToColorEx)(bvec4_t color, bvec4_t conditional);
	void		(*WR_SetDynamap)(int x, int y, bvec4_t color);
	void        (*WR_GetDynamap)(int gridx, int gridy, bvec4_t color);

	void		(*WR_SetColormap)(int x, int y, bvec4_t color);
	void		(*WR_GetColormap)(int x, int y, bvec4_t color);

	void		(*WR_PaintShader)(int x, int y, residx_t shader, bool layer2);

	float		(*WR_FarClip)();

//	void		(*WR_FitQuad)(scenefacevert_t quadverts[4], residx_t shader, int flags);

	int			(*WO_GetNumObjdefs)();
	char		*(*WO_GetObjdefName)(int n);
	int			(*WO_RegisterObjectVar)(const char *objclass, const char *varname);
	char		*(*WO_GetObjdefVar)(int objdef_id, const char *varname);
	float		(*WO_GetObjdefVarValue)(int objdef_id, const char *varname);
	bool		(*WO_UseObjdef)(const char *objdefName);
	bool		(*WO_GetObjectPos)(int id, objectPosition_t *objpos);
	residx_t	(*WO_GetObjectSound)(int id);
	void		(*WO_CreateObjectClass)(const char *objclass);
	bool		(*WO_CreateObject)(int objdef, const char *optmodel, const char *optskin, objectPosition_t *pos, int id);
 	void		(*WO_ClearObjects)();
	bool		(*WO_GetObjectBounds)(int id, vec3_t bmin, vec3_t bmax);
	void		(*WO_DeleteObject)(int id);
	int         (*WO_GetObjectObjdef)(int id);
	residx_t	(*WO_GetObjectModel)(int id);
	residx_t	(*WO_GetObjectSkin)(int id);
	int			(*WO_GetObjdefId)(const char *name);
	bool		(*WO_IsObjectDoubleSided)(int id);
	bool		(*WO_IsObjectReference)(int id);
	void		(*WO_SetObjectModelAndSkin)(int id, residx_t model, residx_t skin);
	void        (*WO_RenderObjects)();

	// input
	void    	(*Input_Event)(int key, int rawchar, bool down);
	void		(*Input_SetMouseMode)(int mode);
	int			(*Input_GetMouseMode)();
	bool		(*Input_IsKeyDown)(int key);
	void		(*Input_CenterMouse)();
	void		(*Input_SetMouseAngles)(const vec3_t angles);
	void		(*Input_SetMouseXY)(int x, int y);
	void		(*Input_GetMousePos)(mousepos_t *mp);
	int     	(*Input_GetButtons)();
	bool		(*Input_ActivateInputCallback)(inputcallback_t func, bool binds_active, gui_element_t *widget);
	bool		(*Input_DeactivateInputCallback)(bool binds_active);
	void		*(*Input_GetCallbackWidget)();
	char    	*(*Input_GetNameFromKey)(int key);
	int     	(*Input_GetKeyFromName)(char *keyname);
	char		*(*Input_GetBindDownCmd)();
	char		*(*Input_GetBindUpCmd)();
	void		(*Input_SetSensitivityScale)(float scale);


	//scene management

	void		(*Scene_AddObject)(sceneobj_t *scobj);	
	void		(*Scene_AddLight)(scenelight_t *light);
	void		(*Scene_AddOccluder)(occluder_t *occluder);
	void		(*Scene_Clear)();
	void		(*Scene_Render)(camera_t *camera, vec3_t userpos);
	void		(*Scene_AddPoly)(int nverts,  scenefacevert_t *verts, residx_t shader, int flags);
	void		(*Scene_AddSkyObj)(sceneobj_t *sky);
	int			(*Scene_SelectObjects)(camera_t *camera, int rx, int ry, int rw, int rh, int *buf, int bufsize);
	void		(*Scene_SetFrustum)(camera_t *camera);
	bool		(*Scene_ObjectInFrustum)(sceneobj_t *obj);
	
	//2d rendering

	void		(*Draw_SetShaderTime)(float time);
	void		(*Draw_SetColor)(vec4_t color);
	void		(*Draw_Quad2d)(float x, float y, float w, float h, float s1, float t1, float s2, float t2, residx_t shader);
	void		(*Draw_Poly2d)(vec2_t v1, vec2_t v2, vec2_t v3, vec2_t v4, float s1, float t1, float s2, float t2, residx_t shader);
	void		(*Draw_RotatedQuad2d)(float x, float y, float w, float h, float ang, float s1, float t1, float s2, float t2, residx_t shader);

	//color

	void		(*Color_Clamp)(vec4_t color);
	void		(*Color_ToFloat)(bvec4_t color, vec4_t out);
	void		(*Color_ToByte)(vec4_t color, bvec4_t out);

	//vid functions

	int			(*Vid_GetScreenW)();
	int			(*Vid_GetScreenH)();
	void		(*Vid_ProjectVertex)(camera_t *cam, vec3_t vertex, vec2_t result);
	void		(*Vid_ReadZBuffer)(int x, int y, int w, int h, float *zpixels);

	//camera functions

	void		(*Cam_ConstructRay)(camera_t *camera, int winx, int winy, vec3_t out);

	//host functions

	int			(*System_Milliseconds)();
	int			(*Milliseconds)();	
	float		(*FrameSeconds)();
	int			(*FrameMilliseconds)();
	float		(*GetLogicFPS)();

	residx_t	(*GetSavageLogoShader)();
	residx_t	(*GetWhiteShader)();
	residx_t	(*GetSysfontShader)();
	residx_t	(*GetNicefontShader)();

	//resources

	bool		(*Res_GetModelSurfaceBounds)(residx_t model, vec3_t bmin, vec3_t bmax);
	residx_t	(*Res_LoadModel)(const char *name);
	residx_t	(*Res_LoadSound)(const char *name);
	int			(*Res_LoadSkin)(residx_t model, const char *name);
	residx_t	(*Res_LoadShader)(const char *name);
	residx_t	(*Res_LoadShaderEx)(const char *name, int flags);
	residx_t	(*Res_LoadTerrainShader)(const char *name);
	void		(*Res_GetModelVisualBounds)(residx_t model, vec3_t bmin, vec3_t bmax);
	char		*(*Res_IsShaderFile)(char *name);
	int			(*Res_GetNumTextureFrames)(residx_t shader);
	int			(*Res_GetAnimatedTextureFPS)(residx_t shader);
	residx_t    (*Res_GetDynamapShader)(residx_t shader);
	int			(*Res_GetMoviePlayCount)(residx_t shadernum);
	void		(*Res_BinkStop)(residx_t shadernum);
	void		(*Res_BinkContinue)(residx_t shadernum);
	void		(*Res_BinkRestart)(residx_t shadernum);
	void		(*Res_BinkUnload)(residx_t shadernum);
	void		(*Res_TheoraStop)(residx_t shadernum);
	void		(*Res_TheoraContinue)(residx_t shadernum);
	void		(*Res_TheoraRestart)(residx_t shadernum);
	void		(*Res_TheoraUnload)(residx_t shadernum);


	//bitmap

	bool		(*Bitmap_Load)(const char *filename, bitmap_t *bmp);
	void		(*Bitmap_GetColor)(const bitmap_t *bmp, int x, int y, bvec4_t color);
	void		(*Bitmap_Free)(bitmap_t *bmp);
	void		(*Bitmap_Scale)(const bitmap_t *bmp, bitmap_t *out, int width, int height);
	void		(*Bitmap_GenerateThumbnails)(char *dirname, int size);
	void		(*Bitmap_SetPixel4b)(bitmap_t *bmp, int x, int y, byte r, byte g, byte b, byte a);

	//sound

	unsigned int	(*Sound_Play)(residx_t sampleidx, int sourcenum, const vec3_t pos, float volume, int channel, int priority);
	void			(*Sound_ClearLoopingSounds)();
	void			(*Sound_AddLoopingSound)(residx_t sampleidx, int sourcenum, int priority);
	void 			(*Sound_StopSource)(int sourcenum);
	void			(*Sound_StopHandle)(unsigned int handle);
	void			(*Sound_StopChannel)(int channel);
	void			(*Sound_MoveSource)(int sourcenum, vec3_t pos, vec3_t velocity);

	void 			(*Sound_SetListenerPosition)(const vec3_t pos, const vec3_t forward, const vec3_t up, bool warp);

	void			(*Sound_PlayMusic)(const char *streamFile);
	void			(*Sound_StopMusic)();

	//GUI
	void	    *(*GUI_GetUserdata)(char *class_name, gui_element_t *obj);
	void	    *(*GUI_SetUserdata)(char *class_name, gui_element_t *obj, void *user_data);
	void	    *(*GUI_RegisterClass)(char *class_name, void *(*create)(gui_element_t *obj, int argc, char *argv[]));

	char		*(*GUI_SetName)(gui_element_t *obj, char *name);
	char		*(*GUI_SetClass)(gui_element_t *obj, char *class_name);

	void		(*GUI_Notify)(int argc, char *argv[]);

	bool		(*GUI_IsVisible)(gui_element_t *obj);
	bool		(*GUI_IsInteractive)(gui_element_t *obj);

	void		(*GUI_Exec)(char *command);
	void		(*GUI_Move)(gui_element_t *obj, int x, int y);
	void		(*GUI_Resize)(gui_element_t *obj, int w, int h);
	void		(*GUI_Show)(gui_element_t *obj);
	void		(*GUI_Hide)(gui_element_t *obj);
	void		(*GUI_Unfocus)(gui_element_t *obj);
	void		(*GUI_Focus)(gui_element_t *obj);
	bool		(*GUI_Select)(gui_element_t *obj);
	void		(*GUI_Param)(gui_element_t *obj, int argc, char *argv[]);
	void		(*GUI_UpdateDims)();
	void		(*GUI_GetPosition)(gui_element_t *obj, ivec2_t pos);
	void		(*GUI_GetSize)(gui_element_t *obj, ivec2_t size);

	void        (*GUI_FadeIn)(gui_element_t *obj, float time);
	void        (*GUI_FadeOut)(gui_element_t *obj, float time);
	gui_element_t *(*GUI_GetWidgetAtXY)(int x, int y);
	bool		(*GUI_CheckMouseAgainstUI)(int x, int y);
	bool		(*GUI_SendMouseDown)(mouse_button_enum button, int x, int y);
	bool		(*GUI_SendMouseUp)(mouse_button_enum button, int x, int y);
	bool    	(*GUI_WidgetWantsMouseClicks)(mouse_button_enum button, int x, int y, bool down);
	bool    	(*GUI_WidgetWantsKey)(int key, bool down);
	void		(*GUI_ResetFocus)();
	void    	(*GUI_ActivateNextInteractiveWidget)();

    void        (*GUI_RotatedQuad2d_S)(int x, int y, int w, int h, float ang, residx_t shader);
	void		(*GUI_Quad2d_S)(int x, int y, int w, int h, residx_t shader);
	void		(*GUI_ShadowQuad2d_S)(int x, int y, int w, int h, residx_t shader);
	void    	(*GUI_GetStringRowsCols)(char *str, int *rows, int *cols);
	void		(*GUI_DrawString)(int x, int y, const char *string, int charHeight, int iconHeight, int maxRows, int maxWidth, residx_t fontshader);
	void		(*GUI_DrawShadowedString)(int x, int y, const char *string, int charHeight, int iconHeight, int maxRows, int maxWidth, residx_t fontshader, float r, float g, float b, float a);
	void		(*GUI_DrawString_S)(int x, int y, const char *string, int charHeight, int iconHeight, int maxRows, int maxWidth, residx_t fontshader);
	void 		(*GUI_DrawStringBillboard)(camera_t *cam, vec3_t pos, const char *string, int charHeight, int maxRows, residx_t fontshader);
	void    	(*GUI_DrawStringMonospaced)(int x, int y, const char *string, int charWidth, int charHeight, int maxRows, int maxChars, residx_t fontshader);
	int			(*GUI_StringWidth)(const char *string, int charHeight, int iconHeight, int maxRows, int maxChars, residx_t fontshader);
	void		(*GUI_LineBox2d_S)(int x, int y, int w, int h, int thickness);	

	void		(*GUI_SetRGB)(float r, float g, float b);
	void		(*GUI_SetRGBA)(float r, float g, float b, float a);

	void		(*GUI_Panel_AddObject)(gui_element_t *object);
	void		(*GUI_Panel_Move)(gui_panel_t *panel, int x, int y);
	void		(*GUI_Panel_MoveRelative)(gui_panel_t *panel, int x, int y);
	void		(*GUI_Panel_Hide)(gui_panel_t *panel);
	void		(*GUI_Panel_Show)(gui_panel_t *panel);
	gui_panel_t *(*GUI_GetPanel)(char *name);

	void			(*GUI_AddElement)(gui_element_t *elem);
	void            (*GUI_GetScreenPosition)(gui_element_t *obj, ivec2_t pos);
	void            (*GUI_GetScreenSize)(gui_element_t *obj, ivec2_t size);
	gui_element_t   *(*GUI_GetObject)(char *hierarchy);
	void			*(*GUI_GetClass)(char *hierarchy, char *classname);
	void			(*GUI_List_Cmd)(int argc, char *argv[]);

	void		(*GUI_Draw)();
	void		(*GUI_Init)();

	void		*(*GUI_Malloc)(int size);
	void		(*GUI_Free)(void *ptr);
	void		*(*GUI_ReAlloc)(void *ptr, int size);
	char		*(*GUI_StrDup)(const char *string);

	void		(*GUI_Coord)(int *x, int *y);
	void		(*GUI_ConvertToScreenCoord)(int *x, int *y);
	void		(*GUI_ScaleToScreen)(int *w, int *h);
	void		(*GUI_Scale)(int *w, int *h);

	int			(*GUI_GetScreenWidth)();
	int			(*GUI_GetScreenHeight)();

	void		(*GUI_HideAllPanels)();
	void    	(*GUI_DestroyAll)();



/*	void		(*Particles_Reset)(int particle_id, float time);
	int		(*Particles_GetConfigCount)();
	int		(*Particles_GetSystemCount)();
	int		(*Particles_GetBehaviorCount)();
	bool	(*Particles_Contains)( char *name );
	void	(*Particles_DisplayActive)( int argc, char *argv[]);
	int		(*Particles_Allocate)(char *configName);
	void	(*Particles_Free)(int particle_id);
	void	(*Particles_Behavior_Init)();
	int		(*Particles_UpdateConfig)(int sysIndex, int configIndex);
	int		(*Particles_MakeConfig)(char *name);
*/
	void	(*SetTimeOfDay)(float time);
	int     (*TOD_GetNumKeyframes)();
	int		(*TOD_GetKeyframeTime)(int keyframe);
	int		(*TOD_MoveKeyframe)(int keyframe, int newtime);


	void	(*Client_SendMessageToServer)(char *buf);
	void	(*Client_RequestStateString)(int id, int num);
	void	(*Client_GameObjectPointer)(void *base, int stride, int num_objects);
	void	(*Client_GetInputState)(unsigned int num, inputState_t *is);
	unsigned int (*Client_GetCurrentInputStateNum)();
	void	(*Client_GetRealPlayerState)(playerState_t *ps);
	int		(*Client_GetOwnClientNum)();
	bool	(*Client_ObjectsUpdated)(int *updatetime);
	void	(*Client_ModifyInputStateItem)(byte item);		//right now 'item' is the only thing we need to modify
	void	(*Client_ModifyInputStateAngles)(short pitch, short yaw);
	void	(*Client_GetStateString)(int id, char *buf, int size);
	int		(*Client_GetConnectionState)();

	bool	(*Client_ConnectionProblems)();

	bool	(*Client_IsPlayingDemo)();

	int     (*MasterServer_GetGameList)();
	int		(*MasterServer_GetGameList_Frame)(server_info_t *servers, int max_servers, int *num_servers);
	bool    (*MasterServer_GetGameInfo)(char *address, int port, int *ping, char **coreInfo, char **gameInfo);
	int     (*MasterServer_FindUsers)(char *name);
	int		(*MasterServer_GetUserList_Frame)(user_info_t *users, int max_users, int *num_users);
	int     (*MasterServer_GetUserInfo)(char *cookie, char *server, int port);
	int		(*MasterServer_GetUserInfo_Frame)(char *info, int maxlen);

	//skeletal animation
	void	(*Geom_BeginPose)(skeleton_t *skeleton, residx_t refmodel);
	int		(*Geom_SetBoneAnim)(const char *parentBone, const char *animName, float animTime, int currentTime, int blendTime, int eventChannel);	
	void	(*Geom_SetBoneAnimNoPose)(const char *animName, float animTime, int currentTime, int eventChannel);	
	void	(*Geom_SetBoneState)(const char *boneName, int posestate);
	void	(*Geom_RotateBone)(skeleton_t *skeleton, const char *boneName, float pitch_offset, float roll_offset, float yaw_offset, bool multiply);
	void	(*Geom_EndPose)();
	int		(*Geom_GetBoneIndex)(skeleton_t *skeleton, const char *boneName);
	void	(*Geom_FreeSkeleton)(skeleton_t *skeleton);
//	void	(*Geom_MapSkeleton)(const skeleton_t *in, residx_t model, skeleton_t *out);

//	void	(*Path_DrawPaths)(int model, int skin);
//	void	(*Path_Create)(vec2_t src, vec2_t dest);

	void    (*File_CacheClanIcon)(int clan_id);
	void	(*File_FlushClanIcon)(int clan_id);

	int    (*HTTP_GetText)(char *addr, char *buf, int buf_size);
	int    (*HTTP_GetFile)(char *url, char *filename);
	file_t  *(*HTTP_OpenFile)(char *url);

	void*		(*Tag_Malloc)(int size);
	void		(*Tag_Free)(void *mem);
	
	bool	(*Speak)(const char *string);

	char		*(*TranslateString)(const char *s);
} coreAPI_client_t;

typedef struct
{
	//system

	void		(*System_Dir)(char *directory, char *wildcard, bool recurse, void(*dirCallback)(const char *dir, void *userdata), void(*fileCallback)(const char *filename, void *userdata), void *userdata);

	//math
	void 	(*CopyVec2)(const vec2_t in, vec2_t out);
	void    (*CopyVec3)(const vec3_t in, vec3_t out);
	void    (*CopyVec4)(const vec4_t in, vec4_t out);
	
	//cmd

	void		(*Cmd_Register)(const char *name, cmdfunc_t cmdfunc);
	void		(*Cmd_BufPrintf)(const char *fmt, ...);
	//void		(*Cmd_ExecBuf)();
	void		(*Cmd_Exec)(const char *cmd);
	void		(*Cmd_FlushScriptBuffer)();
	void		(*Cmd_ReadConfigFileVars)(const char *filename, void(*callback)(char *varname, char *string, void *userdata), void *userdata);
	void		(*Cmd_Enable)(const char *name);
	void		(*Cmd_Disable)(const char *name);



	//console variables
	void		(*Cvar_Register)(cvar_t *var);
	void		(*Cvar_Set)(const char *name, const char *string);
	void		(*Cvar_SetVar)(cvar_t *var, const char *string);
	void		(*Cvar_SetValue)(const char *name, float value);
	float		(*Cvar_GetValue)(const char *name);
	int			(*Cvar_GetInteger)(const char *name);
	char		*(*Cvar_GetString)(const char *name);
	void		(*Cvar_SetVarValue)(cvar_t *var, float value);
	int			(*Cvar_GetModifiedCount)(const char *name);



	//console printing

	//normal messages
	void		(*Console_Printf)(const char *fmt, ...);
	//debug messages
	void		(*Console_DPrintf)(const char *fmt, ...);
	//error messages
	void		(*Console_Errorf)(const char *fmt, ...);

	//world management

	void		(*World_SampleGround)(float xpos, float ypos, pointinfo_t *result);
	void		(*World_GetBounds)(vec3_t bmin, vec3_t bmax);
	heightmap_t	(*World_GetGridHeight)(int xpos, int ypos);
	void		(*World_Destroy)();
	void		(*World_DeformGround)(int xpos, int ypos, heightmap_t newheight);
//	void		(*World_PlaceSolid)(solid_t *solid);
//	void		(*World_RemoveSolid)(solid_t *solid);
//	int			(*World_TestBox)(const vec3_t bpos, const vec3_t bext, solid_t *solidlist[]);
//	void		(*World_TraceLineSegment)(traceinfo_t *result, const vec3_t start, const vec3_t end, bool testEntities, bool testTerrain, const solid_t *exclude);
//	int			(*World_TestSolid)(solid_t *solid, solid_t *solidlist[]);
	bool		(*World_TraceBox)(traceinfo_t *result, const vec3_t start, const vec3_t end, const vec3_t bmin, const vec3_t bmax, int ignoreSurface);
	bool		(*World_TraceBoxEx)(traceinfo_t *result, const vec3_t start, const vec3_t end, const vec3_t bmin, const vec3_t bmax, int ignoreSurface, int ignoreIndex);
	void		(*World_LinkObject)(baseObject_t *obj);
	void		(*World_UnlinkObject)(baseObject_t *obj);
	bool		(*World_IsLinked)(baseObject_t *obj);
	float		(*World_CalcMaxZ)(float x1, float y1, float x2, float y2);
	char		*(*World_GetName)();
	float		(*WorldToGrid)(float coord);
	float		(*GridToWorld)(float gridcoord);

	int			(*WO_GetNumObjdefs)();
	char		*(*WO_GetObjdefName)(int n);
	int			(*WO_RegisterObjectVar)(const char *objclass, const char *varname);
	char		*(*WO_GetObjdefVar)(int objdef_id, const char *varname);
	float		(*WO_GetObjdefVarValue)(int objdef_id, const char *varname);
	bool		(*WO_UseObjdef)(const char *objdefName);
	bool		(*WO_GetObjectPos)(int id, objectPosition_t *objpos);
	void		(*WO_CreateObjectClass)(const char *objclass);
	bool		(*WO_CreateObject)(int objdef, const char *optmodel, const char *optskin, objectPosition_t *pos, int id);
 	void		(*WO_ClearObjects)();
	bool		(*WO_GetObjectBounds)(int id, vec3_t bmin, vec3_t bmax);
	void		(*WO_DeleteObject)(int id);
	int         (*WO_GetObjectObjdef)(int id);
	residx_t	(*WO_GetObjectModel)(int id);
	residx_t	(*WO_GetObjectSkin)(int id);
	int			(*WO_GetObjdefId)(const char *name);
	bool		(*WO_IsObjectDoubleSided)(int id);
	int			(*WO_FindReferenceObject)(int startindex, const char *refname, referenceObject_t *refobj);

	//world objectgrid functions
	int     	(*WOG_FindClosestObjects)(int *objectTypes, int num_objectTypes, float min_dist, 
										  float max_dist, vec3_t pos, int team, float *closest_dist);
	//to get all objects of any type, just do num_objectTypes = 1, objectTypes[0] = -1
	int 		(*WOG_FindObjectsInRadius)(int *objectTypes, int num_objectTypes,
	                            		   float radius, vec3_t pos, 
										   int *objects, int numObjects);
	//navrep functions
	void		(*NavRep_CSGSubtract)(const baseObject_t* obj);
	void		(*NavRep_CSGAdd)(const baseObject_t* obj);
	bool		(*NavRep_Trace)(navmeshsize_t navmeshsize, const vec2_t src, const vec2_t dest);
	bool		(*NavRep_CanPlaceBuilding)(const vec3_t bminw, const vec3_t bmaxw);

	//navrep path functions
	navpath_t*	(*NavRep_PathCreateToPosition)(navmeshsize_t navmeshsize, const vec3_t src, struct navpoly_s* navPolySrc, const vec3_t dest);
	navpath_t*	(*NavRep_PathCreateToObject)(navmeshsize_t navmeshsize, const vec3_t src, struct navpoly_s* navPolySrc, const baseObject_t* obj, bool close);
	void		(*NavRep_PathDestroy)(navmeshsize_t navmeshsize, navpath_t* navpath);
	bool		(*NavRep_PathOptimize)(navmeshsize_t navmeshsize, navpath_t* navpath);

//	void		(*Path_FindPath)(navpath_t *navpath, vec3_t start, vec3_t end, vec3_t bmin, vec3_t bmax);
//	bool		(*DPath_FindPath)(navpath_t *navpath, int walker, int blocker_id, vec3_t endpos, int ignoreObjectIndex);
	//void		(*Path_FreePath)(navpath_t *navpath);
	
	//camera functions

	void		(*Cam_ConstructRay)(camera_t *camera, int winx, int winy, vec3_t out);

	//resource functions...we need these to get some surface info out of the models, this should be cleaned up
	bool		(*Res_GetModelSurfaceBounds)(residx_t model, vec3_t bmin, vec3_t bmax);
	residx_t	(*Res_LoadModel)(const char *name);

	//host functions

	int			(*System_Milliseconds)();
	int			(*Milliseconds)();	
	float		(*FrameSeconds)();
	int			(*FrameMilliseconds)();
	float		(*GetLogicFPS)();

	//packet functions
	void    (*Pkt_Clear)(packet_t *pkt);

	bool    (*Pkt_WriteInt)(packet_t *pkt, int i);
	bool    (*Pkt_WriteShort)(packet_t *pkt, short i);
	bool    (*Pkt_WriteByte)(packet_t *pkt, byte b);
	bool    (*Pkt_WriteString)(packet_t *pkt, char *s);
	bool    (*Pkt_WriteCmd)(packet_t *pkt, byte cmd);
	bool    (*Pkt_WriteCoord)(packet_t *pkt, float coord);
	bool    (*Pkt_WriteByteAngle)(packet_t *pkt, float angle);
	bool    (*Pkt_WriteWordAngle)(packet_t *pkt, float angle);
	bool    (*Pkt_WriteFloat)(packet_t *pkt, float f);


	int		(*Server_GetNumClients)();
	void	(*Server_BroadcastMessage)(int sender, char *msg);
	void    (*Server_SendMessage)(int sender, int client, char *msg);
	void	(*Server_SendUnreliableMessage)(int sender, int client, char *msg);
	void	(*Server_SpawnObject)(void *obj, int index);
	void	(*Server_GameObjectPointer)(void *base, int stride, int num_objects);
	void	(*Server_ClientExclusionListPointer)(void *base, int stride, int num_clients);
	void	(*Server_UpdatePlayerState)(int clientnum, playerState_t *ps);
	void	(*Server_FreeObject)(void *objdata);
	char 	*(*Server_GetClientCookie)(int client_id);
	unsigned int 	(*Server_GetClientUID)(int client_id);
	unsigned int 	(*Server_GetClientGUID)(int client_id);
	char    *(*Server_CheckForVIP)(char *cookie);
	short	(*Server_GetPort)();
	int     (*Server_GetStatus)();
	void	(*Server_SetStateString)(int id, const char *string);
	char	*(*Server_GetStateString)(int id, char *buf, int bufsize);
	void	(*Server_SetRequestOnly)(int id);
	void	(*Server_SendRequestString)(int clientnum, int id);
	int		(*Server_GetClientPing)(int clientnum);
	const char*	(*Server_GetClientIP)(int clientnum);
	int		(*Server_AllocVirtualClientSlot)();
#ifdef SAVAGE_DEMO
	bool	(*Server_IsDemoPlayer)(int clientnum);
#endif


	void*		(*Tag_Malloc)(int size);
	void		(*Tag_Free)(void *mem);
	void*		(*Tag_MallocGameScript)(int size);
	void		(*Tag_FreeGameScript)();

	bool	(*Server_StartStats)(char *server_stats);
	bool	(*Server_AddClientStats)(char *cookie, char *user_stats);
	bool	(*Server_SendStats)();
} coreAPI_server_t;

//interface API
typedef struct
{
	//system

	void		(*System_Dir)(char *directory, char *wildcard, bool recurse, void(*dirCallback)(const char *dir, void *userdata), void(*fileCallback)(const char *filename, void *userdata), void *userdata);

	//cmd
	void		(*Cmd_Register)(const char *name, cmdfunc_t cmdfunc);
	void		(*Cmd_BufPrintf)(const char *fmt, ...);
	//void		(*Cmd_ExecBuf)();
	void		(*Cmd_Exec)(const char *cmd);
	void		(*Cmd_FlushScriptBuffer)();
	void		(*Cmd_ReadConfigFileVars)(const char *filename, void(*callback)(char *varname, char *string, void *userdata), void *userdata);
	void		(*Cmd_Enable)(const char *name);
	void		(*Cmd_Disable)(const char *name);
	void    	(*Cmd_ProcessString)(char *str);



	//console variables

	void		(*Cvar_Register)(cvar_t *var);
	void		(*Cvar_Set)(const char *name, const char *string);
	void		(*Cvar_SetVar)(cvar_t *var, const char *string);
	void		(*Cvar_SetValue)(const char *name, float value);
	float		(*Cvar_GetValue)(const char *name);
	char		*(*Cvar_GetString)(const char *name);
	int			(*Cvar_GetInteger)(const char *name);
	void		(*Cvar_SetVarValue)(cvar_t *var, float value);
	int			(*Cvar_GetModifiedCount)(const char *name);



	//console printing

	//normal messages
	void		(*Console_Printf)(const char *fmt, ...);
	//debug messages
	void		(*Console_DPrintf)(const char *fmt, ...);
	//error messages
	void		(*Console_Errorf)(const char *fmt, ...);

	// input
	void    	(*Input_Event)(int key, int rawchar, bool down);
	void		(*Input_SetMouseMode)(int mode);
	int			(*Input_GetMouseMode)();
	bool		(*Input_IsKeyDown)(int key);
	void		(*Input_CenterMouse)();
	void		(*Input_SetMouseAngles)(const vec3_t angles);
	void		(*Input_SetMouseXY)(int x, int y);
	void		(*Input_GetMousePos)(mousepos_t *mp);
	int     	(*Input_GetButtons)();
	bool		(*Input_ActivateInputCallback)(inputcallback_t func, bool binds_active, void *widget);
	bool		(*Input_DeactivateInputCallback)(bool binds_active);
	void		(*Input_GetCallbackWidget)();

	//scene management

	void		(*Scene_AddObject)(sceneobj_t *scobj);
	void		(*Scene_AddOccluder)(occluder_t *occluder);
	void		(*Scene_AddLight)(scenelight_t *light);
	void		(*Scene_Clear)();
	void		(*Scene_Render)(camera_t *camera, vec3_t userpos);
	void		(*Scene_AddPoly)(int nverts,  scenefacevert_t *verts, residx_t shader, int flags);
	void		(*Scene_AddSkyObj)(sceneobj_t *sky);
	int			(*Scene_SelectObjects)(camera_t *camera, int rx, int ry, int rw, int rh, int *buf, int bufsize);

	//2d rendering

	void		(*Draw_SetShaderTime)(float time);
	void		(*Draw_SetColor)(vec4_t color);
	void		(*Draw_Quad2d)(float x, float y, float w, float h, float s1, float t1, float s2, float t2, residx_t shader);
	void		(*Draw_Poly2d)(vec2_t v1, vec2_t v2, vec2_t v3, vec2_t v4, float s1, float t1, float s2, float t2, residx_t shader);
	void		(*Draw_RotatedQuad2d)(float x, float y, float w, float h, float ang, float s1, float t1, float s2, float t2, residx_t shader);

	//color

	void		(*Color_Clamp)(vec4_t color);
	void		(*Color_ToFloat)(bvec4_t color, vec4_t out);
	void		(*Color_ToByte)(vec4_t color, bvec4_t out);

	//vid functions

	int			(*Vid_GetScreenW)();
	int			(*Vid_GetScreenH)();
	void		(*Vid_ProjectVertex)(camera_t *cam, vec3_t vertex, vec2_t result);
	void		(*Vid_ReadZBuffer)(int x, int y, int w, int h, float *zpixels);

	//camera functions

	void		(*Cam_ConstructRay)(camera_t *camera, int winx, int winy, vec3_t out);

	//host functions

	int			(*Milliseconds)();	
	float		(*FrameSeconds)();
	int			(*FrameMilliseconds)();
	float		(*GetLogicFPS)();

	residx_t	(*GetSavageLogoShader)();
	residx_t	(*GetWhiteShader)();
	residx_t	(*GetLoadingScreenShader)();
	residx_t	(*GetSysfontShader)();
	residx_t	(*GetNicefontShader)();

	//resources

	bool		(*Res_GetModelSurfaceBounds)(residx_t model, vec3_t bmin, vec3_t bmax);
	residx_t	(*Res_LoadModel)(const char *name);
	residx_t	(*Res_LoadSound)(const char *name);
	residx_t	(*Res_LoadShader)(const char *name);
	residx_t	(*Res_LoadShaderEx)(const char *name, int flags);
	int			(*Res_LoadSkin)(residx_t model, const char *name);
	residx_t	(*Res_LoadTerrainShader)(const char *name);
	void		(*Res_GetModelVisualBounds)(residx_t model, vec3_t bmin, vec3_t bmax);
	char		*(*Res_IsShaderFile)(char *name);
	int			(*Res_GetNumTextureFrames)(residx_t shader);
	int			(*Res_GetAnimatedTextureFPS)(residx_t shader);
	residx_t    (*Res_GetDynamapShader)(residx_t shader);

	//bitmap

	bool		(*Bitmap_Load)(const char *filename, bitmap_t *bmp);
	void		(*Bitmap_GetColor)(const bitmap_t *bmp, int x, int y, bvec4_t color);
	void		(*Bitmap_Free)(bitmap_t *bmp);
	void		(*Bitmap_Scale)(const bitmap_t *bmp, bitmap_t *out, int width, int height);
	void		(*Bitmap_GenerateThumbnails)(char *dirname, int size);
	void		(*Bitmap_SetPixel4b)(bitmap_t *bmp, int x, int y, byte r, byte g, byte b, byte a);

	//sound

	unsigned int	(*Sound_Play)(residx_t sampleidx, int sourcenum, const vec3_t pos, float volume, int channel, int priority);
	void			(*Sound_ClearLoopingSounds)();
	void			(*Sound_AddLoopingSound)(residx_t sampleidx, int sourcenum, int priority);
	void 			(*Sound_StopSource)(int sourcenum);
	void			(*Sound_StopHandle)(unsigned int handle);
	void			(*Sound_StopChannel)(int channel);
	void			(*Sound_MoveSource)(int sourcenum, vec3_t pos, vec3_t velocity);

	void 			(*Sound_SetListenerPosition)(const vec3_t pos, const vec3_t forward, const vec3_t up, bool warp);

	void			(*Sound_PlayMusic)(const char *streamFile);
	void			(*Sound_StopMusic)();

	//sound manager
	void	(*SC_PlayObjectSound)(int id, residx_t sound, float volume, int channel, bool loop);
	void	(*SC_StopObjectChannel)(int id, int channel);
	void	(*SC_ResetObjectPosition)(int id, vec3_t pos);
	void	(*SC_MoveObject)(int id, vec3_t pos);

	//GUI
	void	    *(*GUI_GetUserdata)(char *class_name, gui_element_t *obj);
	void	    *(*GUI_SetUserdata)(char *class_name, gui_element_t *obj, void *user_data);
	void	    *(*GUI_RegisterClass)(char *class_name, void *(*create)(gui_element_t *obj, int argc, char *argv[]));

	char		*(*GUI_SetName)(gui_element_t *obj, char *name);
	char		*(*GUI_SetClass)(gui_element_t *obj, char *class_name);

	void		(*GUI_Notify)(int argc, char *argv[]);

	bool		(*GUI_IsVisible)(gui_element_t *obj);
	bool		(*GUI_IsInteractive)(gui_element_t *obj);

	void		(*GUI_Exec)(char *command);
	void		(*GUI_Move)(gui_element_t *obj, int x, int y);
	void		(*GUI_Resize)(gui_element_t *obj, int w, int h);
	void		(*GUI_Show)(gui_element_t *obj);
	void		(*GUI_Hide)(gui_element_t *obj);
	void		(*GUI_Unfocus)(gui_element_t *obj);
	void		(*GUI_Focus)(gui_element_t *obj);
	bool		(*GUI_Select)(gui_element_t *obj);
	void		(*GUI_Param)(gui_element_t *obj, int argc, char *argv[]);
	void		(*GUI_UpdateDims)();
	void		(*GUI_GetPosition)(gui_element_t *obj, ivec2_t pos);
	void		(*GUI_GetSize)(gui_element_t *obj, ivec2_t size);

	void        (*GUI_FadeIn)(gui_element_t *obj, float time);
	void        (*GUI_FadeOut)(gui_element_t *obj, float time);
	gui_element_t *(*GUI_GetWidgetAtXY)(int x, int y);
	bool		(*GUI_CheckMouseAgainstUI)(int x, int y);
	bool		(*GUI_SendMouseDown)(mouse_button_enum button, int x, int y);
	bool		(*GUI_SendMouseUp)(mouse_button_enum button, int x, int y);
	bool    	(*GUI_WidgetWantsMouseClicks)(mouse_button_enum button, int x, int y, bool down);
	bool    	(*GUI_WidgetWantsKey)(int key, bool down);
	void		(*GUI_ResetFocus)();
	void    	(*GUI_ActivateNextInteractiveWidget)();

    void        (*GUI_RotatedQuad2d_S)(int x, int y, int w, int h, float ang, residx_t shader);
	void		(*GUI_Quad2d_S)(int x, int y, int w, int h, residx_t shader);
	void		(*GUI_ShadowQuad2d_S)(int x, int y, int w, int h, residx_t shader);
	void    	(*GUI_GetStringRowsCols)(char *str, int *rows, int *cols);
	void		(*GUI_DrawString)(int x, int y, const char *string, int charHeight, int iconHeight, int maxRows, int maxWidth, residx_t fontshader);
	void		(*GUI_DrawShadowedString)(int x, int y, const char *string, int charHeight, int iconHeight, int maxRows, int maxWidth, residx_t fontshader, float r, float g, float b, float a);
	void		(*GUI_DrawString_S)(int x, int y, const char *string, int charHeight, int iconHeight, int maxRows, int maxWidth, residx_t fontshader);
	void 		(*GUI_DrawStringBillboard)(camera_t *cam, vec3_t pos, const char *string, int charHeight, int maxRows, residx_t fontshader);
	void    	(*GUI_DrawStringMonospaced)(int x, int y, const char *string, int charWidth, int charHeight, int maxRows, int maxChars, residx_t fontshader);
	int			(*GUI_StringWidth)(const char *string, int charHeight, int iconHeight, int maxRows, int maxChars, residx_t fontshader);
	void		(*GUI_LineBox2d_S)(int x, int y, int w, int h, int thickness);	

	void		(*GUI_SetRGB)(float r, float g, float b);
	void		(*GUI_SetRGBA)(float r, float g, float b, float a);

	void		(*GUI_Panel_AddObject)(gui_element_t *object);
	void		(*GUI_Panel_Move)(gui_panel_t *panel, int x, int y);
	void		(*GUI_Panel_MoveRelative)(gui_panel_t *panel, int x, int y);
	void		(*GUI_Panel_Hide)(gui_panel_t *panel);
	void		(*GUI_Panel_Show)(gui_panel_t *panel);
	gui_panel_t *(*GUI_GetPanel)(char *name);

	void			(*GUI_AddElement)(gui_element_t *elem);
	void            (*GUI_GetScreenPosition)(gui_element_t *obj, ivec2_t pos);
	void            (*GUI_GetScreenSize)(gui_element_t *obj, ivec2_t size);
	gui_element_t   *(*GUI_GetObject)(char *hierarchy);
	void			*(*GUI_GetClass)(char *hierarchy, char *classname);
	void			(*GUI_List_Cmd)(int argc, char *argv[]);

	void		(*GUI_Draw)();
	void		(*GUI_Init)();

	void		*(*GUI_Malloc)(int size);
	void		(*GUI_Free)(void *ptr);
	void		*(*GUI_ReAlloc)(void *ptr, int size);
	char		*(*GUI_StrDup)(const char *string);

	void		(*GUI_Coord)(int *x, int *y);
	void		(*GUI_ConvertToScreenCoord)(int *x, int *y);
	void		(*GUI_ScaleToScreen)(int *w, int *h);
	void		(*GUI_Scale)(int *w, int *h);

	int			(*GUI_GetScreenWidth)();
	int			(*GUI_GetScreenHeight)();

	void		(*GUI_HideAllPanels)();
	void		(*GUI_DestroyAll)();

	void    	(*IRC_OutputCallback)(void (*outputcallback)(char *string));
	void    	(*IRC_OutputMsgCallback)(void (*outputcallback)(char *user, char *string, bool incoming));
	char 		**(*IRC_GetUserList)();
	char 		**(*IRC_GetBuddyList)();

	int     	(*MasterServer_GetGameList)();
	int			(*MasterServer_GetGameList_Frame)(server_info_t *servers, int max_servers, int *num_servers);
	bool    	(*MasterServer_GetGameInfo)(char *address, int port, int *ping, char **coreInfo, char **gameInfo);
	int     	(*MasterServer_FindUsers)(char *name);
	int			(*MasterServer_GetUserList_Frame)(user_info_t *users, int max_users, int *num_users);
	int     	(*MasterServer_GetUserInfo)(char *cookie, char *server, int port);
	int			(*MasterServer_GetUserInfo_Frame)(char *info, int maxlen);

	int			(*Client_GetConnectionState)();
	char		*(*World_GetName)();


} coreAPI_interface_t;
