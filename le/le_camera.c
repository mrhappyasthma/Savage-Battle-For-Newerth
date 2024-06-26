// (C) 2003 S2 Games

// le_camera.c

// Camera setup routines

#include "../le/le.h"

int oldmousex, oldmousey;

#define MIN_ZOOM		150

extern cvar_t le_scrollup;
extern cvar_t le_scrolldown;
extern cvar_t le_scrollleft;
extern cvar_t le_scrollright;
extern cvar_t le_cam_tilt;

cvar_t	le_zoomsmooth = { "le_zoomsmooth", "0.3", CVAR_SAVECONFIG };
cvar_t	le_zoom = { "le_zoom", "2000", CVAR_SAVECONFIG };
cvar_t	le_swivel = { "le_swivel", "5", CVAR_SAVECONFIG };
cvar_t	le_accel = { "le_accel", "0.2", CVAR_SAVECONFIG };
cvar_t	le_speed = { "le_speed", "10", CVAR_SAVECONFIG };
cvar_t	le_friction = { "le_friction", "0.2", CVAR_SAVECONFIG };
cvar_t	le_fov = { "le_fov", "90", CVAR_SAVECONFIG };
cvar_t	le_targetrotate = { "le_targetrotate", "0" };
cvar_t	le_mouselook = { "le_mouselook", "0" };
cvar_t	le_showpos = { "le_showpos", "0" };
cvar_t	le_roll = { "le_roll", "0" };

void	LE_StartCameraControl()
{
	le.showmouse = false;
	corec.Input_SetMouseMode(MOUSE_RECENTER);
	oldmousex = le.mousepos.x;
	oldmousey = le.mousepos.y;
}

void	LE_StopCameraControl()
{
//	if (le.dragging)
	//{
		le.showmouse = true;
		le.dragging = 0;
		corec.Input_SetMouseMode(0);
		corec.Input_SetMouseXY(oldmousex, oldmousey);
		le.mousepos.x = oldmousex;
		le.mousepos.y = oldmousey;
//	}
}

void	LE_MoveCamera()
{
	M_AddVec3(le.camera.origin, le.velocity, le.camera.origin);
}

void	LE_Zoom_Cmd(int argc, char *argv[])
{
	vec3_t intent;

	if (argc < 1)
		return;

	M_MultVec3(le.camera.viewaxis[FORWARD], atof(argv[0]), intent);
	M_AddVec3(intent, le.velocity, le.velocity);
}

void	LE_DoCameraControl()
{
	vec3_t intent = { 0,0,0 };

	if (le_targetrotate.integer && le.button & BUTTON_LEFT)
	{
		M_MultVec3(le.camera.viewaxis[FORWARD], -le.mousepos.deltay, intent);
	}
	else
	{
		if (corec.Cvar_GetValue("move_forward"))
		{
			if (le_targetrotate.integer || le_mouselook.integer)
				M_PointOnLine(intent, le.camera.viewaxis[FORWARD], 1, intent);
			else
			{
				//move parallel to the XY plane
				vec3_t forward;
				M_CopyVec3(le.camera.viewaxis[FORWARD], forward);
				forward[2]=0;
				M_NormalizeVec2(forward);
				M_PointOnLine(intent, forward, 1, intent);
			}	
		}
		if (corec.Cvar_GetValue("move_backward"))
		{
			if (le_targetrotate.integer || le_mouselook.integer)
				M_PointOnLine(intent, le.camera.viewaxis[FORWARD], -1, intent);
			else
			{
				//move parallel to the XY plane
				vec3_t forward;
				M_CopyVec3(le.camera.viewaxis[FORWARD], forward);
				forward[2]=0;
				M_NormalizeVec2(forward);
				M_PointOnLine(intent, forward, -1, intent);	
			}
		}
		if (corec.Cvar_GetValue("move_left"))
		{
			M_PointOnLine(intent, le.camera.viewaxis[RIGHT], -1, intent);
		}
		if (corec.Cvar_GetValue("move_right"))
		{
			M_PointOnLine(intent, le.camera.viewaxis[RIGHT], 1, intent);
		}
		if (corec.Cvar_GetValue("move_up"))
		{
			vec3_t up = { 0,0,1 };
			M_AddVec3(intent, up, intent);
		}
		if (corec.Cvar_GetValue("move_down"))
		{
			vec3_t down = { 0,0,-1 };
			M_AddVec3(intent, down, intent);
		}

		M_SetVec3Length(intent, le_speed.value);
	}
	
	M_LerpVec3(le_accel.value, le.velocity, intent, le.velocity);

	if (le_targetrotate.integer)
	{
		if (!(le.button & BUTTON_LEFT))
		{
			le.cam_angle[2] -= (float)le.mousepos.deltax / 5;
			if (le.cam_angle[2]>360) le.cam_angle[2] = 0;
			if (le.cam_angle[2]<0) le.cam_angle[2] = 360;					
			le.cam_angle[0] += (float)le.mousepos.deltay / 15.0;
			if (le.cam_angle[0]>0) le.cam_angle[0] = 0;
			if (le.cam_angle[0]<-90) le.cam_angle[0] = -90;
		}
	}
	else if (le_mouselook.integer)
	{
		le.cam_angle[2] -= (float)le.mousepos.deltax / 5;
		if (le.cam_angle[2]>360) le.cam_angle[2] = 0;
		if (le.cam_angle[2]<0) le.cam_angle[2] = 360;					
		le.cam_angle[0] -= (float)le.mousepos.deltay / 15.0;
		if (le.cam_angle[0]>90) le.cam_angle[0] = 90;
		if (le.cam_angle[0]<-90) le.cam_angle[0] = -90;
	}
}

