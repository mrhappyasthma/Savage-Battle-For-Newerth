/*
 * HUDs
 */

typedef struct
{
	gui_element_t			*parent;

	residx_t				friend_shader;
	residx_t				enemy_shader;
	residx_t				other_shader;
	
	ivec2_t					center;
	vec2_t					wc_view_size; //cache the size of this view of the world, in world coords
	vec2_t					wc_cell_size; //cache of the size of this cell, in world coords
	int						cell_width; //cache the cell width, in screen coords
	int						cell_height; //cache the cell height, in screen coords
	
	int						num_cells_across;
	int						num_cells_down;

	gui_element_t			*element;
} gui_hud_t;

void	GUI_HUD_Init();
void	*GUI_HUD_Create(gui_element_t *obj, int argc, char *argv[]);
void	GUI_HUD_Cmd(int argc, char *argv[]);
void 	GUI_HUD_Param_Cmd(gui_element_t *obj, int argc, char *argv[]);
void    GUI_HUD_Center(gui_element_t *obj, vec3_t center);
