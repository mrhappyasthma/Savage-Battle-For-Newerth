// (C) 2003 S2 Games

// le_tools.h

// level editor tools


#define	MAX_BRUSHES			32
#define	MAX_BRUSH_SIZE		32
#define	MAX_TMAT_STRING		50
#define MAX_TMAT_SHAPES		16
#define MAX_TMAT_SHADERS	256

typedef struct
{
	residx_t	icon;

	residx_t	shaders[MAX_TMAT_SHADERS];
	int			num_shaders;
	residx_t	shapes[MAX_TMAT_SHAPES][5][5];
	int			cur_shape;
	int			width;
	int			height;
} tmat_t;


void		LE_SelectBrush(int brush);
int			LE_CurrentBrush();
void		LE_LoadBrush(byte brush[MAX_BRUSH_SIZE][MAX_BRUSH_SIZE]);
void		LE_LoadBrushBmp(char *filename);
void		LE_SetBrushColor(vec4_t color);
void		LE_SetBrushStrength(float strength);
float		LE_GetBrushStrength();

//terrain operations
void		LE_ModelTerrain(int xpos, int ypos);
void		LE_ModelTerrainXY(int gridx, int gridy, float strength);
void		LE_ColorTerrain(int xpos, int ypos);
void		LE_TextureTerrain(int xpos, int ypos);
void		LE_PaintAlpha(int xpos, int ypos, float destalpha);
void		LE_PaintTMat(int xpos, int ypos, float destalpha);

//model function stuff
void		LE_SetModelFunction(float(*func)(int gridx, int gridy, float source, float strength));
void		*LE_CurrentModelFunction();


//terrain shader stuff
residx_t	LE_CurrentTerrainShader();
void		LE_SetTerrainShader(residx_t shader);

//tmat (terrain shader groups) stuff
void		LE_SetTMat(tmat_t *tmat);
tmat_t		*LE_CurrentTMat();
void		LE_LoadTMat(const char *filename);

void		LE_InitTools();


extern byte	le_brushes[MAX_BRUSHES][MAX_BRUSH_SIZE][MAX_BRUSH_SIZE];
