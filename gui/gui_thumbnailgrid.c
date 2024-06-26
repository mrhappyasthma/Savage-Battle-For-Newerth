// (C) 2003 S2 Games

// le_brushpanel.c

// brush panel routines

#include "../toplayer/tl_shared.h"
#include "gui_thumbnailgrid.h"

static char *class_name = "thumbnailgrid";

void	GUI_Thumbnailgrid_MouseOver(gui_element_t *obj, int x, int y)
{
	gui_thumbnailgrid_t *thumbnailgrid;
	int r, c;

	thumbnailgrid = corec.GUI_GetUserdata(class_name, obj);
	if (!thumbnailgrid)
		return;

	if (!thumbnailgrid->grid_visible || thumbnailgrid->num_thumbnails <= 0)
		return;

	c = (float)x / thumbnailgrid->thumbnail_display_size;
	r = (float)y / thumbnailgrid->thumbnail_display_size;

	if (c > thumbnailgrid->cols-1 || c < 0) 
	{
		thumbnailgrid->selection = -1;
		return;
	}
	if (r > thumbnailgrid->rows-1 || r < 0) 
	{
		thumbnailgrid->selection = -1;
		return;
	}

	thumbnailgrid->selection = r * thumbnailgrid->cols + c;
}

void	GUI_Thumbnailgrid_MouseDown(gui_element_t *obj, mouse_button_enum button, int x, int y)
{
	gui_thumbnailgrid_t *thumbnailgrid;

	thumbnailgrid = corec.GUI_GetUserdata(class_name, obj);
	if (!thumbnailgrid)
		return;

	if (thumbnailgrid->num_thumbnails <= 0)
		return;

	thumbnailgrid->grid_visible = !thumbnailgrid->grid_visible;

	if (!thumbnailgrid->grid_visible)
	{
		if (thumbnailgrid->selection < 0 
			|| thumbnailgrid->selection >= thumbnailgrid->num_thumbnails)
		{
			thumbnailgrid->selection = 0;
		} 
		else 
		{
			corec.Cvar_Set(thumbnailgrid->cvar, thumbnailgrid->filenames[thumbnailgrid->selection]);

			if (thumbnailgrid->cmd)
				corec.GUI_Exec(thumbnailgrid->cmd);
		}

		corec.GUI_Move(obj, obj->bmax[0] - thumbnailgrid->thumbnail_display_size, obj->bmin[1]);
		corec.GUI_Resize(obj, thumbnailgrid->thumbnail_display_size, thumbnailgrid->thumbnail_display_size);
		
	} 
	else 
	{
		corec.GUI_Move(obj, obj->bmax[0] - thumbnailgrid->thumbnail_display_size * thumbnailgrid->cols, obj->bmin[1]);
		corec.GUI_Resize(obj, 
					thumbnailgrid->thumbnail_display_size * thumbnailgrid->cols, 
					thumbnailgrid->thumbnail_display_size * thumbnailgrid->rows);
	}
}

void	GUI_Thumbnailgrid_MouseUp(gui_element_t *obj, mouse_button_enum button, int x, int y)
{
	/*
	gui_thumbnailgrid_t *thumbnailgrid;

	thumbnailgrid = corec.GUI_GetUserdata(class_name, obj);
	*/
}

