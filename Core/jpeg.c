// (C) 2003 S2 Games

// jpeg.c

// jpeg loading functions

//=============================================================================

#include "core.h"

#include <string.h>

#ifndef _S2_EXCLUDE_JPEG

#include <jpeglib.h>
#include <jerror.h>
#include <setjmp.h>

//=============================================================================

typedef struct
{
	struct jpeg_error_mgr pub;	// "public" fields

	jmp_buf setjmp_buffer;		// for return to caller
}
JPEG_errorMgr_t;

//=============================================================================


/*==========================

  JPEG_OutputMessage

  Override for the default message output function, so that it prints to the console

 ==========================*/

void	JPEG_OutputMessage(j_common_ptr cinfo)
{
	char buffer[JMSG_LENGTH_MAX];

    (*cinfo->err->format_message)(cinfo, buffer);
  	Console_Printf("%s\n", buffer);
}
  

/*==========================

  JPEG_ErrorExit

  Override the default handling for a fatal error, which would normally exit the program
  Instead, output the error and jump to the saved stack point

 ==========================*/

void JPEG_ErrorExit(j_common_ptr cinfo)
{
	JPEG_errorMgr_t *myerr = (JPEG_errorMgr_t*) cinfo->err;

	(*cinfo->err->output_message) (cinfo);

	longjmp(myerr->setjmp_buffer, 1);
}

#endif	//_S2_EXCLUDE_JPEG


/******************** JPEG INTERFACE *******************/

/*
 * IMAGE DATA FORMATS:
 *
 * The standard input image format is a rectangular array of pixels, with
 * each pixel having the same number of "component" values (color channels).
 * Each pixel row is an array of JSAMPLEs (which typically are unsigned chars).
 * If you are working with color data, then the color values for each pixel
 * must be adjacent in the row; for example, R,G,B,R,G,B,R,G,B,... for 24-bit
 * RGB color.
 *
 */

