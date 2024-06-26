/*
 *  Sliders
 */

#define DIR_HORIZONTAL	"horizontal"
#define DIR_VERTICAL	"vertical"

typedef struct
{
	gui_element_t			*parent;
	float					low;
	float					hi;
	float					value;
	char					variable[64];
	char					*direction;
	char					*cmd;
	char					*onChanged_cmd;

	float					fillr;
	float					fillg;
	float					fillb;

	float					lastval;

	bool					showValueAsTime;

	bool					drawValue;

	bool					buttonPressed;

	residx_t				bgshader;
	residx_t				handleshader;
	residx_t				fillshader;

	gui_element_t			*element;
} gui_slider_t;

void GUI_Sliders_Init();
void GUI_Slider_Value(gui_element_t *obj, float value);
