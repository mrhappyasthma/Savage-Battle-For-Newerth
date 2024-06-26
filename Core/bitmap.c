// (C) 2003 S2 Games

// bitmap.c

// general bitmap loading functions

/* NOTE:

   All bitmap loading functions flip the bitmap they are
   loading vertically, so that we can use a top-left to
   bottom-right coord system when naming texture coords
*/


#include "core.h"

#include <string.h>
#include <png.h>

cvar_t readCompressedTextures = { "readCompressedTextures", "1" };

extern cvar_t vid_maxtexturesize;
extern cvar_t vid_compressTextures;

//#define NO_TRANSLUCENCY
bool	Bitmap_DetermineTranslucency(bitmap_t *bmp)
{
	int n;

	bmp->translucent = false;

	if (bmp->bmptype != BITMAP_RGBA)
		return false;

	for (n=3; n<bmp->width * bmp->height * 4; n+=4)
	{
#ifdef NO_TRANSLUCENCY
		if (bmp->data[n] < 128)
		{
			bmp->data[n] = 0;
			bmp->translucent = true;
		}
		else		
			bmp->data[n] = 255;
#else
		if (bmp->data[n] > 0 && bmp->data[n] < 255)
		{
			bmp->translucent = true;
			break;
		}
#endif
		//&& bmp->data[n] > 0)
//			bmp->translucent = true;
	}

	return bmp->translucent;
}		



// adapted from winquake as a temp fix


typedef struct _TargaHeader {
	unsigned char 	id_length, colormap_type, image_type;
	unsigned short	colormap_index, colormap_length;
	unsigned char	colormap_size;
	unsigned short	x_origin, y_origin, width, height;
	unsigned char	pixel_size, attributes;
} TargaHeader;

TargaHeader		targa_header;
byte			*targa_rgba;


int fgetLittleShort (file_t *f)
{
	byte	b1, b2;

	b1 = File_getc(f);
	b2 = File_getc(f);

	return (short)(b1 + b2*256);
}

int fgetLittleLong (file_t *f)
{
	byte	b1, b2, b3, b4;

	b1 = File_getc(f);
	b2 = File_getc(f);
	b3 = File_getc(f);
	b4 = File_getc(f);

	return b1 + (b2<<8) + (b3<<16) + (b4<<24);
}

bool Bitmap_CheckForS2G(const char *filename, byte *numbmps, byte maxbmps, int *levels, bitmap_t *bmps)
{
	char s2gname[1024];
	char *tmp;
	STRUCT_STAT origstats, s2gstats;
	
	if (vid_compressTextures.integer && readCompressedTextures.integer)
	{
		strncpySafe(s2gname, filename, 1024);
		tmp = (char *)Filename_GetExtension(s2gname);
		if (tmp)
		{
			strncpy(tmp, "s2g", 3);
	
			if (File_Stat(s2gname, &s2gstats)) //think of this as a File_Exists call
			{
				File_Stat(filename, &origstats);
			
				if (s2gstats.st_mtime < origstats.st_mtime)
				{
					Console_DPrintf("%s not loaded because %s has a newer modification date\n", s2gname, filename);
				}
				else if (Bitmap_LoadS2G(s2gname, numbmps, maxbmps, levels, bmps))
				{					
					return true;
				}
			}
		}
	}
	return false;
}

bool Bitmap_Load (const char *filename, bitmap_t *bitmap)
{
	const char *ext = Filename_GetExtension((char *)filename);

	if (!filename || !bitmap)
		return false;

	//otherwise load the uncompressed texture
	if ((stricmp(ext, "tga")==0))
	{
		return Bitmap_LoadTGA(filename, bitmap);
	} 
    else if ((stricmp(ext, "png")==0))
	{
		return Bitmap_LoadPNG(filename, bitmap);
	}
    else if ((stricmp(ext, "jpg")==0))
	{
		return Bitmap_LoadJPEG(filename, bitmap);
	}
	else if ((stricmp(ext, "thumb")==0))
	{
		return Bitmap_LoadPNG(filename, bitmap);
	}
	return false;
}

//all sizes are assuming the x86 architecture
//s2g format: (version 1)
//string: S2Graphic
//byte: version
//byte: numLevels

//// for each level:
//int: width
//int: height
//int: internal format
//byte: translucent
//byte: level
//int: numbytes   -- don't RELY on this being accurate, as that is an instant buffer overflow
//numbytes of image data

