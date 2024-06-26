// (C) 2003 S2 Games

// le_tools.c

// level editor tools

#include "../le/le.h"

residx_t	current_terrainshader = 0;
tmat_t		*current_tmat = NULL;
int		current_brush = 0;
float	brush_strength = 1;
byte	le_brushes[MAX_BRUSHES][MAX_BRUSH_SIZE][MAX_BRUSH_SIZE];
float	(*current_modelfunc)(int gridx, int gridy, float source, float strength) = NULL;

int		tmatcycle = 0;
cvar_t	le_tmatcycle = {"le_tmatcycle", "1", true};
cvar_t	le_terrainshader = {"le_terrainshader", ""};
cvar_t	le_layer2 = { "le_layer2", "0", true };



void	LE_SelectBrush(int brush)
{
	if (brush<0 || brush>=MAX_BRUSHES) return;

	current_brush = brush;
}

int		LE_CurrentBrush()
{
	return current_brush;
}

void	LE_SetTerrainShader(residx_t shader)
{
	if (current_terrainshader < 0) return;

	current_terrainshader = shader;
}

residx_t	LE_CurrentTerrainShader()
{
	return current_terrainshader;
}

void	LE_LoadBrush(byte brush[MAX_BRUSH_SIZE][MAX_BRUSH_SIZE])
{
	memcpy(le_brushes[current_brush], brush, MAX_BRUSH_SIZE*MAX_BRUSH_SIZE);
}

void	LE_SetBrushColor(vec4_t color)
{
}

void	LE_SetBrushStrength(float strength)
{
	brush_strength = strength;
}

float	LE_GetBrushStrength()
{
	return brush_strength;
}

void	LE_ModelTerrain(int xpos, int ypos)
{
	int x, y, brushx, brushy;
	heightmap_t newheight, oldheight;
	float brushpixel;

	if (!current_modelfunc) return;

	for (y=ypos-16, brushy=0; y<ypos+16 && brushy<32; y++, brushy++)
		for (x=xpos-16, brushx=0; x<xpos+16 && brushx<32; x++, brushx++)
		{
			brushpixel = ((float)le_brushes[current_brush][brushy][brushx] / 255.0);
			oldheight = corec.World_GetGridHeight(x, y);
			newheight = current_modelfunc(x, y, oldheight, brushpixel * brush_strength);

			corec.World_DeformGround(x, y, newheight);
		}
}

void	LE_ModelTerrainXY(int gridx, int gridy, float strength)
{
	float oldheight, newheight;

	if (!current_modelfunc)
		return;

	oldheight = corec.World_GetGridHeight(gridx, gridy);
	newheight = current_modelfunc(gridx, gridy, oldheight, strength);
	corec.World_DeformGround(gridx, gridy, newheight);
}


void	LE_ColorTerrain(int xpos, int ypos)
{
	int x, y, brushx, brushy;
	float brushpixel;
	vec4_t mapcolor;
	bvec4_t mapcol;
	vec4_t destcolor;
	vec4_t outcolor;
	bvec4_t outcol;
	int n;

	//if (!current_modelfunc) return;

	for (y=ypos-16, brushy=0; y<ypos+16 && brushy<32; y++, brushy++)
		for (x=xpos-16, brushx=0; x<xpos+16 && brushx<32; x++, brushx++)
		{
			brushpixel = ((float)le_brushes[current_brush][brushy][brushx] / 255.0);
			//oldheight = World_GetGridHeight(x, y);
			//newheight = current_modelfunc(x, y, oldheight, brushpixel * brush_strength);
			
			corec.WR_GetColormap(x, y, mapcol);
			corec.Color_ToFloat(mapcol, mapcolor);

			destcolor[0] = le_brushr.value*0.5;
			destcolor[1] = le_brushg.value*0.5;
			destcolor[2] = le_brushb.value*0.5;

			outcolor[0] = LERP(brushpixel * le_brushstrength.value, mapcolor[0], destcolor[0]);
			outcolor[1] = LERP(brushpixel * le_brushstrength.value, mapcolor[1], destcolor[1]);
			outcolor[2] = LERP(brushpixel * le_brushstrength.value, mapcolor[2], destcolor[2]);
			outcolor[3] = mapcolor[3]; 
			for (n=0; n<3; n++)
				if (outcolor[n] > 1) outcolor[n] = 1;

			corec.Color_ToByte(outcolor, outcol);

			corec.WR_SetColormap(x, y, outcol);
		}
}

void	LE_PaintAlpha(int xpos, int ypos, float destalpha)
{
	int x, y, brushx, brushy;
	float brushpixel;
	bvec4_t mapcol;
	vec4_t mapcolor;	

	for (y=ypos-16, brushy=0; y<ypos+16 && brushy<32; y++, brushy++)
		for (x=xpos-16, brushx=0; x<xpos+16 && brushx<32; x++, brushx++)
		{
			brushpixel = ((float)le_brushes[current_brush][brushy][brushx] / 255.0);			

			if (brushpixel < 0.2) continue;
			if (le_layer2.value)
			{
				if (brushpixel)
				{
					corec.WR_GetColormap(x, y, mapcol);
					corec.Color_ToFloat(mapcol, mapcolor);		
	
					mapcolor[3] = LERP(brushpixel * le_brushstrength.value, mapcolor[3], destalpha);
	
					corec.Color_ToByte(mapcolor, mapcol);
	
					corec.WR_SetColormap(x, y, mapcol);									
				}
			}			
		}
}

