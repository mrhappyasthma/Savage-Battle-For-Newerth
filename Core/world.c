// (C) 2003 S2 Games

// world.c

// world loading, manipulating, querying, rendering

#include "core.h"
#include "zip.h"

#define WORLD_MAX_GRIDWIDTH	1024
#define	WORLD_MAX_GRIDHEIGHT 1024

world_t world = {0, 0, NULL, 0, 0, 0};
worldrender_t wr = {NULL, NULL, {NULL, NULL}};
cvar_t	wr_treedepth = {"wr_treedepth", "7"};
cvar_t	wr_lightheight = {"wr_lightheight", "0", CVAR_WORLDCONFIG | CVAR_TERRAIN_FX};
cvar_t  wr_tiling = {"wr_tiling", "1", true};
cvar_t	wr_showtree = {"wr_showtree", "0", true};
cvar_t	wr_tracemethod = {"wr_tracemethod", "quadtree", true}; //can also be set to "dda"
cvar_t	wr_raytrace_step = {"wr_raytrace_step", "0.1", true};
cvar_t	wr_raytrace_offset = {"wr_raytrace_offset", "20", true};
cvar_t	wr_marks = {"wr_marks", "1", CVAR_SAVECONFIG};
cvar_t	wr_markrange = {"wr_markrange", "30", CVAR_SAVECONFIG};

cvar_t	world_save_path = {"world_save_path", "/../game"};
cvar_t	world_name	= {"world_name", ""};
cvar_t	world_overhead	= {"world_overhead", ""};
cvar_t	world_scale = { "world_scale", "1", CVAR_WORLDCONFIG };
cvar_t	world_heightscale = { "world_heightscale", "1", CVAR_WORLDCONFIG };

scenelight_t	terrainLights[2];


extern server_t localServer;
extern localClientInfo_t clientLocal;

#ifndef _S2_DONT_INCLUDE_GL
extern cvar_t gfx_foliageMaxSlope;
extern cvar_t gfx_foliageSmooth;
#endif

typedef struct
{
	vec3_t	a[3];
	vec3_t	a_nml;
	float	a_dist;
	vec3_t	b[3];
	vec3_t	b_nml;
	float	b_dist;
} gridcell_t;

void	WR_GenTexcoords();
void	WR_GenColormap();
void	WR_GenShadermap();
void	WR_GenShadermap2();
void	WR_GenShaderRefs();
void	WR_RecalcNormal(int x, int y);
void	WR_GetNormal(int x, int y, vec3_t nml, int tri);
void	World_Save_Cmd(int argc, char *argv[]);
void	WR_CacheFoliage_Cmd(int argc, char *argv[]);
void 	World_ImportTOD_Cmd(int argc, char *argv[]);

void	World_Info_Cmd(int argc, char *argv[])
{
	if (!world.loaded)
		return;

	Console_Printf("Info for world \"%s\"\n", world.name);
	Console_Printf("--------------\n\n");
	Console_Printf("Grid width: %i\n", world.gridwidth);
	Console_Printf("Grid height: %i\n", world.gridheight);
	Console_Printf("MinZ: %i\n", world.min_z);
	Console_Printf("MaxZ: %i\n", world.max_z);
	Console_Printf("Num occluders: %i\n", world.numOccluders);
}

void	World_Init()
{
	Cvar_Register(&wr_treedepth);
	Cvar_Register(&wr_lightheight);
	Cvar_Register(&wr_tiling);
	Cvar_Register(&wr_showtree);
	Cvar_Register(&wr_raytrace_step);
	Cvar_Register(&wr_raytrace_offset);
	Cvar_Register(&wr_tracemethod);
	Cvar_Register(&wr_marks);
	Cvar_Register(&wr_markrange);

	Cvar_Register(&world_overhead);
	Cvar_Register(&world_name);
	Cvar_Register(&world_scale);
	Cvar_Register(&world_heightscale);

	Cmd_Register("writeworld", World_Save_Cmd);
	Cmd_Register("save", World_Save_Cmd);
	Cmd_Register("smoothFoliage", WR_CacheFoliage_Cmd);
	Cmd_Register("tod_import", World_ImportTOD_Cmd);
	Cmd_Register("worldInfo", World_Info_Cmd);
	Cvar_Register(&world_save_path);

	memset(&world, 0, sizeof(world_t));

	WT_Init();
	WO_Init();
	World_InitLighting();
}

float World_WorldToGrid(float coord)
{
	return WORLD_TO_GRID(coord);
}

float World_GridToWorld(float gridcoord)
{
	return GRID_TO_WORLD(gridcoord);
}

void	World_ClampCoords(float *x1, float *y1, float *x2, float *y2)
{
	float *bmin = world.tree[0]->bmin;
	float *bmax = world.tree[0]->bmax;

	if (*x1 > bmax[0])
		*x1 = bmax[0];
	else if (*x1 < bmin[0])
		*x1 = bmin[0];

	if (*x2 > bmax[0])
		*x2 = bmax[0];
	else if (*x2 < bmin[0])
		*x2 = bmin[0];

	if (*y1 > bmax[1])
		*y1 = bmax[1];
	else if (*y1 < bmin[1])
		*y1 = bmin[1];

	if (*y2 > bmax[1])
		*y2 = bmax[1];
	else if (*y2 < bmin[1])
		*y2 = bmin[1];
}

#define _SWAP(a,b) (temp = (a), (a) = (b), (b) = temp)

float	World_CalcMaxZUnaligned(float x1, float y1, float x2, float y2)
{
	int x,y,wx1,wx2,wy1,wy2;
	float temp;

	World_ClampCoords(&x1, &y1, &x2, &y2);

	wx1 = ceil(x1/WORLD_TILESIZE);
	wy1 = ceil(y1/WORLD_TILESIZE);
	wx2 = floor(x2/WORLD_TILESIZE);
	wy2 = floor(y2/WORLD_TILESIZE);

	if (wx1 > wx2)
		_SWAP(wx1,wx2);
	if (wy1 > wy2)
		_SWAP(wy1,wy2);
	
	temp = -FAR_AWAY;

	for(y=wy1; y<=wy2; y++)
		for (x=wx1; x<=wx2; x++)
		{
			float gridref = WORLD_GRIDREF(x,y);

			if ( temp < gridref )
				temp = gridref;
		}

	{
		pointinfo_t pi;

		World_SampleGround(x1, y1, &pi);
		temp = MAX(pi.z, temp);

		World_SampleGround(x1, y2, &pi);
		temp = MAX(pi.z, temp);
		
		World_SampleGround(x2, y2, &pi);
		temp = MAX(pi.z, temp);
		
		World_SampleGround(x2, y1, &pi);
		temp = MAX(pi.z, temp);
	}

	return temp;	
}

float	World_CalcMinZUnaligned(float x1, float y1, float x2, float y2)
{
	int x,y,wx1,wx2,wy1,wy2;
	float temp;

	World_ClampCoords(&x1, &y1, &x2, &y2);

	wx1 = ceil(x1/WORLD_TILESIZE);
	wy1 = ceil(y1/WORLD_TILESIZE);
	wx2 = floor(x2/WORLD_TILESIZE);
	wy2 = floor(y2/WORLD_TILESIZE);
	
	if (wx1 > wx2)
		_SWAP(wx1,wx2);
	if (wy1 > wy2)
		_SWAP(wy1,wy2);

	temp = FAR_AWAY;

	for(y=wy1; y<=wy2; y++)
		for (x=wx1; x<=wx2; x++)
		{
			float gridref = WORLD_GRIDREF(x,y);

			if ( temp > gridref )
				temp = gridref;
		}

	{
		pointinfo_t pi;

		World_SampleGround(x1, y1, &pi);
		temp = MIN(pi.z, temp);

		World_SampleGround(x1, y2, &pi);
		temp = MIN(pi.z, temp);
		
		World_SampleGround(x2, y2, &pi);
		temp = MIN(pi.z, temp);
		
		World_SampleGround(x2, y1, &pi);
		temp = MIN(pi.z, temp);
	}

	return temp;	
}

float	World_CalcMaxZ(float x1, float y1, float x2, float y2)
{
	int x,y;
	float temp;

	World_ClampCoords(&x1, &y1, &x2, &y2);

	if (x1 > x2)
		_SWAP(x1,x2);
	if (y1 > y2)
		_SWAP(y1,y2);

	temp = -FAR_AWAY;

	for(y=WORLD_TO_GRID(y1); y<=WORLD_TO_GRID(y2); y++)
		for (x=WORLD_TO_GRID(x1); x<=WORLD_TO_GRID(x2); x++)
		{
			float gridref;
/*
			if (x >= world.gridwidth || y >= world.gridheight)
			{
				Console_Printf("Horrible error in World_CalcMaxZ:  grid size = (%d, %d), (x, y) are (%d, %d)\n", world.gridwidth, world.gridheight, x, y);
				return temp;
			}
*/
			gridref = WORLD_GRIDREF(x,y);

			if (temp<gridref)
				temp = gridref;			
		}

	return temp;	
}

