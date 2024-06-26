// (C) 2003 S2 Games

// drawutils.c

// misc drawing utilities for core engine

#include "core.h"
#include "wchar.h"

#define MIN_FONT_WIDTH 2
#define FONT_SPACE_SIZE 5
#define SPACE_AFTER_ICON 2

static vec3_t up = { 0, 1, 0 };

int DU_CharWidth(int h, int charnum, int residx, fontData_t *fontData, fontGlyphData_t **chars, bitmap_t *bmp)
{
	int width, c;
	//float frow, fcol;

	if (charnum < 0 || charnum >= MAX_POSSIBLE_CHARS)
		return 0;

	switch(charnum)
	{
		case ' ':
			return (int)((float)h*FONT_SPACE_SIZE/12); //space size is the size of a space at a 12-pixel font
		case '\n':
		case '\t':
		case '\r':
			return 0;
		default:
			break;
	}

	c = fontData->charMap[charnum];
	if (!c)
	{
		Res_AddCharToFont(residx, charnum);
		//now that we've added it, try again
		c = fontData->charMap[charnum];
	}

	if (!chars[c])
		return FONT_SPACE_SIZE;

	/*
	frow = (float)chars[c]->topLeft[0]/bmp->width;
	fcol = (float)chars[c]->topLeft[1]/bmp->height;
	*/

	width = MAX(MIN_FONT_WIDTH, (int)((float)h*(float)chars[c]->dimensions[0]/(float)chars[c]->dimensions[1]));

	return width + 1;
}

int DU_DrawChar(int x, int y, int h, int charnum, int residx, fontData_t *fontData, fontGlyphData_t **chars, bitmap_t *bmp, int fontShader)
{
	int width, c;
	float frow, fcol;

	if (charnum < 0 || charnum >= MAX_POSSIBLE_CHARS)
		return 0;

	switch(charnum)
	{
		case ' ':
			return (int)((float)h*FONT_SPACE_SIZE/12); //space size is the size of a space at a 12-pixel font
		case '\n':
		case '\t':
		case '\r':
			return 0;
		default:
			break;
	}
	/*
	if (charnum > 128)
	{
		printf("unicode char: %i\n", charnum);
	}
	*/

	c = fontData->charMap[charnum];
	if (!c)
	{
		Res_AddCharToFont(residx, charnum);
		//now that we've added it, try again
		c = fontData->charMap[charnum];
	}

	if (!chars[c])
		return FONT_SPACE_SIZE;

	frow = (float)chars[c]->topLeft[0]/bmp->width;
	fcol = (float)chars[c]->topLeft[1]/bmp->height;

	width = MAX(MIN_FONT_WIDTH, (int)((float)h*(float)chars[c]->dimensions[0]/(float)chars[c]->dimensions[1]));

	//Console_DPrintf("metrics: at (%i, %i), size (%i, %i)\n", frow, fcol, chars[c]->dimensions[0], chars[c]->dimensions[1]);
	
	Draw_Quad2d(x, y, width, h, 
					frow, 
					fcol, 
					frow + (float)chars[c]->dimensions[0]/bmp->width, 
					fcol + (float)chars[c]->dimensions[1]/bmp->height, 
					fontShader);

	return width + 1;
}

int DU_DrawCharBillboard(vec3_t pos, float h, int c, residx_t font)
{
	int i, which, width, fontShader;
	float frow, fcol;
	fontData_t *fontData;
	fontGlyphData_t **chars;
	bitmap_t *bmp;
	sceneobj_t charImage;

	if (c < 0 || c >= MAX_POSSIBLE_CHARS)
		return 0;

	switch(c)
	{
		case ' ':
			return (int)((float)h*FONT_SPACE_SIZE/12); //space size is the size of a space at a 12-pixel font
		case '\n':
		case '\t':
		case '\r':
			return 0;
		default:
			break;
	}

	fontData = Res_GetFont(font);
	
	for (i = 0; i < MAX_FONT_SIZES + 1; i++)
	{
		if (i == MAX_FONT_SIZES || h <= fontData->fontSizes[i])
		{
			which = MAX(0,MIN(MAX_FONT_SIZES-1, i));
			break;
		}
	}
	fontShader = fontData->fontShaders[which];
	bmp = fontData->bmps[which];
	chars = fontData->chars[which];
	
	//Console_DPrintf("c: %i, slot %i\n", c, fontData->charMap[c]);
	
	c = fontData->charMap[c];
	if (!fontData || !chars[c])
		return FONT_SPACE_SIZE;

	frow = (float)chars[c]->topLeft[0]/bmp->width;
	fcol = (float)chars[c]->topLeft[1]/bmp->height;

	width = MAX(MIN_FONT_WIDTH, (int)((float)h*(float)chars[c]->dimensions[0]/(float)chars[c]->dimensions[1]));

    charImage.objtype = OBJTYPE_BILLBOARD;
    charImage.flags = SCENEOBJ_BILLBOARD_ALL_AXES;
    charImage.alpha = 1;
	M_CopyVec3(pos, charImage.pos);
	charImage.width = width;
	charImage.height = h;
	charImage.shader = fontShader;
	charImage.s1 = frow; 
	charImage.t1 = fcol; 
	charImage.s2 = frow + (float)chars[c]->dimensions[0]/bmp->width;
	charImage.t2 = fcol + (float)chars[c]->dimensions[1]/bmp->height;
	
	Scene_AddObject(&charImage);

	return width + 1;
}