bool	Bitmap_WriteJPEG(const char * filename, bitmap_t *bitmap, int quality)
{
#ifdef _S2_EXCLUDE_JPEG
	return false;
#else
  /* This struct contains the JPEG compression parameters and pointers to
   * working space (which is allocated as needed by the JPEG library).
   * It is possible to have several such structures, representing multiple
   * compression/decompression processes, in existence at once.  We refer
   * to any one struct (and its associated working data) as a "JPEG object".
   */
  struct jpeg_compress_struct cinfo;
  /* This struct represents a JPEG error handler.  It is declared separately
   * because applications often want to supply a specialized error handler
   * (see the second half of this file for an example).  But here we just
   * take the easy way out and use the standard error handler, which will
   * print a message on stderr and call exit() if compression fails.
   * Note that this struct must live as long as the main JPEG parameter
   * struct, to avoid dangling-pointer problems.
   */
  struct jpeg_error_mgr jerr;
  /* More stuff */
  FILE *outfile;		/* target file */
  JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
  int row_stride;		/* physical row width in image buffer */

  /* Step 1: allocate and initialize JPEG compression object */

  /* We have to set up the error handler first, in case the initialization
   * step fails.  (Unlikely, but it could happen if you are out of memory.)
   * This routine fills in the contents of struct jerr, and returns jerr's
   * address which we place into the link field in cinfo.
   */
  cinfo.err = jpeg_std_error(&jerr);
  /* Now we can initialize the JPEG compression object. */
  jpeg_create_compress(&cinfo);

  /* Step 2: specify data destination (eg, a file) */
  /* Note: steps 2 and 3 can be done in either order. */

  /* Here we use the library-supplied code to send compressed data to a
   * stdio stream.  You can also write your own code to do something else.
   * VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
   * requires it in order to write binary files.
   */
  if ((outfile = fopen(filename, "wb")) == NULL) 
  {
    fprintf(stderr, "can't open %s\n", filename);
    return false;
  }
  jpeg_stdio_dest(&cinfo, outfile);

  /* Step 3: set parameters for compression */

  /* First we supply a description of the input image.
   * Four fields of the cinfo struct must be filled in:
   */
  cinfo.image_width = bitmap->width; 	/* image width and height, in pixels */
  cinfo.image_height = bitmap->height;
  cinfo.input_components = 3;		/* # of color components per pixel */
  cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
  /* Now use the library's routine to set default compression parameters.
   * (You must set at least cinfo.in_color_space before calling this,
   * since the defaults depend on the source color space.)
   */
  jpeg_set_defaults(&cinfo);
  /* Now you can set any non-default parameters you wish to.
   * Here we just illustrate the use of quality (quantization table) scaling:
   */
  jpeg_set_quality(&cinfo, quality * 255 / 100, TRUE /* limit to baseline-JPEG values */);

  /* Step 4: Start compressor */

  /* TRUE ensures that we will write a complete interchange-JPEG file.
   * Pass TRUE unless you are very sure of what you're doing.
   */
  jpeg_start_compress(&cinfo, TRUE);

  /* Step 5: while (scan lines remain to be written) */
  /*           jpeg_write_scanlines(...); */

  /* Here we use the library's state variable cinfo.next_scanline as the
   * loop counter, so that we don't have to keep track ourselves.
   * To keep things simple, we pass one scanline per call; you can pass
   * more if you wish, though.
   */
  row_stride = bitmap->width * 3;	/* JSAMPLEs per row in the image buffer */

  while (cinfo.next_scanline < cinfo.image_height) {
    /* jpeg_write_scanlines expects an array of pointers to scanlines.
     * Here the array is only one element long, but you could pass
     * more than one scanline at a time if that's more convenient.
     */
    row_pointer[0] = &bitmap->data[cinfo.next_scanline * row_stride];
    (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }

  /* Step 6: Finish compression */

  jpeg_finish_compress(&cinfo);
  /* After finish_compress, we can close the output file. */
  fclose(outfile);

  /* Step 7: release JPEG compression object */

  /* This is an important step since it will release a good deal of memory. */
  jpeg_destroy_compress(&cinfo);

  /* And we're done! */
  return true;
#endif
}


/*==========================

  Bitmap_LoadJPEGFile

 ==========================*/

bool	Bitmap_LoadJPEGFile(file_t *f, bitmap_t *bitmap)
{
#ifdef _S2_EXCLUDE_JPEG
	return false;
#else

	int				row_stride;
	int				depth, i;
	unsigned char	**buffer;
	JPEG_errorMgr_t	jerr;

	struct jpeg_decompress_struct	cinfo;


	/* Step 1: allocate and initialize JPEG decompression object */
	
	memset(&cinfo, 0, sizeof(cinfo));

	// Override the default error handling
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = JPEG_ErrorExit;
	jerr.pub.output_message = JPEG_OutputMessage;

	// Set the return point for an error
	if (setjmp(jerr.setjmp_buffer)) 
	{
		Console_Errorf("Aborting JPEG load\n");
		jpeg_destroy_decompress(&cinfo);
		return false;
	}

	// Create the decompression object
	jpeg_create_decompress(&cinfo);


	/* Step 2: specify data source */
	
	jpeg_stdio_src(&cinfo, f->file);

	
	/* Step 3: read file parameters with jpeg_read_header() */
	
	jpeg_read_header(&cinfo, TRUE);

	
	/* Step 4: set parameters for decompression */
	
	/* We don't need to change any of the defaults set by
	 * jpeg_read_header(), so we do nothing here.
	 */


	/* Step 5: Start decompressor */
	
	jpeg_start_decompress(&cinfo);
	
	// Fill in the bitmap_t structure
	bitmap->width = cinfo.output_width;
	bitmap->height = cinfo.output_height;
	depth = 3;
	bitmap->bmptype = BITMAP_RGB;			//jpegs do not support alpha
	bitmap->data = Tag_Malloc(depth * bitmap->width * bitmap->height, MEM_BITMAP);
	
	row_stride = cinfo.output_width * cinfo.output_components;
	
	/* Make a one-row-high sample array that will go away when done with image */
	buffer = (unsigned char**)Tag_Malloc(cinfo.rec_outbuf_height * sizeof(unsigned char*), MEM_BITMAP);
	for (i = 0; i < cinfo.rec_outbuf_height; i++)
		buffer[i] = (unsigned char*)Tag_Malloc(row_stride * sizeof(JSAMPLE), MEM_BITMAP);


	/* Step 6: while (scan lines remain to be read) */
	/*           jpeg_read_scanlines(...); */
	
	/* Here we use the library's state variable cinfo.output_scanline as the
	 * loop counter, so that we don't have to keep track ourselves.
	 */
	while (cinfo.output_scanline < cinfo.output_height) 
	{
	  	jpeg_read_scanlines(&cinfo, buffer, cinfo.rec_outbuf_height);
		for (i = 0; i < cinfo.rec_outbuf_height; i++)
			Mem_Copy(&bitmap->data[((cinfo.output_scanline - 1) * row_stride) + (i * row_stride)], buffer[i], row_stride);
	}

	for (i = 0; i < cinfo.rec_outbuf_height; i++)
		Tag_Free(buffer[i]);
	Tag_Free(buffer);
	buffer = NULL;


	/* Step 7: Finish decompression */
	
	jpeg_finish_decompress(&cinfo);
	
	
	/* Step 8: Release JPEG decompression object */
	
	/* This is an important step since it will release a good deal of memory. */
	jpeg_destroy_decompress(&cinfo);
	
	/* At this point you may want to check to see whether any corrupt-data
	 * warnings occurred (test whether jerr.pub.num_warnings is nonzero).
	 */
	
	/* And we're done! */
	return true;
#endif
}


/*==========================

  Bitmap_LoadJPEG

  ==========================*/

bool	Bitmap_LoadJPEG(const char *filename, bitmap_t *bitmap)
{
#ifdef _S2_EXCLUDE_JPEG
	return false;
#else

	file_t *f;
	bool ret;

	f = File_Open(filename, "rb");
	if (!f)
	{
		Console_Printf("Failed to open JPEG file %s\n", filename);
		return false;
	}

	ret = Bitmap_LoadJPEGFile(f, bitmap);
	
	File_Close(f);

	return ret;
#endif
}
