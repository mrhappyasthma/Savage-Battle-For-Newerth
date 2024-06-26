// (C) 2003 S2 Games

// bink_w32.c

// Bink video playback

#ifdef _WIN32

#include "core.h"


#define MAX_BINK_HANDLES	256
typedef struct
{
	char	filename[_MAX_PATH];
	HBINK	handle;
	int		pitch, width, height;
	byte	*buffer;
	GLenum	type;
	int		numPlays;
}
bink_id;

bink_id	binkHandles[MAX_BINK_HANDLES];



/*==========================

  Bink_Stop

 ==========================*/

void	Bink_Stop(shader_t *shader)
{
	bink_id	*binkinfo;

	if (shader->movieid < 0 || shader->movieid >= MAX_BINK_HANDLES)
		return;

	binkinfo = &binkHandles[shader->movieid];

	BinkPause(binkinfo->handle, 1);
}


/*==========================

  Bink_Continue

 ==========================*/

void	Bink_Continue(shader_t *shader)
{
	bink_id	*binkinfo;

	if (shader->movieid < 0 || shader->movieid >= MAX_BINK_HANDLES)
		return;

	binkinfo = &binkHandles[shader->movieid];

	BinkPause(binkinfo->handle, 0);
}


/*==========================

  Bink_Restart

 ==========================*/

void	Bink_Restart(shader_t *shader)
{
	bink_id	*binkinfo;

	if (shader->movieid < 0 || shader->movieid >= MAX_BINK_HANDLES)
		return;

	binkinfo = &binkHandles[shader->movieid];

	BinkGoto(binkinfo->handle, 0, 0);
}


/*==========================

  Bink_GetFrame

 ==========================*/

void	Bink_GetFrame(shader_t *shader)
{
	bink_id	*binkinfo;

	if (shader->movieid < 0 || shader->movieid >= MAX_BINK_HANDLES)
		return;

	binkinfo = &binkHandles[shader->movieid];

	if (BinkWait(binkinfo->handle))
		return;

	BinkDoFrame(binkinfo->handle);

	BinkCopyToBuffer(binkinfo->handle, binkinfo->buffer, binkinfo->pitch, binkinfo->height, 0, 0, BINKSURFACE24R);

	if (binkinfo->handle->FrameNum == binkinfo->handle->Frames)
	{
		binkinfo->numPlays++;
		shader->numplays++;
	}

	BinkNextFrame(binkinfo->handle);

	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, binkinfo->width, binkinfo->height, 
		binkinfo->type, GL_UNSIGNED_BYTE, binkinfo->buffer);

	Console_DPrintf("Bink_GetFrame(): %i / %i\n", binkinfo->handle->FrameNum, binkinfo->handle->Frames);
}


/*==========================

  Bink_GetID

 ==========================*/

int		Bink_GetID(char *filename)
{
	int index;

	//check for an existing one
	for (index = 1; index < MAX_BINK_HANDLES; index++)
	{
		if (strncmp(binkHandles[index].filename, filename, strlen(filename)) == 0)
			return index;
	}

	//it's not loaded, find a free handle
	for (index = 1; index < MAX_BINK_HANDLES; index++)
	{
		if (!binkHandles[index].handle)
			break;
	}

	//this is either a free one or MAX_BINK_HANDLES
	return index;
}

int	nextPowerOfTwo(int x)
{
	int n;

	//check that it isn't already a power of two
	for (n = 0; n < 32; n++)
	{
		if (x == (1 << n))
			return x;
	}

	//find the highest bit
	n = 0;
	while (x)
	{
		x >>= 1;
		n++;
	}
	
	return (1 << n);
}

/*==========================

  Bink_Load

 ==========================*/

int		Bink_Load(const char *filename, bitmap_t *bitmap, bool preload)
{
	int		id;
	char	full_system_path[_MAX_PATH];

	File_FixPath(filename, full_system_path, true);
	id = Bink_GetID(full_system_path);

	if (id == MAX_BINK_HANDLES)
	{
		Console_DPrintf("Bink_Load() Could not load %s, too many in use\n", full_system_path);
		return -1;
	}

	binkHandles[id].handle = BinkOpen(full_system_path, BINKALPHA | (preload ? BINKPRELOADALL : 0));
	if (!binkHandles[id].handle)
	{
		Console_DPrintf("Bink_Load() - %s (%s)\n", BinkGetError(), full_system_path);
		return -1;
	}

	bitmap->bmptype = (binkHandles[id].handle->OpenFlags & BINKALPHA) ? BITMAP_RGBA : BITMAP_RGB;
	bitmap->mode = 8 * ((binkHandles[id].handle->OpenFlags & BINKALPHA) ? 4 : 3);
	bitmap->width = nextPowerOfTwo(binkHandles[id].handle->Width);
	bitmap->height = nextPowerOfTwo(binkHandles[id].handle->Height);
	bitmap->translucent = (binkHandles[id].handle->OpenFlags & BINKALPHA);
	bitmap->data = Tag_Malloc((bitmap->width * (bitmap->mode / 8)) * bitmap->height, MEM_BINK);

	strncpy(binkHandles[id].filename, full_system_path, _MAX_PATH);
	binkHandles[id].pitch =	bitmap->width * (bitmap->mode / 8);
	binkHandles[id].width = bitmap->width;
	binkHandles[id].height = bitmap->height;
	binkHandles[id].type = (binkHandles[id].handle->OpenFlags & BINKALPHA) ? GL_RGBA : GL_RGB;

	binkHandles[id].buffer = bitmap->data;
	return id;
}


/*==========================

  Bink_Unload

 ==========================*/

void	Bink_Unload(shader_t *shader)
{
	bink_id	*binkinfo;

	if (shader->movieid < 0 || shader->movieid >= MAX_BINK_HANDLES)
		return;

	binkinfo = &binkHandles[shader->movieid];

	Bink_Stop(shader);
	BinkSetSoundOnOff(binkinfo->handle, 0);
	BinkClose(binkinfo->handle);
	memset(binkinfo, 0, sizeof(bink_id));
}


/*==========================

  Bink_ShutDown

 ==========================*/

void	Bink_ShutDown()
{
	int index;

	for (index = 0; index < MAX_BINK_HANDLES; index++)
	{
		if (binkHandles[index].handle)
			BinkClose(binkHandles[index].handle);
	}
}


/*==========================

  Bink_Initialize

 ==========================*/

void	Bink_Initialize()
{
	memset(binkHandles, 0, sizeof(bink_id) * MAX_BINK_HANDLES);

	BinkSoundUseDirectSound(0);
}
#endif	//_WIN32
