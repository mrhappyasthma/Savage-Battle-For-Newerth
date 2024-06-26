// (C) 2001 S2 Games

// gui_track.c

// track control for editor time of day settings


#include "../toplayer/tl_shared.h"
#include "gui_track.h"

static char *class_name = TRACK_CLASS;

void	GUI_Track_GetKeyDims(gui_track_t *track, int keynum, float time, int *x, int *y, int *w, int *h)
{
	float range = 1440;

	*w = track->icon_size[X];
	*h = track->icon_size[Y];

	if (track->element->width > track->element->height)
	{
		*x = (time / range) * track->element->width;
		*y = 0;
	}
	else
	{
		*x = 0;
		*y = (time / range) * track->element->height;
	}
}

void	GUI_Track_Draw(gui_element_t *obj, int w, int h)
{
	gui_track_t *track;
	int numkeys, n;
	float time;
	float tod_time;

	track = corec.GUI_GetUserdata(TRACK_CLASS, obj);
	if (!track)
		return;

	numkeys = corec.TOD_GetNumKeyframes();

	/*corec.GUI_ShadowQuad2d_S(	0, 0, 
		obj->width, 
		obj->height, 
		corec.GetWhiteShader());
	*/

	tod_time = corec.Cvar_GetValue("le_tod");

	for (n=0; n<numkeys; n++)
	{
		int kx,ky,kw,kh;

		time = corec.TOD_GetKeyframeTime(n);
		GUI_Track_GetKeyDims(track, n, time, &kx, &ky, &kw, &kh);

		if (!track->dragging &&
			abs(tod_time - time) < track->laxness)
		{
			corec.Cvar_SetValue("le_tod", time);
			track->selection = n;
		} 
		else if (!track->dragging)
		{
			track->selection = -1;
		}

		corec.GUI_Quad2d_S(kx, ky, kw, kh, 
			track->selection == n ? track->keySelectedShader : track->keyShader);
	}
}

void	GUI_Track_Move(gui_element_t *obj, int x, int y)
{
	corec.GUI_Move(obj, x, y);
}

void	GUI_Track_MouseDown(gui_element_t *obj, mouse_button_enum button, int x, int y)
{
	int n;
	float time;
	gui_track_t *track = corec.GUI_GetUserdata(TRACK_CLASS, obj);

	if (!track)
		return;

	track->dragging = true;
	track->mousedown_pos[X] = x;
	track->mousedown_pos[Y] = y;

	for (n=corec.TOD_GetNumKeyframes()-1; n>=0; n--)	//do it in backwards order so hit testing works visually
	{
		int kx,ky,kw,kh;

		time = corec.TOD_GetKeyframeTime(n);
		GUI_Track_GetKeyDims(track, n, time, &kx, &ky, &kw, &kh);
		
		if (x >= kx && x <= kx+kw && y >= ky && y <= ky+kh)
		{
			//fixme: for multiple selection, check if shift is pressed and add this selection to an array			
			corec.Cvar_SetValue("le_tod", time);			
			track->selection = n;
			return;
		}
	}

	track->selection = -1;
}

void	GUI_Track_MouseOver(gui_element_t *obj, int x, int y)
{
	float time;

	gui_track_t *track = corec.GUI_GetUserdata(TRACK_CLASS, obj);
	if (!track)
		return;

	if (!track->dragging)
		return;

	if (track->selection > -1)
	{
		time = corec.TOD_GetKeyframeTime(track->selection);
		if (track->element->width > track->element->height)
		{
			corec.TOD_MoveKeyframe(track->selection,	time + (((float)(x - track->mousedown_pos[X]) / (float)obj->width)) * 1440);
		}
		else
		{
			corec.TOD_MoveKeyframe(track->selection, time + (((float)(y - track->mousedown_pos[Y]) / (float)obj->height)) * 1440);
		}
	}
	track->mousedown_pos[X] = x;
	track->mousedown_pos[Y] = y;
}

void	GUI_Track_MouseUp(gui_element_t *obj, mouse_button_enum button, int x, int y)
{
	float time;
	gui_track_t *track = corec.GUI_GetUserdata(TRACK_CLASS, obj);
	if (!track)
		return;

	track->dragging = false;

	time = corec.TOD_GetKeyframeTime(track->selection);

	corec.Cvar_SetValue("le_tod", time);
}

