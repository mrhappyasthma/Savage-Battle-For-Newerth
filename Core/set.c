#include "savage_common.h"
#include "set.h"

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

bool Set_Compare(void* pv0, void* pv1)
{
	return pv0 == pv1;
}

//----------------------------------------------------------------------------

bool Set_IsEmpty(set_t* self)
{
	return self->numelements == 0;
}

int Set_GetSize(set_t* self) 
{ 
	return self->numelements; 
}

void Set_Init(set_t* self, int size, set_compare_fn fncompare)
{
	self->numelements = 0;
//	self->elements = Tag_Malloc(sizeof(void*)*size, MEM_SET);
	if ( fncompare )
		self->fncompare = fncompare;
	else
		self->fncompare = Set_Compare;
}

void Set_Term(set_t* self)
{
//	Tag_Free(self->elements);
}

// $todo: optimize this using some kind of ordered tree, or at least binary search

void Set_Insert(set_t* self, void* pv)
{
	int i;
	for ( i = 0 ; i < self->numelements ; ++i )
	{
		if ( self->fncompare(pv, self->elements[i]) )
		{
			return;
		}
	}

	//ASSERT(self->elements < SET_MAX_ELEMENTS);

	self->elements[self->numelements++] = pv;
}

void Set_Remove(set_t* self, void* pv)
{
	int i;
	for ( i = 0 ; i < self->numelements ; ++i )
	{
		if ( self->fncompare(pv, self->elements[i]) )
		{
			self->elements[i] = self->elements[--self->numelements];
			return;
		}
	}
}
