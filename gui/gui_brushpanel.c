// (C) 2003 S2 Games

// gui_brushpanel.c

// brush panel routines

#include "savage.h"
#include "gui_brushpanel.h"
#include "le_tools.h"  //this is a LE-specific widget

static char *class_name = "brushpanel";

void	GUI_BPanel_SetDimensions(gui_element_t *obj)
{
	gui_brushgrid_t *brushpanel;

	brushpanel = core.GUI_GetUserdata(class_name, obj);
	if (!brushpanel)
		return;

	brushpanel->brushgrid_rows = MAX_BRUSHES / le_brushgrid_columns.value;

	bpanel.bmin[0] = le.screenw - le_bpanel_xoffset.value - le_brushdisplaysize.value;
	bpanel.bmin[1] = le_bpanel_yoffset.value;
	bpanel.bmax[0] = bpanel.bmin[0] + le_brushdisplaysize.value;
	bpanel.bmax[1] = bpanel.bmin[1] + le_brushdisplaysize.value;
	brushgrid.bmin[0] = le.screenw - le_brushgrid_xoffset.value - (le_brushdisplaysize.value * le_brushgrid_columns.value);
	brushgrid.bmin[1] = le_brushgrid_yoffset.value;
	brushgrid.bmax[0] = le.screenw - le_brushgrid_xoffset.value;
	brushgrid.bmax[1] = brushgrid.bmin[1] + (brushgrid_rows * le_brushdisplaysize.value);
}

void	GUI_BPanel_MouseOver(gui_element_t *obj, int x, int y)
{	
}

void	GUI_BPanel_MouseDown(gui_element_t *obj, mouse_button_enum button, int x, int y)
{
	//hand off focus to brush grid
	GUI_SetFocus(&brushgrid);
}

void	GUI_BPanel_MouseUp(gui_element_t *obj, mouse_button_enum button, int x, int y)
{
	brushgrid.visible = false;
}

void	GUI_BPanel_Draw(gui_element_t *obj, int w, int h)
{
	GUI_SetRGBA(le_brushr.value, le_brushg.value, le_brushb.value, 0.7);

	GUI_ShadowQuad2d_S(0, 0, w, h, res.brushShaders[GUI_CurrentBrush()]);
}

void	GUI_BPanel_Activate()
{
	bpanel.visible = true;
}

void	GUI_BPanel_Deactivate()
{
	bpanel.visible = false;
	brushgrid.visible = false;
}

void	GUI_BrushGrid_Draw(gui_element_t *obj, int w, int h)
{
	int x, y, n;

	GUI_SetRGB(0, 0, 0);

	GUI_ShadowQuad2d_S(0, 0, w, h, res.white);

	n = 0;

	GUI_SetRGB(1, 1, 1);

	for (y=0; y<brushgrid_rows; y++)
		for (x=0; x<le_brushgrid_columns.value; x++)
		{			
			if (n==brushgrid_selection || (brushgrid_selection==-1 && n==GUI_CurrentBrush()))
			{
				GUI_Quad2d_S(x * le_brushdisplaysize.value, y * le_brushdisplaysize.value, 
					le_brushdisplaysize.value, le_brushdisplaysize.value, res.white);
				GUI_Quad2d_S(x * le_brushdisplaysize.value+2, y * le_brushdisplaysize.value+2, 
					le_brushdisplaysize.value-4, le_brushdisplaysize.value-4, res.brushShaders[n]);
			}
			else
			{
				GUI_Quad2d_S(x * le_brushdisplaysize.value, y * le_brushdisplaysize.value, 
					le_brushdisplaysize.value, le_brushdisplaysize.value, res.brushShaders[n]);
			}
			n++;
			if (n>=MAX_BRUSHES) return;
		}
		
}

void	GUI_BrushGrid_MouseOver(gui_element_t *obj, int x, int y)
{
	int r, c;

	c = (float)x / le_brushdisplaysize.value;
	r = (float)y / le_brushdisplaysize.value;

	if (c > le_brushgrid_columns.value-1 || c < 0) 
	{
		brushgrid_selection = -1;
		return;
	}
	if (r > brushgrid_rows-1 || r < 0) 
	{
		brushgrid_selection = -1;
		return;
	}

	brushgrid_selection = GRID(c, r, le_brushgrid_columns.value);
}

void	GUI_BrushGrid_MouseDown(gui_element_t *obj, int x, int y)
{
	brushgrid_selection = GUI_CurrentBrush();
}

void	GUI_BrushGrid_MouseUp(gui_element_t *obj, int x, int y)
{
	brushgrid.visible = false;
	if (brushgrid_selection >= 0)
		core.Cvar_SetValue("le_brush", brushgrid_selection);
}

void	*GUI_BrushGrid_Create(gui_element_t *obj, int argc, char *argv[])
{
	gui_brushpanel_t *brushpanel;
	char filename[256];
	int n;

	if (argc < 5)
	{
		core.Console_Printf("syntax: create %s name x y w h [\"text\"]\n(be sure to first set:\n    gui_button_down image.tga\n    gui_button_down_cmd [command]\n)\n", class_name);
		return NULL;
	}

	GUI_SetClass(obj, class_name);
	GUI_SetName(obj, argv[0]);
	GUI_Move (obj, atoi(argv[1]), atoi(argv[2]) );
	GUI_Resize (obj, atoi(argv[3]), atoi(argv[4]) );

	brushpanel = GUI_MALLOC(sizeof (gui_brushpanel_t));

	if (!brushpanel)
	{
		core.Console_Printf("Brushpanel error: couldn't enough space to hold brushpanel\n");
		return NULL; 		
	}

	GUI_SetUserdata(class_name, obj, brushpanel);
	brushpanel->element = obj;

	brushpanel->brushgrid_selection = 0;

	for (n=0; n<MAX_BRUSHES; n++)
	{
		BPrintf(s, 255, "%sbrush%i.tga", UI_PREFIX, n+1);
		s[255] = 0;
		GUI_SelectBrush(n);
		res.brushShaders[n] = core.Res_LoadShaderEx(s, SHD_NO_MIPMAPS | SHD_FULL_QUALITY);
		GUI_LoadBrushBmp(s);
	}

	GUI_BPanel_SetDimensions(NULL);


	brushpanel->parent = NULL;
}

void	GUI_BPanel_Init()
{	
	char s[256];
	int n;

	


	bpanel.panel = NULL;
	bpanel.move = NULL;
	bpanel.draw = GUI_BPanel_Draw;
	bpanel.idle = GUI_BPanel_SetDimensions;
	bpanel.mouseout = NULL;
	bpanel.mouseover = GUI_BPanel_MouseOver;
	bpanel.mousedown = GUI_BPanel_MouseDown;
	bpanel.mouseup = GUI_BPanel_MouseUp;

	bpanel.interactive = true;
	
	brushgrid.panel = NULL;
	brushgrid.idle = NULL;
	brushgrid.move = NULL;
	brushgrid.draw = GUI_BrushGrid_Draw;
	brushgrid.mouseout = NULL;
	brushgrid.mousedown = GUI_BrushGrid_MouseDown;
	brushgrid.mouseup = GUI_BrushGrid_MouseUp;
	brushgrid.mouseover = GUI_BrushGrid_MouseOver;
	brushgrid.interactive = true;

	//GUI_AddElement(&brushgrid);
	//GUI_AddElement(&bpanel);
	
	GUI_SetBrushStrength(1);
	GUI_SelectBrush(3);
}
