/*
 * Chat box
 */

typedef struct
{
	gui_scrollbuffer_t			*parent;

	bool						audible;

	int							type;

	char					**parties;
	int					num_parties;

	gui_element_t			*element;
} gui_chatbox_t;

void	GUI_Chatbox_Init();
void	*GUI_Chatbox_Create(gui_element_t *obj, int argc, char *argv[]);
void	GUI_Chatbox_Msg(char *name, char *message);
