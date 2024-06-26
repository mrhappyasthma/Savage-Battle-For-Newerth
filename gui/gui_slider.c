
#include "../toplayer/tl_shared.h"
#include "gui_slider.h"

static char *class_name = "slider";

residx_t sliderbgShader, sliderhandleShader, sliderfillShader, 
		 hsliderbgShader, hsliderhandleShader, hsliderfillShader;

void	GUI_Slider_Draw(gui_element_t *obj, int blah1, int blah2)
{
	gui_slider_t *slider;
	char s[100];	
	float value, nrange;
	int label_shift;
	int x, y, w, h;
	float low, hi;
	
	slider = corec.GUI_GetUserdata(class_name, obj);

	if (!slider)
	{
		corec.Console_Printf("Error: couldn't get userdata for slider %s\n", obj->name);
		return;
	}

	x = 0;
	y = 0;
	w = obj->width;
	h = obj->height;
	hi = MAX(slider->low, slider->hi);
	low = MIN(slider->low, slider->hi);
	nrange = (float)1 / ABS(hi - low);

	if (slider->variable[0])
		value = ABS(corec.Cvar_GetValue(slider->variable) - slider->low);
	else
		value = ABS(slider->value - low);

	if (slider->value < low)
		slider->value = low;
	if (slider->value > hi)
		slider->value = hi;

	corec.GUI_SetRGBA(1, 1, 1, obj->alpha);
	
	if (strcmp(slider->direction, DIR_HORIZONTAL) == 0) 
	{
		if (slider->bgshader > 0)
			corec.GUI_ShadowQuad2d_S(x, y, w, h, slider->bgshader);
	
		if (slider->fillshader > 0)
		{
			//corec.GUI_SetRGB((value*nrange)*slider->fillr, (value*nrange)*slider->fillg, (value*nrange)*slider->fillb);
			corec.GUI_SetRGBA(slider->fillr, slider->fillg, slider->fillb, obj->alpha);
	
			corec.GUI_Quad2d_S(x, y, ((value*nrange)*w)+1, h, slider->fillshader);
		}

		if (slider->handleshader > 0)
		{
			corec.GUI_SetRGBA(1, 1, 1, obj->alpha);

			corec.GUI_Quad2d_S(x+(value*nrange)*w-4, y, 8, h, slider->handleshader);
		}
	
		if (slider->drawValue)
		{
			if (slider->showValueAsTime)
			{
				int hour, min;
				hour = (int)((value + low)/60) % 12;
				if (hour == 0)
					hour = 12;
				min = ((int)(value + low)) % 60;

				BPrintf(s, 99, "%i:%02i%s", hour, min, value + low > 720 ? "pm" : "am");
				s[99] = 0;
			}
			else
			{
				BPrintf(s, 99, "%1.2f", value + low);
				s[99] = 0;
			}
	
			//if (value + low < 0)
			//	label_shift = -2;
			//else
				label_shift = 2;
		
			corec.GUI_SetRGBA(obj->textcolor[0], obj->textcolor[1], obj->textcolor[2], obj->alpha);
	
			corec.GUI_DrawString(x+w-corec.GUI_StringWidth(s, obj->char_height, obj->char_height, 1, 1024, corec.GetNicefontShader()), 
						y+label_shift, s, obj->char_height, obj->char_height, slider->element->height / obj->char_height, slider->element->width, corec.GetNicefontShader());
		}
	} 
	else 
	{
		if (slider->bgshader > 0)
			corec.GUI_ShadowQuad2d_S(x, y, w, h, slider->bgshader);
	
		if (slider->fillshader > 0)
		{
			//corec.GUI_SetRGB((value*nrange)*slider->fillr, (value*nrange)*slider->fillg, (value*nrange)*slider->fillb);
			corec.GUI_SetRGBA(slider->fillr, slider->fillg, slider->fillb, obj->alpha);
	
			corec.GUI_Quad2d_S(x, y+(1-value*nrange)*h, w, h-((1-value*nrange)*h)+1, slider->fillshader);
		}

		if (slider->handleshader > 0)
		{
			corec.GUI_SetRGBA(1, 1, 1, obj->alpha);

			corec.GUI_Quad2d_S(x, y+(1-value*nrange)*h-4, w, 8, slider->handleshader);
		}
	
		if (slider->drawValue)
		{
			BPrintf(s, 99, "%1.2f", value + low);
			s[99] = 0;
	
			if (value + low < 0)
				label_shift = -2;
			else
				label_shift = 4;
		
			corec.GUI_SetRGBA(1,1,1, obj->alpha);
	
			corec.GUI_DrawString(x+label_shift, y+h-14, s, obj->char_height, obj->char_height, slider->element->height / obj->char_height, slider->element->width, corec.GetNicefontShader());
		}
	} 
	
	//corec.GUI_SetRGB(1,1,1);
	//
	//corec.GUI_DrawString(x - strlen(generic_sliders[i].variable) * 4 + w/2, y-13+(h+14)*(i%2), generic_sliders[i].variable, 8, 10, strlen(generic_sliders[i].variable), GetNicefontShader());

	if (slider->lastval != value)
	{
		corec.GUI_Exec(slider->onChanged_cmd);

		slider->lastval = value;
	}
}

