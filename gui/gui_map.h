/*
 * Maps
 */

typedef struct
{
	gui_element_t	*parent;

	residx_t		fow_shader;
	
	residx_t		friend_shader;
	residx_t		friend_player_shader;
	residx_t		friend_seige_shader;
	residx_t		friend_officer_shader;

	residx_t		enemy_shader;
	residx_t		enemy_seige_shader;

	residx_t		other_shader;
	residx_t		blip_shader;

	char			bg_shader_cvar[64];
	
	vec2_t			wc_cell_size; //cache of the size of this cell, in world coords
	
	gui_element_t	*element;
} gui_map_t;

void	GUI_Map_Init();
void	*GUI_Map_Create(gui_element_t *obj, int argc, char *argv[]);
void	GUI_Map_Cmd(int argc, char *argv[]);
void 	GUI_Map_Param_Cmd(gui_element_t *obj, int argc, char *argv[]);