float	World_CalcMinZ(float x1, float y1, float x2, float y2)
{
	int x,y;
	float temp;

	World_ClampCoords(&x1, &y1, &x2, &y2);

	if (x1 > x2)
		_SWAP(x1,x2);
	if (y1 > y2)
		_SWAP(y1,y2);

	temp = FAR_AWAY;

	for(y=WORLD_TO_GRID(y1); y<=WORLD_TO_GRID(y2); y++)
		for (x=WORLD_TO_GRID(x1); x<=WORLD_TO_GRID(x2); x++)
		{
			float gridref;
/*
			if (x >= world.gridwidth || y >= world.gridheight)
			{
				Console_Printf("Horrible error in World_CalcMinZ:  grid size = (%d, %d), (x, y) are (%d, %d)\n", world.gridwidth, world.gridheight, x, y);
				return temp;
			}
*/
			gridref = WORLD_GRIDREF(x,y);

			if (temp>gridref)
				temp = gridref;			
		}

	return temp;	
}

//a heightmap consists of two ints (width and height)
//followed by width*height floats representing the
//height data
bool	World_LoadHeightmap(archive_t *archive, const char *name, float scale)
{
	file_t *f;
	char *fname;
	int n, x, y;

	fname = fmt("%s.hm", name);
	f = Archive_OpenFile(archive, fname, "rb");

	if (!f) 
	{		
		Console_DPrintf("World_LoadHeightmap: %s not found\n", fname);
		return false;
	}

	f->read(&world.gridwidth, sizeof(int), 1, f);
	f->read(&world.gridheight, sizeof(int), 1, f);
	world.gridwidth = LittleInt(world.gridwidth);
	world.gridheight = LittleInt(world.gridheight);

	if (world.gridwidth < 0 || world.gridheight < 0 || world.gridwidth > WORLD_MAX_GRIDWIDTH || world.gridheight > WORLD_MAX_GRIDHEIGHT)
	{
		Console_DPrintf("World_LoadHeightmap: Invalid heightmap dimensions\n");
		return false;
	}

	world.grid = Tag_Malloc((world.gridwidth+1)*(world.gridheight+1)*sizeof(float), MEM_WORLD);
	world.normal[0] = Tag_Malloc((world.gridwidth+1)*(world.gridheight+1)*sizeof(vec3_t), MEM_WORLD);
	world.normal[1] = Tag_Malloc((world.gridwidth+1)*(world.gridheight+1)*sizeof(vec3_t), MEM_WORLD);
	
	if (f->read(world.grid, world.gridwidth * world.gridheight * sizeof(float), 1, f) < 1)
	{
		Console_DPrintf("World_LoadHeightmap: Invalid file contents\n");
		return false;
	}

	for (n=0; n<world.gridwidth*world.gridheight; n++)
	{
		world.grid[n] = LittleFloat(world.grid[n]);
		world.grid[n] *= scale;
	}

	File_Close(f);

	for (y=0; y<world.gridheight-1; y++)
	{
		for (x=0; x<world.gridwidth-1; x++)
		{
			WR_RecalcNormal(x, y);
		}
	}

	world.savescale = 1.0 / scale;

	return true;
}


bool	World_SaveHeightmap(void *zipfile, const char *name, float scale)
{
	char *fname;
	int n, size, ret;
	float height;
	int method = Z_DEFLATED;
	int level = Z_DEFAULT_COMPRESSION;

	size = strlen(name)+4;
	fname = Tag_Malloc(size, MEM_WORLD);

	BPrintf(fname, size, "%s.hm", name);
	fname[size-1] = 0;

	if ((ZIPW_AddFileInZip(zipfile, fname, NULL, NULL, 0, NULL, 0, NULL, method, level)) != ZIP_OK)
	{
		Console_Printf("Failed to open %s in zip\n", fname);
		return false;
	}

	Tag_Free(fname);

	if ((ret = ZIPW_WriteFileInZip(zipfile, &world.gridwidth, sizeof(int))) < 0)
		Console_Printf("Error %i writing to zip file\n", ret);
	if ((ret = ZIPW_WriteFileInZip(zipfile, &world.gridheight, sizeof(int))) < 0)
		Console_Printf("Error %i writing to zip file\n", ret);

	for (n=0; n<world.gridwidth*world.gridheight; n++)
	{
		height = LittleFloat(world.grid[n] * scale);
		if ((ret = ZIPW_WriteFileInZip(zipfile, &height, sizeof(float))) < 0)
			Console_Printf("Error %i writing to zip file\n", ret);
	}

	ret = ZIPW_CloseFileInZip(zipfile);
	if (ret!=ZIP_OK)
		Console_Printf("error in closing %s in the zipfile\n",fname);

	return true;
}

bool	_WR_LoadShadermap(archive_t *archive, const char *name, int layer)
{
	file_t *f;
	char *fname;
	int w, h, size;

	size = strlen(name)+6;
	fname = Tag_Malloc(size, MEM_WORLD);

	if (layer)
	{
		BPrintf(fname, size-1, "%s.sm2", name);
		fname[size-1] = 0;
	}
	else
	{
		BPrintf(fname, size-1, "%s.sm", name);
		fname[size-1] = 0;
	}
	f = Archive_OpenFile(archive, fname, "rb");

	Tag_Free(fname);
	
	if (!f) 
	{		
		Console_DPrintf("WR_LoadShadermap(%i): Skipping...\n", layer);
		return false;
	}

	f->read(&w, sizeof(int), 1, f);
	f->read(&h, sizeof(int), 1, f);
	w = LittleInt(w);
	h = LittleInt(h);

	if (w!=world.gridwidth || h!=world.gridheight)
	{
		Console_DPrintf("WR_LoadShadermap(%i): Width and height do not match world\n", layer);
		File_Close(f);
		//return false;
	}	

	if (layer)
	{		
		f->read(wr.shadermap[1], world.gridwidth*world.gridheight, 1, f);
	}
	else
	{
		f->read(wr.shadermap[0], world.gridwidth*world.gridheight, 1, f);
	}

	File_Close(f);

	return true;
}

bool	WR_LoadShadermap(archive_t *archive, const char *name)
{
	Tag_Free(wr.shadermap[0]);
	wr.shadermap[0] = Tag_Malloc((world.gridwidth+1)*(world.gridheight+1), MEM_WORLD);

	return _WR_LoadShadermap(archive, name, 0);
}

bool	WR_LoadShadermap2(archive_t *archive, const char *name)
{
	Tag_Free(wr.shadermap[1]);
	wr.shadermap[1] = Tag_Malloc((world.gridwidth+1)*(world.gridheight+1), MEM_WORLD);

	return _WR_LoadShadermap(archive, name, 1);	
}

bool	_WR_SaveShadermap(void *zipfile, const char *name, int layer)
{
	char *fname;
	int size, ret;
	int method = Z_DEFLATED;
	int level = Z_DEFAULT_COMPRESSION;

	size = strlen(name)+6;

	fname = Tag_Malloc(size, MEM_WORLD);

	if (layer)
	{
		BPrintf(fname, size-1, "%s.sm2", name);
		fname[size-1] = 0;
	}
	else
	{
		BPrintf(fname, size-1, "%s.sm", name);
		fname[size-1] = 0;
	}

	if ((ZIPW_AddFileInZip(zipfile, fname, NULL, NULL, 0, NULL, 0, NULL, method, level)) != ZIP_OK)
	{
		Console_DPrintf("Failed to open %s in zip\n", fname);
		return false;
	}

	Tag_Free(fname);
	
	ZIPW_WriteFileInZip(zipfile, &world.gridwidth, sizeof(int));
	ZIPW_WriteFileInZip(zipfile, &world.gridheight, sizeof(int));

	if (layer)
	{
		ZIPW_WriteFileInZip(zipfile, wr.shadermap[1], world.gridwidth*world.gridheight);
	}
	else
	{
		ZIPW_WriteFileInZip(zipfile, wr.shadermap[0], world.gridwidth*world.gridheight);
	}

	ret = ZIPW_CloseFileInZip(zipfile);
	if (ret != ZIP_OK)
		Console_DPrintf("error closing zip file\n");

	return true;
}

bool	WR_SaveShadermap(void *zipfile, const char *name)
{
	return _WR_SaveShadermap(zipfile, name, 0);
}

bool	WR_SaveShadermap2(void *zipfile, const char *name)
{
	return _WR_SaveShadermap(zipfile, name, 1);
}

bool	WR_LoadShaderRefs(archive_t *archive, const char *name)
{
	file_t *f;
	char *fname;
	int ret, size;
	char s[1024];
	char line[1024];
	int i = 0;

	memset(wr.shaders, 0, sizeof(wr.shaders));	

	size = strlen(name)+8;
	fname = Tag_Malloc(size, MEM_WORLD);

	sprintf(fname, "%s.shdref", name);

	f = Archive_OpenFile(archive, fname, "r"); 
	Tag_Free(fname);

	if (!f)
	{
		Console_DPrintf("WR_LoadShaderRefs: Skipping...\n");				
		return false;
	}
	
	while (1)
	{
		File_gets(line, 1024, f);
		ret = sscanf(line, "%i = %s", &i, s);
		if (ret==EOF || !ret) 			
		{
			File_Close(f);
			return true;		
		}
		if (i>=0 && i<=MAX_SHADERS-1)
		{
			wr.shaders[i] = Res_LoadTerrainShader(s);
		}
	}
}

