#include "core.h"
#include "savage_types.h"
#include "quadtree.h"
#include "list.h"

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

IMPLEMENT_ALLOCATOR(quadtree_t);
IMPLEMENT_ALLOCATOR(quadtree_node_t);
IMPLEMENT_ALLOCATOR(quadtree_node_element_t);

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

quadtree_node_t* Quadtree_Node_Create(const vec2_t min, const vec2_t max)
{
	quadtree_node_t* node = ALLOCATE(quadtree_node_t);

	M_CopyVec2(min, node->min);
	M_CopyVec2(max, node->max);

	memset(node->children, 0, sizeof(void*)*4);
	
	node->elements.first = NULL;
	node->elements.last = NULL;

	// bottom-left
	node->children_min[0][0] = node->min[0];
	node->children_min[0][1] = node->min[1];
	node->children_max[0][0] = (node->max[0] + node->min[0])/2.0f;
	node->children_max[0][1] = (node->max[1] + node->min[1])/2.0f;

	// top-left
	node->children_min[1][0] = node->min[0];
	node->children_min[1][1] = (node->max[1] + node->min[1])/2.0f;
	node->children_max[1][0] = (node->max[0] + node->min[0])/2.0f;
	node->children_max[1][1] = node->max[1];

	// bottom-right
	node->children_min[2][0] = (node->max[0] + node->min[0])/2.0f;
	node->children_min[2][1] = node->min[1];
	node->children_max[2][0] = node->max[0];
	node->children_max[2][1] = (node->max[1] + node->min[1])/2.0f;

	// top-left
	node->children_min[3][0] = (node->max[0] + node->min[0])/2.0f;
	node->children_min[3][1] = (node->max[1] + node->min[1])/2.0f;
	node->children_max[3][0] = node->max[0];
	node->children_max[3][1] = node->max[1];
	
	return node;
}

//----------------------------------------------------------------------------

