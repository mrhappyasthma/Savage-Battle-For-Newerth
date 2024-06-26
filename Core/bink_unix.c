// (C) 2003 S2 Games

// mpeg_unix.c

// Bink video playback

#include "core.h"

/*==========================

  Bink_GetFrame

 ==========================*/

void	Bink_GetFrame(shader_t *shader)
{
	/*
	mpeg_id	*mpeginfo;

	if (shader->mpegid < 0 || shader->mpegid >= MAX_MPEG_HANDLES)
		return;

	mpeginfo = &mpegHandles[shader->mpegid];

	if (BinkWait(mpeginfo->handle))
		return;

	BinkDoFrame(mpeginfo->handle);

	BinkCopyToBuffer(mpeginfo->handle, mpeginfo->buffer, mpeginfo->pitch, mpeginfo->height, 0, 0, MPEGSURFACE24R);
	BinkNextFrame(mpeginfo->handle);

	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mpeginfo->width, mpeginfo->height, 
		mpeginfo->type, GL_UNSIGNED_BYTE, mpeginfo->buffer);

	Console_DPrintf("Bink_GetFrame(): %i / %i\n", mpeginfo->handle->FrameNum, mpeginfo->handle->Frames);
	*/
}


/*==========================

  Bink_GetID

 ==========================*/

int		Bink_GetID(char *filename)
{
	int index = 0;

	/*
	//check for an existing one
	for (index = 0; index < MAX_MPEG_HANDLES; index++)
	{
		if (strncmp(mpegHandles[index].filename, filename, strlen(filename)) == 0)
			return index;
	}

	//it's not loaded, find a free handle
	for (index = 0; index < MAX_MPEG_HANDLES; index++)
	{
		if (!mpegHandles[index].handle)
			break;
	}

	*/
	//this is either a free one or MAX_MPEG_HANDLES
	return index;
}

/*==========================

  Bink_Load

 ==========================*/

int		Bink_Load(const char *filename, bitmap_t *bitmap, bool preload)
{
	return -1;
	/*
	int		id;
	char	full_system_path[_MAX_PATH];

	File_FixPath(filename, full_system_path, true);
	id = Bink_GetID(full_system_path);

	if (id == MAX_MPEG_HANDLES)
	{
		Console_DPrintf("Bink_Load() Could not load %s, too many in use\n", full_system_path);
		return -1;
	}

	mpegHandles[id].handle = BinkOpen(full_system_path, MPEGALPHA | MPEGPRELOADALL);
	if (!mpegHandles[id].handle)
	{
		Console_DPrintf("Bink_Load() - %s (%s)\n", BinkGetError(), full_system_path);
		return -1;
	}

	bitmap->bmptype = (mpegHandles[id].handle->OpenFlags & MPEGALPHA) ? BITMAP_RGBA : BITMAP_RGB;
	bitmap->mode = 8 * ((mpegHandles[id].handle->OpenFlags & MPEGALPHA) ? 4 : 3);
	bitmap->width = nextPowerOfTwo(mpegHandles[id].handle->Width);
	bitmap->height = nextPowerOfTwo(mpegHandles[id].handle->Height);
	bitmap->translucent = (mpegHandles[id].handle->OpenFlags & MPEGALPHA);
	bitmap->data = Tag_Malloc((bitmap->width * (bitmap->mode / 8)) * bitmap->height, MEM_MPEG);

	strncpy(mpegHandles[id].filename, full_system_path, _MAX_PATH);
	mpegHandles[id].pitch =	bitmap->width * (bitmap->mode / 8);
	mpegHandles[id].width = bitmap->width;
	mpegHandles[id].height = bitmap->height;
	mpegHandles[id].type = (mpegHandles[id].handle->OpenFlags & MPEGALPHA) ? GL_RGBA : GL_RGB;

	mpegHandles[id].buffer = bitmap->data;
	return id;
	*/
}


/*==========================

  Bink_Shutdown

 ==========================*/

void	Bink_Shutdown()
{
	/*
	for (index = 0; index < MAX_MPEG_HANDLES; index++)
	{
		if (mpegHandles[index].handle)
			BinkClose(mpegHandles[index].handle);
	}
	*/
}

void    Bink_Unload(shader_t *shader)
{
}

void    Bink_Stop(shader_t *shader)
{
}

void    Bink_Restart(shader_t *shader)
{
}

void    Bink_Continue(shader_t *shader)
{
}

/*==========================

  Bink_Initialize

 ==========================*/

void	Bink_Initialize()
{
}
