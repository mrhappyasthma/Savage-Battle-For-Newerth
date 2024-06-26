// (C) 2003 S2 Games

// le_modelmode.h

// modeling mode interface elements


#include "../le/le.h"


cvar_t	le_terrain_dheight = { "le_terrain_dheight", "200" };
cvar_t	le_terrain_mergestrength = { "le_terrain_mergestrength", "0" };
cvar_t	le_terrain_r = { "le_terrain_r", "1.4" };
cvar_t	le_terrain_globaldeformbrush = {"le_terrain_globaldeformbrush", ""};


//modeling functions


float	LE_AddModel(int gridx, int gridy, float source, float strength)
{
	return source + strength;
}

float	LE_HalfAndHalfModel(int gridx, int gridy, float source, float strength)
{
	float bstrength = LE_GetBrushStrength();
	float result = strength - bstrength / 2;

	return source + result;  //fixme: result * 2?
}

float	LE_FlattenModel(int gridx, int gridy, float source, float strength)
{
	float ret;

	if (le_flattenheight.value > source)
	{
		ret = source + strength;
		if (ret > le_flattenheight.value) ret = le_flattenheight.value;
	}
	else if (le_flattenheight.value < source)
	{
		ret = source - strength;
		if (ret < le_flattenheight.value) ret = le_flattenheight.value;
	}
	else
		return source;

	return ret;
}

float	LE_SmoothModel(int gridx, int gridy, float source, float strength)
{
	//sample the 8 surrounding points, average them, and move point towards that value
	
	float avg;
	float ret;

	avg = corec.World_GetGridHeight(gridx-1, gridy) + corec.World_GetGridHeight(gridx-1, gridy-1) +
		  corec.World_GetGridHeight(gridx, gridy-1) + corec.World_GetGridHeight(gridx+1, gridy-1) +
		  corec.World_GetGridHeight(gridx+1, gridy) + corec.World_GetGridHeight(gridx+1, gridy+1) +
		  corec.World_GetGridHeight(gridx, gridy+1) + corec.World_GetGridHeight(gridx-1, gridy+1);

	avg /= 8.0;

	if (source > avg)
	{
		ret = source - (strength*0.5);
		if (ret < avg) ret = avg;
	}
	else if (source < avg)
	{
		ret = source + (strength*0.5);
		if (ret > avg) ret = avg;
	}
	else
	{
		ret = source;
	}

	return ret;
}

float	LE_CutModel(int gridx, int gridy, float source, float strength)
{
	if (strength > 0)
		return 0;
	else
		return source;
}

float	LE_Slice(int gridx, int gridy, float source, float strength)
{
	float ret;

	if (source > le_flattenheight.value)
	{
		ret = source - strength;
		if (ret < le_flattenheight.value)
			ret = le_flattenheight.value;
	}
	else
		return source;

	return ret;
}

float	LE_GenerateTerrain_CalcSquare(int x, int y, int stride)
{
	return (corec.World_GetGridHeight(x,y) 
		  + corec.World_GetGridHeight(x+stride,y) 
		  + corec.World_GetGridHeight(x,y+stride) 
		  + corec.World_GetGridHeight(x+stride,y+stride)) * 0.25f;
}

float	LE_GenerateTerrain_CalcDiamond(int x, int y, int half)
{
	return ( corec.World_GetGridHeight(x-half,y) 
		   + corec.World_GetGridHeight(x,y-half) 
		   + corec.World_GetGridHeight(x+half,y) 
		   + corec.World_GetGridHeight(x,y+half)) * 0.25f;
}

void	LE_GenerateTerrain_DiamondSquare(vec2_t v1, vec2_t v2)
{
	int width, height;
	int stride, pass;
	int x, y, density, size, half;
	float rough, strength;

	rough = le_terrain_r.value;

	width = v2[0] - v1[0] + 1;
	height = v2[1] - v1[1] + 1;

	strength = le_brushstrength.value;

	// Calculate log2(width of grid)
	density = log( (double)(width) ) / log( 2.0 );
  
	// Check if 2^dens is smaller than grid size, and correct
	if ( ((1<<density)+1) < (width) ) density++;
	size = (1<<density)+1;
	
	for (pass = 0; pass < density; pass++)
	{
		stride = 1 << (density-pass);
		half = stride / 2;

		// Diamond
		for( x=0; x<(size-1); x+=stride )
		{
			for( y=0; y<(size-1); y+=stride )
			{
				corec.World_DeformGround(x+half, y+half, corec.World_GetGridHeight(x+half, y+half) + strength * (rough * M_Randnum(-0.5f, 0.5f) + LE_GenerateTerrain_CalcSquare(x, y, stride)));
			}
		}

		// Square
		if (half>0)
		{
			for( x=0; x<size; x+=half )
			{
				for( y=((x+half)%stride); y<size; y+=stride )
				{
					corec.World_DeformGround(x, y, corec.World_GetGridHeight(x, y) + strength * (rough * M_Randnum(-0.5f, 0.5f) + LE_GenerateTerrain_CalcDiamond(x, y, half)));
				}
			}
		}
		rough=rough*pow(2.0,-le_terrain_r.value);
	}

//	corec.World_
}

