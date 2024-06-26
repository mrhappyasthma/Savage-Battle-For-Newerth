// (C) 2003 S2 Games

// le_objects.c

// object handling in the editor, pretty crude but useable

// Objects that show up in the editor can be thought of just as representations of ingame objects
// and not the objects themselves.  The object representations are not made part of the collision
// system, and comparisons between objects are done by comparing object O with all other allocated
// objects in this module, as opposed to traversing a quadtree, etc.  Clearly this is not the way it 
// will be done in the game, but for editor purposes it's fine.

// fixme: maniacal clicking results in overlapping objects

#include "../le/le.h"
#include "../gui/gui_menu.h"

cvar_t	le_touchingObjects = { "le_touchingObjects", "1" };
cvar_t	le_selectionLock = { "le_selectionLock", "0" };
cvar_t	le_followTerrain = { "le_followTerrain", "1" };
cvar_t	le_oldObjectMove = { "le_oldObjectMove", "0", CVAR_SAVECONFIG  };

typedef struct
{
	bool	active;
	float	startFrame;
	float	z_offset;
} editorObject_t;

editorObject_t editorObjects[MAX_OBJECTS];

//#define PRECACHE_OBJECTS

int			manipulate_object;
int			current_selection = -1;
int			current_op = 0;			//current object operation (scale, rotate, etc)


bool	LE_UpdateObjectPosition(int object, objectPosition_t *objpos);

void		LE_EnumObjdefFile(const char *filename, void *userdata)
{
	corec.Cmd_Exec(fmt("exec #sys_enumdir#/%s\n", filename));
}

void		LE_RefreshObjMenu_Cmd(int argc, char *argv[])
{
	int n;
	gui_menu_t *objmenu;

	if (!argc)
		return;

	objmenu = corec.GUI_GetClass("objmenu_panel:obj_list_menu", MENU_CLASS);

	if (!objmenu)
	{
		corec.Console_Printf("No object list menu (objmenu_panel:obj_list_menu) was created, so object placement will be disabled\n");
		return;
	}

	GUI_Menu_Clear(objmenu);

	for (n=0; n<corec.WO_GetNumObjdefs(); n++)
	{
		char *category;
		char cmd[256];

		category = corec.WO_GetObjdefVar(n, "obj_editorCategory");

		if (strcmp(category, argv[0])!=0)
			continue;

		BPrintf(cmd, 255, "le_mode object");
		GUI_Menu_AddItem(objmenu, corec.WO_GetObjdefName(n), "");
	}
}

//fill in the object menu, which should have been created in the gui config
//this is a hack, but gets the job done
//a better solution would be to create a derived object menu widget
void		LE_BuildObjectMenu()
{
	int n;
	gui_menu_t *catmenu;

	catmenu = corec.GUI_GetClass("objmenu_panel:obj_category_menu", MENU_CLASS);
	if (!catmenu)
	{
		corec.Console_Printf("No object category menu (objmenu_panel:obj_category_menu) was created, so object placement will be disabled\n");
		return;
	}
	
	for (n=0; n<corec.WO_GetNumObjdefs(); n++)
	{
		char *category;
		char cmd[256];
	
		category = corec.WO_GetObjdefVar(n, "obj_editorCategory");
		
		if (GUI_Menu_ItemExists(catmenu, category))
			continue;

		BPrintf(cmd, 255, "le_mode object; refreshObjMenu %s", category);

		GUI_Menu_AddItem(catmenu, category, cmd);
	}
}

void		LE_LoadAllObjectDefs()
{
	//enumerate the current dir
	corec.System_Dir("props", "*.objdef", true, NULL, LE_EnumObjdefFile, NULL);
	//now enumerate the game dir
	corec.System_Dir(GAME_PATH "props", "*.objdef", true, NULL, LE_EnumObjdefFile, NULL);

	LE_BuildObjectMenu();
}

