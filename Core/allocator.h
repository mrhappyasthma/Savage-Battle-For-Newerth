#ifndef ALLOCATOR_H
#define ALLOCATOR_H

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

#define ALLOCATE(x) \
	Allocator_Allocate(&allocator_##x, sizeof(x))

#define DEALLOCATE(x, pv) \
	Allocator_Deallocate(&allocator_##x, pv)

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

void* Allocator_Allocate(allocator_t* allocator, int size);
void Allocator_Deallocate(allocator_t* allocator, void* pv);

#endif
