/* (C) 2002 S2 Games
 * font.c - a truetype rendering library for generating a pallette of characters at a specific size with no kerning
 */

#include "core.h"
#include <ft2build.h>
//#include FT_FREETYPE_H
#include <freetype/freetype.h>

#define MIN_SIZE 8 //8 pixels high seems reasonably small to me
#define FONT_BITMAP_SIZE 512

extern cvar_t font_useHinting;

FT_Library   library = NULL;   /* handle to library     */

bool	Font_Init()
{
#ifdef _S2_EXCLUDE_FREETYPE
	return true;
#else
	int error;
	int major, minor, patch;

    error = FT_Init_FreeType( &library );
	FT_Library_Version(library, &major, &minor, &patch);
	if (patch != FREETYPE_PATCH)
	{
		Console_Printf("freetype headers don't match binary! - compiled against %i.%i.%i!\n", FREETYPE_MAJOR, FREETYPE_MINOR, FREETYPE_PATCH);
	}
	Console_DPrintf("Initialized freetype library version %i.%i.%i\n", major, minor, patch);
    if ( error ) 
	{
		System_Error("FreeType initialization error %i\n", error);
		return false;
	}
	return true;
#endif
}

void Font_CreateBitmap(FT_Bitmap *fontBitmap, int width, int height, bitmap_t *bitmap)
{
#ifndef _S2_EXCLUDE_FREETYPE
	int i;
	Bitmap_Alloc(bitmap, width, height, BITMAP_RGBA);
	
	//color the whole thing white
	memset(bitmap->data, 255, bitmap->bmptype * width * height);
	//fill in all the alpha values to 0
	for (i = 3; i < width * height * 4; i+=4)
		bitmap->data[i] = 0;
	
	//bitmap->translucent = true;
#endif
}

void Font_DrawCharOnBitmap(fontData_t *fontData, int which, FT_Bitmap *font_bitmap, int charnum, int flags, int topleft_x, int topleft_y, int x, int y, int height, bitmap_t *bitmap)
{
	int i, j, numCharsLoaded, c;
	fontGlyphData_t	**chars;

	chars = fontData->chars[which];

	numCharsLoaded = fontData->numCharsLoaded;
	if (numCharsLoaded >= MAX_CHARS)
	{
		Console_DPrintf("Too many characters loaded, not loading char #%i\n", charnum);
		return;
	}
	if (!fontData->charMap[charnum])
	{
		Console_DPrintf("Putting char %c (%i) in slot %i\n", charnum, charnum, numCharsLoaded+1);
		fontData->charMap[charnum] = numCharsLoaded+1; //0 is reserved
		fontData->numCharsLoaded++;
	}
	c = fontData->charMap[charnum];
	
	if (font_bitmap->width)
	{
	
		chars[c] = Tag_Malloc(sizeof(fontGlyphData_t), MEM_FONT);
		chars[c]->topLeft[0] = topleft_x;
		chars[c]->topLeft[1] = topleft_y;
		chars[c]->dimensions[0] = font_bitmap->width;
		chars[c]->dimensions[1] = height;

		Console_DPrintf("font %i: new char at (%i, %i), width %i, height %i\n", which, x, y, font_bitmap->width, font_bitmap->rows);
	
		for (i = 0; i < font_bitmap->rows; i++)
			for (j = 0; j < font_bitmap->width; j++)
				bitmap->data[(bitmap->width * (y + i) + x + j)*bitmap->bmptype + bitmap->bmptype-1] = font_bitmap->buffer[font_bitmap->width * i + j];
	}
	else
		Console_Printf("warning: character %i (%c) has a 0-width glyph\n");

}

bool	Font_AddCharToBitmap(fontData_t *fontData, int which, FT_Face face, bitmap_t *bitmap, int charnum)
{
#ifndef _S2_EXCLUDE_FREETYPE
	FT_GlyphSlot slot;
	int error, height, flags;
	int *pen_x = &fontData->pen_position[which][X];
	int *pen_y = &fontData->pen_position[which][Y];
	
	flags = FT_LOAD_RENDER; // | FT_LOAD_NO_HINTING; // | FT_LOAD_TARGET_MONO;
	if (!font_useHinting.integer)
		flags |= FT_LOAD_NO_HINTING;
			
	// load glyph image into the slot (erase previous one)
	error = FT_Load_Char( face, charnum, flags);
	if (error) 
		return false;

	slot = face->glyph;  // a small shortcut

	height = face->size->metrics.max_advance/64; 
         
	if ((*pen_x + 4 + (slot->advance.x >> 6)) >= bitmap->width)
	{
		*pen_x = 0;
		*pen_y += height + 2; // + height/2;
		if (*pen_y >= bitmap->height)
			return false;
	}

	if ((face->size->metrics.ascender - face->size->metrics.descender)/64 + *pen_y >= bitmap->height)
	{
		*pen_y = bitmap->height;		
		return false;
	}
		
	// now, draw to our target surface
	Font_DrawCharOnBitmap( fontData, which, &slot->bitmap,
		          		   	charnum,
							flags,
							*pen_x + slot->bitmap_left,
					 		*pen_y - face->size->metrics.ascender/64,
					 		*pen_x + slot->bitmap_left,
					 		*pen_y - slot->bitmap_top,
					 		(face->size->metrics.ascender - face->size->metrics.descender)/64,
					 		bitmap);
                         
	// increment pen position 
	*pen_x += 2 + (slot->advance.x >> 6);

	return true;
#else
	return false;
#endif
}