residx_t	LE_GetActiveModel()
{
	return corec.Res_LoadModel(corec.Cvar_GetString("obj_model"));
}

float	LE_TerrainHeight(float x, float y)
{
	pointinfo_t pi;

	corec.World_SampleGround(x,y,&pi);

	return pi.z;
}



void	LE_GetTerrainPosAtCursorXYZ(float *x, float *y, float *z)
{
	vec3_t dir;
	traceinfo_t trace;
	vec3_t end;
	bool ret;

	Cam_ConstructRay(&le.camera, le.mousepos.x, le.mousepos.y, dir);
	M_Normalize(dir);
	M_MultVec3(dir, 99999, dir);
	M_AddVec3(le.camera.origin, dir, end);
	ret = corec.World_TraceBox(&trace, le.camera.origin, end, zero_vec, zero_vec, ~SURF_TERRAIN);
	if (!ret)
	{
		//try the other direction
		Cam_ConstructRay(&le.camera, le.mousepos.x, le.mousepos.y, dir);
		M_Normalize(dir);
		M_MultVec3(dir, -99999, dir);
		M_AddVec3(le.camera.origin, dir, end);
		ret = corec.World_TraceBox(&trace, le.camera.origin, end, zero_vec, zero_vec, ~SURF_TERRAIN);
	}

	*x = trace.endpos[0];
	*y = trace.endpos[1];
	*z = trace.endpos[2];
}

int mouseoverObj=-1;

typedef enum
{
	OP_CREATE = 1,
	OP_ROTATE,
	OP_SCALE,
	OP_MOVE,
	OP_ALTITUDE
} objectOps_t;

extern cvar_t le_testanim;

//draws and updates objects
void		_LE_DrawObjects()
{
	int n;
	objectPosition_t pos;

	sceneobj_t sc;	

	/*
	light.intensity = 500;
	LE_CursorTrace(&trace, 0);
	SET_VEC3(light.color, 0, 1, 0);
	M_CopyVec3(trace.endpos, light.pos);
	corec.Scene_AddLight(&light);
	*/

	for (n=0; n<MAX_OBJECTS; n++)
	{
		float frame;
		float height;

		CLEAR_SCENEOBJ(sc);

		if (!corec.WO_GetObjectPos(n, &pos))
			continue;

		sc.scale = pos.scale;
		M_CopyVec3(pos.rotation, sc.angle);
		sc.model = corec.WO_GetObjectModel(n);
		sc.skin = corec.WO_GetObjectSkin(n);
		sc.objtype = OBJTYPE_MODEL;
		sc.flags = 0;//SCENEOBJ_SELECTABLE;

		if (n==mouseoverObj)
		{
			sc.flags |= SCENEOBJ_SHOW_WIRE | SCENEOBJ_SHOW_BOUNDS;
			//if (le_animate.integer)
			{
				static skeleton_t skel = { 0 };
				corec.Geom_BeginPose(&skel, sc.model);
				corec.Geom_SetBoneAnim("", le_testanim.string, corec.Milliseconds(), corec.Milliseconds(), 0, 0);
				corec.Geom_EndPose();
				sc.skeleton = &skel;
			}
		}
		sc.selection_id = n;

		frame = (le.frame - editorObjects[n].startFrame) / (30.0);
		sc.loframe = (int)frame;
		sc.hiframe = (int)frame+1;
		sc.lerp_amt = frame - (int)frame;

		//corec.Geom_BeginPose

		height = LE_TerrainHeight(pos.position[X],pos.position[Y]);
		//adjust for any terrain that might have been deformed under us
		if (pos.position[Z] != height + editorObjects[n].z_offset)
		{
			if (corec.Input_IsKeyDown(KEY_SHIFT))
			{
				//keep the object in the same place
				editorObjects[n].z_offset = pos.position[Z] - height;
			}
			else
			{
				//move object with terrain
				pos.position[Z] = height + editorObjects[n].z_offset;
				LE_UpdateObjectPosition(n, &pos);
				corec.WO_GetObjectPos(n, &pos);
			}
		}

		M_CopyVec3(pos.position, sc.pos);
		
		corec.Scene_AddObject(&sc);

		if (corec.WO_GetObjectSound(n))
			corec.Sound_AddLoopingSound(corec.WO_GetObjectSound(n), n, 0);

		corec.Sound_MoveSource(n, pos.position, NULL);

//		LE_AddShadow(&sc);
	}	

	if (le.uistate != UISTATE_GAMEWINDOW)
		return;

	if (current_op == OP_CREATE)
		return;

	if (!le_selectionLock.integer)
	{				
		mouseoverObj = -1;
		if (strcmp(le_mode.string, "object")==0 && current_selection == -1)
		{
			traceinfo_t trace;
					
			LE_CursorTrace(&trace, 0);
						
			mouseoverObj = trace.index;			
		}
	}	
}