//any extra data - to be ignored
bool	Bitmap_LoadS2GFile(file_t *f, byte *numbmps, byte maxbmps, int *levels, bitmap_t *bmps)
{
	char header[S2GRAPHIC_HEADERLEN+1];
	unsigned char version;
	unsigned char translucent;
	unsigned char level;
	byte i;
	int tmp, ret;

	memset(header, 0, S2GRAPHIC_HEADERLEN+1);
	ret = f->read(header, S2GRAPHIC_HEADERLEN, 1, f);
	if (strncmp(header, S2GRAPHIC_HEADER, S2GRAPHIC_HEADERLEN) != 0)
	{
		Console_DPrintf("S2G Error: invalid header %s\n", header);
		File_Close(f);
		return false;
	}
	
	ret = f->read(&tmp, 1, 1, f);
	version = LittleInt(tmp);
	if (version != 1)
	{
		Console_DPrintf("S2G Error: invalid version %b\n", version);
		File_Close(f);
		return false;
	}

	ret = f->read(&tmp, 1, 1, f);
	*numbmps = LittleInt(tmp);
	
	//if the maxbmps is lower than the #, set the # to the max
	*numbmps = MIN(*numbmps, maxbmps);
	
	for (i = 0; i < *numbmps; i++)
	{
		ret = f->read(&tmp, 4, 1, f);
		bmps[i].width = LittleInt(tmp);
		ret = f->read(&tmp, 4, 1, f);
		bmps[i].height = LittleInt(tmp);
	
		if (!ret)
		{
			Console_DPrintf("Truncated S2G file\n");;
			File_Close(f);
			return false;
		}
		
		if (bmps[i].width <= 0 || bmps[i].height <= 0)
		{
			Console_DPrintf("S2G Error: invalid dimensions %i by %i\n", bmps[i].width, bmps[i].height);
			File_Close(f);
			return false;
		}

		if (i > 0 && bmps[i-1].width <= bmps[i].width && bmps[i-1].height <= bmps[i].height)
		{
			Console_DPrintf("S2G Error: invalid dimensions %i by %i for level %i\n", bmps[i].width, bmps[i].height, i);
			File_Close(f);
			return false;
		}
		
		ret = f->read(&tmp, 4, 1, f);
		bmps[i].bmptype = LittleInt(tmp);

		//single byte, no endian worries
		ret = f->read(&translucent, 1, 1, f);

		if (!ret)
		{
			Console_DPrintf("Truncated S2G file\n");;
			File_Close(f);
			return false;
		}
		
		bmps[i].translucent = translucent;

		//fill in the levels array
		ret = f->read(&level, 1, 1, f);
		levels[i] = level;
		if ((i > 0 && levels[i] != levels[i-1]+1) 
			|| levels[0] > 0)
		{
			Console_DPrintf("S2G Error: invalid level %i\n", levels[i]);
			File_Close(f);
			return false;
		}

		ret = f->read(&tmp, 4, 1, f);
		bmps[i].size = LittleInt(tmp);
	
		if (!ret)
		{
			Console_DPrintf("Truncated S2G file\n");;
			File_Close(f);
			return false;
		}

		/*
		if (vid_maxtexturesize.integer > bmps[i].width
			|| vid_maxtexturesize.integer > bmps[i].height)
		{
			f->seek(f, bmps[i].size, SEEK_CUR);
			continue;
		}
		*/
		
		//Console_DPrintf("S2G image level %i (type %i) is %i by %i\n", levels[i], bmps[i].bmptype, bmps[i].width, bmps[i].height);
		if (bmps[i].size < 1)
		{
			Console_DPrintf("S2G Error: invalid size %i\n", bmps[i].size);
			File_Close(f);
			return false;
		}
		bmps[i].data = Tag_Malloc(bmps[i].size * sizeof(unsigned char), MEM_BITMAP);
		ret = f->read(bmps[i].data, bmps[i].size, 1, f);
		if (!ret)
		{
			while (i > 0)
			{
				Tag_Free(bmps[i].data);
				i--;
			}
			File_Close(f);
			return false;
		}

		//all s2g files are RGBA
		bmps[i].mode = 32;
	}
	
	return true;
}

bool	Bitmap_LoadS2G(const char *filename, byte *numbmps, byte maxbmps, int *levels, bitmap_t *bmps)
{
	file_t *file;
	int ret;

	file = File_Open(filename, "rb");
	if (!file)
	{
		Console_DPrintf("S2G Error: read of %s failed\n", filename);
		return false;
	}

	ret = Bitmap_LoadS2GFile(file, numbmps, maxbmps, levels, bmps);

	if (ret)
	{
		Console_DPrintf("Successfully loaded S2G file %s\n", filename);
	}
	File_Close(file);

	return ret;
}

void s2_png_read_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
	file_t *f = png_ptr->io_ptr;
	f->read(data, 1, length, f);
}

void s2_png_write_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
	file_t *f = png_ptr->io_ptr;
	f->write(data, 1, length, f);
}

void s2_png_flush_data(png_structp png_ptr)
{
	return;
}

