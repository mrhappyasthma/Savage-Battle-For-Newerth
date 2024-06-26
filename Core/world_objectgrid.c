/*
 * (C) 2002 S2 Games
 */

#include "core.h"

static object_grid_node_t *free_nodes = NULL;

void	WOG_Reset(object_grid_t *grid, baseObject_t *objects[])
{
	int i, j;
	float maxsize;
	object_grid_node_t *node;

	//free_nodes will either be null, or freed by World_Destroy()
	free_nodes = Tag_Malloc(sizeof(object_grid_node_t), MEM_WORLD);
	LIST_CLEAR(free_nodes);
	
	for (i = 0; i < OBJECT_GRID_WIDTH; i++)
	{
		for (j = 0; j < OBJECT_GRID_HEIGHT; j++)
		{
			node = &grid->object_grid[i][j];
			LIST_CLEAR(node);
			node->index = -1;
		}
	}

	grid->objects = objects;
	
	World_GetBounds(grid->bmin, grid->bmax);
	grid->world_width 	= grid->bmax[X] - grid->bmin[X];
	grid->world_height 	= grid->bmax[Y] - grid->bmin[Y];
	maxsize 			= MAX(grid->world_width, grid->world_height);
	grid->grid_width 	= maxsize / OBJECT_GRID_WIDTH;
	grid->initialized = true;
}

bool	WOG_GetGridSquareForPos(object_grid_t *grid, vec3_t pos, ivec2_t gridpos)
{
	if (pos[X] >= grid->bmin[X] && pos[Y] >= grid->bmin[Y]
		&& pos[X] <= grid->bmax[X]
		&& pos[Y] <= grid->bmax[Y])
	{
		FloatToInt(&gridpos[X], WORLD_TO_GRID_X(pos[X]));
		FloatToInt(&gridpos[Y], WORLD_TO_GRID_Y(pos[Y]));
		return true;
	}
	return false;
}

bool	WOG_GetGridSquareCoords(object_grid_t *grid, int grid_x, int grid_y, vec2_t topleft, vec2_t bottomright)
{
	if (grid_x >= 0 && grid_y >= 0
		&& grid_x < OBJECT_GRID_WIDTH
		&& grid_y < OBJECT_GRID_WIDTH)
	{
		topleft[X] = grid->bmin[X] + (grid->grid_width * grid_x);
		topleft[Y] = grid->bmin[Y] + (grid->grid_width * grid_y);

		bottomright[X] = topleft[X] + grid->grid_width;
		bottomright[Y] = topleft[Y] + grid->grid_width;
		return true;
	}
	return false;
}

/*
 *  If it returns NULL, it means this object doesn't currently exist at this position
 */
object_grid_node_t	*WOG_Find(object_grid_t *grid, int index, int grid_x, int grid_y)
{
	object_grid_node_t *tmp;
	object_grid_node_t *head;
	
	if (grid_x < 0 || grid_x >= OBJECT_GRID_WIDTH
		|| grid_y < 0 || grid_y >= OBJECT_GRID_HEIGHT)
		return NULL;
	
	head = &grid->object_grid[grid_x][grid_y];
	tmp = head->next;
	while (tmp != head)
	{
		if (tmp->index == index)
			return tmp;
		tmp = tmp->next;
	}
	return NULL;
}

void	WOG_AddToGridSquare(object_grid_t *grid, int index, int grid_x, int grid_y)
{
	object_grid_node_t *node;

	if (!grid
		|| !grid->initialized)
		return;
	
	if (LIST_EMPTY(free_nodes))
		node = Tag_Malloc(sizeof(object_grid_node_t), MEM_WORLD);
	else
	{
		node = free_nodes->next;
		LIST_REMOVE(node);
	}
	node->index = index;
	LIST_APPEND((&grid->object_grid[grid_x][grid_y]), node);
}

bool	WOG_Remove(object_grid_t *grid, int index)
{
	bool foundone = false;
	int grid_x, grid_y, bmin_x, bmax_x, bmin_y, bmax_y;
	object_grid_node_t *node;

	if (!grid
		|| !grid->initialized)
		return false;
	
	bmin_x = WORLD_TO_GRID_X(grid->objects[index]->pos[X] + grid->objects[index]->bmin[X]);
	bmax_x = WORLD_TO_GRID_X(grid->objects[index]->pos[X] + grid->objects[index]->bmax[X]);
	bmin_y = WORLD_TO_GRID_Y(grid->objects[index]->pos[Y] + grid->objects[index]->bmin[Y]);
	bmax_y = WORLD_TO_GRID_Y(grid->objects[index]->pos[Y] + grid->objects[index]->bmax[Y]);


	for (grid_x = bmin_x; grid_x <= bmax_x; grid_x++)
	{
		for (grid_y = bmin_y; grid_y <= bmax_y; grid_y++)
		{
			//did it used to exist?
			if ((node = WOG_Find(grid, index, grid_x, grid_y)))
			{
				//okay, remove the old entry
				LIST_REMOVE(node);
				LIST_APPEND(free_nodes, node);
				//Mem_Free(node);
				foundone = true;
			}
		}
	}
	return foundone;
}