void	WR_ConsolidateShaderRefs()
{
	int n, i, j;
	//make sure all our shdrefs are packed together and that there are no duplicates

	return;			//fixme

	for (n=0; n<256; n++)
	{
		if (wr.shaders[n] > 0)
		{
			for (i=0; i<256; i++)
			{
				if (wr.shaders[n] == wr.shaders[i])
				{
					for (j=0; j<world.gridwidth*world.gridheight; j++)
					{
						if (wr.shadermap[0][j] == i)
							wr.shadermap[0][j] = n;
						if (wr.shadermap[1][j] == i)
							wr.shadermap[1][j] = n;
					}
					wr.shaders[i] = 0;
				}
			}	
		}
	}
}

bool	WR_SaveShaderRefs(void *zipfile, const char *name)
{
	char *fname;
	int n;
	bool saved[MAX_SHADERS];
	char line[512];
	shader_t *shd;
	int ref;
	int method = Z_DEFLATED;
	int level = Z_DEFAULT_COMPRESSION;

	WR_ConsolidateShaderRefs();

	memset(saved, 0, sizeof(saved));

	fname = Tag_Malloc(strlen(name)+8, MEM_WORLD);

	sprintf(fname, "%s.shdref", name);

	if ((ZIPW_AddFileInZip(zipfile, fname, NULL, NULL, 0, NULL, 0, NULL, method, level)) != ZIP_OK)
	{
		Console_DPrintf("Failed to open %s in zip\n", fname);
		return false;
	}

	Tag_Free(fname);

	//search through the map to see which shaders are actually being used and save these

	for (n=0; n<world.gridwidth*world.gridheight; n++)
	{
		ref = wr.shadermap[0][n];
		if (saved[ref]) continue;

		shd = Res_GetShader(wr.shaders[ref]);

		BPrintf(line, 512, "%i = %s\n", ref, shd->name);
		ZIPW_WriteFileInZip(zipfile, line, strlen(line));

		saved[ref] = true;
	}

	for (n=0; n<world.gridwidth*world.gridheight; n++)
	{
		ref = wr.shadermap[1][n];
		if (saved[ref]) continue;

		shd = Res_GetShader(wr.shaders[ref]);

		BPrintf(line, 512, "%i = %s\n", ref, shd->name);
		ZIPW_WriteFileInZip(zipfile, line, strlen(line));

		saved[ref] = true;
	}


	ZIPW_CloseFileInZip(zipfile);
	
	return true;
}

//this pass just marks which tiles definite don't have foliage and sets the chance of the rest to 1.0
void	WR_CacheFoliageFirstPass()
{
#ifndef _S2_DONT_INCLUDE_GL
	int i, j;
	shader_t *shader;
	byte colormap;
	vec3_t nml;

	for (i = 0; i < world.gridwidth; i++)
	{
		for (j = 0; j < world.gridheight; j++)
		{
			//hopefully unnecessary
			wr.foliageChance[j * (world.gridwidth) + i] = 0;
						
			colormap = WR_COLREF(i, j)[3];
			if (colormap >= 128)
				shader = Res_GetShader(wr.shaders[WR_SHADERREF2(i, j)]);
			else
				shader = Res_GetShader(wr.shaders[WR_SHADERREF(i, j)]);
			if (!(shader->flags & SHD_NO_FOLIAGE))
			{
				WR_GetNormal(i, j, nml, 1);
				if (nml[2] > gfx_foliageMaxSlope.value)
				{
					if (i+1 < world.gridwidth
			  		    && j+1 < world.gridheight)
					{
						//yes, this does get foliage
						wr.foliageChance[j * (world.gridwidth) + i] = 1.0;
					}
				}
			}
		}
	}
#endif
}

void	WR_CacheFoliageSmoothingPass()
{
#ifndef _S2_DONT_INCLUDE_GL
	int max, i = world.gridwidth;

	max = (world.gridwidth)*(world.gridheight);
	
	while (i < max)
	{
		if (i < world.gridwidth)
		{
			i++;
			continue;
		}

		if (wr.foliageChance[i] > 0)
		{
			wr.foliageChance[i] = 1;
			if (wr.foliageChance[i - world.gridwidth] == 0)
				wr.foliageChance[i] -= wr.foliageChance[i]/gfx_foliageSmooth.value;
			if (wr.foliageChance[i + world.gridwidth] == 0)
				wr.foliageChance[i] -= wr.foliageChance[i]/gfx_foliageSmooth.value;
			if (wr.foliageChance[i - 1] == 0)
				wr.foliageChance[i] -= wr.foliageChance[i]/gfx_foliageSmooth.value;
			if (wr.foliageChance[i + 1] == 0)
				wr.foliageChance[i] -= wr.foliageChance[i]/gfx_foliageSmooth.value;
		}
	
		
		i++;
	}
#endif
}

void	WR_CacheFoliage()
{
	if (wr.foliageChance)
		Tag_Free(wr.foliageChance);

	wr.foliageChance = Tag_Malloc((world.gridwidth+1)*(world.gridheight+1)*sizeof(float), MEM_WORLD);	
	
	WR_CacheFoliageFirstPass();
	WR_CacheFoliageSmoothingPass();
}

void	WR_CacheFoliage_Cmd(int argc, char *argv[])
{
	WR_CacheFoliage();
}


void	World_GetBounds(vec3_t bmin, vec3_t bmax)
{
	if (world.tree[0])
	{
		M_CopyVec3(world.tree[0]->bmin, bmin);
		M_CopyVec3(world.tree[0]->bmax, bmax);
	}
	else
	{
		M_SetVec3(bmin, 0, 0, 0);
		M_SetVec3(bmax, 0, 0, 0);
	}
}

void		World_SampleGround(float xpos, float ypos, pointinfo_t *result)
{
	/*
	the premise of this function is to take one grid square, then do a test to see which
	component triangle of the grid square position is in (with the x<=y test).  then we
	convert the triangle to a plane and use the GetPlaneY() function to derive the Y coord
	from the X and Z values in position
	*/
	int ix, iy;
	vec3_t p;
	float *normal;
	float dist;
	float gridx, gridy;

	gridx = WORLD_TO_GRID(xpos);
	gridy = WORLD_TO_GRID(ypos);

	if (gridx<0 || gridx+1>=world.gridwidth ||
		gridy<0 || gridy+1>=world.gridheight)
	{
		result->z = 0;
		result->nml[0] = 0;
		result->nml[1] = 0;
		result->nml[2] = 1;
		return;
	}

	ix = (int)gridx;
	iy = (int)gridy;

	if (gridx+gridy<ix+iy+1)			//point lies on left triangle
	{
		normal = world.normal[0][GRIDREF(ix, iy)];
	}
	else								//point lies on right triangle
	{
		normal = world.normal[1][GRIDREF(ix, iy)];		
	}

	//we can do the Z calculation in grid space

	p[0] = GRID_TO_WORLD(ix);
	p[1] = GRID_TO_WORLD(iy+1);
	p[2] = WORLD_GRIDREF(ix, iy+1);

	dist = -M_DotProduct(normal, p);

	result->nml[0] = normal[0];
	result->nml[1] = normal[1];
	result->nml[2] = normal[2];	

	// Ax + By + Cz + D = 0 -->  Cz = -Ax - By - D

	result->z = ((-normal[0]*xpos - normal[1]*ypos - dist) / normal[2]);
}


float		World_GetTerrainHeight(float xpos, float ypos)
{
	/*
	the premise of this function is to take one grid square, then do a test to see which
	component triangle of the grid square position is in (with the x<=y test).  then we
	convert the triangle to a plane and use the GetPlaneY() function to derive the Y coord
	from the X and Z values in position
	*/
	int ix, iy;
	vec3_t p;
	float *normal;
	float dist;
	float gridx, gridy;

	gridx = WORLD_TO_GRID(xpos);
	gridy = WORLD_TO_GRID(ypos);

	if (gridx<0 || gridx+1>=world.gridwidth ||
		gridy<0 || gridy+1>=world.gridheight)
	{
		return FAR_AWAY;
	}

	ix = (int)gridx;
	iy = (int)gridy;
	//FloatToInt(&ix, gridx);
	//FloatToInt(&iy, gridy);

	if (gridx+gridy<ix+iy+1)	//point lies on left triangle
	{
		normal = world.normal[0][GRIDREF(ix, iy)];
	}
	else								//point lies on right triangle
	{
		normal = world.normal[1][GRIDREF(ix, iy)];		
	}

	//we can do the Z calculation in grid space

	p[0] = GRID_TO_WORLD(ix);
	p[1] = GRID_TO_WORLD(iy+1);
	p[2] = WORLD_GRIDREF(ix, iy+1);

	dist = -M_DotProduct(normal, p);

	// Ax + By + Cz + D = 0 -->  Cz = -Ax - By - D

	return ((-normal[0]*xpos - normal[1]*ypos - dist) / normal[2]);
}

