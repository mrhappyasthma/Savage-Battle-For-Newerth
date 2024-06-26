// (C) 2003 S2 Games

// tl_drawutils.c

// top layer draw utilities

#include "tl_shared.h"

#include <stdarg.h>

static int tl_coordSystem = SCREEN_COORDS;
static float tl_customCoordsWidth = 640;
static float tl_customCoordsHeight = 480;


void TL_SetCoordSystem(int coordSystem)
{
	tl_coordSystem = coordSystem;
}

void TL_SetCustomCoords(float width, float height)
{
	tl_customCoordsWidth = width;
	tl_customCoordsHeight = height;
	
	tl_coordSystem = CUSTOM_COORDS;
}

void	TL_ConvertCoord4(int *x, int *y, int *w, int *v)
{
	switch(tl_coordSystem)
	{
		case GUI_COORDS:
			corec.GUI_ConvertToScreenCoord(x, y);
			corec.GUI_ScaleToScreen(w, v);
			return;
		case SCREEN_COORDS:
			return;
		case CUSTOM_COORDS:
		{
			float fx,fy,fw,fv;

			fx = *x * (tl_customCoordsWidth / corec.Vid_GetScreenW());
			fy = *y * (tl_customCoordsHeight / corec.Vid_GetScreenH());
			fw = *w * (tl_customCoordsWidth / corec.Vid_GetScreenW());
			fv = *v * (tl_customCoordsHeight / corec.Vid_GetScreenH());

			*x = (int)fx;
			*y = (int)fy;
			*w = (int)fw;
			*v = (int)fv;
			return;
		}
	}
}

void TL_DrawString(int x, int y, const char *string, int charHeight, int rows, int maxWidth, residx_t fontshader)
{
	int oldMode, dummy = 0;
	const char *s;
	s = (char *)string;
	
	oldMode = tl_coordSystem;
	TL_ConvertCoord4(&x, &y, &dummy, &charHeight);

	tl_coordSystem = SCREEN_COORDS;

	corec.GUI_DrawString_S(x, y, string, charHeight, charHeight, rows, maxWidth, corec.GetNicefontShader());

	tl_coordSystem = oldMode;
}

//quick and dirty string printing
void	TL_Printf(int x, int y, const char *format, ...)
{
	char buf[256];
	va_list argptr;

	va_start(argptr, format);
#ifdef _WIN32
	_vsnprintf(buf, 255, format, argptr);
#else //linux?
	_vsnprintf(buf, 255, format, argptr);
#endif
	va_end(argptr);

	TL_DrawString(x, y, buf, 12, 1, 255, corec.GetSysfontShader());
}

void	TL_Quad2d(int x, int y, int w, int h, residx_t shader)
{
	TL_ConvertCoord4(&x,&y,&w,&h);
	corec.Draw_Quad2d(x,y,w,h,0,0,1,1,shader);
}

void	TL_LineBox2d(int x, int y, int w, int h, int thickness)
{			
	if (w < 0)
	{
		w = -w;
		x -= w;
	}
	if (h < 0)
	{
		h = -h;
		y -= h;
	}
	TL_Quad2d(x, y, w, thickness, corec.GetWhiteShader());
 	TL_Quad2d(x, y, thickness, h, corec.GetWhiteShader());
 	TL_Quad2d(x, (y+h)-thickness, w, thickness, corec.GetWhiteShader());
 	TL_Quad2d((x+w)-thickness, y, thickness, h, corec.GetWhiteShader());
}