/* This code is straight from example.c, which came with libpng. -JPS */
bool Bitmap_LoadPNGFile (file_t *f, bitmap_t *bitmap)
{
#ifdef _S2_EXCLUDE_PNG
	return false;
#else
	png_structp png_ptr;
	png_infop info_ptr;
	unsigned int i, depth, bit_depth;
	unsigned int png_transforms;
	bool needToUpdate = false;
	OVERHEAD_INIT;

    /* Create and initialize the png_struct with the desired error handler
     * functions.  If you want to use the default stderr and longjump method,
     * you can supply NULL for the last three parameters.  We also supply the
     * the compiler header file version, so that we know if the application
     * was compiled with a compatible version of the library.  REQUIRED
     */
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
		 NULL, NULL, NULL);

	if (png_ptr == NULL)
	{
		File_Close(f);
		return false;
	}

	/* Allocate/initialize the memory for image information.  REQUIRED. */
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
	{
		png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		return false;
	}

    /* Set error handling if you are using the setjmp/longjmp method (this is
     * the normal method of doing things with libpng).  REQUIRED unless you
     * set up your own error handlers in the png_create_read_struct() earlier.
     */

	if (setjmp(png_jmpbuf(png_ptr)))
	{
		/* Free all of the memory associated with the png_ptr and info_ptr */
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
		/* If we get here, we had a problem reading the file */
		return false;
	}

	/* Set up the input control if you are using standard C streams */
	//png_init_io(png_ptr, f->file);
	png_set_read_fn(png_ptr, (png_voidp)f, s2_png_read_data);

	png_transforms = PNG_TRANSFORM_IDENTITY;

	png_read_png(png_ptr, info_ptr, png_transforms, NULL);
	//png_read_info(png_ptr, info_ptr);

	//png_set_strip_16(png_ptr);

	/* Expand paletted or RGB images with transparency to full alpha channels
	 * so the data will be available as RGBA quartets.
	 */
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
	{
		png_set_tRNS_to_alpha(png_ptr);

		needToUpdate = true;
	}
	
	if (info_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
	{
		png_set_palette_to_rgb(png_ptr);

		png_read_png(png_ptr, info_ptr, png_transforms, NULL);

		needToUpdate = true;
	}

	if (info_ptr->color_type == PNG_COLOR_TYPE_GRAY)
	{
		if (info_ptr->bit_depth < 8) 
			png_set_gray_1_2_4_to_8(png_ptr);

		png_set_palette_to_rgb(png_ptr);

		png_read_png(png_ptr, info_ptr, png_transforms, NULL);

		needToUpdate = true;
	}

	if (needToUpdate)
		png_read_update_info(png_ptr, info_ptr);

	if (info_ptr->color_type != PNG_COLOR_TYPE_RGB
		&& info_ptr->color_type != PNG_COLOR_TYPE_GRAY
		&& info_ptr->color_type != PNG_COLOR_TYPE_RGBA) 
	{
		png_destroy_read_struct(&png_ptr, 0, 0);
		Console_Printf("Invalid png file\n");
		return false;
	}

	bitmap->width = info_ptr->width;
	bitmap->height = info_ptr->height;

	bit_depth = info_ptr->bit_depth;
	
	if (info_ptr->channels == 3)
	{
		bitmap->bmptype = BITMAP_RGB;
		depth = 3;
	} 
	else if (info_ptr->channels == 4) 
	{
		bitmap->bmptype = BITMAP_RGBA;				
		depth = 4;
	}	
	else
	{
		bitmap->bmptype = BITMAP_RGB;
		depth = 3;
	}

	bitmap->data = Tag_Malloc(depth * info_ptr->width * info_ptr->height, MEM_BITMAP);

	//if (info_ptr->channels > 1)
	//{
		for (i = 0; i < png_ptr->num_rows; i++)
		{
			Mem_Copy(&bitmap->data[i * info_ptr->width * info_ptr->pixel_depth/info_ptr->bit_depth], 
				   info_ptr->row_pointers[i], 
				   info_ptr->pixel_depth/info_ptr->bit_depth * info_ptr->width);
		}
	//}
	//else
	//{
	/*
		//for paletted modes...
		for (y = 0; y < bitmap->height; y++) 
		{
			png_byte *row;
			row = info_ptr->row_pointers[y];

			for (x = 0; x < bitmap->width; x++) 
			{
				int index = depth * (y * bitmap->width + x);
				long data = 0;

				for (j = 0; j < bit_depth/8; j++) 
				{
					data <<= 8;
					data += row[x * bit_depth/8 + j];
				}

				for (j = 0; j < depth; j++)
					bitmap->data[index + j] = data; // for floats, you'd use: data / (float)((1 << bit_depth) - 1);
			}
		}
	}
	*/

	
    bitmap->mode = png_ptr->pixel_depth;
	bitmap->size = bitmap->bmptype * bitmap->width * bitmap->height;

	Console_DPrintf(" - %i bytes (%s) ", bitmap->bmptype* MAX(bitmap->width, bitmap->height)*MAX(bitmap->width, bitmap->height), bitmap->bmptype == BITMAP_RGBA ? "RGBA" : "RGB");
	
    /* clean up after the read, and free any memory allocated - REQUIRED */
    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);

	OVERHEAD_COUNT(OVERHEAD_TEXTURES);
    return true;
#endif
}

bool Bitmap_LoadPNG (const char *filename, bitmap_t *bitmap)
{
	file_t *f;
	int ret;

	if ((f = File_Open(filename, "rb")) == NULL)
       return false;

	ret = Bitmap_LoadPNGFile(f, bitmap);
	if (!ret)
		Console_Printf("failed to load %s - invalid PNG\n", filename);
	File_Close(f);

	return ret;
}