void 	WOG_Add(object_grid_t *grid, int index)
{
	baseObject_t *obj;
	int grid_x, grid_y, bmin_x, bmax_x, bmin_y, bmax_y;

	if (!grid
		|| !grid->initialized)
		return;
	
	obj = grid->objects[index];
	if (!obj)
		return;

	bmin_x = WORLD_TO_GRID_X(grid->objects[index]->pos[X] + grid->objects[index]->bmin[X]);
	bmax_x = WORLD_TO_GRID_X(grid->objects[index]->pos[X] + grid->objects[index]->bmax[X]);
	bmin_y = WORLD_TO_GRID_Y(grid->objects[index]->pos[Y] + grid->objects[index]->bmin[Y]);
	bmax_y = WORLD_TO_GRID_Y(grid->objects[index]->pos[Y] + grid->objects[index]->bmax[Y]);

	for (grid_x = bmin_x; grid_x <= bmax_x; grid_x++)
	{
		for (grid_y = bmin_y; grid_y <= bmax_y; grid_y++)
		{
			WOG_AddToGridSquare(grid, index, grid_x, grid_y);
		}
	}
}
//pass in -1 for an objectType to match anything
//returns the next object of a particular type in a certain grid square
object_grid_node_t *WOG_GetNextObjectsInGridSquare(object_grid_t *grid, 
											 int *objectTypes, int num_objectTypes, 
											 object_grid_node_t *head, object_grid_node_t *node)
{
	int i;

	while (node != head)
	{
		for (i = 0; i < num_objectTypes; i++)
			if (objectTypes[i] == -1
				|| grid->objects[node->index]->type == objectTypes[i])
				return node;
		node = node->next;
	}
	return NULL;
}

int WOG_GetClosestObjectsInGridSquare(object_grid_t *grid, int *objectTypes, int num_objectTypes, 
									  vec3_t pos, int team, 
									  int grid_x, int grid_y, float minimum_dist, float *closest_dist)
{
	int closest = -1;
	float dist;
	object_grid_node_t *head = &grid->object_grid[grid_x][grid_y];
	object_grid_node_t *node;

	node = WOG_GetNextObjectsInGridSquare(grid, objectTypes, num_objectTypes, head, head->next);
	while (node)
	{
		if (team < 0 || grid->objects[node->index]->team == team)
		{
	 		dist = M_GetDistanceSq(pos, grid->objects[node->index]->pos);
		
			if (dist >= minimum_dist 
				&& dist < *closest_dist)
			{
				*closest_dist = dist;
				closest = node->index;
			}
		}
		if (node->next != head)
			node = WOG_GetNextObjectsInGridSquare(grid, objectTypes, num_objectTypes, head, node->next);
		else
			return closest;
	}
	return closest;
}

int	WOG_Client_GetNextObjectInGridSquare(object_grid_t *grid, int *objectTypes, int num_objectTypes, int x, int y, int *offset)
{
	object_grid_node_t *head;
	object_grid_node_t *node;
	int i = 0;	

	if (x < 0
		|| y < 0
		|| x >= OBJECT_GRID_WIDTH
		|| y >= OBJECT_GRID_HEIGHT)
		return -1;


	head = &grid->object_grid[x][y];
	node = head->next;

	if (*offset != -1)
	{
		for (i = 0 ; node != head && i < *offset; i++)
			node = node->next;
	}
	*offset = i;

	node = WOG_GetNextObjectsInGridSquare(grid, objectTypes, num_objectTypes, head, node);
	if (node)
		return node->index;
	else
		return -1;
}

/*
 * explanation of the algorithm:
 * start with the center of the grid.  If it has a match, go one more step and see if anything's closer.  
 * If not, you've found it.  
 *
 * Check in a series of hollow square box patterns.
 * 
 * +-+-+-+-+-+
 * |3|3|3|3|3|
 * +-+-+-+-+-+
 * |3|2|2|2|3|
 * +-+-+-+-+-+
 * |3|2|1|2|3|
 * +-+-+-+-+-+
 * |3|2|2|2|3|
 * +-+-+-+-+-+
 * |3|3|3|3|3|
 * +-+-+-+-+-+
 */