void	World_DeformGround(int gridx, int gridy, heightmap_t altitude)
{
	int xstart, ystart;	

	if (gridx > world.gridwidth-1 || gridy > world.gridheight-1 || gridx<0 || gridy<0)
		return;	

	world.grid[GRIDREF(gridx,gridy)] = altitude;

	//recompute altitudes
	
	if (gridx==0) 
		xstart = 0;
	else
		xstart = gridx-1;

	if (gridy==0)
		ystart = 0;
	else
		ystart = gridy-1;

/*	for (y=ystart; y<=gridy; y++)
	{
		for (x=xstart; x<=gridx; x++)
		{
			if (altitude > world.maxalt[GRIDREF(x,y)])
				world.maxalt[GRIDREF(x,y)] = altitude;
			if (altitude < world.minalt[GRIDREF(x,y)])
				world.minalt[GRIDREF(x,y)] = altitude;
		}
	}
*/	
	WT_UpdateTerrainHeight(gridx, gridy, altitude);
		
	WR_RecalcNormal(gridx-1, gridy-1);
	WR_RecalcNormal(gridx, gridy-1);
	WR_RecalcNormal(gridx+1, gridy-1);
	WR_RecalcNormal(gridx-1, gridy);
	WR_RecalcNormal(gridx, gridy);
	WR_RecalcNormal(gridx+1, gridy);
	WR_RecalcNormal(gridx-1, gridy+1);
	WR_RecalcNormal(gridx, gridy+1);
	WR_RecalcNormal(gridx+1, gridy+1);

	world.max_z = world.tree[0]->bmax[2];
	world.min_z = world.tree[0]->bmin[2];

	Vid_Notify(VID_NOTIFY_TERRAIN_VERTEX_MODIFIED, gridx, gridy, 0);
}

heightmap_t	World_GetGridHeight(int gridx, int gridy)
{
	if (gridx > world.gridwidth-1 || gridy > world.gridheight-1 || gridx<0 || gridy<0)
		return 0;

	return world.grid[GRIDREF(gridx,gridy)];
}

bool	WR_LoadColormap(archive_t *archive, const char *name)
{
	file_t *f;
	char *fname;
	int x, y, w, h;
	
	Tag_Free(wr.colormap);
	Tag_Free(wr.dynamap);
	wr.colormap = Tag_Malloc((world.gridwidth+1) * (world.gridheight+1) * sizeof(colormap_t), MEM_WORLD);
	wr.dynamap = Tag_Malloc((world.gridwidth+1) * (world.gridheight+1) * sizeof(colormap_t), MEM_WORLD);

	fname = Tag_Malloc(strlen(name)+4, MEM_WORLD);

	sprintf(fname, "%s.cm", name);
	f = Archive_OpenFile(archive, fname, "rb");
	
	if (!f) 
	{		
		Console_DPrintf("WR_LoadColormap: Skipping...\n");
		return false;
	}

	Tag_Free(fname);

	f->read(&w, sizeof(int), 1, f);
	f->read(&h, sizeof(int), 1, f);
	w = LittleInt(w);
	h = LittleInt(h);

	if (w!=world.gridwidth || h!=world.gridheight)
	{
		Console_DPrintf("WR_LoadColormap: Width and height do not match world\n");
		File_Close(f);
	}	

	for (y=0; y<world.gridheight; y++)
		for (x=0; x<world.gridwidth; x++)
		{
			f->read(wr.colormap[GRIDREF(x,y)], 3, 1, f);
			wr.colormap[GRIDREF(x,y)][3] = 0xff; //don't blend by default
		}	

	File_Close(f);

	wr.ignoreColormap = false;
	
	return true;
}

bool	WR_SaveColormap(void *zipfile, const char *name)
{
	char *fname;
	int x, y;
	int method = Z_DEFLATED;
	int level = Z_DEFAULT_COMPRESSION;

	fname = Tag_Malloc(strlen(name)+4, MEM_WORLD);

	sprintf(fname, "%s.cm", name);
	if ((ZIPW_AddFileInZip(zipfile, fname, NULL, NULL, 0, NULL, 0, NULL, method, level)) != ZIP_OK)
	{
		Console_DPrintf("Failed to open %s in zip\n", fname);
		return false;
	}

	Tag_Free(fname);
	
	ZIPW_WriteFileInZip(zipfile, &world.gridwidth, sizeof(int));
	ZIPW_WriteFileInZip(zipfile, &world.gridheight, sizeof(int));

	for (y=0; y<world.gridheight; y++)
		for (x=0; x<world.gridwidth; x++)
		{
			ZIPW_WriteFileInZip(zipfile, wr.colormap[GRIDREF(x,y)], 3);
		}	

	ZIPW_CloseFileInZip(zipfile);

	return true;
}

//loads an alpha map if present
bool	WR_LoadAlphamap(archive_t *archive, const char *name)
{
	file_t *f;
	char *fname;
	int x, y, w, h;

	fname = Tag_Malloc(strlen(name)+4, MEM_WORLD);

	sprintf(fname, "%s.am", name);
	f = Archive_OpenFile(archive, fname, "rb");
	
	Tag_Free(fname);

	if (!f) 
	{		
		Console_DPrintf("WR_LoadAlphamap: Skipping...\n");
		return false;
	}

	f->read(&w, sizeof(int), 1, f);
	f->read(&h, sizeof(int), 1, f);
	w = LittleInt(w);
	h = LittleInt(h);

	if (w!=world.gridwidth || h!=world.gridheight)
	{
		Console_DPrintf("WR_LoadAlphamap: Width and height do not match world\n");
		File_Close(f);
	}	

	for (y=0; y<world.gridheight; y++)
		for (x=0; x<world.gridwidth; x++)
		{
			f->read(&wr.colormap[GRIDREF(x,y)][3], 1, 1, f);						
		}	

	File_Close(f);

	return true;
}

bool	WR_SaveAlphamap(void *zipfile, const char *name)
{
	char *fname;
	int x, y;
	int method = Z_DEFLATED;
	int level = Z_DEFAULT_COMPRESSION;

	fname = Tag_Malloc(strlen(name)+4, MEM_WORLD);

	sprintf(fname, "%s.am", name);
	if ((ZIPW_AddFileInZip(zipfile, fname, NULL, NULL, 0, NULL, 0, NULL, method, level)) != ZIP_OK)
	{
		Console_DPrintf("Failed to open %s in zip\n", fname);
		return false;
	}

	Tag_Free(fname);
	
	ZIPW_WriteFileInZip(zipfile, &world.gridwidth, sizeof(int));
	ZIPW_WriteFileInZip(zipfile, &world.gridheight, sizeof(int));

	for (y=0; y<world.gridheight; y++)
		for (x=0; x<world.gridwidth; x++)
		{
			ZIPW_WriteFileInZip(zipfile, &wr.colormap[GRIDREF(x,y)][3], 1);
		}	

	ZIPW_CloseFileInZip(zipfile);

	return true;
}

bool	World_LoadClientSide(archive_t *archive, const char *realname)
{
	Cvar_AllowCheats();
	Cmd_ReadConfigFileFromArchive(archive, fmt("%s.cfg", realname), true);
	Cvar_BlockCheats();

	if (!WR_LoadShadermap(archive, realname))
		WR_GenShadermap();		
	if (!WR_LoadShadermap2(archive, realname))
		WR_GenShadermap2();	
	if (!WR_LoadColormap(archive, realname))	
		WR_GenColormap();			
	WR_LoadAlphamap(archive, realname);
	if (!WR_LoadShaderRefs(archive, realname))
		WR_GenShaderRefs();
	WR_CacheFoliage();

	memset(wr.dynamap, 255, world.gridwidth * world.gridheight * sizeof(bvec4_t));

	Vid_Notify(VID_NOTIFY_NEW_WORLD, 0, 0, 0);

	world.cl_loaded = true;

	return true;
}

float	WR_FarClip()
{
	return world.farclip;
}


bool	World_CheckHashes(int s2z_hashlen, unsigned char s2z_hash[MAX_HASH_SIZE])
{
	if (world.s2z_hashlen != s2z_hashlen)
	{
		return false;
	}

	if (memcmp(s2z_hash, world.s2z_hash, world.s2z_hashlen) != 0)
	{
		return false;
	}
	return true;
}

