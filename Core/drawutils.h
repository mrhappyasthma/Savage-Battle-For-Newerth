// (C) 2003 S2 Games

// drawutils.h

// misc drawing utilities for core engine

//int DU_DrawChar(int x, int y, int h, int c, residx_t fontshader);
void DU_DrawString(int x, int y, const unsigned char *string, int charHeight, int iconHeight, int maxRows, int maxWidth, residx_t fontshader, bool docolors);
void DU_DrawStringBillboard(camera_t *cam, vec3_t pos, const unsigned char *string, int charHeight, int maxRows, residx_t fontshader);
int DU_StringWidth(const unsigned char *string, int charHeight, int iconHeight, int maxRows, int maxWidth, residx_t fontshader);

void DU_DrawCharMonospaced(int x, int y, int w, int h, int c, residx_t fontshader);
void DU_DrawStringMonospaced(int x, int y, const unsigned char *string, int charWidth, int charHeight, int maxRows, int maxChars, residx_t fontshader);

void DU_DrawRotatedQuad(int x, int y, int w, int h, float ang, int s1, int t1, int s2, int t2, residx_t shader);

