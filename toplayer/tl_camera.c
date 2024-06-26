/* 
 * (c) 2003 S2 Games
 *
 * tl_camera.c
 *
 * Camera control functions
 */

#include "tl_shared.h"

//cvar_t tl_camera_x = {"tl_camera_x", "0"};
//cvar_t tl_camera_y = {"tl_camera_y", "0"};

cvar_t tl_camera_origin_x = {"tl_camera_origin_x", "0"};
cvar_t tl_camera_origin_y = {"tl_camera_origin_y", "0"};
cvar_t tl_camera_origin_z = {"tl_camera_origin_z", "0"};

//cvar_t tl_camera_width = {"tl_camera_width", "1"};
//cvar_t tl_camera_height = {"tl_camera_height", "1"};

cvar_t	tl_camera_angle_x = { "tl_camera_angle_x", "0" };
cvar_t	tl_camera_angle_y = { "tl_camera_angle_y", "0" };
cvar_t	tl_camera_angle_z = { "tl_camera_angle_z", "0" };

cvar_t tl_camera_fovx = {"tl_camera_fovx", "40" };
//cvar_t tl_camera_zoom = {"tl_camera_zoom", "300"}; //only considered if camera mode is set to "pivot"

//cvar_t tl_camera_collide = {"tl_camera_collide", "1"};

//cvar_t tl_camera_mode = {"tl_camera_mode", "free"};	//can also be set to "pivot"



void	TL_SetupCamera(int screenw, int screenh, int flags, float time, camera_t *cam)
{
/*	pointinfo_t pi;
	vec3_t cam_pos, cam_angle;

	cam->time = time;
	cam->x = tl_camera_x.value * screenw;
	cam->y = tl_camera_y.value * screenh;
	cam->width = tl_camera_width.value * screenw;
	cam->height = tl_camera_height.value * screenh;
	cam->flags = flags;
	cam->fovx = tl_camera_fovx.value;

	Cam_CalcFovy(cam);

	cam_pos[0] = tl_camera_pos_x.value;
	cam_pos[1] = tl_camera_pos_y.value;
	cam_pos[2] = tl_camera_pos_z.value;

	cam_angle[0] = tl_camera_angle_x.value;
	cam_angle[1] = tl_camera_angle_y.value;
	cam_angle[2] = tl_camera_angle_z.value;

	if (strcmp(tl_camera_mode.string, "pivot")==0)
	{
		Cam_SetAngles(cam, cam_angle);
		Cam_SetTarget(cam, cam_pos);
		Cam_SetDistance(cam, tl_camera_zoom.value);
	}
	else
	{
		M_CopyVec3(cam->origin, cam_pos);
		Cam_SetAngles(cam, cam_angle);
	}

	if (tl_camera_collide.integer)
	{
		corec.World_SampleGround(cam->origin[0], cam->origin[1], &pi);

		if (cam->origin[2] < pi.z+30) 
			cam->origin[2] = pi.z+30;
	}
*/
}

void	TL_CopyPosition(camera_t *cam)
{
	vec3_t cam_angle;

	cam->origin[0] = tl_camera_origin_x.value;
	cam->origin[1] = tl_camera_origin_y.value;
	cam->origin[2] = tl_camera_origin_z.value;
/*
	cam->viewaxis[0][X] = tl_camera_viewaxis_xx.value;
	cam->viewaxis[0][Y] = tl_camera_viewaxis_xy.value;
	cam->viewaxis[0][Z] = tl_camera_viewaxis_xz.value;
	cam->viewaxis[1][X] = tl_camera_viewaxis_yx.value;
	cam->viewaxis[1][Y] = tl_camera_viewaxis_yy.value;
	cam->viewaxis[1][Z] = tl_camera_viewaxis_yz.value;
	cam->viewaxis[2][X] = tl_camera_viewaxis_zx.value;
	cam->viewaxis[2][Y] = tl_camera_viewaxis_zy.value;
	cam->viewaxis[2][Z] = tl_camera_viewaxis_zz.value;

	M_Normalize(cam->viewaxis[0]);
	M_Normalize(cam->viewaxis[1]);
	M_Normalize(cam->viewaxis[2]);*/

	cam_angle[0] = tl_camera_angle_x.value;
	cam_angle[1] = tl_camera_angle_y.value;
	cam_angle[2] = tl_camera_angle_z.value;

	Cam_SetAngles(cam, cam_angle);

	cam->fovx = tl_camera_fovx.value;
	Cam_CalcFovy(cam);
}

void	TL_SetCameraCvars(camera_t *cam, vec3_t angles)
{
	corec.Cvar_SetVarValue(&tl_camera_origin_x, cam->origin[0]);
	corec.Cvar_SetVarValue(&tl_camera_origin_y, cam->origin[1]);
	corec.Cvar_SetVarValue(&tl_camera_origin_z, cam->origin[2]);

	corec.Cvar_SetVarValue(&tl_camera_angle_x, angles[0]);
	corec.Cvar_SetVarValue(&tl_camera_angle_y, angles[1]);
	corec.Cvar_SetVarValue(&tl_camera_angle_z, angles[2]);

	corec.Cvar_SetVarValue(&tl_camera_fovx, cam->fovx);
}

void	TL_Camera_Init()
{
	corec.Cvar_Register(&tl_camera_origin_x);
	corec.Cvar_Register(&tl_camera_origin_y);
	corec.Cvar_Register(&tl_camera_origin_z);

	corec.Cvar_Register(&tl_camera_angle_x);
	corec.Cvar_Register(&tl_camera_angle_y);
	corec.Cvar_Register(&tl_camera_angle_z);

	corec.Cvar_Register(&tl_camera_fovx);
}