void	GUI_Thumbnailgrid_Draw(gui_element_t *obj, int w, int h)
{
	gui_thumbnailgrid_t *thumbnailgrid;
	int x, y, n;

	thumbnailgrid = corec.GUI_GetUserdata(class_name, obj);
	if (!thumbnailgrid)
		return;

	if (thumbnailgrid->num_thumbnails <= 0)
	{
		corec.GUI_SetRGB(0, 0, 0);

		corec.GUI_ShadowQuad2d_S(0, 0, w, h, corec.GetWhiteShader());
		return;
	}

	if (thumbnailgrid->grid_visible)
	{
		corec.GUI_SetRGB(0, 0, 0);

		corec.GUI_ShadowQuad2d_S(0, 0, obj->width, obj->height, corec.GetWhiteShader());

		n = 0;

		corec.GUI_SetRGB(1, 1, 1);

		for (y = 0; y < thumbnailgrid->rows; y++)
		{
			for (x = 0; x < thumbnailgrid->cols; x++)
			{		
				if (n >= thumbnailgrid->num_thumbnails)
					return;
				if (n == thumbnailgrid->selection)
				{
					corec.GUI_Quad2d_S(x * thumbnailgrid->thumbnail_display_size, 
								y * thumbnailgrid->thumbnail_display_size, 
								thumbnailgrid->thumbnail_display_size, 
								thumbnailgrid->thumbnail_display_size, 
								corec.GetWhiteShader());
					corec.GUI_Quad2d_S(x * thumbnailgrid->thumbnail_display_size+2, 
								y * thumbnailgrid->thumbnail_display_size+2, 
								thumbnailgrid->thumbnail_display_size-4, 
								thumbnailgrid->thumbnail_display_size-4, 
								thumbnailgrid->shaders[n]);
				}
				else
				{
					corec.GUI_Quad2d_S(x * thumbnailgrid->thumbnail_display_size, 
								y * thumbnailgrid->thumbnail_display_size, 
								thumbnailgrid->thumbnail_display_size, 
								thumbnailgrid->thumbnail_display_size, 
								thumbnailgrid->shaders[n]);
				}
				n++;
			}
		}
	} else {
		if (thumbnailgrid->selection < 0 || thumbnailgrid->selection >= thumbnailgrid->num_thumbnails)
			thumbnailgrid->selection = 0;
		corec.GUI_SetRGB(0, 0, 0);

		corec.GUI_ShadowQuad2d_S(0, 0, w, h, corec.GetWhiteShader());

		corec.GUI_SetRGB(1, 1, 1);

		corec.GUI_Quad2d_S(obj->width - thumbnailgrid->thumbnail_display_size, 
					0, 
					thumbnailgrid->thumbnail_display_size, 
					thumbnailgrid->thumbnail_display_size, 
					thumbnailgrid->shaders[thumbnailgrid->selection]);
	}
}



void	GUI_Thumbnailgrid_AddFile(const char *filename, void *data)
{
	gui_thumbnailgrid_path_t *thumbnailgrid_path = data;
	gui_thumbnailgrid_t *thumbnailgrid;
	char s[256];
	char tmp[256];
	char thumb[256];
	residx_t shader;

	thumbnailgrid = corec.GUI_GetUserdata(class_name, thumbnailgrid_path->obj);
	if (!thumbnailgrid)
		return;
	
	if (thumbnailgrid->num_thumbnails+1 >= thumbnailgrid->alloced_shaders)
	{
		thumbnailgrid->alloced_shaders += 10;
		thumbnailgrid->shaders = corec.GUI_ReAlloc(thumbnailgrid->shaders, sizeof(residx_t) * (thumbnailgrid->alloced_shaders));
		thumbnailgrid->filenames = corec.GUI_ReAlloc(thumbnailgrid->filenames, sizeof(char *) * (thumbnailgrid->alloced_shaders));
	}
	if (thumbnailgrid_path->dir[strlen(thumbnailgrid_path->dir)-1] != '/')
	    BPrintf(s, 255, "%s/", thumbnailgrid_path->dir);
	else
		strncpy(s, thumbnailgrid_path->dir, 255);
	
	BPrintf(thumb, 255, "%sthumbnails/%s.thumb", s, filename);
	BPrintf(tmp, 255, "%s%s", s, filename);
	
	shader = corec.Res_LoadShader(thumb);
/*	if (!shader)
		shader = corec.Res_LoadShader(s);	//try loading the actual shader
*/
	if (!shader)
		return;

	thumbnailgrid->shaders[thumbnailgrid->num_thumbnails] = shader;
	thumbnailgrid->filenames[thumbnailgrid->num_thumbnails] = corec.GUI_StrDup(tmp);
	thumbnailgrid->num_thumbnails++;

	thumbnailgrid->rows = thumbnailgrid->num_thumbnails / thumbnailgrid->cols + (thumbnailgrid->num_thumbnails % thumbnailgrid->cols > 0);
	corec.GUI_Resize (thumbnailgrid_path->obj, thumbnailgrid_path->obj->width, thumbnailgrid->rows * thumbnailgrid->thumbnail_display_size);
}