//for some reason the original source read the tga from
//bottom to top, thus flipping the image over...i fixed
//this
bool Bitmap_LoadTGAFile (file_t *f, bitmap_t *bitmap)
{
	int				columns, rows, numPixels;
	byte			*pixbuf;
	int				row, column;
	OVERHEAD_INIT;

	if (!bitmap) return false;

	bitmap->data = NULL;
	bitmap->translucent = false;

	targa_header.id_length = File_getc(f);
	targa_header.colormap_type = File_getc(f);
	targa_header.image_type = File_getc(f);
	
	targa_header.colormap_index = fgetLittleShort(f);
	targa_header.colormap_length = fgetLittleShort(f);
	targa_header.colormap_size = File_getc(f);
	targa_header.x_origin = fgetLittleShort(f);
	targa_header.y_origin = fgetLittleShort(f);
	targa_header.width = fgetLittleShort(f);
	targa_header.height = fgetLittleShort(f);
	targa_header.pixel_size = File_getc(f);
	targa_header.attributes = File_getc(f);

	if (targa_header.image_type!=2
		&& targa_header.image_type!=10) 
	{
		Console_DPrintf ("LoadTGA: Only type 2 RGB images supported\n");
		return false;
	}

	if (targa_header.colormap_type !=0 
		|| (targa_header.pixel_size!=32 && targa_header.pixel_size!=24))
	{
		Console_DPrintf ("Texture_LoadTGA: Only 32 or 24 bit images supported (no colormaps)\n");
		return false;
	}

	columns = targa_header.width;
	rows = targa_header.height;
	numPixels = columns * rows;

	targa_rgba = Tag_Malloc (numPixels*(targa_header.pixel_size/8), MEM_BITMAP);
	
	if (targa_header.id_length != 0)
	{
		if (targa_header.id_length < numPixels*(targa_header.pixel_size/8))
		{
			//no more seeks due to compressed file code
			//fseek(f, targa_header.id_length, SEEK_CUR);  // skip TARGA image comment
			File_Read(targa_rgba, targa_header.id_length, 1, f);
		}
		else
		{
			Console_DPrintf("error, the size of the comment is larget than the size of the image data!\n");
			return false;
		}
	}
	
	if (targa_header.image_type==2) {  // Uncompressed, RGB images
		for(row=rows-1; row>=0; row--) {
			pixbuf = targa_rgba + row*columns*(targa_header.pixel_size/8);
			for(column=0; column<columns; column++) {
				unsigned char red,green,blue,alphabyte;
				switch (targa_header.pixel_size) {
					case 24:
							
							blue = File_getc(f);
							green = File_getc(f);
							red = File_getc(f);
							*pixbuf++ = red;
							*pixbuf++ = green;
							*pixbuf++ = blue;
							//*pixbuf++ = 255;
							break;
					case 32:
							blue = File_getc(f);
							green = File_getc(f);
							red = File_getc(f);
							alphabyte = File_getc(f);
							*pixbuf++ = red;
							*pixbuf++ = green;
							*pixbuf++ = blue;
							*pixbuf++ = alphabyte;
							if (alphabyte < 255 && alphabyte > 0)
							{
								bitmap->translucent = true;								
							}
							break;
				}
			}
		}
	}
	else if (targa_header.image_type==10) {   // Runlength encoded RGB images
		unsigned char red,green,blue,alphabyte,packetHeader,packetSize,j;
		for(row=rows-1; row>=0; row--) {
			pixbuf = targa_rgba + row*columns*(targa_header.pixel_size/8);
			for(column=0; column<columns; ) {
				packetHeader=File_getc(f);
				packetSize = 1 + (packetHeader & 0x7f);
				if (packetHeader & 0x80) {        // run-length packet
					switch (targa_header.pixel_size) {
						case 24:
								blue = File_getc(f);
								green = File_getc(f);
								red = File_getc(f);
								alphabyte = 255;
								break;
						case 32:
						default:
								blue = File_getc(f);
								green = File_getc(f);
								red = File_getc(f);
								alphabyte = File_getc(f);
								if (alphabyte < 255)
									bitmap->translucent = true;
								break;
					}
	
					for(j=0;j<packetSize;j++) {
						*pixbuf++=red;
						*pixbuf++=green;
						*pixbuf++=blue;
						if (targa_header.pixel_size == 32)
							*pixbuf++=alphabyte;
						column++;
						if (column==columns) { // run spans across rows
							column=0;
							if (row>0)
								row--;
							else
								goto breakOut;
							pixbuf = targa_rgba + row*columns*(targa_header.pixel_size/8);
						}
					}
				}
				else {                            // non run-length packet
					for(j=0;j<packetSize;j++) {
						switch (targa_header.pixel_size) {
							case 24:
									blue = File_getc(f);
									green = File_getc(f);
									red = File_getc(f);
									*pixbuf++ = red;
									*pixbuf++ = green;
									*pixbuf++ = blue;
									//*pixbuf++ = 255;
									break;
							case 32:
									blue = File_getc(f);
									green = File_getc(f);
									red = File_getc(f);
									alphabyte = File_getc(f);
									*pixbuf++ = red;
									*pixbuf++ = green;
									*pixbuf++ = blue;
									*pixbuf++ = alphabyte;
									if (alphabyte < 255 && alphabyte > 0)
										bitmap->translucent = true;
									break;
						}
						column++;
						if (column==columns) { // pixel packet run spans across rows
							column=0;
							if (row>0)
								row--;
							else
								goto breakOut;
							pixbuf = targa_rgba + row*columns*(targa_header.pixel_size/8);
						}						
					}
				}
			}
			breakOut:;
		}
	}
	
	bitmap->width = columns;
	bitmap->height = rows;
	bitmap->mode = targa_header.pixel_size;
	if (bitmap->mode == 32)
		bitmap->bmptype = BITMAP_RGBA;
	else
		bitmap->bmptype = BITMAP_RGB;
	bitmap->data = targa_rgba;

	Console_DPrintf(" - %i bytes (%s - %s) ", bitmap->bmptype* bitmap->width * bitmap->height, bitmap->bmptype == BITMAP_RGBA ? "RGBA" : "RGB", bitmap->translucent ? "translucent" : "opaque");
	OVERHEAD_COUNT(OVERHEAD_TEXTURES);
	return true;
}

bool Bitmap_LoadTGA (const char *filename, bitmap_t *bitmap)
{
	file_t	*f;
	int ret;
	
	f = File_Open(filename, "rb");
	if (!f) 
	{
		Console_DPrintf("Bitmap_LoadTGA: %s not found\n", filename);
		return false;
	}

	ret = Bitmap_LoadTGAFile(f, bitmap);
	File_Close(f);

	return ret;
}


typedef struct
{
	const char *dirname;
	int size;
} thumbHelper_t;