void DU_DrawCharMonospaced(int x, int y, int w, int h, int c, residx_t fontshader)
{
	int row, col;
	float frow, fcol;
	float size;

	if (c < 0 || c >= 128)
		return;

	switch(c)
	{
		case ' ':
		case '\n':
		case '\t':
		case '\r':
			return;
		default:
			break;
	}

	row = c>>4;
	col = c&15;

	frow = (row*0.0625);
	fcol = (col*0.0625);
	size = 0.0625;

	Draw_Quad2d(x, y, w, h, fcol, frow, fcol+size, frow+size, fontshader);
}

void DU_DrawString(int x, int y, const unsigned char *string, int charHeight, int iconHeight, int maxRows, int maxWidth, residx_t fontshader, bool docolors) 
{
	wchar_t		ws[1024];
	wchar_t 	*s;
	int			xx;
	int			i, which;
	int			cnt;
	int 		len;
	int			s_len;
	int			rows = 0;
	int			realFontShader;
	wchar_t		icon_wstring[10];
	wchar_t		clan_wstring[10];
	fontData_t *fontData;
	fontGlyphData_t **chars;
	bitmap_t 	*bmp;

	mbstowcs(icon_wstring, "^icon ", 10);
	icon_wstring[9] = 0;

	mbstowcs(clan_wstring, "^clan ", 10);
	clan_wstring[9] = 0;

	if (string[0] == 0)
		return;

	string = _(string);
	
	/*
	//try to catch that crash...
	for (xx = 0; xx < 1024; xx++)
	{
		if (string[xx] == 0)
			break;
	}
	if (xx == 1024)
	{
		char ss[1025];

		strncpy(ss, string, 1024);
		ss[1024] = 0;
		Console_DPrintf("*** Possible garbage string:\n%s\n", string);
		Game_Error(fmt("*** Possible garbage string:\n%s\n", string));
	}
	*/

	fontData = Res_GetFont(fontshader);
	if (!fontData)
		return;
	
	for (i = 0; i < MAX_FONT_SIZES + 1; i++)
	{
		if (i == MAX_FONT_SIZES || charHeight <= fontData->fontSizes[i])
		{
			which = MAX(0,MIN(MAX_FONT_SIZES-1, i));
			break;
		}
	}
	realFontShader = fontData->fontShaders[which];
	bmp = fontData->bmps[which];
	chars = fontData->chars[which];
	
	//Console_DPrintf("c: %i, slot %i\n", c, fontData->charMap[c]);
	
	len = strlen(string);
	ws[1023]  = 0;
	s_len = mbstowcs(ws, string, 1023);
	if (s_len == -1)
	{
		Console_DPrintf("Invalid multibyte string %s\n", string);
		return;
	}
	ws[s_len] = 0;
	s = ws;
	xx = x;
	cnt = 0;
	while ( *s && rows < maxRows) 
	{	
		switch (*s)
		{
			case '\n':
				rows++;
				xx = x;
				//NOTE: we might want to change this +1 to a param
				y += charHeight + 1;
				cnt = 0;
				break;

			case '^':
				//we could be drawing an icon
				if (wmemcmp(s, icon_wstring, 6)==0 && wcslen(s) > 7 && wcschr(&s[7], '^'))
				{
					char icon[1024];
					int i=0;
					const wchar_t *start = s;
					s+=6;

					while (*s != 0 && *s != '\n' && *s != ' ' && *s != '^' && i < 1023)
					{
						icon[i] = *s;

						s++;
						i++;
					}
					
					//try to show this
					if (i > 0 && *s == '^')
					{
						icon[i] = 0;

						Draw_Quad2d(xx, y, iconHeight, iconHeight, 0, 0, 1, 1, Res_LoadShader(fmt("/textures/econs/%s.tga", icon)));

						xx += iconHeight;

						s++;
						continue;
					}
					else
					{
						s = (wchar_t *)start;
					}
				}
				//we could be drawing a clan icon
				else if (wmemcmp(s, clan_wstring, 6)==0 && wcslen(s) > 7 && wcschr(&s[7], '^'))
				{
					char icon[1024];
					int i=0;
					const wchar_t *start = s;
					s+=6;

					while (*s != 0 && *s != '\n' && *s != ' ' && *s != '^' && i < 1023)
					{
						icon[i] = *s;

						s++;
						i++;
					}
					
					//try to show this
					if (i > 0 && *s == '^')
					{
						icon[i] = 0;

						Draw_Quad2d(xx, y, iconHeight, iconHeight, 0, 0, 1, 1, File_GetClanIcon(atoi(icon)));

						xx += iconHeight + SPACE_AFTER_ICON;

						s++;
						continue;
					}
					else
					{
						s = (wchar_t *)start;
					}
				}
				//check for a color change
				else
				{
					bool valid = true;
					int colorCodeLen = 1;
					vec4_t	tmpcolor = {1.0, 1.0, 1.0, 1.0};

					tmpcolor[3] = Draw_GetCurrentAlpha();

					s += 1;

					switch (*s)
					{
					case 'r':
						//let's use a brighter red to be more easily readable
						tmpcolor[1] = 0.3;
						tmpcolor[2] = 0.3;
						break;
					case 'g':
						tmpcolor[0] = 0.0;
						tmpcolor[2] = 0.0;
						break;
					case 'b':
						//let's use a brighter blue to be more easily readable
						tmpcolor[0] = 0.3;
						tmpcolor[1] = 0.3;
						break;
					case 'w':
						break;
					case 'k':
						tmpcolor[0] = 0.0;
						tmpcolor[1] = 0.0;
						tmpcolor[2] = 0.0;
						break;
					case 'y':
						tmpcolor[2] = 0.1;
						break;
					case 'c':
						tmpcolor[0] = 0.0;
						break;
					case 'm':
						tmpcolor[1] = 0.0;
						break;
					case '0':
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
					case '8':
					case '9':
						if (wcslen(s) < 3)
						{
							s -= 1;
							valid = false;
							break;
						}

						if (s[1] >= '0' && s[1] <= '9' &&
							s[2] >= '0' && s[2] <= '9')
						{
							tmpcolor[0] = ((s[0] - '0') / 9.0);
							tmpcolor[1] = ((s[1] - '0') / 9.0);
							tmpcolor[2] = ((s[2] - '0') / 9.0);
							colorCodeLen = 3;
						}
						else
						{
							s -= 1;
							valid = false;
							break;
						}
						break;
					default:
						s -= 1;
						valid = false;
						break;
					}

					if (valid)
					{
						s += colorCodeLen;
						if (docolors)
							Draw_SetColor(tmpcolor);
						continue;
					}
				}
				//*** intentional fall through ***
			default:
				if (xx - x < maxWidth)
					xx += DU_DrawChar(xx, y, charHeight, *s, fontshader, fontData, chars, bmp, realFontShader);
				cnt++;
		}
		s++;
	}
}

