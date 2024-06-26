/*
 * Textbox
 */


#define	GUI_TEXTBOX_BUFFER_SIZE		1024

enum
{
	INPUT_TYPE_FLOAT,
	INPUT_TYPE_INT,
	INPUT_TYPE_STRING,
	NUM_INPUT_TYPES
};

typedef struct
{
	gui_element_t			*parent;
	
	byte					type;
	
	int						max_length;

	int						border;
	int						cursor;
	int						pos;

    bool                    dropshadow;
	vec3_t                  dropshadowcolor;
	int                     dropshadowoffset;
	
	bool					hide_input;

	char					exec_cmd[CMD_MAX_LENGTH];
	char					abort_cmd[CMD_MAX_LENGTH];

	char					buffer[GUI_TEXTBOX_BUFFER_SIZE];
	char					variable[64];
	char					prefix[32];
	int						variableModifiedCount;

	bool					keepInput;

	gui_element_t			*element;
} gui_textbox_t;

void	GUI_Textbox_Init();
bool	GUI_Textbox_InputCallback(int key, unsigned char rawchar, bool down, void *userdata);
void	GUI_Textbox_Activate(gui_element_t *obj);
void	GUI_Textbox_Abort(gui_element_t *obj);