//finds the closest object in a single grid spot
//if we have found one that is closer than min_dist (the minimum distance found so far), we return true
bool 	WOG_FindClosestObjectsInGridSquare(object_grid_t *grid, 
										   int *objectTypes, int num_objectTypes, 
								   		   vec3_t pos, int team,
										   float min_dist,
								    	   float *closest_dist, int *closest,
										   int grid_x, int grid_y)
{
	int tmp_obj;
	float tmp_dist = FAR_AWAY;

	tmp_obj = WOG_GetClosestObjectsInGridSquare(grid, objectTypes, num_objectTypes, pos, team, grid_x, grid_y, min_dist, &tmp_dist);
	if (tmp_obj >= 0
		&& tmp_dist < *closest_dist)
	{
		*closest_dist = tmp_dist;
		*closest = tmp_obj;
		return true;
	}
	return false;
}

//returns the index of the closest object of any of the types in the objectTypes array
int WOG_FindClosestObjects(object_grid_t *grid, int *objectTypes, int num_objectTypes, 
						   float min_dist, float max_dist, vec3_t pos, int team, float *closest_dist)
{
	int grid_x, grid_y, level, i, max_level;
	int closest = -1;
	float maxdistsq, mindistsq;
	bool found_closest = false;
	int numgrids_width;
	int numgrids_height;
	int width;

	if (!grid
		|| !grid->initialized)
		return -1;
	
	//max dist is squared, so we need to take the square root to use it to measure actual distance
	maxdistsq = max_dist*max_dist;
	mindistsq = min_dist*min_dist;
	//calc the max number of grids out we'll have to check
	FloatToInt(&width, (1 + max_dist / grid->grid_width));
	numgrids_width = MIN(width, OBJECT_GRID_WIDTH);
	numgrids_height = MIN(width, OBJECT_GRID_HEIGHT);
	
	grid_x = WORLD_TO_GRID_X(pos[X]);
	grid_y = WORLD_TO_GRID_Y(pos[Y]);

	max_level = MAX(numgrids_width, numgrids_height);

	//dist acts as the current shortest distance, and so is also our "max dist" param for searching for new close objects
	*closest_dist = maxdistsq;
	if (max_level < 0)
	{
		Console_DPrintf("WOG Error, max_level is %i\n", max_level);
	}
	for (level = 0; level <= max_level; level++)
	{
		//width = level * 2 + 1;
		for (i = MAX(0, grid_x - level); i <= MIN(grid_x + level, OBJECT_GRID_WIDTH -1); i++)
		{
			if (grid_y - level >= 0
				&& grid_y - level < OBJECT_GRID_HEIGHT)
				//do the line at the top of the rect
				found_closest |= WOG_FindClosestObjectsInGridSquare(grid, objectTypes, num_objectTypes, 
																   pos, team, mindistsq, closest_dist, &closest, i, grid_y - level);
			if (grid_y + level >= 0
				&& grid_y + level < OBJECT_GRID_HEIGHT)
				//do the line at the bottom of the rect
				found_closest |= WOG_FindClosestObjectsInGridSquare(grid, objectTypes, num_objectTypes, 
															   pos, team, mindistsq, closest_dist, &closest, i, grid_y + level);
		}
		for (i = MAX(0, grid_y - level + 1); i <= MIN(grid_y + level, OBJECT_GRID_WIDTH -1); i++)
		{
			if (grid_x - level >= 0
				&& grid_x - level  < OBJECT_GRID_WIDTH)
				//do the line on the left, excluding endpoints
				found_closest |= WOG_FindClosestObjectsInGridSquare(grid, objectTypes, num_objectTypes, 
																   pos, team, mindistsq, closest_dist, &closest, grid_x - level, i);
			if (grid_x + level >= 0
				&& grid_x + level < OBJECT_GRID_WIDTH)
				//do the line on the right, excluding endpoints
				found_closest |= WOG_FindClosestObjectsInGridSquare(grid, objectTypes, num_objectTypes, 
																   pos, team, mindistsq, closest_dist, &closest, grid_x + level, i);
		}
		if (found_closest)
		{
			//look one more level out, and if we still haven't found one closer, we know this is the closest
			found_closest = false;
			if (max_level > level + 1)
				max_level = level + 1;
		}
	}
	return closest;
}