#ifdef SAVAGE_DEMO
bool	World_CheckHashForDemo(int length, unsigned char hash[MAX_HASH_SIZE])
{
	unsigned char base[20];
	bool	valid;
	int		i;

	if (length != 20)
		return false;

	//eden2
	//12 4a 86 c2 63 da e5 cf cd b3 37 f0 09 25 16 a2 67 dd 84 6c
	base[10] = 0x37 - 0x09;
	base[11] = 0xf0 - 0x09;
	base[12] = 0x09 - 0x09;
	base[13] = 0x25 - 0x09;
	base[14] = 0x16 - 0x09;

	base[0] = 0x12 - 0x09;
	base[1] = 0x4a - 0x09;
	base[2] = 0x86 - 0x09;
	base[3] = 0xc2 - 0x09;
	base[4] = 0x63 - 0x09;

	base[15] = 0xa2 - 0x09;
	base[16] = 0x67 - 0x09;
	base[17] = 0xdd - 0x09;
	base[18] = 0x84 - 0x09;
	base[19] = 0x6c - 0x09;

	base[5] = 0xda - 0x09;
	base[6] = 0xe5 - 0x09;
	base[7] = 0xcf - 0x09;
	base[8] = 0xcd - 0x09;
	base[9] = 0xb3 - 0x09;

	for (i = 0; i < 20; i++)
	{
		base[i] += 0x09;
	}
	
	valid = true;
	for (i = 0; i < length; i++)
	{
		if (base[i] != hash[i])
			valid = false;
	}
	if (valid)
		return true;
	
	//crossroads
	//c0 8d 6b 54 af 8f a2 2e cc c2 41 c5 a6 a3 f1 d0 0a da 81 41
	base[15] = 0xd0 - 0x0a;
	base[16] = 0x0a - 0x0a;
	base[17] = 0xda - 0x0a;
	base[18] = 0x81 - 0x0a;
	base[19] = 0x41 - 0x0a;

	base[5] = 0x8f - 0x0a;
	base[6] = 0xa2 - 0x0a;
	base[7] = 0x2e - 0x0a;
	base[8] = 0xcc - 0x0a;
	base[9] = 0xc2 - 0x0a;

	base[0] = 0xc0 - 0x0a;
	base[1] = 0x8d - 0x0a;
	base[2] = 0x6b - 0x0a;
	base[3] = 0x54 - 0x0a;
	base[4] = 0xaf - 0x0a;

	base[10] = 0x41 - 0x0a;
	base[11] = 0xc5 - 0x0a;
	base[12] = 0xa6 - 0x0a;
	base[13] = 0xa3 - 0x0a;
	base[14] = 0xf1 - 0x0a;

	for (i = 0; i < 20; i++)
	{
		base[i] += 0x0a;
	}
	
	valid = true;
	for (i = 0; i < length; i++)
	{
		if (base[i] != hash[i])
			valid = false;
	}
	if (valid)
		return true;
	
	return false;
}
#endif //SAVAGE_DEMO

bool	World_Exists(const char *name)
{
	return File_Exists(fmt("world/%s.s2z", name));
}

bool	World_ImportTOD(const char *name)
{
	archive_t *archive;
	char archivename[1024];
	
	if (DLLTYPE != DLLTYPE_EDITOR)
	{
		return false;
	}
	
	BPrintf(archivename, 1024, "world/%s.s2z", name);
	if (!Archive_RegisterArchive(archivename, false))
	{
		BPrintf(archivename, 1024, "%s/world/%s.s2z", DLLTYPE == DLLTYPE_EDITOR ? world_save_path.string : "", name);
		if (!Archive_RegisterArchive(archivename, false))
		{
			Console_DPrintf("Couldn't find world/%s.s2z\n", name);
			return false;
		}
		BPrintf(archivename, 1024, "world/%s.s2z", name);
	}

	archive = Archive_GetArchive(archivename);

	if (!archive)
	{
		Console_DPrintf("Couldn't find world/%s.s2z (I swear it was just here a second ago...)\n", name);
		return false;
	}

	TOD_Load(archive, fmt("world/%s", name));
	Archive_Close(archive);

	return true;
}

void 	World_ImportTOD_Cmd(int argc, char *argv[])
{
	if (!argc)
	{
		Console_Printf("You must specify a map name when importing a time of day config\n");
		return;
	}
	World_ImportTOD(argv[0]);
}

/*==========================

  World_Load

  Load the world into memory.

  loadClientSide should be set if we want to load client specific world data  

 ==========================*/

bool	World_Load(const char *name, bool loadClientSide)
{
	char realname[1024];
	char archivename[1024];
	char *tmp_fname;
	archive_t *archive;

	if (DLLTYPE == DLLTYPE_GAME)
		Cvar_ResetVars(CVAR_WORLDCONFIG);

	if (!localServer.active)//if localServer.active is true, this implies we have already loaded all shared world data
	{
		World_Destroy();
	}

	BPrintf(archivename, 1024, "world/%s.s2z", name);
	if (!Archive_RegisterArchive(archivename, false))
	{
		if (DLLTYPE == DLLTYPE_EDITOR)
		{
			BPrintf(archivename, 1024, "%s/world/%s.s2z", DLLTYPE == DLLTYPE_EDITOR ? world_save_path.string : "", name);
			if (!Archive_RegisterArchive(archivename, false))
			{
				Console_DPrintf("Couldn't find world/%s.s2z\n", name);
				return false;
			}
			BPrintf(archivename, 1024, "world/%s.s2z", name);
		}
		else
		{
			Console_DPrintf("Couldn't find world/%s.s2z\n", name);
			return false;
		}
	}
	archive = Archive_GetArchive(archivename);

	if (!archive)
	{
		Console_DPrintf("Couldn't find world/%s.s2z (I swear it was just here a second ago...)\n", name);
		return false;
	}

	//currently this is only used in World_Destroy
	world.archive = archive;

	BPrintf(realname, 1023, "world/%s", name);

	if (File_Exists(fmt("%s_overhead.tga", realname)))
		Cvar_SetVar(&world_overhead, fmt("%s_overhead.tga", realname));
	else if (File_Exists(fmt("%s_overhead.jpg", realname)))
		Cvar_SetVar(&world_overhead, fmt("%s_overhead.jpg", realname));
	else if (File_Exists(fmt("%s_overhead.png", realname)))
		Cvar_SetVar(&world_overhead, fmt("%s_overhead.png", realname));

	Cvar_SetVar(&world_name, name);
	BPrintf(realname, 1023, "world/%s/%s", name, name);
	
	if (!localServer.active)		//if localServer.active is true, this implies we have already loaded all shared world data
	{
		//load world data needed by both client and server

		Cvar_AllowCheats();
		Cmd_ReadConfigFileFromArchive(archive, fmt("%s.cfg", realname), true);
		Cvar_BlockCheats();

		world.farclip = Cvar_GetValue("gfx_farclip");

		if (!World_LoadHeightmap(archive, realname, 1000 * world_heightscale.value))
		{
			Console_DPrintf("World \"%s\" invalid or not found\n", name);
			return false;
		}		

		//initialize the world collision tree
		WT_BuildTree(wr_treedepth.value);

		//set world name
		strncpySafe(world.name, name, WORLD_NAME_LENGTH);

		//load time of day configs
		Cvar_AllowCheats();
		TOD_Load(archive, fmt("world/%s", world.name));
		Cvar_BlockCheats();

		//load objpos file
		tmp_fname = fmt("%s.objpos", realname);
		Cmd_ReadConfigFileFromArchive(archive, tmp_fname, false);		//make sure not to change to the directory the config is in, or we will have problems loading model files!
	
		world.loaded = true;

		Console_DPrintf("Loaded world \"%s\", dimensions: %ix%i\n", realname, world.gridwidth, world.gridheight);
	}
	world.s2z_hashlen = Hash_Filename(archivename, world.s2z_hash);

#ifdef SAVAGE_DEMO
	/*if (!World_CheckHashForDemo(world.s2z_hashlen, world.s2z_hash))
	{
		World_Destroy();
		Console_Errorf("Not a valid demo world\n");
		return false;
	}*/
#endif

	if (loadClientSide)
	{
		World_LoadClientSide(archive, realname);
	}

	//NOTE: any server-side specific initialization should be done in Server_Start()

	return true;
}

bool	World_Load_Server(char *name)
{
	return World_Load(name, false);
}

bool	World_AddFileToZip(void *zipfile, file_t *f, char *filename)
{
	char buf[1024];
	int size, ret;
	int method = 0; //Z_DEFLATED;
	int level = Z_DEFAULT_COMPRESSION;

	if ((ZIPW_AddFileInZip(zipfile, filename, NULL, NULL, 0, NULL, 0, NULL, method, level)) != ZIP_OK)
	{
		Console_DPrintf("failed to add %s to zip file\n", filename);
		return false;
	}
	while ((size = f->read(buf, 1, 1024, f)) > 0)
	{
		if ((ret = ZIPW_WriteFileInZip(zipfile, buf, size)) < 0)
			Console_Printf("Error %i writing to zip file\n", ret);
	}
	File_Close(f);
	ret = ZIPW_CloseFileInZip(zipfile);
	if (ret!=ZIP_OK)
		Console_Printf("error in closing %s in the zipfile\n",filename);
	return true;
}

bool	World_Save(const char *name)
{
	char *fname, *wcard;
	char pathname[256];
	void *zipfile;

	fname = File_GetFullPath(fmt("%s/world/%s.s2z", world_save_path.string, name));
	BPrintf(pathname, 256, "world/%s/%s", name, name);

	File_Delete(fname);
	zipfile = ZIPW_Open(fname, false);
	if (!zipfile)
	{
		Console_Printf("failed to open %s\n", fname);
		return false;
	}

	if (!World_SaveHeightmap(zipfile, pathname, world.savescale))
	{
		Console_Printf("Error saving heightmap\n");
		return false;
	}
	if (!WR_SaveShaderRefs(zipfile, pathname))
	{
		Console_Printf("Error saving shaderefs\n");
		return false;
	}
	if (!WR_SaveColormap(zipfile, pathname))
	{
		Console_Printf("Error saving colormap\n");
		return false;
	}
	if (!WR_SaveAlphamap(zipfile, pathname))
	{
		Console_Printf("Error saving alphamap\n");
		return false;
	} 
	if (!WR_SaveShadermap(zipfile, pathname))
	{
		Console_Printf("Error saving shadermap layer 1\n");
		return false;
	}
	if (!WR_SaveShadermap2(zipfile, pathname))
	{
		Console_Printf("Error saving shadermap layer 2\n");
		return false;
	}

	WO_WriteObjposFile(zipfile, fmt("%s.objpos", pathname));

	wcard = "";
	Cvar_SaveConfigFileToZip(zipfile, fmt("%s.cfg", pathname), 1, &wcard, CVAR_WORLDCONFIG, false);
	TOD_SaveToZip(zipfile, (char *)name);
	
	ZIPW_Close(zipfile, fmt("Savage Map \"%s\"", name));

	return true;
}


