// (C) 2003 S2 Games

// le_drawutils.h



// converts values given in 640x480 to screenwidth x screenheight
void	LE_640x480(int *x, int *y, int *w, int *h);
// accepts 640x480 coords
void	LE_Quad2d(int x, int y, int w, int h, residx_t shader);
// accepts screen coords
void	LE_Quad2d_S(int x, int y, int w, int h, residx_t shader);
void	LE_ShadowQuad2d_S(int x, int y, int w, int h, residx_t shader);
void	LE_LineBox2d_S(int x, int y, int w, int h, int thickness);

bool	LE_MouseInRect(int x1, int y1, int x2, int y2);

void	LE_OffsetCoords(int x, int y);

void	LE_SetRGB(float r, float g, float b);

void	LE_SetRGBA(float r, float g, float b, float a);
