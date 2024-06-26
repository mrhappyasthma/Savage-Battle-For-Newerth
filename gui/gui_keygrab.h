/*
 * Keygrab widget
 */


typedef struct
{
	gui_element_t	*parent;
	
	int				border;

	gui_panel_t		*panel;	//automatically show/hide this panel when widget is activated/disabled
	
	char			exec_cmd[CMD_MAX_LENGTH];	//execute when a key is pressed
	char			abort_cmd[CMD_MAX_LENGTH];	//execute if the grab is aborted
	char			keyDownCmd[CMD_MAX_LENGTH];	//command to bind to the key when it is pressed
	char			keyUpCmd[CMD_MAX_LENGTH];	//command to bind to the key when it is released

	gui_element_t	*element;
} gui_keygrab_t;

void	GUI_Keygrab_Init();
bool	GUI_Keygrab_InputCallback(int key, char rawchar, bool down, void *userdata);
void	GUI_Keygrab_Activate(gui_element_t *obj);
void	GUI_Keygrab_Abort(gui_keygrab_t *keygrab);