void	LE_DrawObjects()
{
	PERF_BEGIN;
	_LE_DrawObjects();
	PERF_END(PERF_DRAWOBJECTS);
}


void	LE_ObjectMouseDown()
{

//	corec.Scene_SelectObjects(&le.camera, le.mousepos.x, le.mousepos.y, 1, 1, &selection, 1);

	if (mouseoverObj > -1)
	{
		objectPosition_t pos;

		current_selection = mouseoverObj;

		//snap the cursor to the object pos
		if (corec.WO_GetObjectPos(current_selection, &pos))
		{
			vec2_t proj;
			corec.Vid_ProjectVertex(&le.camera, pos.position, proj);
			corec.Input_SetMouseXY(proj[0],proj[1]);
		}

		return;
	}

	current_selection = -1;

	current_op = OP_CREATE;
}

void	LE_ObjectMouseUp()
{
	if (!le_selectionLock.integer)		
		current_selection = -1;
	
	current_op = 0;

	le.uistate = UISTATE_GAMEWINDOW;
}

void	LE_ObjectRightMouseDown()
{

//	corec.Scene_SelectObjects(&le.camera, le.mousepos.x, le.mousepos.y, 1, 1, &selection, 1);

	if (!le_selectionLock.integer)
	{

		if (mouseoverObj > -1)
		{
			current_selection = mouseoverObj;
			return;
		}

		current_selection = -1;
	}

	current_op = 0;
}

void	LE_ObjectRightMouseUp()
{
	if (!le_selectionLock.integer)		
		current_selection = -1;

	current_op = 0;
}

void	LE_DeleteObject_Cmd(int argc, char *argv[])
{
	if (strcmp(le_mode.string, "object")!=0)
		return;

	if (mouseoverObj > -1)
		corec.WO_DeleteObject(mouseoverObj);
}

void	LE_UpdateEditorObjectData(int obj)
{
	if (obj < 0 || obj >= MAX_OBJECTS)
		return;

	editorObjects[obj].startFrame = le.frame;
}

//returns true if we collided, otherwise false
bool	LE_CollideObject(const vec3_t pos, int objdef, float scale, int ignoreindex)
{
	vec3_t bmin = {0,0,0};
	vec3_t bmax = {0,0,0};
	traceinfo_t trace;
	residx_t model;

	model = corec.Res_LoadModel(corec.WO_GetObjdefVar(objdef, "obj_model"));

	if (objdef == -1)
		return true;

	//make sure we don't collide with other objects (ignore terrain)

	if (!corec.Res_GetModelSurfaceBounds(model, bmin, bmax))
	{
		//use visual object bounds
		corec.Res_GetModelVisualBounds(model, bmin, bmax);
		//shrink them a little for tighter object spacing
		M_MultVec3(bmin, 0.3, bmin);
		M_MultVec3(bmax, 0.3, bmax);
	}

	M_MultVec3(bmin,scale,bmin);
	M_MultVec3(bmax,scale,bmax);
	
	corec.World_TraceBoxEx(&trace, pos, pos, bmin, bmax, SURF_TERRAIN, ignoreindex);
	if (trace.fraction < 1)
		return true;

	return false;
}

