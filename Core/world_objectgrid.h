
#define OBJECT_GRID_WIDTH 20
#define OBJECT_GRID_HEIGHT 20

#define WORLD_TO_GRID_X(x) ((x - grid->bmin[X]) / (grid->grid_width))
#define WORLD_TO_GRID_Y(y) ((y - grid->bmin[Y]) / (grid->grid_width))

typedef struct object_grid_node_s
{
	int	   	index;
	struct 	object_grid_node_s *next;
	struct 	object_grid_node_s *prev;
} object_grid_node_t;

typedef struct object_grid_s
{
	bool initialized;
	
	object_grid_node_t object_grid[OBJECT_GRID_WIDTH][OBJECT_GRID_HEIGHT];
	baseObject_t **objects;
	vec3_t bmin, bmax;
	float world_width, world_height;
	float grid_width;
} object_grid_t;

//clears all objects, recalculates the world bmin, bmax data
void    WOG_Reset(object_grid_t *grid, baseObject_t *objects[]);

//adds/updates the object passed in, as long as obj->pos != oldpos (so it won't get added if they match)
void    WOG_Add(object_grid_t *grid, int index);
bool    WOG_Remove(object_grid_t *grid, int index);

//returns the closest object of any of the types listed, within maxdist
int 	WOG_FindClosestObjects(object_grid_t *grid, int *objectTypes, int num_objectTypes, 
							   float min_dist, float max_dist, vec3_t pos, int team, float *closest_dist);
int 	WOG_FindObjectsInRadius(object_grid_t *grid, int *objectTypes, int num_objectTypes,
							    float radius, vec3_t pos, int *objects, int numObjects);

int 	WOG_Client_GetNextObjectInGridSquare(object_grid_t *grid, int *objectTypes, int num_objectTypes, int x, int y, int *offset);
bool    WOG_GetGridSquareCoords(object_grid_t *grid, int grid_x, int grid_y, vec2_t topleft, vec2_t bottomright);
bool    WOG_GetGridSquareForPos(object_grid_t *grid, vec3_t pos, ivec2_t gridpos);

bool	WOG_IsVisible(object_grid_t *grid, const vec2_t start, const vec2_t end, const vec3_t bmin, const vec3_t bmax);
