#include "../le/le.h"
#include "../gui/gui_brushbutton.h"

static char *class_name = "brushbutton";

void	GUI_BrushButton_Draw(gui_element_t *obj, int blah1, int blah2)
{
	int x, y, w, h;

	x = 0;
	y = 0;
	w = obj->width;
	h = obj->height;

	corec.GUI_SetRGBA(le_brushr.value, le_brushg.value, le_brushb.value, 0.7);

	corec.GUI_ShadowQuad2d_S(x, y, w, h, res.brushShaders[LE_CurrentBrush()]);
}


void	*GUI_BrushButton_Create(gui_element_t *obj, int argc, char *argv[])
{
	gui_button_t *parent;
	gui_brushbutton_t *brushbutton;

	if (argc < 5)
	{
		corec.Console_Printf("syntax: brushbutton name x y w h (be sure to set gui_button_down_cmd \"panel show brushgrid\" or similar cmd)\n");
		return NULL; 
	}

	corec.GUI_SetClass(obj, class_name);

	parent = GUI_Button_Create(obj, argc, argv);

	if (!parent)
	{
		corec.Console_Printf("Brushbutton error: couldn't instantiate button class.  See error above.\n");
		return NULL; 
	}
	brushbutton = corec.GUI_Malloc(sizeof (gui_brushbutton_t));

	if (!brushbutton)
	{
		corec.Console_Printf("Brushbutton error: couldn't get enough space to hold brushbutton\n");
		return NULL; 		
	}

	corec.GUI_SetUserdata(class_name, obj, brushbutton);

	brushbutton->parent = parent;

	obj->draw = GUI_BrushButton_Draw;

	return brushbutton;
}

void	GUI_BrushButton_List()
{
	corec.GUI_List_Cmd(1, &class_name);
}

void	GUI_BrushButton_Cmd(int argc, char *argv[])
{
	if (!argc)
	{
		corec.Console_Printf("brushbutton <command> <args>\n");
		corec.Console_Printf("  commands:\n");
		corec.Console_Printf("    list\n");
		return;
	}

	if (strcmp(argv[0], "list") == 0)
	{
		GUI_BrushButton_List();
	}
}

void	GUI_BrushButtons_Init()
{
	corec.GUI_RegisterClass(class_name, GUI_BrushButton_Create);

	corec.Cmd_Register("brushbutton", GUI_BrushButton_Cmd);
}

