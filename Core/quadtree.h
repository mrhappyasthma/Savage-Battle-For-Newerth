#ifndef QUADTREE_H
#define QUADTREE_H

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

EXTERN_ALLOCATOR(quadtree_t);
EXTERN_ALLOCATOR(quadtree_node_t);
EXTERN_ALLOCATOR(quadtree_node_element_t);

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

typedef struct quadtree_node_element_s
{
	void* data;
	vec2_t min;
	vec2_t max;

	struct quadtree_node_element_s*	prev;
	struct quadtree_node_element_s*	next;
}
quadtree_node_element_t;

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

typedef struct quadtree_node_elements_s
{
	quadtree_node_element_t*			first;
	quadtree_node_element_t*			last;
}
quadtree_node_elements_t;

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

typedef struct quadtree_node_s
{
	vec2_t						min, max;
	quadtree_node_elements_t	elements;
	struct quadtree_node_s*		children[4];
	vec2_t						children_min[4];
	vec2_t						children_max[4];
}
quadtree_node_t;

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

typedef struct quadtree_s
{
	quadtree_node_t*	root;
	int					maxlevels;
}
quadtree_t;

//----------------------------------------------------------------------------

typedef bool (*quadtree_lambda_fn)(void* data0, void* data1);

//----------------------------------------------------------------------------

quadtree_t* Quadtree_Create(const vec2_t min, const vec2_t max, int max_levels);
void Quadtree_Destroy(quadtree_t* quadtree);
void Quadtree_Add(quadtree_t* quadtree, void* data, const vec2_t min, const vec2_t max);
bool Quadtree_Remove(quadtree_t* quadtree, void* data, const vec2_t min, const vec2_t max);
bool Quadtree_Has(quadtree_t* quadtree, void* data, const vec2_t min, const vec2_t max);
int Quadtree_Find(quadtree_t* quadtree, void** elements, int max_elements, const vec2_t min, const vec2_t max);
void Quadtree_Lambda(quadtree_t* quadtree, quadtree_lambda_fn fn, void* data1);
int Quadtree_LambdaFind(quadtree_t* quadtree, quadtree_lambda_fn fn, void* data1, void** elements, int max_elements);

#endif
