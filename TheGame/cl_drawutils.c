// (C) 2002 S2 Games

// cl_drawutils.c

#include "client_game.h"

// converts values given in 640x480 to screenwidth x screenheight
void	CL_640x480(int *x, int *y, int *w, int *h)
{	
	float fx,fy,fw,fh;

	fx = *x * cl.screenscalex;
	fy = *y * cl.screenscaley;
	fw = *w * cl.screenscalex;
	fh = *h * cl.screenscaley;

	//assumes coordinate pointers are valid
	*x = (int)fx;
	*y = (int)fy;
	*w = (int)fw;
	*h = (int)fh;
}

// accepts 640x480 coords
void	CL_Quad2d(int x, int y, int w, int h, residx_t shader)
{
	CL_640x480(&x, &y, &w, &h);
	corec.Draw_Quad2d(x, y, w, h, 0, 0, 1, 1, shader);
}

// accepts screen coordinates
void	CL_Quad2d_S(int x, int y, int w, int h, residx_t shader)
{
	corec.Draw_Quad2d(x, y, w, h, 0, 0, 1, 1, shader);
}
