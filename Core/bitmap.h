// (C) 2003 S2 Games

// bitmap.h

// general bitmap loading functions

bool Bitmap_Load(const char *filename, bitmap_t *bmp);
bool Bitmap_LoadPNG(const char *filename, bitmap_t *bmp);
bool Bitmap_LoadPNGFile (file_t *f, bitmap_t *bitmap);
bool Bitmap_LoadTGA(const char *filename, bitmap_t *bmp);
bool Bitmap_LoadTGAFile(file_t *f, bitmap_t *bmp);
bool Bitmap_LoadJPEG(const char *filename, bitmap_t *bitmap);
bool Bitmap_LoadJPEGFile (file_t *f, bitmap_t *bitmap);
bool Bitmap_CheckForS2G(const char *filename, byte *numbmps, byte maxbmps, int *levels, bitmap_t *bmps);
bool Bitmap_LoadS2G(const char *filename, byte *numbmps, byte maxbmps, int *levels, bitmap_t *bmps);
bool Bitmap_LoadS2GFile(file_t *f, byte *numbmps, byte maxbmps, int *levels, bitmap_t *bmps);
void Bitmap_GetColor(const bitmap_t *bmp, int x, int y, bvec4_t color);
void Bitmap_GetAverageColor(const bitmap_t *bmp, bvec4_t color);
void Bitmap_DesaturateToAlpha(bitmap_t *bmp);
void Bitmap_Free(bitmap_t *bmp);
bool Bitmap_WritePNG(const char *filename, bitmap_t *bmp);
bool Bitmap_WriteS2G(const char *filename, byte numbmps, int *levels, bitmap_t *bmps);
bool Bitmap_WriteJPEG(const char *filename, bitmap_t *bmp, int quality); //quality goes from 1 - 100
void Bitmap_SaveScreenshot(const char *filename);
void Bitmap_Flip(bitmap_t *bmp);
void Bitmap_Scale(const bitmap_t *bmp, bitmap_t *out, int width, int height);
void Bitmap_GenerateThumbnails(char *dirname, int size);
void Bitmap_HeightMapToNormalMap(bitmap_t *src, bitmap_t *dst, vec3_t scale);
void Bitmap_SetPixel4b(bitmap_t *bmp, int x, int y, byte r, byte g, byte b, byte a);
void Bitmap_Alloc(bitmap_t *bmp, int width, int height, int type);
bool Bitmap_DetermineTranslucency(bitmap_t *bmp);
void Bitmap_Init();