void generateThumbnailsCallback(const char *filename, void *thumbHelper)
{
    thumbHelper_t *th;
	char fname[1024];
	char thumbname[1024];
	bitmap_t bmp, out;

    th = (thumbHelper_t *)thumbHelper;
	bmp.data = NULL;
	out.data = NULL;

	BPrintf(fname, 1023, "%s/%s", th->dirname, filename);
	BPrintf(thumbname, 1023, "%s/thumbnails/%s.thumb", th->dirname, filename);

	if (File_Exists(thumbname))
		return;  //don't recreate the thumbnail  (fixme: add in a date check to see if the file was modified?)

	if (!Bitmap_Load(fname, &bmp))
	{
		Console_DPrintf("Failure trying to load %s\n", fname);
		return;
	}
	
	Bitmap_Scale(&bmp, &out, th->size, th->size);

	Bitmap_WritePNG(thumbname, &out);

	Bitmap_Free(&bmp);
	Bitmap_Free(&out);
}

//look through all images in a directory and generate thumbnails to be used by the thumbnailgrid
void Bitmap_GenerateThumbnails(char *dirname, int size)
{
	thumbHelper_t th;
	char thumbdir[1024];

	BPrintf(thumbdir, 1023, "%s/thumbnails", dirname);

	//Console_DPrintf("adding %s\n", thumbdir);

	System_CreateDir(thumbdir);

	th.dirname = dirname;
	th.size = size;

	System_Dir(dirname, "*.*", false, NULL, generateThumbnailsCallback, &th);
}

void Bitmap_CreateThumbs_Cmd(int argc, char *argv[])
{
	if (argc < 1)
		return;

	Bitmap_GenerateThumbnails(argv[0], 32);
}

void Bitmap_GetColor(const bitmap_t *bmp, int x, int y, bvec4_t color)
{
	int step;

	if (!bmp) return;
	if (!bmp->data) return;

	step = bmp->bmptype;

	color[0] = bmp->data[(bmp->width*y+x) * step];
	if (step>1)
		color[1] = bmp->data[(bmp->width*y+x) * step + 1];
	if (step>2)
		color[2] = bmp->data[(bmp->width*y+x) * step + 2];
	if (step>3)
		color[3] = bmp->data[(bmp->width*y+x) * step + 3];
}

void	Bitmap_GetAverageColor(const bitmap_t *bmp, bvec4_t color)
{
	vec4_t tmpcol = { 0,0,0,0 };

	int n;
	int component;
	int step;

	if (!bmp) return;
	if (!bmp->data) return;

	step = bmp->mode/8;
	if (step > 4)
	{
		Console_DPrintf("invalid bitmap mode %i\n", bmp->mode);
		return;
	}
	
	for(n=0; n<bmp->width*bmp->height; n+=step)
	{
		for(component=0; component<step; component++)		
		{
			tmpcol[component] += bmp->data[n+component];
		}
	}

	for (component=0; component<step; component++)
	{
		tmpcol[component] /= (bmp->width*bmp->height) / step;
		color[component] = tmpcol[component];
	}
}

void Bitmap_Alloc(bitmap_t *bmp, int width, int height, int type)
{
	if (!bmp) return;
	bmp->data = Tag_Malloc(width*height*type, MEM_BITMAP);
	bmp->bmptype = type;
	bmp->width = width;
	bmp->height = height;
	bmp->mode = bmp->bmptype * 8;
	bmp->translucent = false;
}

void Bitmap_Free(bitmap_t *bmp)
{
	if (!bmp) return;
	if (!bmp->data) return;

	Tag_Free(bmp->data);
}

/* write a png file */
bool Bitmap_WritePNG(const char *file_name, bitmap_t *bitmap)
{
#ifdef _S2_EXCLUDE_PNG
	return false;
#else
   file_t *f;
   png_structp png_ptr;
   png_infop info_ptr;
   int png_transforms;
   int k, color_type, channels;
   OVERHEAD_INIT;

   /* open the file */
   f = File_Open(file_name, "wb");
   if (f == NULL)
      return false;

   /* Create and initialize the png_struct with the desired error handler
    * functions.  If you want to use the default stderr and longjump method,
    * you can supply NULL for the last three parameters.  We also check that
    * the library version is compatible with the one used at compile time,
    * in case we are using dynamically linked libraries.  REQUIRED.
    */
   png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
      NULL, NULL, NULL);

   if (png_ptr == NULL)
   {
      File_Close(f);
      return false;
   }

   /* Allocate/initialize the image information data.  REQUIRED */
   info_ptr = png_create_info_struct(png_ptr);
   if (info_ptr == NULL)
   {
      File_Close(f);
      png_destroy_write_struct(&png_ptr,  (png_infopp)NULL);
      return false;
   }

    /* Set error handling.  REQUIRED if you aren't supplying your own
     * error handling functions in the png_create_write_struct() call.
     */
    if (setjmp(png_jmpbuf(png_ptr)))
    {
       /* If we get here, we had a problem reading the file */
       File_Close(f);
       png_destroy_write_struct(&png_ptr, &info_ptr);
       return false;
   }

    /* set up the output control if you are using standard C streams */
	png_set_write_fn(png_ptr, (png_voidp)f, s2_png_write_data, NULL);

	if (bitmap->bmptype == BITMAP_RGBA)
	{
		color_type = PNG_COLOR_TYPE_RGBA;
		channels = 4;
	}
	else if (bitmap->bmptype == BITMAP_RGB)
	{
		color_type = PNG_COLOR_TYPE_RGB;
		channels = 3;
	}
	else
	{
		Console_DPrintf("PNG error: %s - Unknown PNG type %i\n", file_name, bitmap->bmptype);
		return false;
	}

    png_set_IHDR(png_ptr, info_ptr, bitmap->width, bitmap->height, bitmap->mode / channels, color_type,
      PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	info_ptr->valid = PNG_INFO_IDAT;
	//info_ptr->width = bitmap->width;
	//info_ptr->height = bitmap->height;

   info_ptr->row_pointers = (png_bytepp)png_malloc(png_ptr, sizeof(png_bytep) * (bitmap->height + 1));
   info_ptr->row_pointers[bitmap->height] = 0;
   for (k = 0; k < bitmap->height; k++)
     info_ptr->row_pointers[k] = bitmap->data + k*bitmap->width * bitmap->mode/8;

    /* This is the easy way.  Use it if you already have all the
     * image info living info in the structure.  You could "|" many
     * PNG_TRANSFORM flags into the png_transforms integer here.
     */
    png_transforms = PNG_TRANSFORM_IDENTITY;
    png_write_png(png_ptr, info_ptr, png_transforms, NULL);

    /* clean up after the write, and free any memory allocated */
    png_destroy_write_struct(&png_ptr, &info_ptr);

    File_Close(f);
	OVERHEAD_COUNT(OVERHEAD_IO);
    return true;
#endif
}