void DU_DrawStringBillboard(camera_t *cam, vec3_t pos, const unsigned char *string, int charHeight, int maxRows, residx_t fontshader) 
{
#ifndef _S2_DONT_INCLUDE_GL
	const wchar_t	*s;
	wchar_t		ws[512];
	int			xx;
	int			cnt;
	int			len;
	int			s_len;
	int			rows = 0;
	int			width;
	vec3_t start, towards, right;

	string = _(string);

	len = strlen(string);
	s_len = mbstowcs(ws, string, 512);
	ws[s_len] = 0;
	s = ws;
	
	M_CopyVec3(pos, start);
	
	M_CopyVec3(cam->viewaxis[FORWARD], towards);
	
	glNormal3fv(towards);
	M_CrossProduct(towards, up, right);
	glNormal3fv(right);
	
	//offset it to the left by half the string width
	width = DU_StringWidth(string, charHeight, charHeight, maxRows, 1024, fontshader);
	pos[X] -= right[0] * width/2;
	pos[Y] -= right[1] * width/2;
	
	cnt = 0;
	while ( *s && rows < maxRows) 
	{	
		switch (*s)
		{
			case '\n':
						rows++;
						pos[X] = start[X] - right[0] * width/2;
						pos[Y] = start[Y] - right[1] * width/2;
						//NOTE: we might want to change this +1 to a param
						pos[Z] -= charHeight + 1;
						cnt = 0;
						break;
			default:
						xx = DU_DrawCharBillboard(pos, charHeight, *s, fontshader);
						pos[X] += right[0] * xx;
						pos[Y] += right[1] * xx;
						cnt++;
		}
		s++;
	}
#endif
}