void	LE_TextureTerrain(int xpos, int ypos)
{
	int x, y, brushx, brushy;
	float brushpixel;

	for (y=ypos-16, brushy=0; y<ypos+16 && brushy<32; y++, brushy++)
		for (x=xpos-16, brushx=0; x<xpos+16 && brushx<32; x++, brushx++)
		{
			brushpixel = ((float)le_brushes[current_brush][brushy][brushx] / 255.0);

			//if (brushpixel < 0.4) continue;

			if (le_layer2.value)
			{
				if (brushpixel)							
					corec.WR_PaintShader(x, y, corec.Res_LoadTerrainShader(le_terrainshader.string), true);				
			}
			else
			{
				if (brushpixel)
					corec.WR_PaintShader(x, y, corec.Res_LoadTerrainShader(le_terrainshader.string), false);
			}
		}
}

void	LE_PaintTMat(int xpos, int ypos, float destAlpha)
{
	int x, y, tmatx, tmaty;
	residx_t shader;
	tmat_t	*tm = current_tmat;

	if (!tm) return;

	for (y=ypos, tmaty=0; y<ypos+tm->height && tmaty<tm->height; y++, tmaty++)
		for (x=xpos, tmatx=0; x<xpos+tm->width && tmatx<tm->width; x++, tmatx++)
		{
			if (le_tmatcycle.value)
				shader = tm->shapes[tm->cur_shape][(tmaty+tmatcycle) % tm->height][(tmatx+tmatcycle) % tm->width];
			else
				shader = tm->shapes[tm->cur_shape][tmaty][tmatx];

			if (!shader) continue;

			if (le_layer2.value)
			{
		
				corec.WR_PaintShader(x, y, shader, true);
			}
			else
			{
				corec.WR_PaintShader(x, y, shader, false);
			}
		}

	if (le_tmatcycle.value) tmatcycle++;
}


void	LE_SetModelFunction(float(*func)(int gridx, int gridy, float source, float strength))
{
	current_modelfunc = func;
}

void	*LE_CurrentModelFunction()
{
	return current_modelfunc;
}


void	LE_LoadBrushBmp(char *filename)
{
	int x, y;
	bitmap_t bmp;
	bvec4_t col;
	byte i;
	
	memset(le_brushes[current_brush], 0, MAX_BRUSH_SIZE*MAX_BRUSH_SIZE);
	
	if (!corec.Bitmap_Load(filename, &bmp)) 
		return;

	if (!bmp.data) return;

	for (y=0; y<bmp.height || y<32; y++)
		for (x=0; x<bmp.width || x<32; x++)
		{
			corec.Bitmap_GetColor(&bmp, x, y, col);
			i = col[0];
			if (i<10) i=0;
			le_brushes[current_brush][y][x] = i;
		}

	corec.Bitmap_Free(&bmp);
}

void		LE_SetTMat(tmat_t *tmat)
{
	current_tmat = tmat;
}

tmat_t		*LE_CurrentTMat()
{
	return current_tmat;
}


void		LE_LoadTMat(const char *filename)
{
	file_t *f;	
	int ret;
	char s[1024];
	char line[1024];
	int i = 0, x, y;
	tmat_t *tm = current_tmat;

	if (!current_tmat) return;

	memset(tm->shaders, 0, sizeof(tm->shaders));	
	tm->icon = 0;
	tm->num_shaders = 0;

	f = core.File_Open(filename, "rt");

	if (!f)
	{
		corec.Console_Printf("LE_LoadTMat: Error loading %s\n", filename);				
		return;
	}

	core.File_gets(line, 1024, f);
	sscanf(line, "width = %i\n", &tm->width);
	core.File_gets(line, 1024, f);
	sscanf(line, "height = %i\n", &tm->height);
	
	while (1)
	{
		core.File_gets(line, 1024, f);
		ret = sscanf(line, "%i = %s", &i, s);
		if (ret==EOF || !ret) 			
		{
			core.File_Close(f);
			break;
		}
		if (i>=0 && i<=MAX_TMAT_SHADERS-1)
		{
			tm->shaders[i] = corec.Res_LoadTerrainShader(s);
			tm->num_shaders++;
		}
	}

	if (tm->num_shaders)
	{
		tm->icon = tm->shaders[0];
	}

	i=0;

	memset(tm->shapes, 0, sizeof(tm->shapes));

	for (y=0; y<tm->height; y++)
	{
		for (x=0; x<tm->width; x++)
		{
			tm->shapes[0][y][x] = tm->shaders[i];
			i++;
		}
	}
	tm->cur_shape = 0;	
}


void	LE_InitTools()
{	
	corec.Cvar_Register(&le_tmatcycle);
	corec.Cvar_Register(&le_terrainshader);
	corec.Cvar_Register(&le_layer2);
}
