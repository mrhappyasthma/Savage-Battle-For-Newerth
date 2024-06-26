/*
 * Maps
 */

typedef struct
{
	gui_element_t			*parent;

	residx_t				player_shader; //the icon for the current player

	residx_t				friend_shader;
	residx_t				friend_player_shader;
	residx_t				friend_seige_shader;
	residx_t				friend_officer_shader;
	residx_t				enemy_shader;
	residx_t				enemy_seige_shader;
	residx_t				enemy_officer_shader;
	residx_t				other_shader;
	residx_t				spawnpoint_shader;
	residx_t				commandcenter_shader;
	residx_t				garrison_shader;
	residx_t				waypoint_shader;
	residx_t				underattack_shader;
	residx_t				buildingcomplete_shader;

	char					bg_shader_cvar[64];
	
	vec2_t					wc_cell_size; //cache of the size of this cell, in world coords
	
	bool					spawnselect;
	clientObject_t			*selectedSpawn;
	
	gui_element_t			*element;
} gui_playermap_t;

void	GUI_PlayerMap_Init();
void	*GUI_PlayerMap_Create(gui_element_t *obj, int argc, char *argv[]);
void	GUI_PlayerMap_Cmd(int argc, char *argv[]);
void 	GUI_PlayerMap_Param_Cmd(gui_element_t *obj, int argc, char *argv[]);
