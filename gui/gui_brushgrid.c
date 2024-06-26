// (C) 2003 S2 Games

// le_brushgrid.c

// brush panel routines

#include "../le/le.h"
#include "../gui/gui_brushgrid.h"

static char *class_name = "brushgrid";

void	GUI_Brushgrid_SetDimensions(gui_element_t *obj)
{
	gui_brushgrid_t *brushgrid;

	brushgrid = corec.GUI_GetUserdata(class_name, obj);
}

void	GUI_Brushgrid_MouseOver(gui_element_t *obj, int x, int y)
{
	int col, row, n;
	gui_brushgrid_t *brushgrid;

	brushgrid = corec.GUI_GetUserdata(class_name, obj);
	if (!brushgrid)
		return;

	row = y / brushgrid->brush_display_size;
	col = x / brushgrid->brush_display_size;

	n = row * brushgrid->cols + col;

	brushgrid->selection = n;
}

void	GUI_Brushgrid_MouseDown(gui_element_t *obj, mouse_button_enum button, int x, int y)
{
	GUI_Brushgrid_MouseOver(obj, x, y);
}

void	GUI_Brushgrid_MouseUp(gui_element_t *obj, mouse_button_enum button, int x, int y)
{
	int col, row, n;
	gui_brushgrid_t *brushgrid;

	brushgrid = corec.GUI_GetUserdata(class_name, obj);
	if (!brushgrid)
		return;

	row = y / brushgrid->brush_display_size;
	col = x / brushgrid->brush_display_size;

	n = row * brushgrid->cols + col;

	brushgrid->selection = n;
	corec.Cvar_SetValue("le_brush", n);

	corec.GUI_Hide(obj);
}

void	GUI_Brushgrid_Draw(gui_element_t *obj, int w, int h)
{
	gui_brushgrid_t *brushgrid;
	int x, y, n;

	brushgrid = corec.GUI_GetUserdata(class_name, obj);
	if (!brushgrid)
		return;

	corec.GUI_SetRGB(0, 0, 0);

	corec.GUI_ShadowQuad2d_S(0, 0, w, h, res.white);

	n = 0;

	corec.GUI_SetRGB(1, 1, 1);

	for (y = 0; y < brushgrid->rows; y++)
		for (x = 0; x < brushgrid->cols; x++)
		{			
			if (n == brushgrid->selection 
				|| (brushgrid->selection == -1 && n == LE_CurrentBrush()))
			{
				corec.GUI_Quad2d_S(x * brushgrid->brush_display_size, y * brushgrid->brush_display_size, 
					brushgrid->brush_display_size, brushgrid->brush_display_size, res.white);
				corec.GUI_Quad2d_S(x * brushgrid->brush_display_size+2, y * brushgrid->brush_display_size+2, 
					brushgrid->brush_display_size-4, brushgrid->brush_display_size-4, res.brushShaders[n]);
			}
			else
			{
				corec.GUI_Quad2d_S(x * brushgrid->brush_display_size, y * brushgrid->brush_display_size, 
					brushgrid->brush_display_size, brushgrid->brush_display_size, res.brushShaders[n]);
			}
			n++;
			if (n>=MAX_BRUSHES) return;
		}
		
}

void	GUI_BrushGrid_MouseOver(gui_element_t *obj, int x, int y)
{
	gui_brushgrid_t *brushgrid;
	int r, c;

	brushgrid = corec.GUI_GetUserdata(class_name, obj);
	if (!brushgrid)
		return;

	c = (float)x / brushgrid->brush_display_size;
	r = (float)y / brushgrid->brush_display_size;

	if (c > brushgrid->cols-1 || c < 0) 
	{
		brushgrid->selection = -1;
		return;
	}
	if (r > brushgrid->rows-1 || r < 0) 
	{
		brushgrid->selection = -1;
		return;
	}

	brushgrid->selection = GRID(c, r, brushgrid->cols);
}

void	GUI_BrushGrid_MouseDown(gui_element_t *obj, int x, int y)
{
	gui_brushgrid_t *brushgrid;

	brushgrid = corec.GUI_GetUserdata(class_name, obj);
	if (!brushgrid)
		return;

	brushgrid->selection = LE_CurrentBrush();
}

void	GUI_BrushGrid_MouseUp(gui_element_t *obj, int x, int y)
{
	gui_brushgrid_t *brushgrid;

	brushgrid = corec.GUI_GetUserdata(class_name, obj);
	if (!brushgrid)
		return;

	corec.GUI_Hide(obj);
	if (brushgrid->selection >= 0)
	{
		corec.Cvar_SetValue("le_brush", brushgrid->selection);
	}
}

//brushgrid "name" x y columns
void	*GUI_Brushgrid_Create(gui_element_t *obj, int argc, char *argv[])
{
	gui_brushgrid_t *brushgrid;
	char s[256];
	int n;

	if (argc < 5)
	{
		corec.Console_Printf("syntax: create %s name x y columns brush_displaysize\n", class_name);
		return NULL;
	}

	corec.GUI_SetClass(obj, class_name);
	corec.GUI_SetName(obj, argv[0]);
	corec.GUI_Move (obj, atoi(argv[1]), atoi(argv[2]) );

	brushgrid = corec.GUI_Malloc(sizeof (gui_brushgrid_t));

	if (!brushgrid)
	{
		corec.Console_Printf("Brushgrid error: couldn't enough space to hold brushgrid\n");
		return NULL; 		
	}

	corec.GUI_SetUserdata(class_name, obj, brushgrid);
	brushgrid->element = obj;

	brushgrid->cols = atoi(argv[3]);
	brushgrid->brush_display_size = atoi(argv[4]);
	brushgrid->rows = MAX_BRUSHES / brushgrid->cols;

	brushgrid->element->draw = GUI_Brushgrid_Draw;
	brushgrid->element->mousedown = GUI_Brushgrid_MouseDown;
	brushgrid->element->mouseup = GUI_Brushgrid_MouseUp;
	brushgrid->element->mouseover = GUI_Brushgrid_MouseOver;

	brushgrid->element->noclip = true;

	corec.GUI_Resize (obj, brushgrid->cols * brushgrid->brush_display_size, brushgrid->rows * brushgrid->brush_display_size);

	for (n=0; n<MAX_BRUSHES; n++)
	{
		BPrintf(s, 255, "/brushes/standard/brush%i.tga", n+1); //fixme: selectable themes (-:
		s[255] = 0;
		LE_SelectBrush(n);
		res.brushShaders[n] = corec.Res_LoadShaderEx(s, SHD_NO_MIPMAPS | SHD_FULL_QUALITY);
		LE_LoadBrushBmp(s);
	}

	brushgrid->parent = NULL;

	return brushgrid;
}

void	GUI_Brushgrid_List()
{
	corec.GUI_List_Cmd(1, &class_name);
}

void	GUI_Brushgrid_Cmd(int argc, char *argv[])
{
	if (!argc)
	{
		corec.Console_Printf("brushgrid <command> <args>\n");
		corec.Console_Printf("  commands:\n");
		corec.Console_Printf("    list\n");
		return;
	}

	if (strcmp(argv[0], "list") == 0)
	{
		GUI_Brushgrid_List();
	}
}

void	GUI_Brushgrid_Init()
{	
	corec.GUI_RegisterClass(class_name, GUI_Brushgrid_Create);

	corec.Cmd_Register("brushgrid", GUI_Brushgrid_Cmd);
	
	LE_SetBrushStrength(1);
	LE_SelectBrush(3);
}
