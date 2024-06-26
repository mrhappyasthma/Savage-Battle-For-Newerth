// (C) 2003 S2 Games

// le_camera.h

// camera functions


void	LE_DoCameraControl();
void	LE_StopCameraControl();
void	LE_StartCameraControl();
void	LE_SetupCamera();
void	LE_InitCamera();

extern cvar_t	le_targetrotate;
extern cvar_t	le_mouselook;