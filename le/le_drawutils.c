// (C) 2003 S2 Games

// le_drawutils.c

// various drawing utilities

#include "../le/le.h"

vec4_t color;

// converts values given in 640x480 to screenwidth x screenheight
void	LE_640x480(int *x, int *y, int *w, int *h)
{	
	float fx,fy,fw,fh;

	fx = *x * /*(1024.0/640.0);*/le.screenscalex;
	fy = *y * /*(768.0/480.0);*/le.screenscaley;
	fw = *w * /*(1024.0/640.0);*/le.screenscalex;
	fh = *h * /*(768.0/480.0);*/le.screenscaley;

	//assumes coordinate pointers are valid
	*x = (int)fx;
	*y = (int)fy;
	*w = (int)fw;
	*h = (int)fh;
}

// accepts 640x480 coords
void	LE_Quad2d(int x, int y, int w, int h, residx_t shader)
{
	LE_640x480(&x, &y, &w, &h);
	corec.Draw_Quad2d(x, y, w, h, 0, 0, 1, 1, shader);
}

// accepts screen coordinates
void	LE_Quad2d_S(int x, int y, int w, int h, residx_t shader)
{
	corec.Draw_Quad2d(x, y, w, h, 0, 0, 1, 1, shader);
}

void	LE_ShadowQuad2d_S(int x, int y, int w, int h, residx_t shader)
{
	vec4_t shadow = { 0,0,0,0.5 };
	corec.Draw_SetColor(shadow);
	corec.Draw_Quad2d(x + 3, y + 3, w + 3, h + 3, 0, 0, 1, 1, res.white);
	LE_SetRGBA(color[0],color[1],color[2],1);
	corec.Draw_Quad2d(x, y, w, h, 0, 0, 1, 1, shader);
	corec.Draw_SetColor(color);
}

void	LE_LineBox2d_S(int x, int y, int w, int h, int thickness)
{
	LE_Quad2d_S(x, y, w, thickness, res.white);
	LE_Quad2d_S(x, y, thickness, h, res.white);
	LE_Quad2d_S(x, (y+h)-thickness, w, thickness, res.white);
	LE_Quad2d_S((x+w)-thickness, y, thickness, h, res.white);
}


bool	LE_MouseInRect(int x1, int y1, int x2, int y2)
{
	if (le.mousepos.x >= x1 && le.mousepos.x <= x2 &&
		le.mousepos.y >= y1 && le.mousepos.y <= y2)
		return true;
	return false;
}

void	LE_SetRGB(float r, float g, float b)
{
	color[0] = r;
	color[1] = g;
	color[2] = b;
	color[3] = 1;

	corec.Draw_SetColor(color);
}

void	LE_SetRGBA(float r, float g, float b, float a)
{
	color[0] = r;
	color[1] = g;
	color[2] = b;
	color[3] = a;

	corec.Draw_SetColor(color);
}