void	LE_GenerateTerrain_Cmd(int argc, char *argv[])
{
	vec3_t world_min, world_max;
	vec2_t v1;
	vec2_t v2;
	float height, strength;

	corec.World_GetBounds(world_min, world_max);

	v1[0] = world_min[0];
	v1[1] = world_min[1];
	v2[0] = world_max[0];
	v2[1] = world_max[1];

	strength = le_brushstrength.value;

	height = (rand()*le_terrain_dheight.value/2/RAND_MAX - le_terrain_dheight.value);
	corec.World_DeformGround((int)world_min[0], (int)world_min[1], corec.World_GetGridHeight((int)world_min[0], (int)world_min[1]) + strength * height);
	height = (rand()*le_terrain_dheight.value/2/RAND_MAX - le_terrain_dheight.value);
	corec.World_DeformGround((int)world_min[0], (int)world_max[1], corec.World_GetGridHeight((int)world_min[0], (int)world_min[1]) + strength * height);
	height = (rand()*le_terrain_dheight.value/2/RAND_MAX - le_terrain_dheight.value);
	corec.World_DeformGround((int)world_max[0], (int)world_min[1], corec.World_GetGridHeight((int)world_min[0], (int)world_min[1]) + strength * height);
	height = (rand()*le_terrain_dheight.value/2/RAND_MAX - le_terrain_dheight.value);
	corec.World_DeformGround((int)world_max[0], (int)world_max[1], corec.World_GetGridHeight((int)world_min[0], (int)world_min[1]) + strength * height);

	LE_GenerateTerrain_DiamondSquare(v1, v2);
}

void	LE_MergeTerrain_Cmd(int argc, char *argv[])
{
	vec3_t world_min, world_max;
	float strength, dheight;
	int x, y, width, height, avg;
	bitmap_t in;
	static bitmap_t out = { NULL, 0, 0 };
	static char bmpname[256] = "";

	if (argc < 1)
	{
		corec.Console_Printf("syntax: merge_terrain <filename>\n");
	}

	corec.World_GetBounds(world_min, world_max);
	width = corec.WorldToGrid(world_max[0]) - corec.WorldToGrid(world_min[0]);
	height = corec.WorldToGrid(world_max[1]) - corec.WorldToGrid(world_min[1]); 

	strength = le_brushstrength.value;
	dheight = le_terrain_dheight.value;

	if (strcmp(bmpname, argv[0])!=0 || !out.data)
	{
		strncpy(bmpname, argv[0], 255);

		if (!corec.Bitmap_Load(argv[0], &in))
		{
			corec.Console_Printf("merge_terrain error: couldn't load %s!\n", argv[0]);
			return;
		}

		if (out.data)
			corec.Bitmap_Free(&out);  //fixme: if this function is called any time during the program, we're left with an unfreed bitmap

		memcpy(&out, &in, sizeof (bitmap_t));
		out.data = NULL;
		corec.Bitmap_Scale(&in, &out, width, height);

		corec.Bitmap_Free(&in);
	}
	
	for (y = 0; y < out.height; y++)
		for (x = 0; x < out.width; x++)
		{
			float gridheight;

			avg = (
				out.data[(y * out.width + x) * out.bmptype   + 0] 
				+ out.data[(y * out.width + x) * out.bmptype + 1] 
				+ out.data[(y * out.width + x) * out.bmptype + 2]) / 3;
			
			gridheight = corec.World_GetGridHeight(x, y);

			corec.World_DeformGround(x, y, gridheight + le_terrain_mergestrength.value * avg / 255);
		}

//	corec.Bitmap_Free(&out);
}

// interface

void	LE_ModelMode_Cmd(int argc, char *argv[])
{
	if (argc < 1)
	{
		corec.Console_Printf("syntax: modelmode raiselower|flatten|smooth|cut|halfandhalf\n");
		return;
	}

	if (strcmp(argv[0], "raiselower")==0)
	{
		corec.Cvar_SetVar(&le_mode, "model");
		LE_SetModelFunction(LE_AddModel);
		return;
	}
	else if (strcmp(argv[0], "flatten")==0)
	{
		corec.Cvar_SetVar(&le_mode, "model");
		LE_SetModelFunction(LE_FlattenModel);
		return;
	}
	else if (strcmp(argv[0], "smooth")==0)
	{
		corec.Cvar_SetVar(&le_mode, "model");
		LE_SetModelFunction(LE_SmoothModel);
		return;
	}
	else if (strcmp(argv[0], "cut")==0)
	{
		corec.Cvar_SetVar(&le_mode, "model");
		LE_SetModelFunction(LE_CutModel);
		return;
	}
	else if (strcmp(argv[0], "halfandhalf")==0)
	{
		corec.Cvar_SetVar(&le_mode, "model");
		LE_SetModelFunction(LE_HalfAndHalfModel);
		return;
	}
	else if (strcmp(argv[0], "slice")==0)
	{
		corec.Cvar_SetVar(&le_mode, "model");
		LE_SetModelFunction(LE_Slice);
	}
}

void	LE_ModelMode_Init()
{
	corec.Cvar_Register(&le_terrain_dheight);
	corec.Cvar_Register(&le_terrain_r);
	corec.Cvar_Register(&le_terrain_globaldeformbrush);
	corec.Cvar_Register(&le_terrain_mergestrength);

	corec.Cmd_Register("generate_terrain", LE_GenerateTerrain_Cmd);
	corec.Cmd_Register("merge_terrain", LE_MergeTerrain_Cmd);
	
	corec.Cmd_Register("modelmode", LE_ModelMode_Cmd);
}	

