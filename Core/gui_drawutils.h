// (C) 2003 S2 Games

// gui_drawutils.h

// accepts screen coords
void    GUI_RotatedQuad2d_S(int x, int y, int w, int h, float ang, residx_t shader);
void	GUI_Quad2d_S(int x, int y, int w, int h, residx_t shader);
void	GUI_ShadowQuad2d_S(int x, int y, int w, int h, residx_t shader);
void    GUI_GetStringRowsCols(char *str, int *rows, int *cols);
void	GUI_DrawString(int x, int y, const char *string, int charHeight, int iconHeight, int maxRows, int maxWidth, residx_t fontshader);
void	GUI_DrawShadowedString(int x, int y, const char *string, int charHeight, int iconHeight, int maxRows, int maxWidth, residx_t fontshader, float r, float g, float b, float a);
void	GUI_DrawString_S(int x, int y, const char *string, int charHeight, int iconHeight, int maxRows, int maxWidth, residx_t fontshader);
void    GUI_DrawStringBillboard(camera_t *cam, vec3_t pos, const char *string, int charHeight, int maxRows, residx_t fontshader);
void    GUI_DrawStringMonospaced(int x, int y, const char *string, int charWidth, int charHeight, int maxRows, int maxChars, residx_t fontshader);
int		GUI_StringWidth_S(const char *string, int charHeight, int iconHeight, int maxRows, int maxWidth, residx_t fontshader);
void	GUI_LineBox2d_S(int x, int y, int w, int h, int thickness);

void	GUI_OffsetCoords(int x, int y);

void	GUI_SetRGB(float r, float g, float b);

void	GUI_SetRGBA(float r, float g, float b, float a);
