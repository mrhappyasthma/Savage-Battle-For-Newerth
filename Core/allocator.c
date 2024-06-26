#include <stdlib.h>
#include "core.h"
//#include "allocator.h"

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

void* Allocator_Allocate(allocator_t* allocator, int size)
{
	void* pv = NULL;

	if ( !allocator->free )
	{
		pv = (void*)Tag_Malloc(size, MEM_ALLOCATOR);
		allocator->peak++; // debug only
	}
	else
	{
		void* ppv = (void*)allocator->free;
		pv = (void*)ppv;
		allocator->free = *(void**)allocator->free;
		allocator->saves++; // debug only
	}

	allocator->ref++; // debug only

	return pv;
}

//----------------------------------------------------------------------------

void Allocator_Deallocate(allocator_t* allocator, void* pv)
{
	*(void**)pv = allocator->free;
	allocator->free = pv;

	allocator->ref--; // debug only
}