void	GUI_Thumbnailgrid_AddDirectory(gui_element_t *obj, char *dir)
{
	gui_thumbnailgrid_path_t thumbnailgrid_path;

	//generate thumbnails if they don't already exist
	corec.Bitmap_GenerateThumbnails(dir, 32);

	thumbnailgrid_path.obj = obj;
	thumbnailgrid_path.dir = dir;
	corec.System_Dir(dir, "*.*", false, NULL, GUI_Thumbnailgrid_AddFile, &thumbnailgrid_path);
}

char	*GUI_Thumbnailgrid_GetValue(gui_element_t *obj)
{
	gui_thumbnailgrid_t *tng = corec.GUI_GetUserdata(class_name, obj);
	if (!tng)
		return "";

	if (tng->selection >= tng->num_thumbnails || tng->selection < 0)
		return "";

	return tng->filenames[tng->selection];
}

void	GUI_Thumbnailgrid_Destroy(gui_element_t *obj)
{
	gui_thumbnailgrid_t *thumbnailgrid;
	int i;
	
	thumbnailgrid = corec.GUI_GetUserdata(class_name, obj);

	if (!thumbnailgrid)
		return;
	
	corec.GUI_Free(thumbnailgrid->cmd);
	corec.GUI_Free(thumbnailgrid->cvar);
	
	for (i= 0; i < thumbnailgrid->num_thumbnails; i++)
		corec.GUI_Free(thumbnailgrid->filenames[i]);
	corec.GUI_Free(thumbnailgrid->filenames);
	corec.GUI_Free(thumbnailgrid->shaders);
}

//thumbnailgrid "name" x y columns
void	*GUI_Thumbnailgrid_Create(gui_element_t *obj, int argc, char *argv[])
{
	gui_thumbnailgrid_t *thumbnailgrid;

	if (argc < 6)
	{
		corec.Console_Printf("syntax: create %s name x y columns thumbnail_displaysize cvar_t cmd\n", class_name);
		return NULL;
	}

	corec.GUI_SetClass(obj, class_name);
	corec.GUI_SetName(obj, argv[0]);
	corec.GUI_Move (obj, atoi(argv[1]), atoi(argv[2]) );


	thumbnailgrid = corec.GUI_Malloc(sizeof (gui_thumbnailgrid_t));

	if (!thumbnailgrid)
	{
		corec.Console_Printf("Thumbnailgrid error: couldn't enough space to hold thumbnailgrid\n");
		return NULL;
	}

	corec.GUI_SetUserdata(class_name, obj, thumbnailgrid);
	thumbnailgrid->element = obj;

	thumbnailgrid->cols = atoi(argv[3]);
	thumbnailgrid->thumbnail_display_size = atoi(argv[4]);

	corec.GUI_Resize (obj, thumbnailgrid->thumbnail_display_size, thumbnailgrid->thumbnail_display_size );

	thumbnailgrid->element->destroy = GUI_Thumbnailgrid_Destroy;
	thumbnailgrid->element->draw = GUI_Thumbnailgrid_Draw;
	thumbnailgrid->element->mousedown = GUI_Thumbnailgrid_MouseDown;
	thumbnailgrid->element->mouseup = GUI_Thumbnailgrid_MouseUp;
	thumbnailgrid->element->mouseover = GUI_Thumbnailgrid_MouseOver;

	thumbnailgrid->element->getvalue = GUI_Thumbnailgrid_GetValue;

	strncpySafe(thumbnailgrid->cvar, argv[5], sizeof(thumbnailgrid->cvar));
	if (argv[6])
		thumbnailgrid->cmd = corec.GUI_StrDup(argv[6]);
	else
		thumbnailgrid->cmd = corec.GUI_StrDup("");

	thumbnailgrid->grid_visible = false;
	thumbnailgrid->num_thumbnails = 0;
	thumbnailgrid->shaders = NULL;
	thumbnailgrid->filenames = NULL;
	thumbnailgrid->alloced_shaders = 0;

	thumbnailgrid->element->noclip = true;

	thumbnailgrid->parent = NULL;

	return thumbnailgrid;
}