void	World_Save_Cmd(int argc, char *argv[])
{
	char fname[1024];

	if (DLLTYPE != DLLTYPE_EDITOR)
	{
		Console_Printf("Command only available in editor\n");
		return;
	}
	
	if (!argc)
	{
		Console_Printf("syntax: writeworld <worldname> (don't include any file extension)\n");
		return;
	}

	BPrintf(fname, 1023, "%s", argv[0]);

	//System_CreateDir(dirname);

	World_Save(fname);
}


void	World_Destroy()
{
	if (world.archive)
		Archive_Close(world.archive);
		
	Tag_FreeAll(MEM_WORLD);

	WO_ClearObjects();
	WT_DestroyTree();
	
	memset(&world, 0, sizeof(world_t));
	memset(&wr, 0, sizeof(worldrender_t));
	
	Console_DPrintf("----------------------------------------------\n");
	Console_DPrintf("Destroyed world\n");
	Console_DPrintf("----------------------------------------------\n");
}

//worldrender

void	WR_GenShaderRefs()
{
	int n;

	wr.shaders[0] = Res_LoadShader("textures/core/white.png");	
	for (n=1; n<MAX_SHADERS; n++)
		wr.shaders[n] = wr.shaders[0];
}

//generates a default colormap with some simple lighting code

//#define DEFAULT_COLORMAP_SURFACE_NORMAL   //uncomment to generate colormap based on slope

void	WR_GenColormap()
{
#ifdef DEFAULT_COLORMAP_SURFACE_NORMAL
	vec3_t nml;
#endif
	int x, y;

	for (y=0; y<world.gridheight; y++)
		for (x=0; x<world.gridwidth; x++)
		{
			wr.colormap[GRIDREF(x,y)][0] = 0xff;
			wr.colormap[GRIDREF(x,y)][1] = 0xff;
			wr.colormap[GRIDREF(x,y)][2] = 0xff;
			wr.colormap[GRIDREF(x,y)][3] = 0x00;
		}
/*	
	for (y=0; y<world.gridheight; y++)
		for (x=0; x<world.gridwidth; x++)
		{
			wr.colormap[GRIDREF(x,y)][0] = 0x00;
			wr.colormap[GRIDREF(x,y)][1] = 0x00;
			wr.colormap[GRIDREF(x,y)][2] = 0x00;
			wr.colormap[GRIDREF(x,y)][3] = 0x00;
		}
			
#ifdef DEFAULT_COLORMAP_SURFACE_NORMAL
			WR_GetNormal(x, y, nml);
			light = nml[2] - 0.4;
			if (light<0) light=0;
			wr.colormap[GRIDREF(x,y)][0] = 0xff*light;
			wr.colormap[GRIDREF(x,y)][1] = 0xff*light;
			wr.colormap[GRIDREF(x,y)][2] = 0xff*light;
			wr.colormap[GRIDREF(x,y)][3] = ((WORLD_GRIDREF(x,y) / world.max_z) * 0.5) * 256 + 128;
#else
			light = 0xff * (WORLD_GRIDREF(x,y) / (world.max_z));
			if (light<0) light=0;
			wr.colormap[GRIDREF(x,y)][0] = light;
			wr.colormap[GRIDREF(x,y)][1] = light;
			wr.colormap[GRIDREF(x,y)][2] = light;
			wr.colormap[GRIDREF(x,y)][3] = 0x00; //don't blend by default
#endif
		}*/			
}

void	WR_GenShadermap()
{
	//generates a default shader map
//#if 0
	int x, y, v, lastval;

	v=0;
	lastval = 0;

	for (y=0; y<world.gridheight; y++)
	{
		for (x=0; x<world.gridwidth; x++)							
		{			
			/*wr.shadermap[GRIDREF(x,y)] = rand() % 3;
			if (wr.shadermap[GRIDREF(x,y)] == lastval) 
			{
				wr.shadermap[GRIDREF(x,y)] = (lastval+1) % 3;
			}			
			if (y>0)
			{
				if (wr.shadermap[GRIDREF(x,y-1)] == wr.shadermap[GRIDREF(x,y)])
				{
					wr.shadermap[GRIDREF(x,y)] = (lastval+2) % 3;
				}
			}
			lastval = wr.shadermap[GRIDREF(x,y)];
			*/
			wr.shadermap[0][GRIDREF(x,y)] = lastval % 3;
			lastval++;
		}
		lastval++;
	}
//#endif
	//memset(wr.shadermap[0], 0, world.gridwidth * world.gridheight * sizeof(shadermap_t));
}

void	WR_GenShadermap2()
{
	//generates a default second layer shader map	
/*
	for (y=0; y<world.gridheight; y++)
		for (x=0; x<world.gridwidth; x++)							
		{			
			wr.shadermap[1][GRIDREF(x,y)] = lastval = rand() % 3;
			if (wr.shadermap[1][GRIDREF(x,y)] == lastval) 
			{
				wr.shadermap[1][GRIDREF(x,y)] = lastval = (lastval+1) % 3;				
			}		
			wr.shadermap[1][GRIDREF(x,y)] = 1;
		}
*/
	memset(wr.shadermap[1], 0, world.gridwidth * world.gridheight * sizeof(shadermap_t));
}


void	WR_GetNormal(int x, int y, vec3_t nml, int tri)
{
	int idx = GRIDREF(x,y);

	if (x>world.gridwidth-1 || y>world.gridheight-1 || x<0 || y<0) return;

	nml[0] = world.normal[tri][idx][0];
	nml[1] = world.normal[tri][idx][1];
	nml[2] = world.normal[tri][idx][2];
}


void	WR_RecalcNormal(int x, int y)
{
	vec3_t a, b, c;

	if (x>=world.gridwidth-1 || y>=world.gridheight-1 || x<0 || y<0)
		return;

	a[0] = x;
	a[1] = y;
	a[2] = WORLD_TO_GRID(WORLD_GRIDREF(x, y));
	b[0] = x;
	b[1] = y+1;
	b[2] = WORLD_TO_GRID(WORLD_GRIDREF(x, y+1));
	c[0] = x+1;
	c[1] = y;
	c[2] = WORLD_TO_GRID(WORLD_GRIDREF(x+1, y));

	M_SurfaceNormal(c, b, a, world.normal[0][GRIDREF(x,y)]);

	a[0] = x+1;
	a[1] = y;
	a[2] = WORLD_TO_GRID(WORLD_GRIDREF(x+1, y));
	b[0] = x;
	b[1] = y+1;
	b[2] = WORLD_TO_GRID(WORLD_GRIDREF(x, y+1));
	c[0] = x+1;
	c[1] = y+1;
	c[2] = WORLD_TO_GRID(WORLD_GRIDREF(x+1, y+1));

	M_SurfaceNormal(c, b, a, world.normal[1][GRIDREF(x,y)]);

	Vid_Notify(VID_NOTIFY_TERRAIN_NORMAL_MODIFIED, x, y, 0);
}


#if 0

void	World_ComputeAltitudes()
{
	int x, y;
	heightmap_t temp;
	heightmap_t result;

	//this is used for DDA raytracing
	Tag_Free(world.minalt);
	Tag_Free(world.maxalt);
	world.minalt = Mem_Alloc((world.gridwidth+1) * (world.gridheight+1) * sizeof(heightmap_t));
	world.maxalt = Mem_Alloc((world.gridwidth+1) * (world.gridheight+1) * sizeof(heightmap_t));

	for (y=0; y<world.gridheight - 1; y++)
		for (x=0; x<world.gridwidth - 1; x++)
		{
			result = WORLD_GRIDREF(x, y);
			temp = WORLD_GRIDREF(x+1, y);
			if (temp>result) 
				result = temp;
			temp = WORLD_GRIDREF(x+1, y+1);
			if (temp>result)
				result = temp;
			temp = WORLD_GRIDREF(x, y+1);
			if (temp>result)
				result = temp;

			world.maxalt[GRIDREF(x,y)] = result;
		}
	for (y=0; y<world.gridheight - 1; y++)
		for (x=0; x<world.gridwidth - 1; x++)
		{
			result = WORLD_GRIDREF(x, y);
			temp = WORLD_GRIDREF(x+1, y);
			if (temp<result) 
				result = temp;
			temp = WORLD_GRIDREF(x+1, y+1);
			if (temp<result)
				result = temp;
			temp = WORLD_GRIDREF(x, y+1);
			if (temp<result)
				result = temp;

			world.minalt[GRIDREF(x,y)] = result;
		}
}

#endif



/*==========================

  World_GetOccluder

 ==========================*/

void		World_GetOccluder(int num, occluder_t *out)
{
	if (num < world.numOccluders && num >= 0)
		*out = world.occluders[num];
}


/*==========================

  World_GetNumOccluders

 ==========================*/

int			World_GetNumOccluders()
{
	return world.numOccluders;
}