int DU_StringWidth(const unsigned char *string, int charHeight, int iconHeight, int maxRows, int maxWidth, residx_t fontshader) 
{
	const wchar_t	*s;
	wchar_t		ws[512];
	int			xx;
	int			cnt;
	int			maxRowLength = 0;
	int			rows = 0;
	int			i, which = 0;
	int			len;
	int			s_len;
	wchar_t		icon_wstring[10];
	wchar_t		clan_wstring[10];
	fontData_t *fontData;
	fontGlyphData_t **chars;
	bitmap_t 	*bmp;

	mbstowcs(icon_wstring, "^icon ", 10);
	icon_wstring[9] = 0;

	mbstowcs(clan_wstring, "^clan ", 10);
	clan_wstring[9] = 0;

	fontData = Res_GetFont(fontshader);
	if (!fontData)
		return 0;
	
	for (i = 0; i < MAX_FONT_SIZES + 1; i++)
	{
		if (i == MAX_FONT_SIZES || charHeight <= fontData->fontSizes[i])
		{
			which = MAX(0,MIN(MAX_FONT_SIZES-1, i));
			break;
		}
	}
	bmp = fontData->bmps[which];
	chars = fontData->chars[which];

	string = _(string);
	len = strlen(string);
	s_len = mbstowcs(ws, string, 512);
	ws[s_len] = 0;
	s = ws;
	xx = 0;
	cnt = 0;
	while ( *s && xx < maxWidth && rows < maxRows) 
	{	
		switch (*s)
		{
			case '\n':
						rows++;
						if (xx > maxRowLength)
							maxRowLength = xx;
						xx = 0;
						//NOTE: we might want to change this +1 to a param
						cnt = 0;
						break;
			case '^':
				//we could be drawing an icon
				if (wmemcmp(s, icon_wstring, 6)==0 && wcslen(s) > 7 && wcschr(&s[7], '^'))
				{
					char icon[1024];
					int i=0;
					const wchar_t *start = s;
					s+=6;

					while (*s != 0 && *s != '\n' && *s != ' ' && *s != '^' && i < 1023)
					{
						icon[i] = *s;

						s++;
						i++;
					}
					
					//try to show this
					if (i > 0 && *s == '^')
					{
						icon[i] = 0;

						xx += iconHeight;

						s++;
						continue;
					}
					else
					{
						s = (wchar_t *)start;
					}
				}
				//we could be drawing an icon
				else if (wmemcmp(s, clan_wstring, 6)==0 && wcslen(s) > 7 && wcschr(&s[7], '^'))
				{
					char icon[1024];
					int i=0;
					const wchar_t *start = s;
					s+=6;

					while (*s != 0 && *s != '\n' && *s != ' ' && *s != '^' && i < 1023)
					{
						icon[i] = *s;

						s++;
						i++;
					}
					
					//try to show this
					if (i > 0 && *s == '^')
					{
						icon[i] = 0;

						xx += iconHeight + SPACE_AFTER_ICON;

						s++;
						continue;
					}
					else
					{
						s = (wchar_t *)start;
					}
				}
				//check for a color change
				else
				{
					bool valid = true;
					vec4_t	tmpcolor = {1.0, 1.0, 1.0, 1.0};

					s += 1;

					switch (*s)
					{
					case 'r':
						tmpcolor[1] = 0.0;
						tmpcolor[2] = 0.0;
						break;
					case 'g':
						tmpcolor[0] = 0.0;
						tmpcolor[2] = 0.0;
						break;
					case 'b':
						tmpcolor[0] = 0.0;
						tmpcolor[1] = 0.0;
						break;
					case 'w':
						break;
					case 'k':
						tmpcolor[0] = 0.0;
						tmpcolor[1] = 0.0;
						tmpcolor[2] = 0.0;
						break;
					case 'y':
						tmpcolor[2] = 0.0;
						break;
					case 'c':
						tmpcolor[0] = 0.0;
						break;
					case 'm':
						tmpcolor[1] = 0.0;
						break;
					default:
						s -= 1;
						valid = false;
						break;
					}

					if (valid)
					{
						s += 1;
						continue;
					}
				}
				/* intentional fall-through */
			default:
						xx += DU_CharWidth(charHeight, *s, fontshader, fontData, chars, bmp);
						cnt++;
		}
		s++;
	}
	if (xx > maxRowLength)
		maxRowLength = xx;
	return maxRowLength;
}