void	GUI_Slider_MouseDown(gui_element_t *obj, mouse_button_enum button, int x, int y)
{
	gui_slider_t *slider;

	slider = corec.GUI_GetUserdata(class_name, obj);
	if (!slider)
		return;

	slider->buttonPressed = true;
}

void	GUI_Slider_MouseUp(gui_element_t *obj, mouse_button_enum button, int x, int y)
{
	gui_slider_t *slider;

	slider = corec.GUI_GetUserdata(class_name, obj);
	if (!slider)
		return;

	slider->buttonPressed = false;
}

void	GUI_Slider_MouseOver(gui_element_t *obj, int x, int y)
{
	gui_slider_t *slider;
	float value, hi, low;

	slider = corec.GUI_GetUserdata(class_name, obj);
	if (!slider)
		return;

	if (slider->buttonPressed)
	{
		//get the percentage of the total this click represents
		if (strcmp(slider->direction, DIR_HORIZONTAL) == 0)
			value = (float)x / obj->width;
		else
			value = 1 - ((float)y / obj->height);

		
		//now take that to find the percentage of the range
		value = ((slider->hi - slider->low) * value);
		//now add the lowest value
		value += slider->low;

		low = MIN(slider->hi, slider->low);
		hi = MAX(slider->hi, slider->low);
		if (value < low)
			value = low;
		if (value > hi)
			value = hi;
		if (slider->variable[0])
			corec.GUI_Exec(fmt("set \"%s\" %f", slider->variable, value));	
	}
}

void	GUI_Slider_Move(gui_element_t *obj, int x, int y)
{
	corec.GUI_Move(obj, x, y);
}

void	GUI_Slider_Variable_Cmd(int argc, char *argv[])
{
	gui_slider_t *slider;

	if (argc < 2)
	{	
		corec.Console_Printf("syntax: slider variable <panel:object> <var_name>\n");
		return;
	}

	slider = corec.GUI_GetClass(argv[0], class_name);

	if (!slider)
		return;

	strncpySafe(slider->variable, argv[1], sizeof(slider->variable));
}


void	GUI_Slider_Range_Cmd(int argc, char *argv[])
{
	gui_slider_t *slider;

	if (argc < 2)
	{	
		corec.Console_Printf("syntax: slider range <panel:object> <value1> <value2>\n");
		return;
	}

	slider = corec.GUI_GetClass(argv[0], class_name);

	if (!slider)
		return;

	slider->low = atof(argv[1]);
	slider->hi = atof(argv[2]);
}

void GUI_Slider_Value(gui_element_t *obj, float value)
{
	gui_slider_t *slider;

	slider = corec.GUI_GetUserdata(class_name, obj);

	slider->value = value;
}