bool		World_UpdateOccluder(int num, const occluder_t *occ)
{
	if (num < world.numOccluders && num >= 0)
	{
		if (occ->numpoints <= MAX_OCCLUDER_POINTS && occ->numpoints >= 3)
		{
			world.occluders[num] = *occ;
			return true;
		}
	}

	return false;
}


/*==========================

  World_ClearOccluders

 ==========================*/

void		World_ClearOccluders()
{
	world.numOccluders = 0;
}


/*==========================

  World_AddOccluder

 ==========================*/

int			World_AddOccluder(occluder_t *occluder)
{
	if (world.numOccluders >= MAX_WORLD_OCCLUDERS)
		return -1;

	world.occluders[world.numOccluders] = *occluder;

	world.numOccluders++;

	return world.numOccluders-1;
}


/*==========================

  World_RemoveOccluder

 ==========================*/

void		World_RemoveOccluder(int num)
{
	if (num < 0 || num >= world.numOccluders)
		return;

	if (world.numOccluders < MAX_WORLD_OCCLUDERS)
		memmove(&world.occluders[num], &world.occluders[num+1], sizeof(occluder_t) * (world.numOccluders - num));

	world.numOccluders--;
}


residx_t	WR_GetShaderAt(int layer, int gridx, int gridy)
{
	shadermap_t shaderref;
	
	if (layer > 1)
		return hostmedia.whiteShader;

	shaderref = wr.shadermap[layer][GRIDREF(gridx,gridy)];
	return wr.shaders[shaderref];
}




bool dynamapChanged = true;

void		WR_ClearDynamap()
{
	if (!dynamapChanged)
		return;

	memset(wr.dynamap, 255, world.gridwidth*world.gridheight*sizeof(bvec4_t));

	Vid_Notify(VID_NOTIFY_TERRAIN_COLOR_MODIFIED, 0, 0, 1);
	dynamapChanged = false;
}

void		WR_DynamapToBitmap(bitmap_t *bmp)
{
	Bitmap_Alloc(bmp, world.gridwidth, world.gridheight, BITMAP_RGBA);
	Mem_Copy(bmp->data, wr.dynamap, sizeof (bvec4_t) * (world.gridwidth) * (world.gridheight));
	Bitmap_Flip(bmp);
}

void		WR_ClearDynamapToColor(bvec4_t color)
{
	int n;

	/*
	if (!dynamapChanged)
		return;
	*/
		
	for (n=0; n<world.gridwidth * world.gridheight; n++)
	{		
		wr.dynamap[n][0] = color[0];
		wr.dynamap[n][1] = color[1];
		wr.dynamap[n][2] = color[2];
		wr.dynamap[n][3] = color[3];
	}	

	Vid_Notify(VID_NOTIFY_TERRAIN_COLOR_MODIFIED, 0, 0, 1);
	dynamapChanged = false;
}

void		WR_ClearDynamapToColorEx(bvec4_t color, bvec4_t conditional)
{
	int n;

	/*
	if (!dynamapChanged)
		return;
	*/
		
	for (n=0; n<world.gridwidth * world.gridheight; n++)
	{
		byte *cur = wr.dynamap[n];
		if (cur[0] > conditional[0]
			|| cur[1] > conditional[1]
			|| cur[2] > conditional[2])
		{
			//WR_SetDynamap(x, y, color);
			wr.dynamap[n][0] = color[0];
			wr.dynamap[n][1] = color[1];
			wr.dynamap[n][2] = color[2];
			wr.dynamap[n][3] = color[3];
		}
	}

	Vid_Notify(VID_NOTIFY_TERRAIN_COLOR_MODIFIED, 0, 0, 1);
	dynamapChanged = false;
}

void		WR_SetDynamap(int gridx, int gridy, bvec4_t color)
{
	int idx = GRIDREF(gridx,gridy);

	if (gridx > world.gridwidth-1 || gridy > world.gridheight-1 || gridx<0 || gridy<0)
		return;

	wr.dynamap[idx][0] = color[0];
	wr.dynamap[idx][1] = color[1];
	wr.dynamap[idx][2] = color[2];
	wr.dynamap[idx][3] = color[3];	

	Vid_Notify(VID_NOTIFY_TERRAIN_COLOR_MODIFIED, gridx, gridy, 0);
	dynamapChanged = true;
}

void		WR_GetDynamap(int gridx, int gridy, bvec4_t color)
{
	int idx = GRIDREF(gridx,gridy);
	if (gridx > world.gridwidth-1 || gridy > world.gridheight-1 || gridx<0 || gridy<0)
		return;

	color[0] = wr.dynamap[idx][0];
	color[1] = wr.dynamap[idx][1];
	color[2] = wr.dynamap[idx][2];
	color[3] = wr.dynamap[idx][3];	
}

void		WR_SetColormap(int gridx, int gridy, bvec4_t color)
{
	if (gridx > world.gridwidth-1 || gridy > world.gridheight-1 || gridx<0 || gridy<0)
		return;

	Mem_Copy(wr.colormap[GRIDREF(gridx,gridy)], color, sizeof(bvec4_t));

	Vid_Notify(VID_NOTIFY_TERRAIN_COLOR_MODIFIED, gridx, gridy, 0);
}

void		WR_GetColormap(int gridx, int gridy, bvec4_t color)
{
	if (gridx > world.gridwidth-1 || gridy > world.gridheight-1 || gridx<0 || gridy<0)
		return;

	Mem_Copy(color, wr.colormap[GRIDREF(gridx,gridy)], sizeof(bvec4_t));	
}

//this is only really needed for the level editor so it's not a big hit on performance
//a possible optimization however would be to store a shade reference in the shader
//itself and check against this instead of looking through all 256 references
int WR_AllocateShaderReference(residx_t shader)
{
	int n;
	int marker = -1;

	for (n=0; n<256; n++)
	{
		if (wr.shaders[n] == shader)
			return n;
		if (!wr.shaders[n] && marker == -1)
			marker = n;
	}

	if (marker > -1)	
		wr.shaders[marker] = shader;

	return marker;
}

void		WR_PaintShader(int x, int y, residx_t shader, bool layer2)
{
	//paint a shader onto terrain tile (x,y)
	int index;
	int shdref;

	if (x > world.gridwidth-1 || y > world.gridheight-1 || x<0 || y<0)
		return;

	index = GRIDREF(x,y);
	shdref = WR_AllocateShaderReference(shader);

	if (shdref<0)
	{
		//we might have an unorganized shdref array...sort it and attempt to reallocate
		WR_ConsolidateShaderRefs();
		shdref = WR_AllocateShaderReference(shader);

		if (shdref<0)
		{
			Console_Notify("You have exceeded the maximum quota of shaders for this terrain!\n");
			return;
		}
	}

	wr.shaders[shdref] = shader;

	if (layer2)
		wr.shadermap[1][index] = shdref;
	else
		wr.shadermap[0][index] = shdref;

	Vid_Notify(VID_NOTIFY_TERRAIN_SHADER_MODIFIED, x, y, 0);
}


void	WR_CalcColor(int gridx, int gridy, vec3_t color)
{	
	int arrayidx;	
	int n;

	arrayidx = GRIDREF(gridx, gridy);
/*
	light[0] = -(float)(wr.dynamap[arrayidx][0] * BYTE_TO_FLOAT * wr_dynamapWeight.value) + (float)(wr.colormap[arrayidx][0]) * BYTE_TO_FLOAT;
	light[1] = -(float)(wr.dynamap[arrayidx][1] * BYTE_TO_FLOAT * wr_dynamapWeight.value) + (float)(wr.colormap[arrayidx][1]) * BYTE_TO_FLOAT;
	light[2] = -(float)(wr.dynamap[arrayidx][2] * BYTE_TO_FLOAT * wr_dynamapWeight.value) + (float)(wr.colormap[arrayidx][2]) * BYTE_TO_FLOAT;
	*/

	if (wr.ignoreColormap)
	{
		color[0] = (wr.dynamap[arrayidx][0] * BYTE_TO_FLOAT);
		color[1] = (wr.dynamap[arrayidx][1] * BYTE_TO_FLOAT);
		color[2] = (wr.dynamap[arrayidx][2] * BYTE_TO_FLOAT);
	}
	else
	{
		color[0] = (float)((wr.colormap[arrayidx][0] * BYTE_TO_FLOAT) * (wr.dynamap[arrayidx][0] * BYTE_TO_FLOAT));
		color[1] = (float)((wr.colormap[arrayidx][1] * BYTE_TO_FLOAT) * (wr.dynamap[arrayidx][1] * BYTE_TO_FLOAT));
		color[2] = (float)((wr.colormap[arrayidx][2] * BYTE_TO_FLOAT) * (wr.dynamap[arrayidx][2] * BYTE_TO_FLOAT));
	}

	for (n=0;n<3;n++)
	{
		if (color[n] > 1)
			color[n] = 1;
		if (color[n] < 0)
			color[n] = 0;
	}
}


vec3_t adv;
float	*adhocVector(float x, float y, float z)
{
	adv[0] = x;
	adv[1] = y;
	adv[2] = z;

	return adv;
}

#if 0
//#define OLD_CLIP_METHOD

