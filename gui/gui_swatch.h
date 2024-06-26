/*
 * Color Swatch
 */

typedef struct
{
	gui_element_t			*parent;
	
	float					r;
	float					g;
	float					b;
	float					a;

	char					*r_var;
	char					*g_var;
	char					*b_var;
	char					*a_var;

	gui_element_t			*element;
} gui_swatch_t;

void	GUI_Swatch_Init();
void	GUI_Swatch_Variables(gui_element_t *obj, char *r, char *g, char *b, char *a);