void GUI_Slider_Param_Cmd(gui_element_t *obj, int argc, char *argv[])
{
	gui_slider_t *slider;

	if (argc < 1)
	{
		corec.Console_Printf("slider param <panel:object> <var> <value>\n");
		corec.Console_Printf("valid parameters:\n");
		corec.Console_Printf("   variable <name>          - sets the variable to store the value in\n");
		corec.Console_Printf("   value <float>            - sets the value of the slider\n");
		corec.Console_Printf("   range <value1> <value2>        - sets the range of the slider (floats)\n");
		corec.Console_Printf("   slider_color <r> <g> <b> - sets the slider color\n");
		corec.Console_Printf("   bgshader <image>         - sets the slider background shader\n");
		corec.Console_Printf("   fillshader <image>       - sets the slider fill shader\n");
		corec.Console_Printf("   handleshader <image>     - sets the slider handle shader\n");
		corec.Console_Printf("   showvalue <false|true>   - turns off/on the value getting drawn\n");
		corec.Console_Printf("   showValueAsTime <false|true> - turns off/on showing the value as a time value\n");
		return;
	}

	slider = corec.GUI_GetUserdata(class_name, obj);

	if (!slider)
		return;

	if (strcmp(argv[0], "variable") == 0)
	{
		if (argc > 1)
		{
			strncpySafe(slider->variable, argv[1], sizeof(slider->variable));			
		} else {
			corec.Console_Printf("Not enough parameters.  You must specify the variable name.\n");
		}
	}
	else if (strcmp(argv[0], "value") == 0)
	{
		if (argc > 1)
			slider->value = atof(argv[1]);
		else
			corec.Console_Printf("Not enough paramters.  You must specify the value.\n");
	}
	else if (stricmp(argv[0], "showValueAsTime") == 0)
	{
		if (argc > 1)
			slider->showValueAsTime = atoi(argv[1]);
		else
			corec.Console_Printf("Not enough paramters.  You must specify the value.\n");
	}
	else if (strcmp(argv[0], "slider_color") == 0)
	{
		if (argc > 3)
		{
			slider->fillr = atof(argv[1]);
			slider->fillg = atof(argv[2]);
			slider->fillb = atof(argv[3]);
		} else {
			corec.Console_Printf("Not enough parameters.  You must specify the R, G, and B values (range [0..1])\n");
		}
	}
	else if (strcmp(argv[0], "range") == 0)
	{
		if (argc > 2)
		{
			slider->low = atof(argv[1]);
			slider->hi = atof(argv[2]);
		} else {
			corec.Console_Printf("Not enough parameters.  You must specify the range values.\n");
		}
	}
	else if (strcmp(argv[0], "bgshader") == 0)
	{
		if (argc > 1)
		{
			if (strcmp(argv[1], "none") == 0)
				slider->bgshader = 0;
			else
				slider->bgshader = corec.Res_LoadShaderEx(argv[1], SHD_NO_MIPMAPS | SHD_FULL_QUALITY);
		} else {
			corec.Console_Printf("Not enough parameters.  You must specify the image.\n");
		}
	}
	else if (strcmp(argv[0], "handleshader") == 0)
	{
		if (argc > 1)
		{
			if (strcmp(argv[1], "none") == 0)
				slider->handleshader = 0;
			else
				slider->handleshader = corec.Res_LoadShaderEx(argv[1], SHD_NO_MIPMAPS | SHD_FULL_QUALITY);
		} else {
			corec.Console_Printf("Not enough parameters.  You must specify the image.\n");
		}
	}
	else if (strcmp(argv[0], "fillshader") == 0)
	{
		if (argc > 1)
		{
			if (strcmp(argv[1], "none") == 0)
				slider->fillshader = 0;
			else
				slider->fillshader = corec.Res_LoadShaderEx(argv[1], SHD_NO_MIPMAPS | SHD_FULL_QUALITY);
		} else {
			corec.Console_Printf("Not enough parameters.  You must specify the image.\n");
		}
	}
	else if (strcmp(argv[0], "showvalue") == 0)
	{
		if (argc > 1)
		{
			if (strcmp(argv[1], "true") == 0)
				slider->drawValue = 1;
			else
				slider->drawValue = 0;
		} else {
			corec.Console_Printf("Not enough parameters.  You must specify true or false.\n");
		}
	}
}

void	GUI_Slider_OnChanged_Cmd(int argc, char *argv[])
{
	gui_slider_t *slider;

	if (argc < 2)
	{
		corec.Console_Printf("syntax: slider on_changed <panel:object> cmd\n");
		return;
	}

	slider = corec.GUI_GetClass(argv[0], class_name);

	if (!slider)
		return;

	slider->onChanged_cmd = corec.GUI_StrDup(argv[1]);
}

void	GUI_Slider_Destroy(gui_element_t *obj)
{
	gui_slider_t *slider;

	slider = corec.GUI_GetUserdata(class_name, obj);
	if (!slider)
		return;

	if (slider->onChanged_cmd)
		corec.GUI_Free(slider->onChanged_cmd);
	corec.GUI_Free(slider->direction);
}