//for soft shadows and various marks
//fixme: verrrrrry slow, i think some kind of DDA algorithm could be used as an alternative to this clipping
void WR_FitQuad(scenefacevert_t quadverts[4], residx_t shader, int flags)
{
#define SOLVE_FOR_Z(pl, xpos, ypos) ((-pl.normal[0]*xpos - pl.normal[1]*ypos - pl.dist) / pl.normal[2])

#ifdef OLD_CLIP_METHOD
	plane_t tri1clip[3];
	plane_t tri2clip[3];
	scenefacevert_t clipped[32];
	int	nverts;
	vec3_t normal;
	vec3_t p0, p3;
	
	 
	vec3_t bmin, bmax;
	vec3_t p1, p2;
#endif
	scenefacevert_t clipped_verts1[8], clipped_verts2[8], tmp_verts[8];
	int clipped_numverts1, clipped_numverts2;
	vec2_t gridmin, gridmax;
	int n;
	int x, y;
	ivec2_t min;
	ivec2_t max;
	plane_t plane;
	vec3_t pointOnPlane;

	if (!wr_marks.integer)
		return;

#ifdef OLD_CLIP_METHOD

	
	//set up clipping plane normals for tri1
	SET_VEC3(tri1clip[0].normal, 1, 0, 0);
	SET_VEC3(tri1clip[1].normal, 0, -1, 0);
	SET_VEC3(tri1clip[2].normal, -1, 1, 0);

	//set up clipping plane normals for tri2
	SET_VEC3(tri2clip[0].normal, -1, 0, 0);
	SET_VEC3(tri2clip[1].normal, 0, 1, 0);
	SET_VEC3(tri2clip[2].normal, 1, -1, 0);
	

#endif
	
	//get the grid space we'll be working in
	
	M_ClearRect(gridmin, gridmax);


	for (n=0; n<4; n++)
	{
		quadverts[n].vtx[0] = WORLD_TO_GRID(quadverts[n].vtx[0]);
		quadverts[n].vtx[1] = WORLD_TO_GRID(quadverts[n].vtx[1]);
		quadverts[n].vtx[2] = WORLD_TO_GRID(quadverts[n].vtx[2]);

		M_AddPointToRect(quadverts[n].vtx, gridmin, gridmax);				
	}

	if (gridmin[0] < 0) gridmin[0] = 0;
	if (gridmin[1] < 0) gridmin[1] = 0;
	if (gridmin[0] > world.gridwidth-1) gridmin[0] = world.gridwidth-1;
	if (gridmin[1] > world.gridheight-1) gridmin[1] = world.gridheight-1;

	if (gridmax[0] < 0) gridmax[0] = 0;
	if (gridmax[1] < 0) gridmax[1] = 0;
	if (gridmax[0] > world.gridwidth-1) gridmax[0] = world.gridwidth-1;
	if (gridmax[1] > world.gridheight-1) gridmax[1] = world.gridheight-1;		
/*
	M_ClearBounds(bmin, bmax);

	p1[0] = gridmin[0];
	p1[1] = gridmin[1];
	p1[2] = WORLD_GRIDREF((int)p1[0], (int)p1[1]);
	M_AddPointToBounds(p1, bmin, bmax);

	p2[0] = gridmax[0];
	p2[1] = gridmax[1];
	p2[2] = WORLD_GRIDREF((int)p2[0], (int)p2[1]);
	M_AddPointToBounds(p2, bmin, bmax);

	//to detect flat ground, you can't just check the endpoints
	if (p2[2] - p1[2] == 0)	//flat ground, so we don't need to do any clipping
	{
		for (n=0; n<4; n++)
		{
			quadverts[n].vtx[0] = GRID_TO_WORLD(quadverts[n].vtx[0]);
			quadverts[n].vtx[1] = GRID_TO_WORLD(quadverts[n].vtx[1]);
			quadverts[n].vtx[2] = GRID_TO_WORLD(quadverts[n].vtx[2]) + 0.1;
		}
		
		Scene_AddPoly(4, quadverts, shader, flags);
		return;
	}
*/

#ifndef OLD_CLIP_METHOD
	for (y = (int)gridmin[1]; y <= (int)gridmax[1]; y++)
	{
		for (x = (int)gridmin[0]; x <= (int)gridmax[0]; x++)
		{	
			if (x == world.gridwidth -1)
				continue;
			pointOnPlane[X] = GRID_TO_WORLD(x+1); pointOnPlane[Y] = GRID_TO_WORLD(y); pointOnPlane[Z] = WORLD_GRIDREF(x+1, y);

			//set up the current quad
			min[0] = x;
			min[1] = y;
			max[0] = x+1;
			max[1] = y+1;

			Scene_ClipPolyToTerrainTris(quadverts, 4, min, max, tmp_verts, 
				clipped_verts1, &clipped_numverts1, 
				clipped_verts2, &clipped_numverts2);

			if (clipped_numverts1 > 2)
			{

				//first align the quad with the current plane we are mapping to
				plane.normal[0] = world.normal[0][GRIDREF(x,y)][0];
				plane.normal[1] = world.normal[0][GRIDREF(x,y)][1];
				plane.normal[2] = world.normal[0][GRIDREF(x,y)][2];
				plane.dist = -M_DotProduct(plane.normal, pointOnPlane);
			
				for (n=0; n<clipped_numverts1; n++)			
				{
					clipped_verts1[n].vtx[0] = GRID_TO_WORLD(clipped_verts1[n].vtx[0]);
					clipped_verts1[n].vtx[1] = GRID_TO_WORLD(clipped_verts1[n].vtx[1]);
					clipped_verts1[n].vtx[2] = SOLVE_FOR_Z(plane, clipped_verts1[n].vtx[0], clipped_verts1[n].vtx[1]) + 0.2;
				}

				Scene_AddPoly(clipped_numverts1, clipped_verts1, shader, flags);
			}

			if (clipped_numverts2 > 2)
			{
				plane.normal[0] = world.normal[1][GRIDREF(x,y)][0];
				plane.normal[1] = world.normal[1][GRIDREF(x,y)][1];
				plane.normal[2] = world.normal[1][GRIDREF(x,y)][2];
				plane.dist = -M_DotProduct(plane.normal, pointOnPlane);
			
				for (n=0; n<clipped_numverts2; n++)			
				{
					clipped_verts2[n].vtx[0] = GRID_TO_WORLD(clipped_verts2[n].vtx[0]);
					clipped_verts2[n].vtx[1] = GRID_TO_WORLD(clipped_verts2[n].vtx[1]);
					clipped_verts2[n].vtx[2] = SOLVE_FOR_Z(plane, clipped_verts2[n].vtx[0], clipped_verts2[n].vtx[1]) + 0.2;
				}

				Scene_AddPoly(clipped_numverts2, clipped_verts2, shader, flags);
			}
		}
	}
#else

	for (y=(int)gridmin[1]; y<=(int)gridmax[1]; y++)
	{
		for (x=(int)gridmin[0]; x<=(int)gridmax[0]; x++)
		{	
			int n;
			plane_t plane;
			vec3_t pointOnPlane = { x, y, WORLD_GRIDREF(x, y) };

			//first triangle

			tri1clip[0].dist = -x;
			tri1clip[1].dist = y+1;
			tri1clip[2].dist = (tri2clip[2].normal[0]*(x+1) + tri2clip[2].normal[1]*(y+1));
			
			//first align the quad with the current plane we are mapping to

			plane.normal[0] = world.normal[0][GRIDREF(x,y)][0];
			plane.normal[1] = world.normal[0][GRIDREF(x,y)][1];
			plane.normal[2] = world.normal[0][GRIDREF(x,y)][2];
			plane.dist = -M_DotProduct(plane.normal, pointOnPlane);
			
			for (n=0; n<4; n++)			
			{
				quadverts[n].vtx[2] = SOLVE_FOR_Z(plane, quadverts[n].vtx[0], quadverts[n].vtx[1]);
			}

			nverts = Scene_ClipPoly(quadverts, 4, tri1clip, 3, clipped, 32);
			if (nverts)
				Scene_AddPoly(nverts, clipped, shader, flags);

			//second triangle

			tri2clip[0].dist = x+1;
			tri2clip[1].dist = -y;
			tri2clip[2].dist = -(tri2clip[2].normal[0]*x + tri2clip[2].normal[1]*y);
			
			//first align the quad with the current plane we are mapping to
			
			plane.normal[0] = world.normal[1][GRIDREF(x,y)][0];
			plane.normal[1] = world.normal[1][GRIDREF(x,y)][1];
			plane.normal[2] = world.normal[1][GRIDREF(x,y)][2];
			plane.dist = -M_DotProduct(plane.normal, pointOnPlane);
			
			for (n=0; n<4; n++)			
			{
				quadverts[n].vtx[2] = SOLVE_FOR_Z(plane, quadverts[n].vtx[0], quadverts[n].vtx[1]);
			}

			nverts = Scene_ClipPoly(quadverts, 4, tri2clip, 3, clipped, 32);
			if (nverts)
				Scene_AddPoly(nverts, clipped, shader, flags);

		}
	}
#endif
}

#endif

void	WR_GetSunVector(vec3_t out)
{
	out[0] = wr_sun_x.value;
	out[1] = wr_sun_y.value;
	out[2] = wr_sun_z.value;
}


char		*World_GetName()
{
	return world.name;
}

bool		World_UseColormap(bool use)
{
	wr.ignoreColormap = !use;
	return use;
}