bool	LE_UpdateObjectPosition(int object, objectPosition_t *objpos)
{
	int objdef;
	if (!le_touchingObjects.integer)
	{
		if (LE_CollideObject(objpos->position, corec.WO_GetObjectObjdef(object), objpos->scale, object))
			return false;
	}
	
	//if no collision we can update the object pos...object must be recreated to move
	objdef = corec.WO_GetObjectObjdef(object);
	corec.WO_DeleteObject(object);
	corec.WO_CreateObject(objdef, NULL, NULL, objpos, object);
	return true;
}

int		LE_GetCurrentObjdef()
{
	gui_menu_t *objmenu;
	int objdef;

	//get the currently selected object
	objmenu = corec.GUI_GetClass("objmenu_panel:obj_list_menu", MENU_CLASS);
	if (!objmenu)
		return -1;

	objdef = corec.WO_GetObjdefId(GUI_Menu_GetValue(objmenu->element));
	
	return objdef;
}

bool	LE_CreateObject(objectPosition_t *objpos)
{
	int n;

	if (LE_CollideObject(objpos->position, LE_GetCurrentObjdef(), objpos->scale, -1))
		return false;

	//get a free object slot
	
	for (n=0; n<MAX_OBJECTS; n++)
	{
		if (!editorObjects[n].active)
		{
			if (corec.WO_CreateObject(LE_GetCurrentObjdef(), NULL, NULL, objpos, n))
			{
				memset(&editorObjects[n], 0, sizeof(editorObject_t));
				editorObjects[n].active = true;
				LE_UpdateEditorObjectData(n);
				return true;
			}
			return false;
		}
	}
	
	corec.Console_Printf("Exceeded MAX_OBJECTS (%i)\n", MAX_OBJECTS);
	return false;
}