bool	_Font_LoadTrueTypeFile(const char *filename, FT_Face *face)
{
#ifndef _S2_EXCLUDE_FREETYPE
	int error;
	
   	error = FT_New_Face( library, filename, 0, face);

   	if ( error == FT_Err_Unknown_File_Format )
   	{
   		Console_Printf("FreeType error: the font file '%s' could be opened and read, but it appears that its font format is unsupported.\n", filename);
		return false;
   	}
   	else if ( error )
   	{
   	 	Console_Printf("FreeType error: the font file '%s' could not be opened or read, or it is simply broken.\n", filename);
		return false;
  	}

	if ((*face)->charmap==NULL || (*face)->charmap->encoding != ft_encoding_unicode) 
	{
		Console_Printf("Unicode charmap not available for this font. Very bad!");
		error = FT_Set_Charmap(*face, (*face)->charmaps[0]);
		if (error) 
			Console_Errorf("No charmaps! Strange.");
	}

	return true;
#else
	return false;
#endif
}

bool	Font_AddChar(fontData_t *fontData, int charnum)
{
#ifndef _S2_EXCLUDE_FREETYPE
	int i, error;
	FT_Face face;

	if (!_Font_LoadTrueTypeFile(fontData->filename, &face))
		return false;
	
	Console_Printf("adding character %c (%i) to font\n", charnum, charnum);
	
	for (i = 0; i < MAX_FONT_SIZES; i++)
	{
		error = FT_Set_Pixel_Sizes(face,
             				   		0,      /* pixel_width                      */
              				   		fontData->fontSizes[i] ); /* pixel_height                     */
		/*if (error)
		{
			Console_Printf("FreeType error: the requested size of %i was not available with this font.\n", fontData->fontSizes[i]);
		}	
		else
		{*/
			Font_AddCharToBitmap(fontData, i, face, fontData->bmps[i], charnum);
		//}
	}

	FT_Done_Face(face);
	return true;
#else
	return false;
#endif
}

bool 	Font_LoadFaceSize(fontData_t *fontData, int size, int which)
{
#ifdef _S2_EXCLUDE_FREETYPE
	return false;
#else
	FT_Face face;
	FT_GlyphSlot slot;
	bitmap_t *bitmap = NULL;
	int error, n;
	
	if (!library)
		return false;

	if (!_Font_LoadTrueTypeFile(fontData->filename, &face))
		return false;
	
	error = FT_Set_Pixel_Sizes(
             				   face,   /* handle to face object            */
             				   0,      /* pixel_width                      */
              				   size ); /* pixel_height                     */

	if (error)
	{
		Console_Printf("FreeType error: the requested size of %i was not available with this font.\n", size);
	}	
	else
	{
		bitmap = Tag_Malloc(sizeof(bitmap_t), MEM_FONT);

		slot = face->glyph;  // a small shortcut

		fontData->pen_position[which][X] = 5;
		fontData->pen_position[which][Y] = face->size->metrics.max_advance/64;

		//512x512 is probably overkill, but 256x256 can be pretty small, 
		// and we need to make sure that the GL layer doesn't rescale it 
		// or our position data is worthless
		Font_CreateBitmap(&slot->bitmap, FONT_BITMAP_SIZE, FONT_BITMAP_SIZE, bitmap);

		//actually render it
		for ( n = '!'; n < 127; n++ )
		{
			if (!Font_AddCharToBitmap(fontData, which, face, bitmap, n))
				break;
		}

		fontData->bmps[which] = bitmap;
		fontData->fontSizes[which] = size;

		//Bitmap_WritePNG(fmt("bitmap%i.png", which), bitmap);
	}
	FT_Done_Face(face);
	if (fontData->pen_position[which][Y] >= bitmap->height)
	{
		Tag_Free(bitmap->data);
		Tag_Free(bitmap);
		return false;
	}
	return true;
#endif
}


fontData_t	*Font_LoadFace(const char *fontpath, int sizes[MAX_FONT_SIZES])
{
#ifdef _S2_EXCLUDE_FREETYPE
	return NULL;
#else
	fontData_t *fontData = NULL;
	int i;

	fontData = Tag_Malloc(sizeof(fontData_t), MEM_FONT);
	memset(fontData, 0, sizeof(fontData_t));

	fontData->active = true;
	fontData->filename = Tag_Strdup(fontpath, MEM_FONT);

	for (i = 0; i < MAX_FONT_SIZES; i++)
	{
		if (sizes[i] < MIN_SIZE)
			sizes[i] = MIN_SIZE;

		if (i > 0 && sizes[i] < sizes[i-1])
			sizes[i] = sizes[i-1];

		while (sizes[i] >= MIN_SIZE
				&& !Font_LoadFaceSize(fontData, sizes[i], i))
			sizes[i]--;

		if (sizes[i] < MIN_SIZE)
		{
			Tag_Free(fontData);
			return NULL;
		}
	}

	return fontData;
#endif
}

bool	Font_Unload(fontData_t *fontData)
{
	int i, j;
	
	if (!fontData)
		return false;
	
	fontData->active = false;
	
	for ( j = 0; j < MAX_FONT_SIZES; j++)
	{
		for ( i = 0; i < MAX_CHARS; i++ )
		{
			if (fontData->chars[j][i])
				Tag_Free(fontData->chars[j][fontData->charMap[i]]);
		}
		Tag_Free(fontData->bmps[j]->data);
		Tag_Free(fontData->bmps[j]);
	}
	Tag_Free(fontData->filename);

	Tag_Free(fontData);
	return true;
}