//all sizes are assuming the x86 architecture
//s2g format: (version 1)
//string: S2Graphic
//byte: version
//byte: numLevels

//// for each level:
//int: width
//int: height
//int: internal format
//byte: translucent
//byte: level
//int: numbytes   -- don't RELY on this being accurate, as that is an instant buffer overflow
//numbytes of image data

//any extra data - to be ignored
bool	Bitmap_WriteS2G(const char *filename, byte numbmps, int *levels, bitmap_t *bmps)
{
	file_t *f;
	unsigned char version;
	int ret, tmp;
	unsigned char translucent;
	byte i;

	version = 1;
	
	f = File_Open(filename, "wb");
	if (!f)
	{
		Console_DPrintf("Error: couldn't open %s for writing\n", filename);
		return false;
	}
	ret = f->write(S2GRAPHIC_HEADER, S2GRAPHIC_HEADERLEN, 1, f);
	tmp = LittleInt(version);
	ret = f->write(&tmp, 1, 1, f);
	tmp = LittleInt(numbmps);
	ret = f->write(&tmp, 1, 1, f);

	for (i = 0; i < numbmps; i++)
	{
		Console_DPrintf(" level %i image bmptype is %i and dimensions are %i by %i\n", (int)i, bmps[i].bmptype, bmps[i].width, bmps[i].height);
		tmp = LittleInt(bmps[i].width);
		ret = f->write(&tmp, 4, 1, f);
		tmp = LittleInt(bmps[i].height);
		ret = f->write(&tmp, 4, 1, f);
		tmp = LittleInt(bmps[i].bmptype);
		ret = f->write(&tmp, 4, 1, f);
		translucent = (unsigned char)bmps[i].translucent;
		ret = f->write(&translucent, 1, 1, f);
		ret = f->write(&i, 1, 1, f);
		tmp = LittleInt(bmps[i].size);
		ret = f->write(&tmp, 4, 1, f);
		ret = f->write(bmps[i].data, bmps[i].size, 1, f);
	}
	File_Close(f);

	Console_DPrintf("S2G file %s successfully written\n", filename);

	return true;
}

void Bitmap_Flip(bitmap_t *bmp)
{
	int i, scanline_size, half;
	char *tmp_buf;

	scanline_size = bmp->mode/8 * bmp->width;
	tmp_buf = (char *)Tag_Malloc(scanline_size, MEM_BITMAP);

	half = bmp->height/2;
	for (i = 0; i < half; i++)
	{
		Mem_Move(tmp_buf, &bmp->data[i*scanline_size], scanline_size);
		Mem_Move(&bmp->data[i*scanline_size], 
			    &bmp->data[(bmp->height-i-1)*scanline_size], 
				scanline_size);
		Mem_Move(&bmp->data[(bmp->height-i-1)*scanline_size], tmp_buf, scanline_size);
	}

	Tag_Free(tmp_buf);
}

// Bitmap_HeightMapToNormalMap
void Bitmap_HeightMapToNormalMap(bitmap_t *src, bitmap_t *dst, vec3_t scale)
{
	int i, j, w, h;
	float a;
	vec3_t dfdi, dfdj, n;

#define COORDS_TO_ARRAY_POS(a,b) (((a) * dst->width + (b)) * dst->bmptype)

	w = src->width;
	h = src->height;

	if(scale[0] == 0.f || scale[1] == 0.f || scale[2] == 0.f)
	{
		a = (float)w/h;
		if(a < 1.f)
		{
			scale[0] = 1.f;
			scale[1] = 1.f/a;
		}
		else
		{
			scale[0] = a;
			scale[1] = 1.f;
		}
		scale[2] = 1.f;
	}
	Mem_Copy(dst, src, sizeof(bitmap_t));
	dst->data = Tag_Malloc(src->width * src->height * src->bmptype, MEM_BITMAP);
	
	for (i=1; i < w-1; i++)
	{
		for (j = 1; j < h-1; j++)
		{
			M_SetVec3(dfdi, 2.f, 0.f, (float)(src->data[COORDS_TO_ARRAY_POS(i+1, j  )] - src->data[COORDS_TO_ARRAY_POS(i-1, j  )]/255.f));
			M_SetVec3(dfdj, 0.f, 2.f, (float)(src->data[COORDS_TO_ARRAY_POS(i  , j+1)] - src->data[COORDS_TO_ARRAY_POS(i  , j-1)]/255.f));
			M_CrossProduct(dfdi, dfdj, n);
			M_Modulate(n, scale);
			M_Normalize(n);  
			M_RangeCompressVec3(n, &dst->data[COORDS_TO_ARRAY_POS(i, j)]);
		}
	}
	// microsoft non-ansi c++ scoping concession
	{
		// cheesy boundary cop-out
		for(i = 0; i < w; i++)
		{
			dst->data[COORDS_TO_ARRAY_POS(i,0)]   = dst->data[COORDS_TO_ARRAY_POS(i,1)];
			dst->data[COORDS_TO_ARRAY_POS(i,h-1)] = dst->data[COORDS_TO_ARRAY_POS(i,h-2)];
		}
		for(j = 0; j < h; j++)
		{
			dst->data[COORDS_TO_ARRAY_POS(0,j)]   = dst->data[COORDS_TO_ARRAY_POS(1,j)];
			dst->data[COORDS_TO_ARRAY_POS(w-1,j)] = dst->data[COORDS_TO_ARRAY_POS(w-2,j)];
		}
	}	

#undef COORDS_TO_ARRAY_POS
}