void	LE_SetupCamera()
{
	int flags;
	camera_t *cam = &le.camera;
	
	flags = 0;
	if (le_showboxtree.value)
		flags |= CAM_SHOW_BBOXES;
	if (le_noterrain.value)
		flags |= CAM_NO_TERRAIN;
	if (le_wireframe.value)
		flags |= CAM_WIREFRAME_TERRAIN;
	if (le_notexture.value)
		flags |= CAM_NO_TEXTURE;
	if (le_showlayer.value == 1)
		flags |= CAM_NO_SHADERMAP2;
	if (le_showlayer.value == 2)
		flags |= CAM_NO_SHADERMAP;

	cam->time = (float)corec.Milliseconds() / 1000.0;
	cam->x = 0;
	cam->y = 0;
	cam->width = le.screenw;
	cam->height = le.screenh;
	cam->flags = flags;


	le.camera.fovx = le_fov.value;
	Cam_CalcFovy(&le.camera);

	le.camera.width = corec.Vid_GetScreenW();
	le.camera.height = corec.Vid_GetScreenH();
	le.camera.x = 0;
	le.camera.y = 0;
	Cam_SetAngles(&le.camera, le.cam_angle);
	LE_DoCameraControl();

	if (le_targetrotate.integer)
	{
		vec3_t diff;
		float dist;

		M_SubVec3(le.user_target, le.camera.origin, diff);
		dist = M_GetVec3Length(diff);
		M_PointOnLine(le.user_target, le.camera.viewaxis[FORWARD], -dist, le.camera.origin);
	}

	LE_MoveCamera();

	if (le_showpos.integer)
	{
		corec.Console_Printf("x: %.1f  y: %.1f  z: %.1f  pitch: %.0f  yaw: %.0f  roll: %.0f\n",
			le.camera.origin[0], le.camera.origin[1], le.camera.origin[2],
			le.cam_angle[PITCH], le.cam_angle[YAW], le.cam_angle[ROLL]);
		corec.Console_Printf("(%.3f %.3f %.3f) (%.3f %.3f %.3f) (%.3f %.3f %.3f)\n",
			le.camera.viewaxis[0][0], le.camera.viewaxis[0][1], le.camera.viewaxis[0][2],
			le.camera.viewaxis[1][0], le.camera.viewaxis[1][1], le.camera.viewaxis[1][2],
			le.camera.viewaxis[2][0], le.camera.viewaxis[2][1], le.camera.viewaxis[2][2]);
	}
	le.cam_angle[ROLL] = le_roll.value;

	corec.Sound_SetListenerPosition(le.camera.origin, le.camera.viewaxis[FORWARD], le.camera.viewaxis[UP], false);
}

void	LE_InitCamera()
{
	corec.Cvar_Register(&le_accel);
	corec.Cvar_Register(&le_speed);
	corec.Cvar_Register(&le_friction);
	corec.Cvar_Register(&le_fov);
	corec.Cvar_Register(&le_targetrotate);
	corec.Cvar_Register(&le_mouselook);
	corec.Cvar_Register(&le_showpos);
	corec.Cvar_Register(&le_roll);

	corec.Cmd_Register("zoom", LE_Zoom_Cmd);
}