void Quadtree_Node_Destroy(quadtree_node_t* node)
{
	quadtree_node_element_t* element;
	int child;

	for ( child = 0 ; child < 4 ; ++child )
	{
		if ( node->children[child] )
		{
			Quadtree_Node_Destroy(node->children[child]);
		}
	}

	element = node->elements.first;
	while ( element )
	{
		quadtree_node_element_t* element_next = element->next;
		DEALLOCATE(quadtree_node_element_t, element);
		element = element_next;
	}

	DEALLOCATE(quadtree_node_t, node);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

quadtree_t* Quadtree_Create(const vec2_t min, const vec2_t max, int maxlevels)
{
	quadtree_t* quadtree;
	quadtree_node_t* node;
	
	quadtree = ALLOCATE(quadtree_t);
	node = Quadtree_Node_Create(min, max);
	
	quadtree->root = node;
	quadtree->maxlevels = maxlevels;

	return quadtree;
}

//----------------------------------------------------------------------------

void Quadtree_Destroy(quadtree_t* quadtree)
{
	Quadtree_Node_Destroy(quadtree->root);

	DEALLOCATE(quadtree_t, quadtree);
}

//----------------------------------------------------------------------------

void Quadtree_Add_Internal(quadtree_t* quadtree, quadtree_node_t* node, int level, void* data, const vec2_t min, const vec2_t max)
{
	quadtree_node_element_t* element;

	if ( level < quadtree->maxlevels )
	{
		int child;
		for ( child = 0 ; child < 4 ; ++child )
		{
			// if we fit entirely inside the child 

			if ( M_RectInRect(min, max, node->children_min[child], node->children_max[child]) )
			{
				if ( !node->children[child] )
				{
					node->children[child] = Quadtree_Node_Create(node->children_min[child], node->children_max[child]);
				}

				Quadtree_Add_Internal(quadtree, node->children[child], level+1, data, min, max);
				
				return;
			}
		}
	}

	element = ALLOCATE(quadtree_node_element_t);
	element->data = data;
	M_CopyVec2(min, element->min);
	M_CopyVec2(max, element->max);

	List_PushFront(&node->elements, element);
}

//----------------------------------------------------------------------------

void Quadtree_Add(quadtree_t* quadtree, void* data, const vec2_t min, const vec2_t max)
{
	Quadtree_Add_Internal(quadtree, quadtree->root, 1, data, min, max);
}

//----------------------------------------------------------------------------

bool Quadtree_Remove_Internal(quadtree_node_t* node, void* data, const vec2_t min, const vec2_t max)
{
	quadtree_node_element_t* element;

	{
		int child;
		for ( child = 0 ; child < 4 ; ++child )
		{
			if ( !node->children[child] ) continue;

			// if we fit entirely inside the child 

			if ( M_RectInRect(min, max, node->children_min[child], node->children_max[child]) )
			{
				bool result = Quadtree_Remove_Internal(node->children[child], data, min, max);

				if ( !node->elements.first &&
					 !node->children[child]->elements.first &&
					 !node->children[child]->children[0] &&
					 !node->children[child]->children[1] &&
					 !node->children[child]->children[2] &&
					 !node->children[child]->children[3] )
				{
					Quadtree_Node_Destroy(node->children[child]);
					node->children[child] = NULL;
				}

				return result;
			}
		}
	}

	element = node->elements.first;
	while ( element )
	{
		quadtree_node_element_t* element_next = element->next;
		
		if ( element->data == data )
		{
			List_Remove(&node->elements, element);
			DEALLOCATE(quadtree_node_element_t, element);
			
			return true;
		}
		
		element = element_next;
	}

	return false;
}

//----------------------------------------------------------------------------

bool Quadtree_Remove(quadtree_t* quadtree, void* data, const vec2_t min, const vec2_t max)
{
	return Quadtree_Remove_Internal(quadtree->root, data, min, max);
}

//----------------------------------------------------------------------------

bool Quadtree_Has_Internal(quadtree_node_t* node, void* data, const vec2_t min, const vec2_t max)
{
	quadtree_node_element_t* element;

	{
		int child;
		for ( child = 0 ; child < 4 ; ++child )
		{
			if ( !node->children[child] ) continue;

			// if we fit entirely inside the child 

			if ( M_RectInRect(min, max, node->children_min[child], node->children_max[child]) )
			{
				return Quadtree_Has_Internal(node->children[child], data, min, max);
			}
		}
	}

	element = node->elements.first;
	while ( element )
	{
		quadtree_node_element_t* element_next = element->next;
		
		if ( element->data == data )
		{
			return true;
		}
		
		element = element_next;
	}

	return false;
}

//----------------------------------------------------------------------------

bool Quadtree_Has(quadtree_t* quadtree, void* data, const vec2_t min, const vec2_t max)
{
	return Quadtree_Has_Internal(quadtree->root, data, min, max);
}

//----------------------------------------------------------------------------

void Quadtree_Find_Internal(quadtree_node_t* node, void** elements, int max_elements, int* num_elements, const vec2_t min, const vec2_t max)
{
	quadtree_node_element_t* element;

	element = node->elements.first;
	while ( element )
	{
		if ( M_2dBoundsIntersect(min, max, element->min, element->max) )
		{
			if ( *num_elements == max_elements ) return;

			elements[(*num_elements)++] = element->data;
		}
		element = element->next;
	}

	{
		int child;
		for ( child = 0 ; child < 4 ; ++child )
		{
			if ( !node->children[child] ) continue;

			// if we fit entirely inside the child 

			if ( M_2dBoundsIntersect(min, max, node->children_min[child], node->children_max[child]) )
			{
				Quadtree_Find_Internal(node->children[child], elements, max_elements, num_elements, min, max);
			}
		}
	}

}

//----------------------------------------------------------------------------

int Quadtree_Find(quadtree_t* quadtree, void** elements, int max_elements, const vec2_t min, const vec2_t max)
{
	int num_elements = 0;
	
	Quadtree_Find_Internal(quadtree->root, elements, max_elements, &num_elements, min, max);

	return num_elements;
}

//----------------------------------------------------------------------------

bool Quadtree_Lambda_Internal(quadtree_node_t* node, quadtree_lambda_fn fn, void* data1)
{
	quadtree_node_element_t* element;

	int child;
	for ( child = 0 ; child < 4 ; ++child )
	{
		if ( !node->children[child] ) continue;

		if ( !Quadtree_Lambda_Internal(node->children[child], fn, data1) )
		{
			return false;
		}
	}

	element = node->elements.first;
	while ( element )
	{
		if ( !fn(element->data, data1) )
		{
			return false;
		}

		element = element->next;
	}

	return true;
}

//----------------------------------------------------------------------------

void Quadtree_Lambda(quadtree_t* quadtree, quadtree_lambda_fn fn, void* data1)
{
	Quadtree_Lambda_Internal(quadtree->root, fn, data1);
}

//----------------------------------------------------------------------------

void Quadtree_LambdaFind_Internal(quadtree_node_t* node, quadtree_lambda_fn fn, void* data1, void** elements, int max_elements, int* num_elements)
{
	quadtree_node_element_t* element;

	element = node->elements.first;
	while ( element )
	{
		if ( fn(element->data, data1) )
		{
			if ( *num_elements == max_elements ) return;

			elements[(*num_elements)++] = element->data;
		}
		element = element->next;
	}

	{
		int child;
		for ( child = 0 ; child < 4 ; ++child )
		{
			if ( !node->children[child] ) continue;

			Quadtree_LambdaFind_Internal(node->children[child], fn, data1, elements, max_elements, num_elements);
		}
	}

}

//----------------------------------------------------------------------------

int Quadtree_LambdaFind(quadtree_t* quadtree, quadtree_lambda_fn fn, void* data1, void** elements, int max_elements)
{
	int num_elements = 0;
	
	Quadtree_LambdaFind_Internal(quadtree->root, fn, data1, elements, max_elements, &num_elements);

	return num_elements;
}