void GUI_Track_Param_Cmd(gui_element_t *obj, int argc, char *argv[])
{
	char filename[256];
	gui_track_t *track;

	if (argc < 1)
	{
		corec.Console_Printf("track param <panel:object> <var> <value>\n");
		corec.Console_Printf("valid parameters:\n");
		corec.Console_Printf("   laxness <value>			- the amount of \"give\" before deciding the current\ncontroller value is on a keyframe (and setting it to be exactly on it)\n");
		corec.Console_Printf("   icon_width <width>      - sets the width of the icon\n");
		corec.Console_Printf("   icon_height <height>    - sets the height of the icon\n");
		corec.Console_Printf("   icon_selected <image>   - sets the image for a selected keyframe\n");
		corec.Console_Printf("   icon_unselected <image> - sets the image for an unselected keyframe\n");
		return;
	}

	track = corec.GUI_GetUserdata(TRACK_CLASS, obj);

	if (!track)
		return;

	if (strcmp(argv[0], "laxness") == 0)
	{
		if (argc > 1)
		{
			track->laxness = atof(argv[1]);
		} else {
			corec.Console_Printf("Not enough parameters.  You must specify the laxness.\n");
		}
	}
	else if (strcmp(argv[0], "icon_width") == 0)
	{
		if (argc > 1)
		{
			track->icon_size[X] = atoi(argv[1]);
		} else {
			corec.Console_Printf("Not enough parameters.  You must specify the icon width.\n");
		}
	}
	else if (strcmp(argv[0], "icon_height") == 0)
	{
		if (argc > 1)
		{
			track->icon_size[Y] = atoi(argv[1]);
		} else {
			corec.Console_Printf("Not enough parameters.  You must specify the icon height.\n");
		}
	}
	else if (strcmp(argv[0], "icon_unselected") == 0)
	{
		if (argc > 1)
		{
			BPrintf(filename, 255, "%s%s", UI_PREFIX, argv[1]);
			filename[255] = 0;
			track->keyShader = corec.Res_LoadShaderEx(filename, SHD_NO_MIPMAPS | SHD_FULL_QUALITY);
		} else {
			corec.Console_Printf("Not enough parameters.  You must specify the image filename\n");
		}
	}
	else if (strcmp(argv[0], "icon_selected") == 0)
	{
		if (argc > 1)
		{
			BPrintf(filename, 255, "%s%s", UI_PREFIX, argv[1]);
			filename[255] = 0;
			track->keySelectedShader = corec.Res_LoadShaderEx(filename, SHD_NO_MIPMAPS | SHD_FULL_QUALITY);
		} else {
			corec.Console_Printf("Not enough parameters.  You must specify the image filename\n");
		}
	}
}

void	GUI_Track_Destroy(gui_element_t *obj)
{
	gui_track_t *track;
	track = corec.GUI_GetUserdata(class_name, obj);

	if (!track)
		return;
}

void	*GUI_Track_Create(gui_element_t *obj, int argc, char *argv[])
{
	gui_track_t *track;
	
	if (argc < 5)
	{
		corec.Console_Printf("syntax: create %s name x y w h\n", TRACK_CLASS);
		return NULL;
	}

	corec.GUI_SetClass(obj, TRACK_CLASS);
	corec.GUI_SetName(obj, argv[0]);
	corec.GUI_Move (obj, atoi(argv[1]), atoi(argv[2]) );
	corec.GUI_Resize(obj, atoi(argv[3]), atoi(argv[4]));

	track = corec.GUI_Malloc(sizeof (gui_track_t));

	if (!track)
	{
		corec.Console_Printf("TrackControl error: couldn't alloc enough space to hold trackcontrol\n");
		return NULL; 		
	}

	corec.GUI_SetUserdata(TRACK_CLASS, obj, track);
	track->element = obj;

	obj->destroy = GUI_Track_Destroy;
	obj->draw = GUI_Track_Draw;
	obj->idle = NULL;
	obj->move = GUI_Track_Move;
	obj->mouseup = GUI_Track_MouseUp;
	obj->mousedown = GUI_Track_MouseDown;
	//obj->mouseout = GUI_Track_MouseOut;
	obj->mouseover = GUI_Track_MouseOver;
	
	obj->param = GUI_Track_Param_Cmd;

	obj->getvalue = NULL;
	//obj->getvalue = GUI_Track_GetValue;

	obj->noclip = false;

	track->laxness = 10;

	track->icon_size[X] = obj->height;
	track->icon_size[Y] = obj->height;

	track->selection = 0;

	track->keyShader = 0;
	track->keySelectedShader = 0; 
	track->dragging = false;

	track->parent = NULL;

	return track;
}

void	GUI_Track_Init()
{
	corec.GUI_RegisterClass(TRACK_CLASS, GUI_Track_Create);

	//corec.Cmd_Register("trackcontrol", GUI_Track_Cmd);
}