//return the number of objects added to the array
int 	WOG_FindObjectsInGridSquare(object_grid_t *grid, 
										   int *objectTypes, int num_objectTypes, 
								   		   vec3_t pos,
										   float max_distsq,
										   int *objects, int objIndex, int maxObjects,
										   int grid_x, int grid_y)
{
	vec3_t dir, out, bmin, bmax;
	float dist;
	int i, numObjects = 0;
	object_grid_node_t *head;
	object_grid_node_t *node;
	
	if (!grid)
	{
		System_Error("Grid is null!\n");
		return -1;
	}
	
	head = &grid->object_grid[grid_x][grid_y];
	
	node = WOG_GetNextObjectsInGridSquare(grid, objectTypes, num_objectTypes, head, head->next);
	while (node && numObjects < maxObjects)
	{
		M_SubVec3(grid->objects[node->index]->pos, pos, dir);
		M_Normalize(dir);
		M_AddVec3(grid->objects[node->index]->pos, grid->objects[node->index]->bmin, bmin);
		M_AddVec3(grid->objects[node->index]->pos, grid->objects[node->index]->bmax, bmax);
		if (M_RayBoxIntersect(pos, dir, bmin, bmax, out))
		{
			dist = M_GetDistanceSq(pos, out);
			if (dist <= max_distsq)
			{
				//ugh, there goes efficiency, we need to make sure this object wasn't already added
				// (since objects can be in more than one grid square)
				i = 0;
				while (i <= objIndex
					   && objects[i] != node->index)
					i++;
				if (i >= objIndex)
				{
					objects[objIndex++] = node->index;
					numObjects++;
				}
			}
		}
		if (node->next != head)
			node = WOG_GetNextObjectsInGridSquare(grid, objectTypes, num_objectTypes, head, node->next);
		else
			return numObjects;
	}
	return numObjects;
}

//fills in an array of all the matching objects within the radius of the central point
int WOG_FindObjectsInRadius(object_grid_t *grid, int *objectTypes, int num_objectTypes, 
						    float radius, vec3_t pos, int *objects, int maxObjects)
{
	int grid_x, grid_y, i, level, max_level, numObjects = 0;
	float maxdistsq;
	int numgrids_width;
	int numgrids_height;
	int width;

	if (!grid
		|| !grid->initialized)
		return -1;
	
	//max dist is squared, so we need to take the square root to use it to measure actual distance
	maxdistsq = radius*radius;
	//calc the max number of grids out we'll have to check
	FloatToInt(&width, (1 + radius / grid->grid_width));
	numgrids_width = MIN(width, OBJECT_GRID_WIDTH);
	numgrids_height = MIN(width, OBJECT_GRID_HEIGHT);
	
	grid_x = WORLD_TO_GRID_X(pos[X]);
	grid_y = WORLD_TO_GRID_Y(pos[Y]);

	max_level = MAX(numgrids_width, numgrids_height);

	if (max_level < 0)
	{
		Console_DPrintf("WOG Error, max_level is %i\n", max_level);
	}

	for (level = 0; level <= max_level; level++)
	{
		//width = level * 2 + 1;
		for (i = MAX(0, grid_x - level); i <= MIN(grid_x + level, OBJECT_GRID_WIDTH -1); i++)
		{
			if (grid_y - level >= 0
				&& grid_y - level < OBJECT_GRID_HEIGHT)
				//do the line at the top of the rect
				numObjects += WOG_FindObjectsInGridSquare(grid, objectTypes, num_objectTypes, 
														   pos, maxdistsq, objects, numObjects, 
														   maxObjects, i, grid_y - level);
			//if level is 0, the size of the box is 1, so these other "sides" are just wasted work
			if (level > 0 
				&& grid_y + level >= 0
				&& grid_y + level < OBJECT_GRID_HEIGHT)
				//do the line at the bottom of the rect
				numObjects += WOG_FindObjectsInGridSquare(grid, objectTypes, num_objectTypes, 
														   pos, maxdistsq, objects, numObjects, 
														   maxObjects, i, grid_y + level);
		}
		//if level is 0, the size of the box is 1, so these other "sides" are just wasted work
		if (level > 0)
		{
			for (i = MAX(0, grid_y - level + 1); i <= MIN(grid_y + level, OBJECT_GRID_WIDTH -1); i++)
			{
				if (grid_x - level >= 0
					&& grid_x - level  < OBJECT_GRID_WIDTH)
					//do the line on the left, excluding endpoints
					numObjects += WOG_FindObjectsInGridSquare(grid, objectTypes, num_objectTypes, 
															   pos, maxdistsq, objects, numObjects, 
															   maxObjects, grid_x - level, i);
				if (grid_x + level >= 0
					&& grid_x + level < OBJECT_GRID_WIDTH)
					//do the line on the right, excluding endpoints
					numObjects += WOG_FindObjectsInGridSquare(grid, objectTypes, num_objectTypes, 
											    			   pos, maxdistsq, objects, numObjects, 
															   maxObjects, grid_x + level, i);
			}
		}
	}
	return numObjects;
}

