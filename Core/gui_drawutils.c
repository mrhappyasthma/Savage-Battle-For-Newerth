// (C) 2003 S2 Games

// gui_drawutils.c

// various drawing utilities
// fixme: these functions shouldn't be in the core engine

#include "core.h"

int offsetx, offsety;
int gui_screenw, gui_screenh;

vec4_t color;

void	GUI_RotatedQuad2d_S(int x, int y, int w, int h, float ang, residx_t shader)
{
	GUI_ConvertToScreenCoord(&x, &y);
	GUI_ScaleToScreen(&w, &h);
	DU_DrawRotatedQuad(x, y, w, h, ang, 0, 0, 1, 1, shader);
}
void	GUI_Quad2d_S(int x, int y, int w, int h, residx_t shader)
{
	GUI_ConvertToScreenCoord(&x, &y);
	GUI_ScaleToScreen(&w, &h);
	Draw_Quad2d(x, y, MAX(1,w), MAX(1,h), 0, 0, 1, 1, shader);
}

void	GUI_ShadowQuad2d_S(int x, int y, int w, int h, residx_t shader)
{
	vec4_t shadow = { 0,0,0,0.5 };
	Draw_SetColor(shadow);
	GUI_Quad2d_S(x + 3, y + 3, w, h, shader);
	GUI_SetRGBA(color[0],color[1],color[2],1);
	GUI_Quad2d_S(x, y, w, h, shader);
	Draw_SetColor(color);
}

void	GUI_LineBox2d_S(int x, int y, int w, int h, int thickness)
{
 	GUI_Quad2d_S(x, y, w, thickness, Host_GetWhiteShader());
 	GUI_Quad2d_S(x, y, thickness, h, Host_GetWhiteShader());
 	GUI_Quad2d_S(x, (y+h)-thickness, w, thickness, Host_GetWhiteShader());
 	GUI_Quad2d_S((x+w)-thickness, y, thickness, h, Host_GetWhiteShader());
}

void	GUI_Line2d_S(int x1, int y1, int x2, int y2, int thickness)
{
 	//GUI_Quad2d_S(x, y, w, thickness, Host_GetWhiteShader());
 	//GUI_Quad2d_S(x, y, thickness, h, Host_GetWhiteShader());
 	//GUI_Quad2d_S(x, (y+h)-thickness, w, thickness, Host_GetWhiteShader());
 	//GUI_Quad2d_S((x+w)-thickness, y, thickness, h, Host_GetWhiteShader());
}

void	GUI_GetStringRowsCols(char *str, int *rows, int *cols)
{
	char *lb, *lastbreak;
	int linebreaks = 1;
	int longestline = 0;
	
	lb = lastbreak = str;
	while ((lb = strstr(lb, "\n")))
	{
		if (longestline < lb - str - 1)
			longestline = lb - str - 1;
		lb++;
		linebreaks++;
		lastbreak = lb;
	}
	if (longestline < (int)strlen(lastbreak))
		longestline = strlen(lastbreak);
	*rows = linebreaks;
	*cols = longestline;
}

void	GUI_DrawString(int x, int y, const char *string, int charHeight, int iconHeight, int maxRows, int maxWidth, residx_t fontshader)
{
	int blah = 0;
	GUI_ConvertToScreenCoord(&x, &y);
	GUI_ScaleToScreen(&maxWidth, &charHeight);
	GUI_ScaleToScreen(&blah, &iconHeight);
	DU_DrawString(x, y, string, charHeight, iconHeight, maxRows, maxWidth, fontshader, true);
}

void	GUI_DrawShadowedString(int x, int y, const char *string, int charHeight, int iconHeight, int maxRows, int maxWidth, residx_t fontshader, float r, float g, float b, float a)
{
	int blah = 0;
	GUI_ConvertToScreenCoord(&x, &y);
	GUI_ScaleToScreen(&maxWidth, &charHeight);
	GUI_ScaleToScreen(&blah, &iconHeight);
	GUI_SetRGBA(0, 0, 0, a);
	DU_DrawString(x+2, y+2, string, charHeight, iconHeight, maxRows, maxWidth, fontshader, false);
	GUI_SetRGBA(r, g, b, a);
	DU_DrawString(x, y, string, charHeight, iconHeight, maxRows, maxWidth, fontshader, true);
}

void	GUI_DrawString_S(int x, int y, const char *string, int charHeight, int iconHeight, int maxRows, int maxWidth, residx_t fontshader)
{
	GUI_ScaleToScreen(&maxWidth, &charHeight);
	DU_DrawString(x, y, string, charHeight, iconHeight, maxRows, maxWidth, fontshader, true);
}

void	GUI_DrawStringBillboard(camera_t *cam, vec3_t pos, const char *string, int charHeight, int maxRows, residx_t fontshader)
{
	int dummy = 0;
	GUI_ScaleToScreen(&dummy, &charHeight);
	DU_DrawStringBillboard(cam, pos, string, charHeight, maxRows, fontshader);
}

void	GUI_DrawStringMonospaced(int x, int y, const char *string, int charWidth, int charHeight, int maxRows, int maxChars, residx_t fontshader)
{
	GUI_ConvertToScreenCoord(&x, &y);
	GUI_ScaleToScreen(&charWidth, &charHeight);
	DU_DrawStringMonospaced(x, y, string, charWidth, charHeight, maxRows, maxChars, fontshader);

}

int	GUI_StringWidth_S(const char *string, int charHeight, int iconHeight, int maxRows, int maxWidth, residx_t fontshader)
{
	int dummy = 0, width;

	GUI_ScaleToScreen(&maxWidth, &charHeight);
	width = DU_StringWidth(string, charHeight, iconHeight, maxRows, maxWidth, fontshader);
	GUI_Scale(&width, &dummy);
	return width;
}

void	GUI_SetRGB(float r, float g, float b)
{
	color[0] = r;
	color[1] = g;
	color[2] = b;
	color[3] = 1;

	Draw_SetColor(color);
}

void	GUI_SetRGBA(float r, float g, float b, float a)
{
	color[0] = r;
	color[1] = g;
	color[2] = b;
	color[3] = a;

	Draw_SetColor(color);
}