void DU_DrawStringMonospaced(int x, int y, const unsigned char *string, int charWidth, int charHeight, int maxRows, int maxChars, residx_t fontshader) {
	const char	*s;
	int			xx;
	int			cnt;
	int			rows = 0;
	s = _(string);
	xx = x;
	cnt = 0;
	while ( *s && cnt < maxChars && rows < maxRows) 
	{	
		switch (*s)
		{
			case '\n':
						rows++;
						xx = x;
						//NOTE: we might want to change this +1 to a param
						y += charHeight + 1;
						cnt = 0;
						break;
			case '^':
				//we could be drawing an icon
				if (memcmp(s, "^icon ", 6)==0 && strlen(s) > 7 && strchr(&s[7], '^'))
				{
					char icon[1024];
					int i=0;
					const char *start = s;
					s+=6;

					while (*s != 0 && *s != '\n' && *s != ' ' && *s != '^' && i < 1023)
					{
						icon[i] = *s;

						s++;
						i++;
					}
					
					//try to show this
					if (i > 0 && *s == '^')
					{
						icon[i] = 0;

						Draw_Quad2d(xx, y, charHeight, charHeight, 0, 0, 1, 1, Res_LoadShader(fmt("/textures/econs/%s.tga", icon)));

						xx += charHeight;

						s++;
						continue;
					}
					else
					{
						s = (char *)start;
					}
				}
				//we could be drawing an icon
				else if (memcmp(s, "^clan ", 6)==0 && strlen(s) > 7 && strchr(&s[7], '^'))
				{
					char icon[1024];
					int i=0;
					const char *start = s;
					s+=6;

					while (*s != 0 && *s != '\n' && *s != ' ' && *s != '^' && i < 1023)
					{
						icon[i] = *s;

						s++;
						i++;
					}
					
					//try to show this
					if (i > 0 && *s == '^')
					{
						icon[i] = 0;

						Draw_Quad2d(xx, y, charHeight, charHeight, 0, 0, 1, 1, File_GetClanIcon(atoi(icon)));

						xx += charHeight + SPACE_AFTER_ICON;

						s++;
						continue;
					}
					else
					{
						s = (char *)start;
					}
				}
				//check for a color change
				else
				{
					bool valid = true;
					vec4_t	tmpcolor = {1.0, 1.0, 1.0, 1.0};

					s += 1;

					switch (*s)
					{
					case 'r':
						tmpcolor[1] = 0.0;
						tmpcolor[2] = 0.0;
						break;
					case 'g':
						tmpcolor[0] = 0.0;
						tmpcolor[2] = 0.0;
						break;
					case 'b':
						tmpcolor[0] = 0.0;
						tmpcolor[1] = 0.0;
						break;
					case 'w':
						break;
					case 'k':
						tmpcolor[0] = 0.0;
						tmpcolor[1] = 0.0;
						tmpcolor[2] = 0.0;
						break;
					case 'y':
						tmpcolor[2] = 0.0;
						break;
					case 'c':
						tmpcolor[0] = 0.0;
						break;
					case 'm':
						tmpcolor[1] = 0.0;
						break;
					default:
						s -= 1;
						valid = false;
						break;
					}

					if (valid)
					{
						Draw_SetColor(tmpcolor);
						s += 1;
						continue;
					}
				}
				/* intentional fall-through */
			default:
						DU_DrawCharMonospaced(xx, y, charWidth, charHeight, *s, fontshader);
						xx += charWidth;
						cnt++;
		}
		s++;
	}
}

void DU_DrawRotatedQuad(int x, int y, int w, int h, float ang, int s1, int t1, int s2, int t2, residx_t shader)
{
	vec2_t v1, v2, v3, v4;
	float hw,hh;

	if (ang < 0)
		ang += 360;
	else if (ang > 360)
		ang -= 360;

	hw = w/2;
	hh = h/2;

	v1[0] = -hw;
	v1[1] = -hh;
	v2[0] = -hw;
	v2[1] = hh;
	v3[0] = hw;
	v3[1] = hh;
	v4[0] = hw;
	v4[1] = -hh;
	
	M_RotateVec2(ang, v1);
	M_RotateVec2(ang, v2);
	M_RotateVec2(ang, v3);
	M_RotateVec2(ang, v4);
	
	v1[0] += (x+hw);
	v1[1] += (y+hh);
	v2[0] += (x+hw);
	v2[1] += (y+hh);
	v3[0] += (x+hw);
	v3[1] += (y+hh);
	v4[0] += (x+hw);
	v4[1] += (y+hh);

	Draw_Poly2d(v1, v2, v3, v4, s1, t1, s2, t2, shader);
}