extern worldObject_t objects[];

bool	WOG_IsVisible(object_grid_t *grid, const vec2_t start, const vec2_t end, const vec3_t bmin, const vec3_t bmax)
{
	int i;
	vec3_t start_min, start_max;
	vec3_t end_min, end_max, axis[3];
	vec3_t obj_bmin, obj_bmax;
	float fraction;
	vec2_t rotated_start, rotated_end;
	//vec3_t diff, tmp_start, tmp_end;
	//object_grid_node_t *node;
	//int length, run, type=-1;
	//double x,y;
	//double xincrement;
	//double yincrement;

	
	M_AddVec2(start, bmin, start_min);
	M_AddVec2(start, bmax, start_max);

	M_AddVec2(end, bmin, end_min);
	M_AddVec2(end, bmax, end_max);
	

	/*
	for (run = 0; run < 2; run++)
	{
		switch (run)
		{
			case 0:
					M_AddVec3(start, bmin, tmp_start);
					M_AddVec3(end, bmin, tmp_end);
					break;
					
			case 1:
					M_AddVec3(start, bmax, tmp_start);
					M_AddVec3(end, bmax, tmp_end);
					break;
		}

		M_SubVec2(tmp_end, tmp_start, diff);
		diff[0] = WORLD_TO_GRID_X(diff[0]);
		diff[1] = WORLD_TO_GRID_Y(diff[0]);
		
		length = abs(diff[0]);
		if (abs(diff[1]) > length) length = abs(diff[1]);
		xincrement = (double)(diff[0])/(double)length;
		yincrement = (double)(diff[1])/(double)length;
		x = WORLD_TO_GRID_X(tmp_start[0] + 0.5); 
		y = WORLD_TO_GRID_Y(tmp_start[1] + 0.5);

		for (i = 1; i<= length;++i) 
		{
			node = &grid->object_grid[(int)x][(int)y];	

			while (node)
			{
				node = WOG_GetNextObjectsInGridSquare(grid, &type, 1, &grid->object_grid[(int)x][(int)y], node->next);
				if (node)
				{
					M_InvertAxis(objects[node->index].axis, axis);

					M_RotatePoint2d(tmp_start, (const vec3_t *)axis, rotated_start);
					M_RotatePoint2d(tmp_end, (const vec3_t *)axis, rotated_end);
					M_AddVec3(objects[node->index].bmin, objects[node->index].objpos.position, obj_bmin);
					M_AddVec3(objects[node->index].bmax, objects[node->index].objpos.position, obj_bmax);

					if (M_LineRectIntersect(rotated_start, rotated_end, obj_bmin, obj_bmax, &fraction))
						return false;
				}
			}
				
			x = x + xincrement;
			y = y + yincrement;
		}
	}
	*/

	
	for ( i = 0; i < MAX_WORLDOBJECTS; i++)
	{
		if (!objects[i].active)
			continue;

		M_InvertAxis((const vec3_t *)objects[i].axis, axis);

		M_RotatePoint2d(start_min, (const vec3_t *)axis, rotated_start);
		M_RotatePoint2d(end_min, (const vec3_t *)axis, rotated_end);

		M_AddVec3(objects[i].bmin, objects[i].objpos.position, obj_bmin);
		M_AddVec3(objects[i].bmax, objects[i].objpos.position, obj_bmax);

		if (M_LineRectIntersect(rotated_start, rotated_end, obj_bmin, obj_bmax, &fraction))
			return false;

		M_RotatePoint2d(start_max, (const vec3_t *)axis, rotated_start);
		M_RotatePoint2d(end_max, (const vec3_t *)axis, rotated_end);

		M_AddVec3(objects[i].bmin, objects[i].objpos.position, obj_bmin);
		M_AddVec3(objects[i].bmax, objects[i].objpos.position, obj_bmax);

		if (M_LineRectIntersect(rotated_start, rotated_end, obj_bmin, obj_bmax, &fraction))
			return false;
	}
	
	return true;
}