void	Bitmap_MergePixel(unsigned char *pixel, unsigned char *avg, int bmptype, float amt)
{
	unsigned char component;

	for(component=0; component<bmptype; component++)		
	{
		avg[component] += (unsigned char)(pixel[component] * amt);
	}
}

void	_getColor2f(const bitmap_t *bmp, float x, float y, float *r, float *g, float *b, float *a)
{
	float tmp_r, tmp_g, tmp_b, tmp_a;
	float xdec, ydec;

	if (x+1 >= bmp->width || y+1 >= bmp->height)
	{
		*r = bmp->data[((int)y * bmp->width + (int)x)*bmp->bmptype];
		*g = bmp->data[((int)y * bmp->width + (int)x)*bmp->bmptype + 1];
		*b = bmp->data[((int)y * bmp->width + (int)x)*bmp->bmptype + 2];
		if (bmp->bmptype == BITMAP_RGBA)
			*a = bmp->data[((int)y * bmp->width + (int)x)*bmp->bmptype + 3];
		return;
	}

	xdec = 1 - (x - (int)x);
	ydec = 1 - (y - (int)y);

	tmp_r  = bmp->data[((int)y * bmp->width + (int)x)*bmp->bmptype] * ((xdec + ydec) / 2 );
	tmp_r += bmp->data[((int)y * bmp->width + (int)x+1)*bmp->bmptype] * (((1-xdec) + ydec) / 2 );
	tmp_r += bmp->data[((int)(y+1) * bmp->width + (int)x)*bmp->bmptype] * ((xdec + (1-ydec)) / 2 );
	tmp_r += bmp->data[((int)(y+1) * bmp->width + (int)x+1)*bmp->bmptype] * (((1-xdec) + (1-ydec)) / 2 );

	tmp_g  = bmp->data[((int)y * bmp->width + (int)x)*bmp->bmptype + 1] * ((xdec + ydec) / 2 );
	tmp_g += bmp->data[((int)y * bmp->width + (int)x+1)*bmp->bmptype + 1] * (((1-xdec) + ydec) / 2 );
	tmp_g += bmp->data[((int)(y+1) * bmp->width + (int)x)*bmp->bmptype + 1] * ((xdec + (1-ydec)) / 2 );
	tmp_g += bmp->data[((int)(y+1) * bmp->width + (int)x+1)*bmp->bmptype + 1] * (((1-xdec) + (1-ydec)) / 2 );

	tmp_b  = bmp->data[((int)y * bmp->width + (int)x)*bmp->bmptype + 2] * ((xdec + ydec) / 2 );
	tmp_b += bmp->data[((int)y * bmp->width + (int)x+1)*bmp->bmptype + 2] * (((1-xdec) + ydec) / 2 );
	tmp_b += bmp->data[((int)(y+1) * bmp->width + (int)x)*bmp->bmptype + 2] * ((xdec + (1-ydec)) / 2 );
	tmp_b += bmp->data[((int)(y+1) * bmp->width + (int)x+1)*bmp->bmptype + 2] * (((1-xdec) + (1-ydec)) / 2 );

	if (bmp->bmptype == BITMAP_RGBA)
	{
		tmp_a  = bmp->data[((int)y * bmp->width + (int)x)*bmp->bmptype + 3] * ((xdec + ydec) / 2 );
		tmp_a += bmp->data[((int)y * bmp->width + (int)x+1)*bmp->bmptype + 3] * (((1-xdec) + ydec) / 2 );
		tmp_a += bmp->data[((int)(y+1) * bmp->width + (int)x)*bmp->bmptype + 3] * ((xdec + (1-ydec)) / 2 );
		tmp_a += bmp->data[((int)(y+1) * bmp->width + (int)x+1)*bmp->bmptype + 3] * (((1-xdec) + (1-ydec)) / 2 );
		
		*a = tmp_a / 2;
	}

	*r = tmp_r / 2;
	*g = tmp_g / 2;
	*b = tmp_b / 2;
}

void	Bitmap_ScaleUp(const bitmap_t *bmp, bitmap_t *out, int width, int height)
{
	float x, y, xstep, ystep;
	float avg_r, avg_g, avg_b, avg_a;
	int step, i, j, offset;
	
	step = bmp->bmptype;
	
	ystep = (float)height/bmp->height;
	xstep = (float)width/bmp->width;

	out->height = height;
	out->width = width;
	out->data = Tag_Realloc(out->data, sizeof(unsigned char) * bmp->bmptype * width * height, MEM_BITMAP);
	out->bmptype = bmp->bmptype;
	out->mode = out->bmptype * 8;

	for (j = 0; j < height; j++)
	{
		for (i = 0; i < width; i++)
		{
			x = i / xstep;
			y = j / ystep;

			offset = (j*width + i)*step;

			_getColor2f(bmp, x, y, &avg_r, &avg_g, &avg_b, &avg_a);

			out->data[offset] = (unsigned char)avg_r;
			out->data[offset+1] = (unsigned char)avg_g;
			out->data[offset+2] = (unsigned char)avg_b;
			if (bmp->bmptype == BITMAP_RGBA)
				out->data[offset+3] = (unsigned char)avg_a;
		}
	}
}

