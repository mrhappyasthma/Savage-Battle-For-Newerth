
#define	MAX_FACES 4

//l18n people cringe!
#define MAX_CHARS 4096
#define MAX_POSSIBLE_CHARS	65536

#define	MAX_FONT_SIZES	3

typedef struct 
{
	ivec2_t topLeft;
	ivec2_t dimensions;
} fontGlyphData_t;

typedef struct
{
	bool active;

	char			*filename;

	short			charMap[MAX_POSSIBLE_CHARS];
	short			numCharsLoaded;

	fontGlyphData_t *chars[MAX_FONT_SIZES][MAX_CHARS];	 //store a pointer, so we can use NULL for non-existant chars rather than 4 floats
	bitmap_t		*bmps[MAX_FONT_SIZES];
	residx_t		fontShaders[MAX_FONT_SIZES];
	int				fontSizes[MAX_FONT_SIZES];

	ivec2_t			pen_position[MAX_FONT_SIZES];
} fontData_t;

bool	Font_Init();

fontData_t  *Font_LoadFace(const char *fontpath, int sizes[MAX_FONT_SIZES]);
bool    Font_Unload(fontData_t *fontData);

bool    Font_AddChar(fontData_t *fontData, int charnum);