//slider create name x y w h variable lorange hirange direction
void	*GUI_Slider_Create(gui_element_t *obj, int argc, char *argv[])
{
	gui_slider_t *slider;

	if (argc < 5)
	{
		corec.Console_Printf("syntax: create slider name x y width height\n");
		return NULL;
	}

	corec.GUI_SetClass(obj, class_name);
	corec.GUI_SetName(obj, argv[0]);
	corec.GUI_Move (obj, atoi(argv[1]), atoi(argv[2]) );
	corec.GUI_Resize (obj, atoi(argv[3]), atoi(argv[4]) );

	slider = corec.GUI_Malloc(sizeof (gui_slider_t));

	if (!slider)
	{
		corec.Console_Printf("Slider error: couldn't enough space to hold slider\n");
		return NULL; 		
	}

	corec.GUI_SetUserdata(class_name, obj, slider);
	slider->element = obj;

	//if (argc > 10)
	//{
	//	slider->fillr = atof(argv[8]);
	//	slider->fillg = atof(argv[9]);
	//	slider->fillb = atof(argv[10]);
	//} else {
		slider->fillr = 0.8;
		slider->fillg = 0.8;
		slider->fillb = 0.8;
	//}
	
	if (argc > 5) {
		slider->direction = corec.GUI_StrDup(argv[5]);
		slider->bgshader = hsliderbgShader;
		slider->fillshader = hsliderfillShader;
		slider->handleshader = hsliderhandleShader;
	
	} else {
		// Vertical by default
		slider->direction = corec.GUI_StrDup(DIR_VERTICAL);
		
		slider->bgshader = sliderbgShader;
		slider->fillshader = sliderfillShader;
		slider->handleshader = sliderhandleShader;
	
	}
	slider->value = 0;
	slider->low = 0;
	slider->hi = 1;
	slider->drawValue = true;
	
	obj->destroy = GUI_Slider_Destroy;
	obj->draw = GUI_Slider_Draw;
	obj->move = GUI_Slider_Move;
	obj->mouseover = GUI_Slider_MouseOver;
	obj->mousedown = GUI_Slider_MouseDown;
	obj->mouseup = GUI_Slider_MouseUp;

	obj->param = GUI_Slider_Param_Cmd;
	
	slider->onChanged_cmd = NULL;

	slider->parent = NULL;

	return slider;
}

void	GUI_Slider_Cmd(int argc, char *argv[])
{
	if (!argc)
	{
		corec.Console_Printf("slider <command> <args>\n");
		corec.Console_Printf("  commands:\n");
		corec.Console_Printf("    list\n");
		corec.Console_Printf("    variable\n");
		corec.Console_Printf("    range\n");
		corec.Console_Printf("    direction\n");
		return;
	}

	if (strcmp(argv[0], "list") == 0)
	{
		corec.GUI_List_Cmd(1, &class_name);
	} else if (strcmp(argv[0], "variable") == 0)
	{
		GUI_Slider_Variable_Cmd(argc-1, &argv[1]);
	} else if (strcmp(argv[0], "range") == 0)
	{
		GUI_Slider_Range_Cmd(argc-1, &argv[1]);
	} else if (strcmp(argv[0], "on_changed") == 0)
	{
		GUI_Slider_OnChanged_Cmd(argc-1, &argv[1]);
	}
}

void	GUI_Sliders_Init()
{
	char *gui_basepath = corec.Cvar_GetString("gui_basepath");

	corec.Cmd_Register("slider", GUI_Slider_Cmd);

	corec.GUI_RegisterClass(class_name, GUI_Slider_Create);

	sliderbgShader = corec.Res_LoadShader(fmt("%s/sliderbg.tga", gui_basepath));
	sliderhandleShader = corec.Res_LoadShader(fmt("%s/sliderhandle.tga", gui_basepath));
	sliderfillShader = corec.Res_LoadShader(fmt("%s/sliderfill.tga", gui_basepath));

	hsliderbgShader = corec.Res_LoadShader(fmt("%s/hsliderbg.tga", gui_basepath));
	hsliderhandleShader = corec.Res_LoadShader(fmt("%s/hsliderhandle.tga", gui_basepath));
	hsliderfillShader = corec.Res_LoadShader(fmt("%s/hsliderfill.tga", gui_basepath));
}
