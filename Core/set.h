#ifndef SET_H
#define SET_H

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

typedef bool (*set_compare_fn)(void* pv0, void* pv1);

#define SET_MAX_ELEMENTS 1024

typedef struct set_s
{
	int				numelements;
	void*			elements[SET_MAX_ELEMENTS];
	set_compare_fn	fncompare;
}
set_t;

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

void Set_Init(set_t* self, int size, set_compare_fn fncompare);
void Set_Term(set_t* self);

void Set_Insert(set_t* self, void* pv);
void Set_Remove(set_t* self, void* pv);

bool Set_IsEmpty(set_t* self);
int Set_GetSize(set_t* self);

#endif
