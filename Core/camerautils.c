// (C) 2003 S2 Games

// camerautils.c

#include "savage_types.h"

void Cam_SetAngles(camera_t *camera, const vec3_t angles) 
{
	M_GetAxis(angles[0], angles[1], angles[2], camera->viewaxis);
}

void Cam_SetTarget(camera_t *camera, const vec3_t target)
{
	if (!camera) return;

	M_CopyVec3(target, camera->origin);
}

//specifies distance from target
void Cam_SetDistance(camera_t *camera, float dist)
{
	vec3_t offset;

	M_MultVec3(camera->viewaxis[FORWARD], dist, offset);

	camera->origin[0] -= offset[0];
	camera->origin[1] -= offset[1];
	camera->origin[2] -= offset[2];
	//M_SubVec3(camera->origin, offset, camera->origin);
}


//calculates y fov in degrees
void	Cam_CalcFovy(camera_t *camera)
{
	float a, x;

	if (!camera) return;
	/*
	camera->fovy = atan2(camera->height, camera->width / tan( camera->fovx / 360 * M_PI ));
	camera->fovy = camera->fovy * 360 / M_PI;
	*/
	x = camera->width / tan(camera->fovx/360*M_PI);
	a = atan(camera->height/x);
	a = a*360/M_PI;

	camera->fovy = a;

}


//calculates the direction of a ray originating at the camera origin and
//passing through space down the window coord winx,winy
void	Cam_ConstructRay(camera_t *camera, int winx, int winy, vec3_t out)
{
	float wx, wy;
	int n;

	wx = tan(DEG2RAD(camera->fovx) * 0.5);		//right side of window
	wx *= (((float)winx / (float)camera->width) - 0.5) * 2;
	
	wy = -tan(DEG2RAD(camera->fovy) * 0.5);		//bottom side of window
	wy *= (((float)winy / (float)camera->height) - 0.5) * 2;		

	for (n=0; n<3; n++)
		out[n] = camera->viewaxis[RIGHT][n] * wx + camera->viewaxis[UP][n] * wy + camera->viewaxis[FORWARD][n];

	//M_Normalize(out);
}

void	Cam_ClearOrigin(camera_t *camera)
{
	camera->origin[0] = 0;
	camera->origin[1] = 0;
	camera->origin[2] = 0;
}

void	Cam_ClearAxis(camera_t *camera)
{
	SET_VEC3(camera->viewaxis[0], 1, 0, 0);
	SET_VEC3(camera->viewaxis[1], 0, 1, 0);
	SET_VEC3(camera->viewaxis[2], 0, 0, 1);
}

void	Cam_DefaultCamera(camera_t *camera, int screenw, int screenh)
{
	vec3_t angles = {0,0,0};
	memset(camera, 0, sizeof(camera_t));

	camera->fovx = 90;
	camera->width = screenw;
	camera->height = screenh;
	Cam_CalcFovy(camera);
	Cam_ClearOrigin(camera);
	Cam_SetAngles(camera, angles);
}