void	Bitmap_ScaleDown(const bitmap_t *bmp, bitmap_t *out, int width, int height)
{
	float x, y, xstep, ystep;
	float avg_r, avg_g, avg_b, avg_a;
	int step, tmp_x, tmp_y, i, j, offset;
	
	step = bmp->bmptype;
	
	ystep = (float)bmp->height/height;
	xstep = (float)bmp->width/width;

	out->height = height;
	out->width = width;
	out->data = Tag_Realloc(out->data, sizeof(unsigned char) * bmp->bmptype * width * height, MEM_BITMAP);
	out->bmptype = bmp->bmptype;
	out->mode = out->bmptype * 8;

	for (j = 0; j < height; j++)
	{
		for (i = 0; i < width; i++)
		{
			x = i * xstep;
			y = j * ystep;

			avg_r = avg_g = avg_b = avg_a = 0;
			tmp_x = 0; //in case ystep <= 0, this won't crash
			for (tmp_y = 0;  tmp_y < ystep; tmp_y++)
			{
				for (tmp_x = 0;  tmp_x < xstep; tmp_x++)
				{
					avg_r += bmp->data[(int)(((int)y + tmp_y)*bmp->width + (int)(x+0.5) + tmp_x)*bmp->bmptype];
					avg_g += bmp->data[(int)(((int)y + tmp_y)*bmp->width + (int)(x+0.5) + tmp_x)*bmp->bmptype + 1];
					avg_b += bmp->data[(int)(((int)y + tmp_y)*bmp->width + (int)(x+0.5) + tmp_x)*bmp->bmptype + 2];
					if (bmp->bmptype == BITMAP_RGBA)
						avg_a += bmp->data[(int)(((int)y + tmp_y)*bmp->width + (int)(x+0.5) + tmp_x)*bmp->bmptype + 3];
				}
			}

			avg_r /= tmp_x*tmp_y;
			avg_g /= tmp_x*tmp_y;
			avg_b /= tmp_x*tmp_y;
			avg_a /= tmp_x*tmp_y;
	
			offset = (j*width + i)*step;

			out->data[offset] = (unsigned char)avg_r;
			out->data[offset+1] = (unsigned char)avg_g;
			out->data[offset+2] = (unsigned char)avg_b;
			if (bmp->bmptype == BITMAP_RGBA)
				out->data[offset+3] = (unsigned char)avg_a;

		}
	}
}

void	Bitmap_Scale(const bitmap_t *bmp, bitmap_t *out, int width, int height)
{
	bitmap_t tmp_bmp;

	if (!bmp) return;
	if (!bmp->data) return;

	if (!out) return;

	if (height >= bmp->height && width >= bmp->width)
	{
		Bitmap_ScaleUp(bmp, out, width, height);
	}
	else if (height <= bmp->height && width <= bmp->width)
	{
		Bitmap_ScaleDown(bmp, out, width, height);
	}
	else if (height <= bmp->height && width > bmp->width)
	{
		Mem_Copy(&tmp_bmp, bmp, sizeof (bitmap_t));
		tmp_bmp.data = NULL;
		Bitmap_ScaleUp(bmp, &tmp_bmp, width, bmp->height);
		Bitmap_ScaleDown(&tmp_bmp, out, bmp->width, height);
	}
	else if (height > bmp->height && width <= bmp->width)
	{
		Mem_Copy(&tmp_bmp, bmp, sizeof (bitmap_t));
		tmp_bmp.data = NULL;
		Bitmap_ScaleUp(bmp, &tmp_bmp, bmp->width, height);
		Bitmap_ScaleDown(&tmp_bmp, out, width, bmp->height);
	}
}

void	Bitmap_SetPixel4b(bitmap_t *bmp, int x, int y, byte r, byte g, byte b, byte a)
{
	int index;

	if (!bmp)
		return;

	if (x >= bmp->width || y >= bmp->height || x <= 0 || y <= 0)
		return;

	index = (y * bmp->width + x) * bmp->bmptype;

	bmp->data[index] = r;
	bmp->data[index+1] = g;
	bmp->data[index+2] = b;
	if (bmp->bmptype == BITMAP_RGBA)
		bmp->data[index+3] = a;
}

void	Bitmap_DesaturateToAlpha(bitmap_t *bmp)
{
	int i, j;
	int total;

	for (i = 0; i < world.gridwidth * world.gridheight; i++)
	{
		j = i * bmp->bmptype;
		total = bmp->data[j] + bmp->data[j+1] + bmp->data[j+2];
		memset(&bmp->data[j], 0, sizeof(unsigned char) * 3);
		bmp->data[j+3] = 255 - (unsigned char)((float)total/3);
	}
}
 
void	Bitmap_Init()
{
	Cmd_Register("createthumbs", Bitmap_CreateThumbs_Cmd);
	Cvar_Register(&readCompressedTextures);
	
#ifndef _S2_EXCLUDE_PNG
	Console_DPrintf("Using png library version %u\n", png_access_version_number());

	if (png_access_version_number() != PNG_LIBPNG_VER)
	{
		System_Error("PNG header and library versions do not match\n");
	}
#endif
}