void	LE_ObjectMouseOver()
{
	LE_SetCursor(res.crosshairCursor);

	if (current_op == OP_CREATE)	//place an object
	{
		objectPosition_t pos;
		int objdef;

	//	mouseoverObj = -1;

		//set the object position
		LE_GetTerrainPosAtCursorXYZ(&pos.position[X], &pos.position[Y], &pos.position[Z]);
		objdef = LE_GetCurrentObjdef();
		if (objdef == -1)
			return;		
		pos.scale = M_Randnum(corec.WO_GetObjdefVarValue(LE_GetCurrentObjdef(), "obj_editorScaleRangeLo"), corec.Cvar_GetValue("obj_editorScaleRangeHi"));
		M_SetVec3(pos.rotation, 0, 0, rand() % 359);
		
		if (!LE_CreateObject(&pos))
		{
			LE_SetCursor(res.errorCursor);
		}

		current_op = 0;
	}
	else
	{
		if (le.button & BUTTON_LEFT)
		{
			if (le.button & BUTTON_SHIFT)
			{
				current_op = OP_ALTITUDE;

				if (current_selection > -1)
				{
					objectPosition_t pos;
					float old_z;

					old_z = editorObjects[current_selection].z_offset;
					editorObjects[current_selection].z_offset += -le.mousepos.deltay;

					corec.WO_GetObjectPos(current_selection, &pos);
					pos.position[Z] = editorObjects[current_selection].z_offset + LE_TerrainHeight(pos.position[X],pos.position[Y]);
					
					if (!LE_UpdateObjectPosition(current_selection, &pos))
					{
						LE_SetCursor(res.errorCursor);
						editorObjects[current_selection].z_offset = old_z;
					}
				}
			}
			else
			{
				current_op = OP_MOVE;

				if (current_selection > -1)
				{
					objectPosition_t pos;
					float ter_z;

					corec.WO_GetObjectPos(current_selection, &pos);
					if (le_oldObjectMove.integer || !editorObjects[current_selection].z_offset)
					{
						LE_GetTerrainPosAtCursorXYZ(&pos.position[X], &pos.position[Y], &ter_z);
					}
					else
					{
						//move parallel to the XY plane
						vec3_t forward,right;						
						M_CopyVec3(le.camera.viewaxis[FORWARD], forward);
						M_MultVec3(forward, -le.mousepos.deltay, forward);
						M_CopyVec3(le.camera.viewaxis[RIGHT], right);
						M_MultVec3(right, le.mousepos.deltax, right);
						
						pos.position[X] += forward[0] + right[0];
						pos.position[Y] += forward[1] + right[1];

						ter_z = le_followTerrain.integer ? pos.position[2] : LE_TerrainHeight(pos.position[0],pos.position[1]);
						corec.Console_Printf("%f\n",editorObjects[current_selection].z_offset);
					}
					pos.position[Z] = ter_z + editorObjects[current_selection].z_offset;
					
					if (!LE_UpdateObjectPosition(current_selection, &pos))
					{
						LE_SetCursor(res.errorCursor);
					}
				}
			}
		}
		else if (le.button & BUTTON_RIGHT)
		{
			if (le.button & BUTTON_SHIFT)
			{
				current_op = OP_SCALE;

				if (current_selection > -1)
				{
					objectPosition_t pos;
					corec.WO_GetObjectPos(current_selection, &pos);
					pos.scale += (float)le.mousepos.deltay / -100.0;
					if (pos.scale <= 0.1)
						pos.scale = 0.1;
					LE_UpdateObjectPosition(current_selection, &pos);
				}	
			}
			else
			{
				current_op = OP_ROTATE;

				if (current_selection > -1)
				{
					objectPosition_t pos;

					corec.WO_GetObjectPos(current_selection, &pos);

					if (le.button & BUTTON_CTRL)
						pos.rotation[X] += (float)le.mousepos.deltax;
					else if (le.button & BUTTON_ALT)
						pos.rotation[Y] += (float)le.mousepos.deltax;
					else
						pos.rotation[Z] += (float)le.mousepos.deltax;

					LE_UpdateObjectPosition(current_selection, &pos);
				}
			}
		}
	}
}

void	LE_SelectObject_Cmd(int argc, char *argv[])
{
	if (!argc)
	{
		corec.Console_Printf("syntax: selectobject [objdef_name]\n");
		return;
	}

	corec.WO_UseObjdef(argv[0]);
}

void	LE_SelectionLock_Cmd(int argc, char *argv[])
{

}

void	LE_Objects_Init()
{
	memset(editorObjects, 0, sizeof(editorObjects));
	manipulate_object = 0;

	corec.Cmd_Register("refreshObjMenu", LE_RefreshObjMenu_Cmd);
	corec.Cmd_Register("selectObject", LE_SelectObject_Cmd);
	corec.Cmd_Register("deleteobject", LE_DeleteObject_Cmd);

	corec.Cvar_Register(&le_touchingObjects);
	corec.Cvar_Register(&le_selectionLock);
	corec.Cvar_Register(&le_followTerrain);
	corec.Cvar_Register(&le_oldObjectMove);

	LE_LoadAllObjectDefs();
}

void	LE_ResetObjects()
{
	objectPosition_t pos;
	int n;

	for (n=0; n<MAX_OBJECTS; n++)
	{
		if (!corec.WO_GetObjectPos(n, &pos))
		{
			editorObjects[n].z_offset = 0;
			editorObjects[n].active = false;
		}
		else
		{
			editorObjects[n].z_offset = pos.position[Z] - LE_TerrainHeight(pos.position[X],pos.position[Y]);
			editorObjects[n].active = true;
		}
	}

}
