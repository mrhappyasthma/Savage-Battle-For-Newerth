// (C) 2003 S2 Games

// camerautils.h

// Provides useful camera construction functions


void	Cam_SetAngles(camera_t *camera, const vec3_t angles);
void	Cam_SetTarget(camera_t *camera, const vec3_t target);
void	Cam_SetDistance(camera_t *camera, float dist);
	
void	Cam_CalcFovy(camera_t *camera);
void	Cam_ConstructRay(camera_t *camera, int winx, int winy, vec3_t out);
void	Cam_ClearOrigin(camera_t *camera);
void	Cam_ClearAxis(camera_t *camera);
void	Cam_DefaultCamera(camera_t *camera, int screenw, int screenh);