void	GUI_Thumbnailgrid_AddDirectory_Cmd(int argc, char *argv[])
{
	gui_element_t *obj;

	if (argc < 2)
	{	
		corec.Console_Printf("syntax: thumbnailgrid add_directory <panel:object> <directory>\n");
		return;
	}

	obj = corec.GUI_GetObject(argv[0]);

	if (!obj)
	{
		corec.Console_Printf("error, couldn't find object named %s\n", argv[0]);
		return;
	}

	if (strcmp(obj->class_name, class_name) != 0)
	{
		corec.Console_Printf("Class mismatch, %s is not a %s, it is a %s\n", argv[0], class_name, obj->class_name);
		return;	
	}

	GUI_Thumbnailgrid_AddDirectory(obj, argv[1]);
}

void	GUI_Thumbnailgrid_AddFile_Cmd(int argc, char *argv[])
{
	gui_thumbnailgrid_path_t thumbnailgrid_path;

	if (argc < 2)
	{	
		corec.Console_Printf("syntax: thumbnailgrid add_file <panel:object> <filename>\n");
		return;
	}

	thumbnailgrid_path.obj = corec.GUI_GetObject(argv[0]);
	thumbnailgrid_path.dir = "";

	if (!thumbnailgrid_path.obj)
	{
		corec.Console_Printf("error, couldn't find object named %s\n", argv[0]);
		return;
	}

	if (strcmp(thumbnailgrid_path.obj->class_name, class_name) != 0)
	{
		corec.Console_Printf("Class mismatch, %s is not a %s, it is a %s\n", argv[0], class_name, thumbnailgrid_path.obj->class_name);
		return;	
	}

	GUI_Thumbnailgrid_AddFile(argv[1], &thumbnailgrid_path);
}

void	GUI_Thumbnailgrid_List()
{
	corec.GUI_List_Cmd(1, &class_name);
}

void	GUI_Thumbnailgrid_Clear_Cmd(int argc, char *argv[])
{
	gui_thumbnailgrid_t *tng = corec.GUI_GetClass(argv[0], class_name);

	if (!tng)
		return;

	tng->num_thumbnails = 0;
}

void	GUI_Thumbnailgrid_Cmd(int argc, char *argv[])
{
	if (!argc)
	{
		corec.Console_Printf("thumbnailgrid <command> <args>\n");
		corec.Console_Printf("  commands:\n");
		corec.Console_Printf("    list\n");
		corec.Console_Printf("    add_directory\n");
		corec.Console_Printf("    add_file\n");
		corec.Console_Printf("    clear\n");
		return;
	}

	if (strcmp(argv[0], "list") == 0)
	{
		GUI_Thumbnailgrid_List();
	} else if (strcmp(argv[0], "add_directory") == 0)
	{
		GUI_Thumbnailgrid_AddDirectory_Cmd(argc-1, &argv[1]);
	} else if (strcmp(argv[0], "add_file") == 0)
	{
		GUI_Thumbnailgrid_AddFile_Cmd(argc-1, &argv[1]);
	} else if (strcmp(argv[0], "clear") == 0)
	{
		GUI_Thumbnailgrid_Clear_Cmd(argc-1, &argv[1]);
	}
}

void	GUI_Thumbnailgrid_Init()
{	
	corec.GUI_RegisterClass(class_name, GUI_Thumbnailgrid_Create);

	corec.Cmd_Register("thumbnailgrid", GUI_Thumbnailgrid_Cmd);
